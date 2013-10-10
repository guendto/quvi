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
#include <locale.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <quvi.h>

#include "opts.h"
#include "cmd.h"

const gchar *reserved_chars = "!*'();:@&=+$,/?#[]";

struct opts_s opts;
gint exit_status;
gchar *argv0;
gchar *cmd;

static void _setup_gettext()
{
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
}

static gint _exec_man_man(const gchar *page)
{
  gchar *cmdl;
  GError *e;
  gint r;

  cmdl = g_strdup_printf("man %s", page);
  r = EXIT_SUCCESS;
  e = NULL;

  if (g_spawn_command_line_sync(cmdl, NULL, NULL, NULL, &e) == FALSE)
    {
      g_printerr(_("error: while executing: `%s': %s\n"), cmdl, e->message);
      r = EXIT_FAILURE;
      g_error_free(e);
    }

  g_free(cmdl);
  return (r);
}

static gint _cmd_version(gint, gchar**);
static gint _cmd_help(gint, gchar**);

typedef gint (*cmd_cb)(gint, gchar**);

struct builtin_cmd_s
{
  const gchar *cmd;
  const gchar *descr;
  const cmd_cb cb;
};

static const struct builtin_cmd_s builtin_cmds[] =
{
  {"dump", N_("Query and print the property values"), cmd_dump},
  {"get", N_("Save media stream to a file"), cmd_get},
  {"info", N_("Inspect the configuration and the script properties"), cmd_info},
  {"scan", N_("Scan and print the found embedded media URLs"), cmd_scan},
  /* version */
  {"--version", "", _cmd_version},
  {"-v", "",        _cmd_version},
  /* help */
  {"--help", "", _cmd_help},
  {"help", "",   _cmd_help},
  {"-h", "",     _cmd_help},
  /* eol */
  {NULL, NULL}
};

typedef struct builtin_cmd_s *builtin_cmd_t;

static gint _show_man_page(const gchar *quvi_cmd)
{
  gchar *p;
  gint r;

  p = g_strdup_printf("quvi-%s", quvi_cmd);
  r = _exec_man_man(p);
  g_free(p);

  return (r);
}

static gint _cmd_help(gint argc, gchar **argv)
{
  builtin_cmd_t p = (builtin_cmd_t) builtin_cmds;

  if (argc >1)
    {
      _show_man_page(argv[1]);
      return (EXIT_SUCCESS);
    }

  g_print(_("Usage: quvi [--version] [--help] COMMAND [ARGS]\n\n"));
  g_print(_("quvi commands are:\n"));

  while (p->cmd != NULL)
    {
      if (g_strcmp0(p->cmd, "help") != 0
          && g_str_has_prefix(p->cmd, "-") == FALSE)
        {
          g_print("  %-6s  %s\n", p->cmd, p->descr);
        }
      ++p;
    }

  g_print(_("\nSee 'quvi help COMMAND' for more information on a "
            "specific command.\n"));

  return (EXIT_SUCCESS);
}

static const gchar copyr[] =
  "Copyright (C) 2012,2013  Toni Gundogdu <legatvs@gmail.com>\n"
  "quvi comes with ABSOLUTELY NO WARRANTY.  You may redistribute copies of\n"
  "quvi under the terms of the GNU Affero General Public License version 3\n"
  "or later. For more information, see "
  "<http://www.gnu.org/licenses/agpl.html>.\n\n"
  "To contact the developers, please mail to "
  "<quvi-devel@lists.sourceforge.net>";

static gint _cmd_version(gint argc, gchar **argv)
{
  g_print("quvi %s\n  built on %s for %s\n"
          "    with %s, %s\n"
          "  configuration: %s\n"
          "libquvi %s\n  built on %s for %s\n"
          "    with %s\n"
          "  configuration: %s\n"
          "libquvi-scripts %s\n"
          "  configuration: %s\n",
          VN, BUILD_TIME, CANONICAL_TARGET, CC, CFLAGS, BUILD_OPTS,
          quvi_version(QUVI_VERSION),
          quvi_version(QUVI_VERSION_BUILD_TIME),
          quvi_version(QUVI_VERSION_BUILD_TARGET),
          quvi_version(QUVI_VERSION_BUILD_CC_CFLAGS),
          quvi_version(QUVI_VERSION_CONFIGURATION),
          quvi_version(QUVI_VERSION_SCRIPTS),
          quvi_version(QUVI_VERSION_SCRIPTS_CONFIGURATION));
  g_printerr("\n%s\n", copyr);
  return (EXIT_SUCCESS);
}

static gint _run_internal_cmd(gint argc, gchar **argv)
{
  builtin_cmd_t p = (builtin_cmd_t) builtin_cmds;
  while (p->cmd != NULL)
    {
      if (g_strcmp0(p->cmd, cmd) == 0)
        return (p->cb(argc, argv));
      ++p;
    }
  g_printerr(_("error: `%s' is not a quvi command. See 'quvi help'.\n"), cmd);
  return (EXIT_FAILURE);
}

static void _opts_free()
{
  /* core */

  g_free(opts.core.subtitle_export_format);
  g_free(opts.core.subtitle_language);
  g_free(opts.core.print_format);
  g_free(opts.core.verbosity);
  g_free(opts.core.stream);

  /* exec */

  g_strfreev(opts.exec.external);

  /* get */

  g_strfreev(opts.get.output_regex);
  g_free(opts.get.output_name);
  g_free(opts.get.output_file);
  g_free(opts.get.output_dir);

  /* http */

  g_free(opts.http.user_agent);

  /* other */

  g_strfreev(opts.rargs);

  memset(&opts, 0, sizeof(struct opts_s));
}

static gint _cleanup()
{
  _opts_free();

  g_free(argv0);
  argv0 = NULL;

  g_free(cmd);
  cmd = NULL;

  return (exit_status);
}

gint main(gint argc, gchar **argv)
{
  exit_status = EXIT_SUCCESS;
  argv0 = NULL;

  /* quvi COMMAND. */
  cmd = g_strdup(argv[1]);
  if (cmd == NULL)
    cmd = g_strdup("help");

  argc--;
  argv++;

  _setup_gettext();
#if !GLIB_CHECK_VERSION(2,35,0)
  g_type_init();
#endif

  exit_status = _run_internal_cmd(argc, argv);
  return (_cleanup());
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
