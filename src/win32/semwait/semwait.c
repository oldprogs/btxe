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
 * Filename    : $Source: E:/cvs/btxe/src/win32/semwait/semwait.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 14:59:12 $
 * State       : $State: Exp $
 * Orig. Author: A. Sanda
 *
 * Description : Simple event watcher for Win32
 *
 *---------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  HANDLE hsem;
  LONG previous;
  int sysrc = 0;
  int mode;
  ULONG rc;

  if (argc != 2 && argc != 3)
  {
    fprintf (stderr, "\nSEMWAIT - server/client for BinkleyTerm 2.60 XE\n\n");
    fprintf (stderr, "SEMWAIT spawns <program> when the Win32 Event <name> is signaled.\n");
    fprintf (stderr, "You can start it as a server in startup.bat and let it run detached.\n\n");
    fprintf (stderr, "Spawn Program        : semwait.exe <name> <program.exe>|<program.cmd>\n");
    fprintf (stderr, "Post Event Semaphore : semwait.exe <name>\n");
    return 1;
  }

  if (argc == 2)
  {
    if (argv[1] == NULL)
      return 1;
    mode = 0;
  }

  if (argc == 3)
  {
    if (argv[2] == NULL)
      return 1;
    mode = 1;
  }

  if (!mode)
  {
    printf ("Posting event %s\n", argv[1]);
    hsem = CreateEvent (NULL, TRUE, TRUE, strupr (argv[1]));
    rc = GetLastError ();
    if (hsem == NULL)
    {
      printf ("Error creating event %s\n", argv[1]);
      return -1;
    }
    else
    {
      if (rc == ERROR_ALREADY_EXISTS)
      {
        printf ("Semaphore %s already exists, flagging it\n", argv[1]);
        SetEvent (hsem);
      }
    }
  }
  else
  {
    printf ("Waiting for semaphore %s to appear\n\r", strupr (argv[1]));
    hsem = CreateEvent (NULL, TRUE, TRUE, strupr (argv[1]));
    rc = GetLastError ();
    if (!hsem)
    {
      printf ("System error, can't access semaphore\n");
      return -1;
    }
    else
    {
      if (rc != ERROR_ALREADY_EXISTS)
        ResetEvent (hsem);
    }

    printf ("Got it, it does exist.\n\r");
    /*
     * ok, the semaphore does exists, wait for it to be signaled
     */
    printf ("Now, we wait...\n\r");

    while (sysrc != -1)
    {
      WaitForSingleObject (hsem, INFINITE);
      printf ("The semaphore was signaled\n\r");
      ResetEvent (hsem);
      sysrc = system (argv[2]);
    }

    CloseHandle (hsem);
  }

  return 0;
}

/* $Id: semwait.c,v 1.4 1999/09/27 14:59:12 mr Exp $ */
