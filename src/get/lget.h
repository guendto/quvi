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

#ifndef lget_h
#define lget_h

struct lget_s
{
  lutil_build_fpath_t build_fpath;
  lutil_cb_printerr xperr;
  gpointer qm;
  gpointer q;
  gchar *url;
  struct
  {
    gchar *fpath;
  } result;
  struct
  {
    gboolean overwrite_if_exists;
    gboolean skip_transfer;
    gdouble resume_from;
    gchar *stream;
    struct
    {
      const gchar **external;
      gboolean enable_stderr;
      gboolean enable_stdout;
      gboolean dump_argv;
    } exec;
  } opts;
};

typedef struct lget_s *lget_t;

void lget_free(lget_t);
gint lget_new(lget_t);

gint lget_http_get(lget_t);

#endif /* lget_h */

/* vim: set ts=2 sw=2 tw=72 expandtab: */
