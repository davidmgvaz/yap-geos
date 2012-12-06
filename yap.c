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
#include <stdlib.h>

#include "yap.h"

YAP_Bool Yap_IsNumberTerm (YAP_Term term, YAP_Float *n)
{
  if (YAP_IsIntTerm (term) != FALSE)
    {
      if (n != NULL)
        *n = (YAP_Float) YAP_IntOfTerm (term);
      return (TRUE);
    }
  if (YAP_IsFloatTerm (term) != FALSE)
    {
      if (n != NULL)
        *n = YAP_FloatOfTerm (term);
      return (TRUE);
    }
  return (FALSE);
}
