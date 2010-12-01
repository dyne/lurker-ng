AC_INIT(lurker.conf.in)
AC_CONFIG_AUX_DIR(tools)
AC_PREREQ(2.57)
AM_INIT_AUTOMAKE(lurker, 2.4)

AC_SUBST(VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_PROG_INSTALL
AC_PROG_RANLIB

AC_PROG_CC
AC_PROG_CXX

AC_LANG(C++)

if test "x$USE_MAINTAINER_MODE" != "xno"; then
  CXXFLAGS="$CXXFLAGS -Wall -O0 -g"
fi

AC_CHECK_FUNCS([timegm realpath])
AC_CHECK_HEADERS([sysexits.h sys/param.h])

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
 LIBS="-L\$(top_srcdir)/mymime -lmimelib $LIBS"
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

AC_ARG_WITH([default-config-dir], 
            AC_HELP_STRING([--with-default-config-dir=DIR],
                           [Where the lurker looks for a configuration files [default=SYSCONFDIR/lurker]]), 
            default_config_dir="$withval", 
            default_config_dir="\${sysconfdir}/\${PACKAGE}") 
AC_ARG_WITH([default-www-dir],
            AC_HELP_STRING([--with-default-www-dir=DIR],
                           [Where the lurker places bootstrap web files [default=DATAROOTDIR/lurker/www]]), 
            default_www_dir="$withval", 
            default_www_dir="\${datarootdir}/\${PACKAGE}/www") 
AC_ARG_WITH([cgi-bin-dir],
            AC_HELP_STRING([--with-cgi-bin-dir=DIR],
                           [Where the lurker places cgi files [default=LIBDIR/cgi-bin]]), 
            cgi_bin_dir="$withval", 
            cgi_bin_dir="\${libdir}/cgi-bin") 
AC_SUBST(default_config_dir)
AC_SUBST(default_www_dir)
AC_SUBST(cgi_bin_dir)

# We need to fully evaluate some variables for the config and source files
prefix_backup="$prefix"
exec_prefix_backup="$exec_prefix"
if test "x${prefix}" = "xNONE"; then prefix="${ac_default_prefix}"; fi
if test "x${exec_prefix}" = "xNONE"; then exec_prefix="${prefix}"; fi
LOCALSTATEDIR=`eval echo ${localstatedir}`
DEFAULT_WWW_DIR=`eval echo ${default_www_dir}`
DEFAULT_CONFIG_DIR=`eval echo ${default_config_dir}`
CGI_BIN_DIR=`eval echo ${cgi_bin_dir}`
BINDIR=`eval echo ${bindir}`
LOCALSTATEDIR=`eval echo ${LOCALSTATEDIR}`
DEFAULT_WWW_DIR=`eval echo ${DEFAULT_WWW_DIR}`
DEFAULT_CONFIG_DIR=`eval echo ${DEFAULT_CONFIG_DIR}`
CGI_BIN_DIR=`eval echo ${CGI_BIN_DIR}`
BINDIR=`eval echo ${BINDIR}`
prefix="$prefix_backup"
exec_prefix="$exec_prefix_backup"

# Needed in lurker.conf and apache.conf:
AC_SUBST(LOCALSTATEDIR)
AC_SUBST(DEFAULT_CONFIG_DIR) 
AC_SUBST(DEFAULT_WWW_DIR)
AC_SUBST(CGI_BIN_DIR)
AC_SUBST(BINDIR)

# Needed in source:
AC_DEFINE_UNQUOTED(DEFAULT_CONFIG_FILE, 
                   ["$DEFAULT_CONFIG_DIR/$PACKAGE.conf"], 
                   [The default config file location])

AC_CONFIG_FILES([
	Makefile libesort/Makefile common/Makefile index/Makefile
	ui/Makefile imgs/Makefile prune/Makefile render/Makefile
	mymime/Makefile lurker.conf apache.conf])
AC_OUTPUT
