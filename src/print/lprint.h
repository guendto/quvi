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

#ifndef lprint_h
#define lprint_h

/* callbacks */

typedef void (*lprint_cb_errmsg)(const gchar*, ...);

/* playlist */

typedef gint (*lprint_cb_playlist_properties)(quvi_playlist_t, gpointer);
typedef gint (*lprint_cb_playlist_print_buffer)(gpointer);

typedef gint (*lprint_cb_playlist_new)(quvi_t, gpointer*);
typedef void (*lprint_cb_playlist_free)(gpointer);

/* media */

typedef gint (*lprint_cb_media_stream_properties)(quvi_http_metainfo_t, gpointer);
typedef gint (*lprint_cb_media_print_buffer)(gpointer);
typedef gint (*lprint_cb_media_properties)(gpointer);

typedef gint (*lprint_cb_media_new)(quvi_t, quvi_media_t, gpointer*);
typedef void (*lprint_cb_media_free)(gpointer);

typedef gint (*lprint_cb_media_streams_available)(quvi_t, quvi_media_t);

/* scan */

typedef gint (*lprint_cb_scan_properties)(quvi_scan_t, gpointer);
typedef gint (*lprint_cb_scan_print_buffer)(gpointer);

typedef gint (*lprint_cb_scan_new)(quvi_t, gpointer*);
typedef void (*lprint_cb_scan_free)(gpointer);

/* subtitle */

typedef gint (*lprint_cb_subtitle_lang_properties)(quvi_subtitle_lang_t, gpointer);
typedef gint (*lprint_cb_subtitle_print_buffer)(gpointer);

typedef gint (*lprint_cb_subtitle_new)(quvi_t, gpointer*);
typedef void (*lprint_cb_subtitle_free)(gpointer);

typedef gint (*lprint_cb_subtitles_available)(quvi_t, quvi_subtitle_t);

/* enum */

void lprint_enum_errmsg(const gchar*, ...);

  /* playlist */
gint lprint_enum_playlist_new(quvi_t, gpointer*);
void lprint_enum_playlist_free(gpointer);

gint lprint_enum_playlist_properties(quvi_playlist_t, gpointer);
gint lprint_enum_playlist_print_buffer(gpointer);

  /* media */
gint lprint_enum_media_stream_properties(quvi_http_metainfo_t, gpointer);
gint lprint_enum_media_print_buffer(gpointer);
gint lprint_enum_media_properties(gpointer);

gint lprint_enum_media_new(quvi_t, quvi_media_t, gpointer*);
void lprint_enum_media_free(gpointer);

gint lprint_enum_media_streams_available(quvi_t, quvi_media_t);

  /* scan */
gint lprint_enum_scan_properties(quvi_scan_t, gpointer);
gint lprint_enum_scan_print_buffer(gpointer);

gint lprint_enum_scan_new(quvi_t, gpointer*);
void lprint_enum_scan_free(gpointer);

  /* subtitle */
gint lprint_enum_subtitle_lang_properties(quvi_subtitle_lang_t, gpointer);
gint lprint_enum_subtitle_print_buffer(gpointer);

gint lprint_enum_subtitle_new(quvi_t, gpointer*);
void lprint_enum_subtitle_free(gpointer);

gint lprint_enum_subtitles_available(quvi_t, quvi_subtitle_t);

/* json */

void lprint_json_errmsg(const gchar*, ...);

  /* playlist */
gint lprint_json_playlist_new(quvi_t, gpointer*);
void lprint_json_playlist_free(gpointer);

gint lprint_json_playlist_properties(quvi_playlist_t, gpointer);
gint lprint_json_playlist_print_buffer(gpointer);

  /* media */
gint lprint_json_media_stream_properties(quvi_http_metainfo_t, gpointer);
gint lprint_json_media_print_buffer(gpointer);
gint lprint_json_media_properties(gpointer);

gint lprint_json_media_new(quvi_t, quvi_media_t, gpointer*);
void lprint_json_media_free(gpointer);

gint lprint_json_media_streams_available(quvi_t, quvi_media_t);

  /* scan */
gint lprint_json_scan_properties(quvi_scan_t, gpointer);
gint lprint_json_scan_print_buffer(gpointer);

gint lprint_json_scan_new(quvi_t, gpointer*);
void lprint_json_scan_free(gpointer);

  /* subtitle */
gint lprint_json_subtitle_lang_properties(quvi_subtitle_lang_t, gpointer);
gint lprint_json_subtitle_print_buffer(gpointer);

gint lprint_json_subtitle_new(quvi_t, gpointer*);
void lprint_json_subtitle_free(gpointer);

gint lprint_json_subtitles_available(quvi_t, quvi_subtitle_t);

/* xml */

void lprint_xml_errmsg(const gchar*, ...);

  /* playlist */
gint lprint_xml_playlist_new(quvi_t, gpointer*);
void lprint_xml_playlist_free(gpointer);

gint lprint_xml_playlist_properties(quvi_playlist_t, gpointer);
gint lprint_xml_playlist_print_buffer(gpointer);

  /* media */
gint lprint_xml_media_stream_properties(quvi_http_metainfo_t, gpointer);
gint lprint_xml_media_print_buffer(gpointer);
gint lprint_xml_media_properties(gpointer);

gint lprint_xml_media_new(quvi_t, quvi_media_t, gpointer*);
void lprint_xml_media_free(gpointer);

gint lprint_xml_media_streams_available(quvi_t, quvi_media_t);

  /* scan */
gint lprint_xml_scan_properties(quvi_scan_t, gpointer);
gint lprint_xml_scan_print_buffer(gpointer);

gint lprint_xml_scan_new(quvi_t, gpointer*);
void lprint_xml_scan_free(gpointer);

  /* subtitle */
gint lprint_xml_subtitle_lang_properties(quvi_subtitle_lang_t, gpointer);
gint lprint_xml_subtitle_print_buffer(gpointer);

gint lprint_xml_subtitle_new(quvi_t, gpointer*);
void lprint_xml_subtitle_free(gpointer);

gint lprint_xml_subtitles_available(quvi_t, quvi_subtitle_t);

/* rfc2483: reuses lprint_enum_errmsg */

  /* playlist */
gint lprint_rfc2483_playlist_new(quvi_t, gpointer*);
void lprint_rfc2483_playlist_free(gpointer);

gint lprint_rfc2483_playlist_properties(quvi_playlist_t, gpointer);
gint lprint_rfc2483_playlist_print_buffer(gpointer);

  /* media */
gint lprint_rfc2483_media_stream_properties(quvi_http_metainfo_t, gpointer);
gint lprint_rfc2483_media_print_buffer(gpointer);
gint lprint_rfc2483_media_properties(gpointer);

gint lprint_rfc2483_media_new(quvi_t, quvi_media_t, gpointer*);
void lprint_rfc2483_media_free(gpointer);

gint lprint_rfc2483_media_streams_available(quvi_t, quvi_media_t);

  /* scan */
gint lprint_rfc2483_scan_properties(quvi_scan_t, gpointer);
gint lprint_rfc2483_scan_print_buffer(gpointer);

gint lprint_rfc2483_scan_new(quvi_t, gpointer*);
void lprint_rfc2483_scan_free(gpointer);

  /* subtitle */
gint lprint_rfc2483_subtitle_lang_properties(quvi_subtitle_lang_t, gpointer);
gint lprint_rfc2483_subtitle_print_buffer(gpointer);

gint lprint_rfc2483_subtitle_new(quvi_t, gpointer*);
void lprint_rfc2483_subtitle_free(gpointer);

gint lprint_rfc2483_subtitles_available(quvi_t, quvi_subtitle_t);

#endif /* lprint_h */

/* vim: set ts=2 sw=2 tw=72 expandtab: */
