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

/* Prepend URL to the singly-linked list if a duplicate is not found. */
GSList *lutil_slist_prepend_if_unique(GSList *l, const gchar *url)
{
  GSList *curr = l;
  while (curr != NULL) /* Check for a duplicate. */
    {
      if (g_strcmp0((const gchar*) curr->data, url) ==0)
        return (l);
      curr = g_slist_next(curr);
    }
  return (g_slist_prepend(l, g_strdup(url)));
}

void lutil_slist_free_full(GSList *l, GFunc f)
{
#ifdef HAVE_GLIB_2_28
  g_slist_free_full(l, f);
#else
  g_slist_foreach(l, f, NULL);
  g_slist_free(l);
#endif
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
