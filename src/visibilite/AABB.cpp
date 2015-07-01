#include "AABB.hpp"

#include "gui/osg/osgScene.hpp"

#include "Ray.hpp"

#include <QDir>
#include <QFile>

#include <iostream>

#include "visibilite/Visibilite.hpp"

std::vector<AABB> DoLoadAABB(std::string path)
{
	std::vector<AABB> bSet;

	char line[256];

	std::ifstream ifs (path, std::ifstream::in);

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

		if(minx < maxx && miny < maxy && minz < maxz)
		{
			box.min = osg::Vec3d(minx,miny,minz);
			box.max = osg::Vec3d(maxx,maxy,maxz);
			bSet.push_back(box);

		}
	}

	ifs.close();

	return bSet;
}

AABBCollection LoadAABB(std::string dir)
{
	bool foundBuild = false;
	QFileInfo bDat;
	bool foundTerrain = false;
	QFileInfo tDat;
	bool foundWater = false;
	QFileInfo wDat;

	//Check if our bounding box files do exists
	QDir dt(dir.c_str());
	if(dt.exists())
	{
		for(QFileInfo f : dt.entryInfoList())
		{
			if(f.isFile())
			{
				if(f.fileName() == "_BATI_AABB.dat")
				{
					bDat = f.absoluteFilePath();
					foundBuild = true;
				}
				if(f.fileName() == "_MNT_AABB.dat")
				{
					tDat = f.absoluteFilePath();
					foundTerrain = true;
				}
				if(f.fileName() == "_WATER_AABB.dat")
				{
					wDat = f.absoluteFilePath();
					foundWater = true;
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
	std::vector<AABB> wSet;


	if(foundBuild)
		bSet = DoLoadAABB(dir+"_BATI_AABB.dat");

	if(foundTerrain)
		tSet = DoLoadAABB(dir+"_MNT_AABB.dat");

	if(foundWater)
		wSet = DoLoadAABB(dir+"_WATER_AABB.dat");

	AABBCollection collection;
	collection.building = bSet;
	collection.terrain = tSet;
	collection.water = wSet;

	return collection;
}

std::map<std::string,std::pair<TVec3d,TVec3d>> DoBuildAABB(std::vector<QDir> dirs, TVec3d offset, citygml::CityObjectsType type)
{
	std::map<std::string,std::pair<TVec3d,TVec3d>> AABBs;

	for(QDir bDir : dirs)
	{
		for(QFileInfo f : bDir.entryInfoList())
		{
			TVec3d min(FLT_MAX,FLT_MAX,FLT_MAX);
			TVec3d max(-FLT_MAX,-FLT_MAX,-FLT_MAX);

			if(f.absoluteFilePath().endsWith(".gml"))
			{
				vcity::Tile* tile = new vcity::Tile(f.absoluteFilePath().toAscii().data());

				TriangleList* list = BuildTriangleList(tile,offset,nullptr,type);

				for(Triangle* t : list->triangles)
				{
					min.x = std::min(t->a.x,min.x);min.y = std::min(t->a.y,min.y);if(t->a.z > -500) min.z = std::min(t->a.z,min.z);
					min.x = std::min(t->b.x,min.x);min.y = std::min(t->b.y,min.y);if(t->b.z > -500) min.z = std::min(t->b.z,min.z);
					min.x = std::min(t->c.x,min.x);min.y = std::min(t->c.y,min.y);if(t->b.z > -500) min.z = std::min(t->c.z,min.z);
					max.x = std::max(t->a.x,max.x);max.y = std::max(t->a.y,max.y);if(t->a.z < 1000) max.z = std::max(t->a.z,max.z);
					max.x = std::max(t->b.x,max.x);max.y = std::max(t->b.y,max.y);if(t->b.z < 1000) max.z = std::max(t->b.z,max.z);
					max.x = std::max(t->c.x,max.x);max.y = std::max(t->c.y,max.y);if(t->c.z < 1000) max.z = std::max(t->c.z,max.z);
				}

				AABBs.insert(std::make_pair((bDir.dirName()+"/"+f.fileName()).toAscii().data(),std::make_pair(min,max)));
				std::cout << (bDir.dirName()+"/"+f.fileName()).toAscii().data() << std::endl;
				
				delete list;
				delete tile;
			}
		}
	}

	return AABBs;
}

void DoSaveAABB(std::string filePath, std::map<std::string,std::pair<TVec3d,TVec3d>> AABBs)
{
	std::filebuf fb;
	fb.open(filePath,std::ios::out);

	std::ostream file(&fb);

	file << AABBs.size() << "\n";

	for(std::pair<std::string,std::pair<TVec3d,TVec3d>> p : AABBs)
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
}

void BuildAABB(std::string dir, TVec3d offset)
{
	std::vector<QDir> bDirs;
	std::vector<QDir> tDirs;
	std::vector<QDir> wDirs;

	QDir dt(dir.c_str());
	if(dt.exists())
	{
		for(QFileInfo f : dt.entryInfoList())
		{
			std::cout << f.baseName().toAscii().data() << std::endl;
			if(f.isDir())
			{
				if(f.baseName().endsWith("_BATI"))
					bDirs.push_back(f.absoluteFilePath());
				if(f.baseName().endsWith("_MNT"))
					tDirs.push_back(f.absoluteFilePath());
				if(f.baseName().endsWith("_WATER"))
					wDirs.push_back(f.absoluteFilePath());
			}
		}
	}
	else
	{
		std::cout << "Error, dir does not exists." << std::endl;
	}


	std::map<std::string,std::pair<TVec3d,TVec3d>> bAABBs = DoBuildAABB(bDirs,offset,citygml::CityObjectsType::COT_Building);
	std::map<std::string,std::pair<TVec3d,TVec3d>> tAABBs = DoBuildAABB(tDirs,offset,citygml::CityObjectsType::COT_TINRelief);
	std::map<std::string,std::pair<TVec3d,TVec3d>> wAABBs = DoBuildAABB(wDirs,offset,citygml::CityObjectsType::COT_WaterBody);

	DoSaveAABB(dir+"_BATI_AABB.dat",bAABBs);
	DoSaveAABB(dir+"_MNT_AABB.dat",tAABBs);
	DoSaveAABB(dir+"_WATER_AABB.dat",wAABBs);

	std::cout << "Done." << std::endl;
}