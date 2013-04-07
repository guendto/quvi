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

/* This module prints directly to std{out,err}. */

#include "config.h"

#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <quvi.h>

#include "lutil.h"
/* -- */
#include "lprint.h"

struct enum_s
{
  quvi_media_t qm;
  quvi_t q;
};

typedef struct enum_s *enum_t;

static gint _enum_handle_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  enum_t s;

  g_assert(dst != NULL);

  s = g_new0(struct enum_s, 1);
  s->qm = qm;
  s->q = q;
  *dst = s;

  return (EXIT_SUCCESS);
}

static gint _enum_handle_free(gpointer p, const gint r)
{
  g_free(p);
  return (r);
}

static gint _print(const enum_t p, const lutilPropertyType pt,
                   const gchar *n, const gchar *s, const gdouble d)
{
  gint r;

  r = lutil_chk_property_ok(p->q, pt, n, lprint_enum_errmsg);
  if (r != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (s != NULL)
    g_print("%s=%s\n", n, s);
  else
    g_print("%s=%.0f\n", n, d);

  return (EXIT_SUCCESS);
}

void lprint_enum_errmsg(const gchar *fmt, ...)
{
  va_list args;
  gchar *s;

  va_start(args, fmt);
  if (g_vasprintf(&s, fmt, args) >0)
    {
      g_printerr(_("error: %s\n"), s);
      g_free(s);
    }
  va_end(args);
}

typedef lprint_cb_errmsg cem;

/* media */

gint lprint_enum_media_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  return (_enum_handle_new(q, qm, dst));
}

void lprint_enum_media_free(gpointer data)
{
  _enum_handle_free(data, -1);
}

gint lprint_enum_media_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

static gint _mp_s(const enum_t p, const QuviMediaProperty qmp, const gchar *n)
{
  gchar *s = NULL;
  quvi_media_get(p->qm, qmp, &s);
  return (_print(p, UTIL_PROPERTY_TYPE_MEDIA, n, s, -1));
}

#define _print_mp_s(n)\
  do {\
    if (_mp_s(p, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mp_d(const enum_t p, const QuviMediaProperty qmp, const gchar *n)
{
  gdouble d = 0;
  quvi_media_get(p->qm, qmp, &d);
  return (_print(p, UTIL_PROPERTY_TYPE_MEDIA, n, NULL, d));
}

#define _print_mp_d(n)\
  do {\
    if (_mp_d(p, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mi_s(const enum_t p, const quvi_http_metainfo_t qmi,
                  const QuviHTTPMetaInfoProperty qmip, const gchar *n)
{
  gchar *s = NULL;
  quvi_http_metainfo_get(qmi, qmip, &s);
  return (_print(p, UTIL_PROPERTY_TYPE_HTTP_METAINFO, n, s, -1));
}

#define _print_mi_s(n)\
  do {\
    if (_mi_s(p, qmi, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mi_d(const enum_t p, const quvi_http_metainfo_t qmi,
                  const QuviHTTPMetaInfoProperty qmip, const gchar *n)
{
  gdouble d = 0;
  quvi_http_metainfo_get(qmi, qmip, &d);
  return (_print(p, UTIL_PROPERTY_TYPE_HTTP_METAINFO, n, NULL, d));
}

#define _print_mi_d(n)\
  do {\
    if (_mi_d(p, qmi, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint
_print_media_stream_properties(const enum_t p, const quvi_http_metainfo_t qmi)
{
  _print_mp_s(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_ENCODING);
  _print_mp_s(QUVI_MEDIA_STREAM_PROPERTY_AUDIO_ENCODING);
  _print_mp_s(QUVI_MEDIA_STREAM_PROPERTY_CONTAINER);
  _print_mp_s(QUVI_MEDIA_STREAM_PROPERTY_URL);
  _print_mp_s(QUVI_MEDIA_STREAM_PROPERTY_ID);

  _print_mp_d(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_BITRATE_KBIT_S);
  _print_mp_d(QUVI_MEDIA_STREAM_PROPERTY_AUDIO_BITRATE_KBIT_S);
  _print_mp_d(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_HEIGHT);
  _print_mp_d(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_WIDTH);

  if (qmi != NULL)
    {
      _print_mi_s(QUVI_HTTP_METAINFO_PROPERTY_FILE_EXTENSION);
      _print_mi_s(QUVI_HTTP_METAINFO_PROPERTY_CONTENT_TYPE);
      _print_mi_d(QUVI_HTTP_METAINFO_PROPERTY_LENGTH_BYTES);
    }
  return (EXIT_SUCCESS);
}

#undef _print_mi_s
#undef _print_mi_d

gint
lprint_enum_media_stream_properties(quvi_http_metainfo_t qmi, gpointer data)
{
  return (_print_media_stream_properties(data, qmi));
}

gint lprint_enum_media_streams_available(quvi_t q, quvi_media_t qm)
{
  enum_t p;
  gint r;

  if (_enum_handle_new(q, qm, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  r = EXIT_SUCCESS;
  while (quvi_media_stream_next(qm) == QUVI_TRUE && r ==EXIT_SUCCESS)
    r = _print_media_stream_properties(p, NULL);

  return (_enum_handle_free(p, r));
}

gint lprint_enum_media_properties(gpointer data)
{
  enum_t p;

  g_assert(data != NULL);
  p = (enum_t) data;

  _print_mp_s(QUVI_MEDIA_PROPERTY_THUMBNAIL_URL);
  _print_mp_s(QUVI_MEDIA_PROPERTY_TITLE);
  _print_mp_s(QUVI_MEDIA_PROPERTY_ID);

  _print_mp_d(QUVI_MEDIA_PROPERTY_START_TIME_MS);
  _print_mp_d(QUVI_MEDIA_PROPERTY_DURATION_MS);

  return (EXIT_SUCCESS);
}

#undef _print_mp_s
#undef _print_mp_d

/* playlist */

gint lprint_enum_playlist_new(quvi_t q, gpointer *dst)
{
  return (_enum_handle_new(q, NULL, dst));
}

void lprint_enum_playlist_free(gpointer data)
{
  _enum_handle_free(data, -1);
}

gint lprint_enum_playlist_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

static gint _pp_s(const enum_t p, const quvi_playlist_t qp,
                  const QuviPlaylistProperty qpp, const gchar *n)
{
  gchar *s = NULL;
  quvi_playlist_get(qp, qpp, &s);
  return (_print(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, s, -1));
}

#define _print_pp_s(n)\
  do {\
    if (_pp_s(p, qp, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _pp_d(const enum_t p, const quvi_playlist_t qp,
                  const QuviPlaylistProperty qpp, const gchar *n)
{
  gdouble d = 0;
  quvi_playlist_get(qp, qpp, &d);
  return (_print(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, NULL, d));
}

#define _print_pp_d(n)\
  do {\
    if (_pp_d(p, qp, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

gint lprint_enum_playlist_properties(quvi_playlist_t qp, gpointer data)
{
  enum_t p;

  g_assert(data != NULL);
  p = (enum_t) data;

  _print_pp_s(QUVI_PLAYLIST_PROPERTY_THUMBNAIL_URL);
  _print_pp_s(QUVI_PLAYLIST_PROPERTY_TITLE);
  _print_pp_s(QUVI_PLAYLIST_PROPERTY_ID);

  while (quvi_playlist_media_next(qp) == QUVI_TRUE)
    {
      _print_pp_d(QUVI_PLAYLIST_MEDIA_PROPERTY_DURATION_MS);
      _print_pp_s(QUVI_PLAYLIST_MEDIA_PROPERTY_TITLE);
      _print_pp_s(QUVI_PLAYLIST_MEDIA_PROPERTY_URL);
    }
  return (EXIT_SUCCESS);
}

#undef _print_pp_s
#undef _print_pp_d

/* scan */

gint lprint_enum_scan_new(quvi_t q, gpointer *dst)
{
  return (_enum_handle_new(q, NULL, dst));
}

void lprint_enum_scan_free(gpointer data)
{
  _enum_handle_free(data, -1);
}

gint lprint_enum_scan_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

gint lprint_enum_scan_properties(quvi_scan_t qs, gpointer data)
{
  const gchar *s;

  g_assert(qs != NULL);

  /*
   * 'scan' interface does not have any "properties". Use the function
   * name instead of an enum.
   */

  while ( (s = quvi_scan_next_media_url(qs)) != NULL)
    g_print("quvi_scan_next_media_url=%s\n", s);

  return (EXIT_SUCCESS);
}

/* subtitle */

gint lprint_enum_subtitle_new(quvi_t q, gpointer *dst)
{
  return (_enum_handle_new(q, NULL, dst));
}

void lprint_enum_subtitle_free(gpointer data)
{
  _enum_handle_free(data, -1);
}

gint lprint_enum_subtitle_print_buffer(gpointer data)
{
  /* Nothing to buffer: everything gets printed to stdout immediately */
  return (EXIT_SUCCESS);
}

static gint _stp_d(const enum_t p, const quvi_subtitle_type_t qst,
                   const QuviSubtitleTypeProperty qstp, const gchar *n)
{
  gdouble d = 0;
  quvi_subtitle_type_get(qst, qstp, &d);
  return (_print(p, UTIL_PROPERTY_TYPE_SUBTITLE_TYPE, n, NULL, d));
}

#define _print_stp_d(n)\
  do {\
    if (_stp_d(p, t, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _slp_s(const enum_t p, const quvi_subtitle_lang_t qsl,
                   const QuviSubtitleLangProperty qslp, const gchar *n)
{
  gchar *s = NULL;
  quvi_subtitle_lang_get(qsl, qslp, &s);
  return (_print(p, UTIL_PROPERTY_TYPE_SUBTITLE_LANGUAGE, n, s, -1));
}

#define _print_slp_s(n)\
  do {\
    if (_slp_s(p, l, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

gint
lprint_enum_subtitle_lang_properties(quvi_subtitle_lang_t l, gpointer data)
{
  enum_t p;

  g_assert(data != NULL);
  p = (enum_t) data;

  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_TRANSLATED);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_ORIGINAL);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_CODE);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_URL);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_ID);

  return (EXIT_SUCCESS);
}

gint lprint_enum_subtitles_available(quvi_t q, quvi_subtitle_t qsub)
{
  quvi_subtitle_type_t t;
  quvi_subtitle_lang_t l;
  enum_t p;
  gint r;

  if (_enum_handle_new(q, NULL, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  r = EXIT_SUCCESS;
  while ( (t = quvi_subtitle_type_next(qsub)) != NULL && r ==EXIT_SUCCESS)
    {
      _print_stp_d(QUVI_SUBTITLE_TYPE_PROPERTY_FORMAT);
      _print_stp_d(QUVI_SUBTITLE_TYPE_PROPERTY_TYPE);

      while ( (l = quvi_subtitle_lang_next(t)) != NULL && r ==EXIT_SUCCESS)
        r = lprint_enum_subtitle_lang_properties(l, p);
    }
  return (_enum_handle_free(p, r));
}

#undef _print_stp_d
#undef _print_slp_s

/* vim: set ts=2 sw=2 tw=72 expandtab: */
