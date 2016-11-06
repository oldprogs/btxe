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
 * Filename    : $Source: E:/cvs/btxe/src/timer.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:23 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman, Vince Perriello, Robert Hoerner, Steffen Motzer
 *
 * Description : BinkleyTerm Timer Routines.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#define __DEBUG_TIMERS

/* This file contains routines to implement a simple multiple
 * alarm system.  The routines allow setting any number of alarms,
 * and then checking if any one of them has expired.  It also allows
 * adding time to an alarm. */

/* -------------------------------------------------------------------- */
/* both Robert Hoerner and Steffen Motzer should get a loud "hurray!"   */
/* for their power of working on this subject!                          */
/* -------------------------------------------------------------------- */

long
timerset (unsigned int stop)
{
  long now;
  int wday, hours, mins, secs, hdts;

  /* ----------------------------------------------------------------- */
  /* What this code does                                  (r. hoerner) */
  /* ----------------------------------------------------------------- */
  /* it gets the current system time, computes how many 1/100 seconds  */
  /* are gone at this hour, adds a maximum of 65535 to it and returns  */
  /* the sum of both.                                                  */
  /* ----------------------------------------------------------------- */
  dostime (&wday, &hours, &mins, &secs, &hdts);

  now = (long) mins *(long) PER_MINUTE +  /* types casted for 16 bit */
    (long) (secs * PER_SECOND) +  /* compilers */
    (long) hdts;

  /* ----------------------------------------------------------------- */
  /* the maximum value returned is at xx:59:59:99 and is 359999        */
  /* 59 mins * 6000 + 59 sesc * 100 + 99 ths = 359999  = (PER_HOUR - 1) */
  /* Only one 1/00 sec later at xx:00:00:00 the function will return 0 */
  /*                                                                   */
  /* The maximum "timerset" value it can return is                     */
  /* (current_time_value + 65535) / 100 secs, this allows timers with  */
  /* about 12 minutes expiration time                                  */
  /* ----------------------------------------------------------------- */

  return ((long) (now + (long) stop));
}

bool
timeup (long stop)
{
  long now, diff;

  /* ----------------------------------------------------------------- */
  /* Why this code is correct                             (r. hoerner) */
  /* ----------------------------------------------------------------- */
  /* Assume "stop" has been set at xx:59:59:99, and is 359.999 plus    */
  /* our "timeout" value. We check timeup() 1/100 sec later.           */
  /* Then we get a value of 0 for "now" and have a rollover problem    */
  /* ----------------------------------------------------------------- */
  /* In all other cases, especially when we call timeup() after        */
  /* timerset() but we call both between xx:00:00.00 and xx:59:59:99   */
  /* we do not need any checks for rollover.                           */
  /* In other words: as long as "now" is greater or equal than "stop"  */
  /* then there is NO problem at all.                                  */
  /* You can determine IF there is a problem, if you check the timers  */
  /* at least once within 30 minutes. You should do it anyway...       */
  /* ----------------------------------------------------------------- */

  now = timerset (0);

  /* timer should run until "now" reaches "stop" and */
  /* expires when "stop" <= "now" (this is "diff < 0" */
  /* unfortunately "now" becomes always smaller then */
  /* any "stop" at xx:00:00:00                       */
  diff = stop - now;

  /* then (stop > now) and we _might_ have a problem. */
  /* The problem exists, if "now" is very, very much */
  /* greater than "stop" (think about these timers   */
  /* running in minutes, but never in hours!         */
  if (diff > 0)
  {
    if (diff > PER_HOUR / 2)    /* then assume it is a hour rollover ! */
      diff -= PER_HOUR;         /* and let the timer expire            */
  }

  /* ----------------------------------------------------------------- */
  /* debug code to watch the current timer(s) running                  */
  /* ----------------------------------------------------------------- */

#ifdef DEBUG_TIMERS
  if ((wholewin != NULL) && debugging_log)
  {
    char j[30];

    sprintf (j, " %ld.%ld ", (diff) / 100, (diff) % 100);
    sb_move_puts (wholewin, 0, 10, j);
    sb_show ();
  }
#endif

  /* ----------------------------------------------------------------- */

  return (diff < 0L);           /* Return whether timer has expired */
}

long
longtimerset (long stop)
{
  long now;
  int wday, hours, mins, secs, hdts;

  /* ----------------------------------------------------------------- */
  /* What this code does                                  (r. hoerner) */
  /* ----------------------------------------------------------------- */
  /* it gets the current system time, computes how many 1/100 seconds  */
  /* are gone at this hour, adds a maximum of 2**31 to it and          */
  /* returns the sum of both.                                          */
  /* ----------------------------------------------------------------- */
  dostime (&wday, &hours, &mins, &secs, &hdts);

  now = (long) wday *(long) PER_DAY +
    (long) (hours % 24) * (long) PER_HOUR +
    (long) (mins % 60) * (long) PER_MINUTE +
    (long) ((secs % 60) * PER_SECOND) +
    (long) hdts;

  /* ----------------------------------------------------------------- */
  /* the maximum value returned is at sunday 23:59:59:99 and is        */
  /* 60.479.999.                                                       */
  /* Only one 1/00 sec later the function will return 0                */
  /*                                                                   */
  /* The maximum "longtimerset" value it can return is 2**31/100 secs, */
  /* this allows timers with about 248 days expiration time            */
  /* ----------------------------------------------------------------- */
  /* granularity:                                                      */
  /* theoretically  max "stop" = 2147473648, this is 248 days          */
  /* practical ('cause only weekday and hh:mm:ss:hh is counted and to  */
  /*            check the week rollover problem):                      */
  /*                max "stop" = 25920000, this are 3 days             */
  /* ----------------------------------------------------------------- */
  /* inform the user about a problem if his timer is too large         */
  /* ----------------------------------------------------------------- */

  if (stop > 3 * PER_DAY)
  {
    long value;

    value = stop / PER_SECOND;
    hours = (int) (value / 3600);
    mins = (int) (value % 3600) / 60;
    secs = (int) value % 60;

    status_line ("!Timer value clipped! (%02ld:%02ld:%02ld)",
                 hours, mins, secs);
    stop = 3L * (long) PER_DAY;
  }

  return (now + stop);          /* Return the alarm off time */
}

bool
longtimeup (long stop)
{
  long now, diff;

  /* ----------------------------------------------------------------- */
  /* Why this code is correct                             (r. hoerner) */
  /* ----------------------------------------------------------------- */
  /* Assume "stop" has been set sonday, 23:59:59:99, and is 60.479.999 */
  /* plus our "timeout" value. We check longtimeup() 1/100 sec later.  */
  /* Then we get a value of 0 for "now" and have a rollover problem    */
  /* ----------------------------------------------------------------- */
  /* In all other cases, especially when we call longtimeup() after    */
  /* longtimerset() but we call both between monday 00:00:00.00 and    */
  /* sunday at 23:59:59:99 we do not need any checks for rollover.     */
  /* In other words: as long as "now" is greater or equal than "stop"  */
  /* then there is NO problem at all.                                  */
  /* You can determine IF there is a problem, if you check the timers  */
  /* at least once per 3,5 days. You should do it anyway...            */
  /* ----------------------------------------------------------------- */

  now = longtimerset (0);

  /* timer should run until "now" reaches "stop" and */
  /* expires when "stop" <= "now" (this is "diff < 0" */
  /* unfortunately "now" becomes always smaller then */
  /* any "stop" at monday, midnight                  */
  diff = stop - now;

  /* then (stop > now) and we _might_ have a problem. */
  /* The problem exists, if "now" is very, very much */
  /* greater than "stop" (think about our timers     */
  /* running in minutes/hours, but never in days!    */
  if (diff > 0)
  {
    if (diff > PER_WEEK / 2)    /* then assume it is a week rollover ! */
      diff -= PER_WEEK;         /* and let the timer expire            */
  }

  /* ----------------------------------------------------------------- */
  /* debug code to watch the current timer(s) running                  */
  /* ----------------------------------------------------------------- */

#ifdef DEBUG_TIMERS
  if ((wholewin != NULL) && debugging_log)
  {
    char j[30];

    sprintf (j, " %ld.%ld ", (diff) / 100, (diff) % 100);
    sb_move_puts (wholewin, 0, 10, j);
    sb_show ();
  }
#endif

  /* ----------------------------------------------------------------- */

  return (diff < 0L);
}

/* $Id: timer.c,v 1.3 1999/02/27 01:16:23 mr Exp $ */
