# lget - copy (download) media stream to a file (convenience library)

src=\
  http.c\
  lget.c

hdr=lget.h

noinst_LTLIBRARIES=libget.la
libget_la_SOURCES=$(src) $(hdr)

libget_la_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
  -I$(top_srcdir)/src/get/\
  -I$(top_srcdir)/src/pbar/\
  -I$(top_srcdir)/src/util/\
  -I$(top_srcdir)/src/\
  $(libquvi_CFLAGS)\
  $(libcurl_CFLAGS)\
  $(glib_CFLAGS)\
  $(AM_CPPFLAGS)

libget_la_LDFLAGS=\
  $(AM_LDFLAGS)

libget_la_LIBADD=\
  $(glib_LIBS)

# vim: set ts=2 sw=2 tw=72 expandtab:
