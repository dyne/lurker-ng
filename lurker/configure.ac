AC_INIT(lurker, pre-0.9, wesley@terpstra.ca)
AC_CONFIG_SRCDIR(Jamrules.in)
AC_PREREQ(2.57)

AC_LANG(C++)

AC_SEARCH_HEADER_DIRS(
  [iconv.h],
  [/usr/local/include],,
  [AC_MSG_ERROR(Need iconv.h to compile)])

AC_SEARCH_EXPRESSION_LIBS([iconv_t ic = iconv_open("ISO-8859-1", "UTF-8")],
  [iconv], [#include <iconv.h>],,
  [AC_MSG_ERROR(Need iconv to link)])

AC_ARG_WITH([mimelib-local],   AC_HELP_STRING([--with-mimelib-local],
                                              [Use internal lurker/mimelib/*]),,
            [with_mimelib_local="no"])

AC_ARG_WITH([mimelib-include], AC_HELP_STRING([--with-mimelib-include=DIR],
                                              [Location of mimelib/message.h]),,
            [with_mimelib_include=""])

if test "x$with_mimelib_local" != "xno"; then
 AC_MSG_CHECKING(for mimelib)
 AC_MSG_RESULT(local)
 LOCAL_MIMELIB=yes
 CPPFLAGS="$CPPFLAGS -I\$(TOP)/mimelib"
 LIBS="-L\$(TOP)/mimelib -lmimelib $LIBS"
else
 LOCAL_MIMELIB=no
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
AC_SUBST(LOCAL_MIMELIB, ${LOCAL_MIMELIB})

AC_PATH_PROG(JAM, jam)
if test "x${JAM}" = "x"; then
  AC_MSG_ERROR([cannot build - jam required])
fi

AC_DEFINE_DIR(BINDIR,  "${bindir}")
AC_DEFINE_DIR(LIBDIR,  "${libdir}")
AC_DEFINE_DIR(INCDIR,  "${includedir}")
AC_DEFINE_DIR(VARDIR,  "${localstatedir}")
AC_DEFINE_DIR(ETCDIR,  "${sysconfdir}")
AC_DEFINE_DIR(DATADIR, "${datadir}")
AC_DEFINE_DIR(MANDIR,  "${mandir}")

AC_CONFIG_FILES(Jamrules lurker.conf common/Version.h)
AC_OUTPUT
