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
 * Filename    : $Source: E:/cvs/btxe/src/bbs_ctra.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:36 $
 * State       : $State: Exp $
 * Orig. Author: C. Eike Hofmeister, 2:2426/2280
 *
 * Description : character-translation for bbs
 *
 *---------------------------------------------------------------------------*/

#ifdef BBS_INTERNAL

#include "includes.h"
#include "bbs_incl.h"

char transtable[256];

void
bbsLoadTranslation (char this)
{
  int i;

  for (i = 0; i < 256; i++)
  {
    transtable[i] = i;
  }
}

char
transout (char this)
{
  int i = this;

  return (transtable[i]);
}

char
transin (char this)
{
  int i;

  for (i = 0; i < 256; i++)
  {
    if (transtable[i] == this)
    {
      return (i);
    }
  }
  return ('\0');
}

#endif

/* $Id: bbs_ctra.c,v 1.3 1999/02/27 01:15:36 mr Exp $ */
