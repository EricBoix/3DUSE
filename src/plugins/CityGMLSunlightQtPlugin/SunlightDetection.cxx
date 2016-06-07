#include "SunlightDetection.h"

#include <fstream>
#include <queue>

#include "AABB.hpp"
#include "Triangle.hpp"
#include "libfilters/raytracing/RayTracing.hpp"
#include "libfilters/raytracing/Hit.hpp"
#include "src/core/RayBox.hpp"
#include "quaternion.hpp"
#include "core/dateTime.hpp"

#include "IO.h"
#include "FileInfo.h"


///
/// \brief SetupFileOrder create a std::queue queueing the files intersected by a given RayBoxCollection and sorted by intersection distance
/// \param boxes Vector of files' bounding boxes to check intersection with
/// \param rays Rays for intersection with bounding boxes testing
/// \return a std::queue of RayboxHit sorted by intersection distance (min first)
///
std::queue<RayBoxHit> SetupFileOrder(std::vector<AABB> boxes,RayBoxCollection* rays)
{
    std::map<std::string,int> boxToMaxOrder; //The order in which the box has been crossed : if a ray cross multiple AABB, we record the order of crossing in each one
    std::map<std::string,RayBoxHit> boxToRayBoxHit;//We keep record of the smallest rayboxhit of a box /// MultiResolution

    for(unsigned int i = 0; i < boxes.size(); i++)
    {
        boxToMaxOrder[boxes[i].name] = -1;
    }

    //For each ray
    for(unsigned int i = 0; i < rays->raysBB.size(); i++)
    {
        RayBox* rayBox = rays->raysBB[i];
        rayBox->boxes.clear();

        //For each box we check if the ray intersect it
        for(unsigned int j = 0; j < boxes.size(); j++)
        {
            float hit0,hit1;
            if(rayBox->Intersect(boxes[j],&hit0,&hit1))
            {
                RayBoxHit hTemp;
                hTemp.box = boxes[j];
                hTemp.minDistance = hit0;
                rayBox->boxes.push_back(hTemp); //Add this box to list of intersected boxes by this ray
            }
        }

        std::sort(rayBox->boxes.begin(),rayBox->boxes.end());//Sort the boxes depending on their intersection distance

        for(std::size_t j = 0; j < rayBox->boxes.size(); j++)//We update the order of each boxes
        {
            int current = boxToMaxOrder[rayBox->boxes[j].box.name];
            boxToMaxOrder[rayBox->boxes[j].box.name] = std::max(static_cast<int>(j),current);

            if(boxToRayBoxHit.find(rayBox->boxes[j].box.name) != boxToRayBoxHit.end())//= Si rayBox->boxes[j].box.name existe d�j� dans boxToRayBoxHit/// MultiResolution
            {
                boxToRayBoxHit[rayBox->boxes[j].box.name] = std::min(rayBox->boxes[j],boxToRayBoxHit[rayBox->boxes[j].box.name]);
            }
            else
            {
                boxToRayBoxHit.insert(std::make_pair(rayBox->boxes[j].box.name,rayBox->boxes[j]));
            }
        }
    }

    //Sort the boxes depending on their max order
    std::vector<BoxOrder> boxesOrder;

    for(auto it = boxToMaxOrder.begin();it != boxToMaxOrder.end(); it++)
    {
        if(it->second >= 0)
        {
            BoxOrder boTemp;
            boTemp.box = it->first;
            boTemp.order = it->second;
            boxesOrder.push_back(boTemp);
        }
    }

    std::sort(boxesOrder.begin(),boxesOrder.end());

    //Setup the queue and return it
    std::queue<RayBoxHit> fileOrder;

    for(BoxOrder& bo : boxesOrder)
        fileOrder.push(boxToRayBoxHit[bo.box]);

    return fileOrder;
}


void SunlightDetection(std::string fileDir, std::vector<FileInfo*> filenames, std::string sunpathFile, std::string startDate, std::string endDate, QString outputDir)
{

    QTime time;
    time.start();

    std::cout << "Sunlight Calculation started." << std::endl;

    //Create output folders
    createOutputFolders(outputDir);

    //Convert dates to integer
    int iStartDate = encodeDateTime(startDate,0);
    int iEndDate = encodeDateTime(endDate,23);

    // *** Compute sun's beam direction from sunpathFile and associate them to an hour encoded as an int. *** //
    std::map<int,TVec3d> SunsBeamsDir = loadSunpathFile(sunpathFile, iStartDate, iEndDate);

    // *** Build datetime_sunnyMap : result map associating a datetime to sunny info *** //
    //This map is created once as the sun beams are always the same in one simulation and will be associated with each triangle
    std::map<int,bool> datetime_sunnyMap;

    for(auto const& beamdir : SunsBeamsDir) //For all sun beams
    {
        if(beamdir.second == TVec3d(0.0,0.0,0.0)) //If beam direction is nul, i.e. sun is down
            datetime_sunnyMap[beamdir.first] = false; //sunny = false
        else
            datetime_sunnyMap[beamdir.first] = true; // sunny = true
    }


    // *** Load AABB of all files *** //
    AABBCollection boxes = LoadAABB(fileDir);

    //Concatenate buildingAABB and terrainAABB
    std::vector<AABB> building_terrainBB;

    building_terrainBB.reserve(boxes.building.size() + boxes.terrain.size()); //preallocate memory
    building_terrainBB.insert( building_terrainBB.end(), boxes.building.begin(), boxes.building.end() ); // insert building AABB
    building_terrainBB.insert( building_terrainBB.end(), boxes.terrain.begin(), boxes.terrain.end() ); // insert terrain AABB


    // *** Load files to analyse *** //

    unsigned int cpt_files = 1;
    int time_tot = 0;

    for(FileInfo* f : filenames) //Loop through files
    {

        std::cout << "===================================================" << std::endl;
        std::cout << "Computation of file " << f->WithPrevFolderAndGMLExtension() << "..." << std::endl;
        std::cout << "===================================================" << std::endl;

        //Load TriangleList of file to compute sunlight for
        TriangleList* trianglesfile;

        if(f->m_type == fileType::_BATI)
            trianglesfile = BuildTriangleList(f->m_filepath,citygml::CityObjectsType::COT_Building);
        else if(f->m_type == fileType::_MNT)
            trianglesfile = BuildTriangleList(f->m_filepath,citygml::CityObjectsType::COT_TINRelief);
        else
            trianglesfile = new TriangleList();

        //Create csv file where results will be written
        createFileFolder(f, outputDir);

        int cpt_tri = 1;//output print purpose

        for(Triangle* t : trianglesfile->triangles) //Loop through each triangle
        {
            std::cout << "Triangle " << cpt_tri << " of " << trianglesfile->triangles.size() << "..." << std::endl;

            //Initialize sunlight Info results
            std::map<int,bool> datetimeSunInfo = datetime_sunnyMap;

            //Compute Barycenter of triangle
            TVec3d barycenter = TVec3d();
            barycenter.x = (t->a.x + t->b.x + t->c.x) / 3;
            barycenter.y = (t->a.y + t->b.y + t->c.y) / 3;
            barycenter.z = (t->a.z + t->b.z + t->c.z) / 3;

            //Create rayBoxCollection (All the rays for this triangle)
            RayBoxCollection* raysboxes = new RayBoxCollection();

            for(auto const& beamdir : SunsBeamsDir)
            {
                //If direction is null (ie sun is too low) leave triangle in the shadow and go to next iteration
                if(beamdir.second == TVec3d(0.0,0.0,0.0))
                    continue;

                //Add an offset for raytracing
                TVec3d tmpBarycenter = TVec3d(0.0,0.0,0.0);
                tmpBarycenter.x = barycenter.x + 0.01f*beamdir.second.x;
                tmpBarycenter.y = barycenter.y + 0.01f*beamdir.second.y;
                tmpBarycenter.z = barycenter.z + 0.01f*beamdir.second.z;

                //Add ray to list
                RayBox* raybox = new RayBox(tmpBarycenter,beamdir.second,beamdir.first);
                raysboxes->raysBB.push_back(raybox);
            }

            //Setup and get the file's boxes in the right intersection order
            std::queue<RayBoxHit> fileOrder = SetupFileOrder(building_terrainBB,raysboxes);

            //While we have boxes, files
            while(fileOrder.size() != 0)
            {
                RayBoxHit myRayBoxHit = fileOrder.front();
                std::string fileName_boxhit = myRayBoxHit.box.name;
                fileOrder.pop();

                RayCollection raysTemp;//Not all rays intersect the box

                //We only get the rays that intersect the box
                for(unsigned int i = 0; i < raysboxes->raysBB.size(); i++)
                {
                    RayBox* raybox = raysboxes->raysBB[i];//Get the ray

                    bool found = false;

                    for(RayBoxHit& rbh : raybox->boxes)//Go through all the box that the ray intersect to see if the current box is one of them
                        found = found || rbh.box.name == fileName_boxhit;

                    if(found)
                    {
                        raysTemp.rays.push_back(raybox);
                    }
                }

                if(raysTemp.rays.size() == 0)
                {
                    raysTemp.rays.clear();
                    continue;
                }

                //*** Load triangles and perform analysis ***//

                std::string path_boxhit = fileDir + fileName_boxhit;

                FileInfo fBoxHit = FileInfo(path_boxhit);

                //Get the triangle list of files matching intersected AABB
                TriangleList* trianglesTemp;

                if(fBoxHit.m_type == fileType::_BATI)
                    trianglesTemp = BuildTriangleList(fBoxHit.m_filepath,citygml::CityObjectsType::COT_Building);
                else if(fBoxHit.m_type == fileType::_MNT)
                    trianglesTemp = BuildTriangleList(fBoxHit.m_filepath,citygml::CityObjectsType::COT_TINRelief);


                //Perform raytracing
                std::vector<Hit*>* tmpHits = RayTracing(trianglesTemp,raysTemp.rays);

                //Change sunlight information according to hits found
                for(Hit* h : *tmpHits)
                {
                    datetimeSunInfo[h->ray.id] = false;
                }

                //Delete triangles
                delete trianglesTemp;

                //Clear rays
                raysTemp.rays.clear();

                //Delete hits
                for(unsigned int i = 0 ; i < tmpHits->size() ; ++i)
                    delete tmpHits->at(i);

                delete tmpHits;
            }

            exportLightningToCSV(datetimeSunInfo,t,f, iStartDate, iEndDate, outputDir); //Export result for this triangle in csv files

            //Delete RayBoxes
            delete raysboxes;

            ++cpt_tri;
        }

        //Delete TriangleList
        delete trianglesfile;

        std::cout << "===================================================" << std::endl;
        std::cout << "file " << cpt_files << " of " << filenames.size() << " done in : " << (double)time.elapsed()/60000.0 << " min." << std::endl;
        std::cout << "===================================================" << std::endl;

        time_tot += time.elapsed();
        time.restart();
        ++cpt_files;

    }

    //Delete files info
    for(unsigned int i = 0 ; i < filenames.size() ; ++i)
        delete filenames[i];

    std::cout << "Total time : " << (double)time_tot/60000.0 << " min" << std::endl;
}
