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

typedef enum {TSTRING, TDOUBLE} PropertyType;

struct _media_xchg_table_s
{
  const QuviMediaProperty qmp;
  const PropertyType type;
  const gchar *seq;
};

static const struct _media_xchg_table_s media_xchg_table[] =
{
  {QUVI_MEDIA_PROPERTY_START_TIME_MS, TDOUBLE, "%s"},
  {QUVI_MEDIA_PROPERTY_THUMBNAIL_URL, TSTRING, "%T"},
  {QUVI_MEDIA_PROPERTY_DURATION_MS,   TDOUBLE, "%d"},
  {QUVI_MEDIA_STREAM_PROPERTY_URL,    TSTRING, "%u"},
  {QUVI_MEDIA_STREAM_PROPERTY_ID,     TSTRING, "%I"},
  {QUVI_MEDIA_PROPERTY_TITLE,         TSTRING, "%t"},
  {QUVI_MEDIA_PROPERTY_ID,            TSTRING, "%i"},
  {0, 0, NULL}
};

static const gchar *default_str = N_("default");

/* Return a media property value. */
static gchar *_qm_get(quvi_media_t qm, const gint i)
{
  gchar *r = NULL;

  switch (media_xchg_table[i].type)
    {
    case TSTRING:
    {
      gchar *s;
      quvi_media_get(qm, media_xchg_table[i].qmp, &s);

      if (s != NULL && strlen(s) >0)
        r = g_strdup(s);
    }
    break;

    case TDOUBLE:
    {
      gdouble d;
      quvi_media_get(qm, media_xchg_table[i].qmp, &d);
      return (g_strdup_printf("%.0f", d));
    }
    break;

    default:
      g_warning("[%s] invalid media xchg table type (%d)",
                __func__, media_xchg_table[i].type);
      return (NULL);
    }
  return ((r == NULL) ? g_strdup(default_str):r);
}

/* Regexp eval-callback. */
static gboolean _eval_cb(const GMatchInfo *i, GString *r, gpointer p)
{
  gchar *m, *s;

  m = g_match_info_fetch(i, 0);
  s = g_hash_table_lookup((GHashTable*) p, m);
  g_string_append(r, s);
  g_free(m);

  return (FALSE);
}

/* Append a sequence to the replace-pattern. */
static void _pattern_append(GString *p, const gchar *s)
{
  if (p->len >0)
    g_string_append(p, "|");
  g_string_append(p, s);
}

/* Return a new regex cache. */
static GSList *_regex_cache_new(const gchar **output_regex)
{
  lutil_regex_op_t op;
  GSList *r;
  gint i;

  if (output_regex == NULL)
    return(NULL);

  for (i=0, r=NULL; output_regex[i] != NULL; ++i)
    {
      op = lutil_regex_op_new(output_regex[i], NULL);
      if (op != NULL)
        r = g_slist_prepend(r, op);
    }
  return (g_slist_reverse(r));
}

/* Release regex cache. */
static void _regex_cache_free(GSList *l)
{
  if (l == NULL)
    return;

  lutil_slist_free_full(l, (GFunc) lutil_regex_op_free);
}

/* Apply regex onto the media property value. */
static gchar *_apply_regex(GSList *l, const gchar *seq, gchar *s)
{
  GSList *curr = l;
  while (curr != NULL)
    {
      lutil_regex_op_t op = (lutil_regex_op_t) curr->data;
      if (g_strcmp0(op->sequence, seq) ==0)
        {
          gchar *r = lutil_regex_op_apply(op, s);
          if (r != NULL)
            {
              g_free(s);
              s = r;
            }
        }
      curr = g_slist_next(curr);
    }
  return (s);
}

/* Return a new sequence handle. */
static lutil_xchg_seq_t _new(GSList **regex_cache, const gchar **output_regex,
                             const lutil_cb_printerr xperr, GString **p)
{
  lutil_xchg_seq_t x = g_new0(struct _lutil_xchg_seq_s, 1);

  x->htable = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
  x->printerr = xperr;

  *regex_cache = _regex_cache_new(output_regex);
  *p = g_string_new(NULL);

  return (x);
}

/* Insert media property value into hash table. */
static void _insert(lutil_xchg_seq_t x, GSList *regex_cache,
                    const gchar *seq, gchar *s, GString *p)
{
  if (s == NULL)
    return;

  if (regex_cache != NULL)
    s = _apply_regex(regex_cache, seq, s);

  g_hash_table_insert(x->htable, (gchar*) seq, s);
  _pattern_append(p, seq);
}

/* Create a new regex instance used to replace the sequences. */
static lutil_xchg_seq_t _new_regex(lutil_xchg_seq_t x, GString *p, GSList *c)
{
  GError *e = NULL;

  x->re = g_regex_new(p->str, 0, 0, &e);
  if (e != NULL)
    {
      x->printerr(_("while creating sequence regex: %s"), e->message);
      lutil_xchg_seq_free(x);
      g_error_free(e);
      x = NULL;
    }

  g_string_free(p, TRUE);
  _regex_cache_free(c);

  return (x);
}

/* Return a new sequence handle. */
lutil_xchg_seq_t lutil_xchg_seq_new(const lutil_xchg_seq_opts_t xopts)
{
  lutil_xchg_seq_t x;
  GString *p;
  GSList *c;
  gint i;

  g_assert(xopts->xperr != NULL);
  g_assert(xopts->qm != NULL);

  x = _new(&c, xopts->output_regex, xopts->xperr, &p);

  for (i=0; media_xchg_table[i].seq != NULL; ++i)
    _insert(x, c, media_xchg_table[i].seq, _qm_get(xopts->qm,i), p);

  if (xopts->file_ext != NULL)
    _insert(x, c, "%e", g_strdup(xopts->file_ext), p);

  if (xopts->fpath != NULL)
    _insert(x, c, "%f", g_strdup(xopts->fpath), p);

  return (_new_regex(x, p, c));
}

/* Return a new sequence handle -- without quvi_media_t and quvi_metainfo_t */
lutil_xchg_seq_t lutil_xchg_seq_noq_new(lutil_xchg_seq_noq_t noq,
                                        const lutil_cb_printerr xperr,
                                        const gchar **output_regex)
{
  lutil_xchg_seq_t x;
  GString *p;
  GSList *c;
  gchar *s;
  gint i;

  g_assert(xperr != NULL);
  g_assert(noq != NULL);

  x = _new(&c, output_regex, xperr, &p);

  for (i=0; noq[i].seq != NULL; ++i)
    {
      s = g_strdup( (noq[i].val ==NULL || strlen(s) ==0)
                    ? default_str
                    : noq[i].val);
      _insert(x, c, noq[i].seq, s, p);
    }
  return (_new_regex(x, p, c));
}

/* Free sequence handle. */
void lutil_xchg_seq_free(lutil_xchg_seq_t p)
{
  if (p == NULL)
    return;

  g_hash_table_destroy(p->htable);
  g_regex_unref(p->re);
  g_free(p);

  memset(p, 0, sizeof(struct _lutil_xchg_seq_s));
}

/* Replace the sequences in the given string (g_free the returned string). */
gchar *lutil_xchg_seq_apply(lutil_xchg_seq_t p, const gchar *s)
{
  GError *e;
  gchar *r;

  r = NULL;
  e = NULL;

  r = g_regex_replace_eval(p->re, s, -1, 0, 0, _eval_cb, p->htable, &e);
  if (e != NULL)
    {
      p->printerr(_("while replacing sequences: %s"), e->message);
      g_error_free(e);
      return (NULL);
    }
  return (r);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
