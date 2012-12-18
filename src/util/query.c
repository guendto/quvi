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

void lutil_query_playlist(gpointer p, gpointer userdata)
{
  lutil_query_properties_t qps;
  quvi_playlist_t qp;

  qps = (lutil_query_properties_t) userdata;
  if (qps->exit_status != EXIT_SUCCESS)
    return;

  qp = quvi_playlist_new(qps->q, (const gchar*) p);
  if (quvi_ok(qps->q) == QUVI_TRUE)
    qps->activity(qps, qp, p);
  else
    {
      qps->xperr(_("libquvi: while parsing playlist properties: %s"),
                 quvi_errmsg(qps->q));

      qps->exit_status = EXIT_FAILURE;
    }
  quvi_playlist_free(qp);
}

void lutil_query_media(gpointer p, gpointer userdata)
{
  lutil_query_properties_t qps;
  quvi_media_t qm;

  qps = (lutil_query_properties_t) userdata;
  if (qps->exit_status != EXIT_SUCCESS)
    return;

  qm = quvi_media_new(qps->q, p);
  if (quvi_ok(qps->q) == QUVI_TRUE)
    qps->activity(qps, qm, p);
  else
    {
      qps->xperr(_("libquvi: while parsing media properties: %s"),
                 quvi_errmsg(qps->q));

      qps->exit_status = EXIT_FAILURE;
    }
  quvi_media_free(qm);
}

void lutil_query_subtitle(gpointer p, gpointer userdata)
{
  lutil_query_properties_t qps;
  quvi_subtitle_t qsub;

  qps = (lutil_query_properties_t) userdata;
  if (qps->exit_status != EXIT_SUCCESS)
    return;

  qsub = quvi_subtitle_new(qps->q, p);
  if (quvi_ok(qps->q) == QUVI_TRUE)
    qps->activity(qps, qsub, p);
  else
    {
      qps->xperr(_("libquvi: while querying subtitle properties: %s"),
                 quvi_errmsg(qps->q));
      qps->exit_status = EXIT_FAILURE;
    }
  quvi_subtitle_free(qsub);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
