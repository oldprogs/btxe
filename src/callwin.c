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
 * Filename    : $Source: E:/cvs/btxe/src/callwin.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:45 $
 * State       : $State: Exp $
 * Orig. Author: TS
 *
 * Description : call window/recent activity window scroll and search functs
 *
 * Note        : not yet fully implemented
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static void LOCALFUNC do_callwin (char *p);

int
callwin_dnar (void)
{
  if (callwin->lastshown != callwin->lastline && scrllines > callwin->lines)
  {
    callwin->lastshown += callwin->linesize;
    sb_scrl (callwin, 1);
    sb_move_puts (callwin, callwin->lines - 1, 0,
                  callwin->lastshown);
    sb_show ();
    return (1);
  }
  else
    return (0);
}

int
callwin_upar (void)
{
  char *p;

  if (callwin->lastshown != callwin->buffer && scrllines > callwin->lines)
  {
    int i1 = (int) (callwin->lastshown - callwin->buffer);
    int i2 = (callwin->lines - 1) * callwin->linesize;

    if (i2 < i1)
    {
      p = callwin->lastshown - (callwin->lines * callwin->linesize);
      callwin->lastshown -= callwin->linesize;
      sb_scrl (callwin, -1);
      sb_move_puts (callwin, 0, 0, p);
    }
    sb_show ();
    return (1);
  }
  else
    return (0);
}

int
callwin_pgdn (void)
{
  char *p;

  if (callwin->lastshown != callwin->lastline && scrllines > callwin->lines)
  {
    p = callwin->lastshown + (callwin->lines * callwin->linesize);

    if (p >= callwin->lastline)
      p = callwin->lastline;

    do_callwin (p);
    return (1);
  }
  else
    return (0);
}

int
callwin_pgup (void)
{
  char *p;

  if (scrllines > callwin->lines)
  {
    int i1 = (int) (callwin->lastshown - callwin->buffer);
    int i2 = (callwin->lines - 1) * callwin->linesize;

    if (i2 < i1)
    {
      if ((i1 - i2) < i2)
        p = callwin->buffer + i2;
      else
        p = callwin->buffer + i1 - i2;

      do_callwin (p);
    }
    return (1);
  }
  else
    return (0);
}

int
callwin_top (void)
{
  char *p;

  if (scrllines > callwin->lines)
  {
    p = callwin->buffer + ((callwin->lines - 1) * callwin->linesize);

    if (p > callwin->lastline)
      p = callwin->lastline;

    do_callwin (p);
    return (1);
  }
  else
    return (0);
}

int
callwin_end (void)
{
  char *p;

  if (scrllines > callwin->lines)
  {
    p = callwin->lastline;

    do_callwin (p);
    return (1);
  }
  else
    return (0);
}

static void LOCALFUNC
do_callwin (char *p)
{
  int j;

  if (p != callwin->lastshown)
  {
    callwin->lastshown = p;
    sb_scrl (callwin, 0);

    for (j = callwin->lines - 1; j >= 0 && p >= callwin->buffer; j--)
    {
      sb_move_puts (callwin, j, 0, p);
      p -= callwin->linesize;
    }
    sb_show ();
  }
}

#define MAX_SEARCHSTRING_LEN    48

int
Do_Ras (BINK_SAVEP p, int use_default)
{
  REGIONP r;
  char searchstring[MAX_SEARCHSTRING_LEN + 1];
  int done = 0, c = 0, col = -1, sslen = 0;
  long t_idle;
  char *cp, *q;

  use_default = use_default;    /* makes compiler happy */

  if (p == NULL)
    return (0);

  r = p->region;
  sb_fill (r, ' ', colors.popup);
  sb_box (r, boxtype, colors.popup);
  sb_caption (r, MSG_TXT (M_RAS_CAPTION), colors.popup);
  sb_move_puts (r, 1, 1, MSG_TXT (M_SEEK_MSG));

  searchstring[0] = '\0';
  sb_input_chars (r, 1, 8, searchstring, MAX_SEARCHSTRING_LEN);

  if (0 == (sslen = strlen (searchstring)))
    return 0;

  sb_move_puts (r, 2, 1, MSG_TXT (M_RAS_KEYS));
  t_idle = timerset (PER_SECOND * 10);

  /* MR 971204 switch to normal priority to avoid system slowdown when    */
  /*           searching through the log                                  */
  set_prior (PRIO_NORMAL);

  while (!timeup (t_idle) && (PEEKBYTE () == (short) 0xffff) && !done)
  {
    if (KEYPRESS ())
    {
      t_idle = timerset (30 * PER_SECOND);
      c = (int) KbRemap (FOSSIL_CHAR ());
      switch (c)
      {
      case 27:
        done = 1;
        break;

      case F_PEND_HOME:        /*  go to top of buffer,search downward */
        callwin_top ();
        break;

      case F_PEND_END:         /*  go to bot of buffer, search upward  */
        callwin_end ();
        break;

      case F_PEND_UPAR:        /*  search upward from current position */
        cp = callwin->lastshown;
        while (cp != callwin->buffer)
        {
          cp -= callwin->linesize;
          q = strstrci (cp, searchstring);
          if (q)                /*  string found    */
          {
            col = q - cp;
            break;
          }
          else                  /*  erase popup attrib from previous search */
            col = -1;
        }
        do_callwin (cp);
        sb_move_abs (callwin, callwin->r1 - callwin->r0 - 1, col + 1);
        if (col > -1)
          sb_wa_abs (callwin, colors.popup, sslen);
        else
          sb_wa_abs (callwin, colors.call, sslen);
        break;

      case F_PEND_DNAR:        /*  search downward from current pos    */
        cp = callwin->lastshown;
        while (cp != callwin->lastline)
        {
          cp += callwin->linesize;
          q = strstrci (cp, searchstring);
          if (q)                /*  string found   */
          {
            col = q - cp;
            break;
          }
          else                  /*  erase popup attrib from previous search */
            col = -1;
        }
        do_callwin (cp);
        sb_move_abs (callwin, callwin->r1 - callwin->r0 - 1, col + 1);
        if (col > -1)
          sb_wa_abs (callwin, colors.popup, sslen);
        else
          sb_wa_abs (callwin, colors.call, sslen);
        break;

      default:
        break;
      }
    }

    //display_life();   HJK - already done in time_release()
    time_release ();            /* MR 971204 shouldn't we release some timeslices? */
  }

  set_prior (PRIO_REGULAR);     /* MR 971204 switch back to regular prio... */

  /*  if we found something, we do a final change from highlight to       */
  /*  normal callwin color just to tidy things up.                        */
  if (col > -1)
  {
    sb_move_abs (callwin, callwin->r1 - callwin->r0 - 1, col + 1);
    sb_wa_abs (callwin, colors.call, sslen);
  }
  return 0;
}

/* $Id: callwin.c,v 1.3 1999/02/27 01:15:45 mr Exp $ */
