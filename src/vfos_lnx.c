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
 * Filename    : $Source: E:/cvs/btxe/src/vfos_lnx.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/05/17 15:48:45 $
 * State       : $State: Exp $
 * Orig. Author: Ben Stuyts and Louis Lagendijk
 *
 * Description : BSD implementation of the Video Fossil functions
 *
 * Note        :
 *
 *   BinkleyTerm only uses:
 *     VioWrtTTY
 *     VioWrtCellStr
 *   Refer to FSC-0021 for more details
 *
 *---------------------------------------------------------------------------*/

#define far
typedef unsigned short USHORT;
extern int vfossil_installed;
typedef unsigned char *PCH;

#include <curses.h>
#include "sbuf.h"
#include "vfossil.h"

extern VIOMODEINFO vfos_mode;

// HJK - 99/01/24
//void
//fossil_ver (void)
//{
//}

/* Initialise video Fossil routines */

void
vfossil_init (void)
{
  int i;
  short f, b;

#ifdef NEXTDEBUG
  fprintf (stderr, "vfossil_init\n");
#endif

  initscr ();
  start_color ();

  if (has_colors ())
  {
    for (i = 0; i < 256; i++)
    {
      /* HJK 99/01/01 */
      f = i & 0x07;
      if (f == 0x01 || f == 0x03 || f == 0x04 || f == 0x06)
        f = f ^ 0x05;
      b = (i & 0x70) >> 4;
      if (b == 0x01 || b == 0x03 || b == 0x04 || b == 0x06)
        b = b ^ 0x05;
      init_pair (i, f, b);
    }
  }

  cbreak ();
  noecho ();
  vfos_mode.col = COLS;         /* Number of text columns */
  vfos_mode.row = LINES;        /* Number of test rows */
  vfos_mode.color = 2;          /* Number of colours */
  /* monitor type either.. mono/text/no_color, or other/text/color */
  vfos_mode.fbType = 1;
  vfossil_installed = 1;
}

void
vfossil_close (void)
{
#ifdef NEXTDEBUG
  fprintf (stderr, "vfossil_close\n");
#endif
  vfossil_installed = 0;
  endwin ();
}

/* Enable or disable cursor */

void
vfossil_cursor (int st)
{
}

/* Write a Cell string
 * Note that the string is in Character/Attribute pairs
 */

USHORT
VioWrtCellStr (PCH pchCellString,
               USHORT cbCellString,
               USHORT row,
               USHORT column,
               USHORT hvio)
{
  int i;

  move (row, column);

  if (!has_colors ())
  {
    for (i = 0; i < cbCellString; i += 2)
      addch (pchCellString[i]);
  }
  else
  {
    for (i = 0; i < cbCellString; i += 2)
    {
      if (pchCellString[i + 1])
      {
        /* HJK - 99/01/01 */
        if (((pchCellString[i + 1]) & 0x0F) < 0x08)
          attrset (COLOR_PAIR (pchCellString[i + 1]));
        else
          attrset (COLOR_PAIR (pchCellString[i + 1]) | A_BOLD);
      }
#ifdef USE_ALTCHARSET
      addch (pchCellString[i] | A_ALTCHARSET);
#else
      addch (pchCellString[i]);
#endif
    }
  }

  refresh ();
  return (0);
}

void
fossil_gotoxy (int x, int y)
{
  /* fprintf(stderr, "std move to %03d, %03d\n", x, y); */
  move (y, x);
}

int
fossil_wherex (void)
{
  return stdscr->_curx;
}

int
fossil_wherey (void)
{
  return stdscr->_cury;
}

void
_WRITE_ANSI (int ch)
{
  switch (ch)
  {
  case '\r':
    move (stdscr->_cury, 0);
    break;

  case '\n':
    move (stdscr->_cury + 1, stdscr->_curx);
    break;

  default:
    addch (ch);
    break;
  }
}

void
WRITE_ANSI (int ch)
{
  _WRITE_ANSI (ch);
  refresh ();
}

void
WRITE_ANSI_STR (char *string)
{
  while (*string != 0)
    _WRITE_ANSI (*string++);

  refresh ();
}

USHORT
VioWrtTTY (PCH pchString, USHORT cbString, USHORT hvio)
{
  int i;

  for (i = 0; pchString[i]; i++)
    _WRITE_ANSI (pchString[i]);

  refresh ();
  return (i);
}

/* $Id: vfos_lnx.c,v 1.7 1999/05/17 15:48:45 ceh Exp $ */
