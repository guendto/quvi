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

static gint _cmdline_parse(const lopts_t lopts)
{
  GOptionContext *c;
  GError *e;
  gint r;

  if (lopts->argv == NULL)
    return (EXIT_SUCCESS);

  c = g_option_context_new(NULL);
  r = EXIT_SUCCESS;
  e = NULL;

  g_option_context_add_main_entries(c, lopts->entries, GETTEXT_PACKAGE);
  g_option_context_set_translation_domain(c, GETTEXT_PACKAGE);
  g_option_context_set_help_enabled(c, FALSE);

  if (g_option_context_parse(c, &lopts->argc, &lopts->argv, &e) == FALSE)
    {
      g_printerr(_("error: while parsing options: %s\n"), e->message);
      g_error_free(e);
      r = EXIT_FAILURE;
    }

  g_option_context_free(c);
  return (r);
}

extern void keyfile_read(lopts_t);

gboolean show_config;

gint lopts_new(lopts_t lopts)
{
  g_assert(lopts != NULL);
  g_assert(lopts->entries != NULL);

  show_config = FALSE;

  /* read config files. */

  keyfile_read(lopts);

  /* cmdline values override config values. */

  if (_cmdline_parse(lopts) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  if (lopts->cb.cmdline_validate_values != NULL)
    {
      if (lopts->cb.cmdline_validate_values() != EXIT_SUCCESS)
        return (EXIT_FAILURE);
    }

  if (lopts->cb.set_post_parse_defaults != NULL)
    lopts->cb.set_post_parse_defaults();

  return (EXIT_SUCCESS);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
