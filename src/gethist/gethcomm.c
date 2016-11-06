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
 * Filename    : $Source: E:/cvs/btxe/src/gethist/gethcomm.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:35 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : common history importer stuff
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
pack (char *s)
{
  printf ("In: %s\n", s);

  while (*s == ' ' || *s == '\n' || *s == '\r')
    strcpy (s, s + 1);

  while (s[strlen (s) - 1] == ' ' || s[strlen (s) - 1] == '\n' || s[strlen (s) - 1] == '\r')
    s[strlen (s) - 1] = 0;

  return s;
}

char *
pas2c (char *s)
{
  char c;

  c = s[0];
  memmove (s, s + 1, c);
  s[c] = 0;
  return s;
}

/* $Id: gethcomm.c,v 1.2 1999/02/27 01:16:35 mr Exp $ */
