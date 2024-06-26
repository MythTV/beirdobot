# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.71])

AC_INIT([beirdobot],[GIT],[gjhurlbu@gmail.com])
AC_CONFIG_AUX_DIR(scripts)
AC_CONFIG_SRCDIR(bot/src/main.c)
AC_CONFIG_HEADERS(bot/include/config.h)
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

AC_ARG_VAR(CUT, [location of the cut command])
AC_PATH_PROG(CUT, cut)

AC_MSG_CHECKING(version of the package)

VERSION=`if test -f bot/include/release.h ; then (cat bot/include/release.h | grep "GIT_Version =" | $CUT -d '"' -f 2) ; else echo GIT ; fi`
AC_MSG_RESULT(${VERSION})

AC_SUBST(VERSION)

# Check for Cygwin
AC_MSG_CHECKING([for cygwin])
_using_cygwin=no
test "x$target_os" = "xcygwin" && _using_cygwin=yes
AC_MSG_RESULT($_using_cygwin)
AM_CONDITIONAL([USING_CYGWIN], [test "x$_using_cygwin" = "xyes"])

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for header files.
#AC_HEADER_DIRENT
# Autoupdate added the next two lines to ensure that your configure
# script's behavior did not change.  They are probably safe to remove.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

#AC_HEADER_SYS_WAIT
#AC_CHECK_HEADERS([fcntl.h limits.h malloc.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/file.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

AC_STRUCT_TM
#AC_TYPE_INTPTR_T

#AC_CHECK_TYPES(socklen_t)

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES

AC_FUNC_STRNLEN
#AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([strdup])
#AC_CHECK_FUNCS([bzero gethostbyaddr gethostbyname gethostname getpagesize gettimeofday inet_ntoa isascii memset select socket strcasecmp strchr strdup strncasecmp strstr strsep])

dnl Since the stupid libc-client requires the callbacks to be defined, I need
dnl a version of AC_CHECK_LIB where I can hand in the prolog to AC_LANG_CALL
dnl (passing it in $6).  Note: also removing the library from LIBS to prevent
dnl failure on all subsequent compile tries

AC_DEFUN([AC_CHECK_LIB_PROLOG],
[m4_ifval([$3], , [AH_CHECK_LIB([$1])])
AS_LITERAL_IF([$1],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1_$2])],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1''_$2])])
AC_CACHE_CHECK([for $2 in -l$1], ac_Lib,
[ac_check_lib_save_LIBS=$LIBS
LIBS="-l$1 $5 $LIBS"
AC_LINK_IFELSE([AC_LANG_CALL([$6], [$2])],
               [AS_VAR_SET(ac_Lib, yes)],
               [AS_VAR_SET(ac_Lib, no)])
LIBS=$ac_check_lib_save_LIBS])
AS_IF([test AS_VAR_GET(ac_Lib) = yes],
      [m4_default([$3], 
      [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1))
       LIBS="-l$1 $LIBS"])],
      [$4])
AS_VAR_POPDEF([ac_Lib])
LIBS=$ac_check_lib_save_LIBS
])# AC_CHECK_LIB_PROLOG

AC_DEFUN([AC_CHECK_LIB_PROG],
[m4_ifval([$3], , [AH_CHECK_LIB([$1])])
AS_LITERAL_IF([$1],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1_$2])],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1''_$2])])
AC_CACHE_CHECK([for $2 in -l$1], ac_Lib,
[ac_check_lib_save_LIBS=$LIBS
LIBS="-l$1 $5 $LIBS"
AC_LINK_IFELSE([AC_LANG_PROGRAM([$6], [$2])],
               [AS_VAR_SET(ac_Lib, yes)],
               [AS_VAR_SET(ac_Lib, no)])
LIBS=$ac_check_lib_save_LIBS])
AS_IF([test AS_VAR_GET(ac_Lib) = yes],
      [m4_default([$3], 
      [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1))
       LIBS="-l$1 $LIBS"])],
      [$4])
AS_VAR_POPDEF([ac_Lib])
LIBS=$ac_check_lib_save_LIBS
])# AC_CHECK_LIB_PROG


# Put the lib to check at the END of the line.  Needed for -lperl
AC_DEFUN([AC_CHECK_LIB_POSTLIB],
[m4_ifval([$3], , [AH_CHECK_LIB([$1])])
AS_LITERAL_IF([$1],
	      [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1_$2])],
	      [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1''_$2])])
AC_CACHE_CHECK([for $2 in -l$1], [ac_Lib],
[ac_check_lib_save_LIBS=$LIBS
LIBS="$5 $LIBS -l$1"
AC_LINK_IFELSE([AC_LANG_CALL([], [$2])],
	       [AS_VAR_SET([ac_Lib], [yes])],
	       [AS_VAR_SET([ac_Lib], [no])])
LIBS=$ac_check_lib_save_LIBS])
AS_VAR_IF([ac_Lib], [yes],
      [m4_default([$3], 
      [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1))
       LIBS="-l$1 $LIBS"])],
      [$4])
AS_VAR_POPDEF([ac_Lib])
])# AC_CHECK_LIB_POSTLIB




# Check for C++ libraries
AC_LANG([C++])
#AC_CHECK_LIB_PROG([clucene], [_lucene_shutdown();],,[AC_MSG_ERROR(libclucene not installed)],,[#include <CLucene.h>])

# Checks for libraries.
AC_LANG([C])
AC_CHECK_LIB([pcre], [pcre_config],,[AC_MSG_ERROR(libpcre not installed)])
MYSQL_CLIENT([5.0.0])
AC_CHECK_LIB([mysqlclient], [mysql_query],,
             [AC_MSG_ERROR(libmysqlclient not intalled)],[$MYSQL_CLIENT_LIBS])

# Check for mysql having --libs_r in mysql_config
if mysql_config --libs_r > /dev/null ; then
	ac_cv_mysql_libs="--libs_r"
else
	ac_cv_mysql_libs="--libs"
fi
AC_SUBST(MYSQL_LIBS, ${ac_cv_mysql_libs})

dnl ENABLE_SWITCH(arg-name, arg_name, ARG_NAME, help_LHS, help_RHS, 
dnl		  cache_string, header_comment, default, not_default)
dnl		  library_checks)
AC_DEFUN([ENABLE_SWITCH],
[AC_ARG_ENABLE($1, AS_HELP_STRING([$4], [$5]))
if test "$enable_$2" = "$9"; then
        ac_cv_option_$2=$9
else    
        ac_cv_option_$2=$8
fi

AC_CACHE_CHECK([$6], ac_cv_option_$2)
if test "$ac_cv_option_$2" = "yes" ; then
        AC_DEFINE($3, 1, [$7])
	$10
fi

AC_SUBST($3, ${ac_cv_option_$2})
] )



ENABLE_SWITCH([plugin-dns],[plugin_dns],[PLUGIN_DNS],[--disable-plugin-dns],
              [Disable the dns plugin],
              [to enable the compilation of the dns plugin],
              [To enable the compilation of the dns plugin],
              [yes],[no],
	      [AC_CHECK_LIB([resolv], [gethostbyname],,
	                    [AC_MSG_ERROR(libresolv not installed)],[$MYSQL_CLIENT_LIBS])])

ENABLE_SWITCH([plugin-fart],[plugin_fart],[PLUGIN_FART],[--disable-plugin-fart],
              [Disable the fart plugin],
              [to enable the compilation of the fart plugin],
              [To enable the compilation of the fart plugin],
              [yes],[no])

ENABLE_SWITCH([plugin-luascript],[plugin_luascript],[PLUGIN_LUASCRIPT],
	      [--disable-plugin-luascript],[Disable the luascript plugin],
              [to enable the compilation of the luascript plugin],
              [To enable the compilation of the luascript plugin],
              [yes],[no],
              [AC_SEARCH_LIBS([lua_pcallk],[lua50 lua],,
                              [AC_MSG_ERROR(liblua not installed)])
	       AC_SEARCH_LIBS([luaL_loadfilex],[lualib50 lualib],,
                              [AC_MSG_ERROR(liblualib not installed)])])

ENABLE_SWITCH([plugin-mailbox],[plugin_mailbox],[PLUGIN_MAILBOX],
              [--disable-plugin-mailbox], [Disable the mailbox plugin],
              [to enable the compilation of the mailbox plugin],
              [To enable the compilation of the mailbox plugin],
              [yes],[no],
	      [AC_CHECK_LIB_PROLOG([c-client],[mail_status],,
		            [AC_MSG_ERROR(libc-client not installed)],,
	       [char mm_expunged, mm_diskerror, mm_lsub, mm_flags, mm_fatal, 
                     mm_nocritical, mm_notify, mm_searched, mm_status, mm_login,
	 	     mm_list, mm_critical, mm_exists, mm_log, mm_dlog;])])

ENABLE_SWITCH([plugin-perl],[plugin_perl],[PLUGIN_PERL],
	      [--disable-plugin-perl],[Disable the perl plugin],
              [to enable the compilation of the perl plugin],
              [To enable the compilation of the perl plugin],
              [yes],[no],
              [AX_WITH_PROG(PERL,perl)
               AX_PERL_EXT_FLAGS([PERLXS_CFLAGS], [PERLXS_LDFLAGS])
               AC_SUBST([PERLXS_CFLAGS])
               AC_SUBST([PERLXS_LDFLAGS])
               ac_save_LIB=$LIBS
	       AC_CHECK_LIB_POSTLIB([perl], [perl_alloc],,
	                            [AC_MSG_ERROR(libperl not installed)],
                                    [$PERLXS_LDFLAGS])
               LIBS=$ac_save_LIB])

ENABLE_SWITCH([plugin-rssfeed],[plugin_rssfeed],[PLUGIN_RSSFEED],
              [--disable-plugin-rssfeed],[Disable the rssfeed plugin],
              [to enable the compilation of the rssfeed plugin],
              [To enable the compilation of the rssfeed plugin],
              [yes],[no],
 	      [AC_CHECK_LIB([curl], [curl_escape],,
                            [AC_MSG_ERROR(libcurl not installed)])])

ENABLE_SWITCH([plugin-trac],[plugin_trac],[PLUGIN_TRAC],[--disable-plugin-trac],
              [Disable the trac plugin],
              [to enable the compilation of the trac plugin],
              [To enable the compilation of the trac plugin],
              [yes],[no],
	      [AC_CHECK_LIB([curl], [curl_escape],,
                            [AC_MSG_ERROR(libcurl not installed)])
	       AC_CHECK_LIB([apr-1], [apr_palloc],,
			    [AC_MSG_ERROR(libapr-1 not installed)])
	       AC_CHECK_LIB([aprutil-1], [apu_version],,
			    [AC_MSG_ERROR(libaprutil-1 not installed)])
	       AC_CHECK_LIB([svn_client-1], [svn_client_log],,
			    [AC_MSG_ERROR(libsvn_client-1 not installed)])
              ])

ENABLE_SWITCH([plugin-tracredir],[plugin_tracredir],[PLUGIN_TRACREDIR],
              [--disable-plugin-trac],[Disable the trac plugin],
              [to enable the compilation of the trac redirection plugin],
              [To enable the compilation of the trac redirection plugin],
              [yes],[no])

ENABLE_SWITCH([plugin-trout],[plugin_trout],[PLUGIN_TROUT],
              [--disable-plugin-trout],[Disable the trout plugin],
              [to enable the compilation of the trout plugin],
              [To enable the compilation of the trout plugin],
              [yes],[no])

ENABLE_SWITCH([plugin-url],[plugin_url],[PLUGIN_URL],[--disable-plugin-url],
              [Disable the url plugin],
              [to enable the compilation of the url plugin],
              [To enable the compilation of the url plugin],
              [yes],[no],
	      [AC_CHECK_LIB([curl], [curl_escape],,
                            [AC_MSG_ERROR(libcurl not installed)])])

ENABLE_SWITCH([plugin-svngit],[plugin_svngit],[PLUGIN_SVNGIT],
              [--disable-plugin-svngit],[Disable the SVNGIT plugin],
              [to enable the compilation of the SVNGIT plugin],
              [To enable the compilation of the SVNGIT plugin],
              [yes],[no])



AC_CONFIG_FILES([Makefile web/Makefile bot/src/Makefile 
                 bot/src/plugins/Makefile webservice/Makefile])
AC_OUTPUT
