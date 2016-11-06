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
 * Filename    : $Source: E:/cvs/btxe/include/vt100.h,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/05 02:51:36 $
 * State       : $State: Exp $
 * Orig. Author: Miquel van Smoorenburg
 *
 * Description : ANSI/VT102 emulator definitions.
 *
 * Note        :
 *   This code was integrated to the Minicom communications package, but has
 *   been reworked to allow usage as a separate module.
 *
 *   It's not a "real" vt102 emulator - it's more than that: somewhere between
 *   vt220 and vt420 in commands.
 *
 *   This file is part of the minicom communications package,
 *   Copyright 1991-1995 Miquel van Smoorenburg.
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the
 *   Free Software Foundation; either version 2 of the License, or (at your
 *   option) any later version.
 *
 *---------------------------------------------------------------------------*/

/* moved from vt100.c by Alex Woick, 981215                                  */
/* window.h ================================================================ */

/* One character is contained in a "ELM" */
typedef struct _elm
{
  char value;
  char attr;
  char color;
}
ELM;

/* Control struct of a window */
typedef struct _win
{
  int x1, y1, x2, y2;           /* Coordinates */
  int sy1, sy2;                 /* Scrolling region */
  int xs, ys;                   /* x and y size */
  char border;                  /* type of border */
  char cursor;                  /* Does it have a cursor */
  char attr;                    /* Current attribute of window */
  char color;                   /* Current color of window */
  char autocr;                  /* With '\n', do an automatic '\r' */
  char doscroll;                /* Automatically scroll up */
  char wrap;                    /* Wrap around edge */
  char direct;                  /* Direct write to screen ? */
  short curx, cury;             /* + current x and y */
  char o_cursor;
  short o_curx;
  short o_cury;
  char o_attr;
  char o_color;                 /* Position & attributes before window was opened */
  ELM *map;                     /* Map of contents */
  ELM *histbuf;                 /* History buffer. */
  int histlines;                /* How many lines we keep in the history buffer */
  int histline;                 /* Current line in the history buffer. */
}
WIN;

/* Possible attributes */
#define XA_NORMAL        0
#define XA_BLINK         1
#define XA_BOLD          2
#define XA_REVERSE       4
#define XA_STANDOUT      8
#define XA_UNDERLINE    16
#define XA_ALTCHARSET   32
#define XA_BLANK        64

/* Possible colors */
#define BLACK       0
#define RED         1
#define GREEN       2
#define YELLOW      3
#define BLUE        4
#define MAGENTA     5
#define CYAN        6
#define WHITE       7

#define COLATTR(fg, bg) (((fg) << 4) + (bg))
#define COLFG(ca)   ((ca) >> 4)
#define COLBG(ca)   ((ca) % 16)

/* Possible borders */
#define BNONE   0
#define BSINGLE 1
#define BDOUBLE 2

/* Scrolling directions. */
#define S_UP    1
#define S_DOWN  2

/* Cursor types. */
#define CNONE   0
#define CNORMAL 1

/* Title Positions */
#define TLEFT   0
#define TMID    1
#define TRIGHT  2

/* Function prototypes */
int wxgetch (void);
void wflush (void);
WIN *wopen (int x1, int y1, int x2, int y2, int border,
            int attr, int fg, int bg, int direct, int hl, int rel);
void wclose (WIN * win, int replace);
void wleave (void);
void wreturn (void);
void wresize (WIN * w, int x, int y);
void wredraw (WIN * w, int newdirect);
void wscroll (WIN * win, int dir);
void wlocate (WIN * win, int x, int y);
void wdrawelm (WIN * win, int y, ELM * e);
void wputs (WIN * win, char *s);

/* Should use stdarg et al. */

#ifndef _WIN32                  /* HJK, 970805 */

int wprintf ();

#endif

void wbell (void);
void wcursor (WIN * win, int type);
void wtitle (WIN * w, int pos, char *s);
void wcurbar (WIN * w, int y, int attr);
int wselect (int x, int y, char **choices, void (**funlist) (),
             char *title, int attr, int fg, int bg);
void wclrel (WIN * w);
void wclreol (WIN * w);
void wclrbol (WIN * w);
void wclreos (WIN * w);
void wclrbos (WIN * w);
void winclr (WIN * w);
void winsline (WIN * w);
void wdelline (WIN * w);
void winschar (WIN * w);
void winschar2 (WIN * w, int c, int move);
void wdelchar (WIN * w);
int wgets (WIN * win, char *s, int linemax, int totmax);
void win_end (void);

#ifdef BBS

int win_init (char *term, int lines);

#else

int win_init (int fg, int bg, int attr);

#endif

/* Some macro's that can be used as functions */
#define wsetregion(w, z1, z2) (((w)->sy1=(w)->y1+(z1)),((w)->sy2=(w)->y1+(z2)))
#define wresetregion(w) ( (w)->sy1 = (w)->y1, (w)->sy2 = (w)->y2 )
#define wgetattr(w) ( (w)->attr )
#define wsetattr(w, a) ( (w)->attr = (a) )
#define wsetfgcol(w, fg) ( (w)->color = ((w)->color & 15) + ((fg) << 4))
#define wsetbgcol(w, bg) ( (w)->color = ((w)->color & 240) + (bg) )
#define wsetam(w) ( (w)->wrap = 1 )
#define wresetam(w) ( (w)->wrap = 0 )

/* macro's for our keyboard routines */
#define K_BS        8
#define K_ESC       27
#define K_STOP      256
#define K_F1        257
#define K_F2        258
#define K_F3        259
#define K_F4        260
#define K_F5        261
#define K_F6        262
#define K_F7        263
#define K_F8        264
#define K_F9        265
#define K_F10       266
#define K_HOME      267
#define K_PGUP      268
#define K_UP        269
#define K_LT        270
#define K_RT        271
#define K_DN        272
#define K_END       273
#define K_PGDN      274
#define K_INS       275
#define K_DEL       276
#define K_F11       277
#define K_F12       278

#define NUM_KEYS    23
#define KEY_OFFS    256

/* Here's where the meta keycode start. (512 + keycode). */
#define K_META      512

#ifndef EOF
#  define EOF       ((int) -1)
#endif
#define K_ERA       '\b'
#define K_KILL      ((int) -2)

/* Internal structure. */
struct key
{
  char *cap;
  char len;
};

/* window.h end ============================================================ */


/* Keypad and cursor key modes. */
#define NORMAL  1
#define APPL    2

/* Don't change - hardcoded in minicom's dial.c */
#define VT100   1
#define ANSI    3

/* Prototypes from vt100.c */
/* TE011097: moved them to prototyp.h */


/* config.h =============================================================== */

/* Operating system INdependant parameters. (Usually the same everywhere) */
#define LOGFILE     "minicom.log"  /* Not defined = not used */

/* This defines a special mode in the wwxgetch() routine. The
 * basic idea behind this probably works on the consoles of
 * most PC-based unices, but it's only implemented for Linux.
 */
#if defined (__linux__)
#  define KEY_KLUDGE 1
#endif

/* Define this if you want a history buffer. */
#define HISTORY 1

/* And this for the translation tables (vt100 -> ASCII) */
#if __STDC__
#  define TRANSLATE 1
#  define CONST const
#else
#  define TRANSLATE 0
#endif

/* config.h end =========================================================== */

/* $Id: vt100.h,v 1.4 1999/03/05 02:51:36 mr Exp $ */
