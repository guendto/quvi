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
#include <quvi.h>

#include "lutil.h"

#define _s(n) #n

/* Query metainfo for a HTTP(S) stream. */
gint lutil_query_metainfo(quvi_t q, quvi_media_t qm,
                          quvi_http_metainfo_t *qmi,
                          lutil_cb_printerr xperr)
{
  gchar *m_url;
  gint r;

  g_assert(xperr != NULL);
  g_assert(qm != NULL);
  g_assert(qmi != NULL);
  g_assert(q != NULL);

  quvi_media_get(qm, QUVI_MEDIA_STREAM_PROPERTY_URL, &m_url);

  r = lutil_chk_property_ok(q, UTIL_PROPERTY_TYPE_MEDIA,
                            _s(QUVI_MEDIA_STREAM_PROPERTY_URL), xperr);
  if (r != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  *qmi = quvi_http_metainfo_new(q, m_url);
  if (quvi_ok(q) == QUVI_FALSE)
    {
      xperr(_("libquvi: while querying content meta-info: %s"),
            quvi_errmsg(q));

      r = EXIT_FAILURE;
    }
  return (r);
}

#undef _s

/* vim: set ts=2 sw=2 tw=72 expandtab: */
