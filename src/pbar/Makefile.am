# lpbar - progressbar (convenience library)

src=lpbar.c
hdr=lpbar.h

noinst_LTLIBRARIES=libpbar.la
libpbar_la_SOURCES=$(src) $(hdr)

libpbar_la_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
  -I$(top_srcdir)/src/pbar/\
  -I$(top_srcdir)/src/\
  $(glib_CFLAGS)\
  $(AM_CPPFLAGS)

libpbar_la_LDFLAGS=\
  $(AM_LDFLAGS)

libpbar_la_LIBADD=\
  $(glib_LIBS)

# vim: set ts=2 sw=2 tw=72 expandtab:
