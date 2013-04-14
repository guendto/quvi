/* quvi
 * Copyright (C) 2012,2013  Toni Gundogdu <legatvs@gmail.com>
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
#include <quvi.h>

#include "lutil.h"

static const gchar *_property_type[] =
{
  N_("Subtitle language"),
  N_("Subtitle type"),
  N_("HTTP metainfo"),
  N_("playlist"),
  N_("script"),
  N_("media"),
  NULL
};

/* Check whether a media property could be retrieved. */
gint lutil_chk_property_ok(quvi_t q, const lutilPropertyType t,
                           const gchar *s, lutil_cb_printerr xperr)
{
  gint r;

  g_assert(xperr != NULL);
  g_assert(q != NULL);
  g_assert(s != NULL);

  r = (quvi_ok(q) == QUVI_FALSE) ? EXIT_FAILURE:EXIT_SUCCESS;
  if (r == EXIT_FAILURE)
    {
      xperr(_("libquvi: while trying to retrieve a %s property `%s': %s"),
            g_dgettext(GETTEXT_PACKAGE, _property_type[t]),
            s, quvi_errmsg(q));
    }
  return (r);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
