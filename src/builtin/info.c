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
#include <string.h>
#include <glib/gi18n.h>
#include <quvi.h>

#include "lopts.h"
#include "lutil.h"
#include "opts.h"
#include "cmd.h"

struct _opts_s
{
  gboolean print_subtitle_export_formats;
  gboolean print_media_domains;
  gboolean print_config_values;
  gboolean print_config_paths;
  gchar **script_property;
  gchar *script_type;
};

static struct _opts_s _opts;
static quvi_t q = NULL;

static const GOptionEntry entries[] =
{
  {
    "print-config-values", 'V', 0, G_OPTION_ARG_NONE,
    &_opts.print_config_values, NULL, NULL
  },
  {
    "print-config-paths", 'P', 0, G_OPTION_ARG_NONE,
    &_opts.print_config_paths, NULL, NULL
  },
  {
    "print-media-domains", 'D', 0, G_OPTION_ARG_NONE,
    &_opts.print_media_domains, NULL, NULL
  },
  {
    "print-subtitle-export-formats", 'E', 0, G_OPTION_ARG_NONE,
    &_opts.print_subtitle_export_formats, NULL, NULL
  },
  {
    "script-property", 'p', 0, G_OPTION_ARG_STRING_ARRAY,
    &_opts.script_property, NULL, NULL
  },
  {
    "script-type", 't', 0, G_OPTION_ARG_STRING,
    &_opts.script_type, NULL, NULL
  },
  {NULL, 0, 0, 0, NULL, NULL, NULL}
};

struct _property_lookup_s
{
  QuviScriptProperty to;
  const gchar *from;
};

static const struct _property_lookup_s property_conv[] =
{
  {QUVI_SCRIPT_PROPERTY_EXPORT_FORMAT, "export.format"},
  {QUVI_SCRIPT_PROPERTY_FILEPATH, "filepath"},
  {QUVI_SCRIPT_PROPERTY_FILENAME, "filename"},
  {QUVI_SCRIPT_PROPERTY_DOMAINS,  "domains"},
  {QUVI_SCRIPT_PROPERTY_SHA1,     "sha1"},
  {0, NULL}
};

struct _type_lookup_s
{
  QuviScriptType to;
  const gchar *from;
};

static const struct _type_lookup_s type_conv[] =
{
  {QUVI_SCRIPT_TYPE_SUBTITLE_EXPORT, "subtitle.export"},
  {QUVI_SCRIPT_TYPE_SUBTITLE, "subtitle"},
  {QUVI_SCRIPT_TYPE_PLAYLIST, "playlist"},
  {QUVI_SCRIPT_TYPE_MEDIA,    "media"},
  {QUVI_SCRIPT_TYPE_SCAN,     "scan"},
  {0, NULL}
};

static gint cb_chk_str(const gchar *fpath, const gchar *opt_name,
                       const gchar *opt_val, const gchar **ok_strv)
{
  gchar *inv_val;
  if (lopts_chk_str(opt_val, ok_strv, &inv_val) != EXIT_SUCCESS)
    return (lopts_invalid_value(opt_name, fpath, inv_val, ok_strv));
  return (EXIT_SUCCESS);
}

static gint cb_chk_strv(const gchar *fpath, const gchar *opt_name,
                        const gchar **opt_val, const gchar **ok_strv)
{
  gchar *inv_val;
  if (lopts_chk_strv(opt_val, ok_strv, &inv_val) != EXIT_SUCCESS)
    return (lopts_invalid_value(opt_name, fpath, inv_val, ok_strv));
  return (EXIT_SUCCESS);
}

#define _new_sv(n)\
  do {\
    i = 0;\
    while (n[i].from != NULL)\
      ++i;\
    sv = g_new(gchar*, i+1);\
    i = 0; j = 0;\
    while (n[i].from != NULL)\
      sv[i++] = g_strdup(n[j++].from);\
    sv[i] = NULL;\
  } while (0)

#define _chk_r\
  do {\
    if (r != EXIT_SUCCESS)\
      return (r);\
  } while (0)

static gint _cb_cmdline_validate_values()
{
  gint r,i,j;
  gchar **sv;

  _new_sv(property_conv);
  r = cb_chk_strv(NULL, "script-property",
                  (const gchar**) _opts.script_property, (const gchar**) sv);
  g_strfreev(sv);
  _chk_r;

  _new_sv(type_conv);
  r = cb_chk_str(NULL, "script-type", _opts.script_type, (const gchar**) sv);
  g_strfreev(sv);
  _chk_r;

  return (r);
}

#undef _chk_r
#undef _new_sv

static void _cb_set_post_parse_defaults()
{
  if (_opts.script_property == NULL)
    {
      gchar *v[] = {"sha1", "filename", NULL};
      _opts.script_property = g_strdupv(v);
    }
  if (_opts.script_type == NULL)
    _opts.script_type = g_strdup("media");
}

static gint _parse_opts(lopts_t lopts, gint argc, gchar **argv)
{
  memset(lopts, 0, sizeof(struct lopts_s));

  lopts->entries = entries;
  lopts->argc    = argc;
  lopts->argv    = argv;

  lopts->cb.set_post_parse_defaults = _cb_set_post_parse_defaults;
  lopts->cb.cmdline_validate_values = _cb_cmdline_validate_values;

  return (lopts_new(lopts));
}

static gint _print_config_values(lopts_t lopts)
{
  memset(lopts, 0, sizeof(struct lopts_s));
  lopts->entries = option_entries;

  lopts->cb.set_post_parse_defaults = cb_set_post_parse_defaults;
  lopts->cb.parse_keyfile_values = cb_parse_keyfile_values;
  lopts->cb.get_config_fpath = cb_get_config_fpath;

  if (lopts_new(lopts) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  lopts_print_config_values(lopts);
  return (EXIT_SUCCESS);
}

static void _dump_script(const QuviScriptType qst)
{
  QuviScriptProperty qsp;
  const gchar *p;
  gchar *s;
  gint i,j;

  i = 0;
  while (_opts.script_property[i] != NULL)
    {
      p = _opts.script_property[i];
      j = 0;
      while (property_conv[i].from != NULL)
        {
          if (g_strcmp0(p, property_conv[j].from) ==0)
            {
              qsp = property_conv[j].to;
              break;
            }
          ++j;
        }
      quvi_script_get(q, qst, qsp, &s);
      if (i == 0)
        g_print(_("# Script\n"));
      g_print("%s: %s\n", p, s);
      ++i;
    }
}

static gint _cleanup(gint r)
{
  g_strfreev(_opts.script_property);
  g_free(_opts.script_type);
  quvi_free(q);

  memset(&_opts, 0, sizeof(struct _opts_s));
  return (r);
}

static QuviScriptType _to_script_type()
{
  gint i = 0;
  while (type_conv[i].from != NULL)
    {
      if (g_strcmp0(_opts.script_type, type_conv[i].from) ==0)
        return (type_conv[i].to);
      ++i;
    }
  return (QUVI_SCRIPT_TYPE_MEDIA);
}

static void _print_info(const QuviScriptType qst)
{
  gchar *r = g_strjoinv(",", _opts.script_property);
  g_print(_("# Type: %s (0x%x), Properties: %s\n#\n"),
          _opts.script_type, qst, r);
  g_free(r);
}

static void _print_scripts()
{
  const QuviScriptType qst = _to_script_type();

  _print_info(qst);

  while (quvi_script_next(q, qst) == QUVI_TRUE)
    _dump_script(qst);
}

static void _foreach_domain_str(gpointer p, gpointer userdata)
{
  GString *r = (GString*) userdata;
  g_string_append_printf(r, "%s\n", (gchar*) p);
}

static void _print_each_script_of(const QuviScriptType qst,
                                  const QuviScriptProperty qsp)
{
  GString *r;
  GSList *l;
  gchar *s;

  l = NULL;
  while (quvi_script_next(q, qst) == QUVI_TRUE)
    {
      quvi_script_get(q, qst, qsp, &s);
      l = g_slist_prepend(l, g_strdup(s));
    }

  l = g_slist_sort(l, (GCompareFunc) g_ascii_strcasecmp);
  r = g_string_new(NULL);

  g_slist_foreach(l, _foreach_domain_str, r);
  g_print("%s", r->str);

  lutil_slist_free_full(l, (GFunc) g_free);
  g_string_free(r, TRUE);
}

static void _print_media_domains()
{
  g_print(_("# Found media property scripts for the domains\n#\n"));
  _print_each_script_of(_to_script_type(), QUVI_SCRIPT_PROPERTY_DOMAINS);
}

static void _print_subtitle_export_formats()
{
  g_print(_("# Found export scripts for the subtitle formats\n#\n"));
  _print_each_script_of(QUVI_SCRIPT_TYPE_SUBTITLE_EXPORT,
                        QUVI_SCRIPT_PROPERTY_EXPORT_FORMAT);
}

gint cmd_info(gint argc, gchar **argv)
{
  struct lopts_s lopts;
  gint r;

  memset(&_opts, 0, sizeof(struct _opts_s));

  if (_parse_opts(&lopts, argc, argv) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  r = EXIT_SUCCESS;

  if (_opts.print_config_values == TRUE)
    r = _print_config_values(&lopts);
  else if (_opts.print_config_paths == TRUE)
    lopts_print_config_paths(&lopts);
  else
    {
      r = lutil_quvi_init(&q, NULL);
      if (r == EXIT_SUCCESS)
        {
          if (_opts.print_subtitle_export_formats == TRUE)
            _print_subtitle_export_formats();
          else if (_opts.print_media_domains == TRUE)
            _print_media_domains();
          else
            _print_scripts();
        }
    }
  return (_cleanup(r));
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
