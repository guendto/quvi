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
#include <quvi.h>

#include "lutil.h"

/* Dump the arg array. */
static void dump_argv(const gchar *exec_arg, gchar **argv)
{
  gchar *s = g_strjoinv(" ", argv);
  g_message("[%s]\n  exec_arg=%s\n  => %s", __func__, exec_arg, s);
  g_free(s);
}

/* Execute a command asynchronously. */
gint lutil_exec_cmd(lutil_exec_opts_t opts)
{
  struct lutil_xchg_seq_opts_s xopts;
  lutil_xchg_seq_t xseq;
  GSpawnFlags flags;
  gint argc, i, r;
  gchar **argv;
  GError *e;
  GPid pid;

  g_assert(opts != NULL);
  g_assert(opts->exec_arg != NULL);
  g_assert(opts->xperr != NULL);
  g_assert(opts->qm != NULL);

  r = EXIT_SUCCESS;
  argv = NULL;
  argc = 0;
  e = NULL;

  if (g_shell_parse_argv(opts->exec_arg, &argc, &argv, &e) == FALSE)
    {
      opts->xperr(_("while executing a command asynchronously: %s"),
                  e->message);
      g_error_free(e);
      return (EXIT_FAILURE);
    }

  memset(&xopts, 0, sizeof(struct lutil_xchg_seq_opts_s));

  xopts.file_ext = opts->file_ext;
  xopts.fpath = opts->fpath;
  xopts.xperr = opts->xperr;
  xopts.qm = opts->qm;

  xseq = lutil_xchg_seq_new(&xopts);

  for (i=0; i<argc; ++i)
    {
      gchar *s = lutil_xchg_seq_apply(xseq, argv[i]);
      if (s != NULL)
        {
          g_free(argv[i]);
          argv[i] = s;
        }
    }

  lutil_xchg_seq_free(xseq);

  if (opts->flags.dump_argv == TRUE)
    dump_argv(opts->exec_arg, argv);

  flags = G_SPAWN_SEARCH_PATH;

  if (opts->flags.discard_stderr == TRUE)
    flags |= G_SPAWN_STDERR_TO_DEV_NULL;

  if (opts->flags.discard_stdout == TRUE)
    flags |= G_SPAWN_STDOUT_TO_DEV_NULL;

  if (g_spawn_async(NULL, argv, NULL, flags,
                    NULL, NULL, &pid, &e) == FALSE)
    {
      opts->xperr(_("while spawning a new process: %s (code=0x%x)"),
                  e->message, e->code);
      r = EXIT_FAILURE;
      g_error_free(e);
    }

  g_strfreev(argv);
  return (r);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
