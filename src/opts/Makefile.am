# lopts - cmdline option and config file parsing (convenience library)

src=\
  chk.c\
  config.c\
  err.c\
  get.c\
  lopts.c\
  print.c

hdr=lopts.h

noinst_LTLIBRARIES=libopts.la
libopts_la_SOURCES=$(src) $(hdr)

libopts_la_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
  -I$(top_srcdir)/src/util/\
  -I$(top_srcdir)/src/opts/\
  -I$(top_srcdir)/src/\
  $(libquvi_CFLAGS)\
  $(glib_CFLAGS)\
  $(AM_CPPFLAGS)

libopts_la_LDFLAGS=\
  $(AM_LDFLAGS)

libopts_la_LIBADD=\
  $(libquvi_LIBS)\
  $(glib_LIBS)

# vim: set ts=2 sw=2 tw=72 expandtab:
