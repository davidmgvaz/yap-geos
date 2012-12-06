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
#ifndef MODULE_COMMON
#define MODULE_COMMON

#include "geos_yap.h"

#define NAME_POINT "point"
#define NAME_LINESTRING "linestring"
#define NAME_POLYGON "polygon"
#define NAME_MULTIPOINT "multipoint"
#define NAME_MULTILINESTRING "multilinestring"
#define NAME_MULTIPOLYGON "multipolygon"
#define NAME_GEOMETRYCOLLECTION "geometrycollection"

typedef YAP_Bool (*procedure_term_to_geometry_t) (YAP_Term, geometry_t *);
typedef YAP_Bool (*procedure_geometry_to_term_t) (geometry_t, YAP_Term *);

#include "term_to_geometry.h"
#include "geometry_to_term.h"

struct type_name_arity
{
  const char * name;
  unsigned int arity;
  geometry_type_t geos_type;
  procedure_term_to_geometry_t procedure_import;
  procedure_geometry_to_term_t procedure_export;
};

static struct type_name_arity translate[] =
  {
    { NAME_POINT, 2, GEOS_POINT,
      point_to_geometry, point_to_term },
    { NAME_LINESTRING, 1, GEOS_LINESTRING,
      linestring_to_geometry, linestring_to_term },
    { NAME_POLYGON, 1, GEOS_POLYGON,
      polygon_to_geometry, polygon_to_term },
    { NAME_MULTIPOINT, 1, GEOS_MULTIPOINT,
      multipoint_to_geometry, multipoint_to_term },
    { NAME_MULTILINESTRING, 1, GEOS_MULTILINESTRING,
      multilinestring_to_geometry, multilinestring_to_term },
    { NAME_MULTIPOLYGON, 1, GEOS_MULTIPOLYGON,
      multipolygon_to_geometry, multipolygon_to_term },
    { NAME_GEOMETRYCOLLECTION, 1, GEOS_GEOMETRYCOLLECTION,
      geometrycollection_to_geometry, geometrycollection_to_term }
  };

#endif
