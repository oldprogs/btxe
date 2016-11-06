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
 * Filename    : $Source: E:/cvs/btxe/src/banner.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:36 $
 * State       : $State: Exp $
 *
 * Description : Banner
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

void
parse_BannerCID (char *par)
{
  char *c;
  int nobanner = 0;
  struct SBBSBanner *newbanner;

  newbanner = (struct SBBSBanner *) malloc (sizeof (struct SBBSBanner));

  if (!newbanner)
    return;

  BBSBannerCID = (struct SBBSBanner **) realloc (BBSBannerCID, (1 + definedbanners) * sizeof (struct SBBSBanner *));

  BBSBannerCID[definedbanners] = newbanner;
  c = par;

  while (*c != '\0' && *c != ' ')
    c++;

  if (*c == '\0')
    nobanner = 1;
  else
    *c = '\0';

  strntcpy (newbanner->phone, par, CID_LENGTH);

  if (nobanner)
    newbanner->banner = NULL;
  else
    newbanner->banner = ctl_string (c + 1);

  definedbanners++;
}

void
set_banner (void)
{
  char *c1, *c2;
  unsigned ct, match;           /*  TS 970124   signed-> unsigned   */
  struct SBBSBanner *current;

  if (CurrentCID == NULL)       /* MR 970311 */
  {
    InUseBBSBanner = BBSbanner;
    return;
  }

  for (ct = 0; ct < definedbanners; ct++)
  {
    c1 = CurrentCID;
    current = BBSBannerCID[ct];
    c2 = current->phone;
    match = 1;

    while (*c1 != '\0' || *c2 != '\0')
    {
      if (*c2 == '*')
        break;
      if (*c1 != *c2)
      {
        match = 0;
        break;
      }
      c1++;
      c2++;
    }

    if (match)
    {
      InUseBBSBanner = current->banner;
      return;
    }
  }

  InUseBBSBanner = BBSbanner;   /* No match; use default banner */
}

/* $Id: banner.c,v 1.3 1999/02/27 01:15:36 mr Exp $ */
