// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014
////////////////////////////////////////////////////////////////////////////////
#ifndef __CITYGML_ASSIMPIMPORT_HPP__
#define __CITYGML_ASSIMPIMPORT_HPP__
////////////////////////////////////////////////////////////////////////////////
#include "importer.hpp"
#include "../citygml.hpp"

#include <assimp/ai_assert.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
////////////////////////////////////////////////////////////////////////////////
namespace citygml
{
////////////////////////////////////////////////////////////////////////////////
/// \brief CityGML import using assimp
///
/// example of import obj file then export to CityGML :
/// \code{.cpp}
/// citygml::ImporterAssimp importer;
/// importer.setOffset(offsetx, offsety);
/// citygml::CityModel* model = importer.import("file.obj");
/// citygml::ExporterCityGML exporter("file.gml");
/// exporter.exportCityModel(*model);
/// \endcode
class ImporterAssimp : public Importer
{
public:
    ImporterAssimp();
    virtual ~ImporterAssimp() override;

    /// Read filename with assimp and converts it to CityGML
    /// \param fileName Input file path
    CityModel* import(const std::string& fileName);

private:
    /// Interal method : CityGML converter
    CityModel* assimpSceneToCityGML(const struct aiScene* aiScene);

    /// Internal method : recursive method doing the conversion
    void assimpNodeToCityGML(const struct aiScene* aiScene, const struct aiNode* aiNode, CityObject* parent);

    CityModel* m_model; ///< Result of import
};
////////////////////////////////////////////////////////////////////////////////
} // namespace citygml
////////////////////////////////////////////////////////////////////////////////
#endif // __CITYGML_ASSIMPIMPORT_HPP__
