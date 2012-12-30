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

#ifndef setup_h
#define setup_h

struct setup_query_s
{
  gboolean force_subtitle_mode;
  lutil_cb_printerr xperr;
  lutil_cb_printerr perr;
  linput_t linput;
  quvi_t q;
  struct
  {
    lutil_query_properties_activity_cb playlist;
    lutil_query_properties_activity_cb subtitle;
    lutil_query_properties_activity_cb media;
  } activity;
};

typedef struct setup_query_s *setup_query_t;

gint setup_opts(gint, gchar**, lopts_t);
gint setup_query(setup_query_t);
gint setup_quvi(quvi_t*);

#endif /* setup_h */

/* vim: set ts=2 sw=2 tw=72 expandtab: */
