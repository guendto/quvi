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
#include <glib/gi18n.h>
#include <quvi.h>
#include <curl/curl.h>

#include "lutil.h"
#include "lpbar.h"
#include "lget.h"

static gboolean force_skip_transfer = FALSE;
static gboolean transfer_skipped = FALSE;
static quvi_http_metainfo_t qmi = NULL;
static struct lutil_file_open_s fo;
static gdouble content_length = 0;
static gchar *content_type = NULL;
static gchar *io_errmsg = NULL;
static CURLcode curl_code = 0;
static lpbar_t pbar = NULL;
static lget_t g = NULL;
static CURL *c = NULL;

static gdouble _content_length_from_qmi()
{
  gdouble l = 0;
  quvi_http_metainfo_get(qmi, QUVI_HTTP_METAINFO_PROPERTY_LENGTH_BYTES, &l);
  return (l);
}

static gchar *_content_type_from_qmi()
{
  gchar *s = NULL;
  quvi_http_metainfo_get(qmi, QUVI_HTTP_METAINFO_PROPERTY_CONTENT_TYPE, &s);
  return (g_strdup(s));
}

static gdouble _content_length_from_c()
{
  gdouble l = 0;
  curl_easy_getinfo(c, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &l);
  return (l);
}

static gchar *_content_type_from_c()
{
  gchar *s = NULL;
  curl_easy_getinfo(c, CURLINFO_CONTENT_TYPE, &s);
  return (g_strdup(s));
}

typedef enum {HTTP_HEAD_RESPONSE, HTTP_GET_RESPONSE} HttpResponseType;

static void _content_props_from(const HttpResponseType hrt)
{
  g_free(content_type);
  if (hrt == HTTP_HEAD_RESPONSE)
    {
      content_length = _content_length_from_qmi();
      content_type = _content_type_from_qmi();
    }
  else
    {
      content_length = _content_length_from_c();
      content_type = _content_type_from_c();
    }
  pbar->content_type = g_strdup(content_type);
  pbar->content_bytes = content_length;
}

static gint _cleanup(const gint r)
{
  quvi_http_metainfo_free(qmi);
  qmi = NULL;

  g_free(content_type);
  content_type = NULL;

  lpbar_free(pbar);
  pbar = NULL;

  if (fo.result.file != NULL)
    {
      fflush(fo.result.file);
      fclose(fo.result.file);
      fo.result.file = NULL;
    }
  c = NULL;

  return (r);
}

static gint _print_unexpected_errmsg(const glong rc, const glong cc)
{
  g_printerr(_("error: Server responded with code %ld, "
               "expected 200 or 206 (cc=%ld)\n"), rc, cc);
  return (EXIT_FAILURE);
}

static gint _print_curl_errmsg(const glong rc, const glong cc)
{
  g_printerr(_("error: libcurl: %s (curl_code=%u, rc=%lu, cc=%lu)\n"),
             curl_easy_strerror(curl_code), curl_code, rc, cc);
  return (EXIT_FAILURE);
}

static gint _print_io_errmsg()
{
  if (io_errmsg != NULL)
    {
      g_printerr(_("error: while writing to file: %s\n"), io_errmsg);
      g_free(io_errmsg);
    }
  return (EXIT_FAILURE);
}

static gint _chk_transfer_errors(CURL *c)
{
  glong rc, cc;
  gint r;

  r = EXIT_SUCCESS;
  rc = 0;
  cc = 0;

  curl_easy_getinfo(c, CURLINFO_HTTP_CONNECTCODE, &cc);
  curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &rc);

  if (curl_code == CURLE_OK)
    {
      if (rc != 200 && rc != 206)
        r = _print_unexpected_errmsg(rc, cc);
    }
  else
    {
      if (curl_code != CURLE_WRITE_ERROR)
        r = _print_curl_errmsg(rc, cc);
      else /* _write_cb returned error (0) */
        {
          r = EXIT_FAILURE;
          if (fo.result.skip_retrieved_already == FALSE
              && force_skip_transfer == FALSE)
            {
              _print_io_errmsg();
            }
        }
    }
  return (r);
}

static gint _set_io_errmsg()
{
  io_errmsg = lutil_strerror();
  return (0);
}

static gint _build_fpath()
{
  lutil_build_fpath_t b;
  quvi_file_ext_t qfe;

  if (g->result.fpath != NULL) /* Skip re-building. */
    return (EXIT_SUCCESS);

  b = g->build_fpath;
  qfe = quvi_file_ext_new(g->q, content_type);

  if (quvi_ok(g->q) == FALSE)
    {
      g->xperr(_("libquvi: while parsing file extension: %s"),
               quvi_errmsg(g->q));
      quvi_file_ext_free(qfe);
      return (EXIT_FAILURE);
    }

  b->file_ext = quvi_file_ext_get(qfe);
  g->result.fpath = lutil_build_fpath(b);

  quvi_file_ext_free(qfe);

  if (g->result.fpath != NULL)
    {
      pbar->fname = g_path_get_basename(g->result.fpath);
      return (EXIT_SUCCESS);
    }
  return (EXIT_FAILURE);
}

static gint _open_file()
{
  if (_build_fpath() != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (force_skip_transfer == TRUE)
    return (EXIT_FAILURE);

  /*
   * Force overwrite if resume-from<0 was given.
   *
   * libcurl requires setting CURLOPT_RESUME_FROM_LARGE before
   * `curl_easy_perform' is called. The the program has no way of
   * knowing whether the transfer should be resumed if `content-length'
   * is not queried by sending a HTTP HEAD request before the transfer
   * begins.
   */

  if (g->opts.resume_from >= 0)
    fo.overwrite_if_exists = g->opts.overwrite_if_exists;
  else
    fo.overwrite_if_exists = TRUE;

  if (content_length >0)
    fo.content_bytes = content_length;

  fo.fpath = g->result.fpath;
  fo.xperr = g->xperr;

  return (lutil_file_open(&fo));
}

/* Check if transfer was skipped for whatever reason. */
static gint _chk_skipped()
{
  pbar->flags.failed = TRUE; /* Do not print update line. */
  transfer_skipped = TRUE; /* Default */

  if (fo.result.skip_retrieved_already == TRUE)
    pbar->mode = retrieved_already;
  else if (force_skip_transfer == TRUE)
    pbar->mode = forced_skip;
  else
    transfer_skipped = FALSE; /* An error (e.g. file open) occurred. */

  if (transfer_skipped == TRUE)
    lpbar_print(pbar);

  return (EXIT_FAILURE);
}

/* Open file when the transfer begins (--resume-from >0). */
static gint _chk_file_open()
{
  if (fo.result.file != NULL)
    return (EXIT_SUCCESS);

  _content_props_from(HTTP_GET_RESPONSE);

  if (_open_file() != EXIT_SUCCESS)
    return (_chk_skipped());

  lpbar_print(pbar);

  return (EXIT_SUCCESS);
}

static gsize _write_cb(gpointer data, gsize size, gsize nmemb, gpointer udata)
{
  if (_chk_file_open() != EXIT_SUCCESS)
    return (0);

  if (fwrite(data, size, nmemb, fo.result.file) != nmemb)
    return (_set_io_errmsg());

  if (fflush(fo.result.file) != 0)
    return (_set_io_errmsg());

  return (size*nmemb);
}

static gint _progress_cb(gpointer clientp, gdouble dltotal, gdouble dlnow,
                         gdouble ultotal, gdouble ulnow)
{
  return (lpbar_update((lpbar_t) clientp, dlnow));
}

static gint _chk_autoresume()
{
  /*
   * Force HTTP HEAD check with resume-from=0 (autoresume).
   *
   * Retrieve the content-{length,type} from the returned response,
   * build the file path, and open the file.
   *
   *  NOTE: file path (or name) cannot be generated unless content-type
   *        has been retrieved, the value is used to create the file
   *        extension which is replaced by the xchg subsystem of lutil.
   *
   * Otherwise, proceed to open the stream, and do all of the above in
   * the write-callback, instead. This has the advantage of skipping the
   * HTTP HEAD request, and the disadvantage of not knowing whether the
   * transfer should be resumed, see the CURLOPT_RESUME_FROM_LARGE notes
   * above.
   */

  lutil_build_fpath_t b = g->build_fpath;

  if (lutil_query_metainfo(g->q, b->qm, &qmi, b->xperr) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  _content_props_from(HTTP_HEAD_RESPONSE);

  if (_open_file() != EXIT_SUCCESS)
    return (_chk_skipped());

  return (EXIT_SUCCESS);
}

static gint _setup_curl()
{
  if (g->opts.resume_from >= 0) /* 0=auto, >0 from the specified offset. */
    {
      gdouble o = g->opts.resume_from;
      if (g->opts.resume_from ==0)
        {
          if (_chk_autoresume() != EXIT_SUCCESS)
            return (EXIT_FAILURE);
          o = fo.result.initial_bytes;
        }
      curl_easy_setopt(c, CURLOPT_RESUME_FROM_LARGE, (curl_off_t) o);
      pbar->initial_bytes = o;
    }

  /*
   * The file was opened, not postponed to be opened in the write
   * callback when the transfer begins (--resume-from >0).
   */
  if (fo.result.file != NULL)
    lpbar_print(pbar);

  /*
   * Set the curl handle options _after_ metainfo has been retrieved.
   * Otherwise these settings would interfere with it.
   */
  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, _write_cb);
  curl_easy_setopt(c, CURLOPT_URL, g->url);

  curl_easy_setopt(c, CURLOPT_ENCODING, "identity");
  curl_easy_setopt(c, CURLOPT_HEADER, 0L);

  curl_easy_setopt(c, CURLOPT_PROGRESSFUNCTION, _progress_cb);
  curl_easy_setopt(c, CURLOPT_PROGRESSDATA, pbar);
  curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);

  return (EXIT_SUCCESS);
}

static void _reset_curl()
{
  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, NULL);
  curl_easy_setopt(c, CURLOPT_ENCODING, "");

  curl_easy_setopt(c, CURLOPT_PROGRESSFUNCTION, NULL);
  curl_easy_setopt(c, CURLOPT_PROGRESSDATA, NULL);
  curl_easy_setopt(c, CURLOPT_NOPROGRESS, 1L);

  curl_easy_setopt(c, CURLOPT_RESUME_FROM_LARGE, 0L);
}

static gint _open_stream()
{
  gint r;

  c = lutil_curl_handle_from(g->q);
  if (c == NULL)
    {
      g->xperr(_("error: failed to retrieve the current curl "
                 "session handle from libquvi"));
      return (EXIT_FAILURE);
    }

  pbar = lpbar_new();
  r = _setup_curl();

  if (r == EXIT_SUCCESS)
    {
      curl_code = curl_easy_perform(c);
      r = _chk_transfer_errors(c);
      _reset_curl();
    }

  pbar->flags.failed = (r != EXIT_SUCCESS) ? TRUE:FALSE;
  return (r);
}

static gint _exec_cmd()
{
  struct lutil_exec_opts_s xopts;
  gint i, r;

  if (g->opts.exec.external == NULL)
    return (EXIT_SUCCESS);

  for (i=0, r=EXIT_SUCCESS;
       g->opts.exec.external[i] != NULL && r == EXIT_SUCCESS;
       ++i)
    {
      memset(&xopts, 0, sizeof(struct lutil_exec_opts_s));

      xopts.flags.discard_stderr = !g->opts.exec.enable_stderr;
      xopts.flags.discard_stdout = !g->opts.exec.enable_stdout;
      xopts.flags.dump_argv = g->opts.exec.dump_argv;

      xopts.exec_arg = g->opts.exec.external[i];
      xopts.xperr = g->xperr;

      xopts.fpath = g->result.fpath;
      xopts.qm = g->build_fpath->qm;

      r = lutil_exec_cmd(&xopts);
    }
  return (r);
}

gint lget_http_get(lget_t handle)
{
  gint r;

  memset(&fo, 0, sizeof(struct lutil_file_open_s));

  g = handle;
  qmi = NULL;

  content_length = 0;
  content_type = NULL;

  io_errmsg = NULL;
  pbar = NULL;

  curl_code = 0;
  c = NULL;

  force_skip_transfer = g->opts.skip_transfer;
  transfer_skipped = FALSE;

  /*
   * If the media stream was retrieved completely already:
   *  lutil_open_file will set the 'skip_retrieved_already' flag, and
   *  return EXIT_FAILURE.
   */
  r = _open_stream();
  if (r == EXIT_SUCCESS || fo.result.skip_retrieved_already == TRUE)
    r = _exec_cmd();

  /* --skip-transfer was specified. */
  if (transfer_skipped == TRUE)
    r = EXIT_SUCCESS;

  return (_cleanup(r));
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
