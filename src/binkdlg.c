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
 * Filename    : $Source: E:/cvs/btxe/src/binkdlg.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:38 $
 * State       : $State: Exp $
 * Orig. Author: r. hoerner
 *
 * Description :
 *   This implements a mechanism for binkley/OS2 to serve a PM programm
 *   with the current, actual screen output of binkley using a named pipe.
 *   Binkley sends a code for the actually written window / row in window
 *   and the written text.
 *
 *   The technique actually used is to copy the text to a temporary storage
 *   and send it with the necessary control information to a thread, which
 *   finally sends it to the client program (binkpipe.exe).
 *
 *   The transfer of data between binkleys main thread (in SendToDlg()) and
 *   the thread that finally writes to the named pipe is managed using a queue.
 *
 *---------------------------------------------------------------------------*/

#ifdef OS_2

#include "includes.h"

/* --------------------------------------------------------------------- */
PID BinkPID;
BOOL running = FALSE;
CHAR BinkPipe[CCHMAXPATH];

/* --------------------------------------------------------------------- */
VOID
WritePipe (PVOID l)
/* --------------------------------------------------------------------- */
/* NOTE: This thread may NOT use any C-library string functions!         */
/* --------------------------------------------------------------------- */
{
  REQUESTDATA qData;            /* bsedos.h    */
  PPIPEDATA pData;              /* binkpipe.h  */
  APIRET rc;                    /* return code */
  ULONG ulA, ulB;               /* dummies     */
  ULONG queueSize;
  HPIPE hpipe;
  HQUEUE hQueue;
  CHAR cPrio;
  PID thisPID;

  l = l;
  pData = NULL;

  running = TRUE;
  rc = DosOpenQueue (&thisPID, &hQueue, (PCSZ) QUEUENAME);

  if (!rc)
    for (;;)
    {
      rc = DosReadQueue (hQueue, &qData, &queueSize,
                      (PPVOID) & pData, 0L, DCWW_WAIT, (PBYTE) & cPrio, 0L);

      if (rc == 0               /* no error                 */
          && (pData != NULL)    /* valid pointer            */
          && queueSize          /* we got at least 1 byte   */
          && (qData.pid == BinkPID)  /* we got it from ourselves */
          && (qData.ulData == 'BINK'))  /* it's not a STOP command  */
      {
        rc = DosOpen ((PSZ) BinkPipe, (PHFILE) & hpipe,
                      &ulB, 0L, 0,
                      OPEN_ACTION_OPEN_IF_EXISTS |
                      OPEN_ACTION_FAIL_IF_NEW,
                      CLNT_OPEN_MODE,
                      NULL);
        if (rc == 0)
        {
          rc = DosSetNPHState (hpipe, NP_NOWAIT | NP_READMODE_MESSAGE);

          if (rc == 0)
            DosWrite (hpipe, pData, queueSize, &ulA);

          DosClose (hpipe);
          DosFreeMem ((PVOID) pData);
          pData = NULL;
        }
      }
      else if ((qData.pid == BinkPID) && (qData.ulData == 'STOP'))
        break;
    }

  if (hQueue)
    DosCloseQueue (hQueue);

  running = FALSE;

  _endthread ();
}
/* --------------------------------------------------------------------- */
VOID
SendToDlg (PSZ text, ULONG row, ULONG col, ULONG window)
/* --------------------------------------------------------------------- */
{
  BOOL found;
  static HQUEUE hqWrite;
  static BOOL goHome = FALSE;   /* a firewall semaphore to stop re-entries  */
  static HWND hwnd = NULLHANDLE;  /* the window handle of binkpipe running on */

  /* this computer.                           */

  if (goHome)                   /* needed if this routine calls status_line() */
    return;

  goHome = TRUE;

  found = (bink_pipe != NULL);
  if (!found)
    found = IsPipeRunning (&hwnd);

  if (text == NULL)             /* beware of NULL */
    text = (PSZ) "empty";

  if (found && strlen ((char *) text))
  {
    PPIPEDATA pData;
    CHAR pName[32];
    APIRET rc = 0;
    ULONG need = sizeof (PIPEDATA) - sizeof (pData->data) + strlen ((char *) text) + 1;

    if (!running)
    {
      sprintf ((char *) pName, PIPE_NAME, TaskNumber);  /* create PIPE name */

      if (bink_pipe != NULL)
        sprintf (BinkPipe, "%s%s", bink_pipe, pName);
      else
        sprintf (BinkPipe, "%s", pName);

      rc = DosCreateQueue (&hqWrite, QUE_FIFO | QUE_NOCONVERT_ADDRESS, (PCSZ) QUEUENAME);
      if (!rc && hqWrite)
      {
        REQUESTDATA qData;
        ULONG ulAction;
        CHAR cPrio;
        USHORT i;

        DosWriteQueue (hqWrite, 'BINK', need, pData, 0L);

        DosReadQueue (hqWrite, &qData, &ulAction, (PPVOID) & pData,
                      0L, DCWW_WAIT, (PBYTE) & cPrio, 0L);
        BinkPID = qData.pid;

        rc = _beginthread (WritePipe, NULL, 32768L, NULL);
        if (rc != -1)
        {
          i = 100;
          while (!running && i--)
            DosSleep (1L);
        }
        else
        {
          DosCloseQueue (hqWrite);
          return;               /* and let "goHome" untouched. We are finished here! */
        }
      }                         /* if create queue "ok" */
      else
        return;                 /* and let "goHome" untouched. We are finished here! */
    }                           /* if not running */

    if (!rc && running && hqWrite)
    {
      if (DosAllocMem ((PPVOID) & pData, need, fALLOC))
      {
        DosWriteQueue (hqWrite, 'STOP', 1L, &found, 0L);
        DosSleep (100L);
        return;                 /* and let "goHome" untouched. We are finished here! */
      }

      if (pData == NULL)
      {
        DosWriteQueue (hqWrite, 'STOP', 1L, &found, 0L);
        DosSleep (100L);
        return;                 /* and let "goHome" untouched. We are finished here! */
      }

      pData->version = (ULONG) BINKPIPE_VERSION;
      pData->window = (ULONG) window;
      pData->row = (ULONG) row;
      pData->col = (ULONG) col;
      memmove (&pData->data, text, strlen ((char *) text) + 1);

      DosWriteQueue (hqWrite, 'BINK', need, pData, 0L);
    }

    if (window == BINK_END)
    {
      DosWriteQueue (hqWrite, 'STOP', 1L, &found, 0L);
      DosSleep (100L);
      return;                   /* and let "goHome" untouched. We are finished here! */
    }
  }
  goHome = FALSE;
}

#else /* #ifdef OS_2        */

void
SendToDlg (char *text, long row, long col, long window)
{
  text = text;
  row = row;
  col = col;
  window = window;
}

#endif /* #ifdef OS_2        */

/* $Id: binkdlg.c,v 1.2 1999/02/27 01:15:38 mr Exp $ */
