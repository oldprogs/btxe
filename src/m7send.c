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
 * Filename    : $Source: E:/cvs/btxe/src/m7send.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/20 04:08:23 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Modem7 Sender State Machine
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int MSInit (XMARGSP);
int MSEnd (XMARGSP);
int MSWaitNak (XMARGSP);
int MSWaitChAck (XMARGSP);
int MSWaitCksm (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
MSTATES, *MSTATEP;

MSTATES Modem7_Sender[] =
{
  {"MSInit", MSInit},
  {"MSEnd", MSEnd},
  {"MS0", MSWaitNak},
  {"MS1", MSWaitChAck},
  {"MS2", MSWaitCksm}
};

int
MSInit (XMARGSP args)
{
  char *p;
  int i;
  struct FILEINFO dta;

  XON_DISABLE ();
  args->tries = 0;
  dfind (&dta, args->filename, 0);
  dfind (&dta, NULL, 2);
  strcpy (args->m7name, "           ");

  for (i = 0, p = dta.name; i < 8; p++, i++)
    if ((*p != '.') && (*p != '\0'))
      args->m7name[i] = (char) toupper (*p);
    else
      break;

  if (*p == '.')
    ++p;

  for (i = 8; i < 11; p++, i++)
    if ((*p != '.') && (*p != '\0'))
      args->m7name[i] = (char) toupper (*p);
    else
      break;

  /* Now do the checksum */
  args->check = SUB;
  for (i = 0; i < 11; i++)
    args->check += (unsigned char) args->m7name[i];

  return ((int) args->control);
}

int
MSEnd (XMARGSP args)
{
  args->result = (int) args->control;
  return (args->result);
}

int
MSWaitNak (XMARGSP args)
{
  if (args->tries >= 10)
    return (FNAME_ERR);

  if (!CARRIER)
    return (CARRIER_ERR);

  if (TIMED_READ (10) != NAK)
  {
    ++args->tries;
    return (MS0);
  }

  SENDBYTE (ACK);
  SENDBYTE ((unsigned char) *(args->m7name));
  args->fptr = args->m7name + 1;
  return (MS1);
}

int
MSWaitChAck (XMARGSP args)
{
  if (!CARRIER)
    return (CARRIER_ERR);

  if (TIMED_READ (10) != ACK)
  {
    ++args->tries;
    SENDBYTE ('u');
    return (MS0);
  }

  if (!args->fptr)
    return (SYSTEM_ERR);

  /* If filename done */
  if (*(args->fptr) == '\0')
  {
    SENDBYTE (SUB);
    return (MS2);
  }
  else
  {
    /* Send next char of name */
    SENDBYTE ((unsigned char) *args->fptr++);
    return (MS1);
  }
}

int
MSWaitCksm (XMARGSP args)
{
  int in_char;

  if (!CARRIER)
    return (CARRIER_ERR);

  if (((in_char = TIMED_READ (10)) < 0)
      || ((unsigned char) in_char != args->check))
  {
    SENDBYTE ('u');
    ++args->tries;
    return (MS0);
  }
  else
  {
    SENDBYTE (ACK);
    return (SUCCESS);
  }
}

int
Modem7_Send_File (char *filename)
{
  XMARGS batch;

  batch.result = 0;
  batch.filename = filename;
  return (state_machine ((STATEP) Modem7_Sender, &batch, 2));
}

/* $Id: m7send.c,v 1.5 1999/03/20 04:08:23 mr Exp $ */
