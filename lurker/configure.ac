AC_INIT(lurker.conf.in)
AC_CONFIG_AUX_DIR(tools)
AC_PREREQ(2.57)
AM_INIT_AUTOMAKE(lurker, 1.2)

AC_SUBST(VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_PROG_INSTALL
AC_PROG_RANLIB

AC_PROG_CC
AC_PROG_CXX

AC_LANG(C++)

if test "x$USE_MAINTAINER_MODE" != "xno"; then
  CXXFLAGS="$CXXFLAGS -Wall -O2 -g"
fi

AC_SEARCH_HEADER_DIRS(
  [iconv.h],
  [/usr/local/include],,
  [AC_MSG_ERROR(Need iconv.h to compile)])

AC_SEARCH_EXPRESSION_LIBS([iconv_t ic = iconv_open("ISO-8859-1", "UTF-8")],
  [iconv], [#include <iconv.h>],,
  [AC_MSG_ERROR(Need iconv to link)])

AC_ARG_WITH([zlib-include], AC_HELP_STRING([--with-zlib-include=DIR],
                                           [Location of zlib.h]),,
            [with_zlib_include=""])

AC_SEARCH_HEADER_DIRS(
  [zlib.h], 
  [$with_zlib_include /usr/include /usr/local/include],, 
  [AC_MSG_ERROR(Need zlib.h to compile)])

AC_ARG_WITH([zlib-libname], AC_HELP_STRING([--with-zlib-libname=LIB],
                                              [Try an alternative library name]),,
            [with_zlib_libname=""])

AC_SEARCH_EXPRESSION_LIBS([gzFile f = gzdopen(0, "r")], 
  [$with_zlib_libname zlib z libz], [#include <zlib.h>],,
  [AC_MSG_ERROR(Need zlib to link)])

AC_ARG_WITH([mimelib-local],   AC_HELP_STRING([--with-mimelib-local],
                                              [Use internal lurker/mimelib/*]),,
            [with_mimelib_local="no"])

AC_ARG_WITH([mimelib-include], AC_HELP_STRING([--with-mimelib-include=DIR],
                                              [Location of mimelib/message.h]),,
            [with_mimelib_include=""])

if test "x$with_mimelib_local" != "xno"; then
 AM_CONDITIONAL(LOCAL_MIMELIB, true)
 AC_MSG_CHECKING(for mimelib)
 AC_MSG_RESULT(local)
 CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/mimelib"
 LIBS="-L\$(top_srcdir)/mimelib -lmimelib $LIBS"
else
 AM_CONDITIONAL(LOCAL_MIMELIB, false)
 AC_SEARCH_HEADER_DIRS(
   [mimelib/message.h], 
   [$with_mimelib_include /usr/include/kde /usr/local/include /usr/local/include/kde],, 
   [AC_MSG_ERROR(Need mimelib/*.h to compile)])
 
 AC_ARG_WITH([mimelib-libname], AC_HELP_STRING([--with-mimelib-libname=LIB],
                                               [Try an alternative library name]),,
             [with_mimelib_libname=""])
 
 AC_SEARCH_CLASS_LIBS(DwMessage, 
   [$with_mimelib_libname mimelib mimepp], [#include <mimelib/message.h>],,
   [AC_MSG_ERROR(Need mimelib to link)])
fi

wwwdir="${localstatedir}/www/${PACKAGE}"

prefix_backup="$prefix"
exec_prefix_backup="$exec_prefix"
if test "x${prefix}" = "xNONE"; then prefix="${ac_default_prefix}"; fi
if test "x${exec_prefix}" = "xNONE"; then exec_prefix="${prefix}"; fi
vardir=`eval echo ${localstatedir}/lib/${PACKAGE}`
etcdir=`eval echo ${sysconfdir}`
prefix="$prefix_backup"
exec_prefix="$exec_prefix_backup"

AC_SUBST(wwwdir)
AC_SUBST(vardir)
AC_SUBST(etcdir)

AC_CONFIG_FILES([
	Makefile libesort/Makefile common/Makefile index/Makefile
	ui/Makefile imgs/Makefile prune/Makefile render/Makefile
	lurker.conf])
AC_OUTPUT
