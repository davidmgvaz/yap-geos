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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "yap.h"
#include "geos_yap.h"

void warning (const char *p, ...)
{
  va_list n;
  va_start (n, p);
  fprintf (stderr, "warning: ");
  vfprintf (stderr, p, n);
  fprintf (stderr, "\n");
  fflush (stderr);
  va_end (n);
}

/* Since the `intersection', `difference', `symdifference' and `union'
 * are very similar (binary functions) they handled with the same
 * procedure.
 */
#if GEOS_VERSION_MAJOR == 3
typedef geometry_t (*binary_procedure_t) (const GEOSGeometry *,
                                          const GEOSGeometry *);
#else
typedef geometry_t (*binary_procedure_t) (geometry_t, geometry_t);
#endif

YAP_Bool binary (YAP_Term geometry_1_term,
                   YAP_Term geometry_2_term,
                   binary_procedure_t procedure,
                   YAP_Term geometry_out_term)
{
  geometry_t geometry_1, geometry_2, geometry_out;
  YAP_Term term;

  if (term_to_geometry (geometry_1_term, &geometry_1) == FALSE)
    return (FALSE);
  if (term_to_geometry (geometry_2_term, &geometry_2) == FALSE)
    {
      GEOSGeom_destroy (geometry_1);
      return (FALSE);
    }
  geometry_out = procedure (geometry_1, geometry_2);
  GEOSGeom_destroy (geometry_1);
  GEOSGeom_destroy (geometry_2);
  if (geometry_out == NULL)
    return (FALSE);
  if (geometry_to_term (geometry_out, &term) == FALSE)
    {
      GEOSGeom_destroy (geometry_out);
      return (FALSE);
    }

  GEOSGeom_destroy (geometry_out);
  return (YAP_Unify (geometry_out_term, term));
}

/* Binary boolean relations. */
#if GEOS_VERSION_MAJOR == 3
typedef char (*binary_bool_procedure_t) (const GEOSGeometry *,
                                         const GEOSGeometry *);
#else
typedef char (*binary_bool_procedure_t) (geometry_t, geometry_t);
#endif

YAP_Bool binary_bool (YAP_Term geometry_1_term,
                        YAP_Term geometry_2_term,
                        binary_bool_procedure_t procedure)
{
  char v;
  geometry_t geometry_1, geometry_2;

  if (term_to_geometry (geometry_1_term, &geometry_1) == FALSE)
    return (FALSE);
  if (term_to_geometry (geometry_2_term, &geometry_2) == FALSE)
    {
      GEOSGeom_destroy (geometry_1);
      return (FALSE);
    }
  v = procedure (geometry_1, geometry_2);
  GEOSGeom_destroy (geometry_1);
  GEOSGeom_destroy (geometry_2);

  if (v == 1)
    return (TRUE);
  else if (v == 2)
    warning ("exception");
  return (FALSE);
}

/* Unary relations. */
#if GEOS_VERSION_MAJOR == 3
typedef geometry_t (*unary_procedure_t) (const GEOSGeometry *);
#else
typedef geometry_t (*unary_procedure_t) (geometry_t);
#endif

YAP_Bool unary (YAP_Term geometry_term,
                  unary_procedure_t procedure,
                  YAP_Term geometry_out_term)
{
  geometry_t geometry, geometry_out;
  YAP_Term term;

  if (term_to_geometry (geometry_term, &geometry) == FALSE)
    return (FALSE);
  geometry_out = procedure (geometry);
  GEOSGeom_destroy (geometry);
  if (geometry_out == NULL)
    return (FALSE);
  if (geometry_to_term (geometry_out, &term) == FALSE)
    {
      GEOSGeom_destroy (geometry_out);
      return (FALSE);
    }

  GEOSGeom_destroy (geometry_out);
  return (YAP_Unify (geometry_out_term, term));
}

/* Unary boolean relations. */
#if GEOS_VERSION_MAJOR == 3
typedef char (*unary_bool_procedure_t) (const GEOSGeometry *);
#else
typedef char (*unary_bool_procedure_t) (geometry_t);
#endif

YAP_Bool unary_bool (YAP_Term geometry_term,
                       unary_bool_procedure_t procedure)
{
  char v;
  geometry_t geometry;

  if (term_to_geometry (geometry_term, &geometry) == FALSE)
    return (FALSE);
  v = procedure (geometry);
  GEOSGeom_destroy (geometry);

  if (v == 1)
    return (TRUE);
  else if (v == 2)
    warning ("exception");
  return (FALSE);
}

#define DEFAULT_QUADS 4
YAP_Bool c_buffer (void)
{
  geometry_t geometry, buffer;
  YAP_Float width;
  YAP_Term term;

  /* Convert the second argument first since it is faster if it has
     the wrong type and because it is not needed to destroy the
     geometry_t if something goes wrong. */
  if (Yap_IsNumberTerm (YAP_ARG2, &width) == FALSE)
    return (FALSE);
  if (term_to_geometry (YAP_ARG1, &geometry) == FALSE)
    return (FALSE);
  buffer = GEOSBuffer (geometry, width, DEFAULT_QUADS);
  GEOSGeom_destroy (geometry);
  if (buffer == NULL)
    return (FALSE);
  if (geometry_to_term (buffer, &term) == FALSE)
    {
      GEOSGeom_destroy (buffer);
      return (FALSE);
    }

  GEOSGeom_destroy (buffer);
  return (YAP_Unify (YAP_ARG3, term));
}

YAP_Bool c_equals_exact (void)
{
  char v;
  geometry_t geometry1, geometry2;
  YAP_Float tolerance;

  if (Yap_IsNumberTerm (YAP_ARG3, &tolerance) == FALSE)
    return (FALSE);
  if (term_to_geometry (YAP_ARG1, &geometry1) == FALSE)
    return (FALSE);
  if (term_to_geometry (YAP_ARG2, &geometry2) == FALSE)
  {
	  GEOSGeom_destroy (geometry1);
	  return (FALSE);
  }
  v = GEOSEqualsExact (geometry1, geometry2, tolerance);
  GEOSGeom_destroy (geometry1);
  GEOSGeom_destroy (geometry2);

  if (v == 1)
    return (TRUE);
  else if (v == 2)
    warning ("exception");
  return (FALSE);
}

#if GEOS_VERSION_MAJOR == 3
YAP_Bool c_area (void)
{
  geometry_t geometry;
  YAP_Float area;

  if (term_to_geometry (YAP_ARG1, &geometry) == FALSE)
    return (FALSE);
  if (GEOSArea (geometry, &area) == 0)
    {
      GEOSGeom_destroy (geometry);
      return (FALSE);
    }

  GEOSGeom_destroy (geometry);
  return (YAP_Unify (YAP_ARG2, YAP_MkFloatTerm (area)));
}

YAP_Bool c_length (void)
{
  geometry_t geometry;
  YAP_Float length;

  if (term_to_geometry (YAP_ARG1, &geometry) == FALSE)
    return (FALSE);
  if (GEOSLength (geometry, &length) == 0)
    {
      GEOSGeom_destroy (geometry);
      return (FALSE);
    }

  GEOSGeom_destroy (geometry);
  return (YAP_Unify (YAP_ARG2, YAP_MkFloatTerm (length)));
}
#endif

YAP_Bool c_distance (void)
{
  geometry_t geometry_1, geometry_2;
  YAP_Float distance;

  if (term_to_geometry (YAP_ARG1, &geometry_1) == FALSE)
    return (FALSE);
  if (term_to_geometry (YAP_ARG2, &geometry_2) == FALSE)
    {
      GEOSGeom_destroy (geometry_1);
      return (FALSE);
    }
  if (GEOSDistance (geometry_1, geometry_2, &distance) == 0)
    {
      GEOSGeom_destroy (geometry_1);
      GEOSGeom_destroy (geometry_2);
      return (FALSE);
    }

  GEOSGeom_destroy (geometry_1);
  GEOSGeom_destroy (geometry_2);
  return (YAP_Unify (YAP_ARG3, YAP_MkFloatTerm (distance)));
}

YAP_Bool matrix_compatible (char *given, char *computed)
{
  while ((*given != '\0') && (*computed != '\0'))
    {
      switch (*given)
        {
          case 'T':
            switch (*computed)
              {
                case '0':
                case '1':
                case '2':
                  break;
                default:
                  return (FALSE);
              }
            break;
          case '*':
            break;
          case 'F':
          case '0':
          case '1':
          case '2':
            if (*computed != *given)
              return (FALSE);
            break;
          default:
            return (FALSE);
        }
      given ++;
      computed ++;
    }
  if ((*given == '\0') && (*computed == '\0'))
    return (TRUE);
  return (FALSE);
}

YAP_Bool c_relate (void)
{
  geometry_t geometry_1, geometry_2;
  char *matrix,*matrix_3;
  YAP_Term matrix_term;
  YAP_Bool t;

  if (term_to_geometry (YAP_ARG1, &geometry_1) == FALSE)
    return (FALSE);
  if (term_to_geometry (YAP_ARG2, &geometry_2) == FALSE)
    {
      GEOSGeom_destroy (geometry_1);
      return (FALSE);
    }
  matrix = GEOSRelate (geometry_1, geometry_2);
  GEOSGeom_destroy (geometry_1);
  GEOSGeom_destroy (geometry_2);

  if (YAP_IsVarTerm (YAP_ARG3) == TRUE)
    {
      matrix_term = YAP_MkAtomTerm (YAP_LookupAtom (matrix));
      free (matrix);
      return (YAP_Unify (matrix_term, YAP_ARG3));
    }
  if (YAP_IsAtomTerm (YAP_ARG3) == TRUE)
    {
      matrix_3 = (char *) YAP_AtomName (YAP_AtomOfTerm (YAP_ARG3));
      t = matrix_compatible (matrix_3, matrix);
      free (matrix);
      return (t);
    }

  free (matrix);
  return (FALSE);
}

YAP_Bool c_geometry_to_wkt (void)
{
  geometry_t geometry;
  char *wkt;

  if (term_to_geometry (YAP_ARG1, &geometry) == FALSE)
    return (FALSE);

  wkt = GEOSGeomToWKT(geometry);
  GEOSGeom_destroy (geometry);
  if (wkt == NULL)
    return (FALSE);
 
/*   if (YAP_Unify(YAP_ARG2, YAP_MkAtomTerm (YAP_LookupAtom (wkt)))) */
/*     { */
/*       free(wkt); */
/*       return (TRUE); */
/*     } */
  printf("%s",wkt);
  free(wkt);
  return (TRUE);
}

YAP_Bool c_test (void)
{
  geometry_t geometry;
  YAP_Term term;

  if (term_to_geometry (YAP_ARG1, &geometry) == FALSE)
    return (FALSE);
  if (geometry_to_term (geometry, &term) == FALSE)
    {
      GEOSGeom_destroy (geometry);
      return (FALSE);
    }
  GEOSGeom_destroy (geometry);
  return (YAP_Unify (YAP_ARG2, term));
}

#include "macros.h"

inline void geos_yap_halt (int exit, void* stuff)
{
	finishGEOS();
}

void geos_yap_init (void)
{
  initGEOS (warning, warning);
  YAP_HaltRegisterHook (geos_yap_halt, NULL);

  /* For debug proposes only. */
  USER_C_CALLBACK (test, 2);

  /* Binary geometry. */
  USER_C_CALLBACK (intersection, 3);
  USER_C_CALLBACK (difference, 3);
  USER_C_CALLBACK (symdifference, 3);
  USER_C_CALLBACK (union, 3);

  /* Binary boolean. */
  USER_C_CALLBACK (disjoint, 2);
  USER_C_CALLBACK (touches, 2);
  USER_C_CALLBACK (intersects, 2);
  USER_C_CALLBACK (crosses, 2);
  USER_C_CALLBACK (within, 2);
  USER_C_CALLBACK (contains, 2);
  USER_C_CALLBACK (overlaps, 2);
  USER_C_CALLBACK (equals, 2);

  /* Unary geometry. */
#if GEOS_VERSION_MAJOR == 3
  USER_C_CALLBACK (envelope, 2);
#endif
  USER_C_CALLBACK (convex_hull, 2);
  USER_C_CALLBACK (boundary, 2);
  USER_C_CALLBACK (point_on_surface, 2);
  USER_C_CALLBACK (centroid, 2);
  USER_C_CALLBACK (union_cascaded, 2);


  /* Unary boolean. */
  USER_C_CALLBACK (is_empty, 1);
  USER_C_CALLBACK (is_simple, 1);
  USER_C_CALLBACK (is_ring, 1);

  USER_C_CALLBACK (buffer, 3);
  USER_C_CALLBACK (equals_exact, 3);
  USER_C_CALLBACK (convex_hull, 2);

#if GEOS_VERSION_MAJOR == 3
  USER_C_CALLBACK (area, 2);
  USER_C_CALLBACK (length, 2);
#endif
  USER_C_CALLBACK (distance, 3);

  USER_C_CALLBACK (relate, 3);

  /*utility functions*/
  USER_C_CALLBACK (geometry_to_wkt, 1);
}
