/* quvi
 * Copyright (C) 2012  Toni Gundogdu <legatvs@gmail.com>
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

#include "lopts.h"

gchar **search_dirs()
{
  const gchar* const *data_dirs;
  gchar **r;
  gint i,j;

  data_dirs = g_get_system_data_dirs();
  /* 3=user data, user config, trailing NULL. */
  r = g_new(gchar*, g_strv_length((gchar**) data_dirs)+3);
  i = 0;
  j = 0;

  while (data_dirs[j] != NULL)
    r[i++] = g_strdup(data_dirs[j++]);

  r[i++] = g_strdup(g_get_user_data_dir());
  r[i++] = g_strdup(g_get_user_config_dir());
  r[i] = NULL;

  return (r);
}

extern gboolean show_config;

static void _keyfile_printerr(const gchar *fpath, GError *e)
{
  if (e->domain == G_KEY_FILE_ERROR)
    {
      g_printerr(_("error: %s: while reading config: %s (code=%d)\n"),
                 fpath, e->message, e->code);
    }
  else
    {
      if (show_config == TRUE)
        {
          g_message(_("rejected: %s: %s (code=%d)"),
                    fpath, e->message, e->code);
        }
    }
  g_error_free(e);
}

static void _keyfile_parse(lopts_t lopts, const gchar *fpath)
{
  GKeyFile *f;
  GError *e;

  e = NULL;
  f = g_key_file_new();

  g_key_file_set_list_separator(f, ',');

  if (g_key_file_load_from_file(f, fpath, G_KEY_FILE_NONE, &e) == TRUE)
    {
      if (show_config == TRUE)
        g_message(_("parse: %s"), fpath);

      if (lopts->cb.parse_keyfile_values != NULL)
        lopts->cb.parse_keyfile_values(f, fpath);
    }
  else
    _keyfile_printerr(fpath, e);

  g_key_file_free(f);
}

void keyfile_read(lopts_t lopts)
{
  gchar *rcpath, **dirs, *p;
  const gchar *v;
  gint i;

  v = g_getenv("QUVI_SHOW_CONFIG");
  show_config = (v != NULL && strlen(v) >0) ? TRUE:FALSE;

  if (lopts->cb.get_config_fpath != NULL)
    {
      p = lopts->cb.get_config_fpath();
      if (p != NULL) /* QUVI_CONFIG was defined. */
        {
          _keyfile_parse(lopts, p);
          g_free(p);
          return;
        }
    }

  rcpath = g_build_path(G_DIR_SEPARATOR_S, "quvi", "quvirc", NULL);
  dirs = search_dirs();

  i = 0;
  while (dirs[i] != NULL)
    {
      p = g_build_path(G_DIR_SEPARATOR_S, dirs[i++], rcpath, NULL);
      _keyfile_parse(lopts, p);
      g_free(p);
    }
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
