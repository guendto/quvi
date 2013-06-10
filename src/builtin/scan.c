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

#include "lprint.h"
#include "linput.h"
#include "lopts.h"
#include "lutil.h"
#include "setup.h"
#include "opts.h"
#include "sig.h"
#include "cmd.h"

static gint exit_status = EXIT_SUCCESS;
static lutil_cb_printerr xperr = NULL;
static GSList *media_urls = NULL;
static quvi_t q = NULL;

static struct sigaction saw, sao;
static struct linput_s linput;
static struct lopts_s lopts;
extern struct opts_s opts;

static void _foreach_scan_url(gpointer p, gpointer userdata)
{
  lprint_cb_scan_print_buffer scan_print_buffer;
  lprint_cb_scan_properties   scan_properties;
  lprint_cb_scan_free         scan_free;
  lprint_cb_scan_new          scan_new;
  quvi_scan_t qs;

  if (exit_status != EXIT_SUCCESS)
    return;

  /* default. */

  scan_print_buffer = lprint_rfc2483_scan_print_buffer;
  scan_properties   = lprint_rfc2483_scan_properties;
  scan_free         = lprint_rfc2483_scan_free;
  scan_new          = lprint_rfc2483_scan_new;

  if (g_strcmp0(opts.core.print_format, "enum") ==0)
    {
      scan_print_buffer = lprint_enum_scan_print_buffer;
      scan_properties   = lprint_enum_scan_properties;
      scan_free         = lprint_enum_scan_free;
      scan_new          = lprint_enum_scan_new;
    }

#ifdef HAVE_JSON_GLIB
  if (g_strcmp0(opts.core.print_format, "json") ==0)
    {
      scan_print_buffer = lprint_json_scan_print_buffer;
      scan_properties   = lprint_json_scan_properties;
      scan_free         = lprint_json_scan_free;
      scan_new          = lprint_json_scan_new;
    }
#endif

#ifdef HAVE_LIBXML
  if (g_strcmp0(opts.core.print_format, "xml") ==0)
    {
      scan_print_buffer = lprint_xml_scan_print_buffer;
      scan_properties   = lprint_xml_scan_properties;
      scan_free         = lprint_xml_scan_free;
      scan_new          = lprint_xml_scan_new;
    }
#endif

  qs = quvi_scan_new(q, (const gchar*) p);
  if (quvi_ok(q) == QUVI_FALSE)
    {
      xperr(_("libquvi: while scanning: %s"), quvi_errmsg(q));
      exit_status = EXIT_FAILURE;
    }
  else
    {
      gpointer h;

      exit_status = scan_new(q, &h);
      if (exit_status == EXIT_SUCCESS)
        {
          exit_status = scan_properties(qs, h);
          if (exit_status == EXIT_SUCCESS)
            exit_status = scan_print_buffer(h);
        }
      scan_free(h);
    }
  quvi_scan_free(qs);
}

static gint _cleanup()
{
  lutil_slist_free_full(media_urls, (GFunc) g_free);
  media_urls = NULL;

  sigwinch_reset(&sao);
  linput_free(&linput);
  quvi_free(q);

  return (exit_status);
}

gint cmd_scan(gint argc, gchar **argv)
{
  if (setup_opts(argc, argv, &lopts) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (lutil_parse_input(&linput, (const gchar**) opts.rargs) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (setup_quvi(&q) != EXIT_SUCCESS)
    {
      linput_free(&linput);
      return (EXIT_FAILURE);
    }

  sigwinch_setup(&saw, &sao);

  xperr = lprint_enum_errmsg; /* rfc2483 uses this also. */

#ifdef HAVE_JSON_GLIB
  if (g_strcmp0(opts.core.print_format, "json") ==0)
    xperr = lprint_json_errmsg;
#endif
#ifdef HAVE_LIBXML
  if (g_strcmp0(opts.core.print_format, "xml") ==0)
    xperr = lprint_xml_errmsg;
#endif

  g_slist_foreach(linput.url.input, _foreach_scan_url, NULL);
  return (_cleanup());
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
