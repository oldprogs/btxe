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
 * Filename    : $Source: E:/cvs/btxe/src/os2/binkpipe/binkpipe.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:39 $
 * State       : $State: Exp $
 * Orig. Author: Robert Hoerner
 *
 * Description :
 *   This program is able to receive the complete text strings and their
 *   exact positions from Binkley in VIO mode, displaying them in a
 *   Binkley-style dialog box. The information is sent by Binkley via a
 *   named pipe and polled and displayed here. It works with OS/2 only,
 *   others may find it useful and transfer it to other OS.
 *
 *---------------------------------------------------------------------------*/

#define INCL_PM
#define INCL_WIN
#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include "binkpipe.h"

HWND hwndDlg = NULLHANDLE;
HMQ hMQThread = NULLHANDLE;
ULONG weiss = CLR_WHITE;
ULONG grau = CLR_PALEGRAY;
ULONG gruen = CLR_GREEN;
ULONG blau = CLR_BLUE;
ULONG rot = CLR_RED;
ULONG schwarz = CLR_BLACK;
ULONG gelb = CLR_YELLOW;
ULONG braun = CLR_BROWN;
CHAR cPipeName[CCHMAXPATH];
HMQ hMQ;
PIPEDATA buffer;
PIPEDATA SendBuffer;

// =====================================================================

HWND
FixSysMenu (HWND hwndDlg)
{
  HWND hwndMenu;

  hwndMenu = WinWindowFromID (hwndDlg, FID_SYSMENU);
  WinSendMsg (hwndMenu, MM_DELETEITEM, MPFS (SC_SIZE, TRUE), MPFROMP (NULL));
  WinSendMsg (hwndMenu, MM_DELETEITEM, MPFS (SC_HIDE, TRUE), MPFROMP (NULL));
  WinSendMsg (hwndMenu, MM_DELETEITEM, MPFS (SC_MAXIMIZE, TRUE), MPFROMP (NULL));
  WinSendMsg (hwndMenu, MM_DELETEITEM, MPFS (SC_RESTORE, TRUE), MPFROMP (NULL));
  return hwndMenu;
}

// =========================================================================

VOID
InsertInSysMenu (HWND hwndMenu, USHORT id, PSZ text)
{
  MENUITEM item;
  SHORT sMenuItems;

  sMenuItems = (SHORT) WinSendMsg (hwndMenu, MM_QUERYITEMCOUNT, NULL, NULL);

  if (text != NULL)
    item.afStyle = MIS_TEXT;
  else
    item.afStyle = MIS_SEPARATOR;

  item.iPosition = sMenuItems;
  item.afAttribute = 0;
  item.id = id;
  item.hwndSubMenu = NULLHANDLE;
  item.hItem = NULLHANDLE;

  WinSendMsg (hwndMenu, MM_INSERTITEM, MPFROMP (&item), MPFROMP (text));
}

// =====================================================================
// This sets the color for the "Callwin" listbox, if it is disabled
// =====================================================================

VOID
Disable (HWND hwnd, USHORT id)
{
  WinSetPresParam (WinWindowFromID (hwnd, id),
                   PP_DISABLEDBACKGROUNDCOLORINDEX, sizeof (grau), &grau);
  WinSetPresParam (WinWindowFromID (hwnd, id),
                   PP_DISABLEDFOREGROUNDCOLORINDEX, sizeof (weiss), &weiss);
}

// =====================================================================
// A normal dialog window routine
// Hint:
// 'cause we are using a global "buffer", each WM_USERMSG message to the
// dialog must be sent (and not posted)!
// =====================================================================

MRESULT EXPENTRY
processDlg (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PSZ pszText;
  ULONG ulWindow;
  ULONG ulRow, ulVersion, ulCol;
  ULONG k, tmp;
  HWND hwndMenu, hwndTmp;
  CHAR line[256];
  char *p, *q;
  USHORT spin, para;

  para = SHORT1FROMMP (mp1);
  spin = SHORT2FROMMP (mp1);

  switch (msg)
  {
  case WM_USERMSG:
    if (mp1 != NULL)            /* self sent message */
    {
      ulWindow = (ULONG) mp1;
      pszText = (PSZ) mp2;
      ulCol = ulRow = 0;
    }
    else
      /* received through pipe */
    {
      ulVersion = buffer.version;
      if (ulVersion != BINKPIPE_VERSION)
        break;

      ulWindow = buffer.window;
      ulRow = buffer.row;
      ulCol = buffer.col;
      pszText = buffer.data;
    }

    hwndTmp = WinWindowFromID (hwnd, ulWindow);
    if (pszText == NULL)
      pszText = "empty";

    /* perform action on selected window */
    switch (ulWindow)
    {
    case BINK_START:           /* intro/start/stop messages */
      WinSetWindowText (hwnd, pszText);
      WinEnableWindow (WinWindowFromID (hwnd, CALLWIN), TRUE);
      break;

    case BINK_END:
      WinSetWindowText (hwnd, STR_WAITING);
      WinEnableWindow (WinWindowFromID (hwnd, CALLWIN), FALSE);
      break;

    case BINK_INTRO:
      hwndTmp = WinWindowFromID (hwnd, BOTTOM_LINE_L);
      WinEnableWindow (WinWindowFromID (hwnd, CALLWIN), TRUE);
      WinSetWindowText (hwndTmp, pszText);
      break;

    case CALLWIN:              /* action and log messages */
      WinEnableWindow (WinWindowFromID (hwnd, CALLWIN), TRUE);
      k = (ULONG) WinSendMsg (hwndTmp, LM_QUERYITEMCOUNT, NULL, NULL);
      if (k > 1024)
      {
        WinSendMsg (hwndTmp, LM_DELETEITEM, MPFROM2SHORT (1, 0), NULL);
        k = (ULONG) WinSendMsg (hwndTmp, LM_QUERYITEMCOUNT, NULL, NULL);
      }
      WinSendMsg (hwndTmp, LM_INSERTITEM, MPFROM2SHORT (k, 0), (MPARAM) pszText);
      WinSendMsg (hwndTmp, LM_SELECTITEM, MPFROM2SHORT (k, 0), NULL);
      break;

    case E_HOLDWIN_0:
    case E_HOLDWIN_1:
    case E_HOLDWIN_2:
    case E_HOLDWIN_3:
    case E_HOLDWIN_4:
    case E_HOLDWIN_5:          /* this might be a prometer line */
      WinEnableWindow (WinWindowFromID (hwnd, CALLWIN), TRUE);
      WinQueryWindowText (hwndTmp, sizeof (line) - 1, (PSZ) & line);
      p = line;
      k = 0;
      while (*p)
      {
        p++;
        k++;
      }

      if (k < ulCol)
      {
        while (k < ulCol)
        {
          *p++ = ' ';
          k++;
        }
      }
      else
      {
        p = line;
        p += ulCol;
      }
      *p = 0;
      q = pszText;

      while (*q)
        *p++ = *q++;
      *p = 0;
      WinSetWindowText (hwndTmp, line);
      break;

    case CLEAR_WHOLEWIN:       /* clear window messages */
      WinSendMsg (hwnd, WM_USERMSG, (MPARAM) CLEAR_SETTINGS, NULL);
      WinSendMsg (hwnd, WM_USERMSG, (MPARAM) CLEAR_FILEWIN, NULL);
      WinSendMsg (hwnd, WM_USERMSG, (MPARAM) CLEAR_HOLDWIN, NULL);
      WinSendMsg (hwnd, WM_USERMSG, (MPARAM) CLEAR_CALLWIN, NULL);
      WinSendMsg (hwnd, WM_USERMSG, (MPARAM) CLEAR_HISTORY, NULL);
      break;

    case CLEAR_FILEWIN:
      WinSetDlgItemText (hwnd, TRANSFER_SEND, "");
      WinSetDlgItemText (hwnd, TRANSFER_RECV, "");
      break;

    case CLEAR_HOLDWIN:
      WinSetDlgItemText (hwnd, E_HOLDWIN_0, "");
      WinSetDlgItemText (hwnd, E_HOLDWIN_1, "");
      WinSetDlgItemText (hwnd, E_HOLDWIN_2, "");
      WinSetDlgItemText (hwnd, E_HOLDWIN_3, "");
      WinSetDlgItemText (hwnd, E_HOLDWIN_4, "");
      WinSetDlgItemText (hwnd, E_HOLDWIN_5, "");
      break;

    case CLEAR_CALLWIN:
      WinSendDlgItemMsg (hwnd, CALLWIN, LM_DELETEALL, NULL, NULL);
      break;

    case CLEAR_SETTINGS:
      WinSetDlgItemText (hwnd, S_TEXT_DATUM, "");
      WinSetDlgItemText (hwnd, S_TEXT_EVENT, "");
      WinSetDlgItemText (hwnd, S_TEXT_PORT, "");
      WinSetDlgItemText (hwnd, S_TEXT_DTE, "");
      WinSetDlgItemText (hwnd, S_TEXT_STATUS, "");
      WinSetDlgItemText (hwnd, E_EVENT, "");
      WinSetDlgItemText (hwnd, E_PORT, "");
      WinSetDlgItemText (hwnd, E_DTE, "");
      WinSetDlgItemText (hwnd, E_STATUS, "");
      /* RS 10.97 start new Binkley style */
      WinSetDlgItemText (hwnd, S_TEXT_SESSION, "");
      WinSetDlgItemText (hwnd, S_TEXT_PROTOCOL, "");
      WinSetDlgItemText (hwnd, S_TEXT_NETMAIL, "");
      WinSetDlgItemText (hwnd, S_TEXT_MEMORY, "");
      WinSetDlgItemText (hwnd, E_SESSION, "");
      WinSetDlgItemText (hwnd, E_PROTOCOL, "");
      WinSetDlgItemText (hwnd, E_NETMAIL, "");
      WinSetDlgItemText (hwnd, E_MEMORY, "");
      /* RS 10.97 end new Binkley style */
      break;

    case CLEAR_HISTORY:
      WinSetDlgItemText (hwnd, E_INOUT, "");
      WinSetDlgItemText (hwnd, E_OUTBOUND, "");
      /* E_Cost and E_FILES stand for the old style, */
      /* but reused RS 9710 */
      WinSetDlgItemText (hwnd, E_COST, "");
      WinSetDlgItemText (hwnd, E_FILES, "");
      WinSetDlgItemText (hwnd, E_LAST, "");
      break;

    default:                   /* normal display messages */
      WinEnableWindow (WinWindowFromID (hwnd, CALLWIN), TRUE);
      WinSetWindowText (hwndTmp, pszText);
      break;
    }
    break;

  case WM_HELP:                /* no help available */
    break;

  case WM_INITDLG:
    hwndDlg = hwnd;
    tmp = (ULONG) 'BINK';
    WinSetWindowULong (hwnd, QWL_USER, (ULONG) tmp);
    WinSetWindowText (hwnd, STR_WAITING);
    WinSendDlgItemMsg (hwnd, CALLWIN, LM_DELETEALL, NULL, NULL);
    Disable (hwnd, CALLWIN);
    WinEnableWindow (WinWindowFromID (hwnd, CALLWIN), FALSE);
    WinSetDlgItemText (hwnd, BOTTOM_LINE_R, cPipeName);

#if (0)                         /* not yet implemented in binkley */
    hwndMenu = FixSysMenu (hwnd);
    InsertInSysMenu (hwndMenu, IDM_BINKQUIT, "~Quit");
    InsertInSysMenu (hwndMenu, IDM_BINKPOLL, "~Poll");
    InsertInSysMenu (hwndMenu, IDM_BINKRESCAN, "~Rescan");
    InsertInSysMenu (hwndMenu, IDM_BINKINIT, "~Init");
    InsertInSysMenu (hwndMenu, IDM_BINKANSWER, "~Answer");
    InsertInSysMenu (hwndMenu, IDM_BINKDEBUG, "~Debug");
    InsertInSysMenu (hwndMenu, IDM_BINKHELP, "~Help");
#else
    hwndMenu = hwndMenu;
#endif
    break;

    // =====================================================================

  case WM_MENUSELECT:
    if (spin)
      switch (para)
      {
      case IDM_BINKQUIT:
        SendBuffer.window = PIPE_MSG_QUIT;
        sprintf (SendBuffer.data, "QUIT: Das ist ein Test");
        break;

      case IDM_BINKPOLL:
        SendBuffer.window = PIPE_MSG_POLL;
        sprintf (SendBuffer.data, "POLL: Das ist ein Test");
        break;

      case IDM_BINKRESCAN:
        SendBuffer.window = PIPE_MSG_RESCAN;
        sprintf (SendBuffer.data, "RESCAN: Das ist ein Test");
        break;

      case IDM_BINKINIT:
        SendBuffer.window = PIPE_MSG_INIT;
        sprintf (SendBuffer.data, "INIT: Das ist ein Test");
        break;

      case IDM_BINKANSWER:
        SendBuffer.window = PIPE_MSG_ANSWER;
        sprintf (SendBuffer.data, "ANSWER: Das ist ein Test");
        break;

      case IDM_BINKHELP:
        SendBuffer.window = PIPE_MSG_HELP;
        sprintf (SendBuffer.data, "HELP: Das ist ein Test");
        break;

      case IDM_BINKDEBUG:
        SendBuffer.window = PIPE_MSG_HELP;
        sprintf (SendBuffer.data, "DEBUG: Das ist ein Test");
        break;
      }

    switch (para)
    {
    case IDM_BINKQUIT:
    case IDM_BINKPOLL:
    case IDM_BINKRESCAN:
    case IDM_BINKINIT:
    case IDM_BINKANSWER:
    case IDM_BINKDEBUG:
    case IDM_BINKHELP:
      k = sizeof (SendBuffer) -
        sizeof (SendBuffer.data) + strlen (SendBuffer.data) + 1;
      WinPostQueueMsg (hMQ, TRY_WRITE_PIPE,
                       &SendBuffer,
                       MPFROMLONG (k));
      return (MRESULT) FALSE;

    default:
      break;
    }

    return (WinDefDlgProc (hwnd, msg, mp1, mp2));

    // =====================================================================

  case WM_QUIT:
  case WM_CLOSE:
  default:
    return (WinDefDlgProc (hwnd, msg, mp1, mp2));
  }

  return (MRESULT) FALSE;
}

// ========================================================================

APIRET
OpenNPipe (PSZ pipename, PHPIPE hpipe)
{
  APIRET rc;
  HPIPE hp;

  rc = DosCreateNPipe ((PSZ) pipename, &hp, SRVR_OPEN_MODE, SRVR_PIPE_MODE,
                       sizeof (PIPEDATA), sizeof (PIPEDATA), 1000L);
  if (rc != 0)
    return (rc);

  *hpipe = hp;
  return (rc);
}

// ========================================================================

APIRET
ConnectNPipe (HPIPE hpipe)
{
  APIRET rc;
  USHORT tries;

  tries = 0;
  do
  {
    rc = DosConnectNPipe (hpipe);
    if (rc == 0)
      DosResetBuffer (hpipe);
    else
    {
      tries++;
      DosSleep (20L);
    }
  }
  while ((tries < 10) && (rc != 0));
  return (rc);
}

// ========================================================================

ULONG
ReadNPipe (HPIPE hpipe)
{
  ULONG ult;

  if (hpipe == NULLHANDLE)
    return (0);
  DosRead (hpipe, &buffer, sizeof (buffer) - 1, &ult);
  return (ult);
}

// ========================================================================

ULONG
WriteNPipe (HPIPE hpipe, MPARAM mp1, MPARAM mp2)
{
  ULONG ult;

  if (hpipe == NULLHANDLE)
    return (0);
  DosWrite (hpipe, PVOIDFROMMP (mp1), LONGFROMMP (mp2), &ult);
  return (ult);
}

// ========================================================================

VOID
CloseNPipe (PHPIPE phpipe)
{
  if (*phpipe == NULLHANDLE)
    return;

  DosDisConnectNPipe (*phpipe);
  DosClose (*phpipe);
  *phpipe = NULLHANDLE;
}

// ========================================================================

// =====================================================================
// This thread polls the pipe for data. The pipe's name is defined
// in binkpipe.h. It's "extension" is either ".001" or the tasknumber
// as given via the commandline.
// =====================================================================

VOID
Poll (PVOID p)
{
  HAB hAB;
  QMSG qmsg;
  HPIPE hP;
  BOOL bConnected = FALSE;

  p = p;

  if (!OpenNPipe (cPipeName, &hP))
  {
    hAB = WinInitialize (0);
    if (hAB)
    {
      hMQ = WinCreateMsgQueue (hAB, 32);
      if (hMQ)
      {
        hMQThread = hMQ;
        WinPostQueueMsg (hMQ, TRY_CONNECT_PIPE, NULL, NULL);

        while (WinGetMsg (hAB, &qmsg, 0, 0, 0))
        {
          if (qmsg.msg == WM_QUIT)
            break;

          switch (qmsg.msg)
          {
          case TRY_CONNECT_PIPE:
            if (ConnectNPipe (hP))
              WinPostQueueMsg (hMQ, TRY_CONNECT_PIPE, NULL, NULL);
            else
            {
              bConnected = TRUE;
              WinPostQueueMsg (hMQ, TRY_READ_PIPE, NULL, NULL);
            }
            break;

          case TRY_READ_PIPE:
            if (ReadNPipe (hP))
            {
              WinSendMsg (hwndDlg, WM_USERMSG, NULL, NULL);
              WinPostQueueMsg (hMQ, TRY_READ_PIPE, NULL, NULL);
            }
            else
              WinPostQueueMsg (hMQ, TRY_DISCONNECT_PIPE, NULL, NULL);
            break;

          case TRY_WRITE_PIPE:
            if (bConnected)
              WriteNPipe (hP, qmsg.mp1, qmsg.mp2);
            else
              WinPostQueueMsg (hMQ, TRY_WRITE_PIPE, qmsg.mp1, qmsg.mp2);
            break;

          case TRY_DISCONNECT_PIPE:
            DosDisConnectNPipe (hP);
            bConnected = FALSE;
            WinPostQueueMsg (hMQ, TRY_CONNECT_PIPE, NULL, NULL);
            break;
          }
        }

        WinDestroyMsgQueue (hMQ);
      }

      WinTerminate (hAB);
    }

    CloseNPipe (&hP);
  }

  WinPostMsg (hwndDlg, WM_QUIT, NULL, NULL);
  _endthread ();
}


// =====================================================================
// Main routine.
// After initializing the presentation manager and creating a message queue
// it takes argument 1 from the commandline as "task" number and
// starts the poll thread. If it succeeds it runs the dialog box.
// After finishing with the dialog box it stops the poll thread.
// =====================================================================

int
main (int argc, char *argv[])
{
  HAB hAB;
  HMQ hMQ;
  USHORT usTask;

  hAB = WinInitialize (0);
  if (hAB)
  {
    hMQ = WinCreateMsgQueue (hAB, 32);
    if (hMQ)
    {
      if (argc < 2)
        usTask = 1;
      else
        sscanf (argv[1], "%u", &usTask);

      sprintf (cPipeName, PIPE_NAME, usTask);

      if (_beginthread (&Poll, NULL, 32768, NULL) != -1)
        WinDlgBox (HWND_DESKTOP, HWND_DESKTOP, (PFNWP) processDlg,
                   (HMODULE) NULLHANDLE, IDD_BINKLEY, (PVOID) NULL);

      WinPostQueueMsg (hMQThread, WM_QUIT, NULL, NULL);
      WinDestroyMsgQueue (hMQ);
    }

    WinTerminate (hAB);
  }

  DosExit (EXIT_PROCESS, 0);
  return (0);                   /* to make the compiler happy */
}

// =====================================================================

/* $Id: binkpipe.c,v 1.3 1999/02/27 01:16:39 mr Exp $ */
