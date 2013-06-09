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
#include <glib/gi18n.h>
#include <quvi.h>

#include "linput.h"
#include "lprint.h"
#include "lopts.h"
#include "lutil.h"
#include "setup.h"
#include "opts.h"
#include "sig.h"
#include "cmd.h"

static struct sigaction saw, sao;
static struct linput_s linput;
static struct lopts_s lopts;
extern struct opts_s opts;
static quvi_t q;

static void _foreach_playlist_url(gpointer p, gpointer userdata,
                                  const gchar *url)
{
  lprint_cb_playlist_print_buffer playlist_print_buffer;
  lprint_cb_playlist_properties   playlist_properties;
  lprint_cb_playlist_free         playlist_free;
  lprint_cb_playlist_new          playlist_new;

  lutil_query_properties_t qps;
  quvi_playlist_t qp;
  gpointer h;

  g_assert(userdata != NULL);
  g_assert(p != NULL);

  qps = (lutil_query_properties_t) p;
  qp = (quvi_playlist_t) userdata;

  if (qps->exit_status != EXIT_SUCCESS)
    return;

  /* default. */

  playlist_print_buffer = lprint_rfc2483_playlist_print_buffer;
  playlist_properties   = lprint_rfc2483_playlist_properties;
  playlist_free         = lprint_rfc2483_playlist_free;
  playlist_new          = lprint_rfc2483_playlist_new;

  if (g_strcmp0(opts.core.print_format, "enum") ==0)
    {
      playlist_print_buffer = lprint_enum_playlist_print_buffer;
      playlist_properties   = lprint_enum_playlist_properties;
      playlist_free         = lprint_enum_playlist_free;
      playlist_new          = lprint_enum_playlist_new;
    }

#ifdef HAVE_JSON_GLIB
  if (g_strcmp0(opts.core.print_format, "json") ==0)
    {
      playlist_print_buffer = lprint_json_playlist_print_buffer;
      playlist_properties   = lprint_json_playlist_properties;
      playlist_free         = lprint_json_playlist_free;
      playlist_new          = lprint_json_playlist_new;
    }
#endif

#ifdef HAVE_LIBXML
  if (g_strcmp0(opts.core.print_format, "xml") ==0)
    {
      playlist_print_buffer = lprint_xml_playlist_print_buffer;
      playlist_properties   = lprint_xml_playlist_properties;
      playlist_free         = lprint_xml_playlist_free;
      playlist_new          = lprint_xml_playlist_new;
    }
#endif

  qps->exit_status = playlist_new(qps->q, &h);
  if (qps->exit_status != EXIT_SUCCESS)
    return;

  qps->exit_status = playlist_properties(qp, h);
  if (qps->exit_status == EXIT_SUCCESS)
    qps->exit_status = playlist_print_buffer(h);

  playlist_free(h);
}

static gint _file_ext_from(const lutil_query_properties_t qps,
                           const quvi_http_metainfo_t qmi, gchar **dst)
{
  gchar *s;
  gint r;

  *dst = NULL;

  if (qmi == NULL)
    return (EXIT_SUCCESS);

  r = EXIT_SUCCESS;
  s = NULL;

  quvi_http_metainfo_get(qmi, QUVI_HTTP_METAINFO_PROPERTY_CONTENT_TYPE, &s);
  if (quvi_ok(qps->q) == TRUE)
    {
      quvi_file_ext_t qfe = quvi_file_ext_new(qps->q, s);
      if (quvi_ok(qps->q) == TRUE)
        *dst = g_strdup(quvi_file_ext_get(qfe));
      else
        {
          qps->xperr(_("libquvi: while creating file extension: %s"),
                     quvi_errmsg(qps->q));
          r = EXIT_FAILURE;
        }
      quvi_file_ext_free(qfe);
    }
  else
    {
      qps->xperr(_("libquvi: while getting http metainfo property: %s"),
                 quvi_errmsg(qps->q));
      r = EXIT_FAILURE;
    }
  return (r);
}

static gint _exec_cmd(const lutil_query_properties_t qps,
                      const quvi_http_metainfo_t qmi,
                      const quvi_media_t qm)
{
  struct lutil_exec_opts_s xopts;
  gchar *file_ext;
  gint i, r;

  if (opts.exec.external == NULL)
    return (EXIT_SUCCESS);

  if (_file_ext_from(qps, qmi, &file_ext) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  r = EXIT_SUCCESS;
  i = 0;

  while (opts.exec.external[i] != NULL && r == EXIT_SUCCESS)
    {
      memset(&xopts, 0, sizeof(struct lutil_exec_opts_s));

      xopts.flags.discard_stderr = !opts.exec.enable_stderr;
      xopts.flags.discard_stdout = !opts.exec.enable_stdout;
      xopts.flags.dump_argv = opts.exec.dump_argv;

      xopts.exec_arg = opts.exec.external[i];
      xopts.xperr = qps->xperr;

      xopts.file_ext = file_ext;
      xopts.qm = qm;

      r = lutil_exec_cmd(&xopts);
      ++i;
    }
  g_free(file_ext);
  return (r);
}

static gint _query_metainfo(const lutil_query_properties_t qps,
                            quvi_http_metainfo_t *qmi,
                            const quvi_media_t qm)
{
  *qmi = NULL;
  if (opts.dump.query_metainfo == FALSE)
    return (EXIT_SUCCESS);
  return (lutil_query_metainfo(qps->q, qm, qmi, qps->xperr));
}

static void _foreach_subtitle_url(gpointer p, gpointer userdata,
                                  const gchar *url)
{
  lprint_cb_subtitle_lang_properties subtitle_lang_properties;
  lprint_cb_subtitle_print_buffer subtitle_print_buffer;
  lprint_cb_subtitles_available   subtitles_available;
  lprint_cb_subtitle_free         subtitle_free;
  lprint_cb_subtitle_new          subtitle_new;

  lutil_query_properties_t qps;
  quvi_subtitle_t qsub;
  gpointer h;

  g_assert(userdata != NULL);
  g_assert(p != NULL);

  qps = (lutil_query_properties_t) p;
  qsub = (quvi_subtitle_t) userdata;

  if (qps->exit_status != EXIT_SUCCESS)
    return;

  /* default. */

  subtitle_lang_properties = lprint_rfc2483_subtitle_lang_properties;
  subtitle_print_buffer = lprint_rfc2483_subtitle_print_buffer;
  subtitles_available   = lprint_rfc2483_subtitles_available;
  subtitle_free         = lprint_rfc2483_subtitle_free;
  subtitle_new          = lprint_rfc2483_subtitle_new;

  if (g_strcmp0(opts.core.print_format, "enum") ==0)
    {
      subtitle_lang_properties = lprint_enum_subtitle_lang_properties;
      subtitle_print_buffer = lprint_enum_subtitle_print_buffer;
      subtitles_available   = lprint_enum_subtitles_available;
      subtitle_free         = lprint_enum_subtitle_free;
      subtitle_new          = lprint_enum_subtitle_new;
    }

#ifdef HAVE_JSON_GLIB
  if (g_strcmp0(opts.core.print_format, "json") ==0)
    {
      subtitle_lang_properties = lprint_json_subtitle_lang_properties;
      subtitle_print_buffer = lprint_json_subtitle_print_buffer;
      subtitles_available   = lprint_json_subtitles_available;
      subtitle_free         = lprint_json_subtitle_free;
      subtitle_new          = lprint_json_subtitle_new;
    }
#endif

#ifdef HAVE_LIBXML
  if (g_strcmp0(opts.core.print_format, "xml") ==0)
    {
      subtitle_lang_properties = lprint_xml_subtitle_lang_properties;
      subtitle_print_buffer = lprint_xml_subtitle_print_buffer;
      subtitles_available   = lprint_xml_subtitles_available;
      subtitle_free         = lprint_xml_subtitle_free;
      subtitle_new          = lprint_xml_subtitle_new;
    }
#endif

  if (opts.core.print_subtitles == FALSE)
    {
      const gchar *lang;
      quvi_subtitle_t l;

      /* Choose the subtitle, otherwise use the default. */

      lang = (opts.core.subtitle_language != NULL)
             ? opts.core.subtitle_language
             : "default"; /* Assumes nothing matches this in the array. */

      qps->exit_status =
        lutil_choose_subtitle(qps->q, qsub, lang, qps->xperr, &l, TRUE);

      if (qps->exit_status != EXIT_SUCCESS)
        return;

      qps->exit_status = subtitle_new(qps->q, &h);
      if (qps->exit_status != EXIT_SUCCESS)
        return;

      qps->exit_status = subtitle_lang_properties(l, h);
      if (qps->exit_status == EXIT_SUCCESS)
        qps->exit_status = subtitle_print_buffer(h);

      subtitle_free(h);
    }
  else
    qps->exit_status = subtitles_available(qps->q, qsub);
}

static void _foreach_media_url(gpointer p, gpointer userdata,
                               const gchar *url)
{
  lprint_cb_media_streams_available media_streams_available;
  lprint_cb_media_stream_properties media_stream_properties;
  lprint_cb_media_print_buffer      media_print_buffer;
  lprint_cb_media_properties        media_properties;
  lprint_cb_media_free              media_free;
  lprint_cb_media_new               media_new;

  lutil_query_properties_t qps;
  quvi_media_t qm;

  g_assert(userdata != NULL);
  g_assert(p != NULL);

  qps = (lutil_query_properties_t) p;
  qm = (quvi_media_t) userdata;

  if (qps->exit_status != EXIT_SUCCESS)
    return;

  /* default. */

  media_streams_available = lprint_enum_media_streams_available;
  media_stream_properties = lprint_enum_media_stream_properties;
  media_print_buffer      = lprint_enum_media_print_buffer;
  media_properties        = lprint_enum_media_properties;
  media_free              = lprint_enum_media_free;
  media_new               = lprint_enum_media_new;

  if (g_strcmp0(opts.core.print_format, "rfc2483") ==0)
    {
      media_streams_available = lprint_rfc2483_media_streams_available;
      media_stream_properties = lprint_rfc2483_media_stream_properties;
      media_print_buffer      = lprint_rfc2483_media_print_buffer;
      media_properties        = lprint_rfc2483_media_properties;
      media_free              = lprint_rfc2483_media_free;
      media_new               = lprint_rfc2483_media_new;
    }

#ifdef HAVE_JSON_GLIB
  if (g_strcmp0(opts.core.print_format, "json") ==0)
    {
      media_streams_available = lprint_json_media_streams_available;
      media_stream_properties = lprint_json_media_stream_properties;
      media_print_buffer      = lprint_json_media_print_buffer;
      media_properties        = lprint_json_media_properties;
      media_free              = lprint_json_media_free;
      media_new               = lprint_json_media_new;
    }
#endif

#ifdef HAVE_LIBXML
  if (g_strcmp0(opts.core.print_format, "xml") ==0)
    {
      media_streams_available = lprint_xml_media_streams_available;
      media_stream_properties = lprint_xml_media_stream_properties;
      media_print_buffer      = lprint_xml_media_print_buffer;
      media_properties        = lprint_xml_media_properties;
      media_free              = lprint_xml_media_free;
      media_new               = lprint_xml_media_new;
    }
#endif

  if (opts.core.print_streams == FALSE)
    {
      quvi_http_metainfo_t qmi;
      gpointer h;

      qps->exit_status = media_new(qps->q, qm, &h);
      if (qps->exit_status != EXIT_SUCCESS)
        return;

      /* Choose the stream, otherwise use the default. */

      if (opts.core.stream != NULL)
        {
          qps->exit_status = lutil_choose_stream(qps->q, qm, opts.core.stream,
                                                 qps->xperr);
          if (qps->exit_status != EXIT_SUCCESS)
            {
              media_free(h);
              return;
            }
        }

      /* Query HTTP metainfo (if at all). */

      qps->exit_status = _query_metainfo(qps, &qmi, qm);
      if (qps->exit_status != EXIT_SUCCESS)
        {
          quvi_http_metainfo_free(qmi);
          media_free(h);
          return;
        }

      /* Dump the properties. */

      qps->exit_status = media_properties(h);
      if (qps->exit_status == EXIT_SUCCESS)
        {
          qps->exit_status = media_stream_properties(qmi, h);
          if (qps->exit_status == EXIT_SUCCESS)
            qps->exit_status = media_print_buffer(h);
        }

      if (qps->exit_status == EXIT_SUCCESS)
        qps->exit_status = _exec_cmd(qps, qmi, qm);

      quvi_http_metainfo_free(qmi);
      media_free(h);
    }
  else
    qps->exit_status = media_streams_available(qps->q, qm);
}

static gint _cleanup(const gint r)
{
  sigwinch_reset(&sao);
  linput_free(&linput);
  quvi_free(q);
  return (r);
}

gint cmd_dump(gint argc, gchar **argv)
{
  lutil_cb_printerr xperr;
  struct setup_query_s sq;

  if (setup_opts(argc, argv, &lopts) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (lutil_parse_input(&linput, (const gchar**) opts.rargs) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (setup_quvi(&q) != EXIT_SUCCESS)
    {
      linput_free(&linput);
      return (EXIT_FAILURE);
    }

  /* Check {media,playlist} URL support. */

  xperr = lprint_enum_errmsg; /* rfc2483 uses this also. */

#ifdef HAVE_JSON_GLIB
  if (g_strcmp0(opts.core.print_format, "json") ==0)
    xperr = lprint_json_errmsg;
#endif
#ifdef HAVE_LIBXML
  if (g_strcmp0(opts.core.print_format, "xml") ==0)
    xperr = lprint_xml_errmsg;
#endif

  memset(&sq, 0, sizeof(struct setup_query_s));

  sq.force_subtitle_mode =
    (opts.core.print_subtitles || opts.core.subtitle_language != NULL)
    ? TRUE
    : FALSE;

  sq.activity.playlist = _foreach_playlist_url;
  sq.activity.subtitle = _foreach_subtitle_url;
  sq.activity.media = _foreach_media_url;

  sq.perr = lutil_print_stderr_unless_quiet;
  sq.xperr = xperr;

  sq.linput = &linput;
  sq.q = q;

  sigwinch_setup(&saw, &sao);

  return (_cleanup(setup_query(&sq)));
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
