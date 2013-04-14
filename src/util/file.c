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
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include "lutil.h"

static gint _perr(lutil_file_open_t p, const gchar *c)
{
  if (c != NULL)
    {
      gchar *s = lutil_strerror();
      p->xperr(_("%s: while opening file: %s: %s"), c, p->fpath, s);
      g_free(s);
    }
  return (EXIT_FAILURE);
}

static gint _chk_len(lutil_file_open_t p, gchar **mode)
{
#ifdef HAVE_GLIB_2_26
  GStatBuf b;
#else
  struct stat b;
#endif
  if (g_stat(p->fpath, &b) == -1)
    return (_perr(p, "g_stat"));

  if (b.st_size >= p->content_bytes)
    {
      p->result.skip_retrieved_already = TRUE;
      return (_perr(p, NULL));
    }
  else if (b.st_size < p->content_bytes)
    {
      p->result.initial_bytes = b.st_size;
      if (mode != NULL)
        *mode = "ab";
    }
  return (EXIT_SUCCESS);
}

gint lutil_file_open(lutil_file_open_t p)
{
  gchar *mode;

  g_assert(p != NULL);
  g_assert(p->xperr != NULL);
  g_assert(p->fpath != NULL);

  p->result.skip_retrieved_already = FALSE;
  p->result.initial_bytes = 0;
  p->result.file = NULL;
  mode = "wb";

  if (p->overwrite_if_exists == FALSE)
    {
      if (g_file_test(p->fpath, G_FILE_TEST_EXISTS) == TRUE)
        {
          if (_chk_len(p, &mode) != EXIT_SUCCESS)
            return (EXIT_FAILURE);
        }
    }

  p->result.file = fopen(p->fpath, mode);
  if (p->result.file == NULL)
    return (_perr(p, "fopen"));

  return (EXIT_SUCCESS);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
