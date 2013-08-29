#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

AC_INIT([quvi], m4_esyscmd([./gen-ver.sh -c | tr -d '\n']),
        [http://quvi.sf.net/bugs/],[],[http://quvi.sf.net/])

AC_DEFINE_UNQUOTED([BUILD_OPTS], "$@",
  [Define to configure invocation command line options])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config.aux])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_TARGET
AC_USE_SYSTEM_EXTENSIONS

AC_DEFINE_UNQUOTED([CANONICAL_TARGET], "$target", [...])
AC_DEFINE_UNQUOTED([CANONICAL_HOST], "$host", [...])

# GNU Automake 1.13 spews a warning about AM_GNU_GETTEXT (0.18.1,
# 0.18.2) using the deprecated AM_PROG_MKDIR_P macro. Do not specify
# '-Werror` in the options to work around this.
AM_INIT_AUTOMAKE([1.11.1 -Wall dist-xz no-dist-gzip tar-ustar])
AM_SILENT_RULES([yes])

# GNU Automake 1.12 requires this macro. Earlier versions do not
# recognize this macro. Work around this.
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT([disable-static])
LT_PREREQ([2.2.6])

AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external])
# GETTEXT_PACKAGE is used by glib.
AC_DEFINE([GETTEXT_PACKAGE], [PACKAGE], [Define to the gettext package name])
AC_SUBST([GETTEXT_PACKAGE], [PACKAGE])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

AC_DEFINE_UNQUOTED([CFLAGS], "$CFLAGS", [Define to compiler flags])
AC_DEFINE_UNQUOTED([CC], "$CC", [Define to compiler])

AC_PATH_PROG([DATE], [date], [no])
AS_IF([test x"$DATE" != "xno"], [build_time=`$DATE +"%F %T %z"`])
AC_DEFINE_UNQUOTED([BUILD_TIME], ["$build_time"], [We have build time])

AC_PATH_PROG([A2X], [a2x], [no])
AM_CONDITIONAL([HAVE_A2X], [test x"$A2X" != "xno"])
AC_SUBST([A2X])

AS_IF([test x"$A2X" = "xno" && test -d "$srcdir/.git"],
  AC_MSG_ERROR([a2x is required to create man pages when building from git])])

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
AC_CHECK_HEADERS([locale.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([setlocale memset strerror])
AC_FUNC_STRERROR_R

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
  doc/man5/Makefile
  src/get/Makefile
  src/input/Makefile
  src/opts/Makefile
  src/pbar/Makefile
  src/print/Makefile
  src/util/Makefile
  src/Makefile
  po/Makefile.in
])
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

# vim: set ts=2 sw=2 tw=72 expandtab:
