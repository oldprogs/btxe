
/******************************
 *       screen classes       *
 *                            *
 *  OS dependend stuff - DOS  *
 *                            *
 * written by Michael Reinsch *
 *                            *
 *                      v0.20 *
 ******************************/

#ifdef __DOS__

# ifdef __cplusplus
extern "C" {
# endif

//  Macros for managing direct video writes by Jerry Houston

/*
     COLORMODE  = true/false, are we using color?
     EXT_KBD    = true/false, extended keyboard in use?
     VIDPAGE    = current video page in use
     SCANLINES  = number of scan lines in a character.
     SCRBUFF    = returns B800:0000 if using color, B000:0000 if mono.
     SCREENSEG  = when you just need the segment portion.
     SCRNBYTES  = number of bytes required to save screen.
     SCRNPIXELS = number of (2-byte) pixels required to save screen.
     SCREENCOLS = number of columns, often 80.
     SCREENROWS = number of rows, usually defaults to 25.
*/

# if !defined(COLORMODE)
#  define FAR        __far
#  define COLORMODE  ((*(char FAR *)0x0449L) != 7)
#  define EXT_KBD    (*(char FAR *)0x0496L & 16)
#  define VIDPAGE    (*((unsigned char far *)0x0462L))
#  define ROWSIZE    (*(int FAR *)0x044AL)
#  define SCANLINES  ((int)*(char FAR*)0x0461L)
#  define SCRBUFF    ((unsigned FAR *)((COLORMODE)?0xB8000000L:0xB0000000L))
#  define SCREENSEG  ((unsigned)((COLORMODE)?0xB800:0xB000))
#  define SCRNBYTES  (*(int FAR *)0x44CL)
#  define SCRNPIXELS (SCRNBYTES >> 1)
#  define SCREENCOLS (*(int FAR *)0x044AL)
#  define SCREENROWS ((*(char FAR *)0x0484L) ? 1 + (*(char FAR *)0x0484L): 25)
# endif


// OS/2 Vio calls for DOS

typedef char *  PCH;
typedef UINT16  USHORT;
typedef UINT32  APIRET;

typedef struct _VIOMODEINFO
{
   USHORT cb;
   UCHAR  fbType;
   UCHAR  color;
   USHORT col;
   USHORT row;
   USHORT hres;
   USHORT vres;
   UCHAR  fmt_ID;
   UCHAR  attrib;
   ULONG  buf_addr;
   ULONG  buf_length;
   ULONG  full_length;
   ULONG  partial_length;
   PCH    ext_data_addr;
} VIOMODEINFO;

typedef struct _VIOCURSORINFO
{
   USHORT   yStart;
   USHORT   cEnd;
   USHORT   cx;
   USHORT   attr;
} VIOCURSORINFO;


APIRET VioWrtCellStr ( char *cell, UINT16 len, UINT16 row, UINT16 col, UINT16 dummy )
{
  dummy = dummy;
  _fmemcpy (SCRBUFF + ( row*SCREENCOLS + col ), cell, len );

  return 0;
}

APIRET VioGetMode ( VIOMODEINFO *modeinf, UINT16 dummy )
{
  dummy        = dummy;
  modeinf->row = (UINT16) SCREENROWS;
  modeinf->col = (UINT16) SCREENCOLS;

  return 0;
}


APIRET VioSetMode ( VIOMODEINFO *modeinf, UINT16 dummy )
{
  dummy = dummy;

  // still to do...

  return 0;
}

APIRET VioSetCurType ( VIOCURSORINFO *cursorinfo, UINT16 dummy )
{
  dummy = dummy;

  // still to do...

  return 0;
}

# ifdef __cplusplus
}
# endif

#endif
