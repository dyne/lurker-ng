AC_INIT(lurker, 0.1b)

AC_PREREQ(2.52)
AC_REVISION($Id: configure.ac,v 1.28 2002-07-11 13:27:16 terpstra Exp $)
AC_CONFIG_SRCDIR(store/main.c)
AC_CONFIG_AUX_DIR(tools)

AM_INIT_AUTOMAKE(lurker, 0.1b)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

# Which languages are supported
ALL_LINGUAS=""


AC_ARG_WITH(wwwdir, AC_HELP_STRING(
	[--with-wwwdir], 
	[www directory (default: PREFIX/var/www/lurker)]))
if test "x$with_wwwdir" = "x"; then
  wwwdir="\${localstatedir}/www/\${PACKAGE}"
else
  wwwdir=$with_wwwdir
fi
AC_SUBST(wwwdir)

AC_ARG_WITH(urldir, AC_HELP_STRING(
	[--with-url], 
	[url location (default: http://this.host/lurker)]))
if test "x$with_urldir" = "x"; then
  urldir="/${PACKAGE}"
else
  urldir=$with_urldir
fi
AC_SUBST(urldir)

AC_ARG_WITH(localedir, AC_HELP_STRING(
	[--with-localedir], 
	[locale directory (default: DATADIR/locale)]))
if test "x$with_localedir" = "x"; then
  localedir="\${datadir}/locale"
else
  localedir=$with_localedir
fi
AC_SUBST(localedir)

AC_ARG_WITH(dbdir, AC_HELP_STRING(
	[--with-dbdir], 
	[db directory (default: PREFIX/var/lib/lurker)]))
if test "x$with_dbdir" = "x"; then
  dbdir="\${localstatedir}/lib/\${PACKAGE}"
else
  dbdir=$with_dbdir
fi
AC_SUBST(dbdir)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_RANLIB

AC_HEADER_STDC
AC_HEADER_DIRENT

AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_CHAR_UNSIGNED

AC_HEADER_TIME

AC_FUNC_ALLOCA
AC_FUNC_MMAP

ST_CHECK(STORAGE)
DB3_CHECK(STORAGE)
C_CLIENT_CHECK(STORAGE)
XSLT_CHECK(RENDER)

AC_CHECK_HEADERS(unistd.h signal.h fcntl.h io.h time.h sys/mman.h sys/time.h sys/file.h assert.h sys/fcntl.h errno.h syslog.h)
AC_CHECK_FUNCS(setsid fork flock fcntl lockf)

AC_MSG_CHECKING(for F_GETLK)
AC_TRY_COMPILE(
[#include <unistd.h>
#include <fcntl.h>],
[	fnctl(0, F_GETLK, 0);
	return 0;],
[AC_MSG_RESULT(yes)
 AC_DEFINE(HAVE_F_GETLK, 1, [Do we have getlk fnctl?])],
[AC_MSG_RESULT(no)])

AC_SYS_LARGEFILE
AM_GNU_GETTEXT
AM_ICONV

if test "x$am_cv_func_iconv" != "xyes"; then
  AC_MSG_ERROR(Need iconv to function)
fi

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

AC_CHECK_TYPE(time_t,
 [AC_DEFINE(HAVE_TIME_T,1,[Do we have time_t?])],[],
[#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
])

AC_DEFINE_DIR(SYSCONFDIR,    "${sysconfdir}",    [Where do we find our configuration?])
AC_DEFINE_DIR(DATADIR,       "${datadir}",       [Where do we find our documentation?])
AC_DEFINE_DIR(WWWDIR,        "${wwwdir}",        [Where do we put the web files?])
AC_DEFINE_DIR(LOCALSTATEDIR, "${localstatedir}", [Where do we keep our pid?])
AC_DEFINE_DIR(DBDIR,         "${dbdir}",         [Where do we put our database files?])
AC_DEFINE_DIR(LOCALEDIR,     "${localedir}",     [Where do we put out .mo files?])

AC_OUTPUT([
 Makefile 
  libkap/Makefile
  common/Makefile 
  store/Makefile 
  render/Makefile 
  xslt/Makefile 
  imgs/Makefile 
  doc/Makefile 
  intl/Makefile 
  po/Makefile.in 
 lurker.conf 
  doc/lurker.mbox 
  render/.htaccess])
