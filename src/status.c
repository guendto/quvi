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

#include <glib/gi18n.h>
#include <quvi.h>

#include "lutil.h"
#include "sig.h"

static const lutil_cb_printerr perr = lutil_print_stderr_unless_quiet;
static gint curr_frame = 0;

static const gchar *frames[] =
{
  "o---",
  "-o--",
  "--o-",
  "---o",
  NULL
};

static void _saymsg(const gchar *m)
{
  gsize l, i;
  gchar *s;

  if (strlen(m) >0)
    {
      if (frames[curr_frame] == NULL)
        curr_frame = 0;
      s = g_strdup_printf(C_("Status update with {curr_anim_frame, message}",
                             "status: %s %s"), frames[curr_frame++], m);
    }
  else
    s = g_strdup(m);

  l = sigwinch_term_spaceleft(strlen(s));
  perr("%s", s);
  g_free(s);

  for (i=0; i<l; ++i)
    perr(" ");
  perr("\r");
}

static void _say(const gchar *m)
{
  _saymsg(m);
}

static void _done()
{
  _saymsg("");
}

static void _fetch(const quvi_word type, const gpointer data)
{
  if (type != QUVI_CALLBACK_STATUS_DONE)
    _say(_("fetch <url> ..."));
  else
    _done();
}

static void _resolve(const quvi_word type)
{
  if (type != QUVI_CALLBACK_STATUS_DONE)
    _say(_("resolve <url> ..."));
  else
    _done();
}

static void _query_metainfo(const quvi_word type)
{
  if (type != QUVI_CALLBACK_STATUS_DONE)
    _say(_("metainfo <url> ..."));
  else
    _done();
}

QuviError cb_status(glong status_type, gpointer data, gpointer user_data)
{
  quvi_word status, type;

  status = quvi_loword(status_type);
  type = quvi_hiword(status_type);

  switch (status)
    {
    case QUVI_CALLBACK_STATUS_HTTP_QUERY_METAINFO:
      _query_metainfo(type);
      break;
    case QUVI_CALLBACK_STATUS_RESOLVE:
      _resolve(type);
      break;
    case QUVI_CALLBACK_STATUS_FETCH:
      _fetch(type, data);
      break;
    default:
      g_warning("[%s] unknown quvi status code 0x%x", __func__, status);
      break;
    }
  return (QUVI_OK);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
