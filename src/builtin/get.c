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
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <quvi.h>

#include "lprint.h"
#include "linput.h"
#include "lopts.h"
#include "lutil.h"
#include "lget.h"
#include "setup.h"
#include "opts.h"
#include "sig.h"
#include "cmd.h"

static struct sigaction saw, sao;
static struct linput_s linput;
static struct lopts_s lopts;
extern struct opts_s opts;
static quvi_t q;

static gint _write_subtitle(quvi_subtitle_lang_t ql,
                            quvi_subtitle_export_t qse,
                            const gchar *mfpath,
                            lutil_cb_printerr xperr)
{
  gchar *fname, *fpath, *tmp;
  const gchar *data, *s;
  lutil_regex_op_t rx;
  GError *e;
  gsize n;
  gint r;

  /*
   * Produce the output fpath for the subtitle file by using the media
   * fpath as a template: replace the media file extension with the
   * subtitle file extension.
   *
   * The '%x' is being used only to pass the lutil_regex_op_new regular
   * expression validation step. This could be anything matching '%\w'.
   */

  tmp = g_strdup_printf("%%x:s/\\.\\w+$/.%s/",
                        opts.core.subtitle_export_format);

  rx = lutil_regex_op_new(tmp, NULL);
  g_free(tmp);

  tmp = g_path_get_basename(mfpath);
  fpath = lutil_regex_op_apply(rx, tmp);

  lutil_regex_op_free(rx);
  g_free(tmp);

  fname = g_path_get_basename(fpath);

  data = quvi_subtitle_export_data(qse);
  n = strlen(data);

  quvi_subtitle_lang_get(ql, QUVI_SUBTITLE_LANG_PROPERTY_ID, &s);

  g_print(_("file: %s  [subtitle]\n"), fname);
  g_print(_("  content length: %"G_GSIZE_FORMAT" bytes"), n);
  g_print(_("  type/language: %s"), s);
  g_print(_("  mode: write\n"));

  r = EXIT_SUCCESS;
  e = NULL;

  if (g_file_set_contents(fpath, data, n, &e) == FALSE)
    {
      xperr(_("while writing: %s: %s"), fpath, e->message);
      g_error_free(e);
      r = EXIT_FAILURE;
    }
  g_free(fname);
  g_free(fpath);

  return (r);
}

static GSList *_subtitle_languages(quvi_subtitle_t qsub)
{
  quvi_subtitle_type_t t;
  quvi_subtitle_lang_t l;
  const gchar *s;
  GSList *r;

  g_assert(qsub != NULL);

  r = NULL;
  while ( (t = quvi_subtitle_type_next(qsub)) != NULL)
    {
      while ( (l = quvi_subtitle_lang_next(t)) != NULL)
        {
          quvi_subtitle_lang_get(l, QUVI_SUBTITLE_LANG_PROPERTY_ID, &s);
          r = g_slist_prepend(r, g_strdup(s));
        }
    }
  return (g_slist_reverse(r));
}

struct foreach_s
{
  const gchar *f;
  GString *s;
  gint i;
  gint n;
};

typedef struct foreach_s *foreach_t;

static void _foreach_str(gpointer p, gpointer userdata)
{
  foreach_t m = (foreach_t) userdata;

  g_string_append_printf(m->s, m->f, (gchar*) p);

  if (++(m->i) == m->n)
    {
      g_string_append(m->s, "\n  ");
      m->i = 0;
    }
}

static gchar *_str_list_to_s(GSList *l, const gchar *f, const gint n)
{
  struct foreach_s m;
  gchar *r;

  memset(&m, 0, sizeof(struct foreach_s));
  m.s = g_string_new(NULL);
  m.f = f;
  m.n = n;

  g_slist_foreach(l, _foreach_str, &m);
  r = m.s->str;

  g_string_free(m.s, FALSE);
  return (r);
}

static void _dump_languages(quvi_subtitle_t qsub)
{
  GSList *l;
  gchar *s;

  l = _subtitle_languages(qsub);
  s = _str_list_to_s(l, "%10s  ", 6);

  g_print(_("subtitles (found):\n  %s\n"), (strlen(s) ==0) ? _("none"):s);

  lutil_slist_free_full(l, (GFunc) g_free);
  g_free(s);
}

static gint _copy_subtitle(const gchar *mfpath, const gchar *url,
                           lutil_cb_printerr xperr)
{
  quvi_subtitle_lang_t ql;
  quvi_subtitle_t qsub;
  gint r;

  if (opts.core.subtitle_language == NULL)
    return (EXIT_SUCCESS);

  qsub = quvi_subtitle_new(q, url);
  if (quvi_ok(q) == FALSE)
    {
      xperr(_("libquvi: while querying subtitles: %s"), quvi_errmsg(q));
      return (EXIT_FAILURE);
    }

  _dump_languages(qsub);

  r = lutil_choose_subtitle(q, qsub, opts.core.subtitle_language,
                            xperr, &ql, FALSE);
  if (r == EXIT_SUCCESS)
    {
      if (ql != NULL)
        {
          quvi_subtitle_export_t qse;

          qse = quvi_subtitle_export_new(ql, opts.core.subtitle_export_format);
          if (quvi_ok(q) == TRUE)
            r = _write_subtitle(ql, qse, mfpath, xperr);
          else
            xperr(_("libquvi: while exporting subtitle: %s"), quvi_errmsg(q));

          quvi_subtitle_export_free(qse);
        }
      else
        g_print(_("skip <transfer>: subtitle extraction\n"));
    }
  quvi_subtitle_free(qsub);

  return (r);
}

static void _copy_media_stream(lutil_query_properties_t qps, quvi_media_t qm,
                               const gchar *url)
{
  struct lutil_build_fpath_s b;
  struct lget_s g;

  memset(&b, 0, sizeof(struct lutil_build_fpath_s));
  memset(&g, 0, sizeof(struct lget_s));

  b.output_regex = opts.get.output_regex;
  b.output_file = opts.get.output_file;
  b.output_name = opts.get.output_name;
  b.output_dir = opts.get.output_dir;
  b.xperr = qps->xperr;
  b.qm = qm;

  g.build_fpath = &b;
  g.xperr = qps->xperr;
  g.qm = qm;
  g.q = q;

  g.opts.overwrite_if_exists = opts.get.overwrite;
  g.opts.skip_transfer = opts.get.skip_transfer;
  g.opts.resume_from = opts.get.resume_from;
  g.opts.stream = opts.core.stream;

  g.opts.exec.external = (const gchar**) opts.exec.external;
  g.opts.exec.enable_stderr = opts.exec.enable_stderr;
  g.opts.exec.enable_stdout = opts.exec.enable_stdout;
  g.opts.exec.dump_argv = opts.exec.dump_argv;

  qps->exit_status = lget_new(&g);

  if (qps->exit_status == EXIT_SUCCESS)
    qps->exit_status = _copy_subtitle(g.result.fpath, url, qps->xperr);

  lget_free(&g);
}

static GSList *_media_streams(quvi_media_t qm)
{
  const gchar *s;
  GSList *r;

  r = NULL;
  while (quvi_media_stream_next(qm) == QUVI_TRUE)
    {
      quvi_media_get(qm, QUVI_MEDIA_STREAM_PROPERTY_ID, &s);
      if (strlen(s) >0)
        r = g_slist_prepend(r, g_strdup(s));
    }

  if (g_slist_length(r) ==0)
    r = g_slist_prepend(r, g_strdup(_("default")));

  return (g_slist_reverse(r));
}

static void _print_streams(lutil_query_properties_t qps,  quvi_media_t qm)
{
  GSList *l;
  gchar *s;

  l = _media_streams(qm);
  s = _str_list_to_s(l, "%22s  ", 3);

  g_print(_("streams (found):\n  %s\n"), s);

  lutil_slist_free_full(l, (GFunc) g_free);
  g_free(s);
}

static void _foreach_media_url(gpointer p, gpointer userdata,
                               const gchar *url)
{
  lutil_query_properties_t qps;
  quvi_media_t qm;

  g_assert(userdata != NULL);
  g_assert(p != NULL);

  qps = (lutil_query_properties_t) p;
  qm = (quvi_media_t) userdata;

  if (qps->exit_status != EXIT_SUCCESS)
    return;

  if (opts.core.print_streams == FALSE)
    _copy_media_stream(qps, qm, url);
  else
    _print_streams(qps, qm);
}

static void _foreach_playlist_url(gpointer p, gpointer userdata,
                                  const gchar *url)
{
  lutil_query_properties_t qps;
  quvi_playlist_t qp;
  quvi_media_t qm;
  gchar *m_url;

  g_assert(userdata != NULL);
  g_assert(p != NULL);

  qps = (lutil_query_properties_t) p;
  qp = (quvi_playlist_t) userdata;

  if (qps->exit_status != EXIT_SUCCESS)
    return;

  while (quvi_playlist_media_next(qp) == QUVI_TRUE)
    {
      quvi_playlist_get(qp, QUVI_PLAYLIST_MEDIA_PROPERTY_URL, &m_url);
      qm = quvi_media_new(qps->q, m_url);

      _foreach_media_url(qps, qm, m_url);
      quvi_media_free(qm);

      if (qps->exit_status != EXIT_SUCCESS)
        return;
    }
}

static void _foreach_subtitle_url(gpointer p, gpointer userdata,
                                  const gchar *url)
{
  lutil_query_properties_t qps;
  quvi_subtitle_t qsub;

  g_assert(userdata != NULL);
  g_assert(p != NULL);

  qps = (lutil_query_properties_t) p;
  qsub = (quvi_subtitle_t) userdata;

  if (qps->exit_status != EXIT_SUCCESS)
    return;

  _dump_languages(qsub);
}

static gint _cleanup(const gint r)
{
  sigwinch_reset(&sao);
  linput_free(&linput);
  quvi_free(q);
  return (r);
}

gint cmd_get(gint argc, gchar **argv)
{
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

  memset(&sq, 0, sizeof(struct setup_query_s));

  sq.force_subtitle_mode = opts.core.print_subtitles;

  sq.activity.playlist = _foreach_playlist_url;
  sq.activity.subtitle = _foreach_subtitle_url;
  sq.activity.media = _foreach_media_url;

  sq.perr = lutil_print_stderr_unless_quiet;
  sq.xperr = lprint_enum_errmsg;

  sq.linput = &linput;
  sq.q = q;

  sigwinch_setup(&saw, &sao);

  return (_cleanup(setup_query(&sq)));
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
