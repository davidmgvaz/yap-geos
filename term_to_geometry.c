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

YAP_Bool term_to_geometry (YAP_Term term, geometry_t *geometry)
{
  YAP_Functor functor;
  const char * functor_name;
  unsigned int n, arity, size;

  assert (geometry != NULL);
  *geometry = NULL;
  if (YAP_IsApplTerm (term) == FALSE)
    return (FALSE);

  functor = YAP_FunctorOfTerm (term);
  functor_name = YAP_AtomName (YAP_NameOfFunctor (functor));
  arity = YAP_ArityOfFunctor (functor);

  size = sizeof (translate) / sizeof (struct type_name_arity);
  for (n = 0; n < size; n ++)
    if (strcmp (translate[n].name, functor_name) == 0)
      {
        if (translate[n].arity == arity)
          return (translate[n].procedure_import (term, geometry));
        break;
      }

  return (FALSE);
}

YAP_Bool point_to_geometry (YAP_Term term, geometry_t *geometry)
{
  sequence_t sequence;
  YAP_Float x, y;
  YAP_Functor functor;
  const char * functor_name;
  unsigned int arity;

  assert (geometry != NULL);
  if (YAP_IsApplTerm (term) == FALSE)
    return (FALSE);

  functor = YAP_FunctorOfTerm (term);
  functor_name = YAP_AtomName (YAP_NameOfFunctor (functor));
  arity = YAP_ArityOfFunctor (functor);
  if ((strcmp (functor_name, NAME_POINT) != 0) || (arity != 2))
    return (FALSE);

  if ((Yap_IsNumberTerm (YAP_ArgOfTerm (1, term), &x) == FALSE)
      || (Yap_IsNumberTerm (YAP_ArgOfTerm (2, term), &y) == FALSE))
    return (FALSE);

  sequence = GEOSCoordSeq_create (1, 2);
  if (sequence == NULL)
    return (FALSE);
  if ((GEOSCoordSeq_setX (sequence, 0, x) == 0)
      || (GEOSCoordSeq_setY (sequence, 0, y) == 0))
    {
      GEOSCoordSeq_destroy (sequence);
      return (FALSE);
    }
  *geometry = GEOSGeom_createPoint (sequence);
  if (*geometry == NULL)
    return (FALSE);

  return (TRUE);
}

static YAP_Bool point_list_to_sequence (YAP_Term term,
                                          unsigned int size,
                                          sequence_t *sequence)
{
  YAP_Float x, y;
  unsigned int n;
  YAP_Term head;

  *sequence = GEOSCoordSeq_create (size, 2);
  if (*sequence == NULL)
    return (FALSE);
  for (n = 0; YAP_IsPairTerm (term) != FALSE; n ++)
    {
      assert (n < size);
      head = YAP_HeadOfTerm (term);
      if ((Yap_IsNumberTerm (YAP_ArgOfTerm (1, head), &x) == FALSE)
          || (Yap_IsNumberTerm (YAP_ArgOfTerm (2, head), &y) == FALSE)
          || (GEOSCoordSeq_setX (*sequence, n, x) == 0)
          || (GEOSCoordSeq_setY (*sequence, n, y) == 0))
        {
          GEOSCoordSeq_destroy (*sequence);
          return (FALSE);
        }
      term = YAP_TailOfTerm (term);
    }
  assert (n == size);
  assert (YAP_IsAtomTerm (term) != FALSE);
  assert (strcmp (YAP_AtomName (YAP_AtomOfTerm (term)), "[]") == 0);
  return (TRUE);
}

/* Check if a prolog `term' is a well formed list and optionally
   return it's size if the variable `size' is not NULL. */
static YAP_Bool is_list_get_size (YAP_Term term, unsigned int *size)
{
  unsigned int n;
  for (n = 0; YAP_IsPairTerm (term) != FALSE; n ++)
    term = YAP_TailOfTerm (term);
  if ((YAP_IsAtomTerm (term) == FALSE)
      || (strcmp (YAP_AtomName (YAP_AtomOfTerm (term)), "[]") != 0))
    return (FALSE);
  if (size != NULL)
    *size = n;
  return (TRUE);
}

typedef YAP_Bool (*procedure_to_geometry_t) (YAP_Term, geometry_t *);

static YAP_Bool list_to_geometry (YAP_Term term,
                                    unsigned int minimum,
                                    procedure_to_geometry_t procedure,
                                    geometry_type_t geometry_type,
                                    geometry_t *geometry)
{
  geometry_t *p;
  unsigned int size;
  unsigned int v, n;
  YAP_Term head;

  assert (geometry != NULL);
  term = YAP_ArgOfTerm (1, term);
  if ((is_list_get_size (term, &size) == FALSE) || (size < minimum))
    return (FALSE);

  p = (geometry_t *) malloc (sizeof (geometry_t) * size);
  if (p == NULL)
    {
      warning ("%s: list_to_geometry: not enough memory", __FILE__);
      return (FALSE);
    }
  memset (p, 0, sizeof (geometry_t) * size);
  for (n = 0; YAP_IsPairTerm (term) != FALSE; n ++)
    {
      assert (n < size);
      head = YAP_HeadOfTerm (term);
      if (procedure (head, &p[n]) == FALSE)
        {
          for (v = 0; v < n; v ++)
            GEOSGeom_destroy (p[v]);
          free (p);
          return (FALSE);
        }
      term = YAP_TailOfTerm (term);
    }
  assert (n == size);
  assert (YAP_IsAtomTerm (term) != FALSE);
  assert (strcmp (YAP_AtomName (YAP_AtomOfTerm (term)), "[]") == 0);
  if (geometry_type == GEOS_POLYGON)
    *geometry = GEOSGeom_createPolygon (p[0], p + 1, size - 1);
  else
    *geometry = GEOSGeom_createCollection (geometry_type, p, size);
  memset (p, 0, sizeof (geometry_t) * size);
  free (p);

  if (*geometry == NULL)
    return (FALSE);
  return (TRUE);
}

static YAP_Bool make_point_to_geometry (YAP_Term term,
                                          geometry_t *geometry)
{
  YAP_Functor functor;
  const char * functor_name;
  unsigned int arity;
  YAP_Term p[2];
  if (YAP_IsApplTerm (term) == FALSE)
    return (FALSE);
  functor = YAP_FunctorOfTerm (term);
  functor_name = YAP_AtomName (YAP_NameOfFunctor (functor));
  arity = YAP_ArityOfFunctor (functor);
  if ((strcmp (functor_name, ",") != 0) || (arity != 2))
    return (FALSE);
  p[0] = YAP_ArgOfTerm (1, term);
  p[1] = YAP_ArgOfTerm (2, term);
  functor = YAP_MkFunctor (YAP_LookupAtom (NAME_POINT), 2);
  term = YAP_MkApplTerm (functor, 2, p);
  return (point_to_geometry (term, geometry));
}

static YAP_Bool line_to_geometry (YAP_Term term,
                                    geometry_type_t type,
                                    geometry_t *geometry)
{
  sequence_t sequence;
  unsigned int size;

  assert ((type == GEOS_LINESTRING) || (type == GEOS_LINEARRING));
  assert (geometry != NULL);
  term = YAP_ArgOfTerm (1, term);
  if ((is_list_get_size (term, &size) == FALSE)
      || ((type == GEOS_LINESTRING) && (size < 2))
      || ((type == GEOS_LINEARRING) && (size < 4)))
    return (FALSE);
  if (point_list_to_sequence (term, size, &sequence) == FALSE)
    return (FALSE);
  if (type == GEOS_LINESTRING)
    *geometry = GEOSGeom_createLineString (sequence);
  else
    *geometry = GEOSGeom_createLinearRing (sequence);
  if (*geometry == NULL)
    return (FALSE);

  return (TRUE);
}

YAP_Bool linestring_to_geometry (YAP_Term term,
                                   geometry_t *geometry)
{
  return (line_to_geometry (term, GEOS_LINESTRING, geometry));
}

static YAP_Bool linearring_to_geometry (YAP_Term term,
                                          geometry_t *geometry)
{
  return (line_to_geometry (term, GEOS_LINEARRING, geometry));
}

static YAP_Bool make_linestring_to_geometry (YAP_Term term,
                                               geometry_t *geometry)
{
  YAP_Functor functor;
  functor = YAP_MkFunctor (YAP_LookupAtom (","), 1);
  term = YAP_MkApplTerm (functor, 1, &term);
  return (linestring_to_geometry (term, geometry));
}

static YAP_Bool make_linearring_to_geometry (YAP_Term term,
                                               geometry_t *geometry)
{
  YAP_Functor functor;
  functor = YAP_MkFunctor (YAP_LookupAtom (","), 1);
  term = YAP_MkApplTerm (functor, 1, &term);
  return (linearring_to_geometry (term, geometry));
}

YAP_Bool polygon_to_geometry (YAP_Term term,
                                geometry_t *geometry)
{
  return (list_to_geometry (term, 1, make_linearring_to_geometry,
                            GEOS_POLYGON, geometry));
}

static YAP_Bool make_polygon_to_geometry (YAP_Term term,
                                            geometry_t *geometry)
{
  YAP_Functor functor;
  functor = YAP_MkFunctor (YAP_LookupAtom (","), 1);
  term = YAP_MkApplTerm (functor, 1, &term);
  return (polygon_to_geometry (term, geometry));
}

YAP_Bool multipoint_to_geometry (YAP_Term term,
                                   geometry_t *geometry)
{
  return (list_to_geometry (term, 1, make_point_to_geometry,
                            GEOS_MULTIPOINT, geometry));
}

YAP_Bool multilinestring_to_geometry (YAP_Term term,
                                        geometry_t *geometry)
{
  return (list_to_geometry (term, 1, make_linestring_to_geometry,
                            GEOS_MULTILINESTRING, geometry));
}

YAP_Bool multipolygon_to_geometry (YAP_Term term,
                                     geometry_t *geometry)
{
  return (list_to_geometry (term, 1, make_polygon_to_geometry,
                            GEOS_MULTIPOLYGON, geometry));
}

YAP_Bool geometrycollection_to_geometry (YAP_Term term,
                                           geometry_t *geometry)
{
  return (list_to_geometry (term, 0, term_to_geometry,
                            GEOS_GEOMETRYCOLLECTION, geometry));
}
