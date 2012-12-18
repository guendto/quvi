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

#include <unistd.h>
#include <string.h>
#include <glib/gi18n.h>

#include "lutil.h"
#include "linput.h"

static gchar *_read_stdin(linput_t linput)
{
  gchar *s, *r, *p;
  GIOChannel *c;
  gsize n, sn;

  c = g_io_channel_unix_new(STDIN_FILENO);
  s = NULL;
  r = NULL;
  n = 0;

  while (g_io_channel_read_line(c, &s, NULL, NULL, NULL) != G_IO_STATUS_EOF)
    {
      sn = strlen(s);
      p = g_realloc(r, n+sn+1);
      if (p != NULL)
        {
          r = p;
          memcpy(&(r[n]), s, sn);
          n += sn;
          r[n] = '\0';
        }
      g_free(s);
      s = NULL;
    }
  return (r);
}

static gboolean _has_uri_scheme(const gchar *s, gchar **dst)
{
  gchar *p, *t;
  gboolean r;

  t = g_uri_unescape_string(s, NULL);
  p = g_uri_parse_scheme(t);
  r = (p != NULL) ? TRUE:FALSE;
  g_free(p);

  if (dst != NULL && r == TRUE)
    *dst = t;
  else
    g_free(t);

  return (r);
}

static void _extract_urls(const gchar *b, linput_t l)
{
  gchar **r, *u;
  gint i;

  r = g_uri_list_extract_uris(b);
  i = 0;

  while (r[i] != NULL)
    {
      if (_has_uri_scheme(r[i], &u) == TRUE)
        l->url.input = lutil_slist_prepend_if_unique(l->url.input, u);
      ++i;
    }
  g_strfreev(r);
}

static gchar *_read_file(const gchar *fpath)
{
  GError *e;
  gchar *r;

  e = NULL;
  r = NULL;

  if (g_file_get_contents(fpath, &r, NULL, &e) == FALSE)
    {
      if (e != NULL)
        {
          g_printerr(_("error: %s: while reading file: %s\n"),
                     fpath, e->message);
          g_error_free(e);
          e = NULL;
        }
    }
  return (r);
}

static void _no_rargs(linput_t linput)
{
  gchar *b = _read_stdin(linput);
  if (b != NULL)
    {
      _extract_urls(b, linput);
      g_free(b);
    }
}

static void _have_rargs(linput_t linput, const gchar **rargs)
{
  static const gchar *_E =
    N_("error: ignoring input value `%s': not an URL or a file\n");

  gchar *p, *u;
  gint i;

  i = 0;
  while (rargs[i] != NULL)
    {
      p = (gchar*) rargs[i++];
      g_strstrip(p);

      /* If file. */
      if (g_file_test(p, G_FILE_TEST_IS_REGULAR) == TRUE)
        {
          gchar *b = _read_file(p);
          if (b != NULL)
            {
              _extract_urls(b, linput);
              g_free(b);
            }
        }
      /* If URI. */
      else if (_has_uri_scheme(p, &u) == TRUE)
        {
          linput->url.input =
            lutil_slist_prepend_if_unique(linput->url.input, u);
        }
      /* Unable to determine. */
      else
        g_printerr(g_dgettext(GETTEXT_PACKAGE, _E), p);
    }
}

void linput_new(linput_t linput, const gchar **rargs)
{
  g_assert(linput != NULL);
  g_assert(linput->url.input == NULL);

  if (rargs == NULL || g_strv_length((gchar**) rargs) == 0)
    _no_rargs(linput);
  else
    _have_rargs(linput, rargs);

  linput->url.input = g_slist_reverse(linput->url.input);
}

void linput_free(linput_t linput)
{
  lutil_slist_free_full(linput->url.input, (GFunc) g_free);
  linput->url.input = NULL;
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
