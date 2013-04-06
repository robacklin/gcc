dnl See if stdbool.h properly defines bool and true/false.
AC_DEFUN(gcc_AC_HEADER_STDBOOL,
[AC_CACHE_CHECK([for working stdbool.h],
  ac_cv_header_stdbool_h,
[AC_TRY_COMPILE([#include <stdbool.h>],
[bool foo = false;],
ac_cv_header_stdbool_h=yes, ac_cv_header_stdbool_h=no)])
if test $ac_cv_header_stdbool_h = yes; then
  AC_DEFINE(HAVE_STDBOOL_H, 1,
  [Define if you have a working <stdbool.h> header file.])
fi
])

dnl See whether we can include both string.h and strings.h.
AC_DEFUN(gcc_AC_HEADER_STRING,
[AC_CACHE_CHECK([whether string.h and strings.h may both be included],
  gcc_cv_header_string,
[AC_TRY_COMPILE([#include <string.h>
#include <strings.h>], , gcc_cv_header_string=yes, gcc_cv_header_string=no)])
if test $gcc_cv_header_string = yes; then
  AC_DEFINE(STRING_WITH_STRINGS, 1, [Define if you can safely include both <string.h> and <strings.h>.])
fi
])

dnl See whether we need a declaration for a function.
dnl The result is highly dependent on the INCLUDES passed in, so make sure
dnl to use a different cache variable name in this macro if it is invoked
dnl in a different context somewhere else.
dnl gcc_AC_CHECK_DECL(SYMBOL,
dnl 	[ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, INCLUDES]]])
AC_DEFUN(gcc_AC_CHECK_DECL,
[AC_MSG_CHECKING([whether $1 is declared])
AC_CACHE_VAL(gcc_cv_have_decl_$1,
[AC_TRY_COMPILE([$4],
[#ifndef $1
char *(*pfn) = (char *(*)) $1 ;
#endif], eval "gcc_cv_have_decl_$1=yes", eval "gcc_cv_have_decl_$1=no")])
if eval "test \"`echo '$gcc_cv_have_decl_'$1`\" = yes"; then
  AC_MSG_RESULT(yes) ; ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT(no) ; ifelse([$3], , :, [$3])
fi
])dnl

dnl Check multiple functions to see whether each needs a declaration.
dnl Arrange to define HAVE_DECL_<FUNCTION> to 0 or 1 as appropriate.
dnl gcc_AC_CHECK_DECLS(SYMBOLS,
dnl 	[ACTION-IF-NEEDED [, ACTION-IF-NOT-NEEDED [, INCLUDES]]])
AC_DEFUN(gcc_AC_CHECK_DECLS,
[for ac_func in $1
do
changequote(, )dnl
  ac_tr_decl=HAVE_DECL_`echo $ac_func | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
changequote([, ])dnl
gcc_AC_CHECK_DECL($ac_func,
  [AC_DEFINE_UNQUOTED($ac_tr_decl, 1) $2],
  [AC_DEFINE_UNQUOTED($ac_tr_decl, 0) $3],
dnl It is possible that the include files passed in here are local headers
dnl which supply a backup declaration for the relevant prototype based on
dnl the definition of (or lack of) the HAVE_DECL_ macro.  If so, this test
dnl will always return success.  E.g. see libiberty.h's handling of
dnl `basename'.  To avoid this, we define the relevant HAVE_DECL_ macro to
dnl 1 so that any local headers used do not provide their own prototype
dnl during this test.
#undef $ac_tr_decl
#define $ac_tr_decl 1
  $4
)
done
dnl Automatically generate config.h entries via autoheader.
if test x = y ; then
  patsubst(translit([$1], [a-z], [A-Z]), [\w+],
    [AC_DEFINE([HAVE_DECL_\&], 1,
      [Define to 1 if we found this declaration otherwise define to 0.])])dnl
fi
])

dnl Check if we have vprintf and possibly _doprnt.
dnl Note autoconf checks for vprintf even though we care about vfprintf.
AC_DEFUN(gcc_AC_FUNC_VFPRINTF_DOPRNT,
[AC_FUNC_VPRINTF
vfprintf=
doprint=
if test $ac_cv_func_vprintf != yes ; then
  vfprintf=vfprintf.o
  if test $ac_cv_func__doprnt != yes ; then
    doprint=doprint.o
  fi
fi
AC_SUBST(vfprintf)
AC_SUBST(doprint)
])    

dnl Check if we have strstr.
AC_DEFUN([gcc_AC_FUNC_STRSTR],
  [AC_CHECK_FUNCS([strstr], [strstr=], [strstr=strstr.o])
   AC_SUBST([strstr])])

dnl See if the printf functions in libc support %p in format strings.
AC_DEFUN(gcc_AC_FUNC_PRINTF_PTR,
[AC_CACHE_CHECK(whether the printf functions support %p,
  gcc_cv_func_printf_ptr,
[AC_TRY_RUN([#include <stdio.h>

int main()
{
  char buf[64];
  char *p = buf, *q = NULL;
  sprintf(buf, "%p", p);
  sscanf(buf, "%p", &q);
  return (p != q);
}], gcc_cv_func_printf_ptr=yes, gcc_cv_func_printf_ptr=no,
	gcc_cv_func_printf_ptr=no)
rm -f core core.* *.core])
if test $gcc_cv_func_printf_ptr = yes ; then
  AC_DEFINE(HAVE_PRINTF_PTR, 1, [Define if printf supports "%p".])
fi
])

dnl See if symbolic links work and if not, try to substitute either hard links or simple copy.
AC_DEFUN(gcc_AC_PROG_LN_S,
[AC_MSG_CHECKING(whether ln -s works)
AC_CACHE_VAL(gcc_cv_prog_LN_S,
[rm -f conftestdata_t
echo >conftestdata_f
if ln -s conftestdata_f conftestdata_t 2>/dev/null
then
  gcc_cv_prog_LN_S="ln -s"
else
  if ln conftestdata_f conftestdata_t 2>/dev/null
  then
    gcc_cv_prog_LN_S=ln
  else
    gcc_cv_prog_LN_S=cp
  fi
fi
rm -f conftestdata_f conftestdata_t
])dnl
LN_S="$gcc_cv_prog_LN_S"
if test "$gcc_cv_prog_LN_S" = "ln -s"; then
  AC_MSG_RESULT(yes)
else
  if test "$gcc_cv_prog_LN_S" = "ln"; then
    AC_MSG_RESULT([no, using ln])
  else
    AC_MSG_RESULT([no, and neither does ln, so using cp])
  fi
fi
AC_SUBST(LN_S)dnl
])

dnl See if hard links work and if not, try to substitute either symbolic links or simple copy.
AC_DEFUN(gcc_AC_PROG_LN,
[AC_MSG_CHECKING(whether ln works)
AC_CACHE_VAL(gcc_cv_prog_LN,
[rm -f conftestdata_t
echo >conftestdata_f
if ln conftestdata_f conftestdata_t 2>/dev/null
then
  gcc_cv_prog_LN="ln"
else
  if ln -s conftestdata_f conftestdata_t 2>/dev/null
  then
    gcc_cv_prog_LN="ln -s"
  else
    gcc_cv_prog_LN=cp
  fi
fi
rm -f conftestdata_f conftestdata_t
])dnl
LN="$gcc_cv_prog_LN"
if test "$gcc_cv_prog_LN" = "ln"; then
  AC_MSG_RESULT(yes)
else
  if test "$gcc_cv_prog_LN" = "ln -s"; then
    AC_MSG_RESULT([no, using ln -s])
  else
    AC_MSG_RESULT([no, and neither does ln -s, so using cp])
  fi
fi
AC_SUBST(LN)dnl
])

dnl See whether the stage1 host compiler accepts the volatile keyword.
AC_DEFUN(gcc_AC_C_VOLATILE,
[AC_CACHE_CHECK([for volatile], gcc_cv_c_volatile,
[AC_TRY_COMPILE(, [volatile int foo;],
        gcc_cv_c_volatile=yes, gcc_cv_c_volatile=no)])
if test $gcc_cv_c_volatile = yes ; then
  AC_DEFINE(HAVE_VOLATILE, 1, [Define if your compiler understands volatile.])
fi
])

dnl Check whether long double is supported.  This differs from the
dnl built-in autoconf test in that it works for cross compiles.
AC_DEFUN(gcc_AC_C_LONG_DOUBLE,
[AC_CACHE_CHECK(for long double, gcc_cv_c_long_double,
[if test "$GCC" = yes; then
  gcc_cv_c_long_double=yes
else
AC_TRY_COMPILE(,
[/* The Stardent Vistra knows sizeof(long double), but does not support it.  */
long double foo = 0.0;
/* On Ultrix 4.3 cc, long double is 4 and double is 8.  */
switch (0) case 0: case (sizeof(long double) >= sizeof(double)):;],
gcc_cv_c_long_double=yes, gcc_cv_c_long_double=no)
fi])
if test $gcc_cv_c_long_double = yes; then
  AC_DEFINE(HAVE_LONG_DOUBLE, 1, 
      [Define if your compiler supports the \`long double' type.])
fi
])

dnl Check whether _Bool is built-in.
AC_DEFUN(gcc_AC_C__BOOL,
[AC_CACHE_CHECK(for built-in _Bool, gcc_cv_c__bool,
[AC_TRY_COMPILE(,
[_Bool foo;],
gcc_cv_c__bool=yes, gcc_cv_c__bool=no)
])
if test $gcc_cv_c__bool = yes; then
  AC_DEFINE(HAVE__BOOL, 1, [Define if the \`_Bool' type is built-in.])
fi
])

dnl Define MKDIR_TAKES_ONE_ARG if mkdir accepts only one argument instead
dnl of the usual 2.
AC_DEFUN(gcc_AC_FUNC_MKDIR_TAKES_ONE_ARG,
[AC_CACHE_CHECK([if mkdir takes one argument], gcc_cv_mkdir_takes_one_arg,
[AC_TRY_COMPILE([
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif], [mkdir ("foo", 0);], 
        gcc_cv_mkdir_takes_one_arg=no, gcc_cv_mkdir_takes_one_arg=yes)])
if test $gcc_cv_mkdir_takes_one_arg = yes ; then
  AC_DEFINE(MKDIR_TAKES_ONE_ARG, 1, [Define if host mkdir takes a single argument.])
fi
])

AC_DEFUN(gcc_AC_PROG_INSTALL,
[AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT])dnl
# Find a good install program.  We prefer a C program (faster),
# so one script is as good as another.  But avoid the broken or
# incompatible versions:
# SysV /etc/install, /usr/sbin/install
# SunOS /usr/etc/install
# IRIX /sbin/install
# AIX /bin/install
# AFS /usr/afsws/bin/install, which mishandles nonexistent args
# SVR4 /usr/ucb/install, which tries to use the nonexistent group "staff"
# ./install, which can be erroneously created by make from ./install.sh.
AC_MSG_CHECKING(for a BSD compatible install)
if test -z "$INSTALL"; then
AC_CACHE_VAL(ac_cv_path_install,
[  IFS="${IFS= 	}"; ac_save_IFS="$IFS"; IFS="${IFS}:"
  for ac_dir in $PATH; do
    # Account for people who put trailing slashes in PATH elements.
    case "$ac_dir/" in
    /|./|.//|/etc/*|/usr/sbin/*|/usr/etc/*|/sbin/*|/usr/afsws/bin/*|/usr/ucb/*) ;;
    *)
      # OSF1 and SCO ODT 3.0 have their own names for install.
      for ac_prog in ginstall scoinst install; do
        if test -f $ac_dir/$ac_prog; then
	  if test $ac_prog = install &&
            grep dspmsg $ac_dir/$ac_prog >/dev/null 2>&1; then
	    # AIX install.  It has an incompatible calling convention.
	    # OSF/1 installbsd also uses dspmsg, but is usable.
	    :
	  else
	    ac_cv_path_install="$ac_dir/$ac_prog -c"
	    break 2
	  fi
	fi
      done
      ;;
    esac
  done
  IFS="$ac_save_IFS"
])dnl
  if test "${ac_cv_path_install+set}" = set; then
    INSTALL="$ac_cv_path_install"
  else
    # As a last resort, use the slow shell script.  We don't cache a
    # path for INSTALL within a source directory, because that will
    # break other packages using the cache if that directory is
    # removed, or if the path is relative.
    INSTALL="$ac_install_sh"
  fi
fi
dnl We do special magic for INSTALL instead of AC_SUBST, to get
dnl relative paths right.
AC_MSG_RESULT($INSTALL)
AC_SUBST(INSTALL)dnl

# Use test -z because SunOS4 sh mishandles braces in ${var-val}.
# It thinks the first close brace ends the variable substitution.
test -z "$INSTALL_PROGRAM" && INSTALL_PROGRAM='${INSTALL}'
AC_SUBST(INSTALL_PROGRAM)dnl

test -z "$INSTALL_DATA" && INSTALL_DATA='${INSTALL} -m 644'
AC_SUBST(INSTALL_DATA)dnl
])

#serial 1
dnl This test replaces the one in autoconf.
dnl Currently this macro should have the same name as the autoconf macro
dnl because gettext's gettext.m4 (distributed in the automake package)
dnl still uses it.  Otherwise, the use in gettext.m4 makes autoheader
dnl give these diagnostics:
dnl   configure.in:556: AC_TRY_COMPILE was called before AC_ISC_POSIX
dnl   configure.in:556: AC_TRY_RUN was called before AC_ISC_POSIX

undefine([AC_ISC_POSIX])
AC_DEFUN(AC_ISC_POSIX,
  [
    dnl This test replaces the obsolescent AC_ISC_POSIX kludge.
    AC_CHECK_LIB(cposix, strerror, [LIBS="$LIBS -lcposix"])
  ]
)


dnl GCC_PATH_PROG(VARIABLE, PROG-TO-CHECK-FOR [, VALUE-IF-NOT-FOUND [, PATH]])
dnl like AC_PATH_PROG but use other cache variables
AC_DEFUN(GCC_PATH_PROG,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(gcc_cv_path_$1,
[case "[$]$1" in
  /*)
  gcc_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  ?:/*)			 
  gcc_cv_path_$1="[$]$1" # Let the user override the test with a dos path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
dnl $ac_dummy forces splitting on constant user-supplied paths.
dnl POSIX.2 word splitting is done only on the output of word expansions,
dnl not every word.  This closes a longstanding sh security hole.
  ac_dummy="ifelse([$4], , $PATH, [$4])"
  for ac_dir in $ac_dummy; do 
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      gcc_cv_path_$1="$ac_dir/$ac_word"
      break
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 3rd arg is given, leave the cache variable unset,
dnl so GCC_PATH_PROGS will keep looking.
ifelse([$3], , , [  test -z "[$]gcc_cv_path_$1" && gcc_cv_path_$1="$3"
])dnl
  ;;
esac])dnl
$1="$gcc_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])



# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 5

AC_DEFUN(AM_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS)
      AC_MSG_CHECKING([whether included gettext is requested])
      AC_ARG_WITH(included-gettext,
        [  --with-included-gettext use the GNU gettext library included here],
        nls_cv_force_use_gnu_gettext=$withval,
        nls_cv_force_use_gnu_gettext=no)
      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CACHE_CHECK([for gettext in libintl],
		 gt_cv_func_gettext_libintl,
		 [AC_CHECK_LIB(intl, gettext,
		  gt_cv_func_gettext_libintl=yes,
		  gt_cv_func_gettext_libintl=no)],
		 gt_cv_func_gettext_libintl=no)])
	   fi

	   if test "$gt_cv_func_gettext_libintl" = "yes"; then
	     LIBS="-lintl $LIBS"
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$gt_cv_func_gettext_libintl" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT)
	      GCC_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		AC_CHECK_FUNCS(dcgettext)
		GCC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		GCC_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
	      else
		create_catalogs="no"
	      fi
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS)
	       INTLOBJS="\$(CATOBJS)"
	       GCC_PATH_PROG(GENCAT, gencat, no)dnl
	       if test "$GENCAT" != "no"; then
		 GCC_PATH_PROG(GMSGFMT, gmsgfmt, no)
		 if test "$GMSGFMT" = "no"; then
		   GCC_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
		   if test "$GMSGFMT" = "msgfmt"; then
		     create_catalogs="no"
		   fi
		 fi
		 GCC_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		 USE_INCLUDED_LIBINTL=yes
		 CATOBJEXT=.cat
		 INSTOBJEXT=.cat
		 DATADIRNAME=lib
		 INTLDEPS='$(top_builddir)/intl/libintl.a'
		 INTLLIBS=$INTLDEPS
		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
		 nls_cv_header_intl=intl/libintl.h
		 nls_cv_header_libgt=intl/libgettext.h
	       fi])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" = "yes"; then
        dnl Mark actions used to generate GNU NLS library.
        INTLOBJS="\$(GETTOBJS)"
        GCC_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
        GCC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
        dnl If we didn't find either msgfmt or gmsgfmt, don't try to
        dnl create a catalog.
	if test "$MSGFMT" = "msgfmt" && test "$GMSGFMT" = "msgfmt"; then
	  create_catalogs="no"
	fi
        GCC_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
        AC_SUBST(MSGFMT)
	USE_INCLUDED_LIBINTL=yes
        CATOBJEXT=.gmo
        INSTOBJEXT=.mo
        DATADIRNAME=share
	INTLDEPS='$(top_builddir)/intl/libintl.a'
	INTLLIBS=$INTLDEPS
	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
        nls_cv_header_intl=intl/libintl.h
        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi



      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


    # If this is used in GNU gettext we have to set USE_NLS to `yes'
    # because some of the sources are only built for this goal.
    if test "$PACKAGE" = gettext; then
      USE_NLS=yes
      USE_INCLUDED_LIBINTL=yes
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN(AM_GNU_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY)
   fi

   AM_LC_MESSAGES
   AM_WITH_NLS

   if test "x$create_catalogs" = "xno"; then
     AC_MSG_WARN([No program for building catalogs found -> building disabled])
   fi

   if test "x$CATOBJEXT" != "x" && test "x$create_catalogs" != "xno" ; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       if test "x$LINGUAS" = "x"; then
	 LINGUAS=$ALL_LINGUAS
       else
	 NEW_LINGUAS=
	 for lang in $ALL_LINGUAS; do
	   case " $LINGUAS " in
	     *" $lang "*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
	   esac
	 done
         LINGUAS=$NEW_LINGUAS
       fi
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but ($top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   if test "x$srcdir" != "x."; then
     if test "x`echo $srcdir | sed 's@/.*@@'`" = "x"; then
       posrcprefix="$srcdir/"
     else
       posrcprefix="../$srcdir/"
     fi
   else
     posrcprefix="../"
   fi
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])

# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

AC_DEFUN(AM_LC_MESSAGES,
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES)
    fi
  fi])

# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

dnl GCC_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(GCC_PATH_PROG_WITH_TEST,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(gcc_cv_path_$1,
[case "[$]$1" in
  /*)
  gcc_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	gcc_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so GCC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]gcc_cv_path_$1" && gcc_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$gcc_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])

# Check whether mmap can map an arbitrary page from /dev/zero or with
# MAP_ANONYMOUS, without MAP_FIXED.
AC_DEFUN([AC_FUNC_MMAP_ANYWHERE],
[AC_CHECK_FUNCS(getpagesize)
# The test program for the next two tests is the same except for one
# set of ifdefs.
changequote({{{,}}})dnl
{{{cat >ct-mmap.inc <<'EOF'
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>

#if !defined (MAP_ANONYMOUS) && defined (MAP_ANON)
# define MAP_ANONYMOUS MAP_ANON
#endif

/* This mess was copied from the GNU getpagesize.h.  */
#ifndef HAVE_GETPAGESIZE
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif

/* Assume that all systems that can run configure have sys/param.h.  */
# ifndef HAVE_SYS_PARAM_H
#  define HAVE_SYS_PARAM_H 1
# endif

# ifdef _SC_PAGESIZE
#  define getpagesize() sysconf(_SC_PAGESIZE)
# else /* no _SC_PAGESIZE */
#  ifdef HAVE_SYS_PARAM_H
#   include <sys/param.h>
#   ifdef EXEC_PAGESIZE
#    define getpagesize() EXEC_PAGESIZE
#   else /* no EXEC_PAGESIZE */
#    ifdef NBPG
#     define getpagesize() NBPG * CLSIZE
#     ifndef CLSIZE
#      define CLSIZE 1
#     endif /* no CLSIZE */
#    else /* no NBPG */
#     ifdef NBPC
#      define getpagesize() NBPC
#     else /* no NBPC */
#      ifdef PAGESIZE
#       define getpagesize() PAGESIZE
#      endif /* PAGESIZE */
#     endif /* no NBPC */
#    endif /* no NBPG */
#   endif /* no EXEC_PAGESIZE */
#  else /* no HAVE_SYS_PARAM_H */
#   define getpagesize() 8192	/* punt totally */
#  endif /* no HAVE_SYS_PARAM_H */
# endif /* no _SC_PAGESIZE */

#endif /* no HAVE_GETPAGESIZE */

#ifndef MAP_FAILED
# define MAP_FAILED -1
#endif

#undef perror_exit
#define perror_exit(str, val) \
  do { perror(str); exit(val); } while (0)

/* Some versions of cygwin mmap require that munmap is called with the
   same parameters as mmap.  GCC expects that this is not the case.
   Test for various forms of this problem.  Warning - icky signal games.  */

static sigset_t unblock_sigsegv;
static jmp_buf r;
static size_t pg;
static int devzero;

static char *
anonmap (size)
     size_t size;
{
#ifdef USE_MAP_ANON
  return (char *) mmap (0, size, PROT_READ|PROT_WRITE,
			MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#else
  return (char *) mmap (0, size, PROT_READ|PROT_WRITE,
			MAP_PRIVATE, devzero, 0);
#endif
}

static void
sigsegv (unused)
     int unused;
{
  sigprocmask (SIG_UNBLOCK, &unblock_sigsegv, 0);
  longjmp (r, 1);
}

/* Basic functionality test.  */
void
test_0 ()
{
  char *x = anonmap (pg);
  if (x == (char *) MAP_FAILED)
    perror_exit("test 0 mmap", 2);

  *(int *)x += 1;

  if (munmap(x, pg) < 0)
    perror_exit("test 0 munmap", 3);
}

/* 1. If we map a 2-page region and unmap its second page, the first page
   must remain.  */
static void
test_1 ()
{
  char *x = anonmap (pg * 2);
  if (x == (char *)MAP_FAILED)
    perror_exit ("test 1 mmap", 4);

  signal (SIGSEGV, sigsegv);
  if (setjmp (r))
    perror_exit ("test 1 fault", 5);

  x[0] = 1;
  x[pg] = 1;

  if (munmap (x + pg, pg) < 0)
    perror_exit ("test 1 munmap 1", 6);
  x[0] = 2;

  if (setjmp (r) == 0)
    {
      x[pg] = 1;
      perror_exit ("test 1 no fault", 7);
    }
  if (munmap (x, pg) < 0)
    perror_exit ("test 1 munmap 2", 8);
}

/* 2. If we map a 2-page region and unmap its first page, the second
   page must remain.  */
static void
test_2 ()
{
  char *x = anonmap (pg * 2);
  if (x == (char *)MAP_FAILED)
    perror_exit ("test 2 mmap", 9);

  signal (SIGSEGV, sigsegv);
  if (setjmp (r))
    perror_exit ("test 2 fault", 10);

  x[0] = 1;
  x[pg] = 1;

  if (munmap (x, pg) < 0)
    perror_exit ("test 2 munmap 1", 11);

  x[pg] = 2;

  if (setjmp (r) == 0)
    {
      x[0] = 1;
      perror_exit ("test 2 no fault", 12);
    }

  if (munmap (x+pg, pg) < 0)
    perror_exit ("test 2 munmap 2", 13);
}

/* 3. If we map two adjacent 1-page regions and unmap them both with
   one munmap, both must go away.

   Getting two adjacent 1-page regions with two mmap calls is slightly
   tricky.  All OS's tested skip over already-allocated blocks; therefore
   we have been careful to unmap all allocated regions in previous tests.
   HP/UX allocates pages backward in memory.  No OS has yet been observed
   to be so perverse as to leave unmapped space between consecutive calls
   to mmap.  */

static void
test_3 ()
{
  char *x, *y, *z;

  x = anonmap (pg);
  if (x == (char *)MAP_FAILED)
    perror_exit ("test 3 mmap 1", 14);
  y = anonmap (pg);
  if (y == (char *)MAP_FAILED)
    perror_exit ("test 3 mmap 2", 15);

  if (y != x + pg)
    {
      if (y == x - pg)
	z = y, y = x, x = z;
      else
	{
	  fprintf (stderr, "test 3 nonconsecutive pages - %lx, %lx\n",
		   (unsigned long)x, (unsigned long)y);
	  exit (16);
	}
    }

  signal (SIGSEGV, sigsegv);
  if (setjmp (r))
    perror_exit ("test 3 fault", 17);

  x[0] = 1;
  y[0] = 1;

  if (munmap (x, pg*2) < 0)
    perror_exit ("test 3 munmap", 18);

  if (setjmp (r) == 0)
    {
      x[0] = 1;
      perror_exit ("test 3 no fault 1", 19);
    }
  
  signal (SIGSEGV, sigsegv);
  if (setjmp (r) == 0)
    {
      y[0] = 1;
      perror_exit ("test 3 no fault 2", 20);
    }
}

int
main ()
{
  sigemptyset (&unblock_sigsegv);
  sigaddset (&unblock_sigsegv, SIGSEGV);
  pg = getpagesize ();
#ifndef USE_MAP_ANON
  devzero = open ("/dev/zero", O_RDWR);
  if (devzero < 0)
    perror_exit ("open /dev/zero", 1);
#endif

  test_0();
  test_1();
  test_2();
  test_3();

  exit(0);
}
EOF}}}
changequote([,])dnl

AC_CACHE_CHECK(for working mmap from /dev/zero,
  ac_cv_func_mmap_dev_zero,
[AC_TRY_RUN(
 [#include "ct-mmap.inc"],
 ac_cv_func_mmap_dev_zero=yes,
 [if test $? -lt 4
 then ac_cv_func_mmap_dev_zero=no
 else ac_cv_func_mmap_dev_zero=buggy
 fi],
 # If this is not cygwin, and /dev/zero is a character device, it's probably
 # safe to assume it works.
 [case "$host_os" in
   cygwin* | win32 | pe | mingw* ) ac_cv_func_mmap_dev_zero=buggy ;;
   * ) if test -c /dev/zero
       then ac_cv_func_mmap_dev_zero=yes
       else ac_cv_func_mmap_dev_zero=no
       fi ;;
  esac])
])
if test $ac_cv_func_mmap_dev_zero = yes; then
  AC_DEFINE(HAVE_MMAP_DEV_ZERO, 1,
	    [Define if mmap can get us zeroed pages from /dev/zero.])
fi

AC_CACHE_CHECK([for working mmap with MAP_ANON(YMOUS)],
  ac_cv_func_mmap_anon,
[AC_TRY_RUN(
 [#define USE_MAP_ANON
#include "ct-mmap.inc"],
 ac_cv_func_mmap_anon=yes,
 [if test $? -lt 4
 then ac_cv_func_mmap_anon=no
 else ac_cv_func_mmap_anon=buggy
 fi],
 # Unlike /dev/zero, it is not safe to assume MAP_ANON(YMOUS) works
 # just because it's there. Some SCO Un*xen define it but don't implement it.
 ac_cv_func_mmap_anon=no)
])
if test $ac_cv_func_mmap_anon = yes; then
  AC_DEFINE(HAVE_MMAP_ANON, 1,
	    [Define if mmap can get us zeroed pages using MAP_ANON(YMOUS).])
fi
rm -f ct-mmap.inc
])

# Check whether mmap can map a plain file, without MAP_FIXED.
AC_DEFUN([AC_FUNC_MMAP_FILE], 
[AC_CACHE_CHECK(for working mmap of a file, ac_cv_func_mmap_file,
[# Create a file one thousand bytes long.
for i in 1 2 3 4 5 6 7 8 9 0
do for j in 1 2 3 4 5 6 7 8 9 0
do echo $i $j xxxxx
done
done > conftestdata$$

AC_TRY_RUN([
/* Test by Zack Weinberg.  Modified from MMAP_ANYWHERE test by
   Richard Henderson and Alexandre Oliva.
   Check whether read-only mmap of a plain file works. */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main()
{
  char *x;
  int fd;
  struct stat st;

  fd = open("conftestdata$$", O_RDONLY);
  if (fd < 0)
    exit(1);

  if (fstat (fd, &st))
    exit(2);

  x = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (x == (char *) -1)
    exit(3);

  if (x[0] != '1' || x[1] != ' ' || x[2] != '1' || x[3] != ' ')
    exit(4);

  if (munmap(x, st.st_size) < 0)
    exit(5);

  exit(0);
}], ac_cv_func_mmap_file=yes, ac_cv_func_mmap_file=no,
ac_cv_func_mmap_file=no)])
if test $ac_cv_func_mmap_file = yes; then
  AC_DEFINE(HAVE_MMAP_FILE, 1,
	    [Define if read-only mmap of a plain file works.])
fi
])

dnl Locate a program and check that its version is acceptable.
dnl AC_PROG_CHECK_VER(var, name, version-switch,
dnl                  version-extract-regexp, version-glob)
AC_DEFUN(gcc_AC_CHECK_PROG_VER,
[AC_CHECK_PROG([$1], [$2], [$2])
if test -n "[$]$1"; then
  # Found it, now check the version.
  AC_CACHE_CHECK(for modern $2, gcc_cv_prog_$2_modern,
[changequote(<<,>>)dnl
  ac_prog_version=`<<$>>$1 $3 2>&1 |
                   sed -n 's/^.*patsubst(<<$4>>,/,\/).*$/\1/p'`
changequote([,])dnl
  echo "configure:__oline__: version of $2 is $ac_prog_version" >&AC_FD_CC
changequote(<<,>>)dnl
  case $ac_prog_version in
    '')     gcc_cv_prog_$2_modern=no;;
    <<$5>>)
            gcc_cv_prog_$2_modern=yes;;
    *)      gcc_cv_prog_$2_modern=no;;
  esac
changequote([,])dnl
])
else
  gcc_cv_prog_$2_modern=no
fi
])

#serial AM2

dnl From Bruno Haible.

AC_DEFUN([AM_ICONV],
[
  dnl Some systems have iconv in libc, some have it in libiconv (OSF/1 and
  dnl those with the standalone portable GNU libiconv installed).

  AC_ARG_WITH([libiconv-prefix],
[  --with-libiconv-prefix=DIR  search for libiconv in DIR/include and DIR/lib], [
    for dir in `echo "$withval" | tr : ' '`; do
      if test -d $dir/include; then CPPFLAGS="$CPPFLAGS -I$dir/include"; fi
      if test -d $dir/lib; then LDFLAGS="$LDFLAGS -L$dir/lib"; fi
    done
   ])

  AC_CACHE_CHECK(for iconv, am_cv_func_iconv, [
    am_cv_func_iconv="no, consider installing GNU libiconv"
    am_cv_lib_iconv=no
    AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
      [iconv_t cd = iconv_open("","");
       iconv(cd,NULL,NULL,NULL,NULL);
       iconv_close(cd);],
      am_cv_func_iconv=yes)
    if test "$am_cv_func_iconv" != yes; then
      am_save_LIBS="$LIBS"
      LIBS="$LIBS -liconv"
      AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
        [iconv_t cd = iconv_open("","");
         iconv(cd,NULL,NULL,NULL,NULL);
         iconv_close(cd);],
        am_cv_lib_iconv=yes
        am_cv_func_iconv=yes)
      LIBS="$am_save_LIBS"
    fi
  ])
  if test "$am_cv_func_iconv" = yes; then
    AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])
    AC_MSG_CHECKING([for iconv declaration])
    AC_CACHE_VAL(am_cv_proto_iconv, [
      AC_TRY_COMPILE([
#include <stdlib.h>
#include <iconv.h>
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
#else
size_t iconv();
#endif
], [], am_cv_proto_iconv_arg1="", am_cv_proto_iconv_arg1="const")
      am_cv_proto_iconv="extern size_t iconv (iconv_t cd, $am_cv_proto_iconv_arg1 char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);"])
    am_cv_proto_iconv=`echo "[$]am_cv_proto_iconv" | tr -s ' ' | sed -e 's/( /(/'`
    AC_MSG_RESULT([$]{ac_t:-
         }[$]am_cv_proto_iconv)
    AC_DEFINE_UNQUOTED(ICONV_CONST, $am_cv_proto_iconv_arg1,
      [Define as const if the declaration of iconv() needs const.])
  fi
  LIBICONV=
  if test "$am_cv_lib_iconv" = yes; then
    LIBICONV="-liconv"
  fi
  AC_SUBST(LIBICONV)
])
