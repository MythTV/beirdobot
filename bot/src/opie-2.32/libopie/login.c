/* login.c: The opielogin() library function.

%%% copyright-cmetz-96
This software is Copyright 1996-1998 by Craig Metz, All Rights Reserved.
The Inner Net License Version 2 applies to this software.
You should have received a copy of the license with this software. If
you didn't get a copy, you may request one from <license@inner.net>.

        History:

	Modified by cmetz for OPIE 2.31. If the OS won't tell us where
		_PATH_WTMP[X] is, try playing the SVID game, then use
		Autoconf-discovered values. Fixed gettimeofday() call
		and updwtmpx() call. Call endutxent for utmpx. Added
		DISABLE_UTMP.
        Created by cmetz for OPIE 2.3.
*/

#include "opie_cfg.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <utmp.h>

#if DOUTMPX
#include <utmpx.h>
#define pututline(x) pututxline(x)
#define endutent endutxent
#define utmp utmpx
#endif /* DOUTMPX */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#include <sys/stat.h>
#if DEBUG
#include <syslog.h>
#include <errno.h>
#endif /* DEBUG */
#include "opie.h"

int opielogin FUNCTION((line, name, host), char *line AND char *name AND char *host)
{
  struct utmp u;
  int rval = 0;

#if !DISABLE_UTMP
  if (__opiegetutmpentry(line, &u)) {
#if DEBUG
    syslog(LOG_DEBUG, "opielogin: __opiegetutmpentry(line=%s, &u) failed", line);
#endif /* DEBUG */
    memset(&u, 0, sizeof(struct utmp));
    if (!strncmp(line, "/dev/", 5))
      strncpy(u.ut_line, line + 5, sizeof(u.ut_line));
    else
      strncpy(u.ut_line, line, sizeof(u.ut_line));
#if DEBUG
    syslog(LOG_DEBUG, "opielogin: continuing with ut_line=%s", u.ut_line);
#endif /* DEBUG */
  }

#if HAVE_UT_TYPE && defined(USER_PROCESS)
  u.ut_type = USER_PROCESS;
#endif /* HAVE_UT_TYPE && defined(USER_PROCESS) */
#if HAVE_UT_PID
  u.ut_pid = getpid();
#endif /* HAVE_UT_PID */

#if HAVE_UT_NAME
  strncpy(u.ut_name, name, sizeof(u.ut_name));
  u.ut_name[sizeof(u.ut_name)-1] = 0;
#else /* HAVE_UT_NAME */
#error No ut_name field in struct utmp? (Please send in a bug report)
#endif /* HAVE_UT_NAME */

#if HAVE_UT_HOST
  strncpy(u.ut_host, host, sizeof(u.ut_host));
  u.ut_host[sizeof(u.ut_host)-1] = 0;
#endif /* HAVE_UT_HOST */

#if DOUTMPX
#ifdef HAVE_ONE_ARG_GETTIMEOFDAY
  gettimeofday(&u.ut_tv);
#else /* HAVE_ONE_ARG_GETTIMEOFDAY */
#if __WORDSIZE_TIME64_COMPAT32
  struct timeval tv;
  gettimeofday(&tv, NULL);
  u.ut_tv.tv_sec = tv.tv_sec;
  u.ut_tv.tv_usec = tv.tv_usec;
#else
  gettimeofday(&u.ut_tv, NULL);
#endif
#endif /* HAVE_ONE_ARG_GETTIMEOFDAY */
#else /* DOUTMPX */
  time(&u.ut_time);
#endif /* DOUTMPX */

  pututline(&u);
  endutent();

#if DEBUG
  syslog(LOG_DEBUG, "opielogin: utmp suceeded");
#endif /* DEBUG */
#endif /* !DISABLE_UTMP */

dowtmp:
  opielogwtmp(line, name, host);
  opielogwtmp(NULL, NULL, NULL);

dosetlogin:
#if HAVE_SETLOGIN
  setlogin(name);
#endif /* HAVE_SETLOGIN */

#if DEBUG
  syslog(LOG_DEBUG, "opielogin: rval=%d", rval);
#endif /* DEBUG */

  return rval;
}
