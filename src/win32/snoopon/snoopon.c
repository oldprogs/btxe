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
 * Filename    : $Source: E:/cvs/btxe/src/win32/snoopon/snoopon.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:43 $
 * State       : $State: Exp $
 *
 * Description : simple pipesnooper for Bink-Win32
 *
 *---------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void close_it (int);
HANDLE hpipe;

int
main (int argc, char *argv[])
{
  char temp[500];
  DWORD dwRead;
  DWORD ec;
  char pipename[256];

  if (argc != 2)
  {
    printf ("Usage: snoopon <pipename>\n\r");
    exit (0);
  }

  strcpy (pipename, "\\\\.\\pipe\\");
  strcat (pipename, argv[1]);

  hpipe = CreateFile (pipename, GENERIC_READ, 0,
                      NULL, OPEN_ALWAYS, 0, 0);


  if (hpipe == INVALID_HANDLE_VALUE)
  {
    ec = GetLastError ();
    printf ("CreateFile() failed with %d\n", ec);
    if (ec == ERROR_PIPE_BUSY)
    {
      printf ("Waiting for pipe %s, press <Ctrl-C> to cancel\n\r", pipename);

      /*
       * now, wait until the pipe is ready again, don't hog the cpu
       */

      do
      {
        Sleep (500L);
        hpipe = CreateFile (pipename, GENERIC_READ, 0,
                            NULL, OPEN_ALWAYS, 0, 0);
      }
      while (GetLastError () == ERROR_PIPE_BUSY);

      if (hpipe == INVALID_HANDLE_VALUE)
        exit (0);

      printf ("Connection to %s re-established\n", pipename);
    }
    else
    {
      printf ("The pipe %s does not seem to exist.\n", pipename);
      exit (0);
    }
  }

  do
  {
    ReadFile (hpipe, temp, sizeof (temp), &dwRead, NULL);
    if (!dwRead)
    {
      ec = GetLastError ();
      printf ("\nPipe broken, waiting.. 0\n");
      CloseHandle (hpipe);
      printf ("\Now attempting to re-establish pipe connection, wait...\n");

      do
      {
        Sleep (500L);
        hpipe = CreateFile (pipename, GENERIC_READ, 0,
                            NULL, OPEN_ALWAYS, 0, 0);
      }
      while (hpipe == INVALID_HANDLE_VALUE);

      printf ("Connection to %s re-established\n", pipename);
    }

    temp[dwRead] = '\0';
    printf ("%s\r", temp);
  }
  while (TRUE);
}

/* $Id: snoopon.c,v 1.3 1999/02/27 01:16:43 mr Exp $ */
