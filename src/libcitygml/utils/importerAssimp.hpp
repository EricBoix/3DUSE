// Copyright University of Lyon, 2012 - 2017
// Distributed under the GNU Lesser General Public License Version 2.1 (LGPLv2)
// (Refer to accompanying file LICENSE.md or copy at
//  https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html )

////////////////////////////////////////////////////////////////////////////////
#ifndef __CITYGML_ASSIMPIMPORT_HPP__
#define __CITYGML_ASSIMPIMPORT_HPP__

#include <assimp/ai_assert.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

#include "libcitygml/import/importer.hpp"
#include "libcitygml/citygml.hpp"
#include "citygmlutils_export.h"
#include <QString>

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
class CITYGMLUTILS_EXPORT ImporterAssimp : public Importer
{
public:
      ImporterAssimp();
      virtual ~ImporterAssimp() override;

      /// Read filename with assimp and converts it to CityGML
      /// \param fileName Input file path
      CityModel* import(const std::string& fileName, bool detectRoof = true, QString Prefix = "");

   private:
      /// Interal method : CityGML converter
      CityModel* assimpSceneToCityGML(const struct aiScene* aiScene, QString Prefix = "");

      /// Internal method : recursive method doing the conversion
      void assimpNodeToCityGML(const struct aiScene* aiScene, const struct aiNode* aiNode, CityObject* parent, QString Prefix = "");

      bool _detectRoof;

      CityModel* m_model; ///< Result of import
   };
   ////////////////////////////////////////////////////////////////////////////////
} // namespace citygml

#endif // __CITYGML_ASSIMPIMPORT_HPP__
