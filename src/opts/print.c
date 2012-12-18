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

#include <glib/gi18n.h>

#include "lopts.h"

void lopts_print_config_values(lopts_t lopts)
{
  const GOptionEntry *p;

  g_assert(lopts != NULL);
  g_assert(lopts->entries != NULL);

  g_print(_("# Value of all configuration options\n"));
  g_print("#\n");

  p = lopts->entries;
  while (p->long_name != NULL)
    {
      if (g_strcmp0(p->long_name, G_OPTION_REMAINING) != 0)
        {
          switch (p->arg)
            {
            case G_OPTION_ARG_FILENAME_ARRAY:
            case G_OPTION_ARG_STRING_ARRAY:
            {
              gchar **sv = *(gchar***) p->arg_data;
              if (sv != NULL)
                {
                  gchar *s = g_strjoinv(",", sv);
                  g_print("%s=%s\n", p->long_name, s);
                  g_free(s);
                }
              else
                g_print(_("%s is unset\n"), p->long_name);
            }
            break;

            case G_OPTION_ARG_FILENAME:
            case G_OPTION_ARG_STRING:
            {
              const gchar *s = *(const gchar**) p->arg_data;
              if (s != NULL)
                g_print("%s=%s\n", p->long_name, s);
              else
                g_print(_("%s is unset\n"), p->long_name);
            }
            break;

            case G_OPTION_ARG_DOUBLE:
              g_print("%s=%g\n", p->long_name, *(gdouble*) p->arg_data);
              break;

            case G_OPTION_ARG_INT64:
              g_print("%s=%"G_GINT64_FORMAT"\n",
                      p->long_name, *(gint64*) p->arg_data);
              break;

            case G_OPTION_ARG_NONE:
            {
              const gboolean r = (*(gboolean*) p->arg_data);
              if (r == TRUE)
                g_print(_("%s is set\n"), p->long_name);
              else
                g_print(_("%s is unset\n"), p->long_name);
            }
            break;

            case G_OPTION_ARG_INT:
              g_print("%s=%d\n", p->long_name, *(gint*) p->arg_data);
              break;

            case G_OPTION_ARG_CALLBACK:
            default:
              break;
            }
        }
      ++p;
    }
}

extern gchar **search_dirs();

void lopts_print_config_paths(lopts_t lopts)
{
  gchar *rcpath, **dirs, *p;
  gint i;

  g_print(_("# Search paths for the configuration file\n#\n"));

  rcpath = g_build_path(G_DIR_SEPARATOR_S, "quvi", "quvirc", NULL);
  dirs = search_dirs();

  i = 0;
  while (dirs[i] != NULL)
    {
      p = g_build_path(G_DIR_SEPARATOR_S, dirs[i++], rcpath, NULL);
      g_print("%s\n", p);
      g_free(p);
    }
  g_strfreev(dirs);
  g_free(rcpath);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
