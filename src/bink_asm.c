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
 * Filename    : $Source: E:/cvs/btxe/src/bink_asm.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 17:48:18 $
 * State       : $State: Exp $
 * Orig. Author: tom schlangen
 *
 * Description : replaces bink_asm.asm
 *
 * Note        :
 *   the routines in this module are intended to replace the ones in
 *   BINK_ASM.ASM. if you encounter problems with these routines concearning
 *   certain m'taskers, please notify me @2:2450/10 and switch back to the
 *   ones in BINK_ASM.ASM as a first help. this .ASM file is not distributed
 *   in this sourcetree, though.
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

#ifdef DOS16

#ifndef MK_FP
#define MK_FP(seg,off) ((void far *)(((unsigned long)(seg) << 16)|(unsigned)(off)))
#endif

unsigned _cdecl
dos_largest_free_block (void)   /*  TS 970524   */
{
  union REGS regs;

  regs.h.ah = 0x48;             /* this is the dos int 21h allocate memory func  */
  regs.x.bx = 0xffff;           /* request impossible large number of paragraphs */
  int86 (0x21, &regs, &regs);   /* will fail and return avail paras in BX   */
  return (regs.x.bx >> 6);      /* return value scaled to kB (1024 bytes)   */
}

int _cdecl
real_flush (int handle)
{
  /* strategy used: duplicate the filehandle and close the dup'ed handle    */
  union REGS regs;

  regs.h.ah = 0x45;             /*  int21h request dup file handle function */
  regs.x.bx = handle;
  int86 (0x21, &regs, &regs);
  if (regs.x.cflag == 1)        /*  if failed, signal error on return       */
    return 1;
  /*  duplicated handle is in ax; now close it                              */
  regs.x.bx = regs.x.ax;
  regs.h.ah = 0x3E;             /*  int21h close file handle function       */
  int86 (0x21, &regs, &regs);
  return 0;
}

void _cdecl
msdos_pause (void)
{
  union REGS regs;

  int86 (0x28, &regs, &regs);   /*  what an overhead for an int call :-(    */
}

void _cdecl
windows_pause (void)
{
  union REGS regs;

  regs.x.ax = 0x1680;
  int86 (0x2f, &regs, &regs);   /* needed for MS-DOS   */

  regs.x.ax = 0x1680;
  int86 (0x28, &regs, &regs);   /* fix for warp4 bug   */
  /* works with DOS, too */
}

void _cdecl
dv_pause (void)
{
  union REGS regs;

  /*  AW 980222 we don't need stack switching for time slice
   *  that was the cause for the hangups in DesqView since XR5
   * regs.x.ax=0x101A;
   * int86(0x15, &regs, &regs);
   */

  regs.x.ax = 0x1000;
  int86 (0x15, &regs, &regs);

  /*  AW 980222 we don't need stack switching for time slice
   * regs.x.ax=0x1025;
   * int86(0x15, &regs, &regs);
   */
}

unsigned _cdecl
dv_get_version (void)
{
  union REGS regs;

  regs.x.cx = 0x4445;
  regs.x.dx = 0x5351;
  regs.x.ax = 0x2B01;
  int86 (0x21, &regs, &regs);
  if (regs.h.al != 0xFF)
    return (regs.x.bx);
  else
    return 0;
}

unsigned _cdecl
windows_active (void)
{
  union REGS regs;

  regs.x.ax = 0x160A;
  int86 (0x2F, &regs, &regs);
  if (regs.x.ax == 0)
    return 1;
  else
    return 0;
}

/* added from bink_asm.asm (r. hoerner) */

int _cdecl
get_cshape (void)
{
  union REGS regs;

  regs.h.ah = 0x0f;
  int86 (0x10, &regs, &regs);   /* returns active screen page in regs.h.bh */
  regs.h.ah = 0x03;
  int86 (0x10, &regs, &regs);   /* ah=start scan line, al=stop scan line */

  return ((int) regs.x.ax);
}

void _cdecl
set_cshape (int shape)
{
  union REGS regs;

  regs.h.ah = 0x01;
  regs.x.cx = shape;
  int86 (0x10, &regs, &regs);
}

void _cdecl
hide_cursor (int shape)
{
  union REGS regs;

  regs.h.ah = 0x01;
  regs.x.cx = shape;
  regs.h.ch = 0x30;             /* my doku says: 0x20, tom. */
  int86 (0x10, &regs, &regs);
}

int _cdecl
getbiosscreenrows (void)
{
  unsigned char rows;

  rows = (unsigned char) (*((char far *) MK_FP (0x0040, 0x0084)));
  return ((int) rows);
}

void _cdecl
clear_screen (void)             /* r. hoerner */
{
  union REGS regs;

  regs.h.ah = 0x0f;
  int86 (0x10, &regs, &regs);   /* get video mode */
  regs.h.ah = 0x00;
  int86 (0x10, &regs, &regs);   /* set video mode */
}

void _cdecl
clear_to_eol (void)             /* r. hoerner */
{
  union REGS regs;
  unsigned char columns;

  regs.h.ah = 0x0f;
  int86 (0x10, &regs, &regs);   /* ret: al=videomode, bh=page, bl: colums */
  columns = regs.h.bl;
  regs.h.ah = 0x03;
  int86 (0x10, &regs, &regs);   /* get cursor position: dl=current column */
  regs.h.al = ' ';
  regs.h.ah = 0x0a;
  regs.x.cx = columns - regs.h.dl - 1;  /* dl is zero based! */
  int86 (0x10, &regs, &regs);   /* write character in al, cx times */
  regs.h.ah = 0x03;
  int86 (0x10, &regs, &regs);   /* get cursor position: dl=current column */
  regs.h.dl = columns;
  regs.h.ah = 0x02;
  int86 (0x10, &regs, &regs);   /* set cursor to end of line */
}

#endif /*  DOS16   */

/* $Id: bink_asm.c,v 1.3 1999/02/27 17:48:18 mr Exp $ */
