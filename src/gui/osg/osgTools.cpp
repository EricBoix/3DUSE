// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <osg/Geometry>
#include <osg/ValueObject>
#include "osgTools.hpp"

osg::ref_ptr<osg::Geode> osgTools::buildBBox( osg::Vec3 lowerBound,
                                              osg::Vec3 upperBound)
{
    osg::Vec3 step = upperBound - lowerBound;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    for (int x = 0; x <= 1; ++x)
    {
        for (int y = 0; y <= 1; ++y)
        {
            for (int z = 0; z <= 1; ++z)
            {
                vertices->push_back( osg::Vec3( lowerBound.x() + x*step.x(),
                                                lowerBound.y() + y*step.y(),
                                                lowerBound.z() + z*step.z() ));
            }
        }
    }

    indices->push_back(0); indices->push_back(1);
    indices->push_back(0); indices->push_back(2);
    indices->push_back(0); indices->push_back(4);
    indices->push_back(3); indices->push_back(1);
    indices->push_back(3); indices->push_back(2);
    indices->push_back(3); indices->push_back(7);
    indices->push_back(5); indices->push_back(1);
    indices->push_back(5); indices->push_back(4);
    indices->push_back(5); indices->push_back(7);
    indices->push_back(6); indices->push_back(2);
    indices->push_back(6); indices->push_back(4);
    indices->push_back(6); indices->push_back(7);

    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(indices);
    geode->addDrawable(geom);

    return geode;
}
////////////////////////////////////////////////////////////////////////////////
vcity::URI osgTools::getURI(osg::Node* node)
{
    std::string strType;

    osg::Node* parent = node;
    vcity::URI uri;// = node->getName();
    uri.append(node->getName());
    //while((parent = (osg::Node*)(parent->getParent(0))) != NULL)
    while (parent->getNumParents() > 0 && (parent = (osg::Node*)(parent->getParent(0))) != NULL)
    {
        if (parent->getName() == "layers")
            break;

        strType = "";
        bool bType = parent->getUserValue("type", strType);
        if (bType)
        {
            if (strType == "Workspace")
            {
                //std::cout << "---> WORKSPACE " << std::endl;
            }
            if (strType == "Version")
            {
                //std::cout << "---> VERSION " << std::endl;
            }
        }

        //URI.insert(0, parent->getName());
        uri.prepend(parent->getName(), strType);
    }

    return uri;
}
////////////////////////////////////////////////////////////////////////////////
