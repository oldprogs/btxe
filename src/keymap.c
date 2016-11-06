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
 * Filename    : $Source: E:/cvs/btxe/src/keymap.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:01 $
 * State       : $State: Exp $
 * Orig. Author: Harry Lee
 *
 * Description : Keyboard Remapping Routine
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static struct _key_fnc_hdr *CrntKeyFncHdr;

struct _key_fnc_hdr *
KbMapSet (struct _key_fnc_hdr *KeyFncHdr)
{
  struct _key_fnc_hdr *OldKeyFncHdr = CrntKeyFncHdr;

  CrntKeyFncHdr = KeyFncHdr;
  return OldKeyFncHdr;
}

short
ScanCompare (struct _key_fnc *x, struct _key_fnc *y)
{
  return (short) (x->ScanCode - y->ScanCode);
}

unsigned short pascal
KbRemap (unsigned short ScanCode)
{
  struct _key_fnc InFnc;
  struct _key_fnc *KeyFnc;
  int i;

  /*
   * FOSSIL keyboarding is so wierd.
   *
   * The spec is really clear about what you're supposed to do with
   * letter and function keys. Some of the IBM FOSSILs didn't give a
   * damn and put scan codes in the high order part of the character
   * keys. That breaks this guy utterly. So we'll get rid of the
   * bits if they happen to be there.
   */

  if (ScanCode & 0xff)
    ScanCode &= 0xff;           /* Clean up after Opus!Comm */

  /* If the table is empty, searching it is sort of pointless. */

  if (!CrntKeyFncHdr->KeyFncCnt)
    return ScanCode;

  InFnc.ScanCode = ScanCode;

  KeyFnc = CrntKeyFncHdr->KeyFncTbl;

  for (i = CrntKeyFncHdr->KeyFncCnt; i > 0; i--, KeyFnc++)
  {
    if (ScanCompare (KeyFnc, &InFnc) == 0)
      return KeyFnc->FncIdx;
  }

  return ScanCode;
}

/* $Id: keymap.c,v 1.2 1999/02/27 01:16:01 mr Exp $ */
