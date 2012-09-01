#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])

AC_INIT([quvi], m4_esyscmd([./gen-ver.sh -c | tr -d '\n']),
        [http://quvi.sf.net/bugs/],[],[http://quvi.sf.net/])

AC_CONFIG_SRCDIR([src/quvi/quvi.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config.aux])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_TARGET
AC_USE_SYSTEM_EXTENSIONS

AC_DEFINE_UNQUOTED([CANONICAL_TARGET], "$target", [...])
AC_DEFINE_UNQUOTED([CANONICAL_HOST], "$host", [...])

AM_INIT_AUTOMAKE([1.11.1 -Wall -Werror dist-xz no-dist-gzip tar-ustar])
AM_SILENT_RULES([yes])
#AM_PROG_AR

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
PKG_CHECK_MODULES([libquvi], [libquvi-0.9 >= 0.9])
PKG_CHECK_MODULES([libcurl], [libcurl >= 7.18.2])
PKG_CHECK_MODULES([gobject], [gobject-2.0 >= 2.24])
PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.24])

PKG_CHECK_MODULES([json_glib], [json-glib-1.0 >= 0.12],
  [have_json_glib=yes
   AC_DEFINE([HAVE_JSON_GLIB], [1], [Define to json-glib package])
  ],
  [have_json_glib=no
   AC_MSG_NOTICE([json-glib 0.12+ not found, building without json output])
  ])
AM_CONDITIONAL([HAVE_JSON_GLIB], [test x"$have_json_glib" = "xyes"])
 
PKG_CHECK_MODULES([libxml], [libxml-2.0 >= 2.7.8],
  [have_libxml=yes
   AC_DEFINE([HAVE_LIBXML], [1], [Define to libxml package])
  ],
  [have_libxml=no
   AC_MSG_NOTICE([libxml 2.7.8+ not found, building without xml output])
  ])
AM_CONDITIONAL([HAVE_LIBXML], [test x"$have_libxml" = "xyes"])

# Checks for header files.
AC_CHECK_HEADERS([stdarg.h stdlib.h string.h stddef.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([setlocale])

# Version.
VN=`$srcdir/gen-ver.sh`
AC_DEFINE_UNQUOTED([VN],["$VN"], [We have version number from gen-ver.sh])

# --with-manual
AC_ARG_WITH([manual],
  [AS_HELP_STRING([--with-manual],
    [install manual page(s) @<:@default=yes@:>@])],
  [],
  [with_manual=yes])
AM_CONDITIONAL([WITH_MANUAL], [test x"$with_manual" != "xno"])

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  doc/man1/Makefile
  src/Makefile
  src/quvi/Makefile])
AC_OUTPUT

AC_MSG_NOTICE([
  version:        ${VERSION}
  prefix:         ${prefix}
  compiler:       ${CC}
  cflags:         ${CFLAGS}
System types
  target          ${target}
  build           ${build}
  host            ${host}
Build options
  json-glib 0.12+ ${have_json_glib}
  libxml 2.7.8+   ${have_libxml}
Install options
  with
  - manual  ${with_manual}])
