# linput - input parsing (convenience library)
#
# See lopts for cmdline and config file parsing; this library collects
# the URLs from the input.

src=linput.c
hdr=linput.h

noinst_LTLIBRARIES=libinput.la
libinput_la_SOURCES=$(src) $(hdr)

libinput_la_CPPFLAGS=\
  -DLOCALEDIR=\""$(localedir)"\"\
  -I$(top_srcdir)/src/util/\
  -I$(top_srcdir)/src/input/\
  -I$(top_srcdir)/src/\
  $(glib_CFLAGS)\
  $(AM_CPPFLAGS)

libinput_la_LDFLAGS=\
  $(AM_LDFLAGS)

libinput_la_LIBADD=\
  $(glib_LIBS)

# vim: set ts=2 sw=2 tw=72 expandtab:
