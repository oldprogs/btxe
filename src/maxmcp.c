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
 * Filename    : $Source: E:/cvs/btxe/src/maxmcp.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:52 $
 * State       : $State: Exp $
 *
 * Description : Maximus 3.x MCP IPC and Maximus 2.x IPC routines
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

#ifndef __FLAT__
#define __FLAT__
#endif

/* stuff snatched from the max include files */

/* from max.h */

struct _cstat
{
  unsigned short avail;
  unsigned char username[36];
  unsigned char status[80];
};

/* Data element in IPCxx.BBS file (see MAX_CHAT.C) */

struct _cdat
{
  unsigned short tid;
  unsigned short type;
  unsigned short len;
  unsigned short dest_tid;
  unsigned int rsvd1;
};

/* IPC struct (also used for DOS IPC) */

struct Ipc
{
  short avail;
  char username[36];
  char status[80];
  unsigned short msgs_waiting;
  unsigned long next_msgofs;
  unsigned long new_msgofs;
};

#ifdef OS_2
char *MCPpipename = NULL;
static HPIPE hpMCP = NULLHANDLE;
static struct _cstat cstat;

#endif /* OS_2 */

/* Send a ping to the server every two minutes to show that we're awake */

#ifdef OS_2
void
IPC_Ping (void)
{
  static long time_ping = 0L;

  if (hpMCP && timeup (time_ping))
  {
    McpSendMsg (hpMCP, PMSG_PING, NULL, 0);
    time_ping = timerset (2 * PER_MINUTE);
    /* send ping to MCP ~ once every two minutes */
  }
}
#endif

/* Send IPC message                             *
 * in OS/2 Window Title is changed - this       *
 * makes use of an undocumented OS/2 function.  */

void
IPC_SetStatus (char *status)
{
  static char last_string[80] = "i am an empty string";

  if (strcmp (last_string, status) == 0)  /* same string as last string: exit */
    return;

  /* enable for debugging (r.hoerner)
     status_line("#+MCP: %s",status);
   */

  strcpy (last_string, status);

#ifdef _WIN32
  // HJK 98/06/13 - Set title in Windows, title can always be set
  SetWindowTitle (status);
#endif

#ifdef OS_2
  if (title_format)
    SetWindowTitle (status);

  if (hpMCP)
  {
    cstat.avail = 0;
    strcpy ((char *) cstat.username, MSG_TXT (M_MCP_USERNAME));
    strcpy ((char *) cstat.status, status);
    McpSendMsg (hpMCP, PMSG_SET_STATUS, (unsigned char *) &cstat, sizeof cstat);
  }
#endif

  if (ipc_dir)
  {
    struct Ipc ipc;
    char fname[PATHLEN];
    FILE *fp;

    memset (&ipc, 0, sizeof (ipc));
    strncpy (ipc.username, PRDCT_PRTY " " PRDCT_VRSN, 35);
    strncpy (ipc.status, status, 79);
    ipc.avail = 0;
    sprintf (fname, "%sipc%02x.bbs", ipc_dir, TaskNumber);
    fp = share_fopen (fname, write_binary, DENY_WRITE);
    if (fp != NULL)
    {
      fwrite (&ipc, sizeof (ipc), 1, fp);
      fclose (fp);
    }
  }
}

/* Open / Initialize IPC */

#ifdef OS_2
void
IPC_Open (void)
{
  char MCPszPipe[_MAX_PATH];
  APIRET rc, usAction;
  unsigned char tid = TaskNumber;
  long retrytimer;

  /* IPC already open */

  if (hpMCP != NULLHANDLE)
    return;

  /* The place for us to connect is the "\maximus" side off the root
   * MCP "path".
   */

  if (MCPpipename == NULL)      /* there was no MCP pipe definition in binkley.cfg */
  {
    hpMCP = NULLHANDLE;
    return;                     /* i.e.: the user doesn't want it ! */
  }

  strcpy (MCPszPipe, MCPpipename);
  strcat (MCPszPipe, "\\maximus");

  retrytimer = timerset (5 * PER_SECOND);  /* retry for 5s to open pipe */

  do
  {
    rc = DosOpen ((PCSZ) MCPszPipe, &hpMCP, &usAction, 0L, FILE_NORMAL,
                  OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE |
                  OPEN_FLAGS_NOINHERIT, NULL);
    DosSleep (100L);
  }
  while (rc && !timeup (retrytimer));

  if (!rc)
    McpSendMsg (hpMCP, PMSG_HELLO, &tid, 1);
  else
  {
    status_line (MSG_TXT (M_MCP_COULDNTOPENPIPE), MCPpipename);
    hpMCP = NULLHANDLE;
  }
}
#endif


/* Close IPC */

#ifdef OS_2
void
IPC_Close (void)
{
  if (hpMCP)
  {
    McpSendMsg (hpMCP, PMSG_EOT, NULL, 0);
    DosClose (hpMCP);
    hpMCP = NULLHANDLE;
  }
}
#endif

/* $Id: maxmcp.c,v 1.5 1999/03/23 22:28:52 mr Exp $ */
