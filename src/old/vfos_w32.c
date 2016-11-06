// Comment Henk-Jan 99/03/31:
// It is not my intention to use these routines, or to become more OS/2 capable
// So I removed them from the original source and leave them here, I think they
// can be useful, they are good routines.

/*
 * alex, 97-02-17
 *
 * Extended the capabilities of vfos_nt to fake some more OS/2 Vio* API functions.
 * This should enhance the portability of the Win32 version.
 *
 * another faked OS/2 API. Reads a maximum of 500 char/attributes pairs from the
 * given position and stores them to CellString.
 */

USHORT
VioReadCellStr (PCH pchCellString, USHORT * cbCellString, USHORT usRow,
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

  if (*cbCellString > 1000)
    return 1;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  count = *cbCellString >> 1;

  dwCursorPosition.X = usColumn;
  dwCursorPosition.Y = usRow;

  fResult = ReadConsoleOutputCharacter (hConsoleOutput,
                                        vchars,
                                        count,
                                        dwCursorPosition,
                                        &dwWritten);

  if (fResult)
  {
    fResult = ReadConsoleOutputAttribute (hConsoleOutput,
                                          vattrs,
                                          count,
                                          dwCursorPosition,
                                          &dwWritten);
  }

  for (p = vchars, q = vattrs, i = 0; i < count; i++)
  {
    (*s++) = (char) *p++;
    (*s++) = (char) *q++;
  }

  i = hVio;                     /* happy compiler */
  return (fResult != TRUE);
}

/*
 * alex, 97-02-16
 * implemented to make it /2 compatible :-)
 */

USHORT
VioWrtCharStrAtt (PCH string, USHORT usLen, USHORT usY, USHORT usX, PBYTE
                  attr, USHORT hVio)
{
  DWORD dwWritten;
  WORD attrs[500];
  BOOL fResult = TRUE;
  COORD dwCursorPosition;
  int i;

  if (usLen > 1000)
    return 1;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  dwCursorPosition.X = (SHORT) usX;
  dwCursorPosition.Y = (SHORT) usY;

  for (i = 0; i < usLen; i++)
    attrs[i] = (WORD) (*attr);

  fResult = WriteConsoleOutputCharacter (hConsoleOutput,
                                         string,
                                         usLen,
                                         dwCursorPosition,
                                         &dwWritten);

  if (fResult)
  {
    fResult = WriteConsoleOutputAttribute (hConsoleOutput,
                                           attrs,
                                           usLen,
                                           dwCursorPosition,
                                           &dwWritten);
  }

  i = hVio;                     /* compiler -> happy() */
  return (fResult != TRUE);
}

/*
 * write usNumber characters to the given position. max. allowed is 500
 */

USHORT
VioWrtNChar (PCH * character, USHORT usNumber, USHORT usY, USHORT usX,
             USHORT hVio)
{
  DWORD dwWritten;
  char c[500];
  COORD dwCursorPosition;
  int i;
  BOOL fResult;

  if (usNumber > 500)
    return 1;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  for (i = 0; i < usNumber; i++)
    c[i] = (char) *character;

  dwCursorPosition.X = (SHORT) usX;
  dwCursorPosition.Y = (SHORT) usY;

  fResult = WriteConsoleOutputCharacter (hConsoleOutput,
                                         c,
                                         usNumber,
                                         dwCursorPosition,
                                         &dwWritten);

  i = hVio;
  return (fResult != TRUE);
}

/*
 * the same for the attributes
 */

USHORT
VioWrtNAttr (PBYTE attr, USHORT usNumber, USHORT usY, USHORT usX, USHORT hVio)
{
  DWORD dwWritten;
  WORD attrs[500];
  COORD dwCursorPosition;
  int i;
  BOOL fResult;

  if (usNumber > 500)
    return 1;

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  for (i = 0; i < usNumber; i++)
    attrs[i] = (WORD) (*attr);

  dwCursorPosition.X = (SHORT) usX;
  dwCursorPosition.Y = (SHORT) usY;

  fResult = WriteConsoleOutputAttribute (hConsoleOutput,
                                         attrs,
                                         usNumber,
                                         dwCursorPosition,
                                         &dwWritten);

  i = hVio;
  return (fResult != TRUE);
}

/*
 * set cursor position to row and column. Don't affect cursor appearance.
 */

void
VioSetCurPos (USHORT usY, USHORT usX, USHORT hVio)
{
  COORD cCursor;

  hVio = hVio;                  // make compiler happy

  if (hConsoleOutput == INVALID_HANDLE_VALUE)
    vfossil_init ();

  cCursor.X = usX;
  cCursor.Y = usY;

  SetConsoleCursorPosition (hConsoleOutput, cCursor);
}

/*
 * the following 4 routines are also faked OS/2 Vio* API calls. Some functions of enh_ui.c
 * need them. Their purpose is to scroll the contents of a given region (window).
 *
 * alex, 97-02-17
 */

void
VioScrollLf (USHORT usY0, USHORT usX0, USHORT usY1, USHORT usX1, USHORT
             usNumber, PBYTE fillattr, USHORT hVio)
{
  SMALL_RECT source, clip;
  COORD dest;
  CHAR_INFO ci;

  hVio = hVio;                  // make compiler happy

  source.Left = (SHORT) usX0;
  source.Top = (SHORT) usY0;
  source.Right = (SHORT) usX1;
  source.Bottom = (SHORT) usY1;

  clip = source;

  dest.X = (SHORT) usX0 - usNumber;
  dest.Y = (SHORT) usY0;

  ci.Char.AsciiChar = (CHAR) ' ';
  ci.Attributes = (WORD) * fillattr;

  ScrollConsoleScreenBuffer (hConsoleOutput, &source, &clip, dest, &ci);
}

void
VioScrollRt (USHORT usY0, USHORT usX0, USHORT usY1, USHORT usX1, USHORT
             usNumber, PBYTE fillattr, USHORT hVio)
{
  SMALL_RECT source, clip;
  COORD dest;
  CHAR_INFO ci;

  hVio = hVio;                  // make compiler happy

  source.Left = (SHORT) usX0;
  source.Top = (SHORT) usY0;
  source.Right = (SHORT) usX1;
  source.Bottom = (SHORT) usY1;

  clip = source;

  dest.X = (SHORT) usX0 + usNumber;
  dest.Y = (SHORT) usY0;

  ci.Char.AsciiChar = (CHAR) ' ';
  ci.Attributes = (WORD) * fillattr;

  ScrollConsoleScreenBuffer (hConsoleOutput, &source, &clip, dest, &ci);
}

void
VioScrollDn (USHORT usY0, USHORT usX0, USHORT usY1, USHORT usX1, USHORT
             usNumber, PBYTE fillattr, USHORT hVio)
{
  SMALL_RECT source, clip;
  COORD dest;
  CHAR_INFO ci;

  hVio = hVio;                  // make compiler happy

  source.Left = (SHORT) usX0;
  source.Top = (SHORT) usY0;
  source.Right = (SHORT) usX1;
  source.Bottom = (SHORT) usY1;

  clip = source;

  dest.X = (SHORT) usX0;
  dest.Y = (SHORT) usY0 + usNumber;

  ci.Char.AsciiChar = 0x20;
  ci.Attributes = (WORD) * fillattr;

  ScrollConsoleScreenBuffer (hConsoleOutput, &source, &clip, dest, &ci);
}

void
VioScrollUp (USHORT usY0, USHORT usX0, USHORT usY1, USHORT usX1, USHORT
             usNumber, PBYTE fillattr, USHORT hVio)
{
  SMALL_RECT source, clip;
  COORD dest;
  CHAR_INFO ci;

  hVio = hVio;                  // make compiler happy

  source.Left = (SHORT) usX0;
  source.Top = (SHORT) usY0;
  source.Right = (SHORT) usX1;
  source.Bottom = (SHORT) usY1;

  clip = source;

  dest.X = (SHORT) usX0;
  dest.Y = (SHORT) usY0 - usNumber;

  ci.Char.AsciiChar = ' ';
  ci.Attributes = (WORD) * fillattr;

  ScrollConsoleScreenBuffer (hConsoleOutput, &source, &clip, dest, &ci);
}
