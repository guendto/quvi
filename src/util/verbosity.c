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

#include <glib/gprintf.h>
#include <quvi.h>
#include <curl/curl.h>

#include "lutil.h"

/*
 * - debug   (verbose and libcurl CURLOPT_VERBOSE)
 * - verbose (default)
 * - quiet   (errors only)
 * - mute    (nothing, not even errors)
 */

const gchar *lutil_verbosity_possible_values[] =
{
  "debug", "verbose", "quiet", "mute", NULL
};

static lutilVerbosityLevel level = UTIL_VERBOSITY_LEVEL_VERBOSE;

void lutil_print_stderr_unless_quiet(const gchar *fmt, ...)
{
  if (level < UTIL_VERBOSITY_LEVEL_QUIET)
    {
      va_list args;
      gchar *s;

      va_start(args, fmt);
      if (g_vasprintf(&s, fmt, args) >0)
        {
          fprintf(stderr, "%s", s);
          fflush(stderr);
          g_free(s);
        }
      va_end(args);
    }
}

static void _printerr(const gchar *s)
{
  if (level < UTIL_VERBOSITY_LEVEL_MUTE)
    {
      fprintf(stderr, "%s", s);
      fflush(stderr);
    }
}

static void _print(const gchar *s)
{
  if (level < UTIL_VERBOSITY_LEVEL_QUIET)
    {
      fprintf(stdout, "%s", s);
      fflush(stdout);
    }
}

static lutilVerbosityLevel _level_from(const gchar *s)
{
  lutilVerbosityLevel l = UTIL_VERBOSITY_LEVEL_DEBUG;
  while (lutil_verbosity_possible_values[l] != NULL)
    {
      if (g_strcmp0(lutil_verbosity_possible_values[l], s) ==0)
        return (l);
      ++l;
    }
  g_warning("[%s] unknown verbosity level `%s'", __func__, s);
  return (UTIL_VERBOSITY_LEVEL_VERBOSE);
}

lutilVerbosityLevel lutil_setup_verbosity_level(const gchar *s)
{
  level = _level_from(s);

  g_set_printerr_handler(_printerr);
  g_set_print_handler(_print);

  return (level);
}

lutilVerbosityLevel lutil_get_verbosity_level()
{
  return (level);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
