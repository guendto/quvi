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

#include <libxml/xmlwriter.h>
#include <libxml/uri.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <quvi.h>

#include "lutil.h"
/* -- */
#include "lprint.h"

struct xml_s
{
  xmlTextWriterPtr w;
  quvi_media_t qm;
  xmlDocPtr d;
  quvi_t q;
};

typedef struct xml_s *xml_t;

#define _chk_r_e(c)\
  do {\
    const gint r = c;\
    if (r != EXIT_SUCCESS)\
      return (_xml_handle_free(p, r));\
    else\
      return (EXIT_SUCCESS);\
  } while (0)

#define _chk_r(c)\
  do {\
    const gint r = c;\
    if (r != EXIT_SUCCESS)\
      return (_xml_handle_free(p, r));\
  } while (0)

static gint _xml_handle_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  xml_t p;

  g_assert(dst != NULL);

  p = g_new0(struct xml_s, 1);
  p->qm = qm;
  p->q = q;

  p->w = xmlNewTextWriterDoc(&p->d, 0);
  if (p->w == NULL)
    {
      lprint_xml_errmsg(_("while creating the XML writer"));
      xmlFreeDoc(p->d);
      g_free(p);
      return (EXIT_FAILURE);
    }

  if (xmlTextWriterStartDocument(p->w, NULL, "UTF-8", NULL) <0)
    {
      lprint_xml_errmsg(_("while starting the XML document"));
      xmlFreeDoc(p->d);
      g_free(p);
      return (EXIT_FAILURE);
    }
  *dst = p;

  return (EXIT_SUCCESS);
}

static gint _xml_handle_free(gpointer data, const gint r)
{
  xml_t p = (xml_t) data;

  if (p == NULL)
    return (r);

  if (p->w != NULL)
    {
      xmlTextWriterEndDocument(p->w);
      xmlFreeTextWriter(p->w);
    }

  if (p->d != NULL)
    xmlFreeDoc(p->d);

  g_free(p);
  return (r);
}

typedef enum {START_R, END_R, ATTR, START_E, END_E} ErrorMessage;

static const gchar *_msg[]=
{
  N_("while starting the XML document root element `%s'"),
  N_("while ending the XML document root element `%s'"),
  N_("while writing the XML attribute `%s'"),
  N_("while starting the XML element `%s'"),
  N_("while ending the XML element `%s'"),
  NULL
};

static gint _start_e(const xml_t p, const ErrorMessage e, const gchar *w)
{
  gint r = EXIT_SUCCESS;
  if (xmlTextWriterStartElement(p->w, BAD_CAST w) <0)
    {
      lprint_xml_errmsg("%s", g_dgettext(GETTEXT_PACKAGE, _msg[e]), w);
      r = EXIT_FAILURE;
    }
  return (r);
}

static gint _end_e(const xml_t p, const ErrorMessage e, const gchar *w)
{
  gint r = EXIT_SUCCESS;
  if (xmlTextWriterEndElement(p->w) <0)
    {
      lprint_xml_errmsg("%s", g_dgettext(GETTEXT_PACKAGE, _msg[e]), w);
      r = EXIT_FAILURE;
    }
  return (r);
}

extern const gchar *reserved_chars;

static gint _write_attr(const xml_t p, const gchar *n, const gchar *s)
{
  xmlChar *e;
  gint r;

  e = xmlURIEscapeStr(BAD_CAST s, BAD_CAST reserved_chars);


  r = (xmlTextWriterWriteAttribute(p->w, BAD_CAST n, e) <0)
      ? EXIT_FAILURE
      : EXIT_SUCCESS;

  xmlFree(e);

  if (r != EXIT_SUCCESS)
    lprint_xml_errmsg("%s", g_dgettext(GETTEXT_PACKAGE, _msg[ATTR]), n);

  return (r);
}

static gint _attr_new(const xml_t p, const lutilPropertyType pt,
                      const gchar *n, const gchar *s, const gdouble d)
{
  gint r;

  r = lutil_chk_property_ok(p->q, pt, n, lprint_xml_errmsg);
  if (r != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (s != NULL)
    r = _write_attr(p, n, s);
  else
    {
      gchar *t = g_strdup_printf("%.0f", d);
      r = _write_attr(p, n, t);
      g_free(t);
    }
  return (r);
}

static gint _print_buffer(xml_t p)
{
  xmlChar *b;

  g_assert(p != NULL);

  xmlTextWriterFlush(p->w);
  xmlDocDumpFormatMemory(p->d, &b, NULL, 0);

  if (b == NULL)
    {
      lprint_xml_errmsg(_("while dumping the XML document"));
      return (EXIT_FAILURE);
    }
  g_print("%s", b);
  xmlFree(b);

  return (EXIT_SUCCESS);
}

void lprint_xml_errmsg(const gchar *fmt, ...)
{
  va_list args;
  gchar *s;

  va_start(args, fmt);
  if (g_vasprintf(&s, fmt, args) >0)
    {
      xmlChar *e = xmlURIEscapeStr(BAD_CAST s, BAD_CAST reserved_chars);
      g_printerr("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                 "<error message=\"%s\" />", e);
      xmlFree(e);
      g_free(s);
    }
  va_end(args);
}

typedef lprint_cb_errmsg cem;

/* media */

gint lprint_xml_media_new(quvi_t q, quvi_media_t qm, gpointer *dst)
{
  return (_xml_handle_new(q, qm, dst));
}

void lprint_xml_media_free(gpointer data)
{
  _xml_handle_free(data, -1);
}

gint lprint_xml_media_print_buffer(gpointer data)
{
  xml_t p = (xml_t) data;

  g_assert(data != NULL);
  g_assert(p->w != NULL);
  g_assert(p->d != NULL);

  if (_end_e(p, END_E, "media") != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (_end_e(p, END_R, "quvi") != EXIT_SUCCESS)
    return (EXIT_FAILURE);
  else
    return (_print_buffer(p));
}

static gint _mp_attr_s(const xml_t p, const QuviMediaProperty qmp,
                       const gchar *n)
{
  gchar *s = NULL;
  quvi_media_get(p->qm, qmp, &s);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_MEDIA, n, s, -1));
}

#define _print_mp_attr_s(n)\
  do {\
    if (_mp_attr_s(p, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

#define _print_mp_attr_set_r_s(n)\
  do {\
    r = _mp_attr_s(p, n, #n);\
  } while (0)

static gint _mp_attr_d(const xml_t p, const QuviMediaProperty qmp,
                       const gchar *n)
{
  gdouble d = 0;
  quvi_media_get(p->qm, qmp, &d);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_MEDIA, n, NULL, d));
}

#define _print_mp_attr_d(n)\
  do {\
    if (_mp_attr_d(p, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mi_attr_s(const xml_t p, const quvi_http_metainfo_t qmi,
                       const QuviHTTPMetaInfoProperty qmip, const gchar *n)
{
  gchar *s = NULL;
  quvi_http_metainfo_get(qmi, qmip, &s);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_HTTP_METAINFO, n, s, -1));
}

#define _print_mi_attr_s(n)\
  do {\
    if (_mi_attr_s(p, qmi, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _mi_attr_d(const xml_t p, const quvi_http_metainfo_t qmi,
                       const QuviHTTPMetaInfoProperty qmip, const gchar *n)
{
  gdouble d = 0;
  quvi_http_metainfo_get(qmi, qmip, &d);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_HTTP_METAINFO, n, NULL, d));
}

#define _print_mi_attr_d(n)\
  do {\
    if (_mi_attr_d(p, qmi, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint
_print_media_stream_properties(const xml_t p, const quvi_http_metainfo_t qmi)
{
  g_assert(p->w != NULL);
  g_assert(p->d != NULL);

  _chk_r(_start_e(p, START_E, "stream"));

  _print_mp_attr_s(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_ENCODING);
  _print_mp_attr_s(QUVI_MEDIA_STREAM_PROPERTY_AUDIO_ENCODING);
  _print_mp_attr_s(QUVI_MEDIA_STREAM_PROPERTY_CONTAINER);
  _print_mp_attr_s(QUVI_MEDIA_STREAM_PROPERTY_URL);
  _print_mp_attr_s(QUVI_MEDIA_STREAM_PROPERTY_ID);

  _print_mp_attr_d(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_BITRATE_KBIT_S);
  _print_mp_attr_d(QUVI_MEDIA_STREAM_PROPERTY_AUDIO_BITRATE_KBIT_S);
  _print_mp_attr_d(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_HEIGHT);
  _print_mp_attr_d(QUVI_MEDIA_STREAM_PROPERTY_VIDEO_WIDTH);

  if (qmi != NULL)
    {
      _print_mi_attr_s(QUVI_HTTP_METAINFO_PROPERTY_FILE_EXTENSION);
      _print_mi_attr_s(QUVI_HTTP_METAINFO_PROPERTY_CONTENT_TYPE);
      _print_mi_attr_d(QUVI_HTTP_METAINFO_PROPERTY_LENGTH_BYTES);
    }
  return (_end_e(p, END_E, "stream"));
}

#undef _print_mi_s
#undef _print_mi_d

gint
lprint_xml_media_stream_properties(quvi_http_metainfo_t qmi, gpointer data)
{
  g_assert(data != NULL);
  return (_print_media_stream_properties(data, qmi));
}

gint lprint_xml_media_streams_available(quvi_t q, quvi_media_t qm)
{
  xml_t p;

  if (_xml_handle_new(q, qm, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  _chk_r(_start_e(p, START_R, "quvi"));
  _chk_r(_start_e(p, START_E, "media"));
  _chk_r(_start_e(p, START_E, "streams"));

  while (quvi_media_stream_next(qm) == QUVI_TRUE)
    {
      _chk_r(_print_media_stream_properties(p, NULL));
    }

  _chk_r(_end_e(p, END_E, "streams"));
  _chk_r(_end_e(p, END_E, "media"));
  _chk_r(_end_e(p, END_R, "quvi"));

  _chk_r(_print_buffer(p));

  return (_xml_handle_free(p, EXIT_SUCCESS));
}

gint lprint_xml_media_properties(gpointer data)
{
  xml_t p = (xml_t) data;

  g_assert(data != NULL);
  g_assert(p->w != NULL);
  g_assert(p->d != NULL);

  _chk_r(_start_e(p, START_R, "quvi"));
  _chk_r(_start_e(p, START_E, "media"));

  _print_mp_attr_s(QUVI_MEDIA_PROPERTY_THUMBNAIL_URL);
  _print_mp_attr_s(QUVI_MEDIA_PROPERTY_TITLE);
  _print_mp_attr_s(QUVI_MEDIA_PROPERTY_ID);

  _print_mp_attr_d(QUVI_MEDIA_PROPERTY_START_TIME_MS);
  _print_mp_attr_d(QUVI_MEDIA_PROPERTY_DURATION_MS);

  return (EXIT_SUCCESS);
}

#undef _print_mp_s
#undef _print_mp_d

/* playlist */

gint lprint_xml_playlist_new(quvi_t q, gpointer *dst)
{
  return (_xml_handle_new(q, NULL, dst));
}

void lprint_xml_playlist_free(gpointer dst)
{
  _xml_handle_free(dst, -1);
}

gint lprint_xml_playlist_print_buffer(gpointer data)
{
  return (_print_buffer(data));
}

static gint _pp_attr_s(const xml_t p, const quvi_playlist_t qp,
                       const QuviPlaylistProperty qpp, const gchar *n)
{
  gchar *s = NULL;
  quvi_playlist_get(qp, qpp, &s);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, s, -1));
}

#define _print_pp_attr_s(n)\
  do {\
    if (_pp_attr_s(p, qp, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _pp_attr_d(const xml_t p, const quvi_playlist_t qp,
                       const QuviPlaylistProperty qpp, const gchar *n)
{
  gdouble d = 0;
  quvi_playlist_get(qp, qpp, &d);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_PLAYLIST, n, NULL, d));
}

#define _print_pp_attr_d(n)\
  do {\
    if (_pp_attr_d(p, qp, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

gint lprint_xml_playlist_properties(quvi_playlist_t qp, gpointer data)
{
  xml_t p = (xml_t) data;

  g_assert(data != NULL);
  g_assert(qp != NULL);
  g_assert(p->w != NULL);
  g_assert(p->d != NULL);

  _chk_r(_start_e(p, START_R, "quvi"));
  _chk_r(_start_e(p, START_E, "playlist"));

  _print_pp_attr_s(QUVI_PLAYLIST_PROPERTY_THUMBNAIL_URL);
  _print_pp_attr_s(QUVI_PLAYLIST_PROPERTY_TITLE);
  _print_pp_attr_s(QUVI_PLAYLIST_PROPERTY_ID);

  while (quvi_playlist_media_next(qp) == QUVI_TRUE)
    {
      _chk_r(_start_e(p, START_E, "media"));
      _print_pp_attr_d(QUVI_PLAYLIST_MEDIA_PROPERTY_DURATION_MS);
      _print_pp_attr_s(QUVI_PLAYLIST_MEDIA_PROPERTY_TITLE);
      _print_pp_attr_s(QUVI_PLAYLIST_MEDIA_PROPERTY_URL);
      _chk_r(_end_e(p, END_E, "media"));
    }

  _chk_r(_end_e(p, END_E, "playlist"));
  _chk_r_e(_end_e(p, END_R, "quvi"));
}

#undef _print_pp_s
#undef _print_pp_d

/* scan */

gint lprint_xml_scan_new(quvi_t q, gpointer *dst)
{
  return (_xml_handle_new(q, NULL, dst));
}

void lprint_xml_scan_free(gpointer data)
{
  _xml_handle_free(data, -1);
}

gint lprint_xml_scan_print_buffer(gpointer data)
{
  return (_print_buffer(data));
}

gint lprint_xml_scan_properties(quvi_scan_t qs, gpointer data)
{
  const gchar *s;
  xml_t p;

  g_assert(data != NULL);
  g_assert(qs != NULL);

  p = (xml_t) data;

  _chk_r(_start_e(p, START_R, "quvi"));
  _chk_r(_start_e(p, START_E, "scan"));

  while ( (s = quvi_scan_next_media_url(qs)) != NULL)
    {
      _chk_r(_start_e(p, START_E, "media"));
      _chk_r(_write_attr(p, "url", s));
      _chk_r(_end_e(p, END_E, "media"));
    }

  _chk_r(_end_e(p, END_E, "scan"));
  _chk_r_e(_end_e(p, END_R, "quvi"));
}

/* subtitle */

gint lprint_xml_subtitle_new(quvi_t q, gpointer *dst)
{
  return (_xml_handle_new(q, NULL, dst));
}

void lprint_xml_subtitle_free(gpointer data)
{
  _xml_handle_free(data, -1);
}

gint lprint_xml_subtitle_print_buffer(gpointer data)
{
  return (_print_buffer(data));
}

static gint _stp_attr_d(const xml_t p, const quvi_subtitle_type_t qst,
                        const QuviSubtitleTypeProperty qstp, const gchar *n)
{
  gdouble d = 0;
  quvi_subtitle_type_get(qst, qstp, &d);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_SUBTITLE_TYPE, n, NULL, d));
}

#define _print_stp_attr_d(n)\
  do {\
    if (_stp_attr_d(p, t, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _slp_attr_s(const xml_t p, const quvi_subtitle_lang_t qsl,
                        const QuviSubtitleLangProperty qslp, const gchar *n)
{
  gchar *s = NULL;
  quvi_subtitle_lang_get(qsl, qslp, &s);
  _chk_r_e(_attr_new(p, UTIL_PROPERTY_TYPE_SUBTITLE_LANGUAGE, n, s, -1));
}

#define _print_slp_attr_s(n)\
  do {\
    if (_slp_attr_s(p, l, n, #n) != EXIT_SUCCESS)\
      return (EXIT_FAILURE);\
  } while (0)

static gint _write_lang_properties(const quvi_subtitle_lang_t l,
                                   const xml_t p)
{
  _chk_r(_start_e(p, START_E, "language"));

  _print_slp_attr_s(QUVI_SUBTITLE_LANG_PROPERTY_TRANSLATED);
  _print_slp_attr_s(QUVI_SUBTITLE_LANG_PROPERTY_ORIGINAL);
  _print_slp_attr_s(QUVI_SUBTITLE_LANG_PROPERTY_CODE);
  _print_slp_attr_s(QUVI_SUBTITLE_LANG_PROPERTY_URL);
  _print_slp_attr_s(QUVI_SUBTITLE_LANG_PROPERTY_ID);

  _chk_r_e(_end_e(p, END_E, "language"));
}

gint
lprint_xml_subtitle_lang_properties(quvi_subtitle_lang_t l, gpointer data)
{
  xml_t p = (xml_t) data;

  g_assert(p->w != NULL);
  g_assert(p->d != NULL);

  _chk_r(_start_e(p, START_R, "quvi"));
  _chk_r(_start_e(p, START_E, "media"));
  _chk_r(_start_e(p, START_E, "subtitle"));
  _chk_r(_write_lang_properties(l, p));
  _chk_r(_end_e(p, END_E, "subtitle"));
  _chk_r(_end_e(p, END_E, "media"));
  _chk_r_e(_end_e(p, END_R, "quvi"));
}

gint lprint_xml_subtitles_available(quvi_t q, quvi_subtitle_t qsub)
{
  quvi_subtitle_type_t t;
  quvi_subtitle_lang_t l;
  xml_t p;

  g_assert(qsub != NULL);
  g_assert(q != NULL);

  if (_xml_handle_new(q, NULL, (gpointer*) &p) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  _chk_r(_start_e(p, START_R, "quvi"));
  _chk_r(_start_e(p, START_E, "media"));
  _chk_r(_start_e(p, START_E, "subtitles"));

  while ( (t = quvi_subtitle_type_next(qsub)) != NULL)
    {
      _chk_r(_start_e(p, START_E, "type"));

      _print_stp_attr_d(QUVI_SUBTITLE_TYPE_PROPERTY_FORMAT);
      _print_stp_attr_d(QUVI_SUBTITLE_TYPE_PROPERTY_TYPE);

      while ( (l = quvi_subtitle_lang_next(t)) != NULL)
        {
          _chk_r(_write_lang_properties(l, p));
        }
      _chk_r(_end_e(p, END_E, "type"));
    }

  _chk_r(_end_e(p, END_E, "subtitles"));
  _chk_r(_end_e(p, END_E, "media"));
  _chk_r(_end_e(p, END_R, "quvi"));

  _chk_r(_print_buffer(p));

  return (_xml_handle_free(p, EXIT_SUCCESS));
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
