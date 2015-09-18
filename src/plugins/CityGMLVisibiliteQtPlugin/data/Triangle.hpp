#ifndef __TRIANGLE_HPP__
#define __TRIANGLE_HPP__

#include "gui/osg/osgScene.hpp"

/**
*	@brief A triangle create from a citygml polygon
*/
struct Triangle
{
	/**
	*	@brief Build a new triangle
	*	@param a First point of the triangle
	*	@param b Second point of the triangle
	*	@param c Third point of the triangle
	*/
	Triangle(TVec3d a = TVec3d(0.0,0.0,0.0),TVec3d b = TVec3d(0.0,0.0,0.0),TVec3d c = TVec3d(0.0,0.0,0.0))
	{
		this->a = a;
		this->b = b;
		this->c = c;
		objectType = citygml::CityObjectsType::COT_All;
		subObjectType = citygml::CityObjectsType::COT_All;
	}

	TVec3d GetNormal()
	{
		TVec3d normal = (b - a).cross(c - a);
		return normal/normal.length();
	}

	TVec3d a; ///< First point of the triangle
	TVec3d b; ///< Second point of the triangle
	TVec3d c; ///< Third point of the triangle

	citygml::CityObjectsType objectType;
	citygml::CityObjectsType subObjectType;
	std::string objectId;
	std::string polygonId;
	std::string tileFile;
};

/**
*	A list of triangle
*
*/
struct TriangleList
{
	/**
	*	@brief Build a new collection of triangle
	*/
	TriangleList(std::vector<Triangle*> triangles = std::vector<Triangle*>())
	{
		this->triangles = triangles;
	}

	/**
	*	@brief Delete all triangles from the list
	*/
	~TriangleList()
	{
		for(unsigned int i = 0; i < triangles.size(); i++)
		{
			delete triangles[i];
		}
	}

	std::vector<Triangle*> triangles;///< Triangles of the list
};

/**
*	@brief Build list of triangle from a CityGML building tile
*	@param tile CityGML tile from which we want the triangle list
*	@param viewpoint Data about the viewpoint we are rendering
*	@param objectType The type of cityobject to load
*	@return The list of triangle from the CityGML tile
*/
TriangleList* BuildTriangleList(std::string tilefilename, citygml::CityObjectsType objectType);

#endif