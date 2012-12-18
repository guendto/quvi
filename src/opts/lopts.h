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

#ifndef lopts_h
#define lopts_h

/* callback */

typedef void (*lopts_cb_parse_keyfile_values)(GKeyFile*,const gchar*);
typedef gchar *(*lopts_cb_get_config_fpath)();

typedef gint (*lopts_cb_cmdline_validate_values)();
typedef void (*lopts_cb_set_post_parse_defaults)();

typedef gint (*lopts_cb_chk_ok_strv)(const gchar*,
                                     const gchar*,
                                     const gchar**,
                                     const gchar**);

typedef gint (*lopts_cb_chk_ok_re)(const gchar*,
                                   const gchar*,
                                   const gchar**);

typedef gint (*lopts_cb_chk_ok_str)(const gchar*,
                                    const gchar*,
                                    const gchar*,
                                    const gchar**);

typedef gint (*lopts_cb_chk_ok_int)(const gchar*,
                                    const gchar*,
                                    const gint);

/* struct */

struct lopts_s
{
  const GOptionEntry *entries;
  gchar **argv;
  gint argc;
  struct
  {
    lopts_cb_cmdline_validate_values  cmdline_validate_values;
    lopts_cb_set_post_parse_defaults  set_post_parse_defaults;
    lopts_cb_parse_keyfile_values     parse_keyfile_values;
    lopts_cb_get_config_fpath         get_config_fpath;
  } cb;
};

typedef struct lopts_s *lopts_t;

/* function */

gint lopts_new(lopts_t);

gint lopts_chk_strv(const gchar**, const gchar**, gchar**);
gint lopts_chk_str(const gchar*, const gchar**, gchar**);
gint lopts_chk_re(const gchar**, gchar**);

gchar *lopts_keyfile_get_str(GKeyFile*,
                             lopts_cb_chk_ok_str,
                             const gchar*,
                             const gchar*,
                             const gchar**,
                             const gchar*,
                             gchar **dst);

gchar **lopts_keyfile_get_strv(GKeyFile*,
                               lopts_cb_chk_ok_strv,
                               const gchar*,
                               const gchar*,
                               const gchar**,
                               const gchar*,
                               gchar***);

gchar **lopts_keyfile_get_re(GKeyFile*,
                             lopts_cb_chk_ok_re,
                             const gchar*,
                             const gchar*,
                             const gchar**,
                             const gchar*,
                             gchar***);

gboolean lopts_keyfile_get_bool(GKeyFile*,
                                const gchar*,
                                const gchar*,
                                const gchar*,
                                gboolean*);

gint lopts_keyfile_get_double(GKeyFile*,
                           lopts_cb_chk_ok_int,
                           const gchar*,
                           const gchar*,
                           const gchar*,
                           gdouble*);

gint lopts_keyfile_get_int(GKeyFile*,
                           lopts_cb_chk_ok_int,
                           const gchar*,
                           const gchar*,
                           const gchar*,
                           gint*);

gint lopts_invalid_value(const gchar*, const gchar*,
                         const gchar*, const gchar**);

void lopts_print_config_values(lopts_t);
void lopts_print_config_paths(lopts_t);

#endif /* lopts_h */

/* vim: set ts=2 sw=2 tw=72 expandtab: */
