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

#include <stdlib.h>
#include <glib/gi18n.h>

#include "linput.h"
#include "lutil.h"

gint lutil_parse_input(gpointer p, const gchar **rargs)
{
  linput_t l = (linput_t) p;

  if (linput_new(l, rargs) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (g_slist_length(l->url.input) ==0)
    {
      g_printerr(_("error: no input URL\n"));
      return (EXIT_FAILURE);
    }
  return (EXIT_SUCCESS);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
