// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#include "osgScene.hpp"
#include <osg/PositionAttitudeTransform>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
//#include <osgShadow/ViewDependentShadowMap>
#include <osgShadow/SoftShadowMap>
#include <osg/Switch>
//#include <osgShadow/ParallelSplitShadowMap>
#include <osg/ValueObject>
#include <osgText/Text>
#include <osgUtil/Optimizer>

#include <osg/Depth>
#include <osg/CullFace>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osg/VertexProgram>
#include <osg/ShapeDrawable>

#include "gui/applicationGui.hpp"
#include "gui/moc/mainWindow.hpp"
#include "osgCityGML.hpp"
#include "core/dataprofile.hpp"
#include "osgTools.hpp"
#include "../controllerGui.hpp"

#include "osgInfo.hpp"
#include <osg/Point>
#include <osg/MatrixTransform>
#include <osg/NodeCallback>
#include <cmath>
#include <map>

#include <string>

//#include <typeinfo>
////////////////////////////////////////////////////////////////////////////////
/** Provide an simple example of customizing the default UserDataContainer.*/
class MyUserDataContainer : public osg::DefaultUserDataContainer
{
public:
    MyUserDataContainer() {}
    MyUserDataContainer(const MyUserDataContainer& udc, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) :
        DefaultUserDataContainer(udc, copyop) {}

    META_Object(MyNamespace, MyUserDataContainer)

        virtual Object* getUserObject(unsigned int i)
    {
        OSG_NOTICE << "MyUserDataContainer::getUserObject(" << i << ")" << std::endl;
        return  osg::DefaultUserDataContainer::getUserObject(i);
    }

    virtual const Object* getUserObject(unsigned int i) const
    {
        OSG_NOTICE << "MyUserDataContainer::getUserObject(" << i << ") const" << std::endl;
        return osg::DefaultUserDataContainer::getUserObject(i);
    }

protected:
    virtual ~MyUserDataContainer() {}
};
////////////////////////////////////////////////////////////////////////////////


class FindNamedNode : public osg::NodeVisitor
{
public:
    FindNamedNode(const std::string& name)
        : osg::NodeVisitor( // Traverse all children.
            osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _name(name)
    {
        setTraversalMask(0xffffffff);
        setNodeMaskOverride(0xffffffff);
    }

    // This method gets called for every node in the scene
    //   graph. Check each node to see if its name matches
    //   out target. If so, save the node's address.
    virtual void apply(osg::Node& node)
    {
        if (node.getName() == _name)
            _node = &node;

        // Keep traversing the rest of the scene graph.
        traverse(node);
    }

    osg::ref_ptr<osg::Node> getNode() { return _node; }

protected:
    std::string _name;
    osg::ref_ptr<osg::Node> _node;
};


/////////////////////////////////////////////////////////////////////////////////////
//** Skybox related classes
osg::TextureCubeMap* OsgScene::readCubeMap()
{
    osg::TextureCubeMap* cubemap = new osg::TextureCubeMap;

    osg::ref_ptr<osg::Image>imagePosX = osgDB::readRefImageFile("/home/vincent/Documents/VCity_Project/Data/skybox/SkyboxSet1/TropicalSunnyDay/right.png");
    osg::ref_ptr<osg::Image>imageNegX = osgDB::readRefImageFile("/home/vincent/Documents/VCity_Project/Data/skybox/SkyboxSet1/TropicalSunnyDay/left.png");
    osg::ref_ptr<osg::Image>imagePosY = osgDB::readRefImageFile("/home/vincent/Documents/VCity_Project/Data/skybox/SkyboxSet1/TropicalSunnyDay/bottom.png");
    osg::ref_ptr<osg::Image>imageNegY = osgDB::readRefImageFile("/home/vincent/Documents/VCity_Project/Data/skybox/SkyboxSet1/TropicalSunnyDay/top.png");
    osg::ref_ptr<osg::Image>imagePosZ = osgDB::readRefImageFile("/home/vincent/Documents/VCity_Project/Data/skybox/SkyboxSet1/TropicalSunnyDay/back.png");
    osg::ref_ptr<osg::Image>imageNegZ = osgDB::readRefImageFile("/home/vincent/Documents/VCity_Project/Data/skybox/SkyboxSet1/TropicalSunnyDay/front.png");


    if (imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ)
    {
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_X, imagePosX);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X, imageNegX);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y, imagePosY);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y, imageNegY);
        cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z, imagePosZ);
        cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z, imageNegZ);

        cubemap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        cubemap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

        cubemap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
        cubemap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    }

    return cubemap;
}

// Update texture matrix for cubemaps
struct TexMatCallback : public osg::NodeCallback
{
public:

    TexMatCallback(osg::TexMat& tm) :
        _texMat(tm)
    {
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            const osg::Matrix& MV = *(cv->getModelViewMatrix());
            const osg::Matrix R = osg::Matrix::rotate(osg::DegreesToRadians(112.0f), 0.0f, 0.0f, 1.0f)*
                osg::Matrix::rotate(osg::DegreesToRadians(90.0f), 1.0f, 0.0f, 0.0f);

            osg::Quat q = MV.getRotate();
            const osg::Matrix C = osg::Matrix::rotate(q.inverse());

            _texMat.setMatrix(C*R);
        }

        traverse(node, nv);
    }

    osg::TexMat& _texMat;
};


class MoveEarthySkyWithEyePointTransform : public osg::Transform
{
public:
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.preMultTranslate(eyePointLocal);
        }
        return true;
    }

    /** Get the transformation matrix which moves from world coords to local coords.*/
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.postMultTranslate(-eyePointLocal);
        }
        return true;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

OsgScene::OsgScene()
    : osg::Group(), m_shadow(false), m_shadowVec(-1, -1, 0.1, 0)
{
    init();
}

class InfoDataType : public osg::Referenced
{
public:

    InfoDataType(osg::Node*n);
    osg::ref_ptr<osg::Switch> getSwitchRoot();
    void computeDCAM(osg::Camera *cam, osgInfo *info);
    void computeDSC(osg::Camera *cam, int screenX, int screenY, osgInfo *info);
    void computeOVa(int screenX, int screenY, std::map<float, osgInfo *> m_info);
    void updateScale(osgInfo *info, int screenX, int screenY);
    void setDisplayability(osgInfo *info);
    void stairedDisplay(std::map<float, osgInfo*> m_info);
    void display();

protected:
    osg::ref_ptr<osg::Switch> switchRoot;

};

InfoDataType::InfoDataType(osg::Node* n)
{
    switchRoot = n->asSwitch();
}

osg::ref_ptr<osg::Switch> InfoDataType::getSwitchRoot()
{
    return switchRoot;
}

void InfoDataType::computeDCAM(osg::Camera *cam, osgInfo* info)
{
    osg::Vec3d pos;
    osg::Vec3d target;
    osg::Vec3d up;
    cam->getViewMatrixAsLookAt(pos,target,up);
    float DCAM;

    float infoX = info->getPosition().x();
    float infoY = info->getPosition().y();
    float infoZ = info->getPosition().z();
    float camX=pos.x();
    float camY=pos.y();
    float camZ=pos.z();


    DCAM = sqrt((infoX-camX)*(infoX-camX)+(infoY-camY)*(infoY-camY)+(infoZ-camZ)*(infoZ-camZ));
    info->setDCAM(DCAM);
}

void InfoDataType::computeDSC(osg::Camera *cam, int screenX, int screenY, osgInfo* info)
{

    osg::Vec3d pos;
    osg::Vec3d target;
    osg::Vec3d up;
    cam->getViewMatrixAsLookAt(pos,target,up);
//    std::cout<<"Cam pos : ["<<pos.x()<<","<<pos.y()<<","<<pos.z()<<"]"<<std::endl;
//    std::cout<<"Cam target : ["<<target.x()<<","<<target.y()<<","<<target.z()<<"]"<<std::endl;
//    std::cout<<"Cam up : ["<<up.x()<<","<<up.y()<<","<<up.z()<<"]"<<std::endl;

    osg::Matrix Mview = cam->getViewMatrix();
    osg::Matrix Mwin = cam->getViewport()->computeWindowMatrix();
    osg::Matrix Mproj = cam->getProjectionMatrix();

    osg::Vec3 worldcoord = info->getPosition();
    osg::Vec3 screencoord = worldcoord*Mview*Mproj*Mwin;

    int Ds = sqrt((screencoord.x()-screenX/2)*(screencoord.x()-screenX/2)+(screencoord.y()-screenY/2)*(screencoord.y()-screenY/2));
    info->setDSC(Ds);

    TVec3d dirLookAt(target.x() - pos.x(),target.y() - pos.y(),target.z() - pos.z());
    TVec3d dirDoc(info->m_currentposition.x() - pos.x(), info->m_currentposition.y() - pos.y(), info->m_currentposition.z() - pos.z());

    float scalaire = dirLookAt.dot(dirDoc);
    float angle = acos(scalaire/(dirLookAt.length()*dirDoc.length()));
    angle=angle*180/3.1415;

    osg::Vec3f normale(-dirDoc.x, -dirDoc.y, -dirDoc.z);
    osg::Vec3f axis = info->m_billboard->getAxis();
    osg::Vec3f ortho = axis.operator ^(normale);
    osg::Vec3f wCornerMax = info->m_currentposition + (ortho/ortho.length())*(info->m_width/2.0f) + (axis/axis.length())*(info->m_height/2.0f);
    osg::Vec3f wCornerMin = info->m_currentposition - (ortho/ortho.length())*(info->m_width/2.0f) - (axis/axis.length())*(info->m_height/2.0f);

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
        info->setonScreen(true);
    else if(sCornerMax.x()<0.0 && sCornerMin.x()>screenX && sCornerMax.y()<0.0 && sCornerMin.y()>screenY && angle < 90.0)
        info->setonScreen(true);
    else
        info->setonScreen(false);


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
    float Da = abs(screenwidth*screenheight);

    info->m_sCornerMax=sCornerMax;
    info->m_sCornerMin=sCornerMin;

    info->setDa(Da);


    //info->UpdateTetra(normale/normale.length(), axis/axis.length(), ortho/ortho.length());


}

void InfoDataType::computeOVa(int screenX, int screenY, std::map<float, osgInfo*> m_info)
{
   std::vector< std::vector<float> > screen;
   bool found;

    for(int i = 0; i<screenX; i++)
    {
        std::vector<float> col;
        for(int j=0; j<screenY; j++)
        {
            found=false;
            for (std::map<float,osgInfo*>::iterator it=m_info.begin(); it!=m_info.end(); ++it)
            {
                if(i>=(int)it->second->m_sCornerMin.x() && i<=(int)it->second->m_sCornerMax.x() && j>=(int)it->second->m_sCornerMin.y() && j<(int)it->second->m_sCornerMax.y())
                {
                    col.push_back(it->first);
                    found = true;
                    break;
                }
            }
            if(!found)
                col.push_back(0.0f);

        }
        screen.push_back(col);
    }


    for (std::map<float,osgInfo*>::iterator it=m_info.begin(); it!=m_info.end(); ++it)
    {
        it->second->m_OVa=0;
        for(int i = (int)it->second->m_sCornerMin.x(); i<(int)it->second->m_sCornerMax.x(); i++)
        {
            for(int j=(int)it->second->m_sCornerMin.y(); j<(int)it->second->m_sCornerMax.y(); j++)
            {
                if(screen[i][j]!=it->first && screen[i][j]!=0)
                {
                    it->second->m_OVa+=1;
                }
            }
        }
        if(it->second->m_OVa>it->second->m_Da)
            it->second->m_OVa=it->second->m_Da;

    }

    screen.clear();
}

void InfoDataType::setDisplayability(osgInfo* info)
{
    if(info->getInfoLOD()=="street")
    {
        if(info->m_DCAM<=500)
        {
            info->setDisplayable(true);
            info->setTransparency(1.0f);
        }
        else if (info->m_DCAM>500 && info->m_DCAM<1500)
        {
            info->setDisplayable(true);
            info->setTransparency(0.2f);
        }
        else
        {
            info->setDisplayable(false);
        }
    }

    if(info->getInfoLOD()=="building")
    {
        if(info->m_DCAM<=1500 && info->m_DCAM>=500)
        {
            info->setDisplayable(true);
            info->setTransparency(1.0f);
        }
        else if(info->m_DCAM>1500 &&  info->m_DCAM<6000)
        {
            info->setDisplayable(true);
            info->setTransparency(0.2f);
        }
        else
        {
            info->setDisplayable(false);
            info->setTransparency(0.2f);
        }
    }

    if(info->getInfoLOD()=="district")
    {
        if(info->m_DCAM<=6000 && info->m_DCAM>=1500)
        {
            info->setDisplayable(true);
            info->setTransparency(1.0f);
        }
        else if(info->m_DCAM>6000 && info->m_DCAM<30000)
        {
            info->setDisplayable(true);
            info->setTransparency(0.2f);
        }
        else
        {
            info->setDisplayable(false);
            info->setTransparency(0.2f);
        }

    }
    if(info->getInfoLOD()=="city")
    {
        if(info->m_DCAM<=30000 && info->m_DCAM>=6000)
        {
            info->setDisplayable(true);
            info->setTransparency(1.0f);
        }
        else if(info->m_DCAM>30000)
        {
            info->setDisplayable(true);
            info->setTransparency(0.2f);
        }
        else
        {
            info->setDisplayable(false);
            info->setTransparency(0.2f);
        }

    }

}

void InfoDataType::updateScale(osgInfo* info, int screenX, int screenY)
{
    //According to DCAM
    //float scale = info->m_DCAM/50450.1;

    //According to DSC
    float scale;
    float maxscale = std::sqrt(screenX*screenX+screenY*screenY);
    if(info->m_DSC)
        scale = info->m_DSC/maxscale;
    info->Scaling(scale);

}

void InfoDataType::stairedDisplay(std::map<float, osgInfo *> m_info)
{
    std::vector<osgInfo*> tmp_info;
    for (std::map<float,osgInfo*>::iterator it=m_info.begin(); it!=m_info.end(); ++it)
    {
        tmp_info.push_back(it->second);
    }
    for(int i=0; i<tmp_info.size(); i++)
    {
       osgInfo* info = tmp_info[0];
       info->getGroup()->getChild(0)->asTransform()->asPositionAttitudeTransform()->setPosition(info->m_initposition);
       info->UpdatePosition(info->m_initposition);
//       if(i==0)
//           std::cout<<"[stairsDisplay].... image("<<i<<") : "<<info->m_name<<" z value = "<<info->getGroup()->getChild(0)->asTransform()->asPositionAttitudeTransform()->getPosition().z()<<std::endl;
       if(i>0)
       {
           osgInfo* c_info = tmp_info[i];
           osgInfo* p_info = tmp_info[i-1];
           osg::Vec3 newPos = osg::Vec3(c_info->m_currentposition.x(), c_info->m_currentposition.y(),p_info->m_currentposition.z()+p_info->m_height/2+c_info->m_height/2);
           c_info->getGroup()->getChild(0)->asTransform()->asPositionAttitudeTransform()->setPosition(newPos);
           c_info->UpdatePosition(newPos);
           c_info->UpdateAnchoringLine(newPos.z());
//           std::cout<<"[stairsDisplay].... image("<<i<<") : "<<c_info->m_name<<" z value = "<<c_info->getGroup()->getChild(0)->asTransform()->asPositionAttitudeTransform()->getPosition().z()<<std::endl;
       }
    }
//    std::cout<<std::endl;
}

void InfoDataType::display()
{
    for(int i=0; i<switchRoot->getNumChildren(); i++)
    {
        osg::ref_ptr<osg::Switch> subSwitch = switchRoot->getChild(i)->asSwitch();
        if(subSwitch)
        {
            for(int j=0; j<subSwitch->getNumChildren();j++)
            {
                osg::Node* node = subSwitch->getChild(j);
                osgInfo* info = dynamic_cast<osgInfo*>(node);
                if(info->isDisplayable() && info->isRequested())
                {
                    subSwitch->setValue(j,true);
                }
                else
                {
                    subSwitch->setValue(j,false);
                }
            }
        }

    }
}

class UpdateInfo : public osg::NodeCallback
{
public :
    UpdateInfo()
    {
        std::cout<<"[osgScene > UpdateInfo].....created"<<std::endl;
    }
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {

        osg::ref_ptr<InfoDataType> layerInfo = dynamic_cast<InfoDataType*> (node->getUserData() );

        if(layerInfo)
        {

            osg::Camera* cam = appGui().getMainWindow()->m_osgView->m_osgView->getCamera();

            int screenX = appGui().getMainWindow()->m_osgView->m_widget->width();
            int screenY = appGui().getMainWindow()->m_osgView->m_widget->height();
            float Sa = screenX*screenY; //total screen area

            int ND = 0; //number of document
            int NDs = 0; //number of document on screen
            int NDh = 0; //number of docment hidden (>50% overlapped)
            float TDa = 0.0f; //total document area on screen
            float TOVa = 0.0f; //total document overlapped area


            std::map<float,osgInfo*> map_info ;
            std::map<float,osgInfo*> map_street ;
            std::map<float,osgInfo*> map_building ;
            std::map<float,osgInfo*> map_district ;
            std::map<float,osgInfo*> map_city ;


            osg::ref_ptr<osg::Switch> switchRoot=layerInfo->getSwitchRoot();

            for(int i=0; i<switchRoot->getNumChildren(); i++)
            {
                osg::ref_ptr<osg::Switch> subSwitch = switchRoot->getChild(i)->asSwitch();
                if(subSwitch)
                {
                    for(int j=0; j<subSwitch->getNumChildren();j++)
                    {
                        osg::Node* node = subSwitch->getChild(j);
                        osgInfo* info = dynamic_cast<osgInfo*>(node);

                        layerInfo->computeDCAM(cam, info);
                        layerInfo->computeDSC(cam, screenX, screenY, info);

                        //layerInfo->updateScale(info, screenX, screenY);
                        //layerInfo->setDisplayability(info);
                        info->setDisplayable(true);

                        if(info->isonScreen())
                        {
                            map_info[info->m_DCAM]=info;

                            if(info->getInfoLOD()=="street")
                                map_street[info->m_DCAM]=info;
                            if(info->getInfoLOD()=="building")
                                map_building[info->m_DCAM]=info;
                            if(info->getInfoLOD()=="district")
                                map_district[info->m_DCAM]=info;
                            if(info->getInfoLOD()=="city")
                                map_city[info->m_DCAM]=info;

                            if(info->m_OVa/info->m_Da>0.5)
                                NDh++;

                            TDa+=info->m_Da;
                            TOVa+=info->m_OVa;
                            NDs++;
                        }
                        ND++;
                    }
                }
            }

            layerInfo->computeOVa(screenX, screenY, map_info);

            osg::Vec3d pos;
            osg::Vec3d target;
            osg::Vec3d up;
            cam->getViewMatrixAsLookAt(pos,target,up);
            std::cout<<"Cam pos : ["<<pos.x()<<","<<pos.y()<<","<<pos.z()<<"]"<<std::endl;
            std::cout<<"Cam target : ["<<target.x()<<","<<target.y()<<","<<target.z()<<"]"<<std::endl;
            std::cout<<"Cam up : ["<<up.x()<<","<<up.y()<<","<<up.z()<<"]"<<std::endl;
            std::cout<<std::endl;


            float RDS = (TDa-TOVa)/Sa ; //ratio of all document area to screen area
            float RNDs = (float)NDs/ND; //ratio of all document displayed
            float RNDh = (float)NDh/NDs; //ratio of document hidden


            std::cout<<"RNDs = "<<RNDs*100<<"%"<<std::endl;
            std::cout<<"RNDh = "<<RNDh*100<<"%"<<std::endl;
            std::cout<<"RTDa = "<<TDa/Sa*100<<"%"<<std::endl;
            std::cout<<"ROVa = "<<TOVa/Sa*100<<"%"<<std::endl;
            std::cout<<"RDS = "<<RDS*100<<"%"<<std::endl;
            std::cout<<std::endl;

//            layerInfo->stairedDisplay(map_info);
//            layerInfo->stairedDisplay(map_street);
//            layerInfo->stairedDisplay(map_building);
//            layerInfo->stairedDisplay(map_district);
//            layerInfo->stairedDisplay(map_city);
            layerInfo->display();

        }
        traverse( node, nv );
    }


};

//////////////////////////////////////////////////////////////////////////////////////////////////////

//**** Skybox Creation ***//


osg::Node* OsgScene::createSkybox()
{
    osg::StateSet* stateset = new osg::StateSet();

    osg::TexEnv* te = new osg::TexEnv;
    te->setMode(osg::TexEnv::REPLACE);

    stateset->setTextureAttributeAndModes(0, te, osg::StateAttribute::ON);

    osg::TexGen *tg = new osg::TexGen;
    tg->setMode(osg::TexGen::NORMAL_MAP);

    stateset->setTextureAttributeAndModes(0, tg, osg::StateAttribute::ON);

    osg::TexMat *tm = new osg::TexMat;

    stateset->setTextureAttribute(0, tm);

    osg::TextureCubeMap* skymap = readCubeMap();

    stateset->setTextureAttributeAndModes(0, skymap, osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // clear the depth to the far plane.
    osg::Depth* depth = new osg::Depth;

    depth->setFunction(osg::Depth::ALWAYS);
    depth->setRange(1.0, 1.0);

    stateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
    stateset->setRenderBinDetails(-1, "RenderBin");

    osg::Drawable* drawable = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 100.f));
    osg::Geode* geode = new osg::Geode;

    geode->setCullingActive(false);
    geode->setStateSet(stateset);
    geode->addDrawable(drawable);

    osg::Transform* transform = new MoveEarthySkyWithEyePointTransform;

    transform->setCullingActive(false);
    transform->addChild(geode);

    osg::ClearNode* clearNode = new osg::ClearNode;

    //  clearNode->setRequiresClear(false);
    clearNode->setCullCallback(new TexMatCallback(*tm));
    clearNode->addChild(transform);

    return clearNode;

}

//**** End of Skybox Creation ***//

////////////////////////////////////////////////////////////////////////////////



void OsgScene::init()
{
    setName("root");

    osg::ref_ptr<osg::Group> node = NULL;

    // build effectNone node
    m_effectNone = new osg::Group();
    m_effectNone->setName("effect_none");

    // build effectShadow node
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
    shadowedScene->setName("effect_shadow");
    //osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();

#if 0
    //shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
    //shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
    osg::ref_ptr<osgShadow::ShadowMap> st = new osgShadow::ShadowMap;
    //osg::ref_ptr<osgShadow::ParallelSplitShadowMap> sm = new osgShadow::ParallelSplitShadowMap;
    shadowedScene->setShadowTechnique(st.get());
    int mapres = 2048;
    st->setTextureSize(osg::Vec2s(mapres, mapres));
    //sm->setTextureResolution(mapres);
#elif 0
    osg::ref_ptr<osgShadow::ViewDependentShadowMap> st = new osgShadow::ViewDependentShadowMap();
    int mapres = 1024;
    settings->setTextureSize(osg::Vec2s(mapres, mapres));
    settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
    settings->setNumShadowMapsPerLight(4);
    shadowedScene->setShadowTechnique(st.get());
#else
    osg::ref_ptr<osgShadow::SoftShadowMap> st = new osgShadow::SoftShadowMap();
    int mapres = 2048;
    st->setTextureSize(osg::Vec2s(mapres, mapres));
    shadowedScene->setShadowTechnique(st.get());
#endif

    // light, sun
    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource();
    lightSource->setName("lightsource");
    osg::ref_ptr<osg::Light> light = new osg::Light();
    light->setName("light");
    light->setAmbient(osg::Vec4(0.6, 0.6, 0.6, 1.0));
    lightSource->setLight(light);
    light->setPosition(m_shadowVec);
    shadowedScene->addChild(lightSource);
    m_effectNone->addChild(lightSource);
    m_effectShadow = shadowedScene;

    // add first depth node, to handle effect (shadow)
    if (m_shadow)
    {
        addChild(m_effectShadow);
    }
    else
    {
        addChild(m_effectNone);
    }

    //build skybox
    //addChild(createSkybox());

    // build layers node
    m_layers = new osg::Group();
    m_layers->setName("layers");

    m_effectShadow->addChild(m_layers);
    m_effectNone->addChild(m_layers);

    // build first default layer
    osg::ref_ptr<osg::Group> layer0 = new osg::Group();
    layer0->setName("layer_CityGML");
    m_layers->addChild(layer0);

    // build second default layer
    osg::ref_ptr<osg::Group> layer1 = new osg::Group();
    layer1->setName("layer_Assimp");
    m_layers->addChild(layer1);

    // build third default layer
    osg::ref_ptr<osg::Group> layer2 = new osg::Group();
    layer2->setName("layer_Mnt");
    m_layers->addChild(layer2);

    // build forth default layer
    osg::ref_ptr<osg::Group> layer3 = new osg::Group();
    layer3->setName("layer_Las");
    m_layers->addChild(layer3);

    // build fifth default layer
    osg::ref_ptr<osg::Group> layer4 = new osg::Group();
    layer4->setName("layer_Shp");
    m_layers->addChild(layer4);

	// build sixth default layer
    	osg::ref_ptr<osg::Group> layer5 = new osg::Group();
    	layer5->setName("layer_Info");
    	m_layers->addChild(layer5);

    updateGrid();
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::addTile(const vcity::URI& uriLayer, const vcity::Tile& tile)
{
    osg::ref_ptr<osg::Node> layer = getNode(uriLayer);
    if (layer)
    {
        osg::ref_ptr<osg::Group> layerGroup = layer->asGroup();
        if (layerGroup)
        {
            vcity::URI uri = uriLayer;
            uri.append(tile.getName(), "File");
            uri.setType("Building");
            uri.resetCursor();
            osg::ref_ptr<osg::Node> osgTile = buildTile(uri, tile);
            layerGroup->addChild(osgTile);
            buildTemporalNodes(uri, tile);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void OsgScene::initInfo(const vcity::URI& uriLayer, std::vector<osgInfo*> info)
{
    //uriLayer.resetCursor();

    osg::ref_ptr<osg::Node> layerNode = getNode(uriLayer);
    if(layerNode)
    {
        osg::ref_ptr<osg::Group> layerGroup = layerNode->asGroup();
        if(layerGroup)
        {
            std::cout<<"[osgScene > initInfo].....if(layerGroup)"<<std::endl;
            osg::ref_ptr<osg::Switch> switchRoot = new osg::Switch;
            switchRoot->setName("switch_root");
            layerGroup->addChild(switchRoot);

            osg::ref_ptr<osg::Switch> switch0 = new osg::Switch;
            switch0->setName("city");
            switchRoot->addChild(switch0);

            osg::ref_ptr<osg::Switch> switch1 = new osg::Switch;
            switch1->setName("district");
            switchRoot->addChild(switch1);

            osg::ref_ptr<osg::Switch> switch2 = new osg::Switch;
            switch2->setName("building");
            switchRoot->addChild(switch2);

            osg::ref_ptr<osg::Switch> switch3 = new osg::Switch;
            switch3->setName("street");
            switchRoot->addChild(switch3);

            fillSwitches(switchRoot, info);


            InfoDataType* infoData = new InfoDataType(switchRoot);
            switchRoot->setUserData(infoData);
            switchRoot->setUpdateCallback(new UpdateInfo);

        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void OsgScene::fillSwitches(osg::ref_ptr<osg::Switch> switchRoot, std::vector<osgInfo*> v_info)
{
    int cpt = 0;
    std::cout<<"[osgScene > fillSwitches] "<<std::endl;
        for (int i=0; i<v_info.size(); i++)
        {
            if(v_info[i]->getInfoLOD()=="city")
            {
                //std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getInfoName()<<" is LOD : "<<v_info[i]->getInfoLOD()<<std::endl;
                osg::ref_ptr<osg::Switch> citySwitch = switchRoot->getChild(0)->asSwitch();
                if(citySwitch)
                {
                    citySwitch->addChild(dynamic_cast<osg::Group*>(v_info[i]),true);
                    osg::ref_ptr<osg::Group> cityGroup = citySwitch->getChild(citySwitch->getNumChildren()-1)->asGroup();
                    cityGroup->setName(v_info[i]->getInfoName());
                    if(cityGroup)
                    {
                        cityGroup->addChild(v_info[i]->getGroup());
                        std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getInfoName()<<" is added to "<<v_info[i]->getInfoLOD()<<std::endl;
                    }
                }

            }
            if(v_info[i]->getInfoLOD()=="district")
            {
                cpt++;
                //std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getInfoName()<<" is LOD : "<<v_info[i]->getInfoLOD()<<std::endl;
                osg::ref_ptr<osg::Switch> districtSwitch = switchRoot->getChild(1)->asSwitch();
                if(districtSwitch)
                {
                    districtSwitch->addChild(dynamic_cast<osg::Group*>(v_info[i]),true);
                    osg::ref_ptr<osg::Group> districtGroup = districtSwitch->getChild(districtSwitch->getNumChildren()-1)->asGroup();
                    districtGroup->setName(v_info[i]->getInfoName());
                    if(districtGroup)
                    {
                        districtGroup->addChild(v_info[i]->getGroup());
                        std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getInfoName()<<" is added to "<<v_info[i]->getInfoLOD()<<std::endl;
                    }
                }
            }
            if(v_info[i]->getInfoLOD()=="building")
            {
                //std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getInfoName()<<" is LOD : "<<v_info[i]->getInfoLOD()<<std::endl;
                osg::ref_ptr<osg::Switch> buildingSwitch = switchRoot->getChild(2)->asSwitch();
                if(buildingSwitch)
                {
                    //std::cout<<"[osgScene > fillSwitches].....if(buildingSwitch)"<<std::endl;
                    buildingSwitch->addChild(dynamic_cast<osg::Group*>(v_info[i]));
                    //std::cout<<"[osgScene > fillSwitches].....if(buildingSwitch).....dynamic_cast"<<std::endl;
                    osg::ref_ptr<osg::Group> buildingGroup = buildingSwitch->getChild(buildingSwitch->getNumChildren()-1)->asGroup();
                    buildingGroup->setName(v_info[i]->getInfoName());
                    if(buildingGroup)
                    {
                        //std::cout<<"[osgScene > fillSwitches].....if(buildingGroup)"<<std::endl;
                        buildingGroup->addChild(v_info[i]->getGroup());
                        std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getInfoName()<<" is added to "<<v_info[i]->getInfoLOD()<<std::endl;
                    }
                }
            }
            if(v_info[i]->getInfoLOD()=="street")
            {
                //std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getName()<<" is LOD : "<<v_info[i]->getInfoLOD()<<std::endl;
                osg::ref_ptr<osg::Switch> streetSwitch = switchRoot->getChild(3)->asSwitch();
                if(streetSwitch)
                {
                    streetSwitch->addChild(dynamic_cast<osg::Group*>(v_info[i]),true);
                    osg::ref_ptr<osg::Group> streetGroup = streetSwitch->getChild(streetSwitch->getNumChildren()-1)->asGroup();
                    streetGroup->setName(v_info[i]->getInfoName());
                    if(streetGroup)
                    {
                        streetGroup->addChild(v_info[i]->getGroup());
                        std::cout<<"[osgScene > fillSwitches].....info "<<v_info[i]->getInfoName()<<" is added to "<<v_info[i]->getInfoLOD()<<std::endl;
                    }
                }
            }

        }
}

void OsgScene:: filterInfo(const QString& filter)
{
    std::cout<<"[osgScene > filterInfo]"<<std::endl;
    osg::ref_ptr<osg::Node> layer = m_layers->getChild(5);
    if(layer)
    {
        //std::cout<<"[osgScene > filterInfo].....if(layer)"<<std::endl;
        osg::ref_ptr<osg::Group> layerGroup = layer->asGroup();
        if(layerGroup)
        {
            //std::cout<<"[osgScene > filterInfo].....if(layerGroup)"<<std::endl;
            osg::ref_ptr<osg::Switch> switchRoot = layerGroup->getChild(0)->asSwitch();
            if(switchRoot)
            {
                //std::cout<<"[osgScene > filterInfo].....if(layerSwitch)"<<std::endl;
                for(int i=0;i<switchRoot->getNumChildren();++i)
                {
                    osg::ref_ptr<osg::Switch> switchChild = switchRoot->getChild(i)->asSwitch();
                    if(switchChild)
                    {
                        for(int j=0;j<switchChild->getNumChildren();j++)
                        {
                            osg::Node* node = switchChild->getChild(j);
                            osgInfo* info = dynamic_cast<osgInfo*>(node);
        //                    found = filter.toStdString().find(info->getInfoName());
        //                    std::cout<<"[osgScene > filterInfo].....in doc "<<info->getInfoName()<<" found = "<< found <<std::endl;
                                if(filter.toStdString()==info->getType())
                                {
                                    //std::cout<<"[osgScene > filterInfo].....in doc "<<info->getInfoName()<<" found filetype : "<< info->getType() <<std::endl;
                                    info->setRequested(true);
                                }
                                else if(filter.toStdString()==info->getSourceType())
                                {
                                    info->setRequested(true);
                                    //std::cout<<"[osgScene > filterInfo].....in doc "<<info->getInfoName()<<" found sourcetype : "<< info->getSourceType() <<std::endl;
                                }
                                else if(info->getInfoName().find(filter.toStdString())<=100)
                                {
                                    //std::cout<<"[osgScene > filterInfo].....in doc "<<info->getInfoName()<<" found word : "<< filter.toStdString() <<std::endl;
                                    info->setRequested(true);
                                }
                                else if(filter.toStdString()=="")
                                {
                                    info->setRequested(true);
                                }
                                else
                                {
                                    info->setRequested(false);
                                }
                                std::cout<<"[osgScene > filterInfo].....document : "<<info->getInfoName()<<std::endl;
                                std::cout<<"[osgScene > filterInfo]...........isRequested : "<<info->isRequested()<<std::endl;
                        }
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void OsgScene::setTileName(const vcity::URI& uriTile, const std::string& name)
{
    osg::ref_ptr<osg::Node> tile = getNode(uriTile);
    if (tile)
    {
        tile->setName(name);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::deleteTile(const vcity::URI& uriTile)
{
    osg::ref_ptr<osg::Node> tile = getNode(uriTile);
    if (tile)
    {
        tile->getParent(0)->removeChild(tile);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::addAssimpNode(const vcity::URI& uriLayer, const osg::ref_ptr<osg::Node> node)
{
    osg::ref_ptr<osg::Node> layer = getNode(uriLayer);
    if (layer)
    {
        osg::ref_ptr<osg::Group> layerGroup = layer->asGroup();
        if (layerGroup)
            layerGroup->addChild(node);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::setAssimpNodeName(const vcity::URI& uri, const std::string& name)
{
    osg::ref_ptr<osg::Node> assimpNode = getNode(uri);
    if (assimpNode)
    {
        assimpNode->setName(name);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::deleteAssimpNode(const vcity::URI& uri)
{
    osg::ref_ptr<osg::Node> assimpNode = getNode(uri);
    if (assimpNode)
    {
        assimpNode->getParent(0)->removeChild(assimpNode);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::addMntAscNode(const vcity::URI& uriLayer, const osg::ref_ptr<osg::Node> node)
{
    osg::ref_ptr<osg::Node> layer = getNode(uriLayer);
    if (layer)
    {
        osg::ref_ptr<osg::Group> layerGroup = layer->asGroup();
        if (layerGroup)
            layerGroup->addChild(node);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::addLasNode(const vcity::URI& uriLayer, const osg::ref_ptr<osg::Node> node)
{
    osg::ref_ptr<osg::Node> layer = getNode(uriLayer);
    if (layer)
    {
        osg::ref_ptr<osg::Group> layerGroup = layer->asGroup();
        if (layerGroup)
            layerGroup->addChild(node);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::addShpNode(const vcity::URI& uriLayer, const osg::ref_ptr<osg::Node> node)
{
    osg::ref_ptr<osg::Node> layer = getNode(uriLayer);
    if (layer)
    {
        osg::ref_ptr<osg::Group> layerGroup = layer->asGroup();
        if (layerGroup)
            layerGroup->addChild(node);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::addLayer(const std::string& name)
{
    osg::ref_ptr<osg::Group> layer = new osg::Group();
    layer->setName(name);
    m_layers->addChild(layer);
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::setLayerName(const vcity::URI& uriLayer, const std::string& name)
{
    osg::ref_ptr<osg::Node> layer = getNode(uriLayer);
    if (layer)
    {
        layer->setName(name);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::deleteLayer(const vcity::URI& uriLayer)
{
    osg::ref_ptr<osg::Node> layer = getNode(uriLayer);
    if (layer)
    {
        layer->getParent(0)->removeChild(layer);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::deleteNode(const vcity::URI& uri)
{
    osg::ref_ptr<osg::Node> node = getNode(uri);
    if (node)
    {
        node->getParent(0)->removeChild(node);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::updateGrid()
{
    // remove previous grid
    vcity::URI uri;
    uri.append("grid");
    deleteNode(uri);

    // set grid
    const vcity::DataProfile& dp = appGui().getSettings().getDataProfile();
    osg::Vec3 bbox_lower(dp.m_bboxLowerBound.x, dp.m_bboxLowerBound.y, dp.m_bboxLowerBound.z);
    osg::Vec3 bbox_upper(dp.m_bboxUpperBound.x, dp.m_bboxUpperBound.y, dp.m_bboxUpperBound.z);
    osg::Vec2 step(dp.m_xStep, dp.m_yStep);
    osg::Vec3 offset(dp.m_offset.x, dp.m_offset.y, dp.m_offset.z);
    osg::Vec2 tileOffset(dp.m_TileIdOriginX, dp.m_TileIdOriginY);
    osg::ref_ptr<osg::Node> grid = buildGrid(bbox_lower, bbox_upper, step, offset, tileOffset);

    m_layers->addChild(grid);
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::setShadow(bool shadow)
{
    if (shadow)
    {
        replaceChild(m_effectNone, m_effectShadow);
    }
    else
    {
        replaceChild(m_effectShadow, m_effectNone);
    }

    m_shadow = shadow;
}
////////////////////////////////////////////////////////////////////////////////
void setTexture(osg::ref_ptr<osg::Node> node, citygml::CityObjectTag* tag, osg::ref_ptr<osg::Texture2D> texture)
{
    osg::ref_ptr<osg::Group> grp = node->asGroup();
    if (grp)
    {
        for (unsigned int i = 0; i < grp->getNumChildren(); ++i)
        {
            osg::ref_ptr<osg::Node> child = grp->getChild(i);
            setTexture(child, tag, texture);
        }
    }

    osg::ref_ptr<osg::Geode> geode = node->asGeode();
    if (geode)
    {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i)
        {
            osg::StateSet* stateset = geode->getDrawable(i)->getOrCreateStateSet();
            //if(texture) stateset->setTextureAttributeAndModes( 0, texture, osg::StateAttribute::ON );
            std::cout << "texture : " << texture << std::endl;
            if (texture) stateset->setTextureAttribute(0, texture, osg::StateAttribute::ON);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::setDateRec(const QDateTime& date, osg::ref_ptr<osg::Node> node)
{
    // -4000 is used as a special value to disable time
    int year = date.date().year();

    osg::ref_ptr<osg::Group> grp = node->asGroup();
    if (grp)
    {
        // dyntag
        /*double val;
        bool hasFlag = node->getUserValue("TAGPTR", val);
        if(hasFlag)
        {
        citygml::CityObjectTag* tag;
        memcpy(&tag, &val, sizeof(tag));
        std::string texturePath = tag->getAttribute("texture", date);
        if(texturePath != "none")
        {
        std::cout << date.toString().toStdString() << " : texture : " << texturePath << std::endl;

        // check cache
        osg::ref_ptr<osg::Texture2D> texture = nullptr;
        std::map<std::string, osg::ref_ptr<osg::Texture2D> >::iterator it = m_texManager.find(texturePath);
        if(it!=m_texManager.end())
        {
        texture = it->second;
        }
        else
        {
        if(osg::Image* image = osgDB::readImageFile(texturePath))
        {
        //osg::notify(osg::NOTICE) << "  Info: Texture " << m_settings.m_filepath+"/"+t->getUrl() << " loaded." << std::endl;
        //std::cout << "  Loading texture " << t->getUrl() << " for polygon " << p->getId() << "..." << std::endl;
        texture = new osg::Texture2D;
        texture->setImage( image );
        texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
        texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
        texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
        texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
        texture->setWrap( osg::Texture::WRAP_R, osg::Texture::REPEAT );

        m_texManager[texturePath] = texture;
        }
        else
        osg::notify(osg::NOTICE) << "  Warning: Texture " << texturePath << " not found!" << std::endl;
        }

        setTexture(node, tag, texture);
        }
        }*/

        // get attributes in cityobject
#if 0
        vcity::URI uri = osgTools::getURI(node);
        citygml::CityObject* obj = appGui().getScene().getCityObjectNode(uri);
        if (obj && obj->getType() == citygml::COT_Building)
        {
            std::string strAttr = obj->getAttribute("yearOfConstruction");
            int yearOfConstruction = (strAttr.empty() ? -4000 : std::stoi(strAttr));
            strAttr = obj->getAttribute("yearOfDemolition");
            int yearOfDemolition = (strAttr.empty() ? -4000 : std::stoi(strAttr));

            //std::cout << obj->getId() << " : " << yearOfConstruction << " / " << yearOfDemolition << std::endl;

            if (((yearOfConstruction == -4000 || yearOfDemolition == -4000) || (yearOfConstruction < year && year <= yearOfDemolition)))
            {
                node->setNodeMask(0xffffffff);
            }
            else
            {
                node->setNodeMask(0);
            }
        }
#endif

        //hide node if unchecked in treeview
        vcity::URI uri = osgTools::getURI(node);
        QTreeWidgetItem* item = appGui().getTreeView()->getNode(uri);
        bool unchecked = (item == nullptr) ? false : item->checkState(0) == Qt::CheckState::Unchecked;
        if (unchecked)
        {
            node->setNodeMask(0);
        }
        else
        {
            // check attributes from tags
            int yearOfConstruction;
            int yearOfDemolition;

            bool a = node->getUserValue("yearOfConstruction", yearOfConstruction);
            bool b = node->getUserValue("yearOfDemolition", yearOfDemolition);

            //std::cout << node->getName() << " : " << a <<  " : yearOfConstruction : " << yearOfConstruction << std::endl;
            //std::cout << node->getName() << " : " << b << " : yearOfDemolition : " << yearOfDemolition << std::endl;

            std::string cDate;
            std::string dDate;
            bool c = node->getUserValue("creationDate", cDate);
            bool d = node->getUserValue("terminationDate", dDate);

            if (a && b)
            {
                if ((yearOfConstruction < year && year <= yearOfDemolition))
                {
                    node->setNodeMask(0xffffffff);
                }
                else
                {
                    node->setNodeMask(0);
                }
                //node->setNodeMask(0xffffffff - node->getNodeMask());
            }
            else if (c)
            {
                QDateTime creationDate = QDateTime::fromString(QString::fromStdString(cDate), Qt::ISODate);
                if (d)
                {
                    QDateTime terminationDate = QDateTime::fromString(QString::fromStdString(dDate), Qt::ISODate);
                    if (creationDate < date && date <= terminationDate)
                    {
                        node->setNodeMask(0xffffffff);
                    }
                    else
                    {
                        node->setNodeMask(0);
                    }
                }
                else {
                    if (creationDate < date) node->setNodeMask(0xffffffff);
                    else node->setNodeMask(0);
                }
            }
            else node->setNodeMask(0xffffffff);
            for (unsigned int i = 0; i < grp->getNumChildren(); ++i)
            {
                osg::ref_ptr<osg::Node> child = grp->getChild(i);
                setDateRec(date, child);
            }
        }

        //osg::ref_ptr<osg::Geode> geode = node->asGeode();
        if (node)
        {
            int tagged = 0;
            bool c = node->getUserValue("TAGGED", tagged);
            if (c && tagged)
            {
                node->setNodeMask(0);
                //node->getParent(0)->setNodeMask(0);
                //std::cout << "hide TAGGED default geom : " << osgTools::getURI(node).getStringURI() << " / " << typeid(node).name() << " : " << node->getNodeMask() << std::endl;
            }
        }
    }

    // reset : force draw
    if (date.date().year() == -4000)
    {
        //node->setNodeMask(0xffffffff);
        vcity::URI uri = osgTools::getURI(node);
        QTreeWidgetItem* item = appGui().getTreeView()->getNode(uri);
        bool unchecked = (item == nullptr) ? false : item->checkState(0) == Qt::CheckState::Unchecked;
        if (unchecked)
            node->setNodeMask(0);
        else node->setNodeMask(0xffffffff);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::setDate(const QDateTime& date)
{
    setDateRec(date, this);
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::reset()
{
    std::vector<osg::Node*> nodes;
    for (unsigned int i = 0; i < getNumChildren(); ++i)
    {
        nodes.push_back(getChild(i));
    }
    std::vector<osg::Node*>::iterator it = nodes.begin();
    for (; it != nodes.end(); ++it)
    {
        removeChild(*it);
    }

    init();
}
////////////////////////////////////////////////////////////////////////////////
void forceLODrec(int lod, osg::ref_ptr<osg::Node> node)
{
    //05/11/2015 Frederic : Fonction qui ne me semble pas bien propre. On parcourt l'arbre osg qui semble vide de semantique, on a juste un ensemble de nodes et de sous nodes.
    //Le CityModel est le sous node du layer CityGML, ensuite il y a les batiments, puis les murs, puis les geometries, puis les polygones, ...
    //Visiblement, un objet concerne par des LODs possedent obligatoirement 5 enfants "geodes" (1 par LoD) et on va donc chercher les objets qui ont 5 fils repertories comme des "geodes" (lors du chargement, je suppose qu'on doit assimiler les Lods a �a).
    //Parmi ces 5 enfants, on va donc cacher le "lodieme".
    //Probleme entre LoD1 et LoD2 : la notion de Lod apparait un cran plus haut sur un LOD1 que sur un LOD2 (car il y a un fils Wall, Roof, ... dans le LOD2 avant le Lod2multisurface).
    //Donc quand on arrive au niveau du LOD1, finalement le Lod2 correspondant (grp->getChild(2)) ne signifie rien, il n'appaitra reellement que lorsque l'on parcourera les murs et walls par la suite. Et a ce moment, c'est grp->getChild(1) qui n'aura pas de sens.
    //Cela oblige a tester tous les fils car on ne sait jamais a quel niveau on va trouver une notion de LOD et c'est donc tres long !

    appGui().getControllerGui().resetSelection();
    osg::ref_ptr<osg::Group> grp = node->asGroup();

    if (grp)
    {
        int count = grp->getNumChildren();

        // check if we had 5 LODs geodes
        int numGeodes = 0;
        for (int i = 0; i < count; ++i)
        {
            osg::ref_ptr<osg::Node> child = grp->getChild(i);
            if (child->asGeode())
            {
                ++numGeodes;
            }
        }

        if (numGeodes == 5) // yes, enable or disable the good lods
        {
            grp->getChild(lod)->setNodeMask(0xffffffff - grp->getChild(lod)->getNodeMask());
        }
        if (lod != 1 || numGeodes != 5) //Si on travaille sur le LOD1, pas besoin d'aller voir les enfants si le grp courant a deja cache/montre son LOD1 -> gain de temps
        {
            for (int i = 0; i < count; ++i)
            {
                osg::ref_ptr<osg::Node> child = grp->getChild(i);
                forceLODrec(lod, child);
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::forceLOD(int lod)
{
    forceLODrec(lod, m_layers);
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::showNode(osg::ref_ptr<osg::Node> node, bool show)
{
    if (node)
    {
        //std::cout << node->getName() << std::endl;
        if (show)
        {
            if (appGui().getMainWindow()->m_useTemporal)
            {
                QDateTime date = appGui().getMainWindow()->m_currentDate;
                int year = date.date().year();
                // check attributes from tags
                int yearOfConstruction;
                int yearOfDemolition;
                bool a = node->getUserValue("yearOfConstruction", yearOfConstruction);
                bool b = node->getUserValue("yearOfDemolition", yearOfDemolition);

                int cDate;
                int dDate;
                bool c = node->getUserValue("creationDate", cDate);
                bool d = node->getUserValue("terminationDate", dDate);

                if (a && b)
                {
                    if ((yearOfConstruction < year && year <= yearOfDemolition)) node->setNodeMask(0xffffffff);
                    else node->setNodeMask(0);
                }
                else if (c)
                {
                    QDateTime creationDate = QDateTime::fromString(QString::fromStdString(std::to_string(cDate)), QString("yyyyMMdd"));
                    if (d)
                    {
                        QDateTime terminationDate = QDateTime::fromString(QString::fromStdString(std::to_string(dDate)), QString("yyyyMMdd"));
                        if (creationDate < date && date <= terminationDate) node->setNodeMask(0xffffffff);
                        else node->setNodeMask(0);
                    }
                    else {
                        if (creationDate < date) node->setNodeMask(0xffffffff);
                        else node->setNodeMask(0);
                    }
                }
                else node->setNodeMask(0xffffffff);
            }
            else
            {
                //node->setNodeMask(~0x0);
                node->setNodeMask(0xffffffff);
                /*if(node->asGroup())
                {
                node->asGroup()->getChild(0)->setNodeMask(0xffffffff);
                }*/
            }
        }
        else
        {
            node->setNodeMask(0x0);
            /*if(node->asGroup())
            {
            node->asGroup()->getChild(0)->setNodeMask(0x0);
            }*/
            //node->getParent(0)->setNodeMask(0x0);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::showNode(const vcity::URI& uri, bool show)
{
    showNode(getNode(uri), show);
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::centerOn(const vcity::URI& uri)
{
    osg::ref_ptr<osg::Node> node = getNode(uri);
    if (node)
    {
        appGui().getMainWindow()->m_osgView->m_osgView->getCameraManipulator()->setNode(node);
        appGui().getMainWindow()->m_osgView->m_osgView->getCameraManipulator()->computeHomePosition();
        appGui().getMainWindow()->m_osgView->m_osgView->home();
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::dump(std::ostream& out, osg::ref_ptr<osg::Node> node, int depth)
{
    if (node == NULL)
    {
        node = this;
    }

    for (int i = 0; i < depth; ++i) out << "  ";
    out << depth << " " << node->getName() << " : " << node << std::endl;

    osg::ref_ptr<osg::Group> grp = node->asGroup();
    if (grp)
    {
        for (unsigned int i = 0; i < grp->getNumChildren(); ++i)
        {
            osg::ref_ptr<osg::Node> child = grp->getChild(i);
            dump(out, child, depth + 1);
        }
    }

    if (node->asTransform() && node->asTransform()->asPositionAttitudeTransform())
    {
        const osg::Vec3d pos = node->asTransform()->asPositionAttitudeTransform()->getPosition();
        out << "pos : " << pos.x() << ", " << pos.y() << std::endl;
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::optim()
{
    osgUtil::Optimizer optimizer;
    optimizer.optimize(this, osgUtil::Optimizer::ALL_OPTIMIZATIONS);
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::buildTemporalNodes(const vcity::URI& uri, const vcity::Tile& tile)
{
    for (citygml::CityObject* child : tile.getCityModel()->getCityObjectsRoots())
    {
        vcity::URI u = uri;
        u.append(child->getId(), child->getTypeAsString());
        u.resetCursor();
        buildTemporalNodesRec(u, child);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::buildTemporalNodesRec(const vcity::URI& uri, citygml::CityObject* obj)
{
    // add tags geom
    for (citygml::CityObjectTag* tag : obj->getTags())
    {
        appGui().getControllerGui().addTag(uri, tag);
    }
    //add yearOfConstruction and yearOfDemolition to tags geom
    obj->checkTags();
    // recursive call
    for (citygml::CityObject* child : obj->getChildren())
    {
        vcity::URI u = uri;
        u.append(obj->getId(), obj->getTypeAsString());
        u.resetCursor();
        buildTemporalNodesRec(u, child);
    }
}
////////////////////////////////////////////////////////////////////////////////
void OsgScene::buildCityObject(const vcity::URI& uri, osg::ref_ptr<osg::Group> nodeOsg, citygml::CityObject* obj, ReaderOsgCityGML& reader, int depth, osg::ref_ptr<osg::Group> nodeVersion, osg::ref_ptr<osg::Group> nodeWorkspace)
{
    osg::ref_ptr<osg::Group> node = reader.createCityObject(obj);

    osg::ref_ptr<osg::Group> fatherNode;
    fatherNode = nodeOsg;

    if (nodeWorkspace)
    {
        fatherNode->addChild(nodeWorkspace);
        fatherNode = nodeWorkspace;
    }
    if (nodeVersion)
    {
        fatherNode->addChild(nodeVersion);
        fatherNode = nodeVersion;
    }
    fatherNode->addChild(node);

    for (citygml::CityObject* child : obj->getChildren())
    {
        vcity::URI u = uri;
        u.append(child->getId(), child->getTypeAsString());
        u.resetCursor();
        buildCityObject(u, node, child, reader, depth + 1);
    }
    for (citygml::Object* target : obj->getXLinkTargets())
    {
        citygml::CityObject* child = (citygml::CityObject*) target;
        vcity::URI u = uri;
        u.append(child->getId(), child->getTypeAsString());
        u.resetCursor();
        buildCityObject(u, node, child, reader, depth + 1);
    }

    // add tags geom
    /*for(citygml::CityObjectTag* tag : obj->getTags())
    {
    appGui().getControllerGui().addTag(uri, tag);
    }*/
}
////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Node> OsgScene::buildTile(const vcity::URI& uri, const vcity::Tile& tile)
{
    osg::ref_ptr<osg::PositionAttitudeTransform> root = new osg::PositionAttitudeTransform();
    root->setName(tile.getName());

    // create osg geometry builder
    size_t pos = tile.getCityGMLfilePath().find_last_of("/\\");
    std::string path = tile.getCityGMLfilePath().substr(0, pos);
    ReaderOsgCityGML readerOsgGml(path);
    readerOsgGml.m_settings.m_useTextures = vcity::app().getSettings().m_loadTextures;

    // VERSIONS & WORKSPACES
    const citygml::CityModel* citymodel = tile.getCityModel();

    //std::cout<<"Workspaces:"<<std::endl;
    std::map<std::string, temporal::Workspace> workspaces = citymodel->getWorkspaces();
    for (std::map<std::string, temporal::Workspace>::iterator it = workspaces.begin(); it != workspaces.end(); it++)
    {
        //std::cout<<it->second.name<<std::endl;
        osg::ref_ptr<osg::Group> groupWorkspace;
        groupWorkspace = new osg::Group;
        groupWorkspace->setName(it->second.name);
        std::string strType = "Workspace";
        groupWorkspace->setUserValue("type", strType);

        for (temporal::Version* v : it->second.versions)
        {
            //std::cout<<"    - "<<v->getId()<<std::endl;
            osg::ref_ptr<osg::Group> groupVersion;
            groupVersion = new osg::Group;
            groupVersion->setName(v->getId());
            std::string strType = "Version";
            groupVersion->setUserValue("type", strType);

            std::vector<citygml::CityObject*>* members = v->getVersionMembers();
            for (std::vector<citygml::CityObject*>::iterator it = members->begin(); it != members->end(); it++)
            {
                //std::cout<<"        - member: "<<(*it)->getId()<<std::endl;
                {
                    vcity::URI u = uri;
                    u.append((*it)->getId(), (*it)->getTypeAsString());
                    u.resetCursor();
                    buildCityObject(u, root, (*it), readerOsgGml, 0, groupVersion, groupWorkspace);
                }
            }

            groupVersion.release();
        }

        groupWorkspace.release();
    }

    //std::cout<<"Versions:"<<std::endl;
    std::vector<temporal::Version*> versions = citymodel->getVersions();
    for (temporal::Version* version : versions)
    {
        if (!version->_isInWorkspace)
        {
            //std::cout<<"Version \""<<version->getId()<<"\" :"<<std::endl;
            osg::ref_ptr<osg::Group> groupVersion;
            groupVersion = new osg::Group;
            groupVersion->setName(version->getId());
            std::string strType = "Version";
            groupVersion->setUserValue("type", strType);

            std::vector<citygml::CityObject*>* members = version->getVersionMembers();
            for (std::vector<citygml::CityObject*>::iterator it = members->begin(); it != members->end(); it++)
            {
                //std::cout<<"    - member: "<<(*it)->getId()<<std::endl;
                {
                    vcity::URI u = uri;
                    u.append((*it)->getId(), (*it)->getTypeAsString());
                    u.resetCursor();
                    buildCityObject(u, root, (*it), readerOsgGml, 0, groupVersion);
                }
            }

            groupVersion.release();
        }
    }
    // VERSIONS & WORKSPACES

    for (citygml::CityObject* child : tile.getCityModel()->getCityObjectsRoots())
    {
        if (!(child)->_isInVersion)
        {
            vcity::URI u = uri;
            u.append(child->getId(), child->getTypeAsString());
            u.resetCursor();
            buildCityObject(u, root, child, readerOsgGml);
        }
    }
    return root;
}
////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Node> OsgScene::getNode(const vcity::URI& uri)
{
    osg::ref_ptr<osg::Node> res = nullptr;
    osg::ref_ptr<osg::Group> current = m_layers;

    while (uri.getCursor() < uri.getDepth())
    {
        int count = current->getNumChildren();
        for (int i = 0; i < count; ++i)
        {
            osg::ref_ptr<osg::Node> child = current->getChild(i);
            if (child->getName() == uri.getCurrentNode())
            {
                res = child;
                if (!(current = child->asGroup()))
                {
                    return res;
                }
                break;
            }
        }
        uri.popFront();
    }

    return res;

    /*FindNamedNode f(uri.getLastNode());
    accept(f);
    return f.getNode();*/
}
////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Node> OsgScene::createInfoBubble(osg::ref_ptr<osg::Node> node)
{
    osg::ref_ptr<osg::Group> grp = node->asGroup();
    if (grp)
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setName("infobubble");

        // Print the city object name on top of it
        geode->getBoundingBox().center();
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setFont("arial.ttf");
        text->setCharacterSize(24);
        text->setBackdropType(osgText::Text::OUTLINE);
        text->setFontResolution(64, 64);
        text->setText(node->getName(), osgText::String::ENCODING_UTF8);
        //text->setCharacterSizeMode( osgText::TextBase::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT );
        text->setCharacterSizeMode(osgText::TextBase::SCREEN_COORDS);
        text->setAxisAlignment(osgText::TextBase::SCREEN);
        text->setAlignment(osgText::TextBase::CENTER_BOTTOM);
        text->setPosition(node->getBound().center() + osg::Vec3(0, 0, node->getBound().radius()));
        text->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);
        geode->addDrawable(text.get());

        grp->addChild(geode);

        return geode;
    }

    return nullptr;
}
////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Node> OsgScene::buildGrid(const osg::Vec3& bbox_lower, const osg::Vec3& bbox_upper, const osg::Vec2& step, const osg::Vec3& offset, const osg::Vec2& tileOffset)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setName("grid_geode");

    osg::Geometry* geom = new osg::Geometry;
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    double lenx = bbox_upper.x() - bbox_lower.x(); // x length of the grid
    double leny = bbox_upper.y() - bbox_lower.y(); // y length of the grid

    int nx = lenx / step.x(); // x number of cell
    int ny = leny / step.y(); // y number of cell

    int indiceOffset = 0;
    for (int i = 0; i <= nx; ++i)
    {
        // vertical line
        vertices->push_back(osg::Vec3(bbox_lower.x() + i*step.x(), bbox_lower.y(), bbox_lower.z() + 0) - offset);
        vertices->push_back(osg::Vec3(bbox_lower.x() + i*step.x(), bbox_upper.y(), bbox_lower.z() + 0) - offset);
        indices->push_back(indiceOffset++); indices->push_back(indiceOffset++);
    }

    for (int j = 0; j <= ny; ++j)
    {
        // horizontal line
        vertices->push_back(osg::Vec3(bbox_lower.x(), bbox_lower.y() + j*step.y(), bbox_lower.z() + 0) - offset);
        vertices->push_back(osg::Vec3(bbox_upper.x(), bbox_lower.y() + j*step.y(), bbox_lower.z() + 0) - offset);
        indices->push_back(indiceOffset++); indices->push_back(indiceOffset++);
    }

    // put names
    for (int i = 0; i < nx; ++i)
    {
        for (int j = 0; j < ny; ++j)
        {
            osgText::Text* text = new osgText::Text;
            std::stringstream ss;
            ss.precision(0);
            ss << std::fixed << ' ' << bbox_lower.x() + i*step.x() - offset.x() << " , " << bbox_lower.y() + j*step.y() - offset.y() << "\n" <<
                ' ' << bbox_lower.x() + i*step.x() << " , " << bbox_lower.y() + j*step.y() << "\n Tile : " << tileOffset.x() + i << "-" << tileOffset.y() + j;
            text->setText(ss.str());
            text->setColor(osg::Vec4(0, 0, 0, 1));
            text->setPosition(osg::Vec3(bbox_lower.x() + i*step.x(), bbox_lower.y() + j*step.y() + step.y()*0.2, bbox_lower.z() + 0) - offset);
            geode->addDrawable(text);
        }
    }

    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(indices);
    geode->addDrawable(geom);

    osg::Group* res = new osg::Group;
    res->setName("grid");
    res->addChild(geode);

    return res;
}
////////////////////////////////////////////////////////////////////////////////
osg::ref_ptr<osg::Geode> OsgScene::buildBBox(osg::Vec3 lowerBound, osg::Vec3 upperBound)
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
                vertices->push_back(osg::Vec3(lowerBound.x() + x*step.x(), lowerBound.y() + y*step.y(), lowerBound.z() + z*step.z()));

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
