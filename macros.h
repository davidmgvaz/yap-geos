/* Copyright (C) 2012 David Vaz <davidvaz@dcc.fc.up.pt>
 *
 * Thanks to Pedro Pereira for the initial help in 2006
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General  Public  License
 * as published by the Free Software Foundation; either  version
 * 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in  the  hope  that  it  will  be
 * useful, but WITHOUT ANY WARRANTY; without  even  the  implied
 * warranty of  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of  the  GNU  General  Public
 * License along with this program; if not, write  to  the  Free
 * Software Foundation, Inc., 51 Franklin Street,  Fifth  Floor,
 * Boston, MA 02110-1301, USA.
 */
#define BINARY_FUNCTION(procedure_name,procedure) \
YAP_Bool c_##procedure_name (void) \
{ \
  return (binary (YAP_ARG1, YAP_ARG2, procedure, YAP_ARG3)); \
}

BINARY_FUNCTION(intersection,GEOSIntersection)
BINARY_FUNCTION(difference,GEOSDifference)
BINARY_FUNCTION(symdifference,GEOSSymDifference)
BINARY_FUNCTION(union,GEOSUnion)

#define BINARY_BOOL_FUNCTION(procedure_name,procedure) \
YAP_Bool c_##procedure_name (void) \
{ \
  return (binary_bool (YAP_ARG1, YAP_ARG2, procedure)); \
}

BINARY_BOOL_FUNCTION (disjoint, GEOSDisjoint)
BINARY_BOOL_FUNCTION (touches, GEOSTouches)
BINARY_BOOL_FUNCTION (intersects, GEOSIntersects)
BINARY_BOOL_FUNCTION (crosses, GEOSCrosses)
BINARY_BOOL_FUNCTION (within, GEOSWithin)
BINARY_BOOL_FUNCTION (contains, GEOSContains)
BINARY_BOOL_FUNCTION (overlaps, GEOSOverlaps)
BINARY_BOOL_FUNCTION (equals, GEOSEquals)

#define UNARY_FUNCTION(procedure_name,procedure) \
YAP_Bool c_##procedure_name (void) \
{ \
  return (unary (YAP_ARG1, procedure, YAP_ARG2)); \
}

#if GEOS_VERSION_MAJOR == 3
UNARY_FUNCTION (envelope, GEOSEnvelope)
#endif
UNARY_FUNCTION (convex_hull, GEOSConvexHull)
UNARY_FUNCTION (boundary, GEOSBoundary)
UNARY_FUNCTION (point_on_surface, GEOSPointOnSurface)
UNARY_FUNCTION (centroid, GEOSGetCentroid)

#define UNARY_BOOL_FUNCTION(procedure_name,procedure) \
YAP_Bool c_##procedure_name (void) \
{ \
  return (unary_bool (YAP_ARG1, procedure)); \
}

UNARY_BOOL_FUNCTION(is_empty,GEOSisEmpty)
UNARY_BOOL_FUNCTION(is_simple,GEOSisSimple)
UNARY_BOOL_FUNCTION(is_ring,GEOSisRing)

#define PREFIX "geos_"
#define USER_C_CALLBACK(procedure_name,arity) \
  YAP_UserCPredicate (PREFIX #procedure_name, c_##procedure_name, arity)
