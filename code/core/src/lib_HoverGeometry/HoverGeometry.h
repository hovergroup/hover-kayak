/*
 * lib_HoverGeometry
 *        File: HoverGeometry.h
 *  Created on: Apr 14, 2014
 *      Author: Josh Leighton
 */

#ifndef LIB_HOVERGEOMETRY_H_
#define LIB_HOVERGEOMETRY_H_

#include "geometry.pb.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"
#include <string>

namespace HoverGeometry {

bool parsePoint(std::string view_point, VIEW_POINT & proto);
std::string printPoint(const VIEW_POINT & proto);

bool parseMarker(std::string view_marker, VIEW_MARKER & proto);
std::string printMarker(const VIEW_MARKER & proto);

bool parseSeglist(std::string view_seglist, VIEW_SEGLIST & proto);
std::string printSeglist(const VIEW_SEGLIST & proto);

bool parsePolygon(std::string view_poly, VIEW_POLYGON & proto);
std::string printPolygon(const VIEW_POLYGON & proto);

};

#endif // LIB_HOVERGEOMETRY_H_
