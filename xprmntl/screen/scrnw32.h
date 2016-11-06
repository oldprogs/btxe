
/******************************
 *       screen classes       *
 *                            *
 * OS dependend stuff - Win32 *
 *                            *
 * written by Henk-Jan Kreuger*
 *                            *
 *                      v0.20 *
 ******************************/

#ifdef __WIN32__

#include <windows.h>

# ifdef __cplusplus
extern "C"
{
# endif

// OS/2 Vio calls for Win32

  typedef char *PCH;
  typedef UINT16 USHORT;
  typedef UINT32 APIRET;

  static HANDLE hStdOut = INVALID_HANDLE_VALUE;

  typedef struct _VIOMODEINFO
  {
    USHORT cb;
    UCHAR fbType;
    UCHAR color;
    USHORT col;
    USHORT row;
    USHORT hres;
    USHORT vres;
    UCHAR fmt_ID;
    UCHAR attrib;
    ULONG buf_addr;
    ULONG buf_length;
    ULONG full_length;
    ULONG partial_length;
    PCH ext_data_addr;
  }
  VIOMODEINFO;

  typedef struct _VIOCURSORINFO
  {
    USHORT yStart;
    USHORT cEnd;
    USHORT cx;
    USHORT attr;
  }
  VIOCURSORINFO;


  APIRET VioWrtCellStr (char *cell, UINT16 len, UINT16 row, UINT16 col,
                        UINT16 dummy)
  {
    static char vchars[500];
    static WORD vattrs[500];
    COORD coords;
    DWORD dwStat;

      dummy = dummy;

    if (hStdOut == INVALID_HANDLE_VALUE)
        hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);

      coords.X = (SHORT) col;
      coords.Y = (SHORT) row;

    CHAR *p = cell;
    WORD *q = vattrs;
    CHAR *c = vchars;

    for (int i = 0; i < len; i++)
    {
      *c++ = *p++;
      *q++ = *p++;
    }

    WriteConsoleOutputAttribute (hStdOut, vattrs, len, coords, &dwStat);
      WriteConsoleOutputCharacter (hStdOut, vchars, len, coords, &dwStat);

      return 0;
  }

  APIRET VioGetMode (VIOMODEINFO * modeinf, UINT16 dummy)
  {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

      dummy = dummy;

    if (hStdOut == INVALID_HANDLE_VALUE)
        hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);

      GetConsoleScreenBufferInfo (hStdOut, &csbi);

      modeinf->col = csbi.dwSize.X;
      modeinf->row = csbi.dwSize.Y;

      return 0;
  }

  APIRET VioSetMode (VIOMODEINFO * modeinf, UINT16 dummy)
  {
    COORD coords;
    SMALL_RECT sr;
    WORD wcbs;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    USHORT xsize, ysize;

      dummy = dummy;

    if (hStdOut == INVALID_HANDLE_VALUE)
        hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);

      GetConsoleScreenBufferInfo (hStdOut, &csbi);

      xsize = modeinf->col;
      ysize = modeinf->row;

      wcbs = csbi.dwSize.X * csbi.dwSize.Y;
      sr.Left = 0;
      sr.Top = 0;
      sr.Right = xsize - 1;
      sr.Bottom = ysize - 1;
      coords.X = xsize;
      coords.Y = ysize;

    if (wcbs > (xsize * ysize))
    {
      SetConsoleWindowInfo (hStdOut, TRUE, &sr);
      SetConsoleScreenBufferSize (hStdOut, coords);
    }
    else if (wcbs < (xsize * ysize))
    {
      SetConsoleScreenBufferSize (hStdOut, coords);
      SetConsoleWindowInfo (hStdOut, TRUE, &sr);
    }

    return 0;
  }

  APIRET VioSetCurType (VIOCURSORINFO * cursorinfo, UINT16 dummy)
  {
    CONSOLE_CURSOR_INFO cci;

      dummy = dummy;

    if (hStdOut == INVALID_HANDLE_VALUE)
        hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);

      cci.dwSize = cursorinfo->yStart;
      cci.bVisible = cursorinfo->cEnd ? true : false;
      SetConsoleCursorInfo (hStdOut, &cci);

      return 0;
  }

# ifdef __cplusplus
}
# endif

#endif
