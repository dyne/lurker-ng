AC_INIT(lurker, 0.6, wesley@terpstra.ca)
AC_CONFIG_SRCDIR(Jamrules.in)
AC_PREREQ(2.57)

AC_LANG(C++)

AC_SEARCH_HEADER_DIRS(
  [iconv.h],
  [/usr/local/include],,
  [AC_MSG_ERROR(Need iconv.h to compile)])

AC_SEARCH_LIBS(iconv,
  [iconv],,
  [AC_MSG_ERROR(Need iconv to link)])

AC_ARG_WITH([mimelib-include], AC_HELP_STRING([--with-mimelib-include],
                                              [Location of mimelib/message.h]),,
            [with_mimelib_include=""])

AC_SEARCH_HEADER_DIRS(
  [mimelib/message.h], 
  [$with_mimelib_include /usr/include/kde /usr/local/include /usr/local/include/kde],, 
  [AC_MSG_ERROR(Need mimelib/*.h to compile)])

AC_ARG_WITH([mimelib-libname], AC_HELP_STRING([--with-mimelib-libname],
                                              [Try an alternative library name]),,
            [with_mimelib_libname=""])

AC_SEARCH_CLASS_LIBS(DwMessage, 
  [$with_mimelib_libname mimelib mimepp], [#include <mimelib/message.h>],,
  [AC_MSG_ERROR(Need mimelib to link)])

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

AC_CONFIG_FILES(Jamrules lurker.conf)
AC_OUTPUT
