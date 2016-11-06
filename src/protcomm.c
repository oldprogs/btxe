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
 * Filename    : $Source: E:/cvs/btxe/src/protcomm.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:16 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : Common Protocol Key Check
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

int
CommonProtocolKeyCheck (word Key)
{
  switch (Key)
  {
  case 0x1f00:                 /* CFS 230396 - skip file but keep bytes received so far */
    ReqSkipFile = 1;
    status_line ("#Received user request to skip file.");
    break;

  case 0x2500:                 /* CFS 230396 - skip file and kill bytes received */
    ReqSkipFile = 2;
    status_line ("#Received user request to skip+kill file.");
    break;

  case 0x1300:
#ifdef OS_2                     /*  TS 970403   */
    ReqHardwareReset = 1;
#endif
    status_line ("#Received user request to reset ISDN hardware.");
    break;

  case 0x4800:
#ifdef OS_2                     /*  TS 970403   */
    ReqChannelChange = 1;
#endif
    break;

  case 0x5000:
#ifdef OS_2                     /*  TS 970403   */
    ReqChannelChange = -1;
#endif
    break;

  default:
    return 0;
  }

  return 1;
}

/* $Id: protcomm.c,v 1.3 1999/02/27 01:16:16 mr Exp $ */
