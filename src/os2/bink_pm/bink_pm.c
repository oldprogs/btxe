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
 * Filename    : $Source: E:/cvs/btxe/src/os2/bink_pm/bink_pm.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:37 $
 * State       : $State: Exp $
 *
 * Description : pm functions dll
 *
 *---------------------------------------------------------------------------*/


#define INCL_WIN
#define INCL_PM
#define INCL_DOS
#include <os2.h>

#define DLL_VERSION  100000L

// ==========================================================================

APIRET16 APIENTRY16 Win16SetTitle (char *);  /* set OS/2 window title */

// ==========================================================================

ULONG EXPENTRY
GetVersion (VOID)
{
  return (DLL_VERSION);
}

// ==========================================================================

BOOL EXPENTRY
IsPipeRunning (HWND * hwnd)
{
  HENUM henum;
  BOOL found;
  ULONG qwl_bink;

  if (*hwnd == NULLHANDLE)
    found = FALSE;
  else
  {
    qwl_bink = WinQueryWindowULong (*hwnd, QWL_USER);
    found = qwl_bink == 'BINK';
  }

  if (!found)                   /* either not a remote server, or no local "hwnd" */
  {                             /* then search all windows: is it binkpipe.exe?   */
    henum = WinBeginEnumWindows (HWND_DESKTOP);
    do
    {
      *hwnd = WinGetNextWindow (henum);
      if (*hwnd)
      {
        qwl_bink = WinQueryWindowULong (*hwnd, QWL_USER);
        found = qwl_bink == 'BINK';
      }
    }
    while ((*hwnd != NULLHANDLE) && (!found));
    WinEndEnumWindows (henum);
  }

  return (found);
}

// ==========================================================================

APIRET EXPENTRY
SetWindowTitle (PSZ title)
{
  APIRET rc;

  rc = (APIRET) Win16SetTitle ((CHAR *) title);
  return (rc);
}

// ==========================================================================

/* $Id: bink_pm.c,v 1.3 1999/02/27 01:16:37 mr Exp $ */
