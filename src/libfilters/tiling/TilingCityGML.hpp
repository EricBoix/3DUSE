// -*-c++-*- VCity project, 3DUSE, Liris, 2013, 2014, 2015
////////////////////////////////////////////////////////////////////////////////
#ifndef __TILINGCITYGML_HPP__
#define __TILINGCITYGML_HPP__

#include <vector>
#include <string>
#ifdef _MSC_VER                // Inhibit dll-interface warnings concerning
# pragma warning(disable:4251) // gdal-1.11.4 internals (cpl_string.h) when
#endif                         // including ogrsf_frmts.h on VCC++
#include <ogrsf_frmts.h>
#include <QDir>

#include "core/tile.hpp"
#include "export/exportCityGML.hpp"

citygml::CityModel* TileCityGML( vcity::Tile* Tile,
                                 std::vector<TextureCityGML*>* TexturesList,
                                 TVec2d MinTile,
                                 TVec2d MaxTile,
                                 std::string PathFolder );

void MergingTile( vcity::Tile* OldTile,
                  citygml::CityModel* NewTile,
                  std::vector<TextureCityGML*>* TexturesList );

citygml::CityModel* CreateBoundingBox( std::string dir );

#endif // __LINKCITYGMLSHAPE_HPP__
