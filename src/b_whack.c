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
 * Filename    : $Source: E:/cvs/btxe/src/b_whack.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:39:59 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm FTSC Mail Session Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int
Whack_CR ()
{
  long t1, t2;
  unsigned char j;
  char fGotAChar;               /* When set we have gotten at least 1 char */
  char fDoingALine;             /* When set we're in middle of line        */
  char fDidALine;               /* When set we've already gotten 1 CR      */
  int k, l;
  unsigned short i;
  char buf[180];

  t1 = timerset (30 * PER_SECOND);  /* set 30 second timeout */
  j = CR;
  strcpy (buf, MSG_TXT (M_INTRO));
  l = (int) strlen (buf);
  fGotAChar = fDoingALine = fDidALine = 0;
  k = 2;

  while (!timeup (t1) && CARRIER)  /* till then or CD lost  */
  {
    time_release ();            /*PLF Sun  12-01-1991  05:13:51 */

    /* We send two XON's to try to unstick a V.42 miscue */
    if (k-- > 0)
      SENDBYTE (XON);           /* Unstick other side?   */

    /* Send either the CR or SPACE */
    SENDBYTE (j);

    /* Now switch to either CR or SPACE based on last output */
    j = (unsigned char) (45 - j);

    /* Set a one second timer */
    t2 = timerset (PER_SECOND);

    /* Now read what we get back to see if we get anything useful.
       But only allow up to that one second to get it. */

    while (!timeup (t2))
    {
      /* Get a character if there is one. Otherwise give up cycles
         to the system. */

      if ((i = (unsigned short) PEEKBYTE ()) == (unsigned short) -1)
      {
        if (fDidALine)
          break;
        time_release ();
        continue;
      }

      /* If we're doing EMSI, this is the first char and it's
         an asterisk, exit now without losing it. */

      if (!fDoingALine && !no_EMSI_Session && (i == (unsigned short) '*'))
        return (0);

      /* Eat the character */

      TIMED_READ (0);

      /* If the character is a CR and we got a response, we can leave. */

      if (i == CR && fGotAChar)
      {
        if (fDoingALine && !fDidALine)
        {
          status_line ("*%s", buf);

          /* We got what we wanted, now try to stop output on an Opus */
          SENDBYTE ('');
        }

        fDoingALine = 0;
        fDidALine = 1;
      }

      /* Otherwise, if it's a printing character, save it for logging. */

      else if (i >= ' ')
      {
        if (fDoingALine || i != ' ')
        {
          fDoingALine = 1;
          if (fDidALine)
            continue;
          buf[l++] = (char) (i & 0xff);
          buf[l] = '\0';
          if (l > (int) SB_COLS - 6)
            l = SB_COLS - 6;
        }

        /* One-shot: reset the 1-second timeout for the first printable
           character we get. */

        if (!fGotAChar)
        {
          fGotAChar = 1;
          t2 = timerset (PER_SECOND);
        }
      }
    }                           /* End 1-second inner timer loop */

    if (fDidALine)
      return (0);

    /* Bail out for keyboard escape */

    if (got_ESC ())
    {
      status_line (MSG_TXT (M_CONNECT_ABORTED));
      modem_hangup ();
      return (-1);
    }

  }                             /* End 30-second braindead timer loop */

  if (CARRIER)
    status_line (MSG_TXT (M_NOBODY_HOME));
  else
    status_line (MSG_TXT (M_NO_CARRIER));
  return (-1);
}

/* $Id: b_whack.c,v 1.4 1999/03/23 22:39:59 mr Exp $ */
