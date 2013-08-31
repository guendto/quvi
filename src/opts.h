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

#ifndef opts_h
#define opts_h

struct opts_s
{
  struct
  {
    gchar *subtitle_export_format;
    gboolean check_mode_offline;
    gchar *subtitle_language;
    gboolean print_subtitles;
    gboolean print_streams;
    gchar *print_format;
    gchar *verbosity;
    gchar *stream;
  } core;
  struct
  {
    gboolean query_metainfo;
  } dump;
  struct
  {
    gboolean enable_stderr;
    gboolean enable_stdout;
    gboolean dump_argv;
    gchar **external;
  } exec;
  struct
  {
    gboolean skip_transfer;
    gchar **output_regex;
    gdouble resume_from;
    gchar *output_name;
    gchar *output_file;
    gboolean overwrite;
    gchar *output_dir;
    gint throttle;
  } get;
  struct
  {
    gboolean enable_cookies;
    gchar *user_agent;
  } http;
  gchar **rargs;
};

typedef struct opts_s *opts_t;

extern const GOptionEntry option_entries[];
extern struct opts_s opts;

/* callbacks */

void cb_parse_keyfile_values(GKeyFile*, const gchar*);
gint cb_cmdline_validate_values();
void cb_set_post_parse_defaults();
gchar *cb_get_config_fpath();

#endif /* opts_h */

/* vim: set ts=2 sw=2 tw=72 expandtab: */
