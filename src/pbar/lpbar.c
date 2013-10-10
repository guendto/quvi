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

#include <glib/gi18n.h>

/* -- */
#include "lpbar.h"

typedef enum
{
  Ki = 1024,
  Mi = 1048576,
  Gi = 1073741824
} ieee_1541;

#define _s(n) #n
static const gchar *_units[] =
{
  _s(Ki),
  _s(Mi),
  _s(Gi),
  NULL
};
#undef _s

static const gchar *_to_unit(gdouble *n)
{
  gint i = 0;
  if (*n >=Gi)
    {
      *n /= Gi;
      i = 2;
    }
  else if (*n >=Mi)
    {
      *n /= Mi;
      i = 1;
    }
  else
    *n /= Ki;
  return (_units[i]);
}

lpbar_t lpbar_new()
{
  lpbar_t p = g_new0(struct lpbar_s, 1);
  p->counters.timer = g_timer_new();
  p->mode = write;
  return (p);
}

void lpbar_free(lpbar_t p)
{
  if (p == NULL)
    return;

  if (p->content_bytes >0
      && (p->counters.count + p->initial_bytes >p->content_bytes))
    {
      p->content_bytes = p->initial_bytes + p->counters.count;
    }

  if (p->flags.failed == FALSE)
    {
      p->flags.done = TRUE;
      lpbar_update(p, -1);
    }

  g_timer_destroy(p->counters.timer);
  g_free(p->content_type);
  g_free(p->fname);
  g_free(p);

  memset(p, 0, sizeof(struct lpbar_s));
}

static const gchar *frames[] =
{
  "=---",
  "-=--",
  "--=-",
  "---=",
  NULL
};

static const gchar *_next_frame(lpbar_t p)
{
  if (frames[p->counters.curr_frame] == NULL)
    p->counters.curr_frame = 0;
  return (frames[p->counters.curr_frame++]);
}

static gchar *_eta(const glong s)
{
  if (s >=86400) /* 24h */
    return (g_strdup_printf(_("%ld hours"), (s/3600%60)));
  return (g_strdup_printf("%02ld:%02ld:%02ld", (s/3600)%60, (s/60)%60, s%60));
}

static const gdouble update_interval = .5;

gint lpbar_update(lpbar_t p, gdouble dlnow)
{
  gdouble rate, size, elapsed, percent;
  const gchar *rate_unit, *frame;
  gboolean inactive;
  gchar *eta;

  if (dlnow == 0 || p->flags.failed == TRUE)
    return (0);

  elapsed = g_timer_elapsed(p->counters.timer, NULL);

  if (p->flags.done == TRUE)
    dlnow = p->content_bytes;
  else
    {
      if ((elapsed - p->counters.last_update) < update_interval)
        return (0);
    }

  size = dlnow;
  if (p->flags.done == FALSE)
    size += p->initial_bytes;

  inactive = (dlnow == 0) ? TRUE:FALSE;
  rate = (elapsed >0) ? (dlnow/elapsed):0;

  if (inactive == FALSE)
    {
      if (p->flags.done == FALSE)
        {
          const gdouble left =
            (p->content_bytes - (dlnow + p->initial_bytes)) / rate;

          eta = _eta(left+.5);
        }
      else
        {
          rate = (p->content_bytes - p->initial_bytes) / elapsed;
          eta = _eta(elapsed);
        }
      rate_unit = _to_unit(&rate);
      frame = _next_frame(p);
    }
  else
    {
      frame = frames[p->counters.curr_frame];
      eta = g_strdup("--:--");
      rate_unit = "--.-";
    }

  percent = 0;
  if (p->content_bytes >0)
    {
      percent = (100.0 * size / p->content_bytes);
      if (percent >= 100)
        percent = 100;
    }

  g_print(_("copy: %s  %3.0f%%  %6.1f%s/s  %4s%s"),
          frame, percent, rate, rate_unit, eta,
          (p->flags.done == TRUE) ? "\n":"\r");

  p->counters.last_update = elapsed;
  p->counters.count = dlnow;
  g_free(eta);

  return (0);
}

void lpbar_print(const lpbar_t p)
{
  const gchar *u;
  gdouble b;

  b = p->content_bytes;
  u = _to_unit(&b);

  g_print(_("file: %s  [media]\n"), p->fname);
  g_print(_("  content length: %.1f%s"), b, u);

  if (p->content_type != NULL)
    g_print(_("  content type: %s"), p->content_type);

  g_print(C_("To indicate transfer mode (resumed, ...) ", "  mode: "));
  switch (p->mode)
  {
    case retrieved_already:
      g_print(C_("Transfer mode with a reason", "skip <retrieved already>"));
      break;
    case forced_skip:
      g_print(C_("Transfer mode with a reason", "skip <forced>"));
      break;
    case resume:
    case write:
    default:
      g_print("%s", (p->initial_bytes ==0)
        ? C_("Transfer mode (begin at offset 0)", "write")
        : C_("Transfer mode", "resume"));
      break;
  }
  g_print("\n");
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
