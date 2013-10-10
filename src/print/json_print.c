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

/* "json" module buffers the data to an instance of JsonBuilder. */

#include "config.h"

#include <stdlib.h>
#include <json-glib/json-glib.h>
#include <glib/gprintf.h>
#include <quvi.h>

#if JSON_MAJOR_VERSION >=0 && JSON_MINOR_VERSION >=14
#define HAVE_PRETTY 1
#endif

#include "lutil.h"
/* -- */
#include "lprint.h"

struct json_s
{
  quvi_media_t qm;
  JsonBuilder *b;
  quvi_t q;
};

typedef struct json_s *json_t;

static gint _json_handle_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  json_t p;

  g_assert(dst != NULL);

  p = g_new0(struct json_s, 1);
  p->b = json_builder_new();
  p->qm = qm;
  p->q = q;

  *dst = p;

  return ((p->b != NULL) ? EXIT_SUCCESS:EXIT_FAILURE);
}

static gint _json_handle_free(gpointer data, const gint r)
{
  json_t p = (json_t) data;

  if (p == NULL)
    return (r);

  if (p->b != NULL)
    {
      g_object_unref(p->b);
      p->b = NULL;
    }
  g_free(p);

  return (r);
}

static gint _print_buffer(const json_t p)
{
  JsonGenerator *g;
  JsonNode *r;
  gchar *s;

  g_assert(p != NULL);

  g = json_generator_new();
  r = json_builder_get_root(p->b);

  json_generator_set_root(g, r);
#ifdef HAVE_PRETTY
  json_generator_set_pretty(g, 1);
#endif
  s = json_generator_to_data(g, NULL);

  json_node_free(r);
  g_object_unref(g);

  g_print("%s\n", s);
  g_free(s);

  return (EXIT_SUCCESS);
}

extern const gchar *reserved_chars;

void lprint_json_errmsg(const gchar *fmt, ...)
{
  va_list args;
  gchar *s;

  va_start(args, fmt);
  if (g_vasprintf(&s, fmt, args) >0)
    {
      gchar *e = g_uri_escape_string(s, reserved_chars, FALSE);
      g_printerr("{\"error\" : \"%s\"}\n", e);
      g_free(e);
      g_free(s);
    }
  va_end(args);
}

typedef lprint_cb_errmsg cem;

/* media */

gint lprint_json_media_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  return (_json_handle_new(q, qm, dst));
}

void lprint_json_media_free(gpointer data)
{
  _json_handle_free(data, -1);
}

gint lprint_json_media_print_buffer(gpointer data)
{
  json_t p = (json_t) data;

  g_assert(data != NULL);
  g_assert(p->b != NULL);

  json_builder_end_object(p->b); /* media */
  json_builder_end_object(p->b); /* quvi */
  json_builder_end_object(p->b); /* root */

  return (_print_buffer(p));
}

static gint _set_member(const json_t p, const lutilPropertyType pt,
                        const gchar *n, const gchar *s, const gdouble d)
{
  const gint r = lutil_chk_property_ok(p->q, pt, n, lprint_json_errmsg);

  if (r != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  json_builder_set_member_name(p->b, n);
  if (s != NULL)
    {
      gchar *e = g_uri_escape_string(s, reserved_chars, FALSE);
      json_builder_add_string_value(p->b, e);
      g_free(e);
    }
  else
    json_builder_add_double_value(p->b, d);

  return (EXIT_SUCCESS);
}

static gint _mp_s(const json_t p, const QuviMediaProperty qmp, const gchar *n)
{
  gchar *s = NULL;
  quvi_media_get(p->qm, qmp, &s);
  return (_set_member(p, UTIL_PROPERTY_TYPE_MEDIA, n, s, -1));
}

#define _print_mp_s(n)\
  do {\
    if (_mp_s(p, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mp_d(const json_t p, const QuviMediaProperty qmp, const gchar *n)
{
  gdouble d = 0;
  quvi_media_get(p->qm, qmp, &d);
  return (_set_member(p, UTIL_PROPERTY_TYPE_MEDIA, n, NULL, d));
}

#define _print_mp_d(n)\
  do {\
    if (_mp_d(p, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mi_s(const json_t p, const quvi_http_metainfo_t qmi,
                  const QuviHTTPMetaInfoProperty qmip, const gchar *n)
{
  gchar *s = NULL;
  quvi_http_metainfo_get(qmi, qmip, &s);
  return (_set_member(p, UTIL_PROPERTY_TYPE_HTTP_METAINFO, n, s, -1));
}

#define _print_mi_s(n)\
  do {\
    if (_mi_s(p, qmi, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mi_d(const json_t p, const quvi_http_metainfo_t qmi,
                  const QuviHTTPMetaInfoProperty qmip, const gchar *n)
{
  gdouble d = 0;
  quvi_http_metainfo_get(qmi, qmip, &d);
  return (_set_member(p, UTIL_PROPERTY_TYPE_HTTP_METAINFO, n, NULL, d));
}

#define _print_mi_d(n)\
  do {\
    if (_mi_d(p, qmi, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint
_print_media_stream_properties(const json_t p, const quvi_http_metainfo_t qmi)
{
  json_builder_begin_object(p->b);

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

  json_builder_end_object(p->b); /* stream */
  return (EXIT_SUCCESS);
}

gint
lprint_json_media_stream_properties(quvi_http_metainfo_t qmi, gpointer data)
{
  json_t p = (json_t) data;

  g_assert(data != NULL);
  g_assert(p->b != NULL);

  json_builder_set_member_name(p->b, "stream");
  return (_print_media_stream_properties(p, qmi));
}

gint lprint_json_media_streams_available(quvi_t q, quvi_media_t qm)
{
  json_t p;
  gint r;

  if (_json_handle_new(q, qm, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  json_builder_begin_object(p->b); /* root */

  json_builder_set_member_name(p->b, "quvi");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "media");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "streams");
  json_builder_begin_array(p->b);

  r = EXIT_SUCCESS;
  while (quvi_media_stream_next(qm) == QUVI_TRUE && r == EXIT_SUCCESS)
    r = _print_media_stream_properties(p, NULL);

  json_builder_end_array(p->b); /* streams */
  json_builder_end_object(p->b); /* media */
  json_builder_end_object(p->b); /* quvi */
  json_builder_end_object(p->b); /* root */

  if (r == EXIT_SUCCESS)
    r = _print_buffer(p);

  return (_json_handle_free(p, r));
}

#undef _print_mi_s
#undef _print_mi_d

gint lprint_json_media_properties(gpointer data)
{
  json_t p = (json_t) data;

  g_assert(data != NULL);
  json_builder_begin_object(p->b); /* root */

  json_builder_set_member_name(p->b, "quvi");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "media");
  json_builder_begin_object(p->b);

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

gint lprint_json_playlist_new(quvi_t q, gpointer *dst)
{
  return (_json_handle_new(q, NULL, dst));
}

void lprint_json_playlist_free(gpointer data)
{
  _json_handle_free(data, -1);
}

gint lprint_json_playlist_print_buffer(gpointer data)
{
  return (_print_buffer(data));
}

static gint _pp_s(const json_t p, const quvi_playlist_t qp,
                  const QuviPlaylistProperty qpp, const gchar *n)
{
  gchar *s = NULL;
  quvi_playlist_get(qp, qpp, &s);
  return (_set_member(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, s, -1));
}

#define _print_pp_s(n)\
  do {\
    if (_pp_s(p, qp, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _pp_d(const json_t p, const quvi_playlist_t qp,
                  const QuviPlaylistProperty qpp, const gchar *n)
{
  gdouble d = 0;
  quvi_playlist_get(qp, qpp, &d);
  return (_set_member(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, NULL, d));
}

#define _print_pp_d(n)\
  do {\
    if (_pp_d(p, qp, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

gint lprint_json_playlist_properties(quvi_playlist_t qp, gpointer data)
{
  json_t p = (json_t) data;

  g_assert(data != NULL);
  g_assert(qp != NULL);

  json_builder_begin_object(p->b); /* root */

  json_builder_set_member_name(p->b, "quvi");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "playlist");
  json_builder_begin_object(p->b);

  _print_pp_s(QUVI_PLAYLIST_PROPERTY_THUMBNAIL_URL);
  _print_pp_s(QUVI_PLAYLIST_PROPERTY_TITLE);
  _print_pp_s(QUVI_PLAYLIST_PROPERTY_ID);

  json_builder_set_member_name(p->b, "media");
  json_builder_begin_array(p->b);

  while (quvi_playlist_media_next(qp) == QUVI_TRUE)
    {
      json_builder_begin_object(p->b); /* media */
      _print_pp_d(QUVI_PLAYLIST_MEDIA_PROPERTY_DURATION_MS);
      _print_pp_s(QUVI_PLAYLIST_MEDIA_PROPERTY_TITLE);
      _print_pp_s(QUVI_PLAYLIST_MEDIA_PROPERTY_URL);
      json_builder_end_object(p->b); /* media */
    }

  json_builder_end_array(p->b); /* media */
  json_builder_end_object(p->b); /* playlist */

  json_builder_end_object(p->b); /* quvi */
  json_builder_end_object(p->b); /* root */

  return (EXIT_SUCCESS);
}

#undef _print_pp_s
#undef _print_pp_d

/* scan */

gint lprint_json_scan_new(quvi_t q, gpointer *dst)
{
  return (_json_handle_new(q, NULL, dst));
}

void lprint_json_scan_free(gpointer data)
{
  _json_handle_free(data, -1);
}

gint lprint_json_scan_print_buffer(gpointer data)
{
  return (_print_buffer(data));
}

gint lprint_json_scan_properties(quvi_scan_t qs, gpointer data)
{
  const gchar *s;
  gchar *e;
  json_t p;

  g_assert(data != NULL);
  g_assert(qs != NULL);
  p = (json_t) data;

  json_builder_begin_object(p->b); /* root */

  json_builder_set_member_name(p->b, "quvi");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "scan");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "media");
  json_builder_begin_array(p->b);

  while ( (s = quvi_scan_next_media_url(qs)) != NULL)
    {
      json_builder_begin_object(p->b);
      json_builder_set_member_name(p->b, "url");

      e = g_uri_escape_string(s, reserved_chars, FALSE);
      json_builder_add_string_value(p->b, e);

      json_builder_end_object(p->b);
      g_free(e);
    }

  json_builder_end_array(p->b); /* media */
  json_builder_end_object(p->b); /* scan */

  json_builder_end_object(p->b); /* quvi */
  json_builder_end_object(p->b); /* root */

  return (EXIT_SUCCESS);
}

/* subtitle */

gint lprint_json_subtitle_new(quvi_t q, gpointer *dst)
{
  return (_json_handle_new(q, NULL, dst));
}

void lprint_json_subtitle_free(gpointer data)
{
  _json_handle_free(data, -1);
}

gint lprint_json_subtitle_print_buffer(gpointer data)
{
  return (_print_buffer(data));
}

static gint _stp_d(const json_t p, const quvi_subtitle_type_t qst,
                   const QuviSubtitleTypeProperty qstp, const gchar *n)
{
  gdouble d = 0;
  quvi_subtitle_type_get(qst, qstp, &d);
  return (_set_member(p, UTIL_PROPERTY_TYPE_SUBTITLE_LANGUAGE, n, NULL, d));
}

#define _print_stp_d(n)\
  do {\
    if (_stp_d(p, t, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _slp_s(const json_t p, const quvi_subtitle_lang_t qsl,
                   const QuviSubtitleLangProperty qslp, const gchar *n)
{
  gchar *s = NULL;
  quvi_subtitle_lang_get(qsl, qslp, &s);
  return (_set_member(p, UTIL_PROPERTY_TYPE_SUBTITLE_LANGUAGE, n, s, -1));
}

#define _print_slp_s(n)\
  do {\
    if (_slp_s(p, l, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _append_lang_properties(const quvi_subtitle_lang_t l,
                                    const json_t p)
{
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_TRANSLATED);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_ORIGINAL);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_CODE);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_URL);
  _print_slp_s(QUVI_SUBTITLE_LANG_PROPERTY_ID);
  return (EXIT_SUCCESS);
}

gint
lprint_json_subtitle_lang_properties(quvi_subtitle_lang_t l, gpointer data)
{
  json_t p;
  gint r;

  g_assert(data != NULL);
  p = (json_t) data;

  json_builder_begin_object(p->b); /* root */

  json_builder_set_member_name(p->b, "quvi");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "media");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "subtitle");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "language");
  json_builder_begin_object(p->b);

  r = _append_lang_properties(l, p);

  json_builder_end_object(p->b); /* language */
  json_builder_end_object(p->b); /* subtitle */

  json_builder_end_object(p->b); /* media */
  json_builder_end_object(p->b); /* quvi */
  json_builder_end_object(p->b); /* root */

  return (r);
}

static gint _foreach_subtitle_lang(const json_t p,
                                   const quvi_subtitle_type_t t)
{
  quvi_subtitle_lang_t l;
  gint r;

  json_builder_set_member_name(p->b, "languages");
  json_builder_begin_array(p->b);

  r = EXIT_SUCCESS;
  while ( (l = quvi_subtitle_lang_next(t)) != NULL && r ==EXIT_SUCCESS)
    {
      json_builder_begin_object(p->b);
      r = _append_lang_properties(l, p);
      json_builder_end_object(p->b);
    }
  json_builder_end_array(p->b); /* languages */

  return (r);
}

gint lprint_json_subtitles_available(quvi_t q, quvi_subtitle_t qsub)
{
  quvi_subtitle_type_t t;
  json_t p;
  gint r;

  g_assert(qsub != NULL);
  g_assert(q != NULL);

  if (_json_handle_new(q, NULL, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  json_builder_begin_object(p->b); /* root */

  json_builder_set_member_name(p->b, "quvi");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "media");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "subtitle");
  json_builder_begin_object(p->b);

  json_builder_set_member_name(p->b, "types");
  json_builder_begin_array(p->b);

  r = EXIT_SUCCESS;
  while ( (t = quvi_subtitle_type_next(qsub)) != NULL && r ==EXIT_SUCCESS)
    {
      json_builder_begin_object(p->b);
      _print_stp_d(QUVI_SUBTITLE_TYPE_PROPERTY_FORMAT);
      _print_stp_d(QUVI_SUBTITLE_TYPE_PROPERTY_TYPE);
      r = _foreach_subtitle_lang(p, t);
      json_builder_end_object(p->b);
    }

  json_builder_end_array(p->b); /* types */
  json_builder_end_object(p->b); /* subtitle */

  json_builder_end_object(p->b); /* media */
  json_builder_end_object(p->b); /* quvi */
  json_builder_end_object(p->b); /* root */

  if (r == EXIT_SUCCESS)
    r = _print_buffer(p);

  return (_json_handle_free(p, r));
}

#undef _print_slp_s
#undef _print_stp_d

/* vim: set ts=2 sw=2 tw=72 expandtab: */
