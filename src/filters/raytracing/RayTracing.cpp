#include "RayTracing.hpp"

#include "Hit.hpp"
//#include "Ray.hpp"
//#include "../include/ViewPoint.hpp"

#include <thread>

/**
*	@brief Data used by a ray tracing thread
*/
struct RayTracingData
{
    TriangleList* triangles; ///< List of triangles of a 3D Model
    std::vector<Ray*>* rowToDo; ///< List of ray to use for ray tracing
    std::vector<Hit*>* Hits; ///< List of hits
};

//Loop through all triangles and check if any rays intersect with triangles
void RayLoop(RayTracingData data)
{
    for(unsigned int k = 0; k < data.rowToDo->size(); k++)
    {
        Ray* ray = data.rowToDo->at(k);

        for(unsigned int l = 0; l < data.triangles->triangles.size(); l++)
        {
            Triangle* tri = data.triangles->triangles.at(l);

            Hit* hit = new Hit();
            if(ray->Intersect(tri,hit))//Check if the ray hit the triangle and
            {
                data.Hits->push_back(hit);
            }
            else
            {
                delete hit;
            }
        }

    }
}

std::vector<Hit*>* RayTracing(TriangleList* triangles, std::vector<Ray*> rays)
{
    QTime time;
    time.start();

    unsigned int tCount = std::thread::hardware_concurrency() - 1;//Get how many thread we have
    unsigned int rayPerThread = rays.size() / tCount;

    //List of rays and their frag coord
    std::vector<Ray*>* toDo = new std::vector<Ray*>[tCount];//List of rays for each threads

    for(unsigned int i = 0; i < tCount; i++)
    {
        toDo[i].insert(toDo[i].begin(),rays.begin()+i*rayPerThread,rays.begin()+(i+1)*rayPerThread);
    }

    //List of hits for each thread
    std::vector<Hit*>* hitsArray = new std::vector<Hit*>[tCount];

    std::cout << "Thread : " << tCount << std::endl;
    std::cout << "Ray count : " << rays.size() << std::endl;

    std::vector<std::thread*> threads;//Our thread list

    for(unsigned int i = 0; i < tCount; i++)
    {
        RayTracingData data;
        data.triangles = triangles;
        data.rowToDo = &toDo[i];
        data.Hits = &hitsArray[i];

        std::thread* t = new std::thread(RayLoop,data);
        threads.push_back(t);
    }

    std::cout << "Thread Launched " << std::endl;

    for(unsigned int i = 0; i < tCount; i++)//Join all our thread and update global data min and max distance
    {
        (*threads[i]).join();
        delete threads[i];
    }

    std::cout << "The joining is completed" << std::endl;

    //Join vector of hits
    std::vector<Hit*>* hits = new std::vector<Hit*>();

    // preallocate memory
     std::vector<Hit*>::size_type mem_size = 0;

    for(unsigned int i = 0; i < tCount; i++)
    {
        std::cout << "Thread " << i << " returned " << hitsArray[i].size() << " hits." << std::endl;
        mem_size += hitsArray[i].size();
    }

    std::cout << "Total Memory Size : " << mem_size << std::endl;

    hits->reserve(mem_size);

    std::cout << "Memory allocated" << std::endl;

    for(unsigned int i = 0; i < tCount; i++)
    {
        hits->insert( hits->end(), hitsArray[i].begin(), hitsArray[i].end() );
    }

    std::cout << "Merging hits done. Size hits : " << hits->size() << std::endl;

    delete[] toDo;

    std::cout << "Time : " << time.elapsed()/1000 << " sec" << std::endl;

    return hits;
}
