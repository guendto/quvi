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

#include <glib/gi18n.h>
#include <quvi.h>

#include "lutil.h"

static const lutil_cb_printerr perr = lutil_print_stderr_unless_quiet;

static void _fetch(const quvi_word type, const gpointer p)
{
  if (type != QUVI_CALLBACK_STATUS_DONE)
    perr(_("fetch <url>: "));
  else
    perr(_("done\n"));
}

static void _resolve(const quvi_word type)
{
  if (type != QUVI_CALLBACK_STATUS_DONE)
    perr(_("resolve <url>: "));
  else
    perr(_("done\n"));
}

static void _query_metainfo(const quvi_word type)
{
  if (type != QUVI_CALLBACK_STATUS_DONE)
    perr(_("metainfo <url>: "));
  else
    perr(_("done\n"));
}

QuviError cb_status(glong param, gpointer p)
{
  quvi_word status, type;

  status = quvi_loword(param);
  type = quvi_hiword(param);

  switch (status)
    {
    case QUVI_CALLBACK_STATUS_HTTP_QUERY_METAINFO:
      _query_metainfo(type);
      break;
    case QUVI_CALLBACK_STATUS_RESOLVE:
      _resolve(type);
      break;
    case QUVI_CALLBACK_STATUS_FETCH:
      _fetch(type, p);
      break;
    default:
      g_warning("[%s] unknown quvi status code 0x%x", __func__, status);
      break;
    }
  return (QUVI_OK);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
