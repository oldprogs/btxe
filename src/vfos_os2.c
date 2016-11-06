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
 * Filename    : $Source: E:/cvs/btxe/src/vfos_os2.c,v $
 * Revision    : $Revision: 1.1 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/06 15:58:33 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm VFOSSIL module.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

extern VIOMODEINFO vfos_mode;

/*
 * PLF Fri  05-05-1989  05:35:11 OS/2 version notes:
 *
 * I have basically made this two completely different versions. A lot
 * of the original real mode version deals with the case when vfossil
 * does not exist. Under OS/2, it is always active. */

extern int vfossil_installed;

void
vfossil_init (void)
{
  PVIOMODEINFO q;

  vfossil_installed = 1;
  q = (PVIOMODEINFO) & vfos_mode;
  vfos_mode.cb = sizeof (VIOMODEINFO);
  VioGetMode (q, 0);
  VioSetAnsi (ANSI_ON, 0);
}

void
vfossil_cursor (int st)
{
  VIOCURSORINFO cur;

  VioGetCurType (&cur, 0);
  cur.attr = (USHORT) (st ? 0 : -1);
  VioSetCurType (&cur, 0);
}

void
vfossil_close (void)
{
  vfossil_cursor (1);
  vfossil_installed = 0;
}

void
fossil_gotoxy (int col, int row)
{
  VioSetCurPos ((USHORT) row, (USHORT) col, 0);
}

int
fossil_wherex (void)
{
  USHORT row, col = 0;

  VioGetCurPos ((PUSHORT) & row, (PUSHORT) & col, 0);
  return (col);
}

int
fossil_wherey (void)
{
  USHORT row = 0, col;

  VioGetCurPos ((PUSHORT) & row, (PUSHORT) & col, 0);
  return (row);
}

#ifdef BINKLEY_SOUNDS           /* MB 93-12-18 */

static HMODULE ModHandle = (HMODULE) NULL;
static ULONG (*APIENTRY mciSendString) (PSZ pszCommandBuf,
                                        PSZ pszReturnString,
                                        USHORT wReturnLength,
                                        HWND hwndCallBack,
                                        USHORT usUserParm);

static BOOL mmpm_unavail = FALSE;

void
Make_Sound (char *WaveFile)
{
  APIRET rc;
  PSZ MMPM_ERROR = "!%s Error %d initializing MMPM/2 support";

  char mciStr[100];
  char mciAlias[8];

  find_event ();

  if (cur_event >= 0)
    if (e_ptrs[cur_event].extramask & EXTR_NOSOUND)
      return;

  if (WaveFile == NULL || mmpm_unavail)
    return;

  if (!ModHandle)
  {
    /* load the MMPM/2 MDM.DLL */
    /* TJW 960414 added !=0 to have happy compiler */
    if ((rc = DosLoadModule ((PSZ) mciStr, sizeof (mciStr), (PCSZ) "MDM",
                             &ModHandle)) != 0)
    {
      status_line ((char *) MMPM_ERROR, "DosLoadModule", rc);
      mmpm_unavail = TRUE;
      return;
    }

    /* get the mciSendString entry point */
    /* TJW 960414 added !=0 to have happy compiler */
    if ((rc = DosQueryProcAddr (ModHandle, 0L, (PCSZ) "mciSendString",
                                (PFN *) & mciSendString)) != 0)
    {
      status_line ((char *) MMPM_ERROR, "DosQueryProcAddr", rc);
      DosFreeModule (ModHandle);
      mmpm_unavail = TRUE;
      return;
    }
  }

  /* Create an alias for talking to mciSendString */
  sprintf (mciAlias, "bt%02xsnd", TaskNumber);

  /* Open the file */
  sprintf (mciStr, "open %s alias %s wait", WaveFile, mciAlias);
  if (!mciSendString ((PSZ) mciStr, (PSZ) NULL, 0, (HWND) 0, 0))
  {
    /* Play the file */
    sprintf (mciStr, "play %s wait", mciAlias);
    mciSendString ((PSZ) mciStr, (PSZ) NULL, 0, (HWND) 0, 0);

    /* Close the file */
    sprintf (mciStr, "close %s wait", mciAlias);
    mciSendString ((PSZ) mciStr, (PSZ) NULL, 0, (HWND) 0, 0);
  }
}
#endif /* BINKLEY_SOUNDS */

/* $Id: vfos_os2.c,v 1.1 1999/03/06 15:58:33 hjk Exp $ */
