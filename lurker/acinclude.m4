AC_DEFUN([AC_DEFINE_DIR],
 [prefix_backup="$prefix"
  exec_prefix_backup="$exec_prefix"
  ac_define_dir_result="$2"
  if test "x${prefix}" = "xNONE"; then prefix="${ac_default_prefix}"; fi
  if test "x${exec_prefix}" = "xNONE"; then exec_prefix="${prefix}"; fi
  while echo "$ac_define_dir_result" | grep -q '\$'; do
    ac_define_dir_result=`eval echo $ac_define_dir_result`
  done
  prefix="${prefix_backup}"
  exec_prefix="${exec_prefix}"
  AC_DEFINE_UNQUOTED([$1], "$ac_define_dir_result", [$3])
  $1_expand="$ac_define_dir_result"
  AC_SUBST($1_expand)
 ])

AC_DEFUN(XSLT_CHECK, [
  if test -z "$XSLT_CONFIG"; then
    AC_PATH_PROG(XSLT_CONFIG, xslt-config, no)
  fi

  if test "$XSLT_CONFIG" = "no" ; then
     echo "*** The xslt-config script could not be found. Make sure it is"
     echo "*** in your path, or set the XSLT_CONFIG environment variable"
     echo "*** to the full path to xslt-config."
     AC_MSG_ERROR(Need xslt to compile)
  else
     AC_MSG_CHECKING(for libxslt)
     xslt_CFLAGS=`$XSLT_CONFIG --cflags || echo no`
     if test "x$$1_CFLAGS" = "xno"; then
       AC_MSG_ERROR(Need xslt to compile)
     fi
     
     xslt_LIBS=`$XSLT_CONFIG --libs || echo no`
     if test "x$$1_LIBS" = "xno"; then
       AC_MSG_ERROR(Need xslt to compile)
     fi
     
     AC_MSG_RESULT(have it)
     
     $1_CFLAGS="$$1_CFLAGS $xslt_CFLAGS"
     $1_LIBS="$$1_LIBS $xslt_LIBS"
     
     AC_SUBST($1_CFLAGS)
     AC_SUBST($1_LIBS)
  fi
])

AC_DEFUN(DB3_CHECK, [
  AC_ARG_WITH(db3_libs, AC_HELP_STRING(
	[--with-db3-libs],
	[library options to link against db3]))
  AC_ARG_WITH(db3_include, AC_HELP_STRING(
	[--with-db3-include],
        [compiler flags to compile against db3]))

  if test "x$with_db3_libs" = "x"; then
   AC_CHECK_LIB(db-3, db_env_create,
    $1_LIBS="$$1_LIBS -ldb-3",
      AC_MSG_ERROR(Need libdb-3 to compile))
  else
   db3_LIBS="$with_db3_libs"
   bak_LIBS="$LIBS"
   LIBS="$LIBS $db3_LIBS"
   AC_MSG_CHECKING(for db3 with $db3_LIBS)
   AC_TRY_LINK(
    [void db_env_create();],
    [db_env_create();],
    [AC_MSG_RESULT(ok)],
    [AC_MSG_ERROR($with_db3_libs does not seem to be db3+)])
   LIBS="$bak_LIBS"
  fi

  db3_CFLAGS="$with_db3_include"
  if test "x$db3_CFLAGS" = "x"; then
    AC_MSG_CHECKING([for db.h >= 3.2.x])
  else
    AC_MSG_CHECKING([for db.h >= 3.2.x with $db3_CFLAGS])
  fi

  bak_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $db3_CFLAGS"
  AC_TRY_COMPILE(
    [#include <db.h>],
    [#if (DB_VERSION_MAJOR == 3 && DB_VERSION_MINOR >= 2) || DB_VERSION_MAJOR > 3
return 0;
#else
#error The db3 header is too old  
#endif],
    [AC_MSG_RESULT([have it])],
    [AC_MSG_ERROR([Need libdb >= 3.2.x])])
CFLAGS="$bak_CFLAGS"

  $1_LIBS="$$1_LIBS $db3_LIBS"
  $1_CFLAGS="$$1_CFLAGS $db3_CFLAGS"

  AC_SUBST($1_LIBS)
  AC_SUBST($1_CFLAGS)
])
