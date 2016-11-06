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
 * Filename    : $Source: E:/cvs/btxe/src/times.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:39 $
 * State       : $State: Exp $
 *
 * Description : Txx flag support
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

/*
   we will have to look into the nodelist file to check these flags..
   +------+----+  +------+----+  +------+----+  +------+----+  +------+----+
   |Letter|Time|  |Letter|Time|  |Letter|Time|  |Letter|Time|  |Letter|Time|
   +------+----+  +------+----+  +------+----+  +------+----+  +------+----+
   |   A  |0000|  |   F  |0500|  |   K  |1000|  |   P  |1500|  |   U  |2000|
   |   a  |0030|  |   f  |0530|  |   k  |1030|  |   p  |1530|  |   u  |2030|
   |   B  |0100|  |   G  |0600|  |   L  |1100|  |   Q  |1600|  |   V  |2100|
   |   b  |0130|  |   g  |0630|  |   l  |1130|  |   q  |1630|  |   v  |2130|
   |   C  |0200|  |   H  |0700|  |   M  |1200|  |   R  |1700|  |   W  |2200|
   |   c  |0230|  |   h  |0730|  |   m  |1230|  |   r  |1730|  |   w  |2230|
   |   D  |0300|  |   I  |0800|  |   N  |1300|  |   S  |1800|  |   X  |2300|
   |   d  |0330|  |   i  |0830|  |   n  |1330|  |   s  |1830|  |   x  |2330|
   |   E  |0400|  |   J  |0900|  |   O  |1400|  |   T  |1900|  |      |    |
   |   e  |0430|  |   j  |0930|  |   o  |1430|  |   t  |1930|  |      |    |
   +------+----+  +------+----+  +------+----+  +------+----+  +------+----+
 */

int
isnow_online (char start, char stop)
{
  unsigned long now;
  int wday, hours, mins, secs, ths;

  start -= 'A';                 /* 'A','a','B','b'... */
  stop -= 'A';
  start *= 2;
  stop *= 2;

  /* result: 0,2,4,6,8... 46, 'cause 'X' is the end */

  if (start > 46)               /* then it was 'a','b', else it was 'A','B'.. */
    start += 1;

  if (stop > 46)
    stop += 1;

  start *= 30;
  stop *= 30;

  /* now start and stop contain the minute of the day when the user is online */

  dostime (&wday, &hours, &mins, &secs, &ths);

  now = hours * 60 + mins;
  return (start <= now && now < stop);  /* return result. */
}

int
IsOnLine (short start, short end)
{
  short is_online_now = 0;
  int wday, hours, mins, secs, ths;
  short now;

  dostime (&wday, &hours, &mins, &secs, &ths);
  now = hours * 60 + mins + (short) (TIMEZONE / 60L);
  if (now < 0)
    now += 1440;

  if (start == end)
    is_online_now = 0;
  else
  {
    if (start < end)
      is_online_now = (start <= now && now < end);
    else
      is_online_now = ((start <= now && now < 1440) ||
                       (now > 0 && now < end));
  }

  return is_online_now;
}

/* $Id: times.c,v 1.4 1999/09/27 20:51:39 mr Exp $ */
