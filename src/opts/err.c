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
#include <glib/gi18n.h>

#include "lopts.h"

enum {IGNORE_VARIABLE, INVALID_VALUE, POSSIBLE_VALUES};

static const gchar *_E[] =
{
  N_("error: %s:\n  ignoring the variable `%s' with an invalid value `%s'\n"),
  N_("error: invalid value `%s' for the option `--%s'\n"),
  N_("Possible values for the option:\n"),
  NULL
};

gint lopts_invalid_value(const gchar *opt_name,
                         const gchar *config_fpath,
                         const gchar *inv_value,
                         const gchar **possible_values)
{
  if (config_fpath != NULL)
    {
      g_printerr(g_dgettext(GETTEXT_PACKAGE, _E[IGNORE_VARIABLE]),
                 config_fpath, opt_name, inv_value);
    }
  else
    {
      g_printerr(g_dgettext(GETTEXT_PACKAGE, _E[INVALID_VALUE]),
                 inv_value, opt_name);
    }

  if (possible_values != NULL)
    {
      gint i = 0;
      g_printerr("%s", g_dgettext(GETTEXT_PACKAGE, _E[POSSIBLE_VALUES]));

      while (possible_values[i] != NULL)
        g_printerr("  %s\n", possible_values[i++]);
    }
  return (EXIT_FAILURE);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
