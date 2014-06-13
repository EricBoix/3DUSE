#ifndef __OSGGDAL_HPP__
#define __OSGGDAL_HPP__
////////////////////////////////////////////////////////////////////////////////
#include "ogrsf_frmts.h"
#include <osg/Geode>

#include "geos/geom/GeometryFactory.h"
#include "core/BatimentShape.hpp"

////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Geode> buildOsgGDAL(OGRDataSource* poDS);
void buildGeosShape(OGRDataSource* poDS, geos::geom::Geometry ** ShapeGeo, std::vector<std::pair<double, double>> * Hauteurs, std::vector<BatimentShape> * BatimentsInfo);
////////////////////////////////////////////////////////////////////////////////
#endif // __OSGGDAL_HPP__
