#include "Triangle.hpp"

//Triangle

Triangle::Triangle(TVec3d a,TVec3d b,TVec3d c)
{
    this->a = a;
    this->b = b;
    this->c = c;
    objectType = citygml::CityObjectsType::COT_All;
    subObjectType = citygml::CityObjectsType::COT_All;
}

TVec3d Triangle::GetNormal()
{
    TVec3d normal = (b - a).cross(c - a);
    return normal/normal.length();
}


//TriangleList

TriangleList::TriangleList(std::vector<Triangle*> triangles)
{
    this->triangles = triangles;
}

//TriangleList* BuildTriangleList(std::string tilefilename)
//{

//}

TriangleList* BuildTriangleList(std::string tilefilename, citygml::CityObjectsType objectType)
{
    std::vector<Triangle*> triangles;

    vcity::Tile* tile = new vcity::Tile(tilefilename);

    citygml::CityModel * model = tile->getCityModel();

    for(citygml::CityObject* obj : model->getCityObjectsRoots()) //For each city object
    {
        if(obj->getType() == citygml::COT_Building && objectType == citygml::COT_Building) //We only take building or terrain
        {
            for(citygml::CityObject* object : obj->getChildren())//On parcourt les objets (Wall, Roof, ...) du b�timent
                for(citygml::Geometry* Geometry : object->getGeometries()) //pour chaque g�om�trie
                    for(citygml::Polygon * PolygonCityGML : Geometry->getPolygons()) //Pour chaque polygone
                    {
                        //Get triangle list
                        const std::vector<TVec3d>& vert = PolygonCityGML->getVertices();
                        const std::vector<unsigned int>& ind = PolygonCityGML->getIndices();

                        for(unsigned int i = 0 ; i < ind.size() / 3; i++)//Push all triangle of the polygon in our list
                        {
                            TVec3d a = vert[ind[ i * 3 + 0 ]];
                            TVec3d b = vert[ind[ i * 3 + 1 ]];
                            TVec3d c = vert[ind[ i * 3 + 2 ]];

                            Triangle* t = new Triangle(a,b,c);
                            t->subObjectType = object->getType();
                            t->objectType = obj->getType();
                            t->objectId = obj->getId();
                            t->polygonId = PolygonCityGML->getId();
                            t->tileFile = tilefilename;

                            triangles.push_back(t);
                        }
                    }
        }
        // #CityObjectType
        // We check if the current cityobject is the same type of the wanted type of cityobject given in parameter
        // Exemple : (obj->getType() == citygml::COT_<MyType> && objectType == citygml::COT_<MyType>
        else if((obj->getType() == citygml::COT_SolitaryVegetationObject  && objectType == citygml::COT_SolitaryVegetationObject) ||
            (obj->getType() == citygml::COT_TINRelief  && objectType == citygml::COT_TINRelief) ||
            (obj->getType() == citygml::COT_WaterBody  && objectType == citygml::COT_WaterBody))
        {

            for(citygml::Geometry* Geometry : obj->getGeometries()) //pour chaque g�om�trie
                for(citygml::Polygon * PolygonCityGML : Geometry->getPolygons()) //Pour chaque polygone
                {
                    //Get triangle list
                    const std::vector<TVec3d>& vert = PolygonCityGML->getVertices();
                    const std::vector<unsigned int>& ind = PolygonCityGML->getIndices();

                    for(unsigned int i = 0 ; i < ind.size() / 3; i++)//Push all triangle of the polygon in our list
                    {
                        TVec3d a = vert[ind[ i * 3 + 0 ]];
                        TVec3d b = vert[ind[ i * 3 + 1 ]];
                        TVec3d c = vert[ind[ i * 3 + 2 ]];

                        Triangle* t = new Triangle(a,b,c);
                        t->objectType = obj->getType();
                        t->objectId = obj->getId();
                        t->polygonId = PolygonCityGML->getId();
                        t->tileFile = tilefilename;

                        triangles.push_back(t);
                    }
                }
        }
    }

    delete tile;

    return new TriangleList(triangles);
}
