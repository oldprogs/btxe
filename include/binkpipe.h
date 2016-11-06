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
 * Filename    : $Source: E:/cvs/btxe/include/binkpipe.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:01 $
 * State       : $State: Exp $
 *
 * Description : include file for binkpipe
 *
 *---------------------------------------------------------------------------*/

#ifndef OS2_INCLUDED
#include <os2.h>
#endif

#define BINKPIPE_VERSION 1

/* --------------------------------------------------------------------- */
#define STR_WAITING     "waiting for binkley"
#define PIPE_NAME       "\\PIPE\\BINKPIPE.%03x"
#define QUEUENAME       "\\QUEUES\\BINKPIPE.QUE"
#define MPFS(x,y)        MPFROM2SHORT(x,y)
#define MPFL(x)          MPFROMLONG(x)

#define SRVR_OPEN_MODE  NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_WRITEBEHIND
#define SRVR_PIPE_MODE  NP_WAIT | NP_READMODE_MESSAGE | NP_TYPE_MESSAGE | 1

#define CLNT_OPEN_MODE  OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_WRITE_THROUGH

/* --------------------------------------------------------------------- */

#define IDD_BINKLEY     1000
#define IDR_EXEICON     1001
/* --------------------------------------------------------------------- */
#define IDC_OK          1

#define BINK_START      20
#define BINK_END        21
#define BINK_INTRO      22

#define CLEAR_WHOLEWIN  30
#define CLEAR_FILEWIN   31
#define CLEAR_CALLWIN   32
#define CLEAR_HOLDWIN   33
#define CLEAR_SETTINGS  34
#define CLEAR_HISTORY   35

#define S_TEXT_DATUM    50
#define S_TEXT_EVENT    51
#define S_TEXT_PORT     52
#define S_TEXT_DTE      53
#define S_TEXT_STATUS   54
#define S_TEXT_SESSION  55
#define S_TEXT_PROTOCOL 56
#define S_TEXT_NETMAIL  57
#define S_TEXT_MEMORY   58

#define T_TEXT_STATUS   70
#define T_TEXT_OVERVIEW 71
#define T_TEXT_OUTBOUND 72
#define T_TEXT_ACTIVITY 73
#define T_TEXT_TRANSFER 74

#define BOTTOM_LINE_R   80
#define BOTTOM_LINE_L   81
#define CALLWIN         82

#define TRANSFER_SEND   90
#define TRANSFER_RECV   91

#define E_EVENT         100
#define E_PORT          101
#define E_DTE           102
#define E_STATUS        103
#define E_SESSION       104
#define E_PROTOCOL      105
#define E_NETMAIL       106
#define E_MEMORY        107

#define E_INOUT         110
#define E_OUTBOUND      111
#define E_COST          112
#define E_FILES         113
#define E_LAST          114

#define E_HOLDWIN_0     120
#define E_HOLDWIN_1     121
#define E_HOLDWIN_2     122
#define E_HOLDWIN_3     123
#define E_HOLDWIN_4     124
#define E_HOLDWIN_5     125
/* --------------------------------------------------------------------- */

#define WM_USERMSG          WM_USER+100
#define TRY_CONNECT_PIPE    WM_USER+1
#define TRY_READ_PIPE       WM_USER+2
#define TRY_WRITE_PIPE      WM_USER+3
#define TRY_DISCONNECT_PIPE WM_USER+4

/* --------------------------------------------------------------------- */
typedef struct
{
  ULONG version;
  ULONG window;
  ULONG row;
  ULONG col;
  CHAR data[4096 - 4 * sizeof (ULONG)];
}
PIPEDATA, *PPIPEDATA;

/* --------------------------------------------------------------------- */
typedef struct
{
  PSZ pipename;
  ULONG size;
  PPIPEDATA data;
}
THRDPARA, *PTHRDPARA;

/* --------------------------------------------------------------------- */
#define IDM_BINKPOLL          2000
#define IDM_BINKQUIT          2001
#define IDM_BINKRESCAN        2002
#define IDM_BINKANSWER        2003
#define IDM_BINKINIT          2004
#define IDM_BINKHELP          2005
#define IDM_BINKDEBUG         2006

/* --------------------------------------------------------------------- */
#define PIPE_MSG_QUIT         0x1000
#define PIPE_MSG_INIT         0x1001
#define PIPE_MSG_RESCAN       0x1002
#define PIPE_MSG_POLL         0x1003
#define PIPE_MSG_ANSWER       0x1004
#define PIPE_MSG_HELP         0x1005
#define PIPE_MSG_DEBUG        0x1006
/* --------------------------------------------------------------------- */

#define DO_QUIT               0x0001
#define DO_INIT               0x0002
#define DO_RESCAN             0x0004
#define DO_POLL               0x0008
#define DO_ANSWER             0x0010
#define DO_HELP               0x0020
#define DO_DEBUG              0x0040

/* --------------------------------------------------------------------- */

/* $Id: binkpipe.h,v 1.2 1999/02/27 01:15:01 mr Exp $ */
