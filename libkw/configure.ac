AC_INIT(libesort/esort.h)
AC_CONFIG_AUX_DIR(tools)
AC_PREREQ(2.57)
AM_INIT_AUTOMAKE(libkw, 0.5)

AC_SUBST(LTVERSION, 1:0:0)
AM_MAINTAINER_MODE

AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_PROG_CC
AC_PROG_CXX

AC_LANG(C++)

if test "x$USE_MAINTAINER_MODE" != "xno"; then
  CXXFLAGS="$CXXFLAGS -Wall -O2 -g"
fi

AC_CONFIG_FILES([
 Makefile libesort/Makefile bindings/Makefile bindings/c/Makefile])
AC_OUTPUT
