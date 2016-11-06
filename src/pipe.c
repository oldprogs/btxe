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
 * Filename    : $Source: E:/cvs/btxe/src/pipe.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:40:03 $
 * State       : $State: Exp $
 * Orig. Author: SM, AS
 *
 * Description : pipe support
 *
 *---------------------------------------------------------------------------*/

#ifdef OS_2                     /* always allow snoop support in OS/2 */

#include "includes.h"

#define SNOOP_PIPE_SIZE       8192
#define MAX_LINE_LENGTH         78
#define MAX_LINE_COUNT          15

static HEV ReadHandle = NULLHANDLE;  /* UnnamedPipe is used as a   */
static HEV WriteHandle = NULLHANDLE;  /* circular buffer            */
static HMTX hmtx = NULLHANDLE;  /* mutex-semaphore handle     */
static ULONG counter = 0;       /* number of elements in the buffer */
static BOOL ok_to_run = FALSE;  /* to finish serverthread after snoop_close */
static PSZ pipeName;

static VOID SnoopServer (PVOID);
static VOID checkPipe (PVOID);

/* init snoop-server                                       */
/* PROJECT : BinkleyTerm 2.60 XE     FILE     : pipe.c     */
/* CREATED : 1996-09-11              MODIFIED : 1997-02-20 */
/* AUTHOR  : Steffen Motzer          VERSION  : 1.2        */
/* REMARK  : snoop_open only needs to be run once at start */

void
snoop_open (PSZ callpipename)
{
  ULONG timeout;
  APIRET ulrc;

  if (callpipename == NULL)
    return;
  if (strlen ((char *) callpipename) == 0)
    return;
  pipeName = malloc (strlen ((char *) callpipename) + 1);
  strcpy (pipeName, callpipename);

  ulrc = DosCreateNPipe (callpipename,
                         &hsnoop,
                         NP_ACCESS_OUTBOUND,
                         NP_NOWAIT | NP_TYPE_MESSAGE | 255,
                         SNOOP_PIPE_SIZE, 0, 0);
  if (ulrc != NO_ERROR)
  {
    hsnoop = (HSNOOP) NULLHANDLE;
    return;
  }

  ulrc = DosConnectNPipe (hsnoop);

  if (ulrc != NO_ERROR)
    for (timeout = 0; timeout < 10; timeout++)
    {
      DosSleep (100);
      ulrc = DosConnectNPipe (hsnoop);
      if (ulrc == NO_ERROR)
        break;
    }

  ulrc = DosCreatePipe (&ReadHandle, &WriteHandle, 1 + MAX_LINE_COUNT * MAX_LINE_LENGTH);

  ulrc = DosCreateMutexSem (NULL, &hmtx, 0, 0);
  ulrc = _beginthread (SnoopServer, NULL, 8192, NULL);
  if (ulrc == -1)
  {
    snoop_close ();
    return;
  }

  ulrc = _beginthread (checkPipe, NULL, 8192, NULL);
}

/* stop and remove snoop-server                            */
/* PROJECT : BinkleyTerm 2.60 XE     FILE     : pipe.c     */
/* CREATED : 1996-09-11              MODIFIED : 1997-02-20 */
/* AUTHOR  : Steffen Motzer          VERSION  : 1.2        */
/* REMARK  : snoop_close disconnects from an existing pipe */

void
snoop_close (void)
{
  if (hsnoop == (HSNOOP) NULLHANDLE)
    return;

  ok_to_run = FALSE;            /* signal "stop" to thread */

  /* give the thread some time to write the last lines
   * before closing the pipe */
  DosSleep (200);

  DosDisConnectNPipe (hsnoop);
  DosClose (hsnoop);
  hsnoop = (HSNOOP) NULLHANDLE;
  DosClose (WriteHandle);
  DosClose (ReadHandle);
  DosCloseMutexSem (hmtx);
  hmtx = NULLHANDLE;
  WriteHandle = NULLHANDLE;
  ReadHandle = NULLHANDLE;
  free (pipeName);
  pipeName = NULL;
  DosSleep (100);
}

/* pass data to snoop-server via UnnamedPipe               */
/* PROJECT : BinkleyTerm 2.60 XE     FILE     : pipe.c     */
/* CREATED : 1996-09-11              MODIFIED : 1997-02-20 */
/* AUTHOR  : Steffen Motzer          VERSION  : 1.2        */
/* REMARK  : the UnnamedPipe must not become full          */

void
SnoopWrite (HSNOOP hsnoop, PSZ tmp)
{
  ULONG cbActual;

  if (hmtx == NULLHANDLE)
    return;
  if (DosRequestMutexSem (hmtx, SEM_INDEFINITE_WAIT) != NO_ERROR)
    return;
  if (hsnoop != (HSNOOP) NULLHANDLE && strlen ((char *) tmp) != 0 && counter != MAX_LINE_COUNT)
  {
    counter++;
    DosReleaseMutexSem (hmtx);
    DosWrite (WriteHandle, tmp, min (strlen ((char *) tmp), MAX_LINE_LENGTH), &cbActual);
  }
  else
    DosReleaseMutexSem (hmtx);
}

VOID
checkPipe (PVOID ignore_me)     /* mtt971027 is pipe is broken reconnect */
{
  ULONG read, pipeState;
  AVAILDATA avail;

  ignore_me = ignore_me;

  while (ok_to_run && ((HSNOOP) hsnoop != NULLHANDLE))
  {
    DosSleep (200);

    DosPeekNPipe (hsnoop, NULL, 0, &read, &avail, &pipeState);
    if (pipeState != NP_STATE_CONNECTED)
    {
      DosDisConnectNPipe (hsnoop);
      DosClose (hsnoop);

      DosCreateNPipe (pipeName,
                      &hsnoop,
                      NP_ACCESS_OUTBOUND,
                      NP_NOWAIT | NP_TYPE_MESSAGE | 255,
                      SNOOP_PIPE_SIZE, 0, 0);
      DosConnectNPipe (hsnoop);
    }
  }

  _endthread ();
}

/* pass data from UnnamedPipe to NamedPipe                 */
/* PROJECT : BinkleyTerm 2.60 XE     FILE     : pipe.c     */
/* CREATED : 1997-01-29              MODIFIED : 1997-02-20 */
/* AUTHOR  : Steffen Motzer          VERSION  : 1.2        */

VOID
SnoopServer (PVOID ignore_me)
{
  ULONG cbActual;
  APIRET ulrc;
  BOOL32 flag = FALSE;
  char buffer[MAX_LINE_LENGTH + 1] =
  {
    0
  };

  ignore_me = ignore_me;
  ok_to_run = TRUE;

  while (hsnoop != NULLHANDLE && ok_to_run)
  {
    ulrc = DosRead (ReadHandle, buffer, MAX_LINE_LENGTH, &cbActual);
    if (ulrc == NO_ERROR && cbActual != 0)
    {
      DosRequestMutexSem (hmtx, SEM_INDEFINITE_WAIT);
      counter--;
      DosReleaseMutexSem (hmtx);
      ulrc = DosConnectNPipe (hsnoop);
      if (ulrc == ERROR_BROKEN_PIPE)  /* if the pipe is broken */
      {
        flag = FALSE;           /* retransmit id-string  */
        DosDisConnectNPipe (hsnoop);  /* disconnect from pipe  */
        ulrc = DosConnectNPipe (hsnoop);  /* and try again         */
      }

      if (ulrc == NO_ERROR)
      {
        /* send id-string */
        if (!flag)
          DosWrite (hsnoop, xfer_id, strlen (xfer_id), &cbActual);

        /* and the status line */
        DosWrite (hsnoop, buffer, strlen (buffer), &cbActual);
        flag = TRUE;
      }
    }
  }

  _endthread ();
}

#endif /* OS_2 */

/*
 * snoop support for Win32 added by alex(as), 97-02-24
 */

#ifdef _WIN32

#include "includes.h"

#define SNOOP_PIPE_SIZE       8192

static HANDLE hev;
static OVERLAPPED ov;
static BOOL connected = FALSE;

/*
 * for Win32 by alex(as)
 * we don't need a seperate thread, because we operate the pipe in asynchronous
 * (overlapped) mode. This means, calls to WriteFile() will always return immediately.
 * The OS perfectly handles aynchronous writes. Note that we create the pipe with
 * the PIPE_WAIT flag. Normally, this creates a blocking pipe, but FILE_FLAG_OVERLAPPED
 * activates async mode.
 */

void
snoop_open (PSZ callpipename)
{
  int timeout;
  DWORD rc;

  if (callpipename == NULL)
    return;
  if (strlen (callpipename) == 0)
    return;

  hev = CreateEvent (NULL, TRUE, FALSE, "BTsn");
  if (hev == INVALID_HANDLE_VALUE)
    return;

  hsnoop = CreateNamedPipe (callpipename,
                            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                            PIPE_WAIT | PIPE_TYPE_MESSAGE, 1,
                            SNOOP_PIPE_SIZE, SNOOP_PIPE_SIZE, 0, NULL);

  if (hsnoop == INVALID_HANDLE_VALUE)
  {
    printf ("CreateNamedPipe failed with %d\n", GetLastError ());
    hsnoop = (HSNOOP) NULLHANDLE;
    CloseHandle (hev);
    return;
  }

  ov.hEvent = hev;
  rc = ConnectNamedPipe (hsnoop, &ov);  /* always returns immediately, because of FILE_FLAG_OVERLAPPED */
  rc = GetLastError ();

  if (rc == ERROR_PIPE_BUSY || rc == ERROR_NO_DATA)
    DisconnectNamedPipe (hsnoop);

  if (rc != ERROR_PIPE_CONNECTED)
  {                             /* client already connected ? */
    for (timeout = 0; timeout < 10; timeout++)
    {
      Sleep (100);
      ConnectNamedPipe (hsnoop, &ov);
      rc = GetLastError ();
      if (rc != ERROR_PIPE_CONNECTED)
        continue;
    }
  }
}

/*
 * close the pipe
 */

void
snoop_close (void)
{
  if (hsnoop == (HSNOOP) NULLHANDLE)
    return;

  DisconnectNamedPipe (hsnoop);
  CloseHandle (hsnoop);
  CloseHandle (hev);
  hsnoop = (HSNOOP) NULLHANDLE;
}

void
SnoopWrite (HSNOOP hsnoop, PSZ tmp)
{
  DWORD rc;
  DWORD ec;
  DWORD dwWritten;

  if (hsnoop == (HSNOOP) NULLHANDLE || strlen (tmp) == 0)
    return;

  ov.hEvent = hev;

  if (!connected)
  {                             /* if we are not connected, do it now */
    rc = ConnectNamedPipe (hsnoop, &ov);
    ec = GetLastError ();

    /*
     * no client has connected
     */

    if (ec == ERROR_NO_DATA || ec == ERROR_PIPE_NOT_CONNECTED || ec == ERROR_PIPE_LISTENING)
    {
      DisconnectNamedPipe (hsnoop);
      connected = FALSE;
      return;
    }

    /*
     * client _IS_ somewhere out there
     * IO_PENDING means that the i/o operation is not completed yet. However, we can assume
     * that at least one client is present.
     * PIPE_CONNECTED occurs when the client has opened the pipe _before_ we tried to connect
     * it.
     */

    if (ec == ERROR_IO_PENDING || ec == ERROR_PIPE_CONNECTED)
    {
      connected = TRUE;
      return;
    }
  }

  if (connected)
  {
    WriteFile (hsnoop, tmp, strlen (tmp), &dwWritten, &ov);
    rc = GetLastError ();
    if (rc == ERROR_NO_DATA || rc == ERROR_PIPE_LISTENING)
    {                           /* client vanished ? */
      DisconnectNamedPipe (hsnoop);
      connected = FALSE;
    }
  }
}

#endif /* _WIN32 */

/* $Id: pipe.c,v 1.5 1999/03/23 22:40:03 mr Exp $ */
