// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#include "osgUpdateInfo.hpp"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

UpdateInfo::UpdateInfo()
{
    std::cout<<"Update created"<<std::endl;
}

void UpdateInfo::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
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

            for(unsigned int i=0; i<switchRoot->getNumChildren(); i++)
            {
                osg::ref_ptr<osg::Switch> subSwitch = switchRoot->getChild(i)->asSwitch();
                if(subSwitch)
                {
                    for(unsigned int j=0; j<subSwitch->getNumChildren();j++)
                    {
                        osg::Node* node = subSwitch->getChild(j);
                        osgInfo* info = dynamic_cast<osgInfo*>(node);

                        layerInfo->computeDCAM(cam, info);
                        layerInfo->computeDSC(cam, screenX, screenY, info);

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
}