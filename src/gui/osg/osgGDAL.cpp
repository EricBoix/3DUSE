////////////////////////////////////////////////////////////////////////////////
#include "osgGDAL.hpp"
#include <osg/Geode>
#include <osg/Geometry>
#include <stdio.h>
#include <iostream>
#include "libcitygml/tesselator.hpp"
#include "core/application.hpp"
#include <vector>

#include "geos/geom/Polygon.h"
#include "geos/geom/Coordinate.h"
#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/Dimension.h"
#include "geos/geom/Envelope.h"
#include "geos/geom/LinearRing.h"
#include "geos/geom/Point.h"
#include "geos/geom/PrecisionModel.h"
#include "geos/io/WKTReader.h"
#include "geos/util/IllegalArgumentException.h"
#include "geos/operation/union/CascadedPolygonUnion.h"
#include "geos/simplify/DouglasPeuckerSimplifier.h"
#include "geos/simplify/TopologyPreservingSimplifier.h"

////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Geode> buildOsgGDAL(OGRDataSource* poDS)
{
    TVec3d offset_ = vcity::app().getSettings().getDataProfile().m_offset;
    osg::Vec3d offset(offset_.x, offset_.y, offset_.z);

    if(poDS)
    {
        printf("Load using Gdal / OGR\n");
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;

        OGRLayer *poLayer;
        int nbLayers = poDS->GetLayerCount();
        if(nbLayers > 0)
        {
            printf("nb layer %d\n", nbLayers);
            poLayer = poDS->GetLayer(0);
            printf("layer %s. Type : %d\n", poLayer->GetName(), poLayer->GetGeomType());
            //poLayer = poDS->GetLayerByName( "point" );

            Tesselator tess;

            OGRFeature *poFeature;
            poLayer->ResetReading();
            while( (poFeature = poLayer->GetNextFeature()) != NULL )
            {      
                OGRGeometry* poGeometry = poFeature->GetGeometryRef();
                //if(poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString25D)
                if(poGeometry != NULL && poGeometry->getGeometryType() == wkbLineString25D)
                {
                    osg::Geometry* geom = new osg::Geometry;
                    osg::Vec3Array* vertices = new osg::Vec3Array;
                    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP, 0);

                    OGRLineString* poLS = (OGRLineString*) poGeometry;
                    int nbPoints = poLS->getNumPoints();
                    for(int i=0; i<nbPoints; ++i)
                    {
                        OGRPoint p;
                        poLS->getPoint(i, &p);
                        //printf( "%f, %f; %f\n", p.getX(), p.getY(), p.getZ() );
                        osg::Vec3d pt = osg::Vec3d(p.getX(), p.getY(), p.getZ()) - offset;
                        vertices->push_back(pt);
                        indices->push_back(i);
                    }

                    geom->setVertexArray(vertices);
                    geom->addPrimitiveSet(indices);
                    geode->addDrawable(geom);
                }
                else if(poGeometry != NULL && (poGeometry->getGeometryType() == wkbPolygon25D || poGeometry->getGeometryType() == wkbPolygon))
                {
                    // get height test
                    /*if(poFeature->GetFieldCount() > 0)
                    {
                        std::cout << "ID : " << poFeature->GetFieldAsString("ID") << std::endl;
                        std::cout << "HAUTEUR : " << poFeature->GetFieldAsDouble("HAUTEUR") << std::endl;
                        std::cout << "Z_MIN : " << poFeature->GetFieldAsDouble("Z_MIN") << std::endl;
                        std::cout << "Z_MAX : " << poFeature->GetFieldAsDouble("Z_MAX") << std::endl;
                    }*/

                    /*
                    osg::Geometry* geom = new osg::Geometry;
                    osg::Vec3Array* vertices = new osg::Vec3Array;
                    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);

                    OGRPolygon* poPG = (OGRPolygon*) poGeometry;
                    OGRLinearRing* poLR = poPG->getExteriorRing();
                    int nbPoints = poLR->getNumPoints();
                    for(int i=0; i<nbPoints; ++i)
                    {
                        OGRPoint p;
                        poLR->getPoint(i, &p);
                        //printf( "%f, %f; %f\n", p.getX(), p.getY(), p.getZ() );
                        osg::Vec3d pt = osg::Vec3d(p.getX(), p.getY(), p.getZ()) - osg::Vec3d(643000.0, 6857000.0, 50.0);
                        vertices->push_back(pt);
                        indices->push_back(i);
                    }

                    geom->setVertexArray(vertices);
                    geom->addPrimitiveSet(indices);
                    geode->addDrawable(geom);
                    /*/
                    osg::Geometry* geom = new osg::Geometry;
                    osg::Vec3Array* vertices = new osg::Vec3Array;
                    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES , 0);

                    OGRPolygon* poPG = (OGRPolygon*) poGeometry;
                    OGRLinearRing* poLR = poPG->getExteriorRing();

                    // count vertices
                    unsigned int verticesCount = poLR->getNumPoints();
                    for(int i=0; i<poPG->getNumInteriorRings(); ++i)
                    {
                        verticesCount += poPG->getInteriorRing(i)->getNumPoints();
                    }
                    tess.init(verticesCount, TVec3d(0.0, 0.0, 1.0));

                    // feed data
                    OGRPoint p;
                    TVec3d v;
                    std::vector<TVec3d> pts;
                    std::vector<TVec2f> texcoords;
                    for(int i=0; i<poLR->getNumPoints(); ++i)
                    {
                        poLR->getPoint(i, &p);
                        v = TVec3d(p.getX(), p.getY(), p.getZ());
                        pts.push_back(v);
                    }
                    tess.addContour(pts, texcoords);

                    for(int j=0; j<poPG->getNumInteriorRings(); ++j)
                    {
                        pts.clear();
                        poLR = poPG->getInteriorRing(j);
                        for(int i=0; i<poLR->getNumPoints(); ++i)
                        {
                            poLR->getPoint(i, &p);
                            v = TVec3d(p.getX(), p.getY(), p.getZ());
                            pts.push_back(v);
                        }
                        tess.addContour(pts, texcoords);
                    }
                    tess.compute();

                    // feed data to osg
                    for(const TVec3d& v : tess.getVertices())
                    {
                        osg::Vec3d pt = osg::Vec3d(v.x, v.y, v.z) - offset;
                        vertices->push_back(pt);
                    }
                    for(unsigned int id : tess.getIndices())
                    {
                        indices->push_back(id);
                    }

                    geom->setVertexArray(vertices);
                    geom->addPrimitiveSet(indices);
                    geode->addDrawable(geom);
                    //*/
                }
                else
                {
                    printf( "%u %s no geometry\n", poGeometry->getGeometryType(), poGeometry->getGeometryName() );
                }
                //poFeature->get
                OGRFeature::DestroyFeature( poFeature );
                //poDS->get
            }
        }

        return geode;
    }

    return 0;
}

void buildGeosShape(OGRDataSource* poDS, geos::geom::Geometry ** ShapeGeo, std::vector<std::pair<double, double>> * Hauteurs, std::vector<BatimentShape> * InfoBatiments)
{
	if(!poDS)
    {
		std::cout << "Erreur chargement fichier .shp \n";
		return;
	}

	TVec3d offset_ = vcity::app().getSettings().getDataProfile().m_offset;

	const geos::geom::GeometryFactory * factory = geos::geom::GeometryFactory::getDefaultInstance();
	std::vector<geos::geom::Geometry*> Polys;
	geos::geom::LinearRing *shell;
	geos::geom::Polygon* P;
	
	OGRLayer *poLayer;
    int nbLayers = poDS->GetLayerCount();
    if(nbLayers > 0)
    {
        poLayer = poDS->GetLayer(0);
		OGRFeature *poFeature;
        poLayer->ResetReading();
        while( (poFeature = poLayer->GetNextFeature()) != NULL )
        {      
            OGRGeometry* poGeometry = poFeature->GetGeometryRef();
			if(poGeometry != NULL && (poGeometry->getGeometryType() == wkbPolygon25D || poGeometry->getGeometryType() == wkbPolygon))
            {
                OGRPolygon* poPG = (OGRPolygon*) poGeometry;
                OGRLinearRing* poLR = poPG->getExteriorRing();
                int nbPoints = poLR->getNumPoints();

				geos::geom::CoordinateArraySequence temp;
				/*std::vector<geos::geom::Geometry*> * Holes = new std::vector<geos::geom::Geometry*>;//

				for(int i = 0; i < poPG->getNumInteriorRings(); i++)// //Pour r�cup�rer les holes des polygons
				{
					OGRLinearRing* Ring = poPG->getInteriorRing(i);

					for(int j = 0; j < Ring->getNumPoints(); j++)
					{
						OGRPoint p;
						Ring->getPoint(j, &p);
						temp.add(geos::geom::Coordinate(p.getX(), p.getY()));
					}
					if(temp.size() > 3)
						Holes->push_back((geos::geom::Geometry*)factory->createLinearRing(temp));
					temp.clear();
				}*/

                for(int i=0; i<nbPoints; ++i)//Pour r�cup�rer les points de l'exterior ring
                {
                    OGRPoint p;
                    poLR->getPoint(i, &p);

					temp.add(geos::geom::Coordinate(p.getX() - offset_.x, p.getY() - offset_.y));
                }

				if(temp.size() > 3)
				{
					shell=factory->createLinearRing(temp);
					P = factory->createPolygon(shell, NULL/*Holes*/); //Les b�timents du cadastre sont r�cup�r�s sans les cours int�rieures. Mettre Holes � la place de NULL pour les avoir.
					if(P->isValid()/* && P->getArea() > 10*/)
					{
						if(poFeature->GetFieldIndex("ID") != -1)
							InfoBatiments->push_back(BatimentShape(P, poFeature->GetFieldAsString("ID")));
						else if(poFeature->GetFieldIndex("OBJECTID") != -1)
							InfoBatiments->push_back(BatimentShape(P, poFeature->GetFieldAsString("OBJECTID")));
						else
							InfoBatiments->push_back(BatimentShape(P, "ID NULL"));

						Polys.push_back(P);
						double H = 20;
						double Zmin = 0;
						if(poFeature->GetFieldIndex("HAUTEUR") != -1)
							H = poFeature->GetFieldAsDouble("HAUTEUR");
						if(poFeature->GetFieldIndex("Z_MIN") != -1)
							Zmin = poFeature->GetFieldAsDouble("Z_MIN");						

						if((H == 0 || Zmin > 1000) && Hauteurs->size()>1)
						{
							H = Hauteurs->at(Hauteurs->size() - 1).first;
							Zmin = Hauteurs->at(Hauteurs->size() - 1).second;
						}

						std::pair<double, double> PairTemp(H, Zmin);
						Hauteurs->push_back(PairTemp);
					}
				}
			}
			OGRFeature::DestroyFeature( poFeature );
		}
		geos::geom::MultiPolygon *MP = factory->createMultiPolygon(Polys);
			
		*ShapeGeo = MP;
	}
}
////////////////////////////////////////////////////////////////////////////////
