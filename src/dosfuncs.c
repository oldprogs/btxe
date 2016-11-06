/*---------------------------------------------------------------------------*
 *
 *                               BinkleyTerm
 *
 *              (C) Copyright 1987-96, Bit Bucket Software Co.
 *     For license and contact information see /doc/orig_260/license.260.
 *
 *           This version was modified by the BinkleyTerm XE Team.
 *        For contact information see /doc/team.lst and /doc/join.us.
 *  For a complete list of changes see /doc/xe_user.doc and /doc/xe_hist.doc.
 *
 * Filename    : $Source: E:/cvs/btxe/src/dosfuncs.c,v $
 * Revision    : $Revision: 1.11 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/05/04 20:20:35 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello, OS/2 code: Bill Andrus
 *
 * Description : OS kernel routines used by BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#ifdef DOS16                    /* Here's the DOS stuff */

void
dostime (int *wday, int *hour, int *min, int *sec, int *hdths)
{
  union REGS r;

  r.h.ah = 0x2c;
  intdos (&r, &r);
  *hour = r.h.ch;
  *min = r.h.cl;
  *sec = r.h.dh;
  *hdths = r.h.dl;

  r.h.ah = 0x2a;
  intdos (&r, &r);
  *wday = (int) r.h.al;         /* r. hoerner. 0=sun,1=mon etc. */
}

void
dos_break_off (void)
{
  union REGS r;

  r.x.ax = 0x3301;
  r.h.dl = 0x00;

  intdos (&r, &r);
}

void
mtask_find ()
{
  for (;;)
  {
    if ((have_dv = dv_get_version ()) != 0)
    {
      mtask_idle = dv_pause;
      break;
    }

    if (_osmajor >= 10)
    {
      mtask_idle = windows_pause;  /* OS/2 weenies will love this! */
      break;
    }

    if ((have_windows = windows_active ()) != 0)
    {
      if (winslice)
        mtask_idle = windows_pause;
      else
        mtask_idle = msdos_pause;
      break;
    }
    /* else */
    {
      mtask_idle = msdos_pause;
      break;
    }
  }
}

int
set_prior (int pclass)
{
  return (pclass);
}

#endif
#ifdef OS_2                     /* here's the OS/2 stuff */

void
dostime (int *wday, int *hour, int *min, int *sec, int *hdths)
{
  DATETIME dt;

  DosGetDateTime (&dt);
  *wday = dt.weekday;           /* current day of week */
  *hour = dt.hours;             /* current hour */
  *min = dt.minutes;            /* current minute */
  *sec = dt.seconds;            /* current second */
  *hdths = dt.hundredths;       /* current hundredths of a second */
}

void _cdecl
os2_pause (void)
{
  DosSleep (1L);
}

void
mtask_find ()
{
  mtask_idle = os2_pause;
}

#ifdef NEED_WRITE_ANSI
void
WRITE_ANSI (char c)
{
  static char s[] =
  {'\0', '\0'};

  *s = c;

  VioWrtTTY (s, 1, (HVIO) 0L);
}

#endif

#ifdef NEED_SET_PRIOR
int
envgetprio (int deltaflag, char *envvarname, int defvalue)  /* TJW 960505 */
{
  char *s;
  int class, delta;

  s = getenv (envvarname);
  if (s)
  {
    switch (toupper (*s))
    {
    case 'I':                  /* Idle */
      class = 1;

      delta = 31;
      break;
    case 'R':                  /* Regular */
      class = 2;

      delta = 31;
      break;
    case 'F':                  /* FgServer */
      class = 4;

      delta = 31;
      break;
    case 'T':                  /* TimeCritical */
      class = 3;

      delta = 31;
      break;
    default:                   /* Invalid */
      status_line (MSG_TXT (M_INVALID_PRIO_ENVVAR), envvarname, s);
      class = 2;

      delta = 31;               /* choose Regular in this case */
      break;
    }
    s++;
    if (*s)
      delta = atoi (s);
    if (delta > 31)
      delta = 31;
    if (delta < 0)
      delta = 0;                /* or -31 ?? */
  }
  else
  {
    class = defvalue;

    delta = defvalue;
  }
  return deltaflag ? delta : class;
}
#endif

int
set_prior (int pclass)          /* TJW 960505 - enhanced version */
{
#ifdef NEED_SET_PRIOR
  static int regular_class = 0;
  static int regular_delta = 0;
  static int modem_class = 0;
  static int modem_delta = 0;
  static int janus_class = 0;
  static int janus_delta = 0;
  static int hydra_class = 0;
  static int hydra_delta = 0;
  static int class = 0;
  int priority_class, priority_delta, ret;

  ret = class;
  class = pclass;

  switch (pclass)
  {
  case PRIO_NORMAL:
    priority_class = 2;
    priority_delta = 0;
    status_line (">Switching to priority: normal (%d:%d)",
                 priority_class, priority_delta);
    break;

  case PRIO_REGULAR:
    if (regular_class)
    {
      priority_class = regular_class;
      priority_delta = regular_delta;
    }
    else
    {
      priority_class = regular_class = envgetprio (0, "REGULARPRIORITY", 2);
      priority_delta = regular_delta = envgetprio (1, "REGULARPRIORITY", 31);
    }
    status_line (">Switching to priority: regular (%d:%d)",
                 priority_class, priority_delta);
    break;

  case PRIO_JANUS:
    if (janus_class)
    {
      priority_class = janus_class;
      priority_delta = janus_delta;
    }
    else
    {
      priority_class = janus_class = envgetprio (0, "JANUSPRIORITY", 4);
      priority_delta = janus_delta = envgetprio (1, "JANUSPRIORITY", 31);
    }
    status_line (">Switching to priority: janus (%d:%d)",
                 priority_class, priority_delta);
    break;

  case PRIO_HYDRA:
    if (hydra_class)
    {
      priority_class = hydra_class;
      priority_delta = hydra_delta;
    }
    else
    {
      priority_class = hydra_class = envgetprio (0, "HYDRAPRIORITY", 4);
      priority_delta = hydra_delta = envgetprio (1, "HYDRAPRIORITY", 31);
    }
    status_line (">Switching to priority: hydra (%d:%d)",
                 priority_class, priority_delta);
    break;

  case PRIO_MODEM:
    if (modem_class)
    {
      priority_class = modem_class;
      priority_delta = modem_delta;
    }
    else
    {
      priority_class = modem_class = envgetprio (0, "MODEMPRIORITY", 4);
      priority_delta = modem_delta = envgetprio (1, "MODEMPRIORITY", 15);
    }
    status_line (">Switching to priority: modem (%d:%d)",
                 priority_class, priority_delta);
    break;

  default:
    priority_class = 2;
    priority_delta = 31;
    break;
  }

  DosSetPrty ((USHORT) 1, (USHORT) priority_class, (USHORT) priority_delta,
              (USHORT) 0);

  return ret;
#else
  return pclass;
#endif
}
#endif /* OS_2 */


#ifdef __unix__

int
set_prior (int p)
{
  return (p);
}

void
mtask_find (void)
{
  mtask_idle = linux_pause;
}

void
dostime (int *wday, int *hour, int *min, int *sec, int *hdths)
{
  time_t t;
  struct timeval tv;
  struct timezone tz;
  struct tm *tim;

  tz.tz_minuteswest = 0;        /* does not matter, we only need the microseconds */
  tz.tz_dsttime = 0;
  gettimeofday (&tv, &tz);
  t = time (NULL);
  tim = localtime (&t);

  *wday = tim->tm_wday;         /* current day of week */
  *hour = tim->tm_hour;         /* current hour */
  *min = tim->tm_min;           /* current minute */
  *sec = tim->tm_sec;           /* current second */
  *hdths = tv.tv_usec / 10000;  /* current hundredths of a second */
}

#endif


#ifdef _WIN32

void
dostime (int *wday, int *hour, int *min, int *sec, int *hdths)
{
  SYSTEMTIME st;

  GetLocalTime (&st);
  *wday = st.wDayOfWeek;
  *hdths = st.wMilliseconds / 10;
  *hour = st.wHour;
  *min = st.wMinute;
  *sec = st.wSecond;
}

void _cdecl
Win32_pause (void)
{
  Sleep (50L);
}

void
mtask_find ()
{
  mtask_idle = Win32_pause;
}

#ifdef NEED_SET_PRIOR
// HJK 98/03/12 - Win32 implementation
int
envgetprio (int pflag, char *envvarname, int defvalue)
{
  char *s;
  int pclass, pthread;

  s = getenv (envvarname);
  if (s)
  {
    switch (toupper (*s))
    {
    case 'I':                  /* Idle - BasePriority : 9 */
      pclass = NORMAL_PRIORITY_CLASS;  /* 32 */
      pthread = THREAD_PRIORITY_NORMAL;  /* 0 */
      break;
    case 'R':                  /* Regular - BasePriority : 11 */
      pclass = NORMAL_PRIORITY_CLASS;  /* 64 */
      pthread = THREAD_PRIORITY_ABOVE_NORMAL;  /* 1 */
      break;
    case 'F':                  /* FgServer - BasePriority : 15 */
      pclass = HIGH_PRIORITY_CLASS;  /* 128 */
      pthread = THREAD_PRIORITY_HIGHEST;  /* 2 */
      break;
    case 'T':                  /* TimeCritical - BasePriority : 24 */
      pclass = REALTIME_PRIORITY_CLASS;  /* 256 */
      pthread = THREAD_PRIORITY_NORMAL;  /* 15 */
      break;
    default:                   /* Invalid */
      status_line (MSG_TXT (M_INVALID_PRIO_ENVVAR), envvarname, s);
      pclass = NORMAL_PRIORITY_CLASS;
      pthread = THREAD_PRIORITY_NORMAL;
      break;
    }
    s++;
    if (*s)
      pthread = atoi (s);
    if (pclass > 31)
      pclass = 31;
    if (pclass < 1)
      pclass = 1;
    pthread -= 16;              /* pclass must be between -15 and 15 */
  }
  else
  {
    pclass = defvalue;
    pthread = defvalue;
  }

  return pflag ? pthread : pclass;
}

#endif

int
set_prior (int pclass)
{
#ifdef NEED_SET_PRIOR
  static int regular_thread = 0;
  static int regular_class = 0;
  static int modem_thread = 0;
  static int modem_class = 0;
  static int janus_thread = 0;
  static int janus_class = 0;
  static int hydra_thread = 0;
  static int hydra_class = 0;
  int priority_class, priority_thread, ret;

  ret = pclass;
  switch (pclass)
  {
  case PRIO_NORMAL:
    priority_class = IDLE_PRIORITY_CLASS;
    priority_thread = THREAD_PRIORITY_NORMAL;
    status_line (">Switching to priority: normal (%d:%d)",
                 priority_thread, priority_class);
    break;

  case PRIO_REGULAR:
    if (regular_class)
    {
      priority_class = regular_class;
      priority_thread = regular_thread;
    }
    else
    {
      priority_class = regular_class = envgetprio (0, "REGULARPRIORITY", NORMAL_PRIORITY_CLASS);
      priority_thread = regular_thread = envgetprio (1, "REGULARPRIORITY", THREAD_PRIORITY_NORMAL);
    }
    status_line (">Switching to priority: regular (%d:%d)",
                 priority_thread, priority_class);
    break;

  case PRIO_JANUS:
    if (janus_class)
    {
      priority_class = janus_class;
      priority_thread = janus_thread;
    }
    else
    {
      priority_class = janus_class = envgetprio (0, "JANUSPRIORITY", NORMAL_PRIORITY_CLASS);
      priority_thread = janus_thread = envgetprio (1, "JANUSPRIORITY", THREAD_PRIORITY_NORMAL);
    }
    status_line (">Switching to priority: janus (%d:%d)",
                 priority_thread, priority_class);
    break;

  case PRIO_HYDRA:
    if (hydra_class)
    {
      priority_class = hydra_class;
      priority_thread = hydra_thread;
    }
    else
    {
      priority_class = hydra_class = envgetprio (0, "HYDRAPRIORITY", NORMAL_PRIORITY_CLASS);
      priority_thread = hydra_thread = envgetprio (1, "HYDRAPRIORITY", THREAD_PRIORITY_NORMAL);
    }
    status_line (">Switching to priority: hydra (%d:%d)",
                 priority_thread, priority_class);
    break;

  case PRIO_MODEM:
    if (modem_class)
    {
      priority_class = modem_class;
      priority_thread = modem_thread;
    }
    else
    {
      priority_class = modem_class = envgetprio (0, "MODEMPRIORITY", NORMAL_PRIORITY_CLASS);
      priority_thread = modem_thread = envgetprio (1, "MODEMPRIORITY", THREAD_PRIORITY_NORMAL);
    }
    status_line (">Switching to priority: modem (%d:%d)",
                 priority_thread, priority_class);
    break;

  default:
    priority_class = IDLE_PRIORITY_CLASS;
    priority_thread = THREAD_PRIORITY_ABOVE_NORMAL;
    break;
  }

  SetPriorityClass (GetCurrentProcess (), (DWORD) priority_class);
  SetThreadPriority (GetCurrentThread (), priority_thread);

  return ret;
#else
  return pclass;
#endif
}

#endif /* _WIN32 */

/* MMP 960425  I intend to remove the conditional flushes needed because
 * of the the mixed buffered and unbuffered output; i.e. cputs and printf.
 * I'll do that by replacing printf with cprintf where cprintf is really
 * intended. Therefore we need a the following implementations.
 */

#ifdef NEED_CPUTS
int _cdecl
cputs (const char *str)
{
  fputs (str, stdout);          /*  TS 960620 parameter sequence was wrong  */
  fflush (stdout);
  return 0;
}

#endif

#ifdef NEED_PUTCH
int _cdecl
putch (int ch)
{
  return putchar (ch);
/*  fflush(stdout);  // jl 960726: unreachable code */
}

#endif

#ifdef NEED_CPRINTF
int _cdecl
cprintf (const char *format,...)
{
  va_list arg_ptr;
  char buf[256];
  int count;

  va_start (arg_ptr, format);
  count = sprintf (buf, format, arg_ptr);
  va_end (arg_ptr);
  cputs (buf);
  return (count);
}

#endif


#if defined(NEED_KBHIT) && defined(OS_2)

int
kbhit (void)
{
  KBDKEYINFO ki;

  ki.fbStatus = 0;
  KbdPeek (&ki, 0);

  if (ki.fbStatus & KBDTRF_FINAL_CHAR_IN)
    return (1);
  else
    return (0);
}

#endif

/* $Id: dosfuncs.c,v 1.11 1999/05/04 20:20:35 hjk Exp $ */
