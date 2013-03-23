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

#include <string.h>
#include <glib/gi18n.h>
#include <quvi.h>

#include "lutil.h"

gchar *lutil_build_fpath(lutil_build_fpath_t p)
{
  gchar *fname, *fpath;

  g_assert(p != NULL);
  g_assert(p->output_regex != NULL);
  g_assert(p->output_name != NULL);
  g_assert(p->xperr != NULL);
  g_assert(p->qm != NULL);

  fname = NULL;
  fpath = NULL;

  if (p->output_file != NULL && strlen(p->output_file) >0)
    fname = g_strdup(p->output_file);
  else
    {
      struct lutil_xchg_seq_opts_s xopts;
      lutil_xchg_seq_t xseq;

      memset(&xopts, 0, sizeof(struct lutil_xchg_seq_opts_s));
      xopts.output_regex = (const gchar**) p->output_regex;
      xopts.file_ext = p->file_ext;
      xopts.xperr = p->xperr;
      xopts.qm = p->qm;

      xseq = lutil_xchg_seq_new(&xopts);
      fname = lutil_xchg_seq_apply(xseq, p->output_name);
      lutil_xchg_seq_free(xseq);

      if (fname == NULL)
        return (NULL);
    }

  if (p->output_dir != NULL && strlen(p->output_dir) >0)
    fpath = g_build_path(G_DIR_SEPARATOR_S, p->output_dir, fname, NULL);
  else
    {
      gchar *cwd = g_get_current_dir();
      fpath = g_build_path(G_DIR_SEPARATOR_S, cwd, fname, NULL);
      g_free(cwd);
    }

  g_free(fname);
  return (fpath);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
