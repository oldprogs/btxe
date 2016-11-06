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
 * Filename    : $Source: E:/cvs/btxe/src/aglcrc.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:22 $
 * State       : $State: Exp $
 * Orig. Author: Arjen Lentz, contributed by Michael Buenter
 *
 * Description : Hydra CRC calculations for BinkleyTerm
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

#include "includes.h"
#include "aglcrc.h"

/* ------------------------------------------------------------------------- */
void cdecl
crc16init (word FAR * crctab, word poly)
{
  register word i, j;
  register word crc;

  for (i = 0; i <= 255; i++)
  {
    crc = i;
    for (j = 8; j > 0; j--)
    {
      if (crc & 1)
        crc = (word) ((crc >> 1) ^ poly);
      else
        crc >>= 1;
    }
    crctab[i] = crc;
  }
}                               /*crc16init() */

/* ------------------------------------------------------------------------- */
word cdecl
crc16block (word FAR * crctab, word crc, byte FAR * buf, word len)
{
  while (len--)
    crc = (word) crc16upd (crctab, crc, *buf++);

  return (crc);
}                               /*crc16block() */

/* ------------------------------------------------------------------------- */

/* TJW 960522 crc32init unnecessary because it generates same as cr3tab[] */

/* commented out ...
 * void cdecl
 * crc32init (ULONG FAR * crctab, ULONG poly)
 * {
 *     register int i, j;
 *     register ULONG crc;
 *
 *     for (i = 0; i <= 255; i++)
 *     {
 *         crc = i;
 *         for (j = 8; j > 0; j--)
 *         {
 *             if (crc & 1)
 *                 crc = (crc >> 1) ^ poly;
 *             else
 *                 crc >>= 1;
 *         }
 *         crctab[i] = crc;
 *     }
 * }
 * ...
 */

/* crc32init() */

/* ------------------------------------------------------------------------- */
ULONG cdecl
crc32block (ULONG FAR * crctab, ULONG crc, byte FAR * buf, word len)
{
  while (len--)
    crc = crc32upd (crctab, crc, *buf++);

  return (crc);
}                               /*crc32block() */

/* $Id: aglcrc.c,v 1.2 1999/02/27 01:15:22 mr Exp $ */
