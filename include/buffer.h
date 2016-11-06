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
 * Filename    : $Source: E:/cvs/btxe/include/buffer.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:02 $
 * State       : $State: Exp $
 * Orig. Author: TJW
 *
 * Description : Buffer union / struct definition
 *
 *---------------------------------------------------------------------------*/

#ifndef _BUFFER_H_INCLUDED
#define _BUFFER_H_INCLUDED

/*------------------------- Buffer layout graphically -----------------------
 *
 * v Secbuf
 * v Txbuf
 * |---------------------------- WAZOOMAX + 16 ----------------------------|
 *                               8192+16=8208
 * 
 * Hydra:
 * v txbuf                              v rxbuf
 * |---------- H_BUFLEN ---------------||-------- H_MAXPKTLEN ----------------|
 *     (2048+8+5)*3+16=6199                       (2048+8+5)*3=6183
 * 
 * hydra.h:
 * #define H_MAXBLKLEN  2048                    Max. length of a HYDRA data block
 * #define H_OVERHEAD      8                    Max. no. control bytes in a pkt
 * #define H_MAXPKTLEN  ((H_MAXBLKLEN + H_OVERHEAD + 5) * 3)      Encoded pkt
 * #define H_BUFLEN     (H_MAXPKTLEN + 16)      Buffer sizes: max.enc.pkt + slack
 * 
 * Janus:
 * v Txbuf                              v Rxbuf               v Rxbufmax
 * |----------4096+8-------------------||------BUFMAX+8-------|
 *                                             2048+8=2056
 * 
 *---------------------------------------------------------------------------*/

#ifndef _ZMODEM_H_INCLUDED
#include "zmodem.h"
#endif
#ifndef _HYDRA_H_INCLUDED
#include "hydra.h"
#endif
#ifndef _JANUS_H_INCLUDED
#include "janus.h"
#endif

#define TOTALBUFSIZE (WAZOOMAX+16)

#define TXBUFSIZE (TOTALBUFSIZE)

#define HYDRATXBUFSIZE (H_MAXBLKLEN+H_OVERHEAD+5)  /* = 2048 + 8 + 5 = 2061 */
#define HYDRARXBUFSIZE (H_BUFLEN)  /* = x*3 = 6199 */

#define JANUSTXBUFSIZE (4096+8) /* ??? */
#define JANUSRXBUFSIZE (BUFMAX+8)  /* ??? */

typedef union
{
  // byte pumpitup[20000];      /* size: */
  byte _all[TOTALBUFSIZE];      /* at least WAZOOMAX+16 *//* 8208 */
  byte _Txbuf[WAZOOMAX];        /* zmodem receive *//* 8192 */
  struct
  {
    byte _p[2048];              /* 2049 */
    byte _q[1];                 /* ? */
  }
  extmail;
  struct
  {                             /* 8208 */
    char _junkbuff[4096];
    char _uni[256];
    char _help[256];
    char _tmpstr[2560];
    char _pq[1040];             /* ? */
  }
  emsi;
#ifndef HYDRA_UUE
  struct
  {                             /* +-10000 */
    byte _txbuf[HYDRATXBUFSIZE + 32];  /* was *2 *//* */
    byte _txbufin[HYDRATXBUFSIZE + 32];  /* disk read buffer */
    byte _rxbuf[HYDRATXBUFSIZE + 32];  /* was: 6199 byte   */
  }
  hydra;
#else                           /* this was the original definition: */
  struct
  {
    byte _txbuf[HYDRATXBUFSIZE * 2];
    byte _txbufin[HYDRATXBUFSIZE];
    byte __dummy[16];
    byte _rxbuf[HYDRARXBUFSIZE];
  }
  hydra;
#endif
  struct
  {                             /* 6160 */
    byte _Txbuf[JANUSTXBUFSIZE];
    byte _Rxbuf[JANUSRXBUFSIZE];
  }
  janus;
}
BUF;

#endif /* _BUFFER_H_INCLUDED */

/* $Id: buffer.h,v 1.2 1999/02/27 01:15:02 mr Exp $ */
