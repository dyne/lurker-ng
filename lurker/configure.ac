AC_INIT(lurker, 0.1)

AC_PREREQ(2.52)
AC_REVISION($Id: configure.ac,v 1.5 2002-01-22 22:17:10 terpstra Exp $)
AC_CONFIG_SRCDIR(store/main.c)
AC_CONFIG_AUX_DIR(tools)

AM_INIT_AUTOMAKE(lurker, 0.1)
AM_CONFIG_HEADER(common/config.h)

AC_ARG_WITH(wwwdir, AC_HELP_STRING(
	[--with-wwwdir], 
	[www directory (default: PREFIX/var/www/lurker)]))
if test "x$with_wwwdir" = "x"; then
  wwwdir="\${localstatedir}/www/\${PACKAGE}"
else
  wwwdir=$with_wwwdir
fi
AC_SUBST(wwwdir)

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

AC_HEADER_STDC
AC_HEADER_DIRENT

AC_C_CONST
AC_C_INLINE
AC_C_CHAR_UNSIGNED

AC_HEADER_TIME
AC_CHECK_HEADERS(unistd.h fcntl.h io.h time.h sys/mman.h sys/time.h sys/fcntl.h errno.h syslog.h)

AC_FUNC_ALLOCA
AC_FUNC_MMAP

AC_CHECK_FUNCS(setsid fork)
AC_SYS_LARGEFILE

PKG_CHECK_MODULES(STORAGE, st >= 1.3)
DB3_CHECK(STORAGE)
XSLT_CHECK(RENDER)

AC_CHECK_LIB(popt, poptGetContext, 
  STORAGE_LIBS="$STORAGE_LIBS -lpopt",
  AC_MSG_ERROR(Need popt to compile))

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
AC_DEFINE_DIR(LOCALSTATEDIR, "${localstatedir}", [Where do we keep our pid?])
AC_DEFINE_DIR(WWWDIR,        "${wwwdir}",        [Where do we render our files to?])
AC_DEFINE_DIR(DBDIR,         "${dbdir}",         [Where do we put our database files?])

AC_OUTPUT(Makefile common/Makefile store/Makefile render/Makefile lurker.conf)
