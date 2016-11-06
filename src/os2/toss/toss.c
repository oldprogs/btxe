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
 * Filename    : $Source: E:/cvs/btxe/src/os2/toss/toss.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:40 $
 * State       : $State: Exp $
 *
 * Description : starts programs when an event semaphore is posted
 *
 *---------------------------------------------------------------------------*/

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_DOSPROCESS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  HEV hev = 0;
  ULONG ulPostCt = 0;
  ULONG mode = 0;
  APIRET ulrc = 0;

  if (argc != 2 && argc != 3)
  {
    fprintf (stderr, "\nTOSS - server/client for BinkleyTerm 2.60 XE\n\n");
    fprintf (stderr, "TOSS spawns <program> when the Event Semaphore \\sem32\\<name> is posted.\n");
    fprintf (stderr, "You can start toss as server in startup.cmd and let it run detached.\n\n");
    fprintf (stderr, "Spawn Program        : toss.exe \\sem32\\<name> <program.exe>|<program.cmd>\n");
    fprintf (stderr, "Post Event Semaphore : toss.exe \\sem32\\<name>\n");
    return 1;
  }

  if (argc == 2)
    if (argv[1] == NULL)
      return 1;

  if (argc == 3)
    if (argv[2] == NULL)
      return 1;

  ulrc = DosCreateEventSem (argv[1], &hev, 0, 0);
  switch (ulrc)
  {
  case ERROR_DUPLICATE_NAME:
    if (argc == 2)
      mode = 1;
    break;

  case NO_ERROR:
    if (argc == 3)
      mode = 2;
    else
      (void) DosCloseEventSem (hev);
    break;

  default:
    return 1;
  }

  switch (mode)
  {
  case 1:
    if (!DosOpenEventSem (argv[1], &hev))
      (void) DosPostEventSem (hev);
    break;

  case 2:
    ulrc = 0;
    while (ulrc != -1)
    {
      (void) DosWaitEventSem (hev, SEM_INDEFINITE_WAIT);
      (void) DosResetEventSem (hev, &ulPostCt);
      ulrc = system (argv[2]);
    }
    break;

  default:
    break;
  }

  (void) DosCloseEventSem (hev);
  return 0;
}

/* $Id: toss.c,v 1.3 1999/02/27 01:16:40 mr Exp $ */
