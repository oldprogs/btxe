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
 * Filename    : $Source: E:/cvs/btxe/src/evtsetup.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 01:49:24 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : event behaviour setup screen
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

static REGIONP evtwin;
static BINK_EVENT eventedit;

void
WriteYesNo (int row, int col, int bit)
{
  sb_move_puts (evtwin, row, col, bit ? "Yes" : "No ");
}

void
EventScreenUpdate (void)
{
  WriteYesNo (2, 31, eventedit.behavior & MAT_CM);
  WriteYesNo (3, 31, eventedit.behavior & MAT_DYNAM);
  WriteYesNo (4, 31, eventedit.behavior & MAT_BBS);
  WriteYesNo (5, 31, !(eventedit.behavior & MAT_NOREQ));
  WriteYesNo (6, 31, eventedit.behavior & MAT_OUTONLY);
  WriteYesNo (7, 31, !(eventedit.behavior & MAT_NOOUT));
  WriteYesNo (8, 31, eventedit.behavior & MAT_LOCAL);
  WriteYesNo (9, 31, eventedit.errlevel[1]);
  WriteYesNo (2, 74, eventedit.behavior & MAT_NOMAIL24);
  WriteYesNo (3, 74, !(eventedit.behavior & MAT_NOOUTREQ));
  WriteYesNo (4, 74, !(eventedit.behavior & MAT_NOCM));
  WriteYesNo (5, 74, eventedit.behavior & MAT_HIPRICM);
  WriteYesNo (6, 74, !(eventedit.behavior & MAT_NOPICKUP));
  WriteYesNo (7, 74, eventedit.behavior & MAT_COSTEVENT);
  WriteYesNo (8, 74, !(eventedit.extramask & EXTR_NOSOUND));
  WriteYesNo (9, 74, eventedit.errlevel[2]);
}

int
Do_Event (BINK_SAVEP rp, int x)
{
  long t_idle;
  int c, changed = 0, done = 0;

  /* is there a window */

  if (rp == NULL)
    return (0);

  evtwin = rp->region;

  /* draw a box */

  sb_fill (evtwin, ' ', colors.hold);
  sb_box (evtwin, boxtype, colors.frames);

  /* give it a title */

  sb_caption (evtwin, "Event behaviour setup", colors.frames);

  evtwin->sr0++;
  evtwin->sr1--;
  evtwin->sc0++;
  evtwin->sc1--;
  sb_fillc (evtwin, ' ');
  evtwin->sr0--;
  evtwin->sr1++;
  evtwin->sc0--;
  evtwin->sc1++;

  show_alive ();

  t_idle = timerset (PER_MINUTE);
  memcpy (&eventedit, &e_ptrs[cur_event], sizeof (BINK_EVENT));

  /* Write options */
  sb_move_puts (evtwin, 2, 1, " 0........Send C?? mail only [   ] 8...................Call CM & non CM [   ]");
  sb_move_puts (evtwin, 3, 1, " 1.............Dynamic event [   ] 9..................Outgoing requests [   ]");
  sb_move_puts (evtwin, 4, 1, " 2.........Allow BBS callers [   ] A...................Send to CM nodes [   ]");
  sb_move_puts (evtwin, 5, 1, " 3.Satisfy incoming requests [   ] B..Send high priority crashmail only [   ]");
  sb_move_puts (evtwin, 6, 1, " 4........Outbound mail only [   ] C...........Pickup on outgoing calls [   ]");
  sb_move_puts (evtwin, 7, 1, " 5............Outgoing calls [   ] D...........................reserved [   ]");
  sb_move_puts (evtwin, 8, 1, " 6..........Local calls only [   ] E.............................Sounds [   ]");
  sb_move_puts (evtwin, 9, 1, " 7..Exit if mail is received [   ] F.Exit if compressed mail is receiv. [   ]");
  sb_move_puts (evtwin, 11, 1, "                     S -> Save changes, ESC -> cancel                        ");
  EventScreenUpdate ();
  while (!timeup (t_idle) && (PEEKBYTE () == (short) 0xffff) && !done)
  {
    if (KEYPRESS ())
    {
      t_idle = timerset (30 * PER_SECOND);
#ifdef DOS16                    /*  TS 970615   */
      c = FOSSIL_CHAR ();
#else
      c = READKB ();            /* AG 990311 emx+gcc doesn't like toupper of 16bit values */
#endif
      if ((c & 0xff) != 0)
      {
        c &= 0xff;
        c = toupper (c);
      }
      switch (c)
      {
      case '0':
        eventedit.behavior ^= MAT_CM;
        WriteYesNo (2, 31, eventedit.behavior & MAT_CM);
        break;
      case '1':
        eventedit.behavior ^= MAT_DYNAM;
        WriteYesNo (3, 31, eventedit.behavior & MAT_DYNAM);
        break;
      case '2':
        eventedit.behavior ^= MAT_BBS;
        WriteYesNo (4, 31, eventedit.behavior & MAT_BBS);
        break;
      case '3':
        eventedit.behavior ^= MAT_NOREQ;
        WriteYesNo (5, 31, !(eventedit.behavior & MAT_NOREQ));
        break;
      case '4':
        eventedit.behavior ^= MAT_OUTONLY;
        WriteYesNo (6, 31, eventedit.behavior & MAT_OUTONLY);
        break;
      case '5':
        eventedit.behavior ^= MAT_NOOUT;
        WriteYesNo (7, 31, !(eventedit.behavior & MAT_NOOUT));
        break;
      case '6':
        eventedit.behavior ^= MAT_LOCAL;
        WriteYesNo (8, 31, eventedit.behavior & MAT_LOCAL);
        break;
      case '7':
        eventedit.errlevel[1] = eventedit.errlevel[1] ? 0 : keeperr1;
        WriteYesNo (9, 31, eventedit.errlevel[1]);
        break;
      case '8':
        eventedit.behavior ^= MAT_NOMAIL24;
        WriteYesNo (2, 74, eventedit.behavior & MAT_NOMAIL24);
        break;
      case '9':
        eventedit.behavior ^= MAT_NOOUTREQ;
        WriteYesNo (3, 74, !(eventedit.behavior & MAT_NOOUTREQ));
        break;
      case 'A':
        eventedit.behavior ^= MAT_NOCM;
        WriteYesNo (4, 74, !(eventedit.behavior & MAT_NOCM));
        break;
      case 'B':
        eventedit.behavior ^= MAT_HIPRICM;
        WriteYesNo (5, 74, eventedit.behavior & MAT_HIPRICM);
        break;
      case 'C':
        eventedit.behavior ^= MAT_NOPICKUP;
        WriteYesNo (6, 74, !(eventedit.behavior & MAT_NOPICKUP));
        break;
      case 'D':
        // TJW 970525: this really makes NO sense!!! :
        // eventedit.behavior^=MAT_COSTEVENT;
        // WriteYesNo ( 7,74,eventedit.behavior & MAT_COSTEVENT);
        break;
      case 'E':
        eventedit.extramask ^= EXTR_NOSOUND;
        WriteYesNo (8, 74, !(eventedit.extramask & EXTR_NOSOUND));
        break;
      case 'F':
        eventedit.errlevel[2] = eventedit.errlevel[2] ? 0 : keeperr2;
        WriteYesNo (9, 74, eventedit.errlevel[2]);
        break;
      case 'S':
        memcpy (&e_ptrs[cur_event], &eventedit, sizeof (BINK_EVENT));
        changed = 1;
        done = 1;
        break;
      case 27:
        done = 1;
        break;
      }
    }

    time_release ();
  }

  if (changed)
  {
    /* Update event-dependent variables too */
    /* Cut'n'pasted from sched.c */
    if (eventedit.behavior & MAT_NOREQ)
    {
      matrix_mask &= ~TAKE_REQ;
      no_requests = 1;
    }
    else
    {
      matrix_mask |= TAKE_REQ;
      no_requests = 0;
    }
    if (eventedit.behavior & MAT_NOOUTREQ)
    {
      requests_ok = 0;
    }
    else
    {
      requests_ok = 1;
    }
    if (eventedit.behavior & MAT_NOPICKUP)
    {
      pickup_ok = 0;            /* no pickup implies that i */
      requests_ok = 0;          /* cannot request something */
    }                           /* - r.hoerner -            */
    else
    {
      pickup_ok = 1;
    }
  }
  return (x);
}

/* $Id: evtsetup.c,v 1.4 1999/03/22 01:49:24 mr Exp $ */
