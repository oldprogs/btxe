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
 * Filename    : $Source: E:/cvs/btxe/src/recvsync.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:40:05 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : FidoNet(R) Mail Session Called System Synchronization
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/*
   Receiver Synchronization state table.

   This logic is used by the called system. It will determine the
   type of mail transfer which can be used in communicating with
   the calling system.

   This stuff was originally copied from a file sent to us by TJ.
   (Of course, then we hacked the heck out of it!!!)

   Thanks, Tom.


   .-----+----------+-------------------------+-------------------------+-----.
   | RS0 | SyncInit |                         | Start 5 second idle tmr | RS1 |
   |-----+----------+-------------------------+-------------------------+-----|
   | RS1 | IdleWait | 1. 5 sec tmr expired    | Take the initiative     | RS2 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Carrier lost         | Session aborted         | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. Peek = YOOHOO        | Looks like a live WaZOO | RS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 4. Peek = TSYNC         | Live FSC001, we think   | RS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 5. Peek = CR, LF, space | He looks alive          | RS2 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 6. Peek = '*'           | Looks like a live EMSI  | RS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 7. Other character      | Eat it                  | RS1 |
   |-----+----------+-------------------------+-------------------------+-----|
   | RS2 |SendBanner| 1. Error returned       | Session aborted         | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Banner sent OK       |                         | RS3 |
   |-----+----------+-------------------------+-------------------------+-----|
   | RS3 |RecvInit  |                         | Init ext-mail scan ptrs | RS4 |
   |     |          |                         | Start 20 sec timer      |     |
   |     |          |                         | Init 10 sec timer       |     |
   |-----+----------+-------------------------+-------------------------+-----|
   | RS4 |SendSync  | 1. Error returned       | Session aborted         | exit|
   |     |(xmit sync+-------------------------+-------------------------+-----|
   |     |string)   | 2. String sent OK       | Watch for sender sync   | RS5 |
   |-----+----------+-------------------------+-------------------------+-----|
   | RS5 | WaitSync | 1. Carrier lost         | Session aborted         | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. YOOHOO received      | WaZOO session selected  | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. TSYNC received       | probable FSC001         | RS6 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 4. CR received          | Still sync'ing          | RS4 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 5. ESC received         | BBS session             | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 6. '*' received         | probable EMSI           | RS8 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 7. Other character rcvd | check ext-mail string   | RS7 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 8. 10 sec timer elapsed | FSC001 protocol selected| exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 9. 20 sec timer elapsed | Not a mail session      | exit|
   |-----+----------+-------------------------+-------------------------+-----|
   | RS6 | TsyncTmr | 1. Timer not running    | Start 10 second timer   | RS5 |
   |     |          |                         | Reset 20 sec timer      |     |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Timer running        | Two TSYNCS = FTS-0001   | exit|
   |-----+----------+-------------------------+-------------------------+-----|
   | RS7 | ExtMail  | 1. String matched       | External mail selected  | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Checked all strings  | Get next input character| RS5 |
   |-----+----------+-------------------------+-------------------------+-----|
   | RS8 | EMSI     | 1. String matched       | Call EMSI handshake     | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. No match             | Get next input character| RS5 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

/*
 * Data structure used by all RecvSync state machine functions.
 * Contains all data which needs to be passed between various states.
 *
 */

typedef struct
{
  long control;                 /* Must always start with a long!   */
  long Idle_Timer;              /* 5 second initial idle timer      */
  long BBS_Timer;               /* 20 second Timer for BBS user     */
  char *ExtMailScan[16];        /* Scan pointer for External Mail   */
  long TSYNC_Timer;             /* 10 second TSYNC timer            */
  char *emsi_ptr;               /* pointer for EMSI_INQ             */
  char *iemsi_ptr;              /* pointer for EMSI_CLI             */
  int emsi_flag;
  int result;                   /* Result we want to send out       */
}
RSARGS, *RSARGSP;

int RSSyncInit (RSARGSP);       /* Called by state machine at start */
int RSExit (RSARGSP);           /* Called by state machine at end   */
int RSIdleWait (RSARGSP);       /* RS1 state processing function    */
int RSSendBannr (RSARGSP);      /* RS2 state processing function    */
int RSRecvInit (RSARGSP);       /* RS3 state processing function    */
int RSSendSync (RSARGSP);       /* RS4 state processing function    */
int RSWaitSync (RSARGSP);       /* RS5 state processing function    */
int RSTsyncTmr (RSARGSP);       /* RS6 state processing function    */
int RSExtMail (RSARGSP);        /* RS7 state processing function    */
int RSEMSIChk (RSARGSP);        /* RS8 state processing function    */

#define RS0    0                /* Reserved value of 0 for init     */
#define RSexit 0                /* Slot 1 is exit, but called by 0  */
#define RS1    2                /* First "user" slot is 2.          */
#define RS2    3                /* After that, it all maps n : n+1  */
#define RS3    4
#define RS4    5
#define RS5    6
#define RS6    7
#define RS7    8
#define RS8    9

typedef struct
{
  char *state_name;
  int (*state_func) (RSARGSP);
}
RSTATES, *RSTATEP;

RSTATES Recv_Sync[] =
{                               /* Table used by state machine      */
  {"RSSyncInit", RSSyncInit},   /* And referred to by 'SSn' defines */
  {"RSExit", RSExit},           /* listed above ...                 */
  {"RSIdleWait", RSIdleWait},
  {"RSSendBannr", RSSendBannr},
  {"RSRecvInit", RSRecvInit},
  {"RSSendSync", RSSendSync},
  {"RSWaitSync", RSWaitSync},
  {"RSTsyncTmr", RSTsyncTmr},
  {"RSExtMail", RSExtMail},
  {"RSEMSIChk", RSEMSIChk},
};

/*
 * CalledRecvSync
 * Determine whether the calling system is an FTS-0001 mailer, an FTS-0006
 * mailer, an external mailer, or a human caller. Use the general state
 * machine driver.
 *
 * This is the only external entry point into this module.
 *
 */

int
CalledRecvSync (void)
{
  RSARGS args;
  int res;

  args.result = UNDEFINED;

  res = state_machine ((STATEP) Recv_Sync, &args, 2);
  return (res);
}

/*
 * This routine is called by the state machine when the 'RSexit'
 * state is seen. Its return value is what the state machine
 * will return to its caller as the result of the function.
 *
 */

int
RSExit (RSARGSP args)
{
  return (args->result);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS0 | SyncInit |                         | Start 2 second idle tmr | RS1 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSSyncInit (RSARGSP args)
{
  char buff[40];

  if (!no_EMSI_Session)
  {
    sprintf (buff, "%s\r              \r", emsistr[EMSI_REQ]);
    if (!SendBanner ((char far *) buff, 1))
    {
      args->result = UNDEFINED;
      return (RSexit);
    }
  }
  args->Idle_Timer = timerset (200);
  return (RS1);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS1 | IdleWait | 1. 2 sec tmr expired    | Take the initiative     | RS2 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Carrier lost         | Session aborted         | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. Peek = YOOHOO        | Looks like a live WaZOO | RS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 4. Peek = TSYNC         | Live FSC001, we think   | RS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 5. Peek = CR, LF, space | He looks alive          | RS2 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 6. Peek = '*'           | Looks like a live EMSI  | RS3 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 7. Other character      | Eat it                  | RS1 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSIdleWait (RSARGSP args)
{
  short i;
  int ret;

  while (!timeup (args->Idle_Timer))
  {
    if (!CARRIER)
    {
      args->result = UNDEFINED;
      return (RSexit);
    }

    if ((i = PEEKBYTE ()) == -1)
    {
      time_release ();          /*PLF Sun  12-01-1991  04:19:36 */
      continue;
    }

    switch (i & 0xff)
    {

    case TSYNC:
    case YOOHOO:
      ret = RS3;                /* Fast lane, no banner */
      break;

    case CR:
    case LF:
    case ' ':
    case ENQ:
    case ESC:
      ret = RS2;                /* He's alive, do banner */
      break;

    case '*':
      if (!no_EMSI_Session)
      {
        ret = RS3;              /* Fast lane, no banner */
        break;
      }

      /* Fall through if not EMSI */

    default:
      TIMED_READ (0);           /* eat it up            */
      ret = RS1;
      break;
    }

    return (ret);
  }

  return (RS2);                 /* Idle for 5 seconds   */
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS2 |SendBanner| 1. Error returned       | Session aborted         | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Banner sent OK       |                         | RS3 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSSendBannr (RSARGSP args)
{
  char buff[128];

  if (!no_EMSI_Session && (PEEKBYTE () != '*'))
  {
    sprintf (buff, "%s\r", emsistr[EMSI_REQ]);
    if (!SendBanner ((char far *) buff, 1))
    {
      args->result = UNDEFINED;
      return (RSexit);
    }
  }

  sprintf (buff, MSG_TXT (M_ADDRESS), Full_Addr_Str (&my_addr), ANNOUNCE,
           ((serial == -1) ? MSG_TXT (M_UNREGISTERED) : " "));

  if (!SendBanner ((char far *) buff, 0))
  {
    args->result = UNDEFINED;
    return (RSexit);
  }

  if (PEEKBYTE () != '*')
  {
    if (InUseBBSBanner != NULL)
    {                           /* If BBS name spec'ed, */
      SENDBYTE ('\r');          /* make sure user gets  */
      SENDBYTE ('\n');          /* a full cr/lf...      */

      if (!SendBanner ((char far *) InUseBBSBanner, 0))
      {
        args->result = UNDEFINED;
        return (RSexit);
      }
    }

    SENDBYTE ('\r');            /* make sure user gets  */
    SENDBYTE ('\n');            /* a full cr/lf...      */
  }

  if (!mail_only && (PEEKBYTE () != '*'))
  {
    if (!SendBanner (BBSwelcome, 0))
    {
      args->result = UNDEFINED;
      return (RSexit);
    }
  }

  return (RS3);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS3 |RecvInit  |                         | Init ext-mail scan ptrs | RS4 |
   |     |          |                         | Start 20 sec timer      |     |
   |     |          |                         | Init 10 sec timer       |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSRecvInit (RSARGSP args)
{
  register int k;

  for (k = 0; k < num_ext_mail; k++)
  {
    args->ExtMailScan[k] = ext_mail_string[k];  /* UUCP handshake   */
  }

  args->emsi_ptr = emsistr[EMSI_INQ];
  args->iemsi_ptr = emsistr[EMSI_CLI];
  args->emsi_flag = FALSE;

  args->BBS_Timer = timerset (BBStimeout);  /* 20 second timeout    */
  args->TSYNC_Timer = 0L;       /* Initialize for test */

  if (!no_EMSI_Session && (PEEKBYTE () == '*'))
    return (RS5);
  else
    return (RS4);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS4 |SendSync  | 1. Error returned       | Session aborted         | exit|
   |     |(xmit sync+-------------------------+-------------------------+-----|
   |     |string)   | 2. String sent OK       | Watch for sender sync   | RS5 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSSendSync (RSARGSP args)
{
  char far *c;
  char buff[128];

  if (!no_EMSI_Session)
  {
    /* If we already have a '*', just go handle it. */

    if (PEEKBYTE () == '*')
      return (RS5);

    sprintf (buff, "%s\r", emsistr[EMSI_REQ]);
    if (!SendBanner ((char far *) buff, 1))
    {
      args->result = UNDEFINED;
      return (RSexit);
    }
  }

  if (mail_only)
  {                             /* If no BBS allowed,   */
    c = (char far *) noBBS;     /* tell human to git    */
  }
  else
  {
    c = (char far *) BBSesc;    /* or hit ESC for BBS   */
  }

  if (!SendBanner (c, mail_only))
  {
    args->result = UNDEFINED;
    return (RSexit);
  }

  return (RS5);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS5 | WaitSync | 1. Carrier lost         | Session aborted         | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. YOOHOO received      | WaZOO session selected  | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 3. TSYNC received       | probable FSC001         | RS6 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 4. CR received          | Still sync'ing          | RS4 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 5. ESC received         | BBS session             | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 6. '*' received         | probable EMSI           | RS8 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 7. Other character rcvd | check ext-mail string   | RS7 |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 8. 10 sec timer elapsed | FSC001 protocol selected| exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 9. 20 sec timer elapsed | Not a mail session      | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSWaitSync (RSARGSP args)
{
  short i;
  int ret = -1;

  while (CARRIER)
  {
    if (got_ESC ())
    {                           /* Manual abort?        */
      modem_hangup ();          /* Yes, drop carrier    */
      args->result = UNDEFINED;
      return (RSexit);
    }

    /* Note: the following line is a NON-DESTRUCTIVE READ! */
    switch ((i = (short) PEEKBYTE ()) & 0xff)
    {
    case YOOHOO:               /* Looks like a WaZOO   */
      if (no_WaZOO_Session)
      {                         /* If we're not WaZOO,  */
        TIMED_READ (0);         /* Eat the YooHoo       */
        continue;
      }

      CLEAR_OUTBOUND ();        /* End noisy banner now */
      args->result = RECVED_WAZOO;  /* WaZOO session        */
      ret = RSexit;
      break;

    case TSYNC:                /* Looks like an FTSC   */
      CLEAR_OUTBOUND ();        /* End noisy banner now */
      TIMED_READ (0);           /* Eat the TSYNC        */

      if (!no_WaZOO_Session)
      {                         /* If we support WaZOO, */
        ret = RS6;              /* Go set up the timer  */
      }
      else
      {
        args->result = RECVED_FTSC;  /* FTS-0001 session     */
        ret = RSexit;
      }
      break;

    case CR:                   /* Still sync'ing       */
      TIMED_READ (0);           /* Eat the character    */
      ret = RS4;                /* Back to that stuff   */
      break;

    case ESC:                  /* User wants the BBS   */
      TIMED_READ (0);           /* Eat the character    */
      if (!mail_only)
      {
        args->result = RECVED_BBS;  /* BBS session          */
        ret = RSexit;
      }
      break;

    case '*':                  /* Looks like a EMSI    */
      if (!no_EMSI_Session)
      {                         /* If we're EMSI,       */
        CLEAR_OUTBOUND ();      /* End noisy banner now */
        ret = RS8;
        break;
      }
      /* Fall through if not EMSI */

    default:                   /* A less special char  */
      if (i == -1)
        time_release ();        /*PLF 12-1-91  04:21:14 */
      else
      {
        if (args->emsi_flag)
          ret = RS8;
        else
          ret = RS7;            /* Check ext mailers    */
      }
      break;
    }

    if (ret >= 0)
      return (ret);

    if ((args->TSYNC_Timer != 0L) && (timeup (args->TSYNC_Timer)))
    {
      args->result = RECVED_FTSC;  /* FTS-0001 session     */
      return (RSexit);
    }

    if (timeup (args->BBS_Timer))
    {                           /* The big timeout      */
      args->result = RECVED_BBS;  /* BBS session          */
      return (RSexit);
    }

    time_release ();            /* Give up the slice    */
  }                             /* End (while CARRIER) */

  args->result = UNDEFINED;     /* carrier loss = abort */
  return (RSexit);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS6 | TsyncTmr | 1. Timer not running    | Start 10 second timer   | RS5 |
   |     |          |                         | Reset 20 sec timer      |     |
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Timer running        | Two TSYNCS = FTS-0001   | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSTsyncTmr (RSARGSP args)
{
  if (args->TSYNC_Timer == 0L)
  {                             /* If not already set,  */
    args->TSYNC_Timer = timerset (1000);  /* Then just set it     */
    args->BBS_Timer = timerset (BBStimeout);  /* Reset 20 sec timer   */
    return (RS5);
  }

  args->result = RECVED_FTSC;   /* FTS-0001 session     */
  return (RSexit);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS7 | ExtMail  | 1. String matched       | External mail selected  | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. Checked all strings  | Get next input character| RS5 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSExtMail (RSARGSP args)
{
  short i, k;

  /* Note: last character was read in RS5 using a NON-DESTRUCTIVE READ.
     We now will get it out with a DESTRUCTIVE READ. */

  i = TIMED_READ (0) & 0xff;    /* Eat the character     */

  for (k = 0; k < num_ext_mail; k++)
  {
    if (i != ((short) *(args->ExtMailScan[k]++) & 0xff))
    {                           /* Does this match next? */
      args->ExtMailScan[k] = ext_mail_string[k];  /* No, reset pointer */
    }
    else
    {
      if (!*args->ExtMailScan[k])
      {                         /* End of UUCP string?   */
        args->result = RECVED_UUCP + k;  /* 5 + n for external    */
        return (RSexit);
      }
    }
  }

  return (RS5);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RS8 | EMSI     | 1. String matched       | Call EMSI handshake     | exit|
   |     |          +-------------------------+-------------------------+-----|
   |     |          | 2. No match             | Get next input character| RS5 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RSEMSIChk (RSARGSP args)
{
  short i;

  /* Note: last character was read in RS5 using a NON-DESTRUCTIVE READ.
     We now will get it out with a DESTRUCTIVE READ. */

  args->emsi_flag = TRUE;

  i = TIMED_READ (0);           /* Eat the character     */
  i = toupper (i & 0xff);

  if ((i != (short) (*(args->emsi_ptr))) &&
      (i != (short) (*(args->iemsi_ptr))))
  {                             /* Does this match next? */
    args->emsi_ptr = emsistr[EMSI_INQ];  /* No, reset pointer     */
    args->iemsi_ptr = emsistr[EMSI_CLI];
    args->emsi_flag = FALSE;
  }
  else
  {
    if (i == (short) (*(args->emsi_ptr)))
      args->emsi_ptr++;

    if (!*args->emsi_ptr)
    {                           /* End of EMSI string?   */
      args->result = RECVED_EMSI;  /* 4 = EMSI */
      return (RSexit);
    }

    if (i == (short) (*(args->iemsi_ptr)))
      args->iemsi_ptr++;

    if (!*args->iemsi_ptr && !mail_only)
    {                           /* End of EMSI string?   */
      args->result = RECVED_BBS;  /* 1 = BBS, fast IEMSI */
      return (RSexit);
    }
  }

  return (RS5);
}

/* $Id: recvsync.c,v 1.4 1999/03/23 22:40:05 mr Exp $ */
