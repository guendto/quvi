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
#include <curl/curl.h>

#include "lutil.h"

gint lutil_quvi_init(quvi_t *q, lutil_net_opts_t o)
{
  *q = quvi_new();
  if (quvi_ok(*q) == QUVI_FALSE)
    {
      g_printerr(_("error: while initializing libquvi: %s\n"),
                 quvi_errmsg(*q));
      return (EXIT_FAILURE);
    }

  if (o != NULL)
    {
      CURL *c = lutil_curl_handle_from(*q);
      if (c == NULL)
        {
          g_printerr(_("error: failed to retrieve the current curl "
                       "session handle from libquvi\n"));
          return (EXIT_FAILURE);
        }
      curl_easy_setopt(c, CURLOPT_VERBOSE, o->verbose);

      curl_easy_setopt(c, CURLOPT_MAX_RECV_SPEED_LARGE,
                       (curl_off_t) o->throttle_ki_s * 1024);
    }
  return (EXIT_SUCCESS);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
