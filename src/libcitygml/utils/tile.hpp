// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#ifndef __TILE_HPP__
#define __TILE_HPP__

#include <memory>
#include <map>
#include "citygml.hpp"
#include "URI.hpp"
#include "citygmlutils_export.h"

#ifdef _MSC_VER
#pragma warning(disable: 4251) // VC++ DLL jejune complains on STL _Id member
#endif

namespace vcity
{

/// \brief The Tile class
/// Holds citygml, shp data
class CITYGMLUTILS_EXPORT Tile
{
public:
  Tile();

  /// Build a tile and load a CityGML file
  /// \param filepath CityGML file path
  Tile(const std::string& filepath);
  //Tile(const TVec3d& pMin, const TVec3d& pMax);
  ~Tile();

  /// Get tile enveloppe
  citygml::Envelope& getEnvelope();
  const citygml::Envelope& getEnvelope() const;

  /// Compute envoloppe
  void computeEnvelope();

  void load(const std::string& filepath);
  //osg::ref_ptr<osg::Node> buildOsgData();

  citygml::CityModel* getCityModel();
  const citygml::CityModel* getCityModel() const;

  /// Get tile name
  const std::string& getName() const;

  /// Set tile name
  void setName(const std::string& name);
  //osg::ref_ptr<osg::Node> getOsgRoot();

  //citygml::CityObject* findNode(const std::string& name);

  void deleteNode(const std::string& name);
  void insertNode(citygml::CityObject* node);
  void replaceNode(const std::string& name, citygml::CityObject* node);

  /// \brief getNode Get a CityGML node
  /// \param uri URI pointing to the CityGML node
  /// \return Ptr to CityGML node or nullptr
  citygml::CityObject* getNode(const URI& uri, bool inPickingMode = false);

  /// Get the path of the CityGML file loaded
  const std::string& getCityGMLfilePath() const;

private:
  std::string m_name;             ///< tile name
  std::string m_citygmlFilePath;  ///< Path of the CityGML file loaded

  citygml::CityModel* m_root; // TODO : remove
};

typedef std::shared_ptr<Tile> TilePtr;

} // namespace vity

#endif // __TILE_HPP__
