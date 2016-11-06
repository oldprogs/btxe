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
 * Filename    : $Source: E:/cvs/btxe/src/vfos_dos.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:44 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm VFOSSIL module DOS.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

extern VIOMODEINFO vfos_mode;

USHORT far pascal write_screen (PCH, USHORT, USHORT, USHORT, USHORT);
USHORT far pascal write_chars (PCH, USHORT, USHORT, USHORT, USHORT);
USHORT far pascal video_mode (PVIOMODEINFO, USHORT);

void
vfossil_init ()
{
  char far *q;
  union REGS inregs, outregs;
  struct SREGS s;
  VFOSSIL v;

  v.vfossil_size = sizeof (VFOSSIL);
  q = (char far *) &v;

  vfossil_installed = 0;

  inregs.h.ah = 0x81;
  inregs.h.al = 0;

  segread (&s);
  s.es = FP_SEG (q);
  inregs.x.di = FP_OFF (q);
  int86x (0x14, &inregs, &outregs, &s);

  if (outregs.x.ax == 0x1954)
  {
    /* There is a VFOSSIL out there, so set it up for use */
    inregs.h.al = 1;
    inregs.x.cx = 80;
    q = (char far *) &vfossil_funcs;
    inregs.x.di = FP_OFF (q);
    s.es = FP_SEG (q);
    int86x (0x14, &inregs, &outregs, &s);
    if ((outregs.x.ax == 0x1954) && (outregs.x.bx >= 14))
    {
      /* It is really out there */
      vfossil_installed = 1;
      q = (char far *) &vfos_mode;
      vfos_mode.cb = sizeof (VIOMODEINFO);
      VioGetMode ((PVIOMODEINFO) q, 0);
    }
  }

  if (!vfossil_installed)
  {
    memset ((char *) &vfossil_funcs, 0, sizeof (vfossil_funcs));
    vfossil_funcs.GetMode = video_mode;
    vfossil_funcs.WrtCellStr = write_screen;
    vfossil_funcs.WrtCharStr = write_chars;
  }
}

void
vfossil_cursor (int st)
{
  CURSOR cur;

  if ((long) vfossil_funcs.GetCurType)
  {
    /* We can make the cursor go away */
    VioGetCurType (&cur, 0);
    cur.cur_attr = st ? 0 : -1;
    VioSetCurType (&cur, 0);
  }

  if (!vfossil_installed)       /*  TS 961215   */
  {
    switch (st)
    {
    case 0:
      hide_cursor (cshape);
      break;

    case 1:
      set_cshape (cshape);
      break;
    }
  }
}

void
vfossil_close ()
{
  union REGS r;

  vfossil_cursor (1);
  r.h.ah = 0x81;
  r.h.al = 2;
  int86 (0x14, &r, &r);
  vfossil_installed = 0;
}

USHORT far pascal
video_mode (PVIOMODEINFO s, USHORT h)
{
  if (s->cb < 8)
    return (382);

  if (h != 0)
    return (436);

  s->fbType = 1;
  s->color = 1;
  s->col = 80;
  s->row = 23;
  return (0);
}

USHORT far pascal
write_screen (PCH s, USHORT l, USHORT r, USHORT c, USHORT h)
{
  USHORT i;
  int far *p;

  p = (int far *) s;

  /* The following line is just to make -W3 happy */
  i = h;
  gotoxy (c, r);
  l = l / 2;
  for (i = 0; i < l; i++)
  {
    WRITE_BIOS (*p);
    ++p;
  }

  return (0);
}

USHORT far pascal
write_chars (PCH s, USHORT l, USHORT r, USHORT c, USHORT h)
{
  USHORT i;

  /* The following line is just to make -W3 happy */
  i = h;
  gotoxy (c, r);
  for (i = 0; i < l; i++)
  {
    WRITE_BIOS (*s);
    ++s;
  }

  return (0);
}

/* $Id: vfos_dos.c,v 1.2 1999/03/22 03:47:44 mr Exp $ */
