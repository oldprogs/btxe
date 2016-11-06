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
 * Filename    : $Source: E:/cvs/btxe/src/squish.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:21 $
 * State       : $State: Exp $
 *
 * Description : Squish scan for unread mail
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#include "squish.h"

/* r. hoerner (jan 13 1997)
 * replaced all handle based file operations with stream based
 * changed to return the actual "unread" value, even if an error occurs. */

#define MSG_RDLEN           sizeof(XMSG) + sqbase.sz_sqhdr

typedef struct
{
  SQHDR sqhdr;
  XMSG xmsg;
}
SQMSG;


int
SquishScan (char *pszName)
{
  FILE *fh;
  int unread = 0;
  FOFS fofs;
  SQBASE sqbase;
  SQMSG sqmsg;
  char szSqd[PATHLEN];

  strcpy (szSqd, pszName);
  strcat (szSqd, ".sqd");

  fh = share_fopen (szSqd, read_binary, DENY_WRITE);
  if (fh != NULL)
  {
    if (fread (&sqbase, 1, sizeof (SQBASE), fh) != sizeof (SQBASE))
    {
      fclose (fh);
      status_line ("!invalid SQBASE in %s", szSqd);
      return -1;
    }
  }
  else
  {
    status_line (MSG_TXT (M_SQUISH_OPENERR), szSqd);
    return 0;
  }

  fofs = sqbase.begin_frame;
  unread = 0;

  while (fofs)
  {
    if (fseek (fh, fofs, SEEK_SET))
    {
      status_line (MSG_TXT (M_SQUISH_SEEKERR), szSqd);
      break;
    }

    if (fread (&sqmsg, 1, MSG_RDLEN, fh) < (int) MSG_RDLEN)
    {
      status_line (MSG_TXT (M_SQUISH_READERR), szSqd);
      break;
    }

    if (sqmsg.sqhdr.id != (dword) SQHDRID)
    {
      status_line (MSG_TXT (M_SQUISH_TRASHED_HDR), szSqd);
      break;
    }

    if (sqmsg.sqhdr.frame_type == FRAME_NORMAL)
    {
      if (!(sqmsg.xmsg.attr & MSGREAD))
        unread++;

      fofs = sqmsg.sqhdr.next_frame;
    }
  }

  fclose (fh);
  return unread;
}

/* $Id: squish.c,v 1.3 1999/02/27 01:16:21 mr Exp $ */
