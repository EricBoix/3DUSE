////////////////////////////////////////////////////////////////////////////////
#ifndef _SHPEXTRUSION_
#define _SHPEXTRUSION_
////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include "core/application.hpp"
////////////////////////////////////////////////////////////////////////////////

typedef std::vector<TVec3d> LRing;

/**
*	@brief Extrude a chosen Shp file
*/
citygml::CityModel* ShpExtrusion(OGRDataSource* poDS, std::string dir);

/**
*	@brief Put a set of point a the height of the terrain
*/
LRing PutLRingOnTerrain(LRing ring, std::string dir);

#endif