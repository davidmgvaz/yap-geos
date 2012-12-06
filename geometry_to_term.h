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
#ifndef MODULE_EXPORT
#define MODULE_EXPORT

YAP_Bool point_to_term (geometry_t, YAP_Term *);
YAP_Bool linestring_to_term (geometry_t, YAP_Term *);
YAP_Bool polygon_to_term (geometry_t, YAP_Term *);
YAP_Bool multipoint_to_term (geometry_t, YAP_Term *);
YAP_Bool multilinestring_to_term (geometry_t, YAP_Term *);
YAP_Bool multipolygon_to_term (geometry_t, YAP_Term *);
YAP_Bool geometrycollection_to_term (geometry_t, YAP_Term *);

#endif
