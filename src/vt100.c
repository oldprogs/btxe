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
 * Filename    : $Source: E:/cvs/btxe/src/vt100.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:27 $
 * State       : $State: Exp $
 * Orig. Author: Miquel van Smoorenburg
 *
 * Description : ANSI/VT102 emulator code.
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

#include "includes.h"
#include "vt100.h"

#define OLD 0

/*
 * The global variable esc_s holds the escape sequence status:
 * 0 - normal
 * 1 - ESC
 * 2 - ESC [
 * 3 - ESC [ ?
 * 4 - ESC (
 * 5 - ESC )
 * 6 - ESC #
 * 7 - ESC P
 */
static int esc_s = 0;

// #define ESC 27   /* already in zmodem.h */

/* Structure to hold escape sequences. */
struct escseq
{
  int code;
  char *vt100_st;
  char *vt100_app;
  char *ansi;
};

/* Escape sequences for different terminal types. */
static struct escseq vt_keys[] =
{
#ifndef _DGUX_SOURCE
  {K_F1, "OP", "OP", "OP"},
  {K_F2, "OQ", "OQ", "OQ"},
  {K_F3, "OR", "OR", "OR"},
  {K_F4, "OS", "OS", "OS"},
  {K_F5, "[16~", "[16~", "OT"},
  {K_F6, "[17~", "[17~", "OU"},
  {K_F7, "[18~", "[18~", "OV"},
  {K_F8, "[19~", "[19~", "OW"},
  {K_F9, "[20~", "[20~", "OX"},
  {K_F10, "[21~", "[21~", "OY"},
  {K_F11, "[23~", "[23~", "OY"},
  {K_F12, "[24~", "[24~", "OY"},
  {K_HOME, "[1~", "[1~", "[H"},
  {K_PGUP, "[5~", "[5~", "[V"},
  {K_UP, "[A", "OA", "[A"},
  {K_LT, "[D", "OD", "[D"},
  {K_RT, "[C", "OC", "[C"},
  {K_DN, "[B", "OB", "[B"},
  {K_END, "[4~", "[4~", "[Y"},
  {K_PGDN, "[6~", "[6~", "[U"},
  {K_INS, "[2~", "[2~", "[@"},
  {K_DEL, "[3~", "[3~", "\177"},
  {0, NULL, NULL, NULL}
#else
  {K_F1, "[17~", "[17~", "OP"},
  {K_F2, "[18~", "[18~", "OQ"},
  {K_F3, "[19~", "[19~", "OR"},
  {K_F4, "[20~", "[20~", "OS"},
  {K_F5, "[21~", "[21~", "OT"},
  {K_F6, "[23~", "[23~", "OU"},
  {K_F7, "[24~", "[24~", "OV"},
  {K_F8, "[25~", "[25~", "OW"},
  {K_F9, "[26~", "[26~", "OX"},
  {K_F10, "[28~", "[28~", "OY"},
  {K_F11, "[29~", "[29~", "OZ"},
  {K_F12, "[31~", "[31~", "OA"},
  {K_HOME, "OP", "OP", "[H"},
  {K_PGUP, "OQ", "OQ", "[V"},
  {K_UP, "[A", "OA", "[A"},
  {K_LT, "[D", "OD", "[D"},
  {K_RT, "[C", "OC", "[C"},
  {K_DN, "[B", "OB", "[B"},
  {K_END, "OR", "OR", "[Y"},
  {K_PGDN, "OS", "OS", "[U"},
  {K_INS, "[1~", "[1~", "[@"},
  {K_DEL, "[3~", "[3~", "\177"},
  {0, NULL, NULL, NULL}
#endif
};

#if TRANSLATE

/* Taken from the Linux kernel source: linux/drivers/char/console.c */
static unsigned char *vt_map[] =
{
  /* 8-bit Latin-1 mapped to the PC character set: '.' means non-printable */
  (unsigned char *)
  "................"
  "................"
  " !\"#$%&'()*+,-./0123456789:;<=>?"
  "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
  "`abcdefghijklmnopqrstuvwxyz{|}~."
  "................"
  "................"
  "\377\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
  "\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
  "\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
  "\376\245\376\376\376\376\231\376\235\376\376\376\232\376\376\341"
  "\205\240\203\376\204\206\221\207\212\202\210\211\215\241\214\213"
  "\376\244\225\242\223\376\224\366\233\227\243\226\201\376\376\230",
  /* vt100 graphics */
  (unsigned char *)
  "................"
  "................"
  " !\"#$%&'()*+,-./0123456789:;<=>?"
  "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^ "
  "\004\261\376\376\376\376\370\361\040\376\331\277\332\300\305\376"
  "\376\304\376\376\303\264\301\302\263\376\376\376\376\376\234."
  "................"
  "................"
  "\040\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
  "\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
  "\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
  "\376\245\376\376\376\376\231\376\376\376\376\376\232\376\376\341"
  "\205\240\203\376\204\206\221\207\212\202\210\211\215\241\214\213"
  "\376\244\225\242\223\376\224\366\376\227\243\226\201\376\376\230"
};

static char *vt_trans[2];
static int vt_charset = 0;      /* Character set. */

#endif

#ifdef _SELECT

static int vt_echo;             /* Local echo on/off. */

#endif

static int vt_type = ANSI;      /* Terminal type. */
static int vt_wrap = 0;         /* Line wrap on/off */
static int vt_addlf = 0;        /* Add linefeed on/off */
static int vt_fg;               /* Standard foreground color. */
static int vt_bg;               /* Standard background color. */
static int vt_keypad;           /* Keypad mode. */
static int vt_cursor;           /* cursor key mode. */
static int vt_bs = 8;           /* Code that backspace key sends. */
static int vt_insert = 0;       /* Insert mode */
static int vt_crlf = 0;         /* Return sends CR/LF */
static int vt_om;               /* Origin mode. */
REGIONP vt_win = NULL;          /* Output window. */
static int vt_docap;            /* Capture on/off. */
static FILE *vt_capfp;          /* Capture file. */
static void (*vt_keyb) ();      /* Gets called for NORMAL/APPL switch. */
static void (*termout) ();      /* Gets called to output a string. */
static int escparms[8];         /* Cumulated escape sequence. */

#if OLD

static int ptr = -2;            /* Index into escparms array. */

#else

static int ptr = 0;             /* Index into escparms array. */

#endif

static long vt_tabs[5];         /* Tab stops for max. 32*5 = 160 columns. */

short newy1 = 0;                /* Current size of scrolling region. */
short newy2 = 23;

/* Saved color and posistions */
static short savex = 0, savey = 0;

#if TRANSLATE

static short savecharset;
static char *savetrans[2];

#endif

/*
 * Initialize the emulator once.
 */
void
vt_install (void (*fun1) (), void (*fun2) (), REGIONP win)
{
  termout = fun1;
  vt_keyb = fun2;
  vt_win = win;
}

/* Partial init (after screen resize) */
void
vt_pinit (REGIONP win, int fg, int bg)
{
  vt_win = win;
  newy1 = 0;
  //newy2 = vt_win->ys - 1;
  //wresetregion(vt_win);
  if (fg > 0)
    vt_fg = fg;
  if (bg > 0)
    vt_bg = bg;
  //wsetfgcol(vt_win, vt_fg);
  //wsetbgcol(vt_win, vt_bg);
}

/* Set characteristics of emulator. */
void
vt_init (int type, int fg, int bg, int wrap, int add)
{
  vt_type = type;
  if (vt_type == ANSI)
  {
    vt_fg = WHITE;
    vt_bg = BLACK;
  }
  else
  {
    vt_fg = fg;
    vt_bg = bg;
  }

  //if (wrap >= 0) vt_win->wrap = vt_wrap = wrap;
  wrap = wrap;
  vt_addlf = add;
  vt_insert = 0;
  vt_crlf = 0;
  vt_om = 0;

  newy1 = 0;
  //newy2 = vt_win->ys - 1;
  //wresetregion(vt_win);
  vt_keypad = NORMAL;
  vt_cursor = NORMAL;

#ifdef _SELECT
  vt_echo = 0;
#endif

  vt_tabs[0] = 0x01010100;
  vt_tabs[1] = vt_tabs[2] = vt_tabs[3] = vt_tabs[4] = 0x01010101;

#if TRANSLATE
  vt_charset = 0;
  vt_trans[0] = vt_map[0];
  vt_trans[1] = vt_map[1];
#endif

#if OLD
  ptr = -2;
#else
  ptr = 0;
  memset (escparms, 0, sizeof (escparms));
#endif

  esc_s = 0;

  if (vt_keyb)
    (*vt_keyb) (vt_keypad, vt_cursor);
  //wsetfgcol(vt_win, vt_fg);
  //wsetbgcol(vt_win, vt_bg);
}

/* Change some things on the fly. */
void
vt_set (int addlf, int wrap,
        FILE * capfp, int docap, int bscode, int echo, int cursor)
{
  if (addlf >= 0)
    vt_addlf = addlf;
  //if (wrap >= 0)  vt_win->wrap = vt_wrap = wrap;
  wrap = wrap;
  if (capfp != (FILE *) 0)
    vt_capfp = capfp;
  if (docap >= 0)
    vt_docap = docap;
  if (bscode >= 0)
    vt_bs = bscode;

#ifdef _SELECT
  if (echo >= 0)
    vt_echo = echo;
#else
  echo = echo;
#endif

  if (cursor >= 0)
    vt_cursor = cursor;
}

/* Output a string to the modem. */
static void
v_termout (char *s, int len)
{
#ifdef _SELECT
  char *p;

  if (vt_echo)
  {
    for (p = s; *p; p++)
    {
      vt_out (*p);
      if (!vt_addlf && *p == '\r')
        vt_out ('\n');
    }
    wflush ();
  }
#endif
  (*termout) (s, len);
}

/*
 * Escape code handling.
 */

/*
 * ESC was seen the last time. Process the next character.
 */
static void
state1 (int c)
{
  // short x, y, f;
  short x, y;

  switch (c)
  {
  case '[':                    /* ESC [ */
    esc_s = 2;
    return;

  case '(':                    /* ESC ( */
    esc_s = 4;
    return;

  case ')':                    /* ESC ) */
    esc_s = 5;
    return;

  case '#':                    /* ESC # */
    esc_s = 6;
    return;

  case 'P':                    /* ESC P (DCS, Device Control String) */
    esc_s = 7;
    return;

  case 'D':                    /* Cursor down */
  case 'M':                    /* Cursor up */
    x = vt_win->col;
    if (c == 'D')
    {                           /* Down. */
      y = vt_win->row + 1;
      // if (y == newy2 + 1)
      //   wscroll(vt_win, S_UP);
      // else if (vt_win->row < vt_win->ys)
      sb_move (vt_win, y, x);
    }

    if (c == 'M')
    {                           /* Up. */
      y = vt_win->row - 1;
      // if (y == newy1 - 1)
      //   wscroll(vt_win, S_DOWN);
      // else if (y >= 0)
      sb_move (vt_win, y, x);
    }
    break;

  case 'E':                    /* CR + NL */
    sb_puts (vt_win, "\r\n");
    break;

  case '7':                    /* Save attributes and cursor position */
  case 's':
    savex = vt_win->col;
    savey = vt_win->row;
    // saveattr = vt_win->attr;
    // savecol = vt_win->color;

#if TRANSLATE
    savecharset = vt_charset;
    savetrans[0] = vt_trans[0];
    savetrans[1] = vt_trans[1];
#endif
    break;

  case '8':                    /* Restore them */
  case 'u':
#if TRANSLATE
    vt_charset = savecharset;
    vt_trans[0] = savetrans[0];
    vt_trans[1] = savetrans[1];
#endif
    // vt_win->color = savecol; /* HACK should use wsetfgcol etc */
    // wsetattr(vt_win, saveattr);
    // wlocate(vt_win, savex, savey);
    break;

  case '=':                    /* Keypad into applications mode */
    vt_keypad = APPL;
    if (vt_keyb)
      (*vt_keyb) (vt_keypad, vt_cursor);
    break;

  case '>':                    /* Keypad into numeric mode */
    vt_keypad = NORMAL;
    if (vt_keyb)
      (*vt_keyb) (vt_keypad, vt_cursor);
    break;

  case 'Z':                    /* Report terminal type */
    if (vt_type == VT100)
      v_termout ("\033[?1;0c", 0);
    else
      v_termout ("\033[?c", 0);
    break;

  case 'c':                    /* Reset to initial state */
    // f = XA_NORMAL;
    // wsetattr(vt_win, f);
    // vt_win->wrap = (vt_type != VT100);
    // if (vt_wrap != -1) vt_win->wrap = vt_wrap;
    // vt_crlf = vt_insert = 0;
    vt_init (vt_type, vt_fg, vt_bg, vt_wrap, 0);
    sb_move (vt_win, 0, 0);
    break;

  case 'H':                    /* Set tab in current position */
    x = vt_win->col;
    if (x > 159)
      x = 159;
    vt_tabs[x / 32] |= 1 << (x % 32);
    break;

  case 'N':                    /* G2 character set for next character only */
  case 'O':                    /* G3 "               "    */
  case '<':                    /* Exit vt52 mode */
  default:
    /* ALL IGNORED */
    break;
  }

  esc_s = 0;
  return;
}

/* ESC [ ... [hl] seen. */
static void
ansi_mode (int on_off)
{
  int i;

  for (i = 0; i <= ptr; i++)
  {
    switch (escparms[i])
    {
    case 4:                    /* Insert mode  */
      vt_insert = on_off;
      break;

    case 20:                   /* Return key mode */
      vt_crlf = on_off;
      break;
    }
  }
}

/*
 * ESC [ ... was seen the last time. Process next character.
 */
static void
state2 (int c)
{
  short x, y, attr, f;
  char temp[32];

  /* See if a number follows */
  if (c >= '0' && c <= '9')
  {
#if OLD
    if (ptr < 0)
      ptr = 0;
#endif
    escparms[ptr] = 10 * (escparms[ptr]) + c - '0';
    return;
  }

  /* Separation between numbers ? */
  if (c == ';')
  {
#if OLD
    if (ptr < 0)
      ptr = 0;                  /* keithr@primenet.com */
    if (ptr >= 0 && ptr < 15)
      ptr++;
#else
    if (ptr < 15)
      ptr++;
#endif
    return;
  }

  /* ESC [ ? sequence */
#if OLD
  if (ptr < 0 && c == '?')
  {
#else
  if (escparms[0] == 0 && ptr == 0 && c == '?')
  {
#endif
    esc_s = 3;
    return;
  }

  /* Process functions with zero, one, two or more arguments */
  switch (c)
  {
  case 'A':
  case 'B':
  case 'C':
  case 'D':                    /* Cursor motion */
    if ((f = escparms[0]) == 0)
      f = 1;
    x = vt_win->col;
    y = vt_win->row;
    x += f * ((c == 'C') - (c == 'D'));
    if (x < 0)
      x = 0;
    if (x > vt_win->sc1)
      x = vt_win->sc1;

    if (c == 'B')
    {                           /* Down. */
      y += f;
      if (y > vt_win->sr1)
        y = vt_win->sr1;
      // if (y >= newy2 + 1) y = newy2;
    }

    if (c == 'A')
    {                           /* Up. */
      y -= f;
      if (y < 0)
        y = 0;
      // if (y <= newy1 - 1) y = newy1;
    }

    sb_move (vt_win, y, x);
    break;

  case 'K':                    /* Line erasing */
    switch (escparms[0])
    {
    case 0:
      // wclreol(vt_win);
      break;
    case 1:
      // wclrbol(vt_win);
      break;
    case 2:
      // wclrel(vt_win);
      break;
    }
    break;

  case 'J':                    /* Screen erasing */
    // x = vt_win->color;
    // y = vt_win->attr;
    // if (vt_type == ANSI) {
    //   wsetattr(vt_win, XA_NORMAL);
    //   wsetfgcol(vt_win, WHITE);
    //   wsetbgcol(vt_win, BLACK);
    // }
    switch (escparms[0])
    {
    case 0:
      // wclreos(vt_win);
      break;

    case 1:
      // wclrbos(vt_win);
      break;

    case 2:
      sb_fillc (vt_win, ' ');
      break;
    }

    // if (vt_type == ANSI) {
    //   vt_win->color = x;
    //   vt_win->attr = y;
    // }
    break;

  case 'n':                    /* Requests / Reports */
    switch (escparms[0])
    {
    case 5:                    /* Status */
      v_termout ("\033[0n", 0);
      break;

    case 6:                    /* Cursor Position */
      sprintf (temp, "\033[%d;%dR", vt_win->row + 1, vt_win->col + 1);
      v_termout (temp, 0);
      break;
    }
    break;

  case 'c':                    /* Identify Terminal Type */
    if (vt_type == VT100)
    {
      v_termout ("\033[?1;2c", 0);
      break;
    }
    v_termout ("\033[?c", 0);
    break;

  case 'x':                    /* Request terminal parameters. */
    /* Always answers 19200-8N1 no options. */
    sprintf (temp, "\033[%c;1;1;120;120;1;0x",
             escparms[0] == 1 ? '3' : '2');
    v_termout (temp, 0);
    break;

  case 's':                    /* Save attributes and cursor position */
    savex = vt_win->col;
    savey = vt_win->row;
    // saveattr = vt_win->attr;
    // savecol = vt_win->color;
#if TRANSLATE
    savecharset = vt_charset;
    savetrans[0] = vt_trans[0];
    savetrans[1] = vt_trans[1];
#endif
    break;

  case 'u':                    /* Restore them */
#if TRANSLATE
    vt_charset = savecharset;
    vt_trans[0] = savetrans[0];
    vt_trans[1] = savetrans[1];
#endif
    break;
    // vt_win->color = savecol; /* HACK should use wsetfgcol etc */
    // wsetattr(vt_win, saveattr);
    // sb_move(vt_win, savey, savex);
    // break;

  case 'h':
    ansi_mode (1);
    break;

  case 'l':
    ansi_mode (0);
    break;

  case 'H':
  case 'f':                    /* Set cursor position */
    if ((y = escparms[0]) == 0)
      y = 1;
    if ((x = escparms[1]) == 0)
      x = 1;
    if (vt_om)
      y += newy1;
    sb_move (vt_win, y - 1, x - 1);
    break;

  case 'g':                    /* Clear tab stop(s) */
    if (escparms[0] == 0)
    {
      x = vt_win->col;
      if (x > 159)
        x = 159;
      vt_tabs[x / 32] &= ~(1 << x % 32);
    }

    if (escparms[0] == 3)
      for (x = 0; x < 5; x++)
        vt_tabs[x] = 0;
    break;

  case 'm':                    /* Set attributes */
#if OLD
    /* Without argument, esc-parms[0] is 0 */
    if (ptr < 0)
      ptr = 0;
#endif
    // attr = wgetattr((vt_win));
    for (f = 0; f <= ptr; f++)
    {
      // if (escparms[f] >= 30 && escparms[f] <= 37)
      //   wsetfgcol(vt_win, escparms[f] - 30);
      // if (escparms[f] >= 40 && escparms[f] <= 47)
      //   wsetbgcol(vt_win, escparms[f] - 40);
      switch (escparms[f])
      {
      case 0:
        attr = XA_NORMAL;
        // wsetfgcol(vt_win, vt_fg);
        // wsetbgcol(vt_win, vt_bg);
        break;

      case 4:
        attr |= XA_UNDERLINE;
        break;

      case 7:
        attr |= XA_REVERSE;
        break;

      case 1:
        attr |= XA_BOLD;
        break;

      case 5:
        attr |= XA_BLINK;
        break;

      case 22:                 /* Bold off */
        attr &= ~XA_BOLD;
        break;

      case 24:                 /* Not underlined */
        attr &= ~XA_UNDERLINE;
        break;

      case 25:                 /* Not blinking */
        attr &= ~XA_BLINK;
        break;

      case 27:                 /* Not reverse */
        attr &= ~XA_REVERSE;
        break;

      case 39:                 /* Default fg color */
        // wsetfgcol(vt_win, vt_fg);
        break;

      case 49:                 /* Default bg color */
        // wsetbgcol(vt_win, vt_bg);
        break;
      }
    }
    // wsetattr(vt_win, attr);
    break;

  case 'L':                    /* Insert lines */
    if ((x = escparms[0]) == 0)
      x = 1;
    // for(f = 0; f < x; f++)
    //   winsline(vt_win);
    break;

  case 'M':                    /* Delete lines */
    if ((x = escparms[0]) == 0)
      x = 1;
    // for(f = 0; f < x; f++)
    // wdelline(vt_win);
    break;

  case 'P':                    /* Delete Characters */
    if ((x = escparms[0]) == 0)
      x = 1;
    // for(f = 0; f < x; f++)
    //   wdelchar(vt_win);
    break;

  case '@':                    /* Insert Characters */
    if ((x = escparms[0]) == 0)
      x = 1;
    // for(f = 0; f < x; f++)
    //   winschar(vt_win);
    break;

  case 'r':                    /* Set scroll region */
    // if ((newy1 = escparms[0]) == 0) newy1 = 1;
    // if ((newy2 = escparms[1]) == 0) newy2 = vt_win->ys;
    // newy1-- ; newy2--;
    // if (newy1 < 0) newy1 = 0;
    // if (newy2 < 0) newy2 = 0;
    // if (newy1 >= vt_win->ys) newy1 = vt_win->ys - 1;
    // if (newy2 >= vt_win->ys) newy2 = vt_win->ys - 1;
    // if (newy1 >= newy2) {
    //     newy1 = 0;
    //     newy2 = vt_win->ys - 1;
    // }
    // wsetregion(vt_win, newy1, newy2);
    // wlocate(vt_win, 0, newy1);
    break;

  case 'i':                    /* Printing */
  case 'y':                    /* Self test modes */
  default:
    /* IGNORED */
    break;
  }

  /* Ok, our escape sequence is all done */
  esc_s = 0;
#if OLD
  ptr = -2;
#else
  ptr = 0;
  memset (escparms, 0, sizeof (escparms));
#endif

  return;
}

/* ESC [? ... [hl] seen. */
static void
dec_mode (int on_off)
{
  int i;

  for (i = 0; i <= ptr; i++)
  {
    switch (escparms[i])
    {
    case 1:                    /* Cursor keys in cursor/appl mode */
      vt_cursor = on_off ? APPL : NORMAL;
      if (vt_keyb)
        (*vt_keyb) (vt_keypad, vt_cursor);
      break;

    case 6:                    /* Origin mode. */
      vt_om = on_off;
      // wlocate(vt_win, 0, newy1);
      break;

    case 7:                    /* Auto wrap */
      // vt_win->wrap = on_off;
      break;

    case 25:                   /* Cursor on/off */
      // wcursor(vt_win, on_off ? CNORMAL : CNONE);
      break;

    case 67:                   /* Backspace key sends. (FIXME: vt420) */
      /* setbackspace(on_off ? 8 : 127); */
      break;

    default:                   /* Mostly set up functions */
      /* IGNORED */
      break;
    }
  }
}

/*
 * ESC [ ? ... seen.
 */
static void
state3 (int c)
{
  /* See if a number follows */
  if (c >= '0' && c <= '9')
  {
#if OLD
    if (ptr < 0)
      ptr = 0;
#endif
    escparms[ptr] = 10 * (escparms[ptr]) + c - '0';
    return;
  }

#if OLD
  /* ESC [ ? number seen */
  if (ptr < 0)
  {
    esc_s = 0;
    return;
  }
#endif

  switch (c)
  {
  case 'h':
    dec_mode (1);
    break;

  case 'l':
    dec_mode (0);
    break;

  case 'i':                    /* Printing */
  case 'n':                    /* Request printer status */
  default:
    /* IGNORED */
    break;
  }

  esc_s = 0;
#if OLD
  ptr = -2;
#else
  ptr = 0;
  memset (escparms, 0, sizeof (escparms));
#endif

  return;
}

/*
 * ESC ( Seen.
 */
static void
state4 (int c)
{
  /* Switch Character Sets. */
#if !TRANSLATE
  /* IGNORED */
  (void) c;
#else
  switch (c)
  {
  case 'A':
  case 'B':
    vt_trans[0] = vt_map[0];
    break;

  case '0':
  case 'O':
    vt_trans[0] = vt_map[1];
    break;
  }
#endif

  esc_s = 0;
}

/*
 * ESC ) Seen.
 */
static void
state5 (int c)
{
  /* Switch Character Sets. */
#if !TRANSLATE
  /* IGNORED */
  (void) c;
#else
  switch (c)
  {
  case 'A':
  case 'B':
    vt_trans[1] = vt_map[0];
    break;

  case 'O':
  case '0':
    vt_trans[1] = vt_map[1];
    break;
  }
#endif

  esc_s = 0;
}

/*
 * ESC # Seen.
 */
static void
state6 (int c)
{
  //int x, y;

  /* Double height, double width and selftests. */
  switch (c)
  {
    //  case '8':
    //      /* Selftest: fill screen with E's */
    //      vt_win->doscroll = 0;
    //      vt_win->direct = 0;
    //      wlocate(vt_win, 0, 0);
    //      for(y = 0; y < vt_win->ys; y++) {
    //          wlocate(vt_win, 0, y);
    //          for(x = 0; x < vt_win->xs; x++)
    //              sb_putc(vt_win, 'E');
    //      }
    //      wlocate(vt_win, 0, 0);
    //      vt_win->doscroll = 1;
    //      wredraw(vt_win, 1);
    //      break;

  default:
    /* IGNORED */
    break;
  }
  esc_s = 0;
}

/*
 * ESC P Seen.
 */
static void
state7 (int c)
{
  /*
   * Device dependant control strings. The Minix virtual console package
   * uses these sequences. We can only turn cursor on or off, because
   * that's the only one supported in termcap. The rest is ignored.
   */
  static char buf[17];
  static int pos = 0;
  static int state = 0;

  if (c == ESC)
  {
    state = 1;
    return;
  }
  if (state == 1)
  {
    buf[pos] = 0;
    pos = 0;
    state = 0;
    esc_s = 0;
    if (c != '\\')
      return;

    /* Process string here! */
    // if (!strcmp(buf, "cursor.on")) wcursor(vt_win, CNORMAL);
    // if (!strcmp(buf, "cursor.off")) wcursor(vt_win, CNONE);
    if (!strcmp (buf, "linewrap.on"))
    {
      vt_wrap = -1;
      // vt_win->wrap = 1;
    }

    if (!strcmp (buf, "linewrap.off"))
    {
      vt_wrap = -1;
      // vt_win->wrap = 0;
    }

    return;
  }

  if (pos > 15)
    return;

  buf[pos++] = c;
}

void
vt_out (int ch)
{
  int f;
  unsigned char c;
  int go_on = 0;

  if (!ch)
    return;

#if OLD
  if (ptr == -2)
  {                             /* Initialize */
    ptr = -1;
    for (f = 0; f < 8; f++)
      escparms[f] = 0;
  }
#endif

  c = (unsigned char) ch;

  if (vt_docap == 2)            /* Literal. */
    fputc (c, vt_capfp);

  /* Process <31 chars first, even in an escape sequence. */
  switch (c)
  {
  case 5:                      /* AnswerBack for vt100's */
    if (vt_type != VT100)
    {
      go_on = 1;
      break;
    }

    // v_termout(SCCS_ID, 0);
    // removed by TJW

    break;

  case '\r':                   /* Carriage return */
    sb_putc (vt_win, c);
    if (vt_addlf)
    {
      sb_putc (vt_win, '\n');
      if (vt_docap == 1)
        fputc ('\n', vt_capfp);
    }
    break;

  case '\t':                   /* Non - destructive TAB */
    /* Find next tab stop. */
    for (f = vt_win->col + 1; f < 160; f++)
      if (vt_tabs[f / 32] & (1 << f % 32))
        break;
    if (f > vt_win->sc1)
      f = vt_win->sc1;
    sb_move (vt_win, vt_win->row, f);
    if (vt_docap == 1)
      fputc (c, vt_capfp);
    break;

  case 013:                    /* Old Minix: CTRL-K = up */
    sb_move (vt_win, vt_win->row - 1, vt_win->col);
    break;

  case '\f':                   /* Form feed: clear screen. */
    sb_fillc (vt_win, ' ');
    sb_move (vt_win, 0, 0);
    break;

#if !TRANSLATE
  case 14:
  case 15:                     /* Change character set. Not supported. */
    break;
#else

  case 14:
    vt_charset = 1;
    break;

  case 15:
    vt_charset = 0;
    break;
#endif

  case 24:
  case 26:                     /* Cancel escape sequence. */
    esc_s = 0;
    break;

  case ESC:                    /* Begin escape sequence */
    esc_s = 1;
    break;

  case 128 + ESC:              /* Begin ESC [ sequence. */
    esc_s = 2;
    break;

  case '\n':
  case '\b':
  case 7:                      /* Bell */
    sb_putc (vt_win, c);
    if (vt_docap == 1)
      fputc (c, vt_capfp);
    break;

  default:
    go_on = 1;
    break;
  }

  if (!go_on)
    return;

  /* Now see which state we are in. */
  switch (esc_s)
  {
  case 0:                      /* Normal character */
#if TRANSLATE
    if (vt_type == VT100)
    {
      // if (vt_insert)
      //   winschar2(vt_win, vt_trans[vt_charset][c], 1);
      // else
      sb_putc (vt_win, vt_trans[vt_charset][c]);
    }
    else
    {
#endif
      // if (vt_insert)
      //   winschar2(vt_win, c, 1);
      // else
      sb_putc (vt_win, c);
#if TRANSLATE
    }
#endif
    if (vt_docap == 1)
      fputc (c, vt_capfp);
    break;

  case 1:                      /* ESC seen */
    state1 (c);
    break;

  case 2:                      /* ESC [ ... seen */
    state2 (c);
    break;

  case 3:
    state3 (c);
    break;

  case 4:
    state4 (c);
    break;

  case 5:
    state5 (c);
    break;

  case 6:
    state6 (c);
    break;

  case 7:
    state7 (c);
    break;
  }

  sb_show ();
}

/* Translate keycode to escape sequence. */
void
vt_send (int c)
{
  char s[3];
  int f;
  int len = 1;

  /* Special key? */
  if (c < 256)
  {
    /* Translate backspace key? */
    if (c == K_ERA)
      c = vt_bs;
    s[0] = c;
    s[1] = 0;

    /* CR/LF mode? */
    if (c == '\r' && vt_crlf)
    {
      s[1] = '\n';
      s[2] = 0;
      len = 2;
    }

    v_termout (s, len);
    return;
  }

  /* Look up code in translation table. */
  for (f = 0; vt_keys[f].code; f++)
    if (vt_keys[f].code == c)
      break;

  if (vt_keys[f].code == 0)
    return;

  /* Now send appropriate escape code. */
  v_termout ("\033", 0);
  if (vt_type == VT100)
  {
    if (vt_cursor == NORMAL)
      v_termout (vt_keys[f].vt100_st, 0);
    else
      v_termout (vt_keys[f].vt100_app, 0);
  }
  else
    v_termout (vt_keys[f].ansi, 0);
}

/* $Id: vt100.c,v 1.4 1999/02/27 01:16:27 mr Exp $ */
