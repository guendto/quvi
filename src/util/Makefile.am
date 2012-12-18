# lutil - misc. utility functions (convenience library)

src=\
  chk.c\
  choose.c\
  curl.c\
  exec.c\
  file.c\
  fpath.c\
  input.c\
  metainfo.c\
  query.c\
  quvi.c\
  regex.c\
  slist.c\
  strerr.c\
  strv.c\
  support.c\
  verbosity.c\
  xchg.c

hdr=lutil.h

noinst_LTLIBRARIES=libutil.la
libutil_la_SOURCES=$(src) $(hdr)

libutil_la_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
  -I$(top_srcdir)/src/input/\
  -I$(top_srcdir)/src/util/\
  -I$(top_srcdir)/src/\
  $(libquvi_CFLAGS)\
  $(glib_CFLAGS)\
  $(AM_CPPFLAGS)

libutil_la_LDFLAGS=\
  $(AM_LDFLAGS)

libutil_la_LIBADD=\
  $(libquvi_LIBS)\
  $(glib_LIBS)

# vim: set ts=2 sw=2 tw=72 expandtab:
