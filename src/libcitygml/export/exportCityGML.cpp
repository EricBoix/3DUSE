////////////////////////////////////////////////////////////////////////////////
#include "exportCityGML.hpp"
#include <libxml/parser.h>
#include <sstream>
////////////////////////////////////////////////////////////////////////////////
namespace citygml
{
////////////////////////////////////////////////////////////////////////////////
ExporterCityGML::ExporterCityGML(const std::string& filename)
    : m_fileName(filename), m_doc(nullptr), m_root_node(nullptr), m_currentAppearence(nullptr)
{
}
////////////////////////////////////////////////////////////////////////////////
ExporterCityGML::~ExporterCityGML()
{
}
////////////////////////////////////////////////////////////////////////////////
void ExporterCityGML::initExport(bool createCityModelRootNode)
{
     m_doc = nullptr;
     m_root_node = nullptr;

     LIBXML_TEST_VERSION;

     /*
      * Creates a new document, a node and set it as a root node
      */
     m_doc = xmlNewDoc(BAD_CAST "1.0");

     if(createCityModelRootNode)
     {
         m_root_node = xmlNewNode(NULL, BAD_CAST "core:CityModel");
     }
}
////////////////////////////////////////////////////////////////////////////////
void ExporterCityGML::endExport()
{
    xmlDocSetRootElement(m_doc, m_root_node);

    /*
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc(m_fileName.c_str(), m_doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(m_doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
}
////////////////////////////////////////////////////////////////////////////////
void ExporterCityGML::exportCityModel(const CityModel& model)
{
    initExport(false);

    m_root_node = exportCityModelXml(model);

    endExport();
}
////////////////////////////////////////////////////////////////////////////////
void ExporterCityGML::exportCityObject(const std::vector<const CityObject*>& models)
{
    initExport(false);

    m_root_node = exportCityObjectModelXml(models);

    endExport();
}
////////////////////////////////////////////////////////////////////////////////
void ExporterCityGML::appendCityObject(const std::vector<const CityObject*>& objs)
{
    for(const CityObject* obj : objs)
    {
        appendCityObject(*obj);
    }
}
////////////////////////////////////////////////////////////////////////////////
void ExporterCityGML::appendCityObject(const CityObject& obj)
{
    xmlNodePtr node = xmlNewChild(m_root_node, NULL, BAD_CAST "core:cityObjectMember", NULL);
    exportCityObjetXml(obj, node);
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportEnvelopeXml(const citygml::Envelope& env, xmlNodePtr parent)
{
    xmlNodePtr res = xmlNewChild(parent, NULL, BAD_CAST "gml:Envelope", NULL);

    std::stringstream ss;
    TVec3d vec = env.getLowerBound();
    ss << std::fixed << vec.x << " " << vec.y << " " << vec.z;
    xmlNewChild(res, NULL, BAD_CAST "gml:lowerCorner", BAD_CAST ss.str().c_str());
    //ss.clear();
    ss.str(std::string());
    vec = env.getUpperBound();
    ss << std::fixed << vec.x << " " << vec.y << " " << vec.z;
    xmlNewChild(res, NULL, BAD_CAST "gml:upperCorner", BAD_CAST ss.str().c_str());

    return res;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportLinearRingXml(const citygml::LinearRing& ring, xmlNodePtr parent)
{
    xmlNodePtr res = xmlNewChild(parent, NULL, BAD_CAST "gml:LinearRing", NULL);
    xmlNewProp(res, BAD_CAST "gml:id", BAD_CAST ring.getId().c_str());

    std::stringstream ss;
    for(const TVec3d& v : ring.getVertices())
    {
        ss << std::fixed << v.x << " " << v.y << " " << v.z << " ";
    }
    const TVec3d& v = ring.getVertices().front();
    ss << std::fixed << v.x << " " << v.y << " " << v.z;
    xmlNewChild(res, NULL, BAD_CAST "gml:posList", BAD_CAST ss.str().c_str());

    return res;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportPolygonXml(const citygml::Polygon& poly, xmlNodePtr parent)
{
    xmlNodePtr res = xmlNewChild(parent, NULL, BAD_CAST "gml:Polygon", NULL);
    xmlNewProp(res, BAD_CAST "gml:id", BAD_CAST poly.getId().c_str());

    const std::vector<citygml::LinearRing*>& lrings = poly.getInteriorRings();
    std::vector<citygml::LinearRing*>::const_iterator it = lrings.begin();
    for(; it != lrings.end(); ++it)
    {
        xmlNodePtr node = xmlNewChild(res, NULL, BAD_CAST "gml:interior", NULL);
        exportLinearRingXml(**it, node);
    }

    if(poly.getExteriorRing())
    {
        xmlNodePtr node = xmlNewChild(res, NULL, BAD_CAST "gml:exterior", NULL);
        exportLinearRingXml(*poly.getExteriorRing(), node);
    }

    return res;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportGeometryGenericXml(const citygml::Geometry& geom, const std::string& nodeType, xmlNodePtr parent)
{
    xmlNodePtr res = xmlNewChild(parent, NULL, BAD_CAST nodeType.c_str(), NULL);
    //xmlNewProp(res, BAD_CAST "gml:id", BAD_CAST bldg.getId().c_str());
    /*xmlNodePtr node1 = */xmlNewChild(res, NULL, BAD_CAST "gml:name", BAD_CAST geom.getId().c_str());
    xmlNodePtr node2 = xmlNewChild(res, NULL, BAD_CAST (std::string("bldg:lod")+std::to_string(geom.getLOD())+"MultiSurface").c_str(), NULL);
    xmlNodePtr node3 = xmlNewChild(node2, NULL, BAD_CAST "gml:surfaceMember", NULL);

    for(const citygml::Polygon* poly : geom.getPolygons())
    {
        exportPolygonAppearanceXml(*poly, m_currentAppearence);
        exportPolygonXml(*poly, node3);
    }

    return res;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportGeometryXml(const citygml::Geometry& geom, xmlNodePtr parent)
{
    switch(geom.getType())
    {
    case citygml::GT_Unknown:
        return exportGeometryGenericXml(geom, "bldg:Unknown", parent);
        break;
    case citygml::GT_Roof:
        return exportGeometryGenericXml(geom, "bldg:Roof", parent);
        break;
    case citygml::GT_Wall:
        return exportGeometryGenericXml(geom, "bldg:Wall", parent);
        break;
    case citygml::GT_Ground:
        return exportGeometryGenericXml(geom, "bldg:Ground", parent);
        break;
    case citygml::GT_Closure:
        return exportGeometryGenericXml(geom, "bldg:Closure", parent);
        break;
    case citygml::GT_Floor:
        return exportGeometryGenericXml(geom, "bldg:Floor", parent);
        break;
    case citygml::GT_InteriorWall:
        return exportGeometryGenericXml(geom, "bldg:InteriorWall", parent);
        break;
    case citygml::GT_Ceiling:
        return exportGeometryGenericXml(geom, "bldg:Ceiling", parent);
        break;
    default:
        break;
    }

    return NULL;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportCityObjetGenericXml(const citygml::CityObject& obj, const std::string& nodeType, xmlNodePtr parent)
{
    xmlNodePtr res = xmlNewChild(parent, NULL, BAD_CAST nodeType.c_str(), NULL);
    xmlNewProp(res, BAD_CAST "gml:id", BAD_CAST obj.getId().c_str());

    // children are not handled here, but in the upper call level (exportCityObjetXml)
    /*for(const auto& child : obj.getChildren())
    {
        exportCityObjetXml(child, res);
    }*/

    // attributes
    for(const auto& attr : obj.getAttributes())
    {
        xmlNodePtr attrNode = xmlNewChild(res, NULL, BAD_CAST "gen:stringAttribute", NULL);
        xmlNewProp(attrNode, BAD_CAST "name", BAD_CAST attr.first.c_str());
        xmlNewChild(attrNode, NULL, BAD_CAST "gen:value", BAD_CAST attr.second.c_str());
    }

    return res;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportCityObjetStateXml(const citygml::CityObjectState& state, const std::string &nodeType, xmlNodePtr parent)
{
    xmlNodePtr res = xmlNewChild(parent, NULL, BAD_CAST nodeType.c_str(), NULL);
    xmlNewProp(res, BAD_CAST "gml:id", BAD_CAST (state.getParent()->getId()+'_'+state.getStringId()).c_str());

    if(state.getGeom())
    {
        // attributes
        for(const auto& attr : state.getGeom()->getAttributes())
        {
            xmlNodePtr attrNode = xmlNewChild(res, NULL, BAD_CAST "gen:stringAttribute", NULL);
            xmlNewProp(attrNode, BAD_CAST "name", BAD_CAST attr.first.c_str());
            xmlNewChild(attrNode, NULL, BAD_CAST "gen:value", BAD_CAST attr.second.c_str());
        }
    }

    return res;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportCityObjetTagXml(const citygml::CityObjectTag& tag, const std::string &nodeType, xmlNodePtr parent)
{
    xmlNodePtr res = xmlNewChild(parent, NULL, BAD_CAST nodeType.c_str(), NULL);
    xmlNewProp(res, BAD_CAST "gml:id", BAD_CAST (tag.getParent()->getId()+'_'+tag.getStringId()).c_str());

    if(tag.getGeom())
    {
        // attributes
        for(const auto& attr : tag.getGeom()->getAttributes())
        {
            xmlNodePtr attrNode = xmlNewChild(res, NULL, BAD_CAST "gen:stringAttribute", NULL);
            xmlNewProp(attrNode, BAD_CAST "name", BAD_CAST attr.first.c_str());
            xmlNewChild(attrNode, NULL, BAD_CAST "gen:value", BAD_CAST attr.second.c_str());
        }
    }

    // date attrib
    xmlNodePtr dateNode = xmlNewChild(res, NULL, BAD_CAST "gen:dateAttribute", NULL);
    xmlNewProp(dateNode, BAD_CAST "name", BAD_CAST "date");
    xmlNewChild(dateNode, NULL, BAD_CAST "gen:value", BAD_CAST tag.m_date.toString().toStdString().c_str());

    return res;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportCityObjetXml(const citygml::CityObject& obj, xmlNodePtr parent, bool rootLevel)
{
    xmlNodePtr res = NULL;

    //std::cout << "type : " << obj.getTypeAsString() << std::endl;

    // temporal test
    if(m_temporalExport)
    {
        std::string strAttr = obj.getAttribute("yearOfConstruction");
        int yearOfConstruction = (strAttr.empty()?-1:std::stoi(strAttr));
        strAttr = obj.getAttribute("yearOfDemolition");
        int yearOfDemolition = (strAttr.empty()?-1:std::stoi(strAttr));

        int year = m_date.date().year();

        if(((yearOfConstruction == -1 || yearOfDemolition == -1) || (yearOfConstruction < year && year < yearOfDemolition)))
        {
            // keep node
        }
        else
        {
             return res;
        }

        // advanced temporal test

    }

    switch(obj.getType())
    {
    case citygml::COT_GenericCityObject:
        break;
    case citygml::COT_Building:
        res = exportCityObjetGenericXml(obj, "bldg:Building", parent);
        break;
    case citygml::COT_Room:
        res = exportCityObjetGenericXml(obj, "bldg:Room", parent);
        break;
    case citygml::COT_BuildingInstallation:
        res = exportCityObjetGenericXml(obj, "bldg:BuildingInstallation", parent);
        break;
    case citygml::COT_BuildingFurniture:
        res = exportCityObjetGenericXml(obj, "bldg:BuildingFurniture", parent);
        break;
    case citygml::COT_Door:
        res = exportCityObjetGenericXml(obj, "bldg:Door", parent);
        break;
    case citygml::COT_Window:
        res = exportCityObjetGenericXml(obj, "bldg:Window", parent);
        break;
    case citygml::COT_CityFurniture:
        res = exportCityObjetGenericXml(obj, "bldg:CityFurniture", parent);
        break;
    case citygml::COT_Track:
        res = exportCityObjetGenericXml(obj, "bldg:Track", parent);
        break;
    case citygml::COT_Road:
        res = exportCityObjetGenericXml(obj, "bldg:Road", parent);
        break;
    case citygml::COT_Railway:
        res = exportCityObjetGenericXml(obj, "bldg:Railway", parent);
        break;
    case citygml::COT_Square:
        res = exportCityObjetGenericXml(obj, "bldg:Square", parent);
        break;
    case citygml::COT_PlantCover:
        res = exportCityObjetGenericXml(obj, "bldg:PlantCover", parent);
        break;
    case citygml::COT_SolitaryVegetationObject:
        res = exportCityObjetGenericXml(obj, "bldg:SolitaryVegetationObject", parent);
        break;
    case citygml::COT_WaterBody:
        res = exportCityObjetGenericXml(obj, "bldg:WaterBody", parent);
        break;
    case citygml::COT_TINRelief:
        res = exportCityObjetGenericXml(obj, "bldg:TINRelief", parent);
        break;
    case citygml::COT_LandUse:
        res = exportCityObjetGenericXml(obj, "bldg:LandUse", parent);
        break;
    case citygml::COT_Tunnel:
        res = exportCityObjetGenericXml(obj, "bldg:Tunnel", parent);
        break;
    case citygml::COT_Bridge:
        res = exportCityObjetGenericXml(obj, "bldg:Bridge", parent);
        break;
    case citygml::COT_BridgeConstructionElement:
        res = exportCityObjetGenericXml(obj, "bldg:BridgeConstructionElement", parent);
        break;
    case citygml::COT_BridgeInstallation:
        res = exportCityObjetGenericXml(obj, "bldg:BridgeInstallation", parent);
        break;
    case citygml::COT_BridgePart:
        res = exportCityObjetGenericXml(obj, "bldg:BridgePart", parent);
        break;
    case citygml::COT_BuildingPart:
        res = exportCityObjetGenericXml(obj, "bldg:BuildingPart", parent);
        break;
    case citygml::COT_WallSurface:
        res = exportCityObjetGenericXml(obj, "bldg:WallSurface", parent);
        break;
    case citygml::COT_RoofSurface:
        res = exportCityObjetGenericXml(obj, "bldg:RoofSurface", parent);
        break;
    case citygml::COT_GroundSurface:
        res = exportCityObjetGenericXml(obj, "bldg:GroundSurface", parent);
        break;
    case citygml::COT_ClosureSurface:
        res = exportCityObjetGenericXml(obj, "bldg:ClosureSurface", parent);
        break;
    case citygml::COT_FloorSurface:
        res = exportCityObjetGenericXml(obj, "bldg:FloorSurface", parent);
        break;
    case citygml::COT_InteriorWallSurface:
        res = exportCityObjetGenericXml(obj, "bldg:InteriorWallSurface", parent);
        break;
    case citygml::COT_CeilingSurface:
        res = exportCityObjetGenericXml(obj, "bldg:CeilingSurface", parent);
        break;
     default:
        break;
    }

    if(!m_temporalExport) // export TAGs and STATEs
    {
        for(CityObjectState* state : obj.getStates())
        {
            xmlNodePtr r = exportCityObjetStateXml(*state, std::string("bldg:")+state->getParent()->getTypeAsString(), parent);

            if(state->getGeom())
            {
                for(const auto& geom : state->getGeom()->getGeometries())
                {
                    exportGeometryXml(*geom, r);
                }

                for(const auto& child : state->getGeom()->getChildren())
                {
                    exportCityObjetXml(*child, r);
                }
            }
        }
        for(CityObjectTag* tag : obj.getTags())
        {
            xmlNodePtr r = exportCityObjetTagXml(*tag, std::string("bldg:")+tag->getParent()->getTypeAsString(), parent);

            if(tag->getGeom())
            {
                for(const auto& geom : tag->getGeom()->getGeometries())
                {
                    exportGeometryXml(*geom, r);
                }

                for(const auto& child : tag->getGeom()->getChildren())
                {
                    exportCityObjetXml(*child, r);
                }
            }
        }
    }

    // build apperance node for current node
    if(rootLevel)
    {
        m_currentAppearence = xmlNewChild(res, NULL, BAD_CAST "app:appearance", NULL);
    }

    for(const auto& geom : obj.getGeometries())
    {
        if(res) exportGeometryXml(*geom, res);
        else exportGeometryXml(*geom, parent);
    }

    for(const auto& child : obj.getChildren())
    {
        if(res) exportCityObjetXml(*child, res);
        else exportCityObjetXml(*child, parent);
    }

    // clear apperance node for current node
    if(rootLevel)
    {
        m_currentAppearence = nullptr;
    }

    return res;
}
////////////////////////////////////////////////////////////////////////////////
std::string getWrapMode(Texture::WrapMode mode)
{
    switch(mode)
    {
        case Texture::WM_WRAP:
            return "wrap";
            break;
        case Texture::WM_MIRROR:
            return "mirror";
            break;
        case Texture::WM_CLAMP:
            return "clamp";
            break;
        case Texture::WM_BORDER:
            return "border";
            break;
        case Texture::WM_NONE:
            return "none";
            break;
    }
    return "none";
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportPolygonAppearanceXml(const citygml::Polygon& poly, xmlNodePtr parent)
{
    if(!parent || !poly.getTexture()) return nullptr;

    std::string buf;

    xmlNodePtr root = xmlNewChild(parent, NULL, BAD_CAST "app:Appearance", NULL);
    xmlNewChild(root, NULL, BAD_CAST "app:theme", BAD_CAST "texturation");
    xmlNodePtr srf = xmlNewChild(root, NULL, BAD_CAST "app:surfaceDataMember", NULL);
    xmlNodePtr tex = xmlNewChild(srf, NULL, BAD_CAST "app:ParameterizedTexture", NULL);
    xmlNewChild(tex, NULL, BAD_CAST "app:imageURI", BAD_CAST poly.getTexture()->getUrl().c_str());
    //xmlNewChild(tex, NULL, BAD_CAST "app:mimeType", BAD_CAST "image/tiff");
    xmlNewChild(tex, NULL, BAD_CAST "app:wrapMode", BAD_CAST getWrapMode(poly.getTexture()->getWrapMode()).c_str());
    xmlNodePtr target = xmlNewChild(tex, NULL, BAD_CAST "app:target", NULL);
    buf = "#"+poly.getId();
    xmlNewProp(target, BAD_CAST "uri", BAD_CAST buf.c_str());
    xmlNodePtr tcl = xmlNewChild(target, NULL, BAD_CAST "app:TexCoordList", NULL);
    buf="";
    for(const TVec2f& coord : poly.getTexCoords())
    {
        buf += std::to_string(coord.x) + ' ' + std::to_string(coord.y) + ' ';
    }
    xmlNodePtr tc = xmlNewChild(tcl, NULL, BAD_CAST "app:textureCoordinates", BAD_CAST buf.c_str());
    buf = "#"+poly.getExteriorRing()->getId();
    xmlNewProp(tc, BAD_CAST "ring", BAD_CAST buf.c_str());

    return root;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportCityModelXml(const citygml::CityModel& model)
{
    xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "core:CityModel");

    // write envelope (bouned by)
    xmlNodePtr nodeEnv = xmlNewChild(root, NULL, BAD_CAST "gml:boundedBy", NULL);
    exportEnvelopeXml(model.getEnvelope(), nodeEnv);

    // do objects
    for(const citygml::CityObject* obj : model.getCityObjectsRoots())
    {
        xmlNodePtr node = xmlNewChild(root, NULL, BAD_CAST "core:cityObjectMember", NULL);
        exportCityObjetXml(*obj, node, true);
    }

    return root;
}
////////////////////////////////////////////////////////////////////////////////
xmlNodePtr ExporterCityGML::exportCityObjectModelXml(const std::vector<const CityObject*>& objs)
{
    xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "core:CityModel");

    // write envelope (bouned by)
    xmlNodePtr node = xmlNewChild(root, NULL, BAD_CAST "gml:boundedBy", NULL);
    Envelope env;
    for(const CityObject* obj : objs) // compute enveloppe
    {
        env.merge(obj->getEnvelope());
    }
    exportEnvelopeXml(env, node);

    // do objects
    for(const CityObject* obj : objs)
    {
        xmlNodePtr node = xmlNewChild(root, NULL, BAD_CAST "core:cityObjectMember", NULL);
        exportCityObjetXml(*obj, node, true);
    }

    return root;
}
////////////////////////////////////////////////////////////////////////////////
} // namespace citygml
////////////////////////////////////////////////////////////////////////////////
