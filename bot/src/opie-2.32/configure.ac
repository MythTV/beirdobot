dnl configure.in: Input for Autoconf
dnl
dnl %%% portions-copyright-cmetz-96
dnl Portions of this software are Copyright 1996-1998 by Craig Metz, All Rights
dnl Reserved. The Inner Net License Version 2 applies to these portions of
dnl the software.
dnl You should have received a copy of the license with this software. If
dnl you didn't get a copy, you may request one from <license@inner.net>.
dnl
dnl Portions of this software are Copyright 1995 by Randall Atkinson and Dan
dnl McDonald, All Rights Reserved. All Rights under this copyright are assigned
dnl to the U.S. Naval Research Laboratory (NRL). The NRL Copyright Notice and
dnl License Agreement applies to this software.   
dnl
dnl	History:
dnl
dnl	Modified by cmetz for OPIE 2.32. Substitute default for LOCK_DIR.
dnl	     Fix the --disable-user-locking bug. AC_DEFINE variables to 1.
dnl          Really check for ut_host.
dnl	Modified by cmetz for OPIE 2.31. Put back manual utmp[x]/wtmp[x]
dnl          checks -- too many OSs can't be trusted to tell us where they are.
dnl	     Check for sys/select.h. Spell endutent right. Replace strtoul()
dnl	     if needed. Removed duplicate check for sysconf. Added check for
dnl          SunOS C2 shadow passwords (may need more work). Replace
dnl          setutent. Added options to disable use of utmp/wtmp. Replace
dnl          seteuid and setegid. Check for usleep. Moved options.h options
dnl          here as enable/disable options.
dnl     Modified by cmetz for OPIE 2.3. Removed redundant memset/memcpy.
dnl	     Changed ls -g test around. Changed logindevperm/fbtab defines.
dnl	     Added check for /etc/environment and /etc/src.sh. Check for
dnl          /var/adm/sulog. Check for {get,put}utxline, provide libmissing
dnl          versionf of {get,put}utline. Added --enable option for anonymous
dnl          FTP. Got rid of a few unneeded checks. Check for functions only
dnl	     used by libmissing only if the replacement function that needs
dnl	     them is itself needed.
dnl	Modified by cmetz for OPIE 2.22. Check for Solaris drain bamaged ls.
dnl          Check for setlogin(). Removed duplicate checks for some funcs.
dnl     Modified by cmetz for OPIE 2.21. Filename must be in utmp[x]/wtmp[x]
dnl          defines.
dnl     Modified by cmetz for OPIE 2.2. Misc changes. Changed for libmissing
dnl          support and building its target object list. Changed to support
dnl          FUNCTION declaration et al. Added a LOT of checks and a LOT of
dnl          fixes.
dnl	Created at NRL for OPIE 2.1.

AC_INIT
AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([config.h])
AC_ARG_ENABLE(access-file, [  --enable-access-file=FILENAME
                          Enable the OPIE access file FILENAME], AC_DEFINE_UNQUOTED(PATH_ACCESS_FILE, "$enable_access_file", [The path to the access file, if we're going to use it]) echo "Using the access file in $enable_access_file -- don't say we didn't warn you!")
ACCESS_FILE="$enable_access_file"
AC_SUBST(ACCESS_FILE)
AC_ARG_ENABLE(server-md4, [  --enable-server-md4     Use MD4 instead of MD5 for the server], AC_DEFINE(MDX, 4, [Defined to the default hash value, always defined]), AC_DEFINE(MDX, 5, [Defined to the default hash value, always defined]))

AC_ARG_ENABLE(user-locking, [  --disable-user-locking  Disable user locking
  --enable-user-locking[=DIR]
                          Put user lock files in DIR [/etc/opielocks]],,)
if test "$enable_user_locking" != no;
then
  if test -z "$enable_user_locking"
  then
    AC_DEFINE(OPIE_LOCK_DIR, "/etc/opielocks", [Defined to the path of the OPIE lock directory])
    LOCK_DIR="/etc/opielocks"
  else
    AC_DEFINE_UNQUOTED(OPIE_LOCK_DIR, "$enable_user_locking")
    LOCK_DIR="$enable_user_locking"
  fi
fi
AC_SUBST(LOCK_DIR)

AC_ARG_ENABLE(retype, [  --enable-retype         Ask users to re-type their secret pass phrases], AC_DEFINE(RETYPE, 1, [Defined if users are to be asked to re-type secret pass phrases]))
AC_ARG_ENABLE(su-star-check, [  --enable-su-star-check  Refuse to switch to disabled accounts], AC_DEFINE(SU_STAR_CHECK, 1, [Defined if su should not switch to disabled accounts]))
AC_ARG_ENABLE(new-prompts, [  --disable-new-prompts   Use more compatible (but less informative) prompts],, AC_DEFINE(NEW_PROMPTS, 1, [Defined if new-style prompts are to be used]))
AC_ARG_ENABLE(insecure-override, [  --enable-insecure-override
                          Allow users to override insecure checks], AC_DEFINE(INSECURE_OVERRIDE, 1, [Defined if you want to allow users to override the insecure checks]))
AC_ARG_ENABLE(anonymous-ftp, [  --enable-anonymous-ftp  Enable anonymous FTP support], AC_DEFINE(DOANONYMOUS, 1, [Define if you want the FTP daemon to support anonymous logins.]) echo "Enabling anonymous FTP support in ftp -- don't say we didn't warn you!")
AC_ARG_ENABLE(utmp, [  --disable-utmp          Disable utmp logging], AC_DEFINE(DISABLE_UTMP, 1, [Defined if you want to disable utmp support]) echo "Disabling utmp logging")
AC_ARG_ENABLE(wtmp, [  --disable-wtmp          Disable wtmp logging], AC_DEFINE(DISABLE_WTMP, 1, [Defined if you want to disable wtmp support]) echo "Disabling wtmp logging")

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_YACC

AC_USE_SYSTEM_EXTENSIONS
AC_SEARCH_LIBS([strerror],[cposix])
AC_USE_SYSTEM_EXTENSIONS

dnl We'd put PATH in these checks, but it turns out that autoconf doesn't
dnl work as documented when it comes to the colon separator...

AC_PATH_PROG(CHOWN, chown, /bin/chown, /usr/bin /bin /usr/sbin /sbin /usr/etc /etc)

AC_PATH_PROG(SU, su, /bin/su, /usr/bin /bin)
AC_PATH_PROG(ALT_SU, su,, /usr/sbin /sbin)

AC_PATH_PROG(SCHEME, scheme,, /usr/lib/iaf/scheme)
AC_PATH_PROG(LOGIN, login, /bin/login, /usr/bin /bin)
dnl AC_DEFINE_UNQUOTED(PATH_LOGIN, "$LOGIN")

if test ! -z "$SCHEME";
then
	LOGIN="$SCHEME";
fi

AC_PATH_PROG(FTPD, ftpd,, /usr/libexec /usr/etc /etc /usr/sbin /sbin /usr/lbin)
AC_PATH_PROG(INFTPD, in.ftpd,, /usr/libexec /usr/etc /etc /usr/sbin /sbin /usr/lbin)

if test -z "$FTPD"
then
	if test ! -z "$INFTPD"
	then
		FTPD="$INFTPD"
	fi
fi

AC_MSG_CHECKING(for default PATH entries)
default_path=""
save_IFS="$IFS"
IFS=" "
for i in /usr/bin /bin /usr/ucb /usr/sbin /usr/bsd /sbin /usr/bin/X11 /etc /usr/local/X11/bin /usr/X11R6/bin /your-system-is-broken
do
	IFS=":"
	for j in $PATH
	do
		if test "$i" = "$j"
		then
			if test -d "$i"
			then
				if test -z "$default_path"
				then
					default_path="$i"
				else
					default_path="$default_path:$i"
				fi
			fi
		fi
	done
	IFS=" "
done
IFS="$save_IFS"
AC_DEFINE_UNQUOTED(DEFAULT_PATH, "$default_path", [The default value of the PATH environment variable])
AC_MSG_RESULT($default_path)

AC_MSG_CHECKING(for test -e flag)
if sh config.testeflag
then
	result=yes
	EXISTS="-e"
else
	result=no
	EXISTS="-f"
fi
AC_SUBST(EXISTS)
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for mkdir -p flag)
if test -d config.tmpdir
then
	rmdir config.tmpdir/foo/bar >/dev/null 2>/dev/null
	rmdir config.tmpdir/foo >/dev/null 2>/dev/null
	rmdir config.tmpdir >/dev/null 2>/dev/null
fi

result=no
if mkdir -p config.tmpdir/foo/bar >/dev/null 2>/dev/null
then
	if test -d config.tmpdir
	then
		if test -d config.tmpdir/foo
		then
			if test -d config.tmpdir/foo/bar
			then
				result=yes
				rmdir config.tmpdir/foo/bar >/dev/null 2>/dev/null
			fi
			rmdir config.tmpdir/foo >/dev/null 2>/dev/null
		fi
		rmdir config.tmpdir >/dev/null 2>/dev/null
	fi
fi

if test "$result" = yes
then
	MKDIR="mkdir -p"
else
	MKDIR="mkdir"
fi
AC_SUBST(MKDIR)
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for ls group field)
lsg=`/bin/ls -ldg / | wc -w | awk '{print $1}'`;
ls=`/bin/ls -ld / | wc -w | awk '{print $1}'`;
result="no"
if test $ls = 9;
then
	result="yes"
else
	if test "$ls" = 8 -a "$lsg" = 9;
	then
		result="yes, with -g"
		AC_DEFINE(HAVE_LS_G_FLAG, 1, [Define if the /bin/ls command seems to support the -g flag])
	fi
fi
AC_MSG_RESULT($result)

dnl Checks for various system characteristics
AC_MSG_CHECKING(for /etc/default/login)
if test $EXISTS /etc/default/login
then
	result=yes
	AC_DEFINE(HAVE_ETC_DEFAULT_LOGIN, 1, [Defined if the file /etc/default/login exists (and, presumably, should be looked at by login)])
else
	result=no
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for /etc/securetty)
if test $EXISTS /etc/securetty
then
	result=yes
	AC_DEFINE(HAVE_SECURETTY, 1, [Defined if the file /etc/securetty exists (and, presumably, should be looked at by login)])
else
	result=no
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for /etc/logindevperm)
if test $EXISTS /etc/logindevperm
then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_LOGIN_PERMFILE, "/etc/logindevperm", [Defined to the name of a file that contains a list of files whose permissions and ownerships should be changed on login.])
else
	AC_MSG_RESULT(no)

	AC_MSG_CHECKING(for /etc/fbtab)
	if test $EXISTS /etc/fbtab
	then
		result=yes
		AC_DEFINE(HAVE_LOGIN_PERMFILE, "/etc/fbtab", [Defined to the name of a file that contains a list of files whose permissions and ownerships should be changed on login.])
	else
		result=no
	fi
	AC_MSG_RESULT($result)
fi

AC_MSG_CHECKING(for /etc/environment)
if test $EXISTS /etc/environment
then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_LOGIN_ENVFILE, "/etc/environment", [Defined to the name of a file that contains a list of environment values that should be set on login.])
else
	AC_MSG_RESULT(no)

	AC_MSG_CHECKING(for /etc/src.sh)
	if test $EXISTS /etc/src.sh
	then
		result=yes
		AC_DEFINE(HAVE_LOGIN_ENVFILE, "/etc/src.sh", [Defined to the name of a file that contains a list of environment values that should be set on login.])
	else
		result=no
	fi
	AC_MSG_RESULT($result)
fi

AC_MSG_CHECKING(for /etc/shadow)
if test $EXISTS /etc/shadow
then
	result=yes
	AC_DEFINE(HAVE_ETC_SHADOW, 1, [Defined if the file /etc/shadow exists (and, presumably, should be looked at for shadow passwords)])
else
	AC_MSG_RESULT(no)

	AC_MSG_CHECKING(for /etc/security/passwd.adjunct)
	if test $EXISTS /etc/security/passwd.adjunct
	then
		result=yes
		AC_DEFINE(HAVE_SUNOS_C2_SHADOW, 1, [Defined if the system has SunOS C2 security shadow passwords])
		LIBOBJS="$LIBOBJS getspnam.o endspent.o"
	else
		result=no
	fi
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(for /var/adm/sulog)
if test $EXISTS /var/adm/sulog
then
	result=yes
	AC_DEFINE(HAVE_SULOG, 1, [Define if the system has the /var/adm/sulog file])
else
	result=no
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(mail spool location)
mail_spool=""
for i in /var/mail /usr/mail /var/spool/mail /usr/spool/mail
do
	if test -d $i
	then
		mail_spool="$i"
	fi
done
if test -z "$mail_spool"
then
	result="not found"
else
	result="$mail_spool"
	AC_DEFINE_UNQUOTED(PATH_MAIL, "$mail_spool", [The path to the mail spool, if we know it])
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(where your system puts the utmp file)
utmp_path=""
for i in /var/run /var/adm /usr/adm /etc
do
	if test $EXISTS $i/utmp
	then
		utmp_path="$i"
	fi
done
if test -z "$utmp_path"
then
	result="not found"
else
	result="$utmp_path"
	AC_DEFINE_UNQUOTED(PATH_UTMP_AC, "$utmp_path/utmp", [The path to the utmp file, if we know it])
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(where your system puts the utmpx file)
utmp_path=""
for i in /var/run /var/adm /usr/adm /etc
do
	if test $EXISTS $i/utmp
	then
		utmp_path="$i"
	fi
done
if test -z "$utmp_path"
then
	result="not found"
	AC_DEFINE_UNQUOTED(PATH_UTMP_AC, "$utmp_path/utmpx", [The path to the utmpx file, if we know it])
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(where your system puts the wtmp file)
wtmp_path=""
for i in /var/run /var/log /var/adm /usr/adm /etc
do
	if test $EXISTS $i/wtmp
	then
		wtmp_path="$i"
	fi
done
if test -z "$wtmp_path"
then
	result="not found"
else
	result="$wtmp_path"
	AC_DEFINE_UNQUOTED(PATH_WTMP_AC, "$wtmp_path/wtmp", [The path to the wtmp file, if we know it])
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(where your system puts the wtmpx file)
wtmpx_path=""
for i in /var/run /var/log /var/adm /usr/adm /etc
do
	if test $EXISTS $i/wtmpx
	then
		wtmpx_path="$i"
	fi
done
if test -z "$wtmpx_path"
then
	result="not found"
else
	result="$wtmpx_path"
	AC_DEFINE_UNQUOTED(PATH_WTMPX_AC, "$wtmpx_path/wtmpx", [The path to the wtmpx file, if we know it])
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(whether the system profile displays the motd)
result=no
if test $EXISTS /etc/profile
then
	if grep motd /etc/profile >/dev/null 2>/dev/null
	then
		result=yes
	fi
fi
if test "$result" = yes
then
	AC_DEFINE(HAVE_MOTD_IN_PROFILE, 1, [Defined if the system's profile (/etc/profile) displays the motd file])
fi
AC_MSG_RESULT($result)

AC_MSG_CHECKING(whether the system profile checks for mail)
result=no
if test $EXISTS /etc/profile
then
	if grep 'mail\.' /etc/profile >/dev/null 2>/dev/null
	then
		result=yes
	fi
fi
if test "$result" = yes
then
	AC_DEFINE(HAVE_MAILCHECK_IN_PROFILE, 1, [Defined if the system's profile (/etc/profile) informs the user of new mail])
fi
AC_MSG_RESULT($result)

dnl Random checks
AC_C_CONST

AC_MSG_CHECKING(to see if your compiler can handle void arguments)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[foo(void) { }]], [[]])],[AC_DEFINE(HAVE_VOIDARG, 1, [Define if the compiler can handle void argument lists to functions]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(to see if your compiler can handle void return values)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[void foo() { }]], [[]])],[AC_DEFINE(HAVE_VOIDRET, 1, [Define if the compiler can handle void return "values" from functions]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(to see if your compiler can handle void pointers)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[foo() { void *bar = (void *)0x42; bar = bar + 1; }]], [[]])],[AC_DEFINE(HAVE_VOIDPTR, 1, [Define if the compiler can handle void pointers to our liking]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(to see if your compiler can handle ANSI argument lists)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[int foo(int bar, int baz) { return 0; }]], [[]])],[AC_DEFINE(HAVE_ANSIDECL, 1, [Define if the compiler can handle ANSI-style argument lists]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(to see if your compiler can handle ANSI prototypes)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[extern int foo(int, int);]], [[]])],[AC_DEFINE(HAVE_ANSIPROTO, 1, [Define if the compiler can handle ANSI-style prototypes]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(to see if your compiler can handle ANSI variable arguments)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdarg.h>
int foo(int arg, ...) {
	va_list ap;
	va_start(ap, arg);
	va_end(ap);
	return 0;
}]], [[]])],[AC_DEFINE(HAVE_ANSISTDARG, 1, [Define if the compiler can handle ANSI-style variable argument lists]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(to see if you have an ANSI-style sprintf)
AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
int main(argc, argv)
int argc;
char *argv[];
{
  char buf[5];
  int i = 2;
  i += sprintf(buf, "1234");
  return (i == 6) ? 0 : -1;
}]])],[AC_DEFINE(HAVE_ANSISPRINTF, 1, [Define if the system has an ANSI-style printf (returns int instead of char *)]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)],[AC_MSG_RESULT(no)])

dnl Checks for libraries.
AC_CHECK_LIB(crypt, crypt)
AC_CHECK_LIB(nsl, gethostname)
AC_CHECK_LIB(posix, main)
AC_CHECK_LIB(socket, socket)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(crypt.h fcntl.h limits.h termios.h sys/file.h sys/ioctl.h sys/time.h syslog.h unistd.h paths.h shadow.h signal.h sys/signal.h lastlog.h sys/utsname.h pwd.h sys/param.h string.h stdlib.h utmpx.h sys/select.h)

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_TYPE_UID_T
dnl AC_TYPE_OFF_T
dnl AC_TYPE_PID_T
AC_CHECK_MEMBERS([struct stat.st_blksize])

dnl AC_STRUCT_TM

AC_MSG_CHECKING(for ut_pid in struct utmp)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp foo; return (int)foo.ut_pid;]])],[AC_DEFINE(HAVE_UT_PID,1,[Define if there is a ut_pid field in struct utmp]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])
AC_MSG_CHECKING(for ut_type in struct utmp)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp foo; return (int)foo.ut_type;]])],[AC_DEFINE(HAVE_UT_TYPE,1,[Define if there is a ut_type field in struct utmp]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])
AC_MSG_CHECKING(for ut_name in struct utmp)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp foo; return (int)foo.ut_name[0];]])],[AC_DEFINE(HAVE_UT_NAME,1,[Define if there is a ut_name field in struct utmp]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])
dnl AC_MSG_CHECKING(for ut_user in struct utmp)
dnl AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
dnl #include <utmp.h>]], [[struct utmp foo; return (int)foo.ut_user[0];]])],[AC_DEFINE(HAVE_UT_USER,1,[Define if there is a ut_user field in struct utmp]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])
AC_MSG_CHECKING(for ut_host in struct utmp)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp foo; return (int)foo.ut_host[0];]])],[AC_DEFINE(HAVE_UT_HOST,1,[Define if there is a ut_host field in struct utmp]) AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

dnl Checks for library functions.
dnl AC_PROG_GCC_TRADITIONAL

AC_CHECK_FUNCS(gettimeofday socket strftime strstr setpriority getttynam setvbuf getspnam endspent setgroups getgroups fpurge setlogin lstat getutxline pututxline usleep)

dnl Libmissing...
AC_FUNC_MEMCMP
AC_FUNC_ALLOCA
AC_REPLACE_FUNCS(getusershell sigaddset sigemptyset strerror strncasecmp)
MISSING="$LIBOBJS $ALLOCA " ;
AC_CHECK_FUNC(atexit,, MISSING="${MISSING}atexit.o "; AC_CHECK_FUNCS(on_exit))
AC_CHECK_FUNC(endutent,, MISSING="${MISSING}endutent.o ")
AC_CHECK_FUNC(memcpy,, MISSING="${MISSING}memcpy.o "; AC_CHECK_FUNCS(bcopy))
AC_CHECK_FUNC(memset,, MISSING="${MISSING}memset.o "; AC_CHECK_FUNCS(bzero))
AC_CHECK_FUNC(getcwd,, MISSING="${MISSING}getcwd.o "; AC_CHECK_FUNCS(getwd))
AC_CHECK_FUNC(getenv, AC_DEFINE(HAVE_GETENV, 1, [Define if the system has the getenv function]), MISSING="${MISSING}env.o ")
AC_CHECK_FUNC(getutline, AC_DEFINE(HAVE_GETUTLINE, 1, [Define if the system has getutline()]), MISSING="${MISSING}getutline.o "; AC_CHECK_FUNCS(ttyslot))
AC_CHECK_FUNC(pututline,, MISSING="${MISSING}pututline.o "; AC_CHECK_FUNCS(ttyslot))
AC_CHECK_FUNC(setenv, AC_DEFINE(HAVE_SETENV, 1, [Define if the system has the setenv function]), MISSING="${MISSING}env.o ")
AC_CHECK_FUNC(setegid,, MISSING="${MISSING}setegid.o "; AC_CHECK_FUNCS(setregid setresgid))
AC_CHECK_FUNC(seteuid,, MISSING="${MISSING}seteuid.o "; AC_CHECK_FUNCS(setreuid setresuid))
AC_CHECK_FUNC(setutent,, MISSING="${MISSING}setutent.o ")
AC_CHECK_FUNC(sigprocmask,, MISSING="${MISSING}sigprocmask.o "; AC_CHECK_FUNCS(sigblock sigsetmask))
AC_CHECK_FUNC(strchr,, MISSING="${MISSING}strchr.o "; AC_CHECK_FUNCS(index))
AC_CHECK_FUNC(strrchr,, MISSING="${MISSING}strrchr.o "; AC_CHECK_FUNCS(rindex))
AC_CHECK_FUNC(strtoul,, MISSING="${MISSING}strtoul.o ")
AC_CHECK_FUNC(sysconf,, MISSING="${MISSING}sysconf.o "; AC_CHECK_FUNCS(getdtablesize))
AC_CHECK_FUNC(uname,, MISSING="${MISSING}uname.o "; AC_CHECK_FUNCS(gethostname))
AC_CHECK_FUNC(unsetenv, AC_DEFINE(HAVE_UNSETENV, 1, [Define if the system has the unsetenv function]), MISSING="${MISSING}env.o ")
AC_SUBST(MISSING)

AC_MSG_CHECKING(for nonstandard gettimeofday)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
]], [[struct timeval tv; gettimeofday(&tv, NULL)]])],
[AC_MSG_RESULT(no)],
[AC_MSG_RESULT(maybe) ; AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #if HAVE_SYS_TIME_H
    #include <sys/time.h>
    #endif /* HAVE_SYS_TIME_H */
    #if HAVE_UNISTD_H
    #include <unistd.h>
    #endif /* HAVE_UNISTD_H */
  ]],
  [[struct timeval tv; gettimeofday(&tv)]])],
  [AC_DEFINE(HAVE_ONE_ARG_GETTIMEOFDAY, 1, [Define if you have a nonstandard gettimeofday() that takes one argument instead of two.]) AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])
])

# Munge out LOCALBIN and LOCALMAN in canonical (no bletch) form
AC_CONFIG_FILES([configure.munger libmissing/Makefile libopie/Makefile Makefile.munge:Makefile.in])
AC_OUTPUT
sh configure.munger
