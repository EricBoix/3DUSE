#ifndef __TEMPORALEXT_H__
#define __TEMPORALEXT_H__
////////////////////////////////////////////////////////////////////////////////
#include <QDateTime>
#include <map>
#include <string>
#include <osg/Group>
////////////////////////////////////////////////////////////////////////////////
namespace citygml
{
class CityObject;

class BuildingTag
{
public:
    BuildingTag(int year, CityObject* geom);

    int getId() const;
    //std::string getStringId() const { std::stringstream ss; ss << "TAG" << m_id << m_name; return ""; /*ss.str();*/ }
    std::string getStringId() const;
    //std::string getGeomName() const { std::string a = m_geom?m_geom->getId():0; return getStringId()+a; }
    CityObject* getGeom();
    osg::ref_ptr<osg::Group> getOsg();
    void setOsg(osg::ref_ptr<osg::Group> node);

    virtual const std::string& getAttribute(const std::string& attribName, QDateTime date) const;

    int m_year;
    QDateTime m_date;
    int m_id;
    std::string m_name;
    CityObject* m_parent;

private:

    CityObject* m_geom;
    osg::ref_ptr<osg::Group> m_osg;
    //int m_year;
};
////////////////////////////////////////////////////////////////////////////////
class BuildingFlag
{
public:
    BuildingFlag(CityObject* geom);

    int getId() const;
    //std::string getStringId() const { std::stringstream ss; ss << "FLAG" << m_id << m_name; return "";/*ss.str();*/ }
    std::string getStringId() const;
    //std::string getGeomName() const { std::string a = m_geom?m_geom->getId():0; return getStringId()+a; }
    CityObject* getGeom();

    virtual const std::string& getAttribute(const std::string& attribName, QDateTime date) const;

    std::map<std::string, std::string> m_attributes;    ///< attributes map

    int m_id;
    std::string m_name;
    CityObject* m_parent;

private:

    CityObject* m_geom;
};
////////////////////////////////////////////////////////////////////////////////
class DataSource
{
public:
    DataSource(const std::string& attributeName);
    virtual ~DataSource() {}

    virtual const std::string& getAttribute(const QDateTime& date) const;


   void dump() const;

//protected:
    std::string m_attribute;

    std::vector<QDateTime> m_dates;
    std::vector<std::string> m_values;
};
////////////////////////////////////////////////////////////////////////////////
class DataSourceArray : public DataSource
{
public:
    DataSourceArray(const std::string& attributeName, const std::string& data);
    //virtual const std::string& getAttribute(const QDateTime& date) const;
    void parse(const std::string& data);
};
////////////////////////////////////////////////////////////////////////////////
class DataSourceFile : public DataSource
{
public:
    DataSourceFile(const std::string& attributeName, const std::string& filePath);
    //virtual const std::string& getAttribute(const QDateTime& date) const;
    void parse(const std::string& filePath);

    std::string m_filePath;
};
////////////////////////////////////////////////////////////////////////////////
class BuildingDynFlag : public BuildingFlag
{
public:
    BuildingDynFlag(CityObject* geom);
    virtual ~BuildingDynFlag() {}

    void addDataSource(DataSource* dsrc);

    virtual const std::string& getAttribute(const std::string& attribName, const QDateTime& date) const;

private:
    std::map<std::string, DataSource*> m_attributes;    ///< attributes map
};
////////////////////////////////////////////////////////////////////////////////
} // namespace citygml
////////////////////////////////////////////////////////////////////////////////
#endif // __TEMPORALEXT_H__
