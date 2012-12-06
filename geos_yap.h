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
#ifndef MODULE_GEOS
#define MODULE_GEOS

/* #include <stdarg.h> */
#include <geos_c.h>

typedef GEOSCoordSeq sequence_t;
typedef GEOSGeom geometry_t;
#if GEOS_VERSION_MAJOR == 3
typedef enum GEOSGeomTypes geometry_type_t;
#else
typedef enum GEOSGeomTypeId geometry_type_t;
#endif

void warning (const char *p, ...);

#include "yap.h"
YAP_Bool term_to_geometry (YAP_Term, geometry_t *);
YAP_Bool geometry_to_term (geometry_t, YAP_Term *);

#endif
