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
 * Filename    : $Source: E:/cvs/btxe/src/m7rec.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/20 04:08:22 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Modem7 Receiver State Machine
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int MRInit (XMARGSP);
int MREnd (XMARGSP);
int MRSendNak (XMARGSP);
int MRWaitAck (XMARGSP);
int MRWaitChar (XMARGSP);
int MRWaitOkCk (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
MSTATES, *MSTATEP;

MSTATES Modem7_Receiver[] =
{
  {"MRInit", MRInit},
  {"MREnd", MREnd},
  {"MR0", MRSendNak},
  {"MR1", MRWaitAck},
  {"MR2", MRWaitChar},
  {"MR3", MRWaitOkCk}
};

int
MRInit (XMARGSP args)
{
  args->tries = 0;
  return ((int) args->control);
}

int
MREnd (XMARGSP args)
{
  args->result = (int) args->control;
  return (args->result);
}

int
MRSendNak (XMARGSP args)
{
  if (args->tries >= 10)
    return (FNAME_ERR);

  args->fptr = args->filename;
  if (!args->fptr)
    return (SYSTEM_ERR);

  SENDBYTE (NAK);
  ++(args->tries);
  return (MR1);
}

int
MRWaitAck (XMARGSP args)
{
  long MR1Timer;
  int in_char;

  MR1Timer = timerset (10 * PER_SECOND);
  while (!timeup (MR1Timer))
  {
    if ((in_char = PEEKBYTE ()) >= 0)
    {
      TIMED_READ (0);

      switch (in_char)
      {
      case ACK:
        return (MR2);

      case EOT:
        args->result = SUCCESS_EOT;
        return (SUCCESS_EOT);
      }
    }
    else
    {
      if (!CARRIER)
        return (CARRIER_ERR);
      else
        time_release ();
    }
  }

  return (MR0);
}

int
MRWaitChar (XMARGSP args)
{
  int in_char;
  unsigned char check;
  char *p;

  in_char = TIMED_READ (10);
  switch (in_char)
  {
  case -1:
    return (MR0);

  case EOT:
    return (SUCCESS);

  case SUB:
    for (p = args->filename, check = SUB; p != args->fptr; p++)
      check += (unsigned char) *p;
    SENDBYTE (check);
    return (MR3);

  case 'u':
    return (MR0);

  default:
    break;
  }

  if (args->fptr)
    *args->fptr++ = (char) (in_char & 0xff);

  SENDBYTE (ACK);
  return (MR2);
}

int
MRWaitOkCk (XMARGSP args)
{
  int in_char;

  in_char = TIMED_READ (10);
  if (in_char == ACK)
  {
    args->result = SUCCESS;
    return (SUCCESS);
  }

  return (MR0);
}

int
Modem7_Receive_File (char *filename)
{
  XMARGS batch;

  batch.result = 0;
  batch.filename = filename;
  return (state_machine ((STATEP) Modem7_Receiver, &batch, 2));
}

/* $Id: m7rec.c,v 1.3 1999/03/20 04:08:22 mr Exp $ */
