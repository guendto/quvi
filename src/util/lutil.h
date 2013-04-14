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

#ifndef lutil_h
#define lutil_h

/* callback */

typedef void (*lutil_cb_printerr)(const gchar*, ...);

/* extern */

extern const gchar *lutil_verbosity_possible_values[];

/* regex op */

struct _lutil_regex_op_s
{
  gchar *modifiers;
  gchar *sequence;
  gchar *regex;
  gchar *mode;
  struct
  {
    gchar *replacement;
  } subst;
};

typedef struct _lutil_regex_op_s *lutil_regex_op_t;

lutil_regex_op_t lutil_regex_op_new(const gchar*, gboolean*);
void lutil_regex_op_free(lutil_regex_op_t);

gchar *lutil_regex_op_apply(lutil_regex_op_t, const gchar*);

/* exec */

struct lutil_exec_opts_s
{
  lutil_cb_printerr xperr;
  const gchar *exec_arg;
  const gchar *file_ext;
  const gchar *fpath;
  gpointer qm;
  struct
  {
    gboolean discard_stderr;
    gboolean discard_stdout;
    gboolean dump_argv;
  } flags;
};

typedef struct lutil_exec_opts_s *lutil_exec_opts_t;

gint lutil_exec_cmd(lutil_exec_opts_t);

/* property */

typedef enum
{
  UTIL_PROPERTY_TYPE_SUBTITLE_LANGUAGE,
  UTIL_PROPERTY_TYPE_SUBTITLE_TYPE,
  UTIL_PROPERTY_TYPE_HTTP_METAINFO,
  UTIL_PROPERTY_TYPE_PLAYLIST,
  UTIL_PROPERTY_TYPE_SCRIPT,
  UTIL_PROPERTY_TYPE_MEDIA
} lutilPropertyType;

gint lutil_chk_property_ok(gpointer, const lutilPropertyType,
                           const gchar*, lutil_cb_printerr);


/* sequence */

struct _lutil_xchg_seq_s
{
  lutil_cb_printerr printerr; /* exportable */
  GHashTable *htable;
  GRegex *re;
};

typedef struct _lutil_xchg_seq_s *lutil_xchg_seq_t;

struct lutil_xchg_seq_opts_s
{
  const gchar **output_regex;
  lutil_cb_printerr xperr;
  const gchar *file_ext;
  const gchar *fpath;
  gpointer qm;
};

typedef struct lutil_xchg_seq_opts_s *lutil_xchg_seq_opts_t;

lutil_xchg_seq_t lutil_xchg_seq_new(const lutil_xchg_seq_opts_t);

struct lutil_xchg_seq_noq_s
{
  const gchar *seq;
  const gchar *val;
};

typedef struct lutil_xchg_seq_noq_s *lutil_xchg_seq_noq_t;

lutil_xchg_seq_t
lutil_xchg_seq_noq_new(lutil_xchg_seq_noq_t, const lutil_cb_printerr,
                       const gchar**);

void lutil_xchg_seq_free(lutil_xchg_seq_t);

gchar *lutil_xchg_seq_apply(lutil_xchg_seq_t, const gchar*);

/* check support */

struct lutil_check_support_s
{
  lutil_cb_printerr xperr; /* exported {json,xml,...} messages */
  lutil_cb_printerr perr; /* status update messages */
  gint exit_status;
  gpointer q;
  gint mode; /* QuviSupportsMode */
  struct
  {
    gboolean force_subtitle_mode;
    gboolean force_offline_mode;
  } flags;
  struct
  {
    GSList *playlist;
    GSList *subtitle;
    GSList *media;
  } url;
};

typedef struct lutil_check_support_s *lutil_check_support_t;

void lutil_check_support(gpointer, gpointer);
void lutil_check_support_free(lutil_check_support_t);

/* query */

typedef void (*lutil_query_properties_activity_cb)(gpointer, gpointer,
    const gchar*);

struct lutil_query_properties_s
{
  lutil_query_properties_activity_cb activity;
  lutil_cb_printerr xperr; /* exported {json,xml,...} messages */
  lutil_cb_printerr perr; /* status update messages */
  gint exit_status;
  const gchar *url;
  gpointer q;
};

typedef struct lutil_query_properties_s *lutil_query_properties_t;

void lutil_query_playlist(gpointer, gpointer);
void lutil_query_subtitle(gpointer, gpointer);
void lutil_query_media(gpointer, gpointer);

/* build fpath */

struct lutil_build_fpath_s
{
  lutil_cb_printerr xperr;
  const gchar *file_ext;
  gchar **output_regex;
  gchar *output_file;
  gchar *output_name;
  gchar *output_dir;
  gpointer qm;
};

typedef struct lutil_build_fpath_s *lutil_build_fpath_t;

gchar *lutil_build_fpath(lutil_build_fpath_t);

/* file */

struct lutil_file_open_s
{
  gboolean overwrite_if_exists;
  lutil_cb_printerr xperr;
  gdouble content_bytes;
  const gchar *fpath;
  struct
  {
    gboolean skip_retrieved_already;
    gdouble initial_bytes;
    void *file;
  } result;
};

typedef struct lutil_file_open_s *lutil_file_open_t;

gint lutil_file_open(lutil_file_open_t);

/* curl */

struct lutil_net_opts_s
{
  gdouble throttle_ki_s;
  gint verbose;
};

typedef struct lutil_net_opts_s *lutil_net_opts_t;

gpointer lutil_curl_handle_from(gpointer);

/* verbosity */

typedef enum
{
  UTIL_VERBOSITY_LEVEL_DEBUG,
  UTIL_VERBOSITY_LEVEL_VERBOSE,
  UTIL_VERBOSITY_LEVEL_QUIET,
  UTIL_VERBOSITY_LEVEL_MUTE
} lutilVerbosityLevel;

lutilVerbosityLevel lutil_setup_verbosity_level(const gchar*);
lutilVerbosityLevel lutil_get_verbosity_level();

void lutil_print_stderr_unless_quiet(const gchar*, ...);

/* other */

gint lutil_choose_stream(const gpointer, const gpointer, const gchar*,
                         const lutil_cb_printerr);

gint lutil_choose_subtitle(const gpointer, const gpointer, const gchar*,
                           const lutil_cb_printerr, gpointer*,
                           const gboolean);

gint lutil_query_metainfo(gpointer, gpointer, gpointer*, lutil_cb_printerr);

GSList *lutil_slist_prepend_if_unique(GSList*, const gchar*);
void lutil_slist_free_full(GSList*, GFunc);

gboolean lutil_strv_contains(const gchar**, const gchar*);
gchar *lutil_strerror();

gint lutil_quvi_init(gpointer*, lutil_net_opts_t o);
gint lutil_parse_input(gpointer, const gchar**);

#endif /* lutil_h */

/* vim: set ts=2 sw=2 tw=72 expandtab: */
