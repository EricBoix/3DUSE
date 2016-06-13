// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#include "osgInfo.hpp"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
osgInfo::osgInfo()
{
    m_texture = new osg::Texture2D;

    m_quad = new osg::Geometry;
    osg::Vec2Array* qTexCoords = new osg::Vec2Array;
    qTexCoords->push_back(osg::Vec2(0.0f,0.0f) );
    qTexCoords->push_back(osg::Vec2(1.0f,0.0f) );
    qTexCoords->push_back(osg::Vec2(1.0f,1.0f) );
    qTexCoords->push_back(osg::Vec2(0.0f,1.0f) );
    m_quad->setTexCoordArray(0,qTexCoords);

    m_state = new osg::StateSet;
    m_state->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    m_geode->addDrawable(m_quad);
    m_pat->addChild(m_geode);

    m_name = "NULL";
    m_filetype = "NULL";
    m_sourcetype = "NULL";
    m_LOD ="NULL";

    m_displayable = true;
    m_requested = true;

}

osgInfo::osgInfo(float height, float width, osg::Vec3 pos, double ang, osg::Vec3 axis, std::string filepath, std::string name, std::string type, std::string source, std::string lod, float anchor, int priority)
{
    //Fill members from constructor parameters
    m_texture = new osg::Texture2D;
    m_quad = new osg::Geometry;
    m_state = new osg::StateSet;
    m_geode = new osg::Geode;
    m_pat = new osg::PositionAttitudeTransform;

    m_initposition = pos;
    m_currentposition = pos;
    m_angle = ang;
    m_axe = axis;

    m_name = name;
    m_filetype = type;
    m_sourcetype = source;
    m_LOD = lod;
    m_filepath=filepath;

    m_texture->setImage(osgDB::readImageFile(filepath));
    m_height = height;
    m_width = width;

    m_anchoring = anchor ;

    m_priority = priority;

    /********** QUAD *********/
    // Create quad geometry
    // Vertices
    osg::Vec3Array* qVertices = new osg::Vec3Array;
    qVertices->push_back( osg::Vec3( -m_width/2, 0, -m_height/2) ); // bottom left
    qVertices->push_back( osg::Vec3(m_width/2, 0, -m_height/2) ); // bottom right
    qVertices->push_back( osg::Vec3(m_width/2,0, m_height/2) ); // top right
    qVertices->push_back( osg::Vec3(-m_width/2,0, m_height/2) ); // top left

    m_quad->setVertexArray( qVertices );

    // Geometry
    osg::DrawElementsUInt* Quad = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    Quad->push_back(3);
    Quad->push_back(2);
    Quad->push_back(1);
    Quad->push_back(0);
    m_quad->addPrimitiveSet(Quad);

    // Texture coordinates
    osg::Vec2Array* qTexCoords = new osg::Vec2Array;
    qTexCoords->push_back(osg::Vec2(0.0f,0.0f) );
    qTexCoords->push_back(osg::Vec2(1.0f,0.0f) );
    qTexCoords->push_back(osg::Vec2(1.0f,1.0f) );
    qTexCoords->push_back(osg::Vec2(0.0f,1.0f) );
    m_quad->setTexCoordArray(0,qTexCoords);

    m_state->setTextureAttributeAndModes(0, m_texture, osg::StateAttribute::ON );

    //Material
    m_material = new osg::Material;
    m_material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    m_material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    m_material->setAlpha(osg::Material::FRONT_AND_BACK, 1);


    m_state->setAttribute(m_material,osg::StateAttribute::ON);
    m_state->setMode(GL_BLEND, osg::StateAttribute::ON);

    osg::BlendFunc* blend = new osg::BlendFunc;
    blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_DST_ALPHA);

    /********** ANCHORING LINE *********/
    // Create anchoring line geometry
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::Geometry* geom = new osg::Geometry;
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    vertices->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()-m_height/2) );
    vertices->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_anchoring ));

    indices->push_back(0);
    indices->push_back(1);

    osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
    color->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(indices);
    geom->setColorArray(color, osg::Array::BIND_OVERALL);

    geode->addDrawable(geom);

    /******************* TETRAEDRE **************************/
    // Create local axis geometry lines (to visualize normale, rotation axis and croos product)
    m_tetra = new osg::Geode;

            //line1
    osg::Geometry* line1 = new osg::Geometry;
    osg::Vec3Array* verticesL1 = new osg::Vec3Array;
    osg::DrawElementsUInt* indicesL1 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    verticesL1->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()+m_height/2));
    verticesL1->push_back( osg::Vec3( m_initposition.x()+20, m_initposition.y(), m_initposition.z()+m_height/2));

    indicesL1->push_back(0);
    indicesL1->push_back(1);

    osg::ref_ptr<osg::Vec4Array> color1 = new osg::Vec4Array;
    color1->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    line1->setVertexArray(verticesL1);
    line1->addPrimitiveSet(indicesL1);
    line1->setColorArray(color1, osg::Array::BIND_OVERALL);

            //line 2
    osg::Geometry* line2 = new osg::Geometry;
    osg::Vec3Array* verticesL2 = new osg::Vec3Array;
    osg::DrawElementsUInt* indicesL2 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    verticesL2->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()+m_height/2));
    verticesL2->push_back( osg::Vec3( m_initposition.x(), m_initposition.y()+20, m_initposition.z()+m_height/2));

    indicesL2->push_back(0);
    indicesL2->push_back(1);

    osg::ref_ptr<osg::Vec4Array> color2 = new osg::Vec4Array;
    color2->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    line2->setVertexArray(verticesL2);
    line2->addPrimitiveSet(indicesL2);
    line2->setColorArray(color2, osg::Array::BIND_OVERALL);

            //line3
    osg::Geometry* line3 = new osg::Geometry;
    osg::Vec3Array* verticesL3 = new osg::Vec3Array;
    osg::DrawElementsUInt* indicesL3 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    verticesL3->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()+m_height/2 ));
    verticesL3->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()+m_height/2+20 ));

    indicesL3->push_back(0);
    indicesL3->push_back(1);

    osg::ref_ptr<osg::Vec4Array> color3 = new osg::Vec4Array;
    color3->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    line3->setVertexArray(verticesL3);
    line3->addPrimitiveSet(indicesL3);
    line3->setColorArray(color3, osg::Array::BIND_OVERALL);


    m_tetra->addDrawable(line1);
    m_tetra->addDrawable(line2);
    m_tetra->addDrawable(line3);



    /*** CREATE LOCAL SCENE GRAPH **/
    m_geode->addDrawable(m_quad);

    m_pat->setStateSet(m_state);
    m_pat->setPosition(m_initposition);
    m_pat->setAttitude(osg::Quat(osg::DegreesToRadians(m_angle), m_axe));

    m_billboard = new osg::Billboard();

    m_billboard->addDrawable(m_quad, osg::Vec3(0,0,0));
    m_billboard->setAxis(osg::Vec3(0,0,1));
    //m_billboard->setMode(osg::Billboard::POINT_ROT_WORLD);


    m_group = new osg::Group;

    m_group->addChild(m_pat);
    m_group->addChild(geode);
    //m_group->addChild(m_tetra);

    m_pat->addChild(m_geode);

    /*** INITIALISE METRICS ***/
    m_displayable = true;
    m_requested = true;
    m_onscreen = false;

    m_DCAM = 0.0f;
    m_DSC = 0.0f;
    m_Da= 0.0f;
    m_OVa = 0.0f;


}

void osgInfo::setBillboarding(bool option)
{
    if(option)
    {
        m_pat->removeChild(m_geode);
        m_pat->addChild(m_billboard);
    }
    else
    {
        m_pat->removeChild(m_billboard);
        m_pat->addChild(m_geode);
    }
}

void osgInfo::UpdateTetra(osg::Vec3f normale, osg::Vec3f axis, osg::Vec3f ortho)
{

    //// TETRA LINE 1

    osg::Geometry* newline1 = new osg::Geometry;
    osg::Vec3Array* verticesL1 = new osg::Vec3Array;
    osg::DrawElementsUInt* indicesL1 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    verticesL1->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()+m_height/2));
    verticesL1->push_back(osg::Vec3(m_initposition.x()+ortho.x()*20, m_initposition.y()+ortho.y()*20, m_initposition.z()+m_height/2+ortho.z()*20));

    indicesL1->push_back(0);
    indicesL1->push_back(1);

    osg::ref_ptr<osg::Vec4Array> colorortho = new osg::Vec4Array;
    colorortho->push_back(osg::Vec4(1.0,0.0,0.0,1.0));

    newline1->setVertexArray(verticesL1);
    newline1->addPrimitiveSet(indicesL1);
    newline1->setColorArray(colorortho, osg::Array::BIND_OVERALL);

    //// TETRA LINE 2
    osg::Geometry* newline2 = new osg::Geometry;
    osg::Vec3Array* verticesL2 = new osg::Vec3Array;
    osg::DrawElementsUInt* indicesL2 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);

    verticesL2->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()+m_height/2));
    verticesL2->push_back(osg::Vec3(m_initposition.x()+normale.x()*20, m_initposition.y()+normale.y()*20, m_initposition.z()+m_height/2+normale.z()*20));

    indicesL2->push_back(0);
    indicesL2->push_back(1);

    osg::ref_ptr<osg::Vec4Array> colornormale = new osg::Vec4Array;
    colornormale->push_back(osg::Vec4(0.0,1.0,0.0,1.0));

    newline2->setVertexArray(verticesL2);
    newline2->addPrimitiveSet(indicesL2);
    newline2->setColorArray(colornormale, osg::Array::BIND_OVERALL);

    //// TETRA LINE 3
    osg::Geometry* newline3 = new osg::Geometry;
    osg::Vec3Array* verticesL3 = new osg::Vec3Array;
    osg::DrawElementsUInt* indicesL3 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    verticesL3->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()+m_height/2));
    verticesL3->push_back( osg::Vec3(m_initposition.x()+axis.x()*20, m_initposition.y()+axis.y()*20, m_initposition.z()+m_height/2+axis.z()*20));

    indicesL3->push_back(0);
    indicesL3->push_back(1);

    osg::ref_ptr<osg::Vec4Array> coloraxis = new osg::Vec4Array;
    coloraxis->push_back(osg::Vec4(0.0,0.0,1.0,1.0));

    newline3->setVertexArray(verticesL3);
    newline3->addPrimitiveSet(indicesL3);
    newline3->setColorArray(coloraxis, osg::Array::BIND_OVERALL);

    m_tetra->removeDrawables(0,3);
    m_tetra->addDrawable(newline1);
    m_tetra->addDrawable(newline2);
    m_tetra->addDrawable(newline3);


}

void osgInfo::UpdateAnchoringLine(float newZ)
{
    m_group->removeChild(1);
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::Geometry* geom = new osg::Geometry;
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    vertices->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), newZ-m_height/2) );
    vertices->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_anchoring ));

    indices->push_back(0);
    indices->push_back(1);

    osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
    color->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(indices);
    geom->setColorArray(color, osg::Array::BIND_OVERALL);

    geode->addDrawable(geom);
    m_group->addChild(geode);
}

void osgInfo::UpdatePosition(osg::Vec3 newPos)
{
    m_currentposition=newPos;
    UpdateAnchoringLine(newPos.z());
}

osg::Vec3 osgInfo::getPosition()
{
    return m_currentposition;
}

float osgInfo::getDCAM()
{
    return m_DCAM;
}

float osgInfo::getDSC()
{
    return m_DSC;
}

osg::Group* osgInfo::getGroup()
{
    return m_group;
    //return m_pat;
}

std::string osgInfo::getInfoName()
{
    return m_name;
}

std::string osgInfo::getType()
{
    return m_filetype;
}

std::string osgInfo::getSourceType()
{
    return m_sourcetype;
}

std::string osgInfo::getInfoLOD()
{
    return m_LOD;
}

bool osgInfo::isDisplayable()
{
    return m_displayable;
}

bool osgInfo::isRequested()
{
    return m_requested;
}

bool osgInfo::isonScreen()
{
    return m_onscreen;
}

void osgInfo::setAxis(osg::Vec3 newAxis)
{
    m_axe=newAxis;
    //TODO : void update pat

}

void osgInfo::setAngle(float newAngle)
{
    m_angle=newAngle;
    //TODO : void update pat

}

void osgInfo::setHeight(float newHeight)
{
    m_height=newHeight;
    //TODO : void update geom

}

void osgInfo::setWidth(float newWidth)
{
    m_width=newWidth;
    //TODO : void update geom

}

void osgInfo::setDCAM(float newDist)
{
    m_DCAM=newDist;

}

void osgInfo::setDSC(float newScreenDist)
{
    m_DSC=newScreenDist;

}

void osgInfo::setAnchoringPoint(float altitude)
{
    m_anchoring=altitude;
    m_group->removeChild(1);

    if(m_LOD=="street")
        m_initposition.z()=m_anchoring+streetZ;
    if(m_LOD=="building")
        m_initposition.z()=m_anchoring+buildingZ;
    if(m_LOD=="district")
        m_initposition.z()=m_anchoring+districtZ;
    if(m_LOD=="city")
        m_initposition.z()=m_anchoring+cityZ;

    m_pat->setPosition(m_initposition);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::Geometry* geom = new osg::Geometry;
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    vertices->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_initposition.z()-m_height/2) );
    vertices->push_back( osg::Vec3( m_initposition.x(), m_initposition.y(), m_anchoring ));

    indices->push_back(0);
    indices->push_back(1);

    osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
    color->push_back(osg::Vec4(1.0,1.0,1.0,1.0));

    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(indices);
    geom->setColorArray(color, osg::Array::BIND_OVERALL);

    geode->addDrawable(geom);
    m_group->addChild(geode);

}

void osgInfo::setTexture(std::string filepath)
{
    m_texture->setImage(osgDB::readImageFile(filepath));

    m_state->setTextureAttributeAndModes(0, m_texture, osg::StateAttribute::ON );

}

void osgInfo::setDisplayable(bool statut)
{
    m_displayable=statut;
}

void osgInfo::setRequested(bool statut)
{
    m_requested=statut;
}

void osgInfo::setonScreen(bool statut)
{
    m_onscreen=statut;
}

void osgInfo::setDa(float area)
{
    m_Da=area;
}

void osgInfo::setTransparency(float alpha)
{
   m_material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

void osgInfo::updateScale( int screenX, int screenY)
{
    float scale;
    float maxscale = std::sqrt(screenX*screenX+screenY*screenY);
    if(m_DSC)
        scale = m_DSC/maxscale;
   scale = 2*(1-scale);
   m_pat->setScale(osg::Vec3(scale, 1, scale));
}

void osgInfo::updateDisplayability()
{
    if(m_LOD=="street")
    {
        if(m_DCAM<=500)
        {
            m_displayable = true;
            setTransparency(1.0f);
        }
        else if (m_DCAM>500 && m_DCAM<1500)
        {
            m_displayable = true;
            setTransparency(0.2f);
        }
        else
        {
            m_displayable = false;
        }
    }

    if(m_LOD=="building")
    {
        if(m_DCAM<=1500 && m_DCAM>=500)
        {
            m_displayable = true;
            setTransparency(1.0f);
        }
        else if(m_DCAM>1500 &&  m_DCAM<6000)
        {
            m_displayable = true;
            setTransparency(0.2f);
        }
        else
        {
            m_displayable = false;
            setTransparency(0.2f);
        }
    }

    if(m_LOD=="district")
    {
        if(m_DCAM<=6000 && m_DCAM>=1500)
        {
            m_displayable = true;
            setTransparency(1.0f);
        }
        else if(m_DCAM>6000 && m_DCAM<30000)
        {
            m_displayable = true;
            setTransparency(0.2f);
        }
        else
        {
            m_displayable = false;
            setTransparency(0.2f);
        }

    }
    if(m_LOD=="city")
    {
        if(m_DCAM<=30000 && m_DCAM>=6000)
        {
            m_displayable = true;
            setTransparency(1.0f);
        }
        else if(m_DCAM>30000)
        {
            m_displayable = true;
            setTransparency(0.2f);
        }
        else
        {
            m_displayable = false;
            setTransparency(0.2f);
        }

    }
}

void osgInfo::computeDSC(osg::Camera *cam, int screenX, int screenY)
{
    osg::Vec3d pos;
    osg::Vec3d target;
    osg::Vec3d up;
    cam->getViewMatrixAsLookAt(pos,target,up);

    osg::Matrix Mview = cam->getViewMatrix();
    osg::Matrix Mwin = cam->getViewport()->computeWindowMatrix();
    osg::Matrix Mproj = cam->getProjectionMatrix();

    osg::Vec3 worldcoord = m_currentposition ;
    osg::Vec3 screencoord = worldcoord*Mview*Mproj*Mwin;


    m_DSC = sqrt((screencoord.x()-screenX/2)*(screencoord.x()-screenX/2)+(screencoord.y()-screenY/2)*(screencoord.y()-screenY/2));

    TVec3d dirLookAt(target.x() - pos.x(),target.y() - pos.y(),target.z() - pos.z());
    TVec3d dirDoc(m_currentposition.x() - pos.x(), m_currentposition.y() - pos.y(), m_currentposition.z() - pos.z());

    float scalaire = dirLookAt.dot(dirDoc);
    float angle = acos(scalaire/(dirLookAt.length()*dirDoc.length()));
    angle=angle*180/3.1415;

    osg::Vec3f normale(-dirDoc.x, -dirDoc.y, -dirDoc.z);
    osg::Vec3f axis = m_billboard->getAxis();
    osg::Vec3f ortho = axis.operator ^(normale);
    osg::Vec3f wCornerMax = m_currentposition + (ortho/ortho.length())*(m_width/2.0f) + (axis/axis.length())*(m_height/2.0f);
    osg::Vec3f wCornerMin = m_currentposition - (ortho/ortho.length())*(m_width/2.0f) - (axis/axis.length())*(m_height/2.0f);

    osg::Vec3 sCornerMax = wCornerMax*Mview*Mproj*Mwin;
    osg::Vec3 sCornerMin = wCornerMin*Mview*Mproj*Mwin;

    /********** RED POINTS TO WITNESS CORNERS ************/
    //                osg::Geode* cornersGeode = new osg::Geode;
    //                osg::Geometry* cornersGeom = new osg::Geometry;
    //                osg::Vec3Array* verticesPoints = new osg::Vec3Array;

    //                verticesPoints->push_back(wCornerMax);
    //                verticesPoints->push_back(wCornerMin);

    //                osg::ref_ptr<osg::Vec4Array> colorpoints = new osg::Vec4Array;
    //                colorpoints->push_back(osg::Vec4(1.0,0.0,0.0,1.0));

    //                cornersGeom->setVertexArray(verticesPoints);
    //                cornersGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,verticesPoints->size()));
    //                cornersGeom->getOrCreateStateSet()->setAttribute(new osg::Point(5.0f), osg::StateAttribute::ON);

    //                cornersGeom->setColorArray(colorpoints, osg::Array::BIND_OVERALL);
    //                cornersGeode->addDrawable(cornersGeom);

    //                if(info->getGroup()->getNumChildren()>3)
    //                {
    //                    info->getGroup()->removeChild(info->getGroup()->getNumChildren()-1);
    //                    info->getGroup()->addChild(cornersGeode);
    //                }
    //                else
    //                    info->getGroup()->addChild(cornersGeode);

    /* Determine if document on screen or not according to the borders of the screen */
    if(sCornerMax.x()>0.0 && sCornerMin.x()<screenX && sCornerMax.y()>0.0 && sCornerMin.y()<screenY && angle < 90.0)
        m_onscreen = true;
    else if(sCornerMax.x()<0.0 && sCornerMin.x()>screenX && sCornerMax.y()<0.0 && sCornerMin.y()>screenY && angle < 90.0)
        m_onscreen = true;
    else
        m_onscreen = false;


    /* Crop document coordinates to tell the exact surface inside the screen */
    if(sCornerMax.x()>screenX)
        sCornerMax.x()=screenX;
    if(sCornerMax.x()<0.0)
        sCornerMax.x()=0.0;
    if(sCornerMax.y()>screenY)
        sCornerMax.y()=screenY;
    if(sCornerMax.y()<0.0)
        sCornerMax.y()=0.0;
    if(sCornerMin.x()<0.0)
        sCornerMin.x()=0.0;
    if(sCornerMin.x()>screenX)
        sCornerMin.x()=screenX;
    if(sCornerMin.y()<0.0)
        sCornerMin.y()=0.0;
    if(sCornerMin.y()>screenY)
        sCornerMin.y()=screenY;

    float screenwidth = sCornerMax.x()-sCornerMin.x();
    float screenheight = sCornerMax.y()-sCornerMin.y();
    m_Da = abs(screenwidth*screenheight);

    m_sCornerMax=sCornerMax;
    m_sCornerMin=sCornerMin;

    //UpdateTetra(normale/normale.length(), axis/axis.length(), ortho/ortho.length());

}

void osgInfo::computeDCAM(osg::Camera *cam)
{
    osg::Vec3d pos;
    osg::Vec3d target;
    osg::Vec3d up;
    cam->getViewMatrixAsLookAt(pos,target,up);

    float infoX = m_currentposition.x();
    float infoY = m_currentposition.y();
    float infoZ = m_currentposition.z();
    float camX=pos.x();
    float camY=pos.y();
    float camZ=pos.z();


    m_DCAM = sqrt((infoX-camX)*(infoX-camX)+(infoY-camY)*(infoY-camY)+(infoZ-camZ)*(infoZ-camZ));
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
