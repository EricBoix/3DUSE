#ifndef __BATIMENTSHAPE_HPP__
#define __BATIMENTSHAPE_HPP__

////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "geos/geom/GeometryFactory.h"

////////////////////////////////////////////////////////////////////////////////

class BatimentShape
{
public:
	BatimentShape();
	BatimentShape(geos::geom::Geometry * Geo, std::string ID);
	geos::geom::Geometry * Geometry;
	std::string ID;

private:
};

#endif