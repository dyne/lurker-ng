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

AC_DEFUN(ST_CHECK,
[ AC_MSG_CHECKING(for the st library)

  AC_ARG_WITH(stdir, AC_HELP_STRING(
	[--with-stdir], 
	[st installation directory (default: none)]))
  if test "x$with_stdir" != "x"; then
    ST_LTEST="-L$with_stdir/lib -L$with_stdir"
    ST_ITEST="-I$with_stdir/include -I$with_stdir"
  fi

  for search in $ST_LTEST '' '-L/usr/local/lib'; do
    save="$LIBS"
    LIBS="$LIBS $search -lst"
    AC_TRY_LINK([], [st_thread_create()],
	[AC_MSG_RESULT(yes)
	 break],
	[LIBS="$save"])
  done

  if test "$save" = "$LIBS"; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([This requires libst to build])
  else
    $1_LIBS="$$1_LIBS $search -lst"
    LIBS="$save"
  fi

  AC_MSG_CHECKING(for st header files)

  for search in $ST_ITEST '' '-I/usr/local/include'; do
    save="$CFLAGS";
    CFLAGS="$CFLAGS $search";

    AC_TRY_COMPILE(
      [#include <st.h>],
      [ st_sleep(1); return 0;],
      [AC_MSG_RESULT(yes)
       break],
      [CFLAGS="$save"])
  done

  if test "$save" = "$CFLAGS"; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Unable to locate st header files])
  else
    $1_CFLAGS="$$1_CFLAGS $search"
    CFLAGS="$save"
  fi

  AC_SUBST($1_LIBS)
  AC_SUBST($1_CFLAGS)])

AC_DEFUN(C_CLIENT_CHECK, 
[ AC_MSG_CHECKING(for the c-client library)
  
  AC_ARG_WITH(cclientdir, AC_HELP_STRING(
	[--with-cclientdir], 
	[c-client installation directory (default: none)]))
  if test "x$with_cclientdir" != "x"; then
    CL_LTEST="-L$with_cclientdir/lib -L$with_cclientdir"
    CL_ITEST="-I$with_cclientdir/include -I$with_cclientdir"
  fi

  for search in $CL_LTEST '' '-L/usr/local/lib'; do
    save="$LIBS"
    LIBS="$LIBS $search -lc-client"
    AC_TRY_LINK(
[void mm_expunged() {}
void mm_diskerror() {}
void mm_lsub() {}
void mm_flags() {}
void mm_fatal() {}
void mm_nocritical() {}
void mm_notify() {}
void mm_searched() {}
void mm_status() {}
void mm_login() {}
void mm_list() {}
void mm_critical() {}
void mm_exists() {}
void mm_log() {}
void mm_dlog() {}
extern void rfc822_parse_msg_full();], [rfc822_parse_msg_full(); ],
	[AC_MSG_RESULT(yes)
	 break],
	[LIBS="$save"])
  done
  
  if test "$save" = "$LIBS"; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([This requires libc-client to build])
  else
    $1_LIBS="$$1_LIBS $search -lc-client"
    LIBS="$save"
  fi

  AC_MSG_CHECKING(for c-client header files)

  for search in $CL_ITEST '' '-I/usr/local/include'; do
    save="$CFLAGS";
    CFLAGS="$CFLAGS $search";

    AC_TRY_COMPILE(
[
#include <sys/types.h>
#include <c-client/mail.h>
#include <c-client/rfc822.h>
#include <c-client/fs.h>
],
      [ const char* buf;
       struct mail_envelope* env;
	struct mail_bodystruct* body;
	STRING bss;
        rfc822_parse_msg(
		&env, 
		&body, 
		buf, 
		(size_t)(23424),
		&bss, 
		"localhost", 
		0);
	return 0;],
      [AC_MSG_RESULT(yes)
       break],
      [CFLAGS="$save"])
  done

  if test "$save" = "$CFLAGS"; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([Unable to locate c-client header files])
  else
    $1_CFLAGS="$$1_CFLAGS $search"
    CFLAGS="$save"
  fi

  AC_SUBST($1_LIBS)
  AC_SUBST($1_CFLAGS)])
