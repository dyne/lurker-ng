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

dnl Since db3 comes with no real way to determine its location,
dnl we'll make an attempt at finding it ourselves.  (This is
dnl especially lame since some linkers don't look in the proper
dnl places, e.g. FreeBSD.  Not to mention the library seems to
dnl be called variously `db3' and `db-3.')
AC_DEFUN(DB3_CHECK, [
  AC_MSG_CHECKING(for the db3 library)

  for id in 'db-3' 'db3'; do
    for search in '' '-L/usr/local/lib'; do
      save="$LIBS"
      LIBS="$LIBS $search -l$id"

      AC_TRY_LINK([], [db_env_create()],
	[AC_MSG_RESULT(yes)
	 break 2],
	[LIBS="$save"])
    done
  done

  if test "$save" = "$LIBS"; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([This requires libdb >= 3.2 to build])
  else
    setvar $1_LIBS "$$1_LIBS $search -l$id"
    LIBS="$save"
  fi

  AC_MSG_CHECKING(for db3.2 header files)

  for search in '' '-I/usr/include/db3' '-I/usr/local/include/db3'; do
    save="$CFLAGS";
    CFLAGS="$CFLAGS $search";

    AC_TRY_COMPILE(
      [#include <db.h>],
      [
#if DB_VERSION_MAJOR > 3 || (DB_VERSION_MAJOR == 3 && DB_VERSION_MINOR >= 2)
	return 0;
#else
#error
#endif],
      [AC_MSG_RESULT(yes)
       break],
      [CFLAGS="$save"])
  done

  if test "$save" = "$CFLAGS"; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Unable to locate libdb >= 3.2 header files])
  else
    setvar $1_CFLAGS "$$1_CFLAGS $search"
    CFLAGS="$save"
  fi

  AC_SUBST($1_LIBS)
  AC_SUBST($1_CFLAGS)])
