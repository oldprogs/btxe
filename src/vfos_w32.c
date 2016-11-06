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
 * Filename    : $Source: E:/cvs/btxe/src/vfos_w32.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/31 17:45:19 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Windows NT VFOSSIL module.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

extern VIOMODEINFO vfos_mode;
extern int vfossil_installed;

static unsigned long saved_key = 0xFFFFFFFF;
static HANDLE hConsoleInput = INVALID_HANDLE_VALUE;
static HANDLE hConsoleOutput = INVALID_HANDLE_VALUE;

void
Make_Sound (char *WaveFile)
{
  if (WaveFile == NULL)
    return;
  PlaySound (WaveFile, 0L, (SND_FILENAME | SND_ASYNC | SND_NOWAIT));
}

short
KBHit (void)
{
  int iKey = 0;
  INPUT_RECORD irBuffer;
  DWORD pcRead;

  if (saved_key != 0xFFFFFFFF)
  {
    return (short) (saved_key);
  }

  if (hConsoleInput == INVALID_HANDLE_VALUE)
  {
    hConsoleInput = GetStdHandle (STD_INPUT_HANDLE);
    if (hConsoleInput == INVALID_HANDLE_VALUE)
    {
      fprintf (stderr, "\nFailed to open console handle!\n");
      exit (3);
    }
  }

  while (1)
  {
    PeekConsoleInput (hConsoleInput, &irBuffer, 1, &pcRead);
    if (pcRead)
    {
      if (irBuffer.EventType == KEY_EVENT &&
          irBuffer.Event.KeyEvent.bKeyDown == TRUE &&
          irBuffer.Event.KeyEvent.wRepeatCount <= 5)
      {
        unsigned short vk;
        unsigned short vs;
        unsigned char uc;
        BOOL fShift;
        BOOL fAlt;
        BOOL fCtrl;

        vk = irBuffer.Event.KeyEvent.wVirtualKeyCode;
        vs = irBuffer.Event.KeyEvent.wVirtualScanCode;
        uc = irBuffer.Event.KeyEvent.uChar.AsciiChar;

        //HJK 980805 For come strange keyboard error on some keyboards
        if ((vs == 0x2c) && (toupper (uc) == 'Y'))
          vs = 0x15;
        else if ((vs == 0x15) && (toupper (uc) == 'Z'))
          vs = 0x2c;

        fShift = (irBuffer.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED));
        fAlt = (irBuffer.Event.KeyEvent.dwControlKeyState &
                (RIGHT_ALT_PRESSED + LEFT_ALT_PRESSED));
        fCtrl = (irBuffer.Event.KeyEvent.dwControlKeyState &
                 (RIGHT_CTRL_PRESSED + LEFT_CTRL_PRESSED));

        /* The following is ugly, incomplete and nonportable.
         ** It mostly handles Fkeys; Alt+Fkeys; Shift+FKeys;
         ** Printable charactes; Alt+printable characters;
         ** Ctrl+printable characters.
         */

        if (uc == 0)            /* FKeys */
        {
          if (vk == 0x21)       /* PG UP */
          {
            if (fCtrl)          /* Special case     */
              vs = 0x84;        /* CTRL+PG UP       */
          }
          else if (vk == 0x22)  /* PG DN */
          {
            if (fCtrl)          /* Special case     */
              vs = 0x76;        /* CTRL+PG DN       */
          }
          else if (vk == 0x23)  /* END */
          {
            if (fCtrl)          /* Special case     */
              vs = 0x75;        /* CTRL+END         */
          }
          else if (vk == 0x24)  /* HOME */
          {
            if (fCtrl)          /* Special case     */
              vs = 0x77;        /* CTRL+HOME        */
          }
          else if (vk == 0x26)  /* UP AR */
          {
            if (fCtrl)          /* Special case     */
              vs = 0x8D;        /* CTRL+UP AR       */
          }
          else if (vk == 0x28)  /* DN AR */
          {
            if (fCtrl)          /* Special case     */
              vs = 0x91;        /* CTRL+DN AR       */
          }
          else if (vk >= 0x70 && vk <= 0x79)  /* FKeys */
          {
            if (fAlt)
              vs += 0x2d;       /* Hack Alt+FKey    */
            else if (fShift)
              vs += 0x19;       /* Hack Shift+Fkey  */
          }
          if (vk > 0x20 && vk < 0x92)
          {                     /* If it's OK   */
            iKey = (vs << 8);   /* use scan code    */
          }
        }
        else
        {
          if (fAlt)             /* Hack Alt Key     */
            iKey = (vs << 8);
          else if (fCtrl)       /* Hack Ctrl Key    */
            iKey = (vk & 0xBF);
          else
            iKey = uc;
        }
      }

      ReadConsoleInput (hConsoleInput, &irBuffer, 1, &pcRead);
    }

    break;
  }

  if (iKey != 0)
    saved_key = iKey;

  return (short) (iKey);
}

short
GetKBKey (void)
{
  int iKey;

  while (saved_key == 0xFFFFFFFF)
  {
    KBHit ();
    Sleep (1);
  }

  iKey = saved_key;
  saved_key = 0xFFFFFFFF;
  return (short) (iKey);
}

void
vfossil_init (void)
{
  PVIOMODEINFO q = &vfos_mode;
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
  {
    hConsoleOutput = GetStdHandle (STD_OUTPUT_HANDLE);
    if (hConsoleOutput == INVALID_HANDLE_VALUE)
    {
      fprintf (stderr, "\nFailed to open console handle!\n");
      exit (3);
    }
  }

  if (!GetConsoleScreenBufferInfo (hConsoleOutput, &csbi))
    return;

  memset ((void *) q, 0, sizeof (VIOMODEINFO));
  q->cb = sizeof (VIOMODEINFO);
  q->col = csbi.dwSize.X;
  q->row = csbi.dwSize.Y;
  vfossil_installed = 1;
}

void
vfossil_cursor (int st)
{
  CONSOLE_CURSOR_INFO cci;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  if (!GetConsoleCursorInfo (hConsoleOutput, &cci))
    return;

  if (cci.bVisible != (st != 0))
  {
    cci.bVisible = (st != 0);
    SetConsoleCursorInfo (hConsoleOutput, &cci);
  }
}

void
vfossil_close (void)
{
  vfossil_cursor (1);
}

void
fossil_gotoxy (int col, int row)
{
  COORD dwCursorPosition;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  dwCursorPosition.X = col;
  dwCursorPosition.Y = row;

  SetConsoleCursorPosition (hConsoleOutput, dwCursorPosition);
}

int
fossil_wherex (void)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  if (!GetConsoleScreenBufferInfo (hConsoleOutput, &csbi))
    return 1;

  return csbi.dwCursorPosition.X;
}

int
fossil_wherey (void)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  if (!GetConsoleScreenBufferInfo (hConsoleOutput, &csbi))
    return 1;

  return csbi.dwCursorPosition.Y;
}

USHORT
VioWrtTTY (PCH pchString, USHORT cbString, USHORT hVio)
{
  DWORD dwCount;
  DWORD dwWritten;
  BOOL fResult;

  hVio = hVio;                  // make compiler happy

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  dwCount = cbString;
  fResult = WriteConsole (hConsoleOutput, pchString, dwCount, &dwWritten, NULL);
  return (fResult != TRUE);
}

/* Raise max to whatever you need. Change 500 to n and 1000 to 2n. */

USHORT
VioWrtCellStr (PCH pchCellString, USHORT cbCellString, USHORT usRow,
               USHORT usColumn, USHORT hVio)
{
  DWORD dwWritten;
  BOOL fResult = TRUE;
  static char vchars[500];
  static WORD vattrs[500];
  COORD dwCursorPosition;

  int i, count;
  char *p;
  unsigned char *s = (unsigned char *) pchCellString;
  WORD *q;

  hVio = hVio;                  // make compiler happy

  if (cbCellString > 1000)
    return 1;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  dwCursorPosition.X = usColumn;
  dwCursorPosition.Y = usRow;

  count = cbCellString >> 1;
  for (p = vchars, q = vattrs, i = 0; i < count; i++)
  {
    *p++ = (char) (*s++);
    *q++ = (WORD) (*s++);
  }

  fResult = WriteConsoleOutputCharacter (hConsoleOutput,
                                         vchars,
                                         count,
                                         dwCursorPosition,
                                         &dwWritten);

  if (fResult)
  {
    fResult = WriteConsoleOutputAttribute (hConsoleOutput,
                                           vattrs,
                                           count,
                                           dwCursorPosition,
                                           &dwWritten);
  }

  return (fResult != TRUE);
}

/* $Id: vfos_w32.c,v 1.4 1999/03/31 17:45:19 hjk Exp $ */
