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

# Checks for header files.
AC_CHECK_HEADERS([stdarg.h stdlib.h string.h stddef.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_TYPES([ptrdiff_t])
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strchr strcspn strdup strerror strspn strstr strtol])
AC_FUNC_REALLOC
AC_FUNC_MALLOC

have_asprintf=no
AC_CHECK_FUNCS([vasprintf], [
  AC_CHECK_FUNCS([asprintf], [have_asprintf=yes])
])
AM_CONDITIONAL([NO_ASPRINTF], [test x"$have_asprintf" != "xyes"])

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
Options
  install manual: ${with_manual}])
