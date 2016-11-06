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
 * Filename    : $Source: E:/cvs/btxe/src/sendsync.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:40:07 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : FidoNet(R) Mail Session Calling System Synchronization.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/* Sender Synchronization state table.

   This logic is used by the calling system. It will determine the
   type of mail transfer which can be used in communicating with
   the called system.

   This stuff was copied almost verbatim from a file sent to us by TJ.
   (Of course, then we hacked the heck out of it!!!)

   Thanks, Tom.

   .-----+----------+-------------------------+-------------------------+-----.
   | SS0 | SyncInit |                         | Prepare 3 sec Sync timer|     |
   |     |          |                         | Prepare .5 sec NAK tmr  |     |
   |     |          |                         | Init NAK Count          |     |
   |     |          |                         | Start 60 sec master tmr | SS1 |
   |-----+----------+-------------------------+-------------------------+-----|
   | SS1 | SendSync | 1. Over 60 seconds      |                         |     |
   |     |          |    or carrier lost      | no response             | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. 3 sec elapsed        | Send YOOHOO, then TSYNC |     |
   |     |          |    or timer not started | Start 3 sec Sync timer  | SS2 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. not elapsed          |                         | SS2 |
   |-----+----------+-------------------------+-------------------------+-----|
   | SS2 | WaitResp | 1. Nothing received     | require a response      | SS1 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. ENQ received         | WaZOO Protocol selected | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. '*' received         | probable EMSI           | SS5 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 4. 'C' received         | probable FSC001         | SS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 5. NAK received         | probable FSC001         | SS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 6. Debris (might include| Reset NAK timer         |     |
   |     |          |    (YOOHOO|TSYNC) & 127)| if started              | SS1 |
   |-----+----------+-------------------------+-------------------------+-----|
   | SS3 | NAKTmr   | 1. Timer not expired    | Zero NAK count          |     |
   |     |          |    or timer not started | Start .5 sec NAK timer  | SS1 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Timer expired        | Bump NAK count          | SS4 |
   |-----+----------+-------------------------+-------------------------+-----|
   | SS4 | NAKCount | 1. Count >= 2?          | assume FSC001           | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Count < 2            | Keep looking            | SS1 |
   |-----+----------+-------------------------+-------------------------+-----|
   | SS5 | EMSI     | 1. String matched       | Check for sync chars    | SS6 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. No match             | Get next input character| SS1 |
   |-----+----------+-------------------------+-------------------------+-----|
   | SS6 | TestSync | 1. No TSYNC or YOOHOO   | Call EMSI handshake     | exit|
   |     |          | sent or 3 sec of quiet  |                         |     |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. ENQ, 'C' or NAK seen | Toss EMSI, try again    | SS2 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

/* Data structure used by all SendSync state machine functions.
 * Contains all data which needs to be passed between various states. */

typedef struct
{
  long control;                 /* Must always start with a long!   */
  long NAK_Timer;               /* 1/2 second NAK interval timer    */
  long SendSyncTimer;           /* 3 second SendSync Timer          */
  long Master_Timer;            /* 60 second master timeout         */
  int NAK_Count;                /* Count of NAK's received          */
  char *emsi_ptr;               /* pointer for EMSI_INQ             */
  int emsi_flag;
  int sync_flag;
  int sent_emsibanner;          /* send it only 1 time per sync     */
  int CR_count;
  int result;                   /* Result we want to send out       */
}
SSARGS, *SSARGSP;

int SSSyncInit (SSARGSP);       /* Called by state machine at start */
int SSExit (SSARGSP);           /* Called by state machine at end   */
int SSSendSync (SSARGSP);       /* SS1 state processing function    */
int SSWaitResp (SSARGSP);       /* SS2 state processing function    */
int SSNAKTmr (SSARGSP);         /* SS3 state processing function    */
int SSNAKCount (SSARGSP);       /* SS4 state processing function    */
int SSTestEMSI (SSARGSP);       /* SS5 state processing function    */
int SSTestSync (SSARGSP);       /* SS6 state processing function    */

#define SS0    0                /* Reserved value of 0 for init     */
#define SSexit 0                /* Slot 1 is exit, but called by 0  */
#define SS1    2                /* First "user" slot is 2.          */
#define SS2    3                /* After that, it all maps n : n+1  */
#define SS3    4
#define SS4    5
#define SS5    6
#define SS6    7

typedef struct
{
  char *state_name;
  int (*state_func) (SSARGSP);
}
SSTATES, *SSTATEP;

SSTATES Send_Sync[] =
{                               /* Table used by state machine      */
  {"SSSyncInit", SSSyncInit},   /* And referred to by 'SSn' defines */
  {"SSExit", SSExit},           /* listed above ...                 */
  {"SSSendSync", SSSendSync},
  {"SSWaitResp", SSWaitResp},
  {"SSNAKTmr", SSNAKTmr},
  {"SSNAKCount", SSNAKCount},
  {"SSTestEMSI", SSTestEMSI},
  {"SSTestSync", SSTestSync}
};

/* CallerSendSync
 * Determine whether we are talking to an FTS-0001 mailer or an FTS-0006
 * mailer. Use the general state machine driver.
 *
 * This is the only external entry point into this module. */

int
CallerSendSync (void)
{
  SSARGS args;
  int res;

  args.result = UNDEFINED;

  res = state_machine ((STATEP) Send_Sync, &args, 2);
  return (res);
}

/* This routine is called by the state machine when the 'SSexit'
 * state is seen. Its return value is what the state machine
 * will return to its caller as the result of the function. */

int
SSExit (SSARGSP args)
{
  return (args->result);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SS0 | SyncInit |                         | Prepare 3 sec Sync timer|     |
   |     |          |                         | Prepare .5 sec NAK tmr  |     |
   |     |          |                         | Init NAK Count          |     |
   |     |          |                         | Start 60 sec master tmr | SS1 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SSSyncInit (SSARGSP args)
{
  args->NAK_Timer = 0L;
  args->SendSyncTimer = 0L;
  args->NAK_Count = 0;
  args->emsi_ptr = emsistr[EMSI_REQ];
  args->emsi_flag = FALSE;
  args->sync_flag = FALSE;
  args->sent_emsibanner = FALSE;
  args->CR_count = 0;
  if (!no_EMSI_Session)
    args->SendSyncTimer = timerset (200);  /* HJK - Back to 200... */
  args->Master_Timer = timerset (6000);
  return (SS1);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SS1 | SendSync | 1. Over 60 seconds      |                         |     |
   |     |          |    or carrier lost      | no response             | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. 3 sec elapsed        | Send YOOHOO, then TSYNC |     |
   |     |          |    or timer not started | Start 3 sec Sync timer  | SS2 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. not elapsed          |                         | SS2 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SSSendSync (SSARGSP args)
{
  if (!(CARRIER) || (timeup (args->Master_Timer)))
  {
    args->result = UNDEFINED;   /* Failure */
    return (SSexit);
  }

  if (got_ESC ())
  {
    status_line (MSG_TXT (M_CONNECT_ABORTED));
    modem_hangup ();
    return (SSexit);
  }

  if (!no_EMSI_Session && CHAR_AVAIL ())
    return (SS2);

  if (!(args->SendSyncTimer) || timeup (args->SendSyncTimer))
  {
    CLEAR_INBOUND ();
    if (!no_WaZOO_Session)      /* If we're WaZOO,   */
      SENDBYTE (YOOHOO);
    SENDBYTE (TSYNC);
    args->sync_flag = TRUE;
    args->SendSyncTimer = timerset (300);
  }
  return (SS2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SS2 | WaitResp | 1. Nothing received     | require a response      | SS1 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. ENQ received         | WaZOO Protocol selected | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. '*' received         | probable EMSI           | SS5 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 4. 'C' received         | probable FSC001         | SS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 5. NAK received         | probable FSC001         | SS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 6. Debris (might include| Reset NAK timer         |     |
   |     |          |    (YOOHOO|TSYNC) & 127)| if started              | SS1 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SSWaitResp (SSARGSP args)
{
  short i;
  int exit_code;

  if (!CHAR_AVAIL ())
  {
    time_release ();            /*PLF Sun  12-01-1991  04:46:58 */
    return (SS1);
  }

  if (args->emsi_flag)
    return (SS5);

  i = PEEKBYTE ();

  switch (i)
  {

  case 'C':
  case NAK:
    exit_code = SS3;
    break;

  case '*':
    if (!no_EMSI_Session)
    {                           /* If we're EMSI,   */
      exit_code = SS5;
      break;
    }
    goto unknown;

  case 0x0d:                   /* Return */
    /* If we get two of these with no sync, give up on EMSI. */
    if ((++args->CR_count) == 2 && !args->sync_flag)
    {
      args->SendSyncTimer = 0;
      TIMED_READ (0);
      exit_code = SS1;
      break;
    }
    goto unknown;

  case ENQ:
    /* If we get this before a YooHoo, send one. */
    if (!args->sync_flag)
      args->SendSyncTimer = 0;
    else if (!no_WaZOO_Session)
    {                           /* If we're WaZOO,   */
      args->result = CALLED_YOOHOO;  /* WaZOO */
      exit_code = SSexit;
      break;
    }
    /* Deliberately fall through from ENQ if we're not doing WaZOO */

  default:

  unknown:
    if (i == -1)
      time_release ();
    else
      TIMED_READ (0);

    if (args->NAK_Timer)
    {
      args->NAK_Timer = timerset (50);
    }
    exit_code = SS1;
    break;
  }

  return (exit_code);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SS3 | NAKTmr   | 1. Timer not expired    | Zero NAK count          |     |
   |     |          |    or timer not started | Start .5 sec NAK timer  | SS1 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Timer expired        | Bump NAK count          | SS4 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SSNAKTmr (SSARGSP args)
{
  if (!(args->NAK_Timer) || !timeup (args->NAK_Timer))
  {
    args->NAK_Count = 0;
    args->NAK_Timer = timerset (50);
    TIMED_READ (0);
    return (SS1);
  }
  else
  {
    (args->NAK_Count)++;
    return (SS4);
  }
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SS4 | NAKCount | 1. Count >= 2?          | assume FSC001           | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Count < 2            | Keep looking            | SS1 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SSNAKCount (SSARGSP args)
{
  if (no_WaZOO_Session || args->NAK_Count >= 2)
  {
    args->result = CALLED_FTSC; /* FSC001 */
    return (SSexit);
  }
  else
  {
    TIMED_READ (0);
    return (SS1);
  }
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SS5 | EMSI     | 1. String matched       | Check for sync chars    | SS6 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. No match             | Get next input character| SS2 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SSTestEMSI (SSARGSP args)
{
  short i;

  i = PEEKBYTE ();              /* Get the character     */

  if ((int) toupper (i & 0xff) != (int) (*(args->emsi_ptr++)))
  {                             /* Does this match next? */
    args->emsi_ptr = emsistr[EMSI_REQ];  /* No, reset pointer */
    args->emsi_flag = FALSE;
    args->SendSyncTimer = timerset (300);
  }
  else
  {
    TIMED_READ (0);             /* Eat the character     */
    if (!*args->emsi_ptr)       /* Was this the last one? */
      return (SS6);
    args->emsi_flag = TRUE;
    args->SendSyncTimer = timerset (300);
  }

  return (SS2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SS6 | TestSync | 1. No TSYNC or YOOHOO   | Call EMSI handshake     | exit|
   |     |          | sent or 3 sec of quiet  |                         |     |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. ENQ, 'C' or NAK seen | Toss EMSI, try again    | SS2 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SSTestSync (SSARGSP args)
{
  char buff[200];

  /* Wait for 3 secs of quiet if we ever sent TSYNC or YOOHOO. During
   * that period, if we see something that might be a response to the
   * TSYNC or YOOHOO, toss the EMSI stuff and try again. */

  if (args->sync_flag)
  {
    short i;
    long t = timerset (300);

    status_line (">Waiting for called system to quiet.");
    while (!timeup (t))
    {
      i = PEEKBYTE ();

      /* Is anything there right now? */
      if (i == -1)
      {
        time_release ();
        continue;
      }

      /* Was it a session startup? */
      if (i == 'C' || i == NAK || i == ENQ || i == '*')
      {
        status_line (">Found called sync, try again.");
        args->emsi_ptr = emsistr[EMSI_REQ];
        args->emsi_flag = FALSE;
        args->SendSyncTimer = timerset (300);
        return (SS2);
      }
      else
      {
        /* No. Eat it and reset the timer. */
        TIMED_READ (0);
        t = timerset (300);
      }
    }

    if (no_EMSI_Session)        /* if no EMSI allowed */
      return (SS2);

    status_line (">All quiet, let's do EMSI.");
  }

  /* TJW 960717 begin of changes
   * sometimes the first chars get lost, so send some unimportant shit first: */

  if (EMSIbanner && !args->sent_emsibanner)
  {
    sprintf (buff, ANNOUNCE);   /* HJK 971012 Sent strings caused some trouble in MT environment */

    if (!SendBanner ((char far *) buff, 0))
    {
      args->result = UNDEFINED;
      return (SSexit);
    }
    args->sent_emsibanner = TRUE;
  }

  /* TJW 960717 end of changes */

  /* We're going with EMSI. Send EMSI_INQ and go with it. */
  SENDCHARS (emsistr[EMSI_INQ], strlen (emsistr[EMSI_INQ]), 1);
  SENDBYTE ('\r');
  args->result = CALLED_EMSI;   /* 4 = EMSI */
  return (SSexit);
}

/* $Id: sendsync.c,v 1.5 1999/03/23 22:40:07 mr Exp $ */
