/* quvi
 * Copyright (C) 2013  Toni Gundogdu <legatvs@gmail.com>
 *
 * This file is part of quvi <http://quvi.sourceforge.net/>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <sys/ioctl.h>
#include <glib/gstdio.h>

#include "sig.h"

static volatile sig_atomic_t recv_sigwinch = 0;
static gsize max_width = 0;

static void _sigwinch(int signo)
{
  recv_sigwinch = 1;
}

void sigwinch_setup(struct sigaction *san, struct sigaction *sao)
{
  san->sa_handler = _sigwinch;
  sigemptyset(&san->sa_mask);
  san->sa_flags = 0;

  sigaction(SIGWINCH, NULL, sao);

  if (sao->sa_handler != SIG_IGN)
    sigaction(SIGWINCH, san, NULL);
}

void sigwinch_reset(const struct sigaction *sao)
{
  if (sao->sa_handler != SIG_IGN)
    sigaction(SIGWINCH, sao, NULL);
}

static gsize _get_term_width()
{
  struct winsize w;
  if (ioctl(fileno(stderr), TIOCGWINSZ, &w) <0)
    return (0);
  return (w.ws_col);
}

gsize sigwinch_term_spaceleft(const gsize len)
{
  static const gsize default_term_width = 80;
  if (max_width ==0 || recv_sigwinch ==1)
    {
      max_width = _get_term_width();
      if (max_width ==0)
        max_width = default_term_width;
    }
  return (max_width - len -1);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
