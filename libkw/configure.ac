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

dnl
dnl Find perl
dnl
AC_DEFUN(SFS_PERLINFO,
[AC_ARG_WITH(perl,
--with-perl=PATH          Specify perl executable to use,
[case "$withval" in
        yes|no|"") ;;
        *) PERL="$withval" ;;
esac])
if test -z "$PERL" || test ! -x "$PERL"; then
        AC_PATH_PROGS(PERL, perl5 perl)
fi
if test -x "$PERL" && $PERL -e 'require 5.004'; then :; else
        AC_MSG_ERROR("Can\'t find perl 5.004 or later")
fi
AC_CACHE_CHECK(for perl includes, sfs_cv_perl_ccopts,
        sfs_cv_perl_ccopts=`$PERL -MExtUtils::Embed -e ccopts`
        sfs_cv_perl_ccopts=`echo $sfs_cv_perl_ccopts`
)
AC_CACHE_CHECK(for perl libraries, sfs_cv_perl_ldopts,
        sfs_cv_perl_ldopts=`$PERL -MExtUtils::Embed -e ldopts -- -std`
        sfs_cv_perl_ldopts=`echo $sfs_cv_perl_ldopts`
)
AC_CACHE_CHECK(for perl xsubpp, sfs_cv_perl_xsubpp,
        sfs_cv_perl_xsubpp="$PERL "`$PERL -MConfig -e 'print qq(\
        -I$Config{"installarchlib"} -I$Config{"installprivlib"}\
        $Config{"installprivlib"}/ExtUtils/xsubpp\
        -typemap $Config{"installprivlib"}/ExtUtils/typemap)'`
        sfs_cv_perl_xsubpp=`echo $sfs_cv_perl_xsubpp`
)
XSUBPP="$sfs_cv_perl_xsubpp"
PERL_INC="$sfs_cv_perl_ccopts"
PERL_LIB="$sfs_cv_perl_ldopts"
PERL_XSI="$PERL -MExtUtils::Embed -e xsinit -- -std"
AC_SUBST(PERL)
AC_SUBST(PERL_INC)
AC_SUBST(PERL_LIB)
AC_SUBST(PERL_XSI)
AC_SUBST(XSUBPP)
])
dnl
dnl Check for perl and for Pod::Man for generating man pages
dnl
AC_DEFUN(SFS_PERL_POD,
[
if test -z "$PERL" || test ! -x "$PERL"; then
        AC_PATH_PROGS(PERL, perl5 perl)
fi
AC_PATH_PROGS(POD2MAN, pod2man)
if test "$PERL"; then
        AC_CACHE_CHECK(for Pod::Man, sfs_cv_perl_pod_man,
                $PERL -e '{ require Pod::Man }' >/dev/null 2>&1
                if test $? = 0; then
                        sfs_cv_perl_pod_man="yes"
                else
                        sfs_cv_perl_pod_man="no"
                fi                
        )
        PERL_POD_MAN="$sfs_cv_perl_pod_man"
fi
AC_SUBST(PERL_POD_MAN)
])
SFS_PERLINFO
SFS_PERL_POD

if test "x$USE_MAINTAINER_MODE" != "xno"; then
  CXXFLAGS="$CXXFLAGS -Wall -O2 -g"
fi

AC_CONFIG_FILES([
 Makefile libesort/Makefile bindings/Makefile 
 bindings/c/Makefile bindings/perl/Makefile])
AC_OUTPUT

