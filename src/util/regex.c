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

/*
 * Supports Perl-like syntax. Supports the 'i' modifier.
 *   m//  - match operation ('m' is optional)
 *   s/// - substitution operation
 *
 * NOTES:
 *  - Matches/replaces all occurences (similar to 'g' modifier of Perl)
 *  - Similar to Perl-syntax; with the exception of the "property
 *    sequence" -prefix being mandatory  (e.g. "%t:s/foo/bar/i")
 */

#include "config.h"

#include <string.h>
#include <glib/gi18n.h>

#include "lutil.h"

/* Check if the pattern is a s/// operation. */
static gboolean _chk_opmode_s(const gchar *p, lutil_regex_op_t *op)
{
  static const gchar *op_s = "^(%\\w):s/(.*)/(.*)/(.*)$";

  GMatchInfo *m;
  GRegex *re;
  gboolean r;
  GError *e;

  *op = NULL;
  r = FALSE;
  m = NULL;
  e = NULL;

  re = g_regex_new(op_s, 0, 0, &e);
  if (e != NULL)
    {
      g_printerr(_("error: %s: while creating the s/// "
                   "operation mode regex: %s\n"), op_s, e->message);
      g_error_free(e);
      return (FALSE);
    }

  r = g_regex_match(re, p, 0, &m);
  if (r == TRUE)
    {
      *op = g_new0(struct _lutil_regex_op_s, 1);
      (*op)->sequence = g_match_info_fetch(m, 1);
      (*op)->mode = g_strdup("s");
      (*op)->regex = g_match_info_fetch(m, 2);
      (*op)->subst.replacement = g_match_info_fetch(m, 3);
      (*op)->modifiers = g_match_info_fetch(m, 4);
    }
  g_match_info_free(m);
  g_regex_unref(re);
  return (r);
}

/* Check if the pattern is a m// operation. */
static gboolean _chk_opmode_m(const gchar *p, lutil_regex_op_t *op)
{
  static const gchar *op_m = "^(%\\w):(.*)/(.*)/(.*)$";

  GMatchInfo *m;
  GRegex *re;
  gboolean r;
  GError *e;

  *op = NULL;
  r = FALSE;
  m = NULL;
  e = NULL;

  re = g_regex_new(op_m, 0, 0, &e);
  if (e != NULL)
    {
      g_printerr(_("error: %s: while creating the m// "
                   "operation mode regex: %s\n"), op_m, e->message);
      g_error_free(e);
      return (FALSE);
    }

  r = g_regex_match(re, p, 0, &m);
  if (r == TRUE)
    {
      *op = g_new0(struct _lutil_regex_op_s, 1);
      (*op)->sequence = g_match_info_fetch(m, 1);
      (*op)->mode = g_match_info_fetch(m, 2);
      if (strlen((*op)->mode) ==0)
        {
          g_free((*op)->mode);
          (*op)->mode = g_strdup("m");
        }
      (*op)->regex = g_match_info_fetch(m, 3);
      (*op)->modifiers = g_match_info_fetch(m, 4);
    }
  g_match_info_free(m);
  g_regex_unref(re);
  return (r);
}

#ifdef _1
static void _dump(lutil_regex_op_t op)
{
  g_message("[%s] op->modifiers=%s", __func__, op->modifiers);
  g_message("[%s] op->sequence=%s", __func__, op->sequence);
  g_message("[%s] op->regex=%s", __func__, op->regex);
  g_message("[%s] op->mode=%s", __func__, op->mode);
  g_message("[%s] op->subst.replacement=%s", __func__, op->subst.replacement);
}
#endif

static const gchar *_EINVSYN =
  N_("error: %s: invalid syntax: must be either "
     "m// or s/// operation\n");

/* If is_valid is set, then validate the pattern only (and return NULL). */
lutil_regex_op_t lutil_regex_op_new(const gchar *p, gboolean *is_valid)
{
  lutil_regex_op_t op = NULL;

  if (is_valid != NULL)
    *is_valid = FALSE;

  if (_chk_opmode_s(p, &op) == FALSE)
    {
      if (_chk_opmode_m(p, &op) == FALSE)
        {
          g_printerr(g_dgettext(GETTEXT_PACKAGE, _EINVSYN), p);
          return (NULL);
        }
    }

  if (g_strcmp0(op->mode,"s") ==0)
    {
      /* For lack of a better regex, check this the hard way. */
      if (op->subst.replacement ==NULL)
        {
          g_printerr(_("error: %s: invalid s/// operation syntax\n"), p);
          return (NULL);
        }
      else
        {
          if (is_valid != NULL)
            *is_valid = TRUE;
        }
    }
  else if (g_strcmp0(op->mode, "m") ==0)
    {
      if (is_valid != NULL)
        *is_valid = TRUE;
    }
  else
    g_printerr(g_dgettext(GETTEXT_PACKAGE, _EINVSYN), p);

#ifdef _1
  _dump(op);
#endif

  if (is_valid != NULL)
    {
      lutil_regex_op_free(op);
      op = NULL;
    }
  return (op);
}

void lutil_regex_op_free(lutil_regex_op_t op)
{
  if (op == NULL)
    return;

  g_free(op->subst.replacement);
  g_free(op->modifiers);
  g_free(op->sequence);
  g_free(op->regex);
  g_free(op->mode);
  g_free(op);

  memset(op, 0, sizeof(struct _lutil_regex_op_s));
}

static gchar *_op_m(lutil_regex_op_t op, const gchar *s)
{
  GRegexCompileFlags flags;
  GMatchInfo *m;
  GRegex *re;
  GError *e;
  gchar *r;

  g_assert(op != NULL);

  flags = (g_strrstr(op->modifiers, "i") != NULL) ? G_REGEX_CASELESS:0;
  e = NULL;
  r = NULL;

  re = g_regex_new(op->regex, flags, 0, &e);
  if (e != NULL)
    {
      g_printerr(_("error: %s: while creating m// "
                   "operation mode regex: %s\n"), op->regex, e->message);
      g_error_free(e);
      return (NULL);
    }

  g_regex_match_full(re, s, -1, 0, 0, &m, &e);
  if (e != NULL)
    {
      g_printerr(_("error: %s: while matching: %s\n"), op->regex, e->message);
      g_error_free(e);
    }
  else
    {
      GString *t = g_string_new(NULL);
      while (g_match_info_matches(m) == TRUE)
        {
          gchar *v = g_match_info_fetch(m, 0);
          g_string_append(t, v);
          g_free(v);
          g_match_info_next(m, &e);
          if (e != NULL)
            {
              g_printerr(_("error: while retrieving match info: %s\n"),
                         e->message);
              g_error_free(e);
              break;
            }
        }
      g_match_info_free(m);
      r = g_strdup(t->str);
      g_string_free(t, TRUE);
    }
  g_regex_unref(re);
  return (r);
}

static gchar *_op_s(lutil_regex_op_t op, const gchar *s)
{
  GRegexCompileFlags flags;
  GRegex *re;
  GError *e;
  gchar *r;

  flags = (g_strrstr(op->modifiers, "i") != NULL) ? G_REGEX_CASELESS:0;
  e = NULL;
  r = NULL;

  re = g_regex_new(op->regex, flags, 0, &e);
  if (e != NULL)
    {
      g_printerr(_("error: %s: while creating s/// "
                   "operation mode regex: %s\n"), op->regex, e->message);
      g_error_free(e);
    }
  else
    {
      r = g_regex_replace(re, s, -1, 0, op->subst.replacement, 0, &e);
      if (e != NULL)
        {
          g_printerr(_("error: %s: while substituting: %s\n"),
                     op->regex, e->message);
          g_error_free(e);
        }
    }
  g_regex_unref(re);
  return (r);
}

/* g_free the returned string. */
gchar *lutil_regex_op_apply(lutil_regex_op_t op, const gchar *s)
{
  gchar *r = NULL;

  g_assert(op != NULL);
  g_assert(s != NULL);

  if (g_strcmp0(op->mode,"s") ==0)
    r = _op_s(op, s);
  else if (g_strcmp0(op->mode,"m") ==0)
    r = _op_m(op, s);

  return (r);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
