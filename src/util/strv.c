/* quvi
 * Copyright (C) 2012  Toni Gundogdu <legatvs@gmail.com>
 *
 * This file is part of quvi <http://quvi.sourceforge.net/>.
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General
 * Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib.h>
#include "lutil.h"

/* Check if a string-array contains the specified string. */
gboolean lutil_strv_contains(const gchar **sv, const gchar *s)
{
  gint i;

  if (sv == NULL)
    return (FALSE);

  for (i=0; sv[i] != NULL; ++i)
    {
      if (g_strcmp0(sv[i], s) == 0)
        return (TRUE);
    }
  return (FALSE);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
