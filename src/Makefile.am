SUBDIRS=

src=\
  main.c

hdr=

bin_PROGRAMS=quvi
quvi_SOURCES=$(src) $(hdr)

quvi_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
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
  $(libquvi_LIBS)\
  $(libcurl_LIBS)\
  $(gobject_LIBS)\
  $(glib_LIBS)\
  $(LIBINTL)

# vim: set ts=2 sw=2 tw=72 expandtab:
