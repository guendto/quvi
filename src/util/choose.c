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

/* Choose a stream from the available streams. */
gint lutil_choose_stream(const quvi_t q, const quvi_media_t qm,
                         const gchar *stream, const lutil_cb_printerr xperr)
{
  g_assert(stream != NULL);
  g_assert(xperr != NULL);
  g_assert(qm != NULL);
  g_assert(q != NULL);

  quvi_media_stream_select(qm, stream);
  if (quvi_ok(q) == QUVI_FALSE)
    {
      xperr(_("libquvi: while selecting stream: %s"), quvi_errmsg(q));
      return (EXIT_FAILURE);
    }
  return (EXIT_SUCCESS);
}

/* Choose subtitle language from the available languages. */
gint lutil_choose_subtitle(const quvi_t q, const quvi_subtitle_t qsub,
                           const gchar *lang, const lutil_cb_printerr xperr,
                           quvi_subtitle_lang_t *l,
                           const gboolean fail_if_none)
{
  g_assert(xperr != NULL);
  g_assert(lang != NULL);
  g_assert(qsub != NULL);
  g_assert(q != NULL);

  *l = quvi_subtitle_select(qsub, lang);
  if (quvi_ok(q) == QUVI_FALSE)
    {
      xperr(_("libquvi: while selecting subtitle: %s"), quvi_errmsg(q));
      return (EXIT_FAILURE);
    }
  else
    {
      if (*l == NULL && fail_if_none == TRUE)
        {
          xperr(_("libquvi: failed to find any subtitles"));
          return (EXIT_FAILURE);
        }
      return (EXIT_SUCCESS);
    }
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
