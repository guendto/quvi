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

#include "config.h"

#include <stdlib.h>
#include <glib.h>
#include <quvi.h>

#include "lopts.h"

/* Return a string value from the keyfile. */
gchar *lopts_keyfile_get_str(GKeyFile *kf, lopts_cb_chk_ok_str cb_chk,
                             const gchar *conf_fpath, const gchar *grp_name,
                             const gchar **ok_strv, const gchar *opt_name,
                             gchar **dst)
{
  gchar *r = g_key_file_get_string(kf, grp_name, opt_name, NULL);

  if (cb_chk != NULL
      && cb_chk(conf_fpath, opt_name, r, ok_strv) != EXIT_SUCCESS)
    {
      g_free(r);
      return (*dst);
    }

  if (*dst != NULL)
    g_free(*dst);
  *dst = r;

  return (*dst);
}

/* Return a string array from the keyfile. */
gchar **lopts_keyfile_get_strv(GKeyFile *kf, lopts_cb_chk_ok_strv cb_chk,
                               const gchar *conf_fpath, const gchar *grp_name,
                               const gchar **ok_strv, const gchar *opt_name,
                               gchar ***dst)
{
  gchar **r = g_key_file_get_string_list(kf, grp_name, opt_name, NULL, NULL);

  if (r == NULL)
    return (*dst);

  if (cb_chk != NULL
      && (cb_chk(conf_fpath, opt_name, (const gchar**) r, ok_strv)
          != EXIT_SUCCESS))
    {
      g_strfreev(r);
      return (*dst);
    }

  if (*dst != NULL)
    g_strfreev(*dst);

  *dst = r;

  return (*dst);
}

/* Return a boolean value from the keyfile. */
gboolean lopts_keyfile_get_bool(GKeyFile *kf, const gchar *conf_fpath,
                                const gchar *grp_name, const gchar *opt_name,
                                gboolean *dst)
{
  gboolean r;
  GError *e;

  e = NULL;
  r = g_key_file_get_boolean(kf, grp_name, opt_name, &e);

  if (e != NULL)
    g_error_free(e);
  else
    *dst = r;

  return (*dst);
}

/* Return a double value from the keyfile. */
gint lopts_keyfile_get_double(GKeyFile *kf, lopts_cb_chk_ok_int cb_chk,
                              const gchar *conf_fpath, const gchar *grp_name,
                              const gchar *opt_name, gdouble *dst)
{
  GError *e;
  gdouble r;

  e = NULL;
  r = g_key_file_get_double(kf, grp_name, opt_name, &e);

  if (e != NULL)
    g_error_free(e);
  else
    {
      if (cb_chk != NULL
          && cb_chk(conf_fpath, opt_name, r) != EXIT_SUCCESS)
        {
          return (*dst);
        }
      *dst = r;
    }
  return (*dst);
}

/* Return an integer value from the keyfile. */
gint lopts_keyfile_get_int(GKeyFile *kf, lopts_cb_chk_ok_int cb_chk,
                           const gchar *conf_fpath, const gchar *grp_name,
                           const gchar *opt_name, gint *dst)
{
  GError *e;
  gint r;

  e = NULL;
  r = g_key_file_get_integer(kf, grp_name, opt_name, &e);

  if (e != NULL)
    g_error_free(e);
  else
    {
      if (cb_chk != NULL
          && cb_chk(conf_fpath, opt_name, r) != EXIT_SUCCESS)
        {
          return (*dst);
        }
      *dst = r;
    }
  return (*dst);
}

/* Return a regex (string) array from the keyfile. */
gchar **lopts_keyfile_get_re(GKeyFile *kf, lopts_cb_chk_ok_re cb_chk,
                             const gchar *conf_fpath, const gchar *grp_name,
                             const gchar **ok_strv, const gchar *opt_name,
                             gchar ***dst)
{
  gchar **r = g_key_file_get_string_list(kf, grp_name, opt_name, NULL, NULL);

  if (r == NULL)
    return (*dst);

  if (cb_chk != NULL
      && (cb_chk(conf_fpath, opt_name, (const gchar**) r) != EXIT_SUCCESS))
    {
      g_strfreev(r);
      return (*dst);
    }

  if (*dst != NULL)
    g_strfreev(*dst);

  *dst = r;

  return (*dst);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
