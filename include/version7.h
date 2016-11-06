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
 * Filename    : $Source: E:/cvs/btxe/include/version7.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:18 $
 * State       : $State: Exp $
 *
 * Description : version 7 nodelist definitions
 *
 *---------------------------------------------------------------------------*/

#if !defined _VERSION7_H_INCLUDED_
#define _VERSION7_H_INCLUDED_

// return codes ============================================================

#define V7_ERROR                      (-1)
#define V7_SUCCESS                    0
#define V7_IDX_OPENFAILED             1
#define V7_IDX_SEEKERROR              2
#define V7_IDX_READERROR              3
#define V7_DTP_OPENFAILED             4
#define V7_DTP_SEEKERROR              5
#define V7_DTP_READERROR              6
#define V7_DAT_OPENFAILED             7
#define V7_DAT_SEEKERROR              8
#define V7_DAT_READERROR              9
#define V7_NODENUMBER_NOTFOUND        10
#define V7_SYSOPNAME_NOTFOUND         11
#define V7_FINDNEXT_END               12
#define V7_OUT_OF_MEMORY              13


#ifndef _WIN32
#ifndef WORD_DEFINED
#define WORD_DEFINED
typedef short WORD;
typedef unsigned short UWORD;
#endif
#endif

// structure of a fido net node.  this structure is based primarily
// on the NEWNODE structure of Opus CBCS

typedef struct
{
  WORD ZoneNumber;
  WORD NetNumber;
  WORD NodeNumber;
  UWORD Cost;                   // cost to user for message
  char SysopName[26];           // sysop
  char SystemName[34];          // node name
  char PhoneNumber[40];         // phone number
  char MiscInfo[30];            // city and state
  char Password[9];             // password, null-terminated
  UWORD RealCost;               // phone company's charge
  short HubNode;                // node # of this node's hub or point number if system is a point
  UWORD BaudRate;               // baud rate divided by 300
  BYTE ModemType;               // modem type
  UWORD NodeFlags;              // set of flags (see below)
  ULONG ulOffset;               // offset into DAT file
  short online_start;           // V7+: ,U,Txy online flags as "minute of the
  short online_end;             // day" - see FSC-0062 v3, only used if not CM
  char rawSysopName[26];        // V7+: raw sysop name
  char rawSystemName[34];       // V7+: raw system name
  char rawMiscInfo[30];         // V7+: raw city and state
  char rawType[10];             // V7+: raw: Zone,Region,Host,Hub,Pvt,...
  char rawFlags[64];            // V7+: raw: Flags
}
V7NODE, *PV7NODE;

typedef struct _v7nodel
{
  V7NODE v7n;
  struct _v7nodel *next;
}
V7NODEL, *PV7NODEL;

// Values for the `NodeFlags' field ========================================

#define B_hub    0x0001         // system is a net hub
#define B_host   0x0002         // system is a net host
#define B_region 0x0004         // system is region coord
#define B_zone   0x0008         // system is a zone gateway
#define B_CM     0x0010         // system runs continuous mail
#define B_res1   0x0020         // reserved by Opus
#define B_res2   0x0040         // reserved by Opus
#define B_res3   0x0080         // reserved by Opus
#define B_res4   0x0100         // reserved by Opus
#define B_res5   0x0200         // reserved for non-Opus
#define B_res6   0x0400         // reserved for non-Opus
#define B_res7   0x0800         // reserved for non-Opus
#define B_point  0x1000         // system is a point
#define B_res9   0x2000         // reserved for non-Opus
#define B_resa   0x4000         // reserved for non-Opus
#define B_resb   0x8000         // reserved for non-Opus

// Values for the `miscflags' field ========================================

#define MF_v7plus   0x0003      /* =0 -> version7, =1 -> version7+ =2,3 -> rsrvd */
#define MF_sysopndx 0x0004      /* =0 -> nodex.sdx, =1 -> sysop.ndx              */

// Values for the `whichindex' field =======================================

#define WI_ndx      1           /* node index */
#define WI_sdx      2           /* sysop index */
#define WI_pdx      3           /* phone index */

// V7Nl request control block ==============================================

typedef struct
{
  PSZ pszPath;                  /* path of nodelist files ("c:\nodelist\") */
  PSZ pszName;                  /* name of data/index/dtp files, e.g. "nodex" */
  UWORD whichindex;             /* which index to use */
  UWORD miscflags;              /* misc flags */
  PV7NODE pV7Node;              /* address of where to copy nodelist record */
  WORD wLen;                    /* length for find first node function
                                   2 = search on Zone
                                   4 = search on Zone + Net
                                   6 = search on Zone + Net + Node
                                   8 = search on Zone + Net + Node + Point
                                 */
  UWORD V7Flags;                /* Flags */
  PVOID pnlFh;                  /* used by find first/next */
}
V7RCB, *PV7RCB;

// Function prototypes =====================================================

int V7Initialize (void);        // call this prior to any other V7* functions!
int V7Finish (void);            // call this when you're finished with V7* functions
int V7FindFirst (PV7RCB);       // finds all and returns first match
int V7FindNext (PV7RCB);        // returns subsequent matches
int V7FindClose (PV7RCB);       // finishes find first/next processing
int V7Find (PV7RCB);            // find a single match

#endif

/* $Id: version7.h,v 1.2 1999/02/27 01:15:18 mr Exp $ */
