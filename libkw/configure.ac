AC_INIT(libesort/esort.h)
AC_CONFIG_AUX_DIR(tools)
AC_PREREQ(2.57)
AM_INIT_AUTOMAKE(libkw, 1.0)

AC_SUBST(LTVERSION, 1:0:0)
AM_MAINTAINER_MODE

AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_PROG_CC
AC_PROG_CXX

AC_LANG(C++)

SFS_PERLINFO
SFS_PERL_POD

AC_PATH_PROGS(PYTHON, python)
PYTHON_VERSION=`$PYTHON -c "import sys; print sys.version[[:3]]"`
PYTHON_PREFIX=`$PYTHON -c "import sys; print sys.prefix"`
PYTHON_EXEC_PREFIX=`$PYTHON -c "import sys; print sys.exec_prefix"`
AC_SUBST(PYTHON_VERSION)
AC_SUBST(PYTHON_PREFIX)
AC_SUBST(PYTHON_EXEC_PREFIX)

if test "x$USE_MAINTAINER_MODE" != "xno"; then
  CXXFLAGS="$CXXFLAGS -Wall -O2 -g"
fi

AC_CONFIG_FILES([
 Makefile libesort/Makefile bindings/Makefile 
 bindings/c/Makefile bindings/perl/Makefile bindings/python/Makefile])
AC_OUTPUT
