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
 * Filename    : $Source: E:/cvs/btxe/src/recvbark.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:40:05 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm "BARK" File request state machine
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int get_req_str (char *);
void gen_req_name (char *);
int cdog_callback (char *);
int cdog_time (long);

int RBInit (BARKARGSP);
int RBEnd (BARKARGSP);
int RBHonorReq (BARKARGSP);
int RBWaitBark (BARKARGSP);
int RBAckBark (BARKARGSP);
int RBWaitStrt (BARKARGSP);
int RBSendFile (BARKARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (BARKARGSP);
}
RSTATES, *RSTATEP;

RSTATES Bark_Receiver[] =
{
  {"RBInit", RBInit},
  {"RBEnd", RBEnd},
  {"RB0", RBHonorReq},
  {"RB1", RBWaitBark},
  {"RB2", RBAckBark},
  {"RB3", RBWaitStrt},
  {"RB4", RBSendFile}
};

int
Receive_Bark_Packet (BARKARGSP args)
{
  if (get_req_str (args->barkpacket))
  {
    gen_req_name (args->barkpacket);
    args->barkok = 1;
    return (0);
  }

  return (1);
}

int
RBInit (BARKARGSP args)
{
  XON_DISABLE ();
  args->barkok = 0;
  return ((int) args->control);
}

int
RBEnd (BARKARGSP args)
{
  if (!QueryNodeFlag (no_requests, NOREQ, &remote_addr))  /* r. hoerner */
  {
    status_line (":%s %s %s", MSG_TXT (M_END_OF), MSG_TXT (M_INBOUND), MSG_TXT (M_FILE_REQUESTS));
  }

  return ((int) args->control);
}

int
RBHonorReq (BARKARGSP args)
{
  args->nfiles = 0;

  if (!QueryNodeFlag (no_requests, NOREQ, &remote_addr))  /* r. hoerner */
  {
    status_line (":%s %s", MSG_TXT (M_INBOUND), MSG_TXT (M_FILE_REQUESTS));
    SENDBYTE (ENQ);
    args->T1 = timerset (200);
    return (RB1);
  }
  else
  {
    SENDBYTE (CAN);
    status_line (MSG_TXT (M_REFUSING_IN_FREQ));
    return (SUCCESS);
  }
}

int
RBWaitBark (BARKARGSP args)
{
  unsigned short c;
  int ret;
  long RB1Timer;

  ret = -1;
  RB1Timer = timerset (2000);
  while (ret < 0)
  {
    while ((c = PEEKBYTE ()) == 0xffff)
    {
      if (!CARRIER)
        return (CARRIER_ERR);
      time_release ();

      if (timeup (args->T1))
        break;
    }

    if (timeup (RB1Timer))
      return (SENDBLOCK_ERR);   /* Report error */

    if (((unsigned short) c == 0xffff) && timeup (args->T1))
    {
      CLEAR_INBOUND ();
      SENDBYTE (ENQ);
      args->T1 = timerset (200);
      continue;
    }

    c = TIMED_READ (0);

    switch (c)
    {
    case ACK:
      if (Receive_Bark_Packet (args) == 0)
        ret = RB2;
      else
        ret = RB0;
      break;

    case ETB:
      ret = SUCCESS;
      break;

    case ENQ:
      SENDBYTE (ETB);
      break;

    default:
      time_release ();          /*PLF Sun  12-01-1991  05:29:25 */
    }
  }

  return (ret);
}

int
RBAckBark (BARKARGSP args)
{
  if (args->barkok)
  {
    SENDBYTE (ACK);
    return (RB3);
  }
  else
  {
    SENDBYTE (NAK);
    return (RB1);
  }
}

int
RBWaitStrt (BARKARGSP args)
{
  short c = 0;
  long RB3Timer;
  long RB3Timer1;

  RB3Timer = timerset (1500);
  while (CARRIER && (!timeup (RB3Timer)))
  {
    RB3Timer1 = timerset (300);
    while (!timeup (RB3Timer1))
    {
      if ((c = PEEKBYTE ()) >= 0)
        break;

      time_release ();
    }

    if ((unsigned short) c == 0xffff)
      SENDBYTE (ACK);
    else if ((c == 'C') || (c == NAK))
      return (RB4);
  }

  args = args;
  return (SENDBLOCK_ERR);       /* Return error */
}

int
RBSendFile (BARKARGSP args)
{
  args->nfiles = n_frproc (args->barkpacket, cdog_callback, cdog_time);

  Batch_Send (NULL);

  if (args->nfiles > 0)
  {
    status_line (MSG_TXT (M_MATCHING_FILES), args->nfiles);
    fsent += args->nfiles;
  }

  return (RB0);
}

int
SEA_recvreq ()
{
  BARKARGS bark;
  int res;

  Netmail_Session = 2;
  CLEAR_INBOUND ();
  res = state_machine ((STATEP) Bark_Receiver, &bark, RB0);
  status_line (">SEA_recvreq() returns %d", res);

  if (res < SUCCESS)
    return (CANNOT_RECV);
  else
    return (FILE_RECV_OK);
}

int
get_req_str (char *req)
{
  word crc, crc1, crc2, crc3;
  int i, j;

  crc = i = 0;
  while (CARRIER)
  {
    j = TIMED_READ (2);
    if (j < 0)
    {
      time_release ();          /* PLF Sun  12-01-1991  05:29:49 */
      return (0);
    }

    if ((j == ACK) && (i == 0))
      continue;                 /* Just skip the extra ACK */

    if (i >= 100)
    {
      /* Too long of a string */
      status_line (MSG_TXT (M_BAD_BARK));
      CLEAR_INBOUND ();
      return (0);
    }

    if (j == ETX)
    {
      crc1 = (word) TIMED_READ (2);
      crc2 = (word) TIMED_READ (2);
      crc3 = (crc2 << 8) + crc1;
      if (crc3 != crc)
      {
        status_line (MSG_TXT (M_BAD_CRC));
        return (0);
      }

      req[i] = '\0';
      return (1);
    }
    else if (j == SUB)
    {
      return (0);
    }
    else
    {
      req[i++] = (char) (j & 0xff);
      crc = xcrc (crc, (j & 0xff));
    }
  }

  return (0);
}

/*
 * gen_req_name -- take the name [time] [password] fields from
 *                 the BARK file request format and reformat to
 *                 name [!password] [+time] WaZOO format for use
 *                 by the WaZOO file request routines.
 *
 * Input:          *req = pointer to character array with Bark string
 * Output:         *req array contents reformatted
 *
 */

void
gen_req_name (char *req)
{
  char *q, *q1;
  char buf[48];
  char *fsecs = NULL;

  q = req;
  q1 = buf;

  /* Get the filename */
  while ((*q) && (!isspace (*q)))
    *q1++ = *q++;
  *q1 = '\0';

  /* If we have more characters, go on */
  if (*q)
  {
    /* Skip the space */
    fsecs = q++;
    *fsecs = '+';

    /* Skip the digits */
    q = skip_to_blank (q);

    /* If we have more, get the password */
    if (*q)
    {
      *q++ = '\0';              /* Skip space, terminate the time */

      *q1++ = ' ';
      *q1++ = '!';
      while (*q)
        *q1++ = *q++;
      *q1 = '\0';
    }

    /* If we got an update time          */
    if (fsecs != NULL)
    {
      *q1++ = ' ';
      while (*fsecs)
        *q1++ = *fsecs++;
      *q1 = '\0';
    }
  }

  strcpy (req, buf);
  return;
}

int
cdog_callback (char *reqs)
{
  int res;

  res = Batch_Send (reqs);
  status_line (">cdog_callback(%s) returns %d", reqs, res);
  return (res);
}

int
cdog_time (long filesize)
{
  long ltemp;

  ltemp = remaining (filesize, 94L);
  return (ltemp < 20L) ? 20 : (int) ltemp;
}

/* $Id: recvbark.c,v 1.4 1999/03/23 22:40:05 mr Exp $ */
