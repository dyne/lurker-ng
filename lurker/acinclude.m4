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
  else
     AC_MSG_CHECKING($1_CFLAGS)
     $1_CFLAGS=`$XSLT_CONFIG --cflags || echo no`
     AC_MSG_RESULT($$1_CFLAGS)
     if test "x$$1_CFLAGS" = "xno"; then
       AC_MSG_ERROR(Need xslt to compile)
     fi
     
     AC_MSG_CHECKING($1_LIBS)
     $1_LIBS=`$XSLT_CONFIG --libs || echo no`
     AC_MSG_RESULT($$1_LIBS)
     if test "x$$1_LIBS" = "xno"; then
       AC_MSG_ERROR(Need xslt to compile)
     fi

     AC_SUBST($1_CFLAGS)
     AC_SUBST($1_LIBS)
  fi
])
