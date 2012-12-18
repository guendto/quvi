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
#include <glib.h>

#include "lutil.h"
#include "lopts.h"

/* Validate string value to possible values. */
gint lopts_chk_str(const gchar *opt_val, const gchar **ok_strv, gchar **dst)
{
  gint i,r;

  *dst = NULL;

  if (opt_val == NULL)
    return (EXIT_SUCCESS);

  for (i=0, r=EXIT_FAILURE; ok_strv[i] != NULL; ++i)
    {
      if (g_strcmp0(ok_strv[i], opt_val) == 0)
        {
          r = EXIT_SUCCESS;
          break;
        }
    }

  if (r == EXIT_FAILURE)
    *dst = (gchar*) opt_val;

  return (r);
}

/* Validate string array of values to possible values. */
gint lopts_chk_strv(const gchar **opt_val, const gchar **ok_strv, gchar **dst)
{
  gint i, r;

  *dst = NULL;

  if (opt_val == NULL)
    return (EXIT_SUCCESS);

  for (i=0, r=EXIT_SUCCESS; opt_val[i] != NULL && r == EXIT_SUCCESS; ++i)
    r = lopts_chk_str(opt_val[i], ok_strv, dst);

  return (r);
}

/* Validate regex patterns. */
gint lopts_chk_re(const gchar **opt_val, gchar **dst)
{
  gboolean is_valid;
  gint r, i;

  *dst = NULL;

  if (opt_val == NULL)
    return (EXIT_SUCCESS);

  for (i=0, r=EXIT_SUCCESS; opt_val[i] != NULL && r == EXIT_SUCCESS; ++i)
    {
      lutil_regex_op_new(opt_val[i], &is_valid);
      if (is_valid == FALSE)
        {
          *dst = (gchar*) opt_val[i];
          r = EXIT_FAILURE;
        }
    }
  return (r);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
