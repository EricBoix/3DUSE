#include "AABB.hpp"

#include "gui/osg/osgScene.hpp"

#include "Ray.hpp"

#include <QDir>
#include <QFile>

#include <iostream>



std::pair<std::vector<AABB>,std::vector<AABB>> LoadAABB(std::string dir, std::string name)
{
	bool foundBuild = false;
	QFileInfo bDat;
	bool foundTerrain = false;
	QFileInfo tDat;

	QDir dt(dir.c_str());
	if(dt.exists())
	{
		for(QFileInfo f : dt.entryInfoList())
		{
			if(f.isFile())
			{
				if(f.fileName() == (name+"_BATI_AABB.dat").c_str())
				{
					bDat = f.absoluteFilePath();
					foundBuild = true;
				}
				if(f.fileName() == (name+"_MNT_AABB.dat").c_str())
				{
					tDat = f.absoluteFilePath();
					foundTerrain = true;
				}
			}
		}
	}
	else
	{
		std::cout << "Error, files does not exists." << std::endl;
	}

	std::vector<AABB> bSet;
	std::vector<AABB> tSet;


	if(foundBuild)
	{
		char line[256];

		std::ifstream ifs (dir+name+"_BATI_AABB.dat", std::ifstream::in);

		ifs.getline(line,256);

		unsigned int count = atoi(line);

		for(unsigned int i = 0; i < count; i++)
		{
			AABB box;
			ifs.getline(line,256);
			box.name = std::string(line);
			double minx;
			double miny;
			double minz;
			double maxx;
			double maxy;
			double maxz;
			
			ifs.getline(line,256);minx = atof(line);
			ifs.getline(line,256);miny = atof(line);
			ifs.getline(line,256);minz = atof(line);
			ifs.getline(line,256);maxx = atof(line);
			ifs.getline(line,256);maxy = atof(line);
			ifs.getline(line,256);maxz = atof(line);

			if(minx < maxx)
			{
				box.min = osg::Vec3d(minx,miny,minz);
				box.max = osg::Vec3d(maxx,maxy,maxz);
				bSet.push_back(box);
			}
		}

		ifs.close();
	}

	if(foundBuild)
	{
		char line[256];

		std::ifstream ifs (dir+name+"_MNT_AABB.dat", std::ifstream::in);

		ifs.getline(line,256);

		unsigned int count = atoi(line);

		for(unsigned int i = 0; i < count; i++)
		{
			AABB box;
			ifs.getline(line,256);
			box.name = std::string(line);
			double minx;
			double miny;
			double minz;
			double maxx;
			double maxy;
			double maxz;
			
			ifs.getline(line,256);minx = atof(line);
			ifs.getline(line,256);miny = atof(line);
			ifs.getline(line,256);minz = atof(line);
			ifs.getline(line,256);maxx = atof(line);
			ifs.getline(line,256);maxy = atof(line);
			ifs.getline(line,256);maxz = atof(line);

			if(minx < maxx)
			{
				box.min = osg::Vec3d(minx,miny,minz);
				box.max = osg::Vec3d(maxx,maxy,maxz);
				tSet.push_back(box);
			}
		}

		ifs.close();
	}

	return std::make_pair(bSet,tSet);
}

void BuildAABB(std::string dir, std::string name, TVec3d offset)
{
	bool foundBuild = false;
	QDir bDir;
	bool foundTerrain = false;
	QDir tDir;

	QDir dt(dir.c_str());
	if(dt.exists())
	{
		for(QFileInfo f : dt.entryInfoList())
		{
			std::cout << f.baseName().toAscii().data() << std::endl;
			if(f.isDir())
			{
				if(f.baseName() == (name+"_BATI").c_str())
				{
					bDir = f.absoluteFilePath();
					foundBuild = true;
				}
				if(f.baseName() == (name+"_MNT").c_str())
				{
					tDir = f.absoluteFilePath();
					foundTerrain = true;
				}
			}
		}
	}
	else
	{
		std::cout << "Error, dir does not exists." << std::endl;
	}


	std::map<std::string,std::pair<TVec3d,TVec3d>> bAABBs;
	std::map<std::string,std::pair<TVec3d,TVec3d>> tAABBs;



	if(foundBuild)
	{
		for(QFileInfo f : bDir.entryInfoList())
		{
			TVec3d min(FLT_MAX,FLT_MAX,FLT_MAX);
			TVec3d max(-FLT_MAX,-FLT_MAX,-FLT_MAX);

			std::cout << f.absoluteFilePath().toAscii().data() << std::endl;
			if(f.absoluteFilePath().endsWith(".gml"))
			{
				vcity::Tile* tile = new vcity::Tile(f.absoluteFilePath().toAscii().data());

				citygml::CityModel * model = tile->getCityModel();
				for(citygml::CityObject* obj : model->getCityObjectsRoots()) //For each city object
					if(obj->getType() == citygml::COT_Building ) //We only take building or terrain
						for(citygml::CityObject* object : obj->getChildren())//On parcourt les objets (Wall, Roof, ...) du b�timent
							for(citygml::Geometry* Geometry : object->getGeometries()) //pour chaque g�om�trie
								for(citygml::Polygon * PolygonCityGML : Geometry->getPolygons()) //Pour chaque polygone
								{
									//Get triangle list
									const std::vector<TVec3d>& vert = PolygonCityGML->getVertices();
									const std::vector<unsigned int>& ind = PolygonCityGML->getIndices();

									for(unsigned int i = 0 ; i < ind.size() / 3; i++)//Push all triangle of the polygon in our list
									{
										TVec3d a = vert[ind[ i * 3 + 0 ]] - offset;
										TVec3d b = vert[ind[ i * 3 + 1 ]] - offset;
										TVec3d c = vert[ind[ i * 3 + 2 ]] - offset;
										min.x = std::min(a.x,min.x);min.y = std::min(a.y,min.y);min.z = std::min(a.z,min.z);
										min.x = std::min(b.x,min.x);min.y = std::min(b.y,min.y);min.z = std::min(b.z,min.z);
										min.x = std::min(c.x,min.x);min.y = std::min(c.y,min.y);min.z = std::min(c.z,min.z);
										max.x = std::max(a.x,max.x);max.y = std::max(a.y,max.y);max.z = std::max(a.z,max.z);
										max.x = std::max(b.x,max.x);max.y = std::max(b.y,max.y);max.z = std::max(b.z,max.z);
										max.x = std::max(c.x,max.x);max.y = std::max(c.y,max.y);max.z = std::max(c.z,max.z);
									}
								}

								bAABBs.insert(std::make_pair(f.baseName().toAscii().data(),std::make_pair(min,max)));

								delete tile;
			}
		}
	}

	if(foundTerrain)
	{
		for(QFileInfo f : tDir.entryInfoList())
		{
			TVec3d min(FLT_MAX,FLT_MAX,FLT_MAX);
			TVec3d max(-FLT_MAX,-FLT_MAX,-FLT_MAX);

			std::cout << f.absoluteFilePath().toAscii().data() << std::endl;
			if(f.absoluteFilePath().endsWith(".gml"))
			{
				vcity::Tile* tile = new vcity::Tile(f.absoluteFilePath().toAscii().data());

				citygml::CityModel * model = tile->getCityModel();
				for(citygml::CityObject* obj : model->getCityObjectsRoots()) //For each city object
					if(obj->getType() == citygml::COT_TINRelief ) //We only take building or terrain
						for(citygml::Geometry* Geometry : obj->getGeometries()) //pour chaque g�om�trie
							for(citygml::Polygon * PolygonCityGML : Geometry->getPolygons()) //Pour chaque polygone
							{
								//Get triangle list
								const std::vector<TVec3d>& vert = PolygonCityGML->getVertices();
								const std::vector<unsigned int>& ind = PolygonCityGML->getIndices();

								for(unsigned int i = 0 ; i < ind.size() / 3; i++)//Push all triangle of the polygon in our list
								{
									TVec3d a = vert[ind[ i * 3 + 0 ]] - offset;
									TVec3d b = vert[ind[ i * 3 + 1 ]] - offset;
									TVec3d c = vert[ind[ i * 3 + 2 ]] - offset;
									min.x = std::min(a.x,min.x);min.y = std::min(a.y,min.y);min.z = std::min(a.z,min.z);
									min.x = std::min(b.x,min.x);min.y = std::min(b.y,min.y);min.z = std::min(b.z,min.z);
									min.x = std::min(c.x,min.x);min.y = std::min(c.y,min.y);min.z = std::min(c.z,min.z);
									max.x = std::max(a.x,max.x);max.y = std::max(a.y,max.y);max.z = std::max(a.z,max.z);
									max.x = std::max(b.x,max.x);max.y = std::max(b.y,max.y);max.z = std::max(b.z,max.z);
									max.x = std::max(c.x,max.x);max.y = std::max(c.y,max.y);max.z = std::max(c.z,max.z);
								}
							}

							tAABBs.insert(std::make_pair(f.baseName().toAscii().data(),std::make_pair(min,max)));

							delete tile;
			}
		}
	}


	std::filebuf fb;
	fb.open(dir+name+"_BATI_AABB.dat",std::ios::out);

	std::ostream file(&fb);

	file << bAABBs.size() << "\n";

	for(std::pair<std::string,std::pair<TVec3d,TVec3d>> p : bAABBs)
	{
		file << p.first << "\n";
		file << p.second.first.x << "\n";
		file << p.second.first.y << "\n";
		file << p.second.first.z << "\n";
		file << p.second.second.x << "\n";
		file << p.second.second.y << "\n";
		file << p.second.second.z << "\n";
	}

	fb.close();

	std::filebuf ft;
	ft.open(dir+name+"_MNT_AABB.dat",std::ios::out);

	std::ostream filet(&ft);

	filet << tAABBs.size() << "\n";

	for(std::pair<std::string,std::pair<TVec3d,TVec3d>> p : tAABBs)
	{
		filet << p.first << "\n";
		filet << p.second.first.x << "\n";
		filet << p.second.first.y << "\n";
		filet << p.second.first.z << "\n";
		filet << p.second.second.x << "\n";
		filet << p.second.second.y << "\n";
		filet << p.second.second.z << "\n";
	}

	fb.close();

	std::cout << "Done." << std::endl;
}