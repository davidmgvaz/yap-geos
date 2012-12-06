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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yap.h"
#include "geos_yap.h"
#include "common.h"

YAP_Bool geometry_to_term (geometry_t geometry, YAP_Term *term)
{
  geometry_type_t type;
  unsigned int n, size;

  assert (term != NULL);
  type = GEOSGeomTypeId (geometry);
  size = sizeof (translate) / sizeof (struct type_name_arity);
  for (n = 0; n < size; n ++)
    if (translate[n].geos_type == type)
      return (translate[n].procedure_export (geometry, term));

  return (FALSE);
}

static YAP_Bool make_point (YAP_Float x,
                              YAP_Float y,
                              const char * functor_name,
                              YAP_Term *term)
{
  YAP_Term p[2];
  YAP_Functor functor;

  if (functor_name == NULL)
    functor = YAP_MkFunctor (YAP_LookupAtom (","), 2);
  else
    functor = YAP_MkFunctor (YAP_LookupAtom (functor_name), 2);
  p[0] = YAP_MkFloatTerm (x);
  p[1] = YAP_MkFloatTerm (y);
  *term = YAP_MkApplTerm (functor, 2, p);
  return (TRUE);
}

static YAP_Bool point_list_to_term (sequence_t sequence, YAP_Term *term)
{
  int n;
  unsigned int size;
  YAP_Float x, y;
  YAP_Term head;

  assert (term != NULL);
  *term = YAP_MkAtomTerm (YAP_LookupAtom ("[]"));
  if (GEOSCoordSeq_getSize (sequence, &size) == 0)
    return (FALSE);
  for (n = size - 1; n >= 0; n --)
    {
      if ((GEOSCoordSeq_getX (sequence, n, &x) == 0)
          || (GEOSCoordSeq_getY (sequence, n, &y) == 0))
        return (FALSE);
      if (make_point (x, y, NULL, &head) == FALSE)
        return (FALSE);
      *term = YAP_MkPairTerm (head, *term);
    }
  return (TRUE);
}

static YAP_Bool make_point_to_term (geometry_t geometry,
                                      const char * functor_name,
                                      YAP_Term *term)
{
  double x, y;
  sequence_t sequence;

  assert (term != NULL);
  sequence = (sequence_t) GEOSGeom_getCoordSeq (geometry);
  if ((sequence == NULL)
      || (GEOSCoordSeq_getX (sequence, 0, &x) == 0)
      || (GEOSCoordSeq_getY (sequence, 0, &y) == 0))
    return (FALSE);
  if (make_point (x, y, functor_name, term) == FALSE)
    return (FALSE);
  return (TRUE);
}

static YAP_Bool make_linestring_to_term (geometry_t geometry,
                                           const char * functor_name,
                                           YAP_Term *term)
{
  sequence_t sequence;
  YAP_Functor functor;

  assert (term != NULL);
  sequence = (sequence_t) GEOSGeom_getCoordSeq (geometry);
  if ((sequence == NULL)
      || (point_list_to_term (sequence, term) == FALSE))
    return (FALSE);
  if (functor_name != NULL)
    {
      functor = YAP_MkFunctor (YAP_LookupAtom (functor_name), 1);
      *term = YAP_MkApplTerm (functor, 1, term);
    }
  return (TRUE);
}

static YAP_Bool make_polygon_to_term (geometry_t geometry,
                                        const char * functor_name,
                                        YAP_Term *term)
{
  int n;
  geometry_t ring;
  sequence_t sequence;
  YAP_Functor functor;
  unsigned int size;
  YAP_Term head;

  assert (term != NULL);
  *term = YAP_MkAtomTerm (YAP_LookupAtom ("[]"));
  size = GEOSGetNumInteriorRings (geometry);
  for (n = size - 1; n >= 0; n --)
    {
      ring = (geometry_t) GEOSGetInteriorRingN (geometry, n);
      sequence = (sequence_t) GEOSGeom_getCoordSeq (ring);
      if ((sequence == NULL)
          || (point_list_to_term (sequence, &head) == FALSE))
        return (FALSE);
      *term = YAP_MkPairTerm (head, *term);
    }
  /* Exterior ring always exists. */
  ring = (geometry_t) GEOSGetExteriorRing (geometry);
  if (ring == NULL)
    return (FALSE);
  sequence = (sequence_t) GEOSGeom_getCoordSeq (ring);
  if ((sequence == NULL)
      || (point_list_to_term (sequence, &head) == FALSE))
    return (FALSE);
  *term = YAP_MkPairTerm (head, *term);

  if (functor_name != NULL)
    {
      functor = YAP_MkFunctor (YAP_LookupAtom (NAME_POLYGON), 1);
      *term = YAP_MkApplTerm (functor, 1, term);
    }
  return (TRUE);
}

typedef YAP_Bool (*procedure_to_term_t) (geometry_t,
                                           const char *,
                                           YAP_Term *);

static YAP_Bool make_multi_to_term (geometry_t geometry,
                                      procedure_to_term_t procedure,
                                      const char * functor_name,
                                      YAP_Term *term)
{
  int n;
  geometry_t geometry_n;
  YAP_Functor functor;
  unsigned int size;
  YAP_Term list, head;

  assert (term != NULL);
  list = YAP_MkAtomTerm (YAP_LookupAtom ("[]"));
  size = GEOSGetNumGeometries (geometry);
  for (n = size - 1; n >= 0; n --)
    {
      geometry_n = (geometry_t) GEOSGetGeometryN (geometry, n);
      if (procedure (geometry_n, NULL, &head) == FALSE)
        return (FALSE);
      list = YAP_MkPairTerm (head, list);
    }
  functor = YAP_MkFunctor (YAP_LookupAtom (functor_name), 1);
  *term = YAP_MkApplTerm (functor, 1, &list);
  return (TRUE);
}

YAP_Bool geometrycollection_to_term (geometry_t geometry, YAP_Term *term)
{
  int n;
  geometry_t geometry_n;
  YAP_Functor functor;
  unsigned int size;
  YAP_Term head;

  assert (term != NULL);
  *term = YAP_MkAtomTerm (YAP_LookupAtom ("[]"));
  size = GEOSGetNumGeometries (geometry);
  for (n = size - 1; n >= 0; n --)
    {
      geometry_n = (geometry_t) GEOSGetGeometryN (geometry, n);
      if (geometry_to_term (geometry_n, &head) == FALSE)
        return (FALSE);
      *term = YAP_MkPairTerm (head, *term);
    }
  functor = YAP_MkFunctor (YAP_LookupAtom (NAME_GEOMETRYCOLLECTION), 1);
  *term = YAP_MkApplTerm (functor, 1, term);
  return (TRUE);
}

YAP_Bool point_to_term (geometry_t geometry, YAP_Term *term)
{
  return (make_point_to_term (geometry, NAME_POINT, term));
}

YAP_Bool linestring_to_term (geometry_t geometry, YAP_Term *term)
{
  return (make_linestring_to_term (geometry, NAME_LINESTRING, term));
}

YAP_Bool polygon_to_term (geometry_t geometry, YAP_Term *term)
{
  return (make_polygon_to_term (geometry, NAME_POLYGON, term));
}

YAP_Bool multipoint_to_term (geometry_t geometry, YAP_Term *term)
{
  return (make_multi_to_term (geometry, make_point_to_term,
                              NAME_MULTIPOINT, term));
}

YAP_Bool multilinestring_to_term (geometry_t geometry, YAP_Term *term)
{
  return (make_multi_to_term (geometry, make_linestring_to_term,
                              NAME_MULTILINESTRING, term));
}

YAP_Bool multipolygon_to_term (geometry_t geometry, YAP_Term *term)
{
  return (make_multi_to_term (geometry, make_polygon_to_term,
                              NAME_MULTIPOLYGON, term));
}
