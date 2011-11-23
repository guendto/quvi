/* quvi
 * Copyright (C) 2009-2011  Toni Gundogdu <legatvs@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

/* quvi.c - query media tool. */

#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <quvi/quvi.h>
#include <quvi/llst.h>

#include <curl/curl.h>

#include "platform.h"
#include "cmdline.h"

#define _free(p) \
    do { if (p) { free(p); p=0; } } while (0)

static const char *config_fname = ".quvirc";

/* strepl.c */
extern char *strepl(const char *s, const char *what, const char *with);

static int verbose_flag = 1;
/*@null@*/
static quvi_t quvi = NULL;
/*@null@*/
static CURL *curl = NULL;

typedef struct gengetopt_args_info *opts_t;
/*@null@*/
static opts_t opts = NULL;

/*@null@*/
static quvi_llst_node_t input = NULL;

/* prints to std(e)rr. */
static void spew_e(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

/* respects (q)uiet, prints to std(e)rr. */
static void spew_qe(const char *fmt, ...)
{
  va_list ap;
  if (verbose_flag == 0)
    return;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

/* glorified printf. */
static void spew(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}

static void dump_error_json(quvi_t quvi, QUVIcode rc)
{
  fprintf(stderr,
          "{\n"
          "  \"error\": [\n"
          "    {\n"
          "      \"message\": \"%s\"\n"
          "    }\n"
          "  ]\n"
          "}\n", quvi_strerror(quvi, rc));
}

static void dump_error_xml(quvi_t quvi, QUVIcode rc)
{
  fprintf(stderr,
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<error>\n"
          "  <message>%s</message>\n"
          "</error>\n",
          quvi_strerror(quvi, rc));
}

static void dump_error(quvi_t quvi, QUVIcode rc)
{
  if (opts->export_errors_given == 1)
    {
      if (opts->xml_given == 1)
        dump_error_xml(quvi, rc);
      else
        dump_error_json(quvi, rc);
    }
  else
    fprintf(stderr, "error: %s\n", quvi_strerror(quvi, rc));
}

static void handle_resolve_status(quvi_word type)
{
  if (type == QUVISTATUSTYPE_DONE)
    spew_qe("done.\n");
  else
    spew_qe(":: Check for URL redirection ...");
}

static void handle_fetch_status(quvi_word type, void *p)
{
  switch (type)
    {
    default:
      spew_qe(":: Fetch %s ...", (char *)p);
      break;
    case QUVISTATUSTYPE_CONFIG:
      spew_qe(":: Fetch config ...");
      break;
    case QUVISTATUSTYPE_PLAYLIST:
      spew_qe(":: Fetch playlist ...");
      break;
    case QUVISTATUSTYPE_DONE:
      spew_qe("done.\n");
      break;
    }
}

static void handle_verify_status(quvi_word type)
{
  switch (type)
    {
    default:
      spew_qe(":: Verify media URL ...");
      break;
    case QUVISTATUSTYPE_DONE:
      spew_qe("done.\n");
      break;
    }
}

static int status_callback(long param, void *data)
{
  quvi_word status, type;

  status = quvi_loword(param);
  type = quvi_hiword(param);

  switch (status)
    {
    case QUVISTATUS_RESOLVE:
      handle_resolve_status(type);
      break;

    case QUVISTATUS_FETCH:
      handle_fetch_status(type, data);
      break;

    case QUVISTATUS_VERIFY:
      handle_verify_status(type);
      break;
    }

  fflush(stderr);

  return (0);
}

/* Divided into smaller blocks. Otherwise -pedantic objects. */

#define LICENSE_1 \
"/* quvi\n" \
" * Copyright (C) 2009-2011  Toni Gundogdu <legatvs@gmail.com>\n"

#define LICENSE_2 \
" * This library is free software; you can redistribute it and/or\n" \
" * modify it under the terms of the GNU Lesser General Public\n" \
" * License as published by the Free Software Foundation; either\n" \
" * version 2.1 of the License, or (at your option) any later version.\n"

#define LICENSE_3 \
" * This library is distributed in the hope that it will be useful,\n" \
" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n" \
" * Lesser General Public License for more details.\n"

#define LICENSE_4 \
" * You should have received a copy of the GNU Lesser General Public\n" \
" * License along with this library; if not, write to the Free Software\n" \
" * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA\n" \
" * 02110-1301  USA\n" " */"

static void license()
{
  printf("%s *\n%s *\n%s *\n%s\n",
         LICENSE_1, LICENSE_2, LICENSE_3, LICENSE_4);
  exit(0);
}

#undef LICENSE_4
#undef LICENSE_3
#undef LICENSE_2
#undef LICENSE_1

static void print_version()
{
  static const char version[] =
#ifdef VN
    VN
#else
    PACKAGE_VERSION
#endif
    " for " CANONICAL_TARGET;
  printf("quvi %s\n  libquvi %s\n  libquvi-scripts %s\n",
         version, quvi_version(QUVI_VERSION_LONG),
         quvi_version(QUVI_VERSION_SCRIPTS));
  exit(0);
}

static void dump_host(char *domain, char *formats)
{
  printf("%s\t%s\n", domain, formats);
  quvi_free(domain);
  quvi_free(formats);
}

/* Wraps quvi_supported_ident. */
static void supported(quvi_t quvi)
{
  quvi_ident_t ident;
  unsigned int i;
  char *formats;
  QUVIcode rc;

  rc = QUVI_NOSUPPORT;

  for (i=0; i<opts->inputs_num; ++i)
    {
      rc = quvi_supported_ident(quvi, (char*)opts->inputs[i], &ident);
      if (rc == QUVI_OK)
        {
          quvi_ident_getprop(ident,
                             QUVI_IDENT_PROPERTY_FORMATS, &formats);
          spew("%10s : %s\n", formats, (char *)opts->inputs[i]);
          quvi_supported_ident_close(&ident);
        }
      else
        dump_error(quvi,rc);
    }

  exit(rc);
}

/* Query which formats are available for the URL */
static void query_formats(quvi_t quvi)
{
  unsigned int i;
  QUVIcode rc;

  if (opts->inputs_num <1)
    {
      spew_qe("error: no input URLs\n");
      exit (QUVI_INVARG);
    }

  for (i=0,rc=0; i<opts->inputs_num; ++i)
    {
      char *formats = NULL;

      rc = quvi_query_formats(quvi, (char*)opts->inputs[i], &formats);
      if (rc == QUVI_OK)
        {
          spew("%10s : %s\n", formats, opts->inputs[i]);
          quvi_free(formats);
        }
      else
        dump_error(quvi,rc);
    }

  exit(rc);
}

/* dumps all supported hosts to stdout. */
static void support(quvi_t quvi)
{
  int done = 0;

  if (opts->inputs_num >0)
    supported(quvi);

  while (done == 0)
    {
      char *domain, *formats;
      QUVIcode rc;

      rc = quvi_next_supported_website(quvi, &domain, &formats);

      switch (rc)
        {
        case QUVI_OK:
          dump_host(domain, formats);
          break;
        case QUVI_LAST:
          done = 1;
          break;
        default:
          spew_e("%s\n", quvi_strerror(quvi, rc));
          break;
        }
    }

  exit(0);
}

static void invoke_exec(quvi_media_t media)
{
  char *cmd, *media_url, *q_media_url;
  char *page_title, *q_page_title, *t;
  int rc;

  quvi_getprop(media, QUVIPROP_PAGETITLE, &page_title);
  t = strdup(page_title);
  t = strepl(t, "\"", "\\\""); /* Escape existing double quotation marks */
  asprintf(&q_page_title, "\"%s\"", t); /* Put inside quotation marks */
  _free(t);

  quvi_getprop(media, QUVIPROP_MEDIAURL, &media_url);
  asprintf(&q_media_url, "\"%s\"", media_url);

  cmd = strdup(opts->exec_arg);
  cmd = strepl(cmd, "%t", q_page_title);
  cmd = strepl(cmd, "%u", q_media_url);

  _free(q_page_title);
  _free(q_media_url);

  rc = system(cmd);

  switch (rc)
    {
    case 0:
      break;
    case -1:
      spew_e("error: failed to execute `%s'\n", cmd);
      break;
    default:
      spew_e("error: child exited with: %d\n", rc >> 8);
      break;
    }

  _free(cmd);
}

struct parsed_url_s
{
  char *media_url;
  char *content_type;
  double content_length;
  char *file_suffix;
};

typedef struct parsed_url_s *parsed_url_t;

static void dump_media_url_xml(parsed_url_t p, int i)
{
  char *media_url = curl_easy_escape(curl, p->media_url, 0);

  spew("   <link id=\"%d\">\n", i);

  if (p->content_length >0)
    spew("       <length_bytes>%.0f</length_bytes>\n", p->content_length);

  if (strlen(p->content_type) >0)
    spew("       <content_type>%s</content_type>\n", p->content_type);

  if (strlen(p->file_suffix) >0)
    spew("       <file_suffix>%s</file_suffix>\n", p->file_suffix);

  spew("       <url>%s</url>\n"
       "   </link>\n",
       (media_url != NULL)
       ? media_url
       : p->media_url);

  _free(media_url);
}

static void dump_media_url_json(parsed_url_t p, int i, int prepend_newln)
{
  if (prepend_newln != 0)
    spew(",\n");

  spew("    {\n"
       "      \"id\": \"%d\",\n", i);

  if (p->content_length >0)
    spew("      \"length_bytes\": \"%.0f\",\n", p->content_length);

  if (strlen(p->content_type) >0)
    spew("      \"content_type\": \"%s\",\n", p->content_type);

  if (strlen(p->file_suffix) >0)
    spew("      \"file_suffix\": \"%s\",\n", p->file_suffix);

  spew("      \"url\": \"%s\"\n"
       "    }",
       p->media_url);
}

static void dump_media_urls(quvi_media_t media)
{
  int json_flag=0, i=1;
  do
    {
      struct parsed_url_s p;

      memset(&p, 0, sizeof(&p));
      quvi_getprop(media, QUVIPROP_MEDIAURL, &p.media_url);
      quvi_getprop(media, QUVIPROP_MEDIACONTENTTYPE, &p.content_type);
      quvi_getprop(media, QUVIPROP_MEDIACONTENTLENGTH, &p.content_length);
      quvi_getprop(media, QUVIPROP_FILESUFFIX, &p.file_suffix);

      if (opts->xml_given == 1)
        dump_media_url_xml(&p,i);
      else
        {
          dump_media_url_json(&p, i, (int)(i>1));
          json_flag = 1;
        }
      ++i;
    }
  while (quvi_next_media_url(media) == QUVI_OK);

  if (json_flag == 1)
    spew("\n");
}

struct parsed_s
{
  char *page_url;
  char *page_title;
  char *media_id;
  char *format;
  char *host;
  char *start_time;
  char *thumb_url;
  double duration;
};

typedef struct parsed_s *parsed_t;

static void dump_media_xml(parsed_t p)
{
  char *e_page_url, *e_thumb_url;

  e_page_url = curl_easy_escape(curl, p->page_url, 0);
  e_thumb_url = curl_easy_escape(curl, p->thumb_url, 0);

  spew("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<media id=\"%s\" host=\"%s\">\n"
       "   <format_requested>%s</format_requested>\n"
       "   <page_title>%s</page_title>\n"
       "   <page_url>%s</page_url>\n",
       p->media_id,
       p->host,
       p->format,
       p->page_title,
       e_page_url ? e_page_url : "");

  if (strlen(p->start_time) >0)
    spew("   <start_time>%s</start_time>\n", p->start_time);

  if (e_thumb_url != NULL && strlen(e_thumb_url) >0)
    spew("   <thumbnail_url>%s</thumbnail_url>\n", e_thumb_url);

  if (p->duration >0)
    spew("   <duration>%.0f</duration>\n", p->duration);

  if (e_page_url != NULL)
    {
      curl_free(e_page_url);
      e_page_url = NULL;
    }

  if (e_thumb_url != NULL)
    {
      curl_free(e_thumb_url);
      e_thumb_url = NULL;
    }
}

static void dump_media_json(parsed_t p)
{
  char *t;

  t = strdup(p->page_title);
  t = strepl(t, "\"", "\\\"");

  spew("{\n"
       "  \"host\": \"%s\",\n"
       "  \"page_title\": \"%s\",\n"
       "  \"page_url\": \"%s\",\n"
       "  \"id\": \"%s\",\n"
       "  \"format_requested\": \"%s\",\n",
       p->host,
       t,
       p->page_url,
       p->media_id,
       p->format);

  if (strlen(p->start_time) >0)
    spew("  \"start_time\": \"%s\",\n", p->start_time);

  if (strlen(p->thumb_url) >0)
    spew("  \"thumbnail_url\": \"%s\",\n", p->thumb_url);

  if (p->duration >0)
    spew("  \"duration\": \"%.0f\",\n", p->duration);

  spew("  \"link\": [\n");

  _free(t);
}

static void dump_media(quvi_media_t media)
{
  struct parsed_s p;

  memset(&p, 0, sizeof(p));

  quvi_getprop(media, QUVIPROP_HOSTID, &p.host);
  quvi_getprop(media, QUVIPROP_PAGEURL, &p.page_url);
  quvi_getprop(media, QUVIPROP_PAGETITLE, &p.page_title);
  quvi_getprop(media, QUVIPROP_MEDIAID, &p.media_id);
  quvi_getprop(media, QUVIPROP_FORMAT, &p.format);
  quvi_getprop(media, QUVIPROP_STARTTIME, &p.start_time);
  quvi_getprop(media, QUVIPROP_MEDIATHUMBNAILURL, &p.thumb_url);
  quvi_getprop(media, QUVIPROP_MEDIADURATION, &p.duration);

  if (opts->xml_given == 1)
    dump_media_xml(&p);
  else
    dump_media_json(&p);

  dump_media_urls(media);

  if (opts->xml_given == 1)
    spew("</media>\n");
  else
    spew("  ]\n}\n");
}

static void dump_category_help()
{
  fprintf(stderr,
          "Usage:\n"
          "  quvi --category <value[,value,...]>\n"
          "Where the values may be:\n"
          "  http, rtmp, rtsp, mms, all\n"
          "Examples:\n"
          "  quvi --category all       ;# program default\n"
          "  quvi --category rtmp,mms  ;# multiple categories\n");
  exit (0);
}

static void check_category(const char *s, QUVIcategory *n)
{
  if (strlen(s) == 0)
    return;

  if (strcmp(s, "all") == 0)
    *n = QUVIPROTO_ALL;
  else if (strcmp(s, "help") == 0)
    dump_category_help();
  else
    {
      if (strcmp(s, "http") == 0)
        *n |= QUVIPROTO_HTTP;
      else if (strcmp(s, "rtmp") == 0)
        *n |= QUVIPROTO_RTMP;
      else if (strcmp(s, "rtsp") == 0)
        *n |= QUVIPROTO_RTSP;
      else if (strcmp(s, "mms") == 0)
        *n |= QUVIPROTO_MMS;
      else
        {
          fprintf(stderr, "warning: --category: %s: invalid value, "
                  "try \"help\"\n", s);
        }
    }
}

static QUVIcategory parse_categories()
{
  char b[4], *p=opts->category_arg;
  QUVIcategory n=0;
  size_t i;

  while (*p != '\0')
    {
      for (i=0; i<sizeof(b) && *p!='\0'; ++i,++p)
        {
          if (*p == ',')
            {
              ++p;
              break;
            }
          b[i] = *p;
        }
      b[i] = '\0';
      check_category(b, &n);
    }

  return (n);
}

static void depr_category(const char *o)
{
  fprintf(stderr, "warning: %s: deprecated, use --category instead\n", o);
}

static void init_quvi()
{
  QUVIcategory category;
  QUVIcode rc;

  if ((rc = quvi_init(&quvi)) != QUVI_OK)
    {
      dump_error(quvi, rc);
      exit(rc);
    }
  assert(quvi != NULL);

  /* Set quvi options. */

  quvi_setopt(quvi, QUVIOPT_FORMAT, opts->format_arg);
  quvi_setopt(quvi, QUVIOPT_NORESOLVE, opts->no_resolve_given);
  quvi_setopt(quvi, QUVIOPT_NOVERIFY, opts->no_verify_given);

  /* Category. */

  category = 0;

  if (opts->category_arg != NULL)
    category = parse_categories();

  /* Category: deprecated. To be removed in later versions. */

  if (opts->category_http_given == 1)
    {
      depr_category("--category-http");
      category |= QUVIPROTO_HTTP;
    }
  if (opts->category_rtmp_given == 1)
    {
      depr_category("--category-rtmp");
      category |= QUVIPROTO_RTMP;
    }
  if (opts->category_rtsp_given == 1)
    {
      depr_category("--category-rtsp");
      category |= QUVIPROTO_RTSP;
    }
  if (opts->category_mms_given == 1)
    {
      depr_category("--category-mms");
      category |= QUVIPROTO_MMS;
    }
  if (opts->category_all_given == 1)
    depr_category("--category-all");

  if (category == 0)
    category = QUVIPROTO_ALL;

  quvi_setopt(quvi, QUVIOPT_CATEGORY, category);

  /* Status callback */

  quvi_setopt(quvi, QUVIOPT_STATUSFUNCTION, status_callback);

  /* Use the quvi created cURL handle. */

  quvi_getinfo(quvi, QUVIINFO_CURL, &curl);
  assert(curl != NULL);

  curl_easy_setopt(curl, CURLOPT_USERAGENT, opts->agent_arg);
  curl_easy_setopt(curl, CURLOPT_PROXY, opts->proxy_arg);

  if (opts->no_proxy_given == 1)
    curl_easy_setopt(curl, CURLOPT_PROXY, "");

  curl_easy_setopt(curl, CURLOPT_VERBOSE, opts->verbose_libcurl_given);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, opts->connect_timeout_arg);
}

static void cleanup()
{
  if (input)
    quvi_llst_free(&input);

  if (quvi)
    quvi_close(&quvi);

  if (opts)
    {
      cmdline_parser_free(opts);
      _free(opts);
    }

  assert(input == NULL);
  assert(quvi == NULL);
  assert(opts == NULL);
}

static void read_from(FILE *f)
{
  char b[256];

  if (!f)
    return;

  while (fgets(b, (int)sizeof(b), f))
    {
      if (strlen(b) >16)
        {
          const size_t n = strlen(b)-1;

          if (b[n] == '\n')
            b[n] = '\0';

          quvi_llst_append(&input, strdup(b));
        }
    }
}

/*@null@*/ static char *parse_url_scheme(const char *url)
{
  char *p, *r;

  p = strstr(url, ":/");
  if (!p)
    return (NULL);

  asprintf(&r, "%.*s", (int)(p - url), url);

  return (r);
}

static int is_url(const char *s)
{
  char *p = parse_url_scheme(s);
  if (p)
    {
      _free(p);
      return (1);
    }
  return (0);
}

static void read_file(const char *path)
{
  FILE *f = fopen(path, "rt");
  if (f == NULL)
    {
#ifdef HAVE_STRERROR
      spew_e("error: %s: %s\n", path, strerror(errno));
#else
      perror("fopen");
#endif
    }
  else
    {
      read_from(f);
      fclose(f);
      f = NULL;
    }
}

static unsigned int read_input()
{
  if (opts->inputs_num == 0)
    read_from(stdin);
  else
    {
      unsigned int i;
      for (i=0; i<opts->inputs_num; ++i)
        {
          if (is_url(opts->inputs[i]) == 0)
            read_file(opts->inputs[i]);
          else /* Must be an URL. */
            quvi_llst_append(&input, strdup(opts->inputs[i]));
        }
    }
  return ((unsigned int)quvi_llst_size(input));
}

static int config_exists(const char *fpath)
{
  FILE *f = fopen(fpath, "r");
  int have_config = 0;
  if (f != NULL)
    {
      have_config = 1;
      fclose(f);
      f = NULL;
    }
  return (have_config);
}

static int parse_config(int argc, char **argv, char *home)
{
  struct cmdline_parser_params *pp = NULL;
  int have_config = 0;
  char *fpath = NULL;

  asprintf(&fpath, "%s/%s", home, config_fname);
  if (fpath == NULL)
    exit (QUVI_MEM);

  have_config = config_exists(fpath);
  if (have_config == 0)
    {
      _free(fpath);
      return (have_config);
    }

  pp = cmdline_parser_params_create();
  pp->check_required = 0;

  have_config = (int)(cmdline_parser_config_file(fpath, opts, pp) == 0);
  if (have_config == 1)
    {
      _free(fpath); /* cmdline_parser_ext may exit early. */

      /* Config parsed at this point. Next, apply the cmdline options
       * and allow overriding the values defined in the config.
       *
       * This step resets the $opt_given flags, e.g. if the config
       * contains the "proxy", the parser sets proxy_given to 1,
       * --proxy is used to override the config value, the flag
       * is reset to 0. */
      pp->check_required = 1;
      pp->initialize = 0;
      pp->override = 1;
      have_config = (int)(cmdline_parser_ext(argc, argv, opts, pp) == 0);
    }

  _free(fpath);
  _free(pp);

  return (have_config);
}

static void parse_cmdline(int argc, char **argv)
{
  int have_config = 0;
  char *home = NULL;

  opts = calloc(1, sizeof(struct gengetopt_args_info));
  if (opts == NULL)
    exit (QUVI_MEM);

  home = getenv("QUVI_HOME");
  if (home == NULL)
    home = getenv("HOME");

  if (home != NULL && getenv("QUVI_NO_CONFIG") == NULL)
    have_config = parse_config(argc, argv, home);

  if (have_config == 0)
    {
      if (cmdline_parser(argc, argv, opts) != 0)
        exit (QUVI_INVARG);
    }
}

int main(int argc, char *argv[])
{
  QUVIcode rc, last_error;
  unsigned int input_num;
  quvi_llst_node_t curr;
  quvi_media_t media;
  int i, errors;

  assert(quvi == NULL);
  assert(curl == NULL);
  assert(opts == NULL);
  assert(input == NULL);

  atexit(cleanup);
  parse_cmdline(argc, argv);

  if (opts->version_given == 1)
    print_version();

  if (opts->license_given == 1)
    license();

  verbose_flag = (int)(opts->quiet_given == 0);
  init_quvi();

  if (opts->query_formats_given == 1)
    query_formats(quvi);

  if (opts->support_given == 1)
    support(quvi);

  /* User input */

  input_num = read_input();

  if (input_num == 0)
    {
      spew_qe("error: no input URLs\n");
      return (QUVI_INVARG);
    }

  last_error = QUVI_OK;
  errors = 0;

  for (i=0, curr=input; curr; ++i)
    {
      char *url = quvi_llst_data(curr);
      rc = quvi_parse(quvi, url, &media);
      if (rc == QUVI_OK)
        {
          assert(media != NULL);
          dump_media(media);

          if (opts->exec_arg != NULL)
            {
              do invoke_exec(media);
              while (quvi_next_media_url(media) == QUVI_OK);
            }
        }
      else
        {
          dump_error(quvi, rc);
          last_error = rc;
          ++errors;
        }
      quvi_parse_close(&media);
      assert(media == NULL);
      curr = quvi_llst_next(curr);
    }

  if (input_num >1)
    {
      spew_qe("Results: %d OK, %d failed (last 0x%02x), exit with 0x%02x\n",
              input_num - errors, errors, last_error, rc);
    }

  return (rc);
}

/* vim: set ts=2 sw=2 tw=72 expandtab: */
