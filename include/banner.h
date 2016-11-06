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
 * Filename    : $Source: E:/cvs/btxe/include/banner.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:00 $
 * State       : $State: Exp $
 *
 * Description : Banner
 *
 *---------------------------------------------------------------------------*/

#define CID_LENGTH 80

struct SBBSBanner
{
  char phone[CID_LENGTH];
  char *banner;
};

void parse_BannerCID (char *par);

/* $Id: banner.h,v 1.2 1999/02/27 01:15:00 mr Exp $ */
