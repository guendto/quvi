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
#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "linput.h"
#include "lutil.h"

static gchar *_read_stdin()
{
  GIOChannel *c;
  gchar *r, *s;
  gsize n;

  c = g_io_channel_unix_new(STDIN_FILENO);
  r = s = NULL;
  n = 0;

  while (g_io_channel_read_line(c, &s, NULL, NULL, NULL) != G_IO_STATUS_EOF)
    {
      gchar *p;
      gsize l;

      l = strlen(s);
      p = g_realloc(r, n+l+1);

      if (p != NULL)
        {
          r = p;
          memcpy(&r[n], s, l);
          n += l;
          r[n] = '\0';
        }
      g_free(s);
    }
  g_io_channel_unref(c);
  return (r);
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
          g_printerr(_("error: while reading file: %s\n"), e->message);
          g_error_free(e);
        }
    }
  return (r);
}

static gint _extract_uris(linput_t, const gchar*);

static gint _read_from_uri(linput_t p, const gchar *u)
{
  GError *e;
  gchar *f;
  gint r;

  r = EXIT_FAILURE;
  e = NULL;
  f = g_filename_from_uri(u, NULL, &e);

  if (f !=NULL)
    {
      gchar *c = _read_file(f);
      if (c !=NULL)
        {
          r = _extract_uris(p, c);
          r = EXIT_SUCCESS;
          g_free(c);
        }
      g_free(f);
    }
  else
    {
      g_printerr(_("error: while converting to URI: %s\n"), e->message);
      g_error_free(e);
    }
  return (r);
}

static gint _determine_input(linput_t p, const gboolean try_read_as_file,
                             const gchar *invalid_msg, const gchar *s)
{
  gchar *c;
  gint r;

  c = g_uri_parse_scheme(s);
  r = EXIT_SUCCESS;

  if ((c ==NULL || strlen(c) ==0) && try_read_as_file ==TRUE)
    {
      gchar *b = _read_file(s);
      if (b != NULL)
        {
          r = _extract_uris(p, b);
          g_free(b);
        }
      else
        r = EXIT_FAILURE;
    }
  else if (g_strcmp0(c, "http") ==0 || g_strcmp0(c, "https") ==0)
    p->url.input = lutil_slist_prepend_if_unique(p->url.input, s);
  else if (g_strcmp0(c, "file") ==0)
    r = _read_from_uri(p, s);
  else
    {
      g_printerr(g_dgettext(GETTEXT_PACKAGE, invalid_msg), s);
      r = EXIT_FAILURE;
    }
  g_free(c);
  return (r);
}

static gint _extract_uris(linput_t p, const gchar *s)
{
  static const gchar *E = N_("error: %s: an invalid URI\n");

  gchar **u;
  gint i, r;

  u = g_uri_list_extract_uris(s);

  for (i=0, r=EXIT_SUCCESS; u[i] !=NULL && r==EXIT_SUCCESS; ++i)
    r = _determine_input(p, FALSE, E, u[i]);

  g_strfreev(u);
  return (r);
}

static gint _parse_without_rargs(linput_t p)
{
  gchar *c;
  gint r;
  c = _read_stdin();
  r = _extract_uris(p, c);
  g_free(c);
  return (r);
}

static gint _parse_with_rargs(linput_t p, const gchar **rargs)
{
  static const gchar *E =
    N_("error: %s: neither a valid URI or a local file\n");

  gint i, r;
  for (i=0, r=EXIT_SUCCESS; rargs[i] !=NULL && r ==EXIT_SUCCESS; ++i)
    r = _determine_input(p, TRUE, E, rargs[i]);

  return (r);
}

gint linput_new(linput_t linput, const gchar **rargs)
{
  gint r;

  g_assert(linput != NULL);
  g_assert(linput->url.input == NULL);

  if (rargs == NULL || g_strv_length((gchar**) rargs) == 0)
    r = _parse_without_rargs(linput);
  else
    r = _parse_with_rargs(linput, rargs);

  linput->url.input = g_slist_reverse(linput->url.input);
  return (r);
}

void linput_free(linput_t linput)
{
  lutil_slist_free_full(linput->url.input, (GFunc) g_free);
  linput->url.input = NULL;
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
