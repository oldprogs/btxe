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
 * Filename    : $Source: E:/cvs/btxe/src/b_today.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:18 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Initial Fullscreen Setup
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
do_today (int show)
{
  char j[30], junk[256];
  int src, dst;

  // ADDR tmp;

  sprintf (j, "%d/%d/%d", hist.mail_calls, hist.bbs_calls, hist.fax_calls);
  sprintf (junk, MSG_TXT (M_CALLS_IN), j);
  sb_move_puts (historywin, HIST_IN_ROW, 0, junk);

  sprintf (j, "%d(%d)/%ld", hist.connects, hist.calls_made, hist.callcost);
  sprintf (junk, MSG_TXT (M_CALLS_OUT), j);
  sb_move_puts (historywin, HIST_OUT_ROW, 0, junk);

  sprintf (j, "%d/%s", hist.files_in, numdisp (hist.size_in, 4));

  for (src = 0, dst = 0; src <= strlen (j); src++)
  {
    if (j[src] != ' ')
      j[dst++] = j[src];
  }

  sprintf (junk, MSG_TXT (M_RX_C_V), j);
  sb_move_puts (historywin, HIST_RX_ROW, 0, junk);

  sprintf (j, "%d/%s", hist.files_out, numdisp (hist.size_out, 4));
  for (src = 0, dst = 0; src <= strlen (j); src++)
  {
    if (j[src] != ' ')
      j[dst++] = j[src];
  }
  sprintf (junk, MSG_TXT (M_TX_C_V), j);
  sb_move_puts (historywin, HIST_TX_ROW, 0, junk);

  sprintf (j, "%d/%d/%d", hist.err_in < 99 ? (int) hist.err_in : 99,
           hist.err_out < 99 ? (int) hist.err_out : 99,
           hist.total_errors < 99 ? hist.total_errors : 99);
  sprintf (junk, MSG_TXT (M_TAAG_ERRORS), j);
  sb_move_puts (historywin, HIST_ERR_ROW, 0, junk);

  sprintf (junk, MSG_TXT (M_LAST), hist.last_str);
  sb_move_puts (historywin, HIST_LAST_ROW, 0, junk);

  if (show)
    sb_show ();
}

void
bottom_line (int force)
{
  if (!(screen_blank && do_screen_blank) &&
#ifdef OS_2                     /* TS 970328, CFS please have a look if this is intentional */
      (!MonitorCFOS || force)
#else
      (force)
#endif
    )
  {
    sb_move_puts (wholewin, (short) (SB_ROWS - 1), 0, ANNOUNCE);

    if (serial == -1)
      sb_puts (wholewin, MSG_TXT (M_UNREGISTERED));
    sb_move_puts (wholewin,
                  (short) (SB_ROWS - 1),
                  (short) (SB_COLS - strlen (MSG_TXT (M_ALTF10))),
                  MSG_TXT (M_ALTF10));
  }
}

/* $Id: b_today.c,v 1.5 1999/03/22 03:47:18 mr Exp $ */
