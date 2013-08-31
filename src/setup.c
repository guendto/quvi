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
#include <string.h>
#include <glib.h>
#include <quvi.h>

#include "linput.h"
#include "lutil.h"
#include "lopts.h"
#include "setup.h"
#include "opts.h"

extern QuviError cb_status(glong, gpointer, gpointer);
extern struct opts_s opts;

gint setup_opts(gint argc, gchar **argv, lopts_t lopts)
{
  memset(lopts, 0, sizeof(struct lopts_s));

  lopts->cb.set_post_parse_defaults = cb_set_post_parse_defaults;
  lopts->cb.cmdline_validate_values = cb_cmdline_validate_values;
  lopts->cb.parse_keyfile_values = cb_parse_keyfile_values;
  lopts->cb.get_config_fpath = cb_get_config_fpath;

  lopts->entries = option_entries;
  lopts->argc = argc;
  lopts->argv = argv;

  if (lopts_new(lopts) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (g_strcmp0(opts.core.verbosity, "debug") ==0)
    lopts_print_config_values(lopts);

  return (EXIT_SUCCESS);
}

gint setup_quvi(quvi_t *q)
{
  struct lutil_net_opts_s o;
  lutilVerbosityLevel v;
  gint r;

  v = lutil_setup_verbosity_level(opts.core.verbosity);
  memset(&o, 0, sizeof(struct lutil_net_opts_s));

  o.verbose = (v == UTIL_VERBOSITY_LEVEL_DEBUG) ? 1:0;
  o.throttle_ki_s = opts.get.throttle;

  r = lutil_quvi_init(q, &o);
  if (r == EXIT_SUCCESS)
    {
      quvi_set(*q, QUVI_OPTION_ALLOW_COOKIES, opts.http.enable_cookies);
      quvi_set(*q, QUVI_OPTION_USER_AGENT, opts.http.user_agent);
      quvi_set(*q, QUVI_OPTION_CALLBACK_STATUS, cb_status);
    }
  return (r);
}

#define _reverse(p)\
  do {\
    if (p != NULL)\
      p = g_slist_reverse(p);\
  } while (0)

static void _reverse_input_url_order(lutil_check_support_t css)
{
  _reverse(css->url.playlist);
  _reverse(css->url.subtitle);
  _reverse(css->url.media);
}

#undef _reverse

gint setup_query(setup_query_t sq)
{
  struct lutil_query_properties_s qps;
  struct lutil_check_support_s css;

  g_assert(sq != NULL);
  g_assert(sq->activity.playlist != NULL);
  g_assert(sq->activity.subtitle != NULL);
  g_assert(sq->activity.media != NULL);
  g_assert(sq->linput != NULL);
  g_assert(sq->xperr != NULL);
  g_assert(sq->perr != NULL);
  g_assert(sq->q != NULL);

  /* check {media,playlist,subtitle} URL support. */

  memset(&css, 0, sizeof(struct lutil_check_support_s));

  css.flags.force_offline_mode = opts.core.check_mode_offline;
  css.flags.force_subtitle_mode = sq->force_subtitle_mode;

  css.exit_status = EXIT_SUCCESS;
  css.xperr = sq->xperr;
  css.perr = sq->perr;
  css.q = sq->q;

  g_slist_foreach(sq->linput->url.input, lutil_check_support, &css);

  if (css.exit_status != EXIT_SUCCESS)
    {
      lutil_check_support_free(&css);
      return (css.exit_status);
    }

  /*
   * The "check support" facility prepends to the lists. Reverse the order
   * to restore the input order.
   */
  _reverse_input_url_order(&css);

  /* query {playlist,media} properties. */

  memset(&qps, 0, sizeof(struct lutil_query_properties_s));

  qps.activity = sq->activity.playlist;
  qps.exit_status = EXIT_SUCCESS;
  qps.xperr = sq->xperr;
  qps.perr = sq->perr;
  qps.q = sq->q;

  g_slist_foreach(css.url.playlist, lutil_query_playlist, &qps);

  if (qps.exit_status == EXIT_SUCCESS)
    {
      if (css.flags.force_subtitle_mode == TRUE)
        {
          qps.activity = sq->activity.subtitle;
          g_slist_foreach(css.url.subtitle, lutil_query_subtitle, &qps);
        }
      else
        {
          qps.activity = sq->activity.media;
          g_slist_foreach(css.url.media, lutil_query_media, &qps);
        }
    }
  lutil_check_support_free(&css);
  return (qps.exit_status);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */

