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
 * Filename    : $Source: E:/cvs/btxe/include/vfossil.h,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/06 15:58:22 $
 * State       : $State: Exp $
 *
 * Description : Video FOSSIL definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

typedef struct
{
  int vfossil_size;
  int vfossil_major;
  int vfossil_revision;
  int vfossil_highest;
}
VFOSSIL, *VFOSSILP;

#ifndef OS_2

typedef struct
{
  int cur_start;
  int cur_end;
  int cur_wid;
  int cur_attr;
}
CURSOR, *CURSORP;

typedef CURSOR far *PVIOCURSORINFO;

typedef struct _VIOCONFIGINFO
{                               /* vioin */
  USHORT cb;
  USHORT adapter;
  USHORT display;
  unsigned long cbMemory;
}
VIOCONFIGINFO;

typedef VIOCONFIGINFO far *PVIOCONFIGINFO;

typedef struct _VIOMODEINFO
{                               /* viomi */
  USHORT cb;
  unsigned char fbType;
  unsigned char color;
  USHORT col;
  USHORT row;
  USHORT hres;
  USHORT vres;
  unsigned char fmt_ID;
  unsigned char attrib;
}
VIOMODEINFO;

typedef VIOMODEINFO far *PVIOMODEINFO;

#else /* !defined OS_2 */

#define INCL_SUB
#define INCL_VIO
/* #include <bsesub.h> */
#define CURSOR VIOCURSORINFO
#define CURSORP PVIOCURSORINFO

#endif  /* !defined OS_2 */

#ifdef _WIN32                   /* Subset Vio with console calls */

USHORT VioWrtTTY (PCH pchString, USHORT cbString, USHORT hvio);
USHORT VioWrtCellStr (PCH pchCellString, USHORT cbCellString, USHORT usRow, USHORT usColumn, USHORT hvio);

#endif

#ifdef DOS16                    /* Vio using VFOSSIL */

struct vfossil_hooks
{                               /* VFossil calls structure */
  USHORT (pascal far * GetMode) (PVIOMODEINFO, USHORT);
  USHORT (pascal far * SetMode) (PVIOMODEINFO, USHORT);
  USHORT (pascal far * GetConfig) (USHORT, PVIOCONFIGINFO, USHORT);
  USHORT (pascal far * WrtTTY) (PCH, USHORT, USHORT);
  USHORT (pascal far * GetAnsi) (USHORT far *, USHORT);
  USHORT (pascal far * SetAnsi) (USHORT, USHORT);
  USHORT (pascal far * GetCurPos) (USHORT far *, USHORT far *, USHORT);
  USHORT (pascal far * SetCurPos) (USHORT, USHORT, USHORT);
  USHORT (pascal far * GetCurType) (PVIOCURSORINFO, USHORT);
  USHORT (pascal far * SetCurType) (PVIOCURSORINFO, USHORT);
  USHORT (pascal far * ScrollUp) (USHORT, USHORT, USHORT, USHORT, USHORT, PCH, USHORT);
  USHORT (pascal far * ScrollDn) (USHORT, USHORT, USHORT, USHORT, USHORT, PCH, USHORT);
  USHORT (pascal far * ReadCellStr) (PCH, USHORT far *, USHORT, USHORT, USHORT);
  USHORT (pascal far * ReadCharStr) (PCH, USHORT far *, USHORT, USHORT, USHORT);
  USHORT (pascal far * WrtCellStr) (PCH, USHORT, USHORT, USHORT, USHORT);
  USHORT (pascal far * WrtCharStr) (PCH, USHORT, USHORT, USHORT, USHORT);
  USHORT (pascal far * WrtCharStrAtt) (PCH, USHORT, USHORT, USHORT, PCH, USHORT);
  USHORT (pascal far * WrtNAttr) (PCH, USHORT, USHORT, USHORT, USHORT);
  USHORT (pascal far * WrtNCell) (USHORT far *, USHORT, USHORT, USHORT, USHORT);
  USHORT (pascal far * WrtNChar) (PCH, USHORT, USHORT, USHORT, USHORT);
};

/* Now for readability (and portability) ... */

#define VioGetMode(a,b)            (*vfossil_funcs.GetMode)(a,b)
#define VioSetMode(a,b)            (*vfossil_funcs.SetMode)(a,b)
#define VioGetConfig(a,b,c)        (*vfossil_funcs.GetConfig)(a,b,c)
#define VioWrtTTY(a,b,c)           (*vfossil_funcs.WrtTTY)(a,b,c)
#define VioGetANSI(a,b)            (*vfossil_funcs.GetANSI)(a,b)
#define VioSetANSI(a,b)            (*vfossil_funcs.SetANSI)(a,b)
#define VioGetCurPos(a,b,c)        (*vfossil_funcs.GetCurPos)(a,b,c)
#define VioSetCurPos(a,b,c)        (*vfossil_funcs.SetCurPos)(a,b,c)
#define VioGetCurType(a,b)         (*vfossil_funcs.GetCurType)(a,b)
#define VioSetCurType(a,b)         (*vfossil_funcs.SetCurType)(a,b)
#define VioScrollUp(a,b,c,d,e,f,g) (*vfossil_funcs.ScrollUp)(a,b,c,d,e,f,g)
#define VioScrollDn(a,b,c,d,e,f,g) (*vfossil_funcs.ScrollDn)(a,b,c,d,e,f,g)
#define VioReadCellStr(a,b,c,d,e)  (*vfossil_funcs.ReadCellStr)(a,b,c,d,e)
#define VioReadCharStr(a,b,c,d,e)  (*vfossil_funcs.ReadCharStr)(a,b,c,d,e)
#define VioWrtCellStr(a,b,c,d,e)   (*vfossil_funcs.WrtCellStr)(a,b,c,d,e)
#define VioWrtCharStr(a,b,c,d,e)   (*vfossil_funcs.WrtCharStr)(a,b,c,d,e)
#define VioWrtCharStrAtt(a,b,c,d,e,f) (*vfossil_funcs.WrtCharStrAtt)(a,b,c,d,e,f)
#define VioWrtNAttr(a,b,c,d,e)     (*vfossil_funcs.WrtNAttr)(a,b,c,d,e)
#define VioWrtNCell(a,b,c,d,e)     (*vfossil_funcs.WrtNCell)(a,b,c,d,e)
#define VioWrtNChar(a,b,c,d,e)     (*vfossil_funcs.WrtNChar)(a,b,c,d,e)

#endif /* DOS16 */


#ifdef __unix__

USHORT VioWrtTTY (PCH pchString, USHORT cbString, USHORT hvio);
USHORT VioWrtCellStr (PCH pchCellString, USHORT cbCellString, USHORT usRow, USHORT usColumn, USHORT hvio);

#endif

/* $Id: vfossil.h,v 1.5 1999/03/06 15:58:22 hjk Exp $ */
