/*
 * lib_HoverGeometry
 *        File: HoverGeometry.cpp
 *  Created on: Apr 14, 2014
 *      Author: Josh Leighton
 */

#include "HoverGeometry.h"

#include "XYFormatUtilsSegl.h"
#include "XYFormatUtilsPoly.h"
#include "XYFormatUtilsPoint.h"
#include "XYFormatUtilsCircle.h"
#include "XYFormatUtilsVector.h"
#include "XYFormatUtilsRangePulse.h"
#include "XYFormatUtilsCommsPulse.h"
#include "XYFormatUtilsMarker.h"
#include "XYFormatUtilsConvexGrid.h"

using namespace HoverGeometry;
using namespace std;

bool HoverGeometry::parsePoint(string view_point, VIEW_POINT & proto) {
    XYPoint point = string2Point(view_point);
    if (!point.valid())
        return false;
    proto.set_x(point.get_vx());
    proto.set_y(point.get_vy());
    proto.set_label(point.get_label());
    if (point.get_vz() != 0)
        proto.set_z(point.get_vz());
    if (!point.get_msg().empty())
        proto.set_msg(point.get_msg());
    if (!point.active())
        proto.set_active(point.active());
    if (point.get_color("label").set())
        proto.set_label_color(point.get_color("label").str());
    if (point.get_color("vertex").set())
        proto.set_vertex_color(point.get_color("vertex").str());
    if (point.vertex_size_set())
        proto.set_vertex_size(point.get_vertex_size());

    return true;
}

string HoverGeometry::printPoint(const VIEW_POINT & proto) {
    XYPoint point;
    point.set_vx(proto.x());
    point.set_vy(proto.y());
    if (proto.has_z())
        point.set_vz(proto.z());
    point.set_label(proto.label());
    if (proto.has_msg())
        point.set_msg(proto.msg());
    if (proto.has_active())
        point.set_active(proto.active());
    if (proto.has_label_color())
        point.set_color("label", proto.label_color());
    if (proto.has_vertex_color())
        point.set_color("vertex", proto.vertex_color());
    if (proto.has_vertex_size())
        point.set_vertex_size(proto.vertex_size());

    return point.get_spec();
}

bool HoverGeometry::parseMarker(string view_marker, VIEW_MARKER & proto) {
    XYMarker mark = string2Marker(view_marker);
    if (!mark.valid())
        return false;

    proto.set_x(mark.get_vx());
    proto.set_y(mark.get_vy());
    proto.set_label(mark.get_label());
    if (!mark.get_msg().empty()) {
        proto.set_msg(mark.get_msg());
    }
    if (mark.get_color("primary_color").set()) {
        proto.set_primary_color(mark.get_color("primary_color").str());
    }
    if (mark.get_color("secondary_color").set()) {
        proto.set_secondary_color(mark.get_color("secondary_color").str());
    }

    string shape = MOOSToUpper(mark.get_type());
    if (shape == "SQUARE") {
        proto.set_shape(SQUARE);
    } else if (shape == "TRIANGLE") {
        proto.set_shape(TRIANGLE);
    } else if (shape == "DIAMOND") {
        proto.set_shape(DIAMOND);
    } else if (shape == "CIRCLE") {
        proto.set_shape(CIRCLE);
    } else if (shape == "GATEWAY") {
        proto.set_shape(GATEWAY);
    } else if (shape == "EFIELD") {
        proto.set_shape(EFIELD);
    }

    if (mark.get_width() != 0) {
        proto.set_width(mark.get_width());
    }

    if (!mark.active())
        proto.set_active(mark.active());

    return true;
}

string HoverGeometry::printMarker(const VIEW_MARKER & proto) {
    XYMarker mark;
    mark.set_vx(proto.x());
    mark.set_vy(proto.y());
    mark.set_label(proto.label());
    if (proto.has_msg()) {
        mark.set_msg(proto.msg());
    }
    if (proto.has_primary_color()) {
        mark.set_color("primary_color", proto.primary_color());
    }
    if (proto.has_secondary_color()) {
        mark.set_color("secondary_color", proto.secondary_color());
    }

    if (proto.has_shape()) {
        switch (proto.shape()) {
        case SQUARE:
            mark.set_type("square");
            break;
        case TRIANGLE:
            mark.set_type("triangle");
            break;
        case CIRCLE:
            mark.set_type("circle");
            break;
        case DIAMOND:
            mark.set_type("diamond");
            break;
        case GATEWAY:
            mark.set_type("gateway");
            break;
        case EFIELD:
            mark.set_type("efield");
            break;
        }
    }

    if (proto.has_width()) {
        mark.set_width(proto.width());
    }

    if (proto.has_active())
        mark.set_active(proto.active());
    return mark.get_spec();
}

bool HoverGeometry::parseSeglist(string view_seglist, VIEW_SEGLIST &proto) {
    XYSegList seg = string2SegList(view_seglist);
    if (!seg.valid())
        return false;

    for (int i = 0; i < seg.size(); i++) {
        proto.add_x(seg.get_vx(i));
        proto.add_y(seg.get_vy(i));
        proto.add_z(seg.get_vz(i));
    }

    proto.set_label(seg.get_label());
    if (!seg.get_msg().empty()) {
        proto.set_msg(seg.get_msg());
    }
    proto.set_vertex_size(seg.get_vertex_size());
    proto.set_edge_size(seg.get_edge_size());

    if (seg.get_color("edge").set()) {
        proto.set_edge_color(seg.get_color("edge").str());
    }
    if (seg.get_color("vertex").set()) {
        proto.set_vertex_color(seg.get_color("vertex").str());
    }

    if (!seg.active())
        proto.set_active(seg.active());

    return true;
}

string HoverGeometry::printSeglist(const VIEW_SEGLIST & proto) {
    XYSegList seg;
    for (int i = 0; i < proto.x_size(); i++) {
        seg.add_vertex(proto.x(i), proto.y(i), proto.z(i));
    }
    seg.set_label(proto.label());
    if (proto.has_msg()) {
        seg.set_msg(proto.msg());
    }
    if (proto.has_vertex_size()) {
        seg.set_vertex_size(proto.vertex_size());
    }
    if (proto.has_edge_size()) {
        seg.set_edge_size(proto.edge_size());
    }
    if (proto.has_edge_color()) {
        seg.set_color("edge", proto.edge_color());
    }
    if (proto.has_vertex_color()) {
        seg.set_color("vertex", proto.vertex_color());
    }
    if (proto.has_active())
        seg.set_active(proto.active());
    return seg.get_spec();
}

bool HoverGeometry::parsePolygon(string view_poly, VIEW_POLYGON & proto) {
    XYPolygon poly = string2Poly(view_poly);
    if (!poly.valid())
        return false;

    proto.set_label(poly.get_label());
    proto.set_view_poly(view_poly);
    return true;
}

string HoverGeometry::printPolygon(const VIEW_POLYGON & proto) {
    return proto.view_poly();
}
