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
#include "lget.h"

gint lget_new(lget_t g)
{
  gchar *s;
  gint r;

  g_assert(g != NULL);
  g_assert(g->build_fpath != NULL);
  g_assert(g->xperr != NULL);
  g_assert(g->url == NULL);
  g_assert(g->qm != NULL);
  g_assert(g->q != NULL);

  if (g->opts.stream != NULL)
    {
      r = lutil_choose_stream(g->q, g->qm, g->opts.stream, g->xperr);
      if (r != EXIT_SUCCESS)
        return (EXIT_FAILURE);
    }
  quvi_media_get(g->qm, QUVI_MEDIA_STREAM_PROPERTY_URL, &g->url);

  s = g_uri_parse_scheme(g->url);
  r = EXIT_FAILURE;

  if (g_strcmp0(s, "http") ==0 || g_strcmp0(s, "https") ==0)
    r = lget_http_get(g);
  else
    {
      if (s == NULL)
        g->xperr(_("could not parse the URI scheme from `%s'"), g->url);
      else
        g->xperr(_("protocol `%s' is not supported"), s);
    }
  g_free(s);
  return (r);
}

void lget_free(lget_t g)
{
  if (g == NULL)
    return;

  g_free(g->result.fpath);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
