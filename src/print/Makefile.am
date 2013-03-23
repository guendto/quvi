# lprint - print properties (convenience library)

src=\
  enum_print.c\
  rfc2483_print.c

if HAVE_JSON_GLIB
src+=json_print.c
endif
if HAVE_LIBXML
src+=xml_print.c
endif

hdr=lprint.h

noinst_LTLIBRARIES=libprint.la
libprint_la_SOURCES=$(src) $(hdr)

libprint_la_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
  -I$(top_srcdir)/src/print/\
  -I$(top_srcdir)/src/util/\
  -I$(top_srcdir)/src/\
  $(json_glib_CFLAGS)\
  $(libquvi_CFLAGS)\
  $(libxml_CFLAGS)\
  $(glib_CFLAGS)\
  $(AM_CPPFLAGS)

libprint_la_LDFLAGS=\
  $(AM_LDFLAGS)

libprint_la_LIBADD=\
  $(json_glib_LIBS)\
  $(libquvi_LIBS)\
  $(libxml_LIBS)\
  $(glib_LIBS)

# vim: set ts=2 sw=2 tw=72 expandtab:
