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
 * Filename    : $Source: E:/cvs/btxe/src/brec.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:37 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Batch Receiver State Machine
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static long batch_no_sealink;

int BRInit (XMARGSP);
int BREnd (XMARGSP);
int BRTestSL (XMARGSP);
int BRCheckSL (XMARGSP);
int BRCheckFNm (XMARGSP);
int BRCheckFile (XMARGSP);
int BRFindType (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
BSTATES, *BSTATEP;

BSTATES Batch_Receiver[] =
{
  {"BRInit", BRInit},
  {"BREnd", BREnd},
  {"BR0", BRTestSL},
  {"BR1", BRCheckSL},
  {"BR2", BRCheckFNm},
  {"BR3", BRCheckFile},
  {"BR4", BRFindType},
};

int
BRInit (XMARGSP args)
{
  XON_DISABLE ();
  args->filename = calloc (1, 13);

  if (!args->filename)
    return (SYSTEM_ERR);

  batch_no_sealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);
  return ((int) args->control);
}

int
BREnd (XMARGSP args)
{
  if (args->filename)
    free (args->filename);
  args->filename = NULL;
  return ((int) args->control);
}

int
BRTestSL (XMARGSP args)
{
  if (!batch_no_sealink)
    SENDBYTE (WANTCRC);

  args->T1 = timerset (10 * PER_SECOND);
  args->T2 = timerset (2 * PER_MINUTE);

  return (BR1);
}

int
BRCheckSL (XMARGSP args)
{
  long BR1Timer;

  BR1Timer = timerset (2 * PER_SECOND);
  while (!timeup (BR1Timer))
  {
    if (timeup (args->T2) || batch_no_sealink)
    {
      args->result = Modem7_Receive_File (args->filename);
      return (BR2);
    }

    if ((args->CHR = PEEKBYTE ()) >= 0)
    {
      return (BR4);
    }

    if (timeup (args->T1))
    {
      args->result = Modem7_Receive_File (args->filename);
      return (BR2);
    }
    else
    {
      if (!CARRIER)
        return (CARRIER_ERR);
      else
        time_release ();
    }
  }

  SENDBYTE (WANTCRC);
  return (BR1);
}

int
BRCheckFNm (XMARGSP args)
{
  char buff1[20];
  char *p;
  int i;

  memset (buff1, 0, 19);

  if (args->result == EOT_RECEIVED)  /* Was it the last file */
  {
    return (SUCCESS);
  }
  else if (args->result == SUCCESS)  /* Did we get a valid filename */
  {
    /* First set up the filename buffer */
    p = buff1;
    for (i = 0; i < 8; ++p, i++)
    {
      if (args->filename[i] != ' ')
      {
        *p = args->filename[i];
      }
      else
        break;
    }

    *p = '.';
    ++p;
    *p = '\0';
    for (i = 8; i < 11; ++p, i++)
    {
      if (args->filename[i] != ' ')
      {
        *p = args->filename[i];
      }
      else
        break;
    }
    *p = '\0';

    /* Rename .REQ files */

    i = strlen (buff1) - 4;
    if ((i > 0) && (stricmp (&buff1[i], ".req") == 0))
    {
      buff1[i] = '\0';
      status_line (MSG_TXT (M_REC_REQ_AS), buff1, buff1, TaskNumber);
      sprintf (&buff1[i], ".r%02x", TaskNumber);
    }

    strcpy (args->filename, buff1);

    args->result = Internal_Xmodem_Receive_File (args->path, args->filename);
    return (BR3);
  }
  else                          /* Otherwise, we have to exit */
    return (args->result);
}

int
BRCheckFile (XMARGSP args)
{
  /* Was the file transfer good */
  if ((args->result == SUCCESS) || (args->result == SUCCESS_EOT))
  {
    return (BR0);
  }
  else
  {
    return (args->result);
  }
}

int
BRFindType (XMARGSP args)
{
  switch (args->CHR)
  {
  case SOH:
  case SYN:
    args->result = Internal_Batch_Xmodem_Receive_File (args->path, args->filename);
    return (BR3);

  case EOT:
    TIMED_READ (0);
    SENDBYTE (ACK);
    return (SUCCESS);

    /*  NUL is a special case of "noise." We want to reset our timer if we
     *  see one.
     */

  case NUL:
    args->T1 = timerset (20 * PER_SECOND);

    /*  Fall-through after resetting timer */

  default:
    break;
  }

  TIMED_READ (0);
  return (BR1);
}

int
Batch_Receive (char *where)
{
  XMARGS batch;
  int res;

  batch.result = 0;
  batch.path = where;
  res = state_machine ((STATEP) Batch_Receiver, &batch, 2);
  status_line (">Batch_Receive(%s) returns %d", where == NULL ? "(null)" : where, res);
  return (res == SUCCESS ? FILE_RECV_OK : CANNOT_RECV);
}

/* $Id: brec.c,v 1.4 1999/03/23 22:28:37 mr Exp $ */
