# AC_SEARCH_HEADER_DIRS(HEADER-FILE, SEARCH-DIRS,
#                        [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                        [INCLUDES])
AC_DEFUN([AC_SEARCH_HEADER_DIRS],
[AS_VAR_PUSHDEF([ac_Header], [ac_cv_header_$1])dnl
AC_CACHE_CHECK([for $1], ac_Header,
[ac_header_search_save_CPPFLAGS=$CPPFLAGS
AS_VAR_SET(ac_Header, no)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT([$5])
@%:@include <$1>])],
                  [AS_VAR_SET(ac_Header, standard)])
if test AS_VAR_GET(ac_Header) = no; then
  for ac_inc in $2; do
    CPPFLAGS="-I$ac_inc $ac_header_search_save_CPPFLAGS"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT([$5])
@%:@include <$1>])],
                      [AS_VAR_SET(ac_Header, -I$ac_inc)
break])
  done
fi
CPPFLAGS=$ac_header_search_save_CPPFLAGS])
AS_IF([test AS_VAR_GET(ac_Header) != no],
  [test AS_VAR_GET(ac_Header) = standard || CPPFLAGS="AS_VAR_GET(ac_Header) $CPPFLAGS"
  $3],
      [$4])dnl
AS_VAR_POPDEF([ac_Header])dnl
])

# AC_SEARCH_CLASS_LIBS(CLASS, SEARCH-LIBS, [INCLUDES]
#                       [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                       [OTHER-LIBRARIES])
AC_DEFUN([AC_SEARCH_CLASS_LIBS],
[AC_CACHE_CHECK([for library containing class $1], [ac_cv_class_$1],
[ac_class_search_save_LIBS=$LIBS
ac_cv_class_$1=no
AC_LINK_IFELSE([AC_LANG_PROGRAM([$3],[$1 foobar])],
               [ac_cv_class_$1="none required"])
if test "$ac_cv_class_$1" = no; then
  for ac_lib in $2; do
    LIBS="-l$ac_lib $6 $ac_class_search_save_LIBS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([$3], [$1 foobar])],
                   [ac_cv_class_$1="-l$ac_lib" 
break])
  done
fi
LIBS=$ac_class_search_save_LIBS])
AS_IF([test "$ac_cv_class_$1" != no],
  [test "$ac_cv_class_$1" = "none required" || LIBS="$ac_cv_class_$1 $LIBS"
  $4],
      [$5])dnl
])

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
  $1="$ac_define_dir_result"
  AC_SUBST($1)
 ])

