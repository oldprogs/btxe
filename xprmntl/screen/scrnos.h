
/******************************
 *       screen classes       *
 *                            *
 * OS dependend stuff - YAOS  *
 *                            *
 * written by Michael Reinsch *
 *                            *
 *                      v0.20 *
 ******************************/

#ifdef __yet_another_OS__

# ifdef __cplusplus
extern "C" {
# endif

// OS/2 Vio calls for YAOS (yet another OS)

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
  // still to do...
  return 0;
}

APIRET VioGetMode ( VIOMODEINFO *modeinf, UINT16 dummy )
{
  // still to do...
  return 0;
}

APIRET VioSetMode ( VIOMODEINFO *modeinf, UINT16 dummy )
{
  // still to do...
  return 0;
}

APIRET VioSetCurType ( VIOCURSORINFO *cursorinfo, UINT16 dummy )
{
  // still to do...
  return 0;
}

# ifdef __cplusplus
}
# endif

#endif
