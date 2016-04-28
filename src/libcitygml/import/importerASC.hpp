#ifndef _IMPORTERASC_HPP_
#define _IMPORTERASC_HPP_

#include <queue>

#include "importer.hpp"
#include "../citygml.hpp"


#include "src/DataStructures/DEM/osgMnt.hpp"
#include "src/gui/osg/osgGDAL.hpp"

namespace citygml
{

  class ImporterASC : public Importer
  {
  public:
    ImporterASC(void);
    ~ImporterASC(void);

    CityModel* reliefToCityGML(MNT*);
    CityModel* waterToCityGML(MNT*);
    CityObject* waterToCityGMLPolygons(MNT*, float);
    CityModel* fusionResolutions(MNT* asc1, MNT* asc2);

  protected:
    Geometry* generateTriangles(MNT*);
    void propagateCategory(MNT*, std::queue<std::pair<int, int>>*, float alt, float zPrec);
    OGRPolygon* createPoly(MNT*, int x, int y, float prec);

    std::vector<OGRPolygon*> geom_list;
    bool* treated;
  };

} //namespace citygml

#endif //_IMPORTERASC_HPP_
