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
 * Filename    : $Source: E:/cvs/btxe/include/box.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/28 18:30:21 $
 * State       : $State: Exp $
 *
 * Description : Definitions used in the BOX subroutines in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

typedef struct box_st
{
  unsigned char ul, ur, ll, lr; /* edges upper/lower left/right   */
  unsigned char tbar, bbar;     /* top/bottom 50% horiz. bar !?!? */
  unsigned char lbar, rbar;     /* left/right 50% vertical bar    */
  unsigned char tt, tb;         /* top/bottom 50% horiz. bar !?!? */
  unsigned char t0, t90, t180, t270;  /* T-edge rotated by 90.. deg     */
  unsigned char plus;           /* all four directions            */
}
BOXTYPE;

/* box types */

#define BOXASCII    0
#define BOX11       1
#define BOX22       2
#define BOX12       3
#define BOX21       4
#define BOXBLK      5

/* $Id: box.h,v 1.3 1999/02/28 18:30:21 ceh Exp $ */
