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
#include <glib/gi18n.h>
#include <quvi.h>

#include "lutil.h"

static glong _support(const lutil_check_support_t css, const gchar *url,
                      const QuviSupportsType type)
{
  glong qc;

  if (quvi_supports(css->q, url, css->mode, type) == QUVI_TRUE)
    return (QUVI_OK);

  qc = quvi_errcode(css->q);
  if (qc != QUVI_ERROR_NO_SUPPORT)
    {
      css->xperr(_("libquvi: while checking for support: %s"),
                 quvi_errmsg(css->q));
    }
  return (qc);
}

static glong _chk(const lutil_check_support_t css, const gchar *url,
                  const QuviSupportsType qst, GSList **dst)
{
  const glong r = _support(css, url, qst);
  if (r == QUVI_OK)
    *dst = lutil_slist_prepend_if_unique(*dst, url);
  return (r);
}

static glong _chk_if_playlist(lutil_check_support_t css, const gchar *url)
{
  return (_chk(css, url, QUVI_SUPPORTS_TYPE_PLAYLIST, &css->url.playlist));
}

static glong _chk_if_subtitle(lutil_check_support_t css, const gchar *url)
{
  return (_chk(css, url, QUVI_SUPPORTS_TYPE_SUBTITLE, &css->url.subtitle));
}

static glong _chk_if_media(lutil_check_support_t css, const gchar *url)
{
  return (_chk(css, url, QUVI_SUPPORTS_TYPE_MEDIA, &css->url.media));
}

/* check method */

typedef glong (*chk_method_cb)(lutil_check_support_t, const gchar*);

struct chk_method_s
{
  chk_method_cb cb;
};

static const struct chk_method_s chk_methods[] =
{
  {_chk_if_media},
  {_chk_if_playlist},
  {NULL}
};

static const struct chk_method_s chk_methods_subtitle_only[] =
{
  {_chk_if_subtitle},
  {NULL}
};

typedef struct chk_method_s *chk_method_t;

void lutil_check_support(gpointer p, gpointer userdata)
{
  lutil_check_support_t css;
  const gchar *url;
  chk_method_t m;

  g_assert(userdata != NULL);
  g_assert(p != NULL);

  css = (lutil_check_support_t) userdata;
  url = (const gchar*) p;

  g_assert(css->xperr != NULL);
  g_assert(css->q != NULL);

  /* Start with offline mode... */
  css->mode = QUVI_SUPPORTS_MODE_OFFLINE;
  css->exit_status = EXIT_FAILURE;

retry:
  m = (chk_method_t) ((css->flags.force_subtitle_mode == TRUE)
                      ? chk_methods_subtitle_only
                      : chk_methods);

  while (m->cb != NULL)
    {
      const glong r = m->cb(css, url);
      switch (r)
        {
        case QUVI_OK:
          css->exit_status = EXIT_SUCCESS;
        default:
          return;
        case QUVI_ERROR_NO_SUPPORT:
          break;
        }
      ++m;
    }

  /* ... Try online mode (unless offline mode is forced). */
  if (css->mode == QUVI_SUPPORTS_MODE_OFFLINE
      && css->flags.force_offline_mode == FALSE)
    {
      css->mode = QUVI_SUPPORTS_MODE_ONLINE;
      goto retry;
    }

  css->xperr(_("cannot find matching libquvi script for <%s>"), url);
}

void lutil_check_support_free(lutil_check_support_t css)
{
  if (css == NULL)
    return;

  lutil_slist_free_full(css->url.playlist, (GFunc) g_free);
  lutil_slist_free_full(css->url.subtitle, (GFunc) g_free);
  lutil_slist_free_full(css->url.media, (GFunc) g_free);

  css->url.playlist = NULL;
  css->url.subtitle = NULL;
  css->url.media = NULL;

  /* Do not free quvi_t handle. */
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
