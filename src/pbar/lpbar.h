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

#ifndef lpbar_h
#define lpbar_h

typedef enum { retrieved_already, forced_skip, resume, write } lpbar_mode;

struct lpbar_s
{
  gdouble content_bytes; /* total media content length */
  gdouble initial_bytes; /* >0 if a resumed transfer */
  gchar *content_type;
  lpbar_mode mode;
  gchar *fname;
  struct
  {
    gdouble last_update;
    gint curr_frame;
    gdouble count;
    GTimer *timer;
  } counters;
  struct
  {
    gboolean failed;
    gboolean done;
  } flags;
};

typedef struct lpbar_s *lpbar_t;

lpbar_t lpbar_new();
void lpbar_free(lpbar_t);

gint lpbar_update(lpbar_t, gdouble);
void lpbar_print(const lpbar_t);

#endif /* lpbar_h */

/* vim: set ts=2 sw=2 tw=72 expandtab: */
