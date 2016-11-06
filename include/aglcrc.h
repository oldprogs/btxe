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
 * Filename    : $Source: E:/cvs/btxe/include/aglcrc.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:14:59 $
 * State       : $State: Exp $
 * Orig. Author: Arjen Lentz, contributed by Michael Buenter
 *
 * Description : Hydra CRC definitions for BinkleyTerm
 *
 * Note        :
 *
 *   Routines for table driven CRC-16 & CRC-32, including building tables
 *   Refer to CRC.DOC for information and documentation.
 *
 *   Information collected and edited by Arjen G. Lentz
 *   Sourcecode in C and 80x86 ASM written by Arjen G. Lentz
 *   COPYRIGHT (C) 1992-1993; ALL RIGHTS RESERVED
 *
 *   CONTACT ADDRESS
 *
 *     LENTZ SOFTWARE-DEVELOPMENT   Arjen Lentz @
 *     Langegracht 7B               AINEX-BBS +31-33-633916
 *     3811 BT  Amersfoort          FidoNet 2:283/512
 *     The Netherlands              arjen_lentz@f512.n283.z2.fidonet.org
 *
 *   DISCLAIMER
 *
 *     This information is provided "as is" and comes with no warranties of
 *     any kind, either expressed or implied. It's intended to be used by
 *     programmers and developers. In no event shall the author be liable to
 *     you or anyone else for any damages, including any lost profits, lost
 *     savings or other incidental or consequential damages arising out of the
 *     use or inability to use this information.
 *
 *   LICENCE
 *
 *     This package may be freely distributed provided the files remain
 *     together, in their original unmodified form.
 *     All files, executables and sourcecode remain the copyrighted property
 *     of Arjen G. Lentz and LENTZ SOFTWARE-DEVELOPMENT.
 *     Licence for any use granted, provided this notice & CRC.DOC are
 *     included. For executable applications, credit should be given in the
 *     appropriate places in the program and documentation.
 *     These notices must be retained in any copies of any part of this
 *     documentation and/or software.
 *
 *     Any use of, or operation on (including copying/distributing) any of
 *     the above mentioned files implies full and unconditional acceptance of
 *     this licence and disclaimer.
 *
 *---------------------------------------------------------------------------*/

#ifndef __CRC_DEF_
#define __CRC_DEF_
/*#include "2types.h" */

#ifndef FAR
#ifdef __MSDOS__
#   if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#      define FAR far
#   else
#      define FAR
#   endif
#else
#   define FAR
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  /* --------------------------------------------------------------------- */
  /* CRC-16 used by ARC and LH, use crc16___ routines                      */
  /* --------------------------------------------------------------------- */
#define CRC16APOLY  (0xA001)    /* Generator polynomial number       */
#define CRC16AINIT  (0)         /* Initial CRC value for calculation */
#define CRC16APOST(crc) (crc)   /* CRC Postconditioning before xmit  */
#define CRC16ATEST  (0)         /* Result to test for at receiver    */

  /* --------------------------------------------------------------------- */
  /* CRC-16 CCITT proper                                                   */
  /* --------------------------------------------------------------------- */
#define CRC16POLY   (0x8408)    /* Generator polynomial number       */
#define CRC16INIT   (0xFFFF)    /* Initial CRC value for calculation */
#define CRC16POST(crc)  (~(crc))  /* CRC Postconditioning before xmit  */
#define CRC16TEST   (0xF0B8)    /* Result to test for at receiver    */

  /* --------------------------------------------------------------------- */
  /* CRC-16 CCITT upside-down                                              */
  /* --------------------------------------------------------------------- */
#define CRC16RPOLY  (0x1021)    /* Generator polynomial number       */
#define CRC16RINIT  (0)         /* Initial CRC value for calculation */
#define CRC16RPOST(crc) (crc)   /* CRC Postconditioning before xmit  */
#define CRC16RTEST  (0)         /* Result to test for at receiver    */

  /* --------------------------------------------------------------------- */
  /* CRC-32 CCITT                                                          */
  /* --------------------------------------------------------------------- */
#define CRC32POLY      (0xEDB88320L)  /* Generator polynomial number       */
#define CRC32INIT      (0xFFFFFFFFL)  /* Initial CRC value for calculation */
#define CRC32POST(crc) (~(crc)) /* CRC Postconditioning before xmit  */
#define CRC32TEST      (0xDEBB20E3L)  /* Result to test for at receiver    */

  /* --------------------------------------------------------------------- */
  /* Number of items in CRC table                                          */
  /* --------------------------------------------------------------------- */
#ifndef CRC_TINY
#define CRC_TABSIZE (256)       /* Normal 256-entry table            */
#else
#define CRC_TABSIZE (2 * 16)    /* Tiny 2x16-entry table             */
#endif

  /* --------------------------------------------------------------------- */
  /* CRC-16 proper, used for both CCITT and the one used by ARC            */
  /* --------------------------------------------------------------------- */

  /* MR 961103: added cdecl */
  void cdecl crc16init (word FAR * crctab, word poly);

  /* MR 961103: added cdecl */
  word cdecl crc16block (word FAR * crctab, word crc, byte FAR * buf, word len);

#ifndef CRC_TINY
#define crc16upd(crctab,crc,c) ((crctab)[((crc) ^ (c)) & 0xff] ^ ((crc) >> 8))
#else
  word crc16upd (word FAR * crctab, word crc, byte c);
#endif

#ifndef CRC_TINY
#define crc16rupd(crctab,crc,c) ((crctab)[(((crc) >> 8) ^ (c)) & 0xff] ^ ((crc) << 8))
#else
  word crc16rupd (word FAR * crctab, word crc, byte c);
#endif

  /* --------------------------------------------------------------------- */
  /* CRC-32                                                                */
  /* --------------------------------------------------------------------- */

  /* MR 961103: added cdecl */
  /* MR 961103: crc32int commented out (see modification of aglcrc.c
   *            done by TJW 960522) */
  /* void cdecl crc32init (ULONG FAR * crctab, ULONG poly);                */

  /* MR 961103: added cdecl */
  ULONG cdecl crc32block (ULONG FAR * crctab, ULONG crc, byte FAR * buf, word len);

#ifndef CRC_TINY
#define crc32upd(crctab,crc,c) ((crctab)[((int) (crc) ^ (c)) & 0xff] ^ ((crc) >> 8))
#else
  ULONG crc32upd (ULONG FAR * crctab, ULONG crc, byte c);
#endif

#ifdef __cplusplus
}

#endif
#endif                          /* __CRC_DEF_ */

/* $Id: aglcrc.h,v 1.3 1999/02/27 01:14:59 mr Exp $ */
