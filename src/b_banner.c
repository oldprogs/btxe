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
 * Filename    : $Source: E:/cvs/btxe/src/b_banner.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:09 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Opening Banner
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
opening_banner ()
{
  if (!un_attended)
  {
    char bd[10];
    char junk[256];

    screen_clear ();
    scr_printf (ANNOUNCE);
    scr_printf ("\r\nA Freely Available<tm> Dumb Terminal and FidoNet Mail Package\r\n");
    scr_printf (MSG_TXT (M_SETTING));
    sprintf (junk, " %s%d", port_device, 1 + port_ptr);
    scr_printf (junk);
    scr_printf (MSG_TXT (M_INITIAL_SPEED));
    sprintf (bd, "%lu", max_baud.rate_value);
    scr_printf (bd);
    scr_printf (MSG_TXT (M_INTRO_END));

#ifdef DOS16
    if (fossil_info.curr_fossil > 0)
      fossil_announce (0);
#endif
  }
}

/* $Id: b_banner.c,v 1.3 1999/03/22 03:47:09 mr Exp $ */
