SUBDIRS=get input pbar print opts util

src=\
  builtin/dump.c\
  builtin/get.c\
  builtin/info.c\
  builtin/scan.c\
  main.c\
  opts.c\
  setup.c\
  sig.c\
  status.c

hdr=\
  cmd.h\
  opts.h\
  setup.h\
  sig.h

bin_PROGRAMS=quvi
quvi_SOURCES=$(src) $(hdr)

quvi_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
  -I$(top_srcdir)/src/get/\
  -I$(top_srcdir)/src/input/\
  -I$(top_srcdir)/src/opts/\
  -I$(top_srcdir)/src/pbar/\
  -I$(top_srcdir)/src/print/\
  -I$(top_srcdir)/src/util/\
  -I$(top_srcdir)/src/\
  $(libquvi_CFLAGS)\
  $(libcurl_CFLAGS)\
  $(gobject_CFLAGS)\
  $(glib_CFLAGS)\
  $(AM_CPPFLAGS)

quvi_CFLAGS=\
  $(AM_CFLAGS)

quvi_LDFLAGS=\
  $(AM_LDFLAGS)

quvi_LDADD=\
  $(top_builddir)/src/get/libget.la\
  $(top_builddir)/src/input/libinput.la\
  $(top_builddir)/src/opts/libopts.la\
  $(top_builddir)/src/pbar/libpbar.la\
  $(top_builddir)/src/print/libprint.la\
  $(top_builddir)/src/util/libutil.la\
  $(libquvi_LIBS)\
  $(libcurl_LIBS)\
  $(gobject_LIBS)\
  $(glib_LIBS)\
  $(LIBINTL)

# vim: set ts=2 sw=2 tw=72 expandtab:
