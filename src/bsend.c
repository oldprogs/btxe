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
 * Filename    : $Source: E:/cvs/btxe/src/bsend.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/20 04:08:20 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Batch Sender State Machine
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#define NO_SEALINK_IS_TELINK 1

static long batch_no_sealink;

/* .-----+----------+-------------------------+-------------------------+-----.
 * |State| State    | Predicate(s)            | Action(s)               | Next|
 * |  #  | Name     |                         |                         | St  |
 * +-----+----------+-+-----------------------+-------------------------+-----+
 * | BS0 | MoreFiles|1| more files to send    |                         | BS1 |
 * |     |          +-+-----------------------+-------------------------+-----+
 * |     |          |2| no more files to send |                         | BS4 |
 * +-----+----------+-+-----------------------+-------------------------+-----+
 * | BS1 | WaitType |1| rec NAK               | (MODEM7 FName send MS0) | BS2 |
 * |     |    (*1)  +-+-----------------------+-------------------------+-----+
 * |     |          |2| rec 'C'               | (SEAlink send file XS0) | BS3 |
 * |     |          +-+-----------------------+-------------------------+-----+
 * |     |          |3| rec other char        | eat character           | BS1 |
 * |     |          +-+-----------------------+-------------------------+-----+
 * |     |          |4| > 20 sec in BS1       | report name send bad    | exit|
 * +-----+----------+-+-----------------------+-------------------------+-----+
 * | BS2 | CheckFNm |1| MODEM7 Filename ok    | (TeLink send file XS0T) | BS3 |
 * |     |    (*2)  +-+-----------------------+-------------------------+-----+
 * |     |          |2| MODEM7 Filename bad   | report name send bad    | exit|
 * +-----+----------+-+-----------------------+-------------------------+-----+
 * | BS3 | CheckFile|1| File send ok          |                         | BS0 |
 * |     |          +-+-----------------------+-------------------------+-----+
 * |     |          |2| File send bad         | report file send bad    | exit|
 * +-----+----------+-+-----------------------+-------------------------+-----+
 * | BS4 | EndSend  |1| rec NAK or 'C'        | send EOT, report send ok| exit|
 * |     |    (*3)  +-+-----------------------+-------------------------+-----+
 * |     |          |2| 10 secs no NAK or 'C' | send EOT, report no NAK | exit|
 * `-----+----------+-+-----------------------+-------------------------+-----'
 *
 * BSx renamed to BSNDx, they are already defined for something else in Linux
 * CEH; 19981106 */

int BSInit (XMARGSP);
int BSEnd (XMARGSP);
int BSMoreFiles (XMARGSP);
int BSWaitType (XMARGSP);
int BSCheckFNm (XMARGSP);
int BSCheckFile (XMARGSP);
int BSEndSend (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
BSTATES, *BSTATEP;

BSTATES Batch_Sender[] =
{
  {"BSInit", BSInit},
  {"BSEnd", BSEnd},
  {"BSND0", BSMoreFiles},
  {"BSND1", BSWaitType},
  {"BSND2", BSCheckFNm},
  {"BSND3", BSCheckFile},
  {"BSND4", BSEndSend},
};

int
BSInit (XMARGSP args)
{
  args->result = (int) args->control;
  batch_no_sealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);
  XON_DISABLE ();
  return ((int) args->control);
}

int
BSEnd (XMARGSP args)
{
  args->result = (int) args->control;
  return ((int) args->control);
}

int
BSMoreFiles (XMARGSP args)
{
  if (args->filename != NULL)
    return (BSND1);
  else                          /* BS0.2 */
    return (BSND4);
}

int
BSWaitType (XMARGSP args)
{
  long BS1Timer;
  int in_char;

  BS1Timer = timerset (20 * PER_SECOND);
  while (!timeup (BS1Timer))
  {
    if ((in_char = PEEKBYTE ()) >= 0)
    {
      switch (in_char)
      {
      case NAK:                /* State BS1.1 */
        args->result = Modem7_Send_File (args->filename);
        return (BSND2);

      case WANTCRC:            /* State BS1.2 */
        if (!batch_no_sealink)
        {
          args->result = Internal_SEAlink_Send_File (args->filename, NULL);
          return (BSND3);
        }
#ifdef NO_SEALINK_IS_TELINK
        else
        {
          args->result = Internal_Telink_Send_File (args->filename, NULL);
          return (BSND3);
        }
#endif
        /* Fallthrough if we aren't doing SEAlink */

      default:                 /* State BS1.3 */
        TIMED_READ (0);
        time_release ();
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

  return (TIME_ERR);            /* State BS1.4 */
}

int
BSCheckFNm (XMARGSP args)
{
  if (args->result == SUCCESS)  /* State BS2.1 */
  {
    args->result = Internal_Telink_Send_File (args->filename, NULL);
    return (BSND3);
  }
  else                          /* State BS2.2 */
    return (FNAME_ERR);
}

int
BSCheckFile (XMARGSP args)
{
  return (args->result);
}

int
BSEndSend (XMARGSP args)
{
  long BS4Timer;

  BS4Timer = timerset (10 * PER_SECOND);
  while (!timeup (BS4Timer))
  {
    switch (TIMED_READ (1))
    {
    case NAK:
    case WANTCRC:
      goto Done;
    }
  }

Done:
  /* State BS4.2 */
  SENDBYTE (EOT);
  args->result = SUCCESS;
  return (SUCCESS);
}

int
Batch_Send (char *filename)
{
  XMARGS batch;
  int res;

  batch.result = 0;
  batch.filename = filename;
  res = state_machine ((STATEP) Batch_Sender, &batch, 2);
  status_line (">Batch_Send(%s) returns %d",
               filename == NULL ? "(null)" : filename, res);
  return (res == SUCCESS ? FILE_SENT_OK : CANNOT_SEND);
}

/* $Id: bsend.c,v 1.4 1999/03/20 04:08:20 mr Exp $ */
