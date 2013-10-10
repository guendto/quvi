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

/*
 * An URI list conforming to the text/uri-list mime type defined in
 * RFC2483 into individual URIs.
 *
 * media: default/selected stream URL only
 *  streams: all media stream URLs
 * scan: embedded media URLs only
 * subtitle: language URLs only
 * playlist: media URLs only
 *
 * This module prints directory to std{out,err}, and reuses the
 * lprint_enum_errmsg function from the enum module.
 */

#include "config.h"

#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <quvi.h>

#include "lprint.h"
#include "lutil.h"

struct rfc2483_s
{
  quvi_media_t qm;
  quvi_t q;
};

typedef struct rfc2483_s *rfc2483_t;

static gint _rfc2483_handle_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  rfc2483_t n;

  g_assert(dst != NULL);

  n = g_new0(struct rfc2483_s, 1);
  n->qm = qm;
  n->q = q;
  *dst = n;

  return (EXIT_SUCCESS);
}

static gint _rfc2483_handle_free(gpointer data, const gint r)
{
  g_free(data);
  return (r);
}

extern const gchar *reserved_chars;

static gint _print(const rfc2483_t p, const lutilPropertyType pt,
                   const gchar *n, const gchar *s, const gdouble d,
                   const gboolean comment_out, const gboolean escape)
{
  const gchar *h;
  gint r;

  r = lutil_chk_property_ok(p->q, pt, n, lprint_enum_errmsg);
  if (r != EXIT_SUCCESS) /* Reuse the lprint_enum_errmsg. */
    return (EXIT_FAILURE);

  h = (comment_out == TRUE) ? "# ":"";

  if (s != NULL)
    {
      if (escape == TRUE)
        {
          gchar *e = g_uri_escape_string(s, reserved_chars, FALSE);
          g_print("%s%s\n", h, e);
          g_free(e);
        }
      else
        g_print("%s%s\n", h, s);
    }
  else
    g_print("%s%.0f\n", h, d);

  return (EXIT_SUCCESS);
}

/* media */

gint lprint_rfc2483_media_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  return (_rfc2483_handle_new(q, qm, dst));
}

void lprint_rfc2483_media_free(gpointer data)
{
  _rfc2483_handle_free(data, -1);
}

gint lprint_rfc2483_media_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

typedef lprint_cb_errmsg cem;

static gint _mp_s(const rfc2483_t p, const QuviMediaProperty qmp,
                  const gchar *n, const gboolean c, const gboolean e)
{
  gchar *s = NULL;
  quvi_media_get(p->qm, qmp, &s);
  return (_print(p, UTIL_PROPERTY_TYPE_MEDIA, n, s, -1, c, e));
}

#define _print_mp_s(n,c,e) /* c=commented-out, e=escaped */ \
  do {\
    if (_mp_s(p, n, #n, c, e) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

gint
lprint_rfc2483_media_stream_properties(quvi_http_metainfo_t qmi,
                                       gpointer data)
{
  rfc2483_t p = (rfc2483_t) data;

  g_assert(data != NULL);

  _print_mp_s(QUVI_MEDIA_STREAM_PROPERTY_ID, TRUE, FALSE);
  _print_mp_s(QUVI_MEDIA_STREAM_PROPERTY_URL, FALSE, TRUE);

  return (EXIT_SUCCESS);
}

gint lprint_rfc2483_media_streams_available(quvi_t q, quvi_media_t qm)
{
  rfc2483_t p;
  gint r;

  if (_rfc2483_handle_new(q, qm, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  g_print(_("# Media streams\n#\n"));

  r = EXIT_SUCCESS;
  while (quvi_media_stream_next(qm) == QUVI_TRUE && r ==EXIT_SUCCESS)
    r = lprint_rfc2483_media_stream_properties(NULL, p);

  return (_rfc2483_handle_free(p, r));
}

#undef _print_mp_s

gint lprint_rfc2483_media_properties(gpointer data)
{
  return (EXIT_SUCCESS);
}

/* playlist */

gint lprint_rfc2483_playlist_new(quvi_t q, gpointer *dst)
{
  return (_rfc2483_handle_new(q, NULL, dst));
}

void lprint_rfc2483_playlist_free(gpointer data)
{
  /* Nothing to release. */
}

gint lprint_rfc2483_playlist_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

static gint _pp_s(const rfc2483_t p, const quvi_playlist_t qp,
                  const QuviPlaylistProperty qpp, const gchar *n,
                  const gboolean c, const gboolean e)
{
  gchar *s = NULL;
  quvi_playlist_get(qp, qpp, &s);
  return (_print(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, s, -1, c, e));
}

#define _print_pp_s(n,c,e) /* c=commented-out, e=escaped */ \
  do {\
    if (_pp_s(p, qp, n, #n, c, e) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _pp_d(const rfc2483_t p, const quvi_playlist_t qp,
                  const QuviPlaylistProperty qpp, const gchar *n,
                  const gboolean c)
{
  gdouble d = 0;
  quvi_playlist_get(qp, qpp, &d);
  return (_print(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, NULL, d, c, FALSE));
}

#define _print_pp_d(n,c) /* c=commented-out */ \
  do {\
    if (_pp_d(p, qp, n, #n, c) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

gint lprint_rfc2483_playlist_properties(quvi_playlist_t qp, gpointer data)
{
  rfc2483_t p = (rfc2483_t) data;

  g_assert(data != NULL);
  g_assert(qp != NULL);

  g_print(_("# Playlist media URLs\n#\n"));

  _print_pp_s(QUVI_PLAYLIST_PROPERTY_TITLE, TRUE, FALSE);
  _print_pp_s(QUVI_PLAYLIST_PROPERTY_ID, TRUE, FALSE);
  _print_pp_s(QUVI_PLAYLIST_PROPERTY_THUMBNAIL_URL, TRUE, FALSE);

  g_print("#\n");

  while (quvi_playlist_media_next(qp) == QUVI_TRUE)
    {
      _print_pp_s(QUVI_PLAYLIST_MEDIA_PROPERTY_TITLE, TRUE, FALSE);
      _print_pp_d(QUVI_PLAYLIST_MEDIA_PROPERTY_DURATION_MS, TRUE);
      _print_pp_s(QUVI_PLAYLIST_MEDIA_PROPERTY_URL, FALSE, TRUE);
    }
  return (EXIT_SUCCESS);
}

#undef _print_pp_s

/* scan */

gint lprint_rfc2483_scan_new(quvi_t q, gpointer *dst)
{
  return (_rfc2483_handle_new(q, NULL, dst));
}

void lprint_rfc2483_scan_free(gpointer data)
{
  _rfc2483_handle_free(data, -1);
}

gint lprint_rfc2483_scan_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

gint lprint_rfc2483_scan_properties(quvi_scan_t qs, gpointer data)
{
  const gchar *s;

  g_assert(qs != NULL);

  /* 'scan' interface does not have any properties. */

  g_print(_("# Embedded media URLs\n#\n"));
  while ( (s = quvi_scan_next_media_url(qs)) != NULL)
    {
      gchar *e = g_uri_escape_string(s, reserved_chars, FALSE);
      g_print("%s\n", e);
      g_free(e);
    }
  return (EXIT_SUCCESS);
}

/* subtitle */

gint lprint_rfc2483_subtitle_new(quvi_t q, gpointer *dst)
{
  return (_rfc2483_handle_new(q, NULL, dst));
}

void lprint_rfc2483_subtitle_free(gpointer data)
{
  _rfc2483_handle_free(data, -1);
}

gint lprint_rfc2483_subtitle_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

static gint _slp_s(const rfc2483_t p, const quvi_subtitle_lang_t qsl,
                   const QuviSubtitleLangProperty qslp, const gchar *n,
                   const gboolean c, const gboolean e)
{
  gchar *s = NULL;
  quvi_subtitle_lang_get(qsl, qslp, &s);
  return (_print(p, UTIL_PROPERTY_TYPE_SUBTITLE_LANGUAGE, n, s, -1, c, e));
}

#define _print_slp_s(n,c,e) /* c=commented-out, e=escaped */ \
  do {\
    if (_slp_s(p, l, n, #n, c, e) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

gint
lprint_rfc2483_subtitle_lang_properties(quvi_subtitle_lang_t l,
                                        gpointer data)
{
  rfc2483_t p;

  g_assert(data != NULL);
  p = (rfc2483_t) data;

  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_ID, TRUE, FALSE);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_URL, FALSE, TRUE);

  return (EXIT_SUCCESS);
}

gint lprint_rfc2483_subtitles_available(quvi_t q, quvi_subtitle_t qsub)
{
  quvi_subtitle_type_t t;
  quvi_subtitle_lang_t l;
  rfc2483_t p;
  gint r;

  if (_rfc2483_handle_new(q, NULL, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  g_print(_("# Subtitles\n#\n"));

  r = EXIT_SUCCESS;
  while ( (t = quvi_subtitle_type_next(qsub)) != NULL && r ==EXIT_SUCCESS)
    {
      while ( (l = quvi_subtitle_lang_next(t)) != NULL && r ==EXIT_SUCCESS)
        r = lprint_rfc2483_subtitle_lang_properties(l, p);
    }
  return (_rfc2483_handle_free(p, r));
}

#undef _print_slp_s

/* vim: set ts=2 sw=2 tw=72 expandtab: */
