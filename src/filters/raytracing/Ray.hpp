#ifndef __RAY_HPP__
#define __RAY_HPP__

//#include "AABB.hpp"
#include "osg/Camera"
#include "libcitygml/vecs.hpp"

struct Hit;
struct RayCollection;
struct ViewPoint;
class Triangle;

/**
*	@brief A simple ray
*/
struct Ray
{
	/**
	*	@brief Build a new ray
	*	@param ori Origin of the ray
	*	@param dir Direction of the ray
	*/
    Ray(TVec3d ori = TVec3d(0.0,0.0,0.0),TVec3d dir = TVec3d(1.0,1.0,1.0));

	/**
	*	@brief To know if the ray instersect a given triangle
	*	@param triangle The triangle to intersect
	*	@param hit Information about to hit of the ray on the triangle
	*	@return True if this ray intersect with triangle
	*/
	bool Intersect(Triangle* triangle, Hit* hit = nullptr);
	/**
    *	@brief To know if the ray instersects a given box
	*	@param box Box to intersect
    *	@param hitt0 t0 at when ray first hits the box
    *	@param hitt1 t1 at when ray first leaves the box
	*	@return True if this ray intersect with the box
	*/
    //bool Intersect(AABB box, float *hitt0, float *hitt1);

    /**
    *	@brief Build a ray
    *	@param fragCoord Coordinate on screen of the ray
    *	@param cam Camera used to build the ray
    *	@return A newly created ray coresponding to the given coord on the cam screen
    */
    static Ray* BuildRd(TVec2d fragCoord,osg::Camera* cam);

	static float DotCross(TVec3d v0, TVec3d v1,
		TVec3d v2);
	static TVec3d Normalized(TVec3d vec);

	TVec2d fragCoord;///< Fragment coordinate of the ray
	TVec3d ori;///< Origin of the ray
	TVec3d dir;///< Direction of the ray
	TVec3d inv_dir;///< inv Direction of the ray
	int sign[3];
	RayCollection* collection;///< Collection in which this ray belong

    //std::vector<RayBoxHit> boxes;///< List of boxes that this ray go through for the multitile algorithm
};

/**
*	@brief A collection of rays
*/
struct RayCollection
{
	/**
	*	@brief Build a new collection
	*/
    RayCollection(std::vector<Ray*> rays = std::vector<Ray*>());

//	/**
//	*	Delete of the rays of the collection
//	*/
//    ~RayCollection();

	/**
	*	@brief Build a collection of rays from a camera
	*	@param cam The camera we want to used to build the collection
	*/
	static RayCollection* BuildCollection(osg::Camera* cam);

	std::vector<Ray*> rays;///< The rays of the collection
    //ViewPoint* viewpoint;///< The viewpoint render using this collection
};

#endif
