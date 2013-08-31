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

#include "lopts.h"
#include "lutil.h"
#include "opts.h"

const GOptionEntry option_entries[] =
{
  /* core */
  {
    "check-mode-offline", 'o', 0, G_OPTION_ARG_NONE,
    &opts.core.check_mode_offline, NULL, NULL
  },
  {
    "print-format", 'p', 0, G_OPTION_ARG_STRING, &opts.core.print_format,
    NULL, NULL
  },
  {
    "print-subtitles", 'B', 0, G_OPTION_ARG_NONE, &opts.core.print_subtitles,
    NULL, NULL
  },
  {
    "print-streams", 'S', 0, G_OPTION_ARG_NONE, &opts.core.print_streams,
    NULL, NULL
  },
  {
    "subtitle-export-format", 'L', 0, G_OPTION_ARG_STRING,
    &opts.core.subtitle_export_format, NULL, NULL
  },
  {
    "subtitle-language", 'l', 0, G_OPTION_ARG_STRING,
    &opts.core.subtitle_language, NULL, NULL
  },
  {
    "stream", 's', 0, G_OPTION_ARG_STRING, &opts.core.stream,
    NULL, NULL
  },
  {
    "verbosity", 'b', 0, G_OPTION_ARG_STRING, &opts.core.verbosity,
    NULL, NULL
  },
  /* dump */
  {
    "query-metainfo", 'q', 0, G_OPTION_ARG_NONE, &opts.dump.query_metainfo,
    NULL, NULL
  },
  /* exec */
  {
    "exec-enable-stderr", 'E', 0, G_OPTION_ARG_NONE, &opts.exec.enable_stderr,
    NULL, NULL
  },
  {
    "exec-enable-stdout", 'O', 0, G_OPTION_ARG_NONE, &opts.exec.enable_stdout,
    NULL, NULL
  },
  {
    "exec-dump-argv", 'A', 0, G_OPTION_ARG_NONE, &opts.exec.dump_argv,
    NULL, NULL
  },
  {
    "exec", 'e', 0, G_OPTION_ARG_STRING_ARRAY, &opts.exec.external,
    NULL, NULL
  },
  /* get */
  {
    "output-regex", 'g', 0, G_OPTION_ARG_STRING_ARRAY, &opts.get.output_regex,
    NULL, NULL
  },
  {
    "output-file", 'f', 0, G_OPTION_ARG_STRING, &opts.get.output_file,
    NULL, NULL
  },
  {
    "output-name", 'n', 0, G_OPTION_ARG_STRING, &opts.get.output_name,
    NULL, NULL
  },
  {
    "output-dir", 'i', 0, G_OPTION_ARG_STRING, &opts.get.output_dir,
    NULL, NULL
  },
  {
    "overwrite", 'w', 0, G_OPTION_ARG_NONE, &opts.get.overwrite,
    NULL, NULL
  },
  {
    "skip-transfer", 'k', 0, G_OPTION_ARG_NONE, &opts.get.skip_transfer,
    NULL, NULL
  },
  {
    "resume-from", 'r', 0, G_OPTION_ARG_DOUBLE, &opts.get.resume_from,
    NULL, NULL
  },
  {
    "throttle", 't', 0, G_OPTION_ARG_INT, &opts.get.throttle,
    NULL, NULL
  },
  /* http */
  {
    "enable-cookies", 'c', 0, G_OPTION_ARG_NONE, &opts.http.enable_cookies,
    NULL, NULL
  },
  {
    "user-agent", 'u', 0, G_OPTION_ARG_STRING, &opts.http.user_agent,
    NULL, NULL
  },
  /* remaining */
  {
    G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &opts.rargs,
    NULL, NULL
  },
  {NULL, 0, 0, 0, NULL, NULL, NULL}
};

static const gchar *dumpformat_possible_values[] =
{
  "enum",
  "rfc2483", /* playlist specific */
#ifdef HAVE_JSON_GLIB
  "json",
#endif
#ifdef HAVE_LIBXML
  "xml",
#endif
  NULL
};

static gint cb_chk_str(const gchar *fpath, const gchar *opt_name,
                       const gchar *opt_val, const gchar **ok_strv)
{
  gchar *inv_val;
  if (lopts_chk_str(opt_val, ok_strv, &inv_val) != EXIT_SUCCESS)
    return (lopts_invalid_value(opt_name, fpath, inv_val, ok_strv));
  return (EXIT_SUCCESS);
}

#ifdef _1
static gint cb_chk_strv(const gchar *fpath, const gchar *opt_name,
                        const gchar **opt_val, const gchar **ok_strv)
{
  gchar *inv_val;
  if (lopts_chk_strv(opt_val, ok_strv, &inv_val) != EXIT_SUCCESS)
    return (lopts_invalid_value(opt_name, fpath, inv_val, ok_strv));
  return (EXIT_SUCCESS);
}
#endif

static gint cb_chk_re(const gchar *fpath, const gchar *opt_name,
                      const gchar **opt_val)
{
  gchar *inv_val;
  if (lopts_chk_re(opt_val, &inv_val) != EXIT_SUCCESS)
    return (lopts_invalid_value(opt_name, fpath, inv_val, NULL));
  return (EXIT_SUCCESS);
}

static gint cb_chk_throttle(const gchar *fpath,
                            const gchar *opt_name,
                            const gint opt_val)
{
  gint r = EXIT_SUCCESS;
  if (opt_val <0)
    {
      gchar *s = g_strdup_printf("%d", opt_val);
      lopts_invalid_value(opt_name, fpath, s, NULL);
      r = EXIT_FAILURE;
      g_free(s);
    }
  return (r);
}

static const gchar g_core[] = "core";
static const gchar g_dump[] = "dump";
static const gchar g_exec[] = "exec";
static const gchar g_http[] = "http";
static const gchar g_get[] = "get";

void cb_parse_keyfile_values(GKeyFile *kf, const gchar *fpath)
{
  /* core */

  lopts_keyfile_get_bool(kf, fpath, g_core,
                         "check-mode-offline", &opts.core.check_mode_offline);

  lopts_keyfile_get_str(kf, cb_chk_str, fpath, g_core,
                        dumpformat_possible_values,
                        "print-format", &opts.core.print_format);

  lopts_keyfile_get_str(kf, NULL, fpath, g_core, NULL,
                        "subtitle-export-format",
                        &opts.core.subtitle_export_format);

  lopts_keyfile_get_str(kf, NULL, fpath, g_core, NULL,
                        "subtitle-language", &opts.core.subtitle_language);

  lopts_keyfile_get_str(kf, NULL, fpath, g_core, NULL,
                        "stream", &opts.core.stream);

  lopts_keyfile_get_str(kf, cb_chk_str, fpath, g_core,
                        lutil_verbosity_possible_values,
                        "verbosity", &opts.core.verbosity);

  /* dump */

  lopts_keyfile_get_bool(kf, fpath, g_dump,
                         "query-metainfo", &opts.dump.query_metainfo);

  /* exec */

  lopts_keyfile_get_bool(kf, fpath, g_exec,
                         "dump-argv", &opts.exec.dump_argv);

  lopts_keyfile_get_bool(kf, fpath, g_exec,
                         "enable-stderr", &opts.exec.enable_stderr);

  lopts_keyfile_get_bool(kf, fpath, g_exec,
                         "enable-stdout", &opts.exec.enable_stdout);

  lopts_keyfile_get_strv(kf, NULL, fpath, g_exec, NULL,
                         "external", &opts.exec.external);

  /* get */

  lopts_keyfile_get_str(kf, NULL, fpath, g_get, NULL,
                        "output-dir", &opts.get.output_dir);

  lopts_keyfile_get_str(kf, NULL, fpath, g_get, NULL,
                        "output-name", &opts.get.output_name);

  lopts_keyfile_get_re(kf, cb_chk_re, fpath, g_get, NULL,
                       "output-regex", &opts.get.output_regex);

  lopts_keyfile_get_double(kf, NULL, fpath, g_get,
                           "resume-from", &opts.get.resume_from);

  lopts_keyfile_get_bool(kf, fpath, g_get,
                         "skip-transfer", &opts.get.skip_transfer);

  lopts_keyfile_get_int(kf, cb_chk_throttle, fpath, g_get,
                        "throttle", &opts.get.throttle);

  /* http */

  lopts_keyfile_get_bool(kf, fpath, g_http,
                         "enable-cookies", &opts.http.enable_cookies);

  lopts_keyfile_get_str(kf, NULL, fpath, g_http, NULL,
                        "user-agent", &opts.http.user_agent);
}

#define _chk_r\
  do {\
    if (r != EXIT_SUCCESS)\
      return (r);\
  } while (0)

gint cb_cmdline_validate_values()
{
  gint r;

  /* output */

  r = cb_chk_str(NULL, "print-format", opts.core.print_format,
                 dumpformat_possible_values);
  _chk_r;

  r = cb_chk_str(NULL, "verbosity", opts.core.verbosity,
                 lutil_verbosity_possible_values);
  _chk_r;

  r = cb_chk_re(NULL, "output-regex", (const gchar**) opts.get.output_regex);
  _chk_r;

  /* get */

  r = cb_chk_throttle(NULL, "throttle", opts.get.throttle);
  _chk_r;

  return (EXIT_SUCCESS);
}

#undef _chk_r

void cb_set_post_parse_defaults()
{
  /* core */

  if (opts.core.subtitle_export_format == NULL)
    opts.core.subtitle_export_format = g_strdup("srt");

  if (opts.core.verbosity == NULL)
    opts.core.verbosity = g_strdup("verbose");

  /* get */

  if (opts.get.output_regex == NULL)
    {
      static gchar *sv[] =
      {
        "%t:/\\w|\\s/",
        "%t:s/\\s\\s+/ /",
        "%t:s/^\\s+//",
        "%t:s/\\s+$//",
        NULL
      };
      opts.get.output_regex = g_strdupv(sv);
    }

  if (opts.get.output_name == NULL)
    opts.get.output_name = g_strdup("%t.%e");

  /* http */

  if (opts.http.user_agent == NULL)
    opts.http.user_agent = g_strdup("Mozilla/5.0");
}

gchar *cb_get_config_fpath()
{
  const gchar *s = g_getenv("QUVI_CONFIG");
  if (s != NULL && strlen(s) >0)
    return (g_strdup(s));
  return (NULL);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
