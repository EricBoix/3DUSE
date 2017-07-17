// Copyright University of Lyon, 2012 - 2017
// Distributed under the GNU Lesser General Public License Version 2.1 (LGPLv2)
// (Refer to accompanying file LICENSE.md or copy at
//  https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html )

#ifndef __EXPORT_HPP__
#define __EXPORT_HPP__

#include "data/ViewPoint.h"

#include <set>

/**
*	@brief % of element in a emblematic view, values are in % 
*/	
struct EmblematicView
{
public:
	float sky;
	float building;
	float remarquableBuilding;
	float vegetation;
	float water;
	float terrain;
	// In order to add a new data set, uncomment exemple and replace fillers <..> by your data
	// float <myData>;
};

class ExportParameter
{
public:
	EmblematicView emblematicView;

	inline static void SetGobalParameter(ExportParameter param) { globalParam = param; }
	inline static ExportParameter GetGlobalParameter() { return globalParam; }

private:
	static ExportParameter globalParam;
};

/**
*	@brief Get the color of the sky
*	@param d Direction of the ray
*	@param light Direction of the light
*/
TVec3d SkyShadeBlue(TVec3d d, TVec3d light);

/**
*	@brief Process the results of a ray tracing algorithm
*	@param viewpoint Data about the viewpoint we are rendering
*	@param result Result from the ray tracing algorithm
*	@param filePrefix prefix to add before a file name
*/
void ExportData(std::string dirTile, ViewPoint* viewpoint, std::string filePrefix = "");

/**
*	@brief Export images from a ray tracing result
*	@param viewpoint Data about the viewpoint we are rendering
*	@param result Result from the ray tracing algorithm
*/
void ExportImages(std::string dirTile, ViewPoint* viewpoint, std::string filePrefix = "");

/**
*	@brief Export image reprensenting the roof and wall of the buildings
*	@param viewpoint Data about the viewpoint we are rendering
*	@param result Result from the ray tracing algorithm
*/
void ExportImageRoofWall(std::string dirTile, ViewPoint* viewpoint, std::string filePrefix = "");

/**
*	@brief Export image reprensenting the zBuffer of the scene
*	@param viewpoint Data about the viewpoint we are rendering
*	@param result Result from the ray tracing algorithm
*/
void ExportImageZBuffer(std::string dirTile, ViewPoint* viewpoint, std::string filePrefix = "");

/**
*	@brief Export image reprensenting the remarquable building highlighted (the reste is in white
*	@param viewpoint Data about the viewpoint we are rendering
*	@param result Result from the ray tracing algorithm
*/
void ExportImageHighlightRemarquable(std::string dirTile, ViewPoint* viewpoint, std::string filePrefix = "");

/**
*	@brief Export image reprensenting the skyline
*	@param viewpoint Data about the viewpoint we are rendering
*	@param result Result from the ray tracing algorithm
*/
void ExportImageSkyline(std::string dirTile, ViewPoint* viewpoint, std::string filePrefix = "");

/**
*	@brief Export the data of the skyline of a panorama to a text file
*	@param front Front view of the panorama
*	@param right Right view of the panorama
*	@param back Back view of the panorama
*	@param left Left view of the panorama
*	@param filePrefix Prefix of the text file created
*/
void ExportPanoramaSkyline(std::string dirTile, ViewPoint* front, ViewPoint* right, ViewPoint* back, ViewPoint* left, std::string filePrefix = "");

/**
*	@brief Compute the skyline volume of a cascade panorama, currently really not good, must be research
*/
void ProcessSkylineVolume();

std::set<std::string> LoadRemarquableBuilding();

#endif
