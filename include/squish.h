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
 * Filename    : $Source: E:/cvs/btxe/include/squish.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:17 $
 * State       : $State: Exp $
 *
 * Description : Squish definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#if !defined _sqhish_h_defined
#define _sqhish_h_defined

#ifndef WORD_DEFINED
#define WORD_DEFINED
#ifndef _WIN32
typedef short WORD;
#endif
#endif
typedef unsigned short sword;
typedef unsigned long FOFS;
typedef unsigned long UMSGID;

struct _stamp                   /* DOS-style datestamp */
{
  struct
  {
    /* IBM Cset/2 is allergic to "unsigned short" when declaring bitfields! */

#ifdef NOSHORT_BITFIELDS
    unsigned int da:5;
    unsigned int mo:4;
    unsigned int yr:7;
#else
    /*lint -e46 */
    WORD da:5;
    WORD mo:4;
    WORD yr:7;
    /*lint -restore */
#endif
  }
  date;

  struct
  {
#ifdef NOSHORT_BITFIELDS
    unsigned int ss:5;
    unsigned int mm:6;
    unsigned int hh:5;
#else
    /*lint -e46 */
    WORD ss:5;
    WORD mm:6;
    WORD hh:5;
    /*lint -restore */
#endif
  }
  time;
};

struct _dos_st
{
  WORD date;
  WORD time;
};

/* Union so we can access stamp as "int" or by individual components */

union stamp_combo
{
  dword ldate;
  struct _stamp msg_st;
  struct _dos_st dos_st;
};

typedef union stamp_combo SCOMBO;

/* Field sizes in XMSG */

#define XMSG_FROM_SIZE  36
#define XMSG_TO_SIZE    36
#define XMSG_SUBJ_SIZE  72

/* Number of reply fields in XMSG.replies */

#define MAX_REPLY 9

/* The network address structure.  The z/n/n/p fields are always             *
 * maintained in parallel to the 'ascii' field, which is simply an ASCII     *
 * representation of the address.  In addition, the 'ascii' field can        *
 * be used for other purposes (such as internet addresses), so the           *
 * contents of this field are implementation-defined, but for most cases,    *
 * should be in the format "1:123/456.7" for Fido addresses.                 */

typedef struct _netaddr
{
  WORD zone;
  WORD net;
  WORD node;
  WORD point;
}
NETADDR;

typedef struct _sqbase
{
  WORD len;                   /* LENGTH OF THIS STRUCTURE! *//*   0 */
  WORD rsvd1;                 /* reserved *//*   2 */

  dword num_msg;              /* Number of messages in area *//*   4 */
  dword high_msg;             /* Highest msg in area. Same as num_msg*//*   8 */
  dword skip_msg;             /* Skip killing first x msgs in area *//*  12 */
  dword high_water;           /* Msg# (not umsgid) of HWM *//*  16 */

  dword uid;                  /* Number of the next UMSGID to use *//*  20 */

  byte base[80];              /* Base name of SquishFile *//*  24 */

  FOFS begin_frame;           /* Offset of first frame in file *//* 104 */
  FOFS last_frame;            /* Offset to last frame in file *//* 108 */
  FOFS free_frame;            /* Offset of first FREE frame in file *//* 112 */
  FOFS last_free_frame;       /* Offset of last free frame in file *//* 116 */
  FOFS end_frame;             /* Pointer to end of file *//* 120 */

  dword max_msg;              /* Max # of msgs to keep in area *//* 124 */
  WORD keep_days;             /* Max age of msgs in area (SQPack) *//* 128 */
  WORD sz_sqhdr;              /* sizeof(SQHDR) *//* 130 */
  byte rsvd2[124];            /* Reserved by Squish for future use*//* 132 */
  /* total: 256 */
}
SQBASE, *PSQBASE;

typedef struct _sqhdr
{
#define SQHDRID       0xafae4453L

  dword id;                   /* sqhdr.id must always equal SQHDRID */

  FOFS next_frame;            /* Next frame in the linked list */
  FOFS prev_frame;            /* Prior frame in the linked list */

  dword frame_length;         /* Length of this frame */
  dword msg_length;           /* Length used in this frame by XMSG, ctrl and text */
  dword clen;                 /* Length used in this frame by ctrl info only */

  WORD frame_type;            /* Type of frame -- see above FRAME_XXXX */
  WORD rsvd;                  /* Reserved for future use */
}
SQHDR, *PSQHDR;

/* No frame offset */

#define NULL_FRAME      ((FOFS)0L)

/* Frame types for sqhdr.frame_type */

#define FRAME_NORMAL    0x00    /* Normal text frame */
#define FRAME_FREE      0x01    /* Part of the free chain */
#define FRAME_LZSS      0x02    /* Not implemented */
#define FRAME_UPDATE    0x03    /* Frame is being updated by another task */

/* The eXtended message structure.  Translation between this structure, and *
 * the structure used by the individual message base formats, is done       *
 * on-the-fly by the API routines.                                          */

typedef struct _xmsg
{
  dword attr;

  byte from[XMSG_FROM_SIZE];
  byte to[XMSG_TO_SIZE];
  byte subj[XMSG_SUBJ_SIZE];

  NETADDR orig;               /* Origination and destination addresses             */
  NETADDR dest;

  struct _stamp date_written; /* When user wrote the msg (UTC)            */
  struct _stamp date_arrived; /* When msg arrived on-line (UTC)           */
  sword utc_ofs;              /* Offset from UTC of message writer, in    *
                               * minutes.                                 */

  UMSGID replyto;             /* This is a reply to message #x            */
  UMSGID replies[MAX_REPLY];  /* Replies to this message                  */
  dword umsgid;               /* UMSGID of this message, if (attr&MSGUID) *
                               * This field is only stored on disk -- it  *
                               * is not read into memory.                 */

  byte __ftsc_date[20];       /* Obsolete date information.  If it weren't for the *
                               * fact that FTSC standards say that one cannot      *
                               * modify an in-transit message, I'd be VERY         *
                               * tempted to axe this field entirely, and recreate  *
                               * an FTSC-compatible date field using               *
                               * the information in 'date_written' upon            *
                               * export.  Nobody should use this field, except     *
                               * possibly for tossers and scanners.  All others    *
                               * should use one of the two binary datestamps,      *
                               * above.                                            */
} XMSG, *PXMSG;

/* Bitmasks for XMSG.attr */

#define MSGPRIVATE 0x0001
#define MSGCRASH   0x0002
#define MSGREAD    0x0004
#define MSGSENT    0x0008
#define MSGFILE    0x0010
#define MSGFWD     0x0020
#define MSGORPHAN  0x0040
#define MSGKILL    0x0080
#define MSGLOCAL   0x0100
#define MSGHOLD    0x0200
#define MSGXX2     0x0400
#define MSGFRQ     0x0800
#define MSGRRQ     0x1000
#define MSGCPT     0x2000
#define MSGARQ     0x4000
#define MSGURQ     0x8000

#define MSGSCANNED 0x00010000L  /* Message has been exported to the network */
#define MSGUID     0x00020000L  /* xmsg.uid field contains umsgid of msg */

#endif                          /*_sqhish_h_defined */

/* $Id: squish.h,v 1.2 1999/02/27 01:15:17 mr Exp $ */
