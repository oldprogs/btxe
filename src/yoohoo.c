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
 * Filename    : $Source: E:/cvs/btxe/src/yoohoo.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:40:13 $
 * State       : $State: Exp $
 *
 * Description : BinkleyTerm "YooHoo" Processor
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int Send_Hello_Packet (int);
int Recv_Hello_Packet (int);

/* Data structure used by all YooHoo state machine functions.
 * Contains all data which needs to be passed between various states. */

typedef struct
{
  long control;                 /* Must always start with a long!   */
  long timer1;                  /* Outermost timer                  */
  long timer2;                  /* Next inner timer                 */
  long timer3;                  /* Next inner timer                 */
  struct _Hello Hello;          /* Data packet                      */
  word crc;                     /* CRC of data packet               */
  word hiscrc;                  /* other's CRC of data packet       */
  int sender;                   /* Set to 1 if we're calling him    */
  int result;                   /* Result we want to send out       */
  int retries;                  /* Number of retries                */
  int count;                    /* Number of characters received    */
}
YHARGS, *YHARGSP;

int YRInit (YHARGSP);           /* Called by state machine at start */
int YRExit (YHARGSP);           /* Called by state machine at end   */
int YRGetHello (YHARGSP);       /* YR1 state processing function    */
int YRWaitResp (YHARGSP);       /* YR2 state processing function    */
int YRPollPeer (YHARGSP);       /* YR3 state processing function    */
int YRSndHello (YHARGSP);       /* YR4 state processing function    */

int YSInit (YHARGSP);           /* Called by state machine at start */
int YSExit (YHARGSP);           /* Called by state machine at end   */
int YSSndHello (YHARGSP);       /* YS1 state processing function    */
int YSWaitResp (YHARGSP);       /* YS2 state processing function    */
int YSGetHello (YHARGSP);       /* YS3 state processing function    */

int SHInit (YHARGSP);           /* Called by state machine at start */
int SHExit (YHARGSP);           /* Called by state machine at end   */
int SHInitSend (YHARGSP);       /* SH1 state processing function    */
int SHSendHedr (YHARGSP);       /* SH2 state processing function    */
int SHSendCRC (YHARGSP);        /* SH3 state processing function    */
int SHGetResp (YHARGSP);        /* SH4 state processing function    */
int SHBumpCnt (YHARGSP);        /* SH5 state processing function    */

int RHInit (YHARGSP);           /* Called by state machine at start */
int RHExit (YHARGSP);           /* Called by state machine at end   */
int RHSendENQ (YHARGSP);        /* RH1 state processing function    */
int RHWaitHedr (YHARGSP);       /* RH2 state processing function    */
int RHTossJunk (YHARGSP);       /* RH3 state processing function    */
int RHReSynch (YHARGSP);        /* RH4 state processing function    */
int RHHdrSetup (YHARGSP);       /* RH5 state processing function    */
int RHGetHChar (YHARGSP);       /* RH6 state processing function    */
int RHStoHChar (YHARGSP);       /* RH7 state processing function    */
int RHCheckCRC (YHARGSP);       /* RH8 state processing function    */
int RHCountERR (YHARGSP);       /* RH9 state processing function    */
int RHHelloOK (YHARGSP);        /* RH10 state processing function   */

#define YR0    0                /* Reserved value of 0 for init     */
#define YRexit 0                /* Slot 1 is exit, but called by 0  */
#define YR1    2                /* First "user" slot is 2.          */
#define YR2    3                /* After that, it all maps n : n+1  */
#define YR3    4
#define YR4    5

#define YS0    0                /* Reserved value of 0 for init     */
#define YSexit 0                /* Slot 1 is exit, but called by 0  */
#define YS1    2                /* First "user" slot is 2.          */
#define YS2    3                /* After that, it all maps n : n+1  */
#define YS3    4

#define SH0    0                /* Reserved value of 0 for init     */
#define SHexit 0                /* Slot 1 is exit, but called by 0  */
#define SH1    2                /* First "user" slot is 2.          */
#define SH2    3                /* After that, it all maps n : n+1  */
#define SH3    4
#define SH4    5
#define SH5    6

#define RH0    0                /* Reserved value of 0 for init     */
#define RHexit 0                /* Slot 1 is exit, but called by 0  */
#define RH1    2                /* First "user" slot is 2.          */
#define RH2    3                /* After that, it all maps n : n+1  */
#define RH3    4
#define RH4    5
#define RH5    6
#define RH6    7
#define RH7    8
#define RH8    9
#define RH9    10
#define RH10   11

typedef struct
{
  char *state_name;
  int (*state_func) (YHARGSP);
}
YSTATES, *YSTATEP;

YSTATES YooHoo_Rcvr[] =
{                               /* Table used by state machine      */
  {"YRInit", YRInit},           /* And referred to by 'YRn' defines */
  {"YRExit", YRExit},           /* listed above ...                 */
  {"YRGetHello", YRGetHello},
  {"YRWaitResp", YRWaitResp},
  {"YRPollPeer", YRPollPeer},
  {"YRSndHello", YRSndHello},
};

YSTATES YooHoo_Sndr[] =
{                               /* Table used by state machine      */
  {"YSInit", YSInit},           /* And referred to by 'YRn' defines */
  {"YSExit", YSExit},           /* listed above ...                 */
  {"YSSndHello", YSSndHello},
  {"YSWaitResp", YSWaitResp},
  {"YSGetHello", YSGetHello},
};

YSTATES Snd_Hello[] =
{                               /* Table used by state machine      */
  {"SHInit", SHInit},           /* And referred to by 'SHn' defines */
  {"SHExit", SHExit},           /* listed above ...                 */
  {"SHInitSend", SHInitSend},
  {"SHSendHedr", SHSendHedr},
  {"SHSendCRC", SHSendCRC},
  {"SHGetResp", SHGetResp},
  {"SHBumpCnt", SHBumpCnt},
};

YSTATES Rcv_Hello[] =
{                               /* Table used by state machine      */
  {"RHInit", RHInit},           /* And referred to by 'RHn' defines */
  {"RHExit", RHExit},           /* listed above ...                 */
  {"RHSendENQ", RHSendENQ},
  {"RHWaitHedr", RHWaitHedr},
  {"RHTossJunk", RHTossJunk},
  {"RHReSynch", RHReSynch},
  {"RHHdrSetup", RHHdrSetup},
  {"RHGetHChar", RHGetHChar},
  {"RHStoHChar", RHStoHChar},
  {"RHCheckCRC", RHCheckCRC},
  {"RHCountERR", RHCountERR},
  {"RHHelloOK", RHHelloOK},
};

/* ------------------------------------------------------------------------ */
/* YOOHOO SENDER    (used when I am the CALLING system)                     */
/* ------------------------------------------------------------------------ */
int
YooHoo_Sender (void)
{
  YHARGS args;
  int res;

  args.result = 0;

  res = state_machine ((STATEP) YooHoo_Sndr, &args, 2);
  return (res);
}

int
YSInit (YHARGSP args)
{
  do_status (M_STATUS_YOOHOO, NULL);

  /* ------------------------------------------------------------------ */
  /* Clean up any mess that may be around                               */
  /* ------------------------------------------------------------------ */

  CLEAR_OUTBOUND ();
  CLEAR_INBOUND ();
  XON_DISABLE ();

  happy_compiler = args->result;

  return (YS1);
}

/* This routine is called by the state machine when the '??exit'
 * state is seen. Its return value is what the state machine
 * will return to its caller as the result of the function. */

int
YSExit (YHARGSP args)
{
  return (args->result);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | YS1 | SndHello | Successful              | Looks like WaZOO        | YS2 |
   |     | (state   +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |  SH1)    | Not successful          | Repeat whole thing      | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
YSSndHello (YHARGSP args)
{
  if (!Send_Hello_Packet (1))
  {
    status_line (MSG_TXT (M_HE_HUNG_UP));
    return (YSexit);
  }

  happy_compiler = args->result;
  return (YS2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | YS2 | WaitResp | 30 sec timer expires    | repeat whole thing      | exit|
   |     |          | or lost carrier         |                         |     |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Received YOOHOO         | Another WaZOO, go       | YS3 |
   |     |          |                         | process receive         |     |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Received debris         | Repeat whole thing      | YS2 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
YSWaitResp (YHARGSP args)
{
  long resp_timer = timerset (30 * PER_SECOND);

  while (TIMED_READ (5) != YOOHOO)
  {
    if (timeup (resp_timer))
    {
      status_line (MSG_TXT (M_NO_YOOHOO_2U2));
      status_line (MSG_TXT (M_I_DONT_KNOW));
      return (YSexit);
    }
  }

  happy_compiler = args->result;
  return (YS3);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | YS3 | GetHello | Information             | Report Success          | exit|
   |     | (state   | Successfully            |                         |     |
   |     |  RH1)    | Exchanged               |                         |     |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Failure                 | Repeat whole thing      | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
YSGetHello (YHARGSP args)
{
  args->result = Recv_Hello_Packet (1);
  return (YSexit);
}

/* ------------------------------------------------------------------------ */
/* YOOHOO RECEIVER  (Used when I am the CALLED system)                      */
/* ------------------------------------------------------------------------ */

int
YooHoo_Receiver (void)
{
  YHARGS args;
  int res;

  args.result = 0;

  res = state_machine ((STATEP) YooHoo_Rcvr, &args, 2);
  return (res);
}

int
YRInit (YHARGSP args)
{
  do_status (M_STATUS_YOOHOO, NULL);

  /* ------------------------------------------------------------------ */
  /* Clean up any mess that may be around                               */
  /* ------------------------------------------------------------------ */

  CLEAR_OUTBOUND ();
  CLEAR_INBOUND ();
  XON_DISABLE ();

  happy_compiler = args->result;

  return (YR1);
}

/* This routine is called by the state machine when the '??exit'
 * state is seen. Its return value is what the state machine
 * will return to its caller as the result of the function. */

int
YRExit (YHARGSP args)
{
  return (args->result);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | YR1 | GetHello | Information             | Start 20 sec timer      | YR2 |
   |     | (state   | Successfully            | Initialize retry count  |     |
   |     |  RH1)    | Exchanged               | Send YooHoo             |     |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Failure                 | Repeat whole thing      | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
YRGetHello (YHARGSP args)
{
  if (Recv_Hello_Packet (0) == 0)
    return (YRexit);
  args->timer1 = timerset (20 * PER_SECOND);
  args->retries = 0;
  SENDBYTE (YOOHOO);
  return (YR2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | YR2 | WaitResp | 20 sec timeout          | try again               | YR3 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Lost carrier            | Failure                 | exit|
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Received ENQ            | Go send hello           | YR4 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Received debris         | Keep looking            | YR2 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
YRWaitResp (YHARGSP args)
{
  while (TIMED_READ (5) != ENQ)
  {
    if (!CARRIER)
    {
      status_line (MSG_TXT (M_NO_CARRIER));
      return (YRexit);
    }
    if (timeup (args->timer1))
      return (YR3);
  }
  return (YR4);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | YR3 | PollPeer | More than 3 retries     | Give it up              | exit|
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Less than 3 retries     | Bump retry count        | YR2 |
   |     |          |                         | Clear input buffer      |     |
   |     |          |                         | Send YOOHOO             |     |
   |     |          |                         | Restart 20 sec timer    |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
YRPollPeer (YHARGSP args)
{
  if (++(args->retries) > 3)
  {
    status_line (MSG_TXT (M_FUBAR_MSG));
    b_init ();
    return (YRexit);
  }

  CLEAR_INBOUND ();
  SENDBYTE (YOOHOO);
  args->timer1 = timerset (20 * PER_SECOND);
  return (YR2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | YR4 | SndHello | Successful              | All done, report success| exit|
   |     | (state   +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |  SH1)    | Not successful          | Repeat whole thing      | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
YRSndHello (YHARGSP args)
{
  if ((args->result = Send_Hello_Packet (0)) == 0)
    b_init ();
  return (YRexit);
}

/* ------------------------------------------------------------------------ */
/* SEND HELLO PACKET                                                        */
/* ------------------------------------------------------------------------ */

int
Send_Hello_Packet (int Sender)
{
  YHARGS args;
  int res;

  args.result = 0;
  args.sender = Sender;

  n_getpassword (&remote_addr); /* Update "assumed" */

  res = state_machine ((STATEP) Snd_Hello, &args, 2);
  return (res);
}

int
SHInit (YHARGSP args)
{
  happy_compiler = args->result;

  return (SH1);
}

/* This routine is called by the state machine when the '??exit'
 * state is seen. Its return value is what the state machine
 * will return to its caller as the result of the function. */

int
SHExit (YHARGSP args)
{
  return (args->result);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SH1 | InitSend |                         | Disable XON/XOFF        | SH2 |
   |     |          |                         | Set retry count to 0    |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SHInitSend (YHARGSP args)
{
  XON_DISABLE ();
  args->retries = 0;

  return (SH2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SH2 | SendHedr |                         | Send Hex 1f, then       | SH3 |
   |     |          |                         | Send HELLO struct       |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SHSendHedr (YHARGSP args)
{
  word can_do_domain = 0;
  word crc;
  char *sptr;
  int i;
  AKA *aka;

  if (alias == NULL)
    return (SHexit);

  for (i = 0, aka = alias; (i < assumed) && (aka != NULL); i++, aka = aka->next) ;

  if (aka == NULL)
    aka = alias;

  /* ------------------------------------------------------------------ */
  /* Setup HELLO structure                                              */
  /* ------------------------------------------------------------------ */

  memset ((char *) &args->Hello, 0, sizeof (struct _Hello));

  args->Hello.signal = 'o';
  args->Hello.hello_version = 1;

  args->Hello.product = PRDCT_CODE;
  args->Hello.product_maj = (word) BINK_MAJVERSION;
  args->Hello.product_min = (word) BINK_MINVERSION;

  strntcpy (args->Hello.sysop, sysop, 20);

  args->Hello.my_zone = aka->Zone;
  if ((pvtnet >= 0) && (args->sender) &&
      ((called_addr.Zone == boss_addr.Zone) || (called_addr.Zone == 0) || (boss_addr.Zone == 0)) &&
      (called_addr.Net == boss_addr.Net) && (called_addr.Node == boss_addr.Node) &&
      ((called_addr.Domain == boss_addr.Domain) || (boss_addr.Domain == NULL) || (called_addr.Domain == NULL)))
  {
    args->Hello.my_net = boss_addr.Net;
    args->Hello.my_node = boss_addr.Node;
    args->Hello.my_point = aka->Node;
  }
  else
  {
    args->Hello.my_net = aka->Net;
    args->Hello.my_node = aka->Node;
    args->Hello.my_point = aka->Point;
  }

  /*
   * If we are the calling system, turn on all our capabilities.
   * If we are the called system, choose from the set that the
   * caller gave us, and only send the one we prefer.
   */

  can_do_domain = (my_addr.Domain != NULL) ? DO_DOMAIN : 0;
  if (args->sender == 0)
  {
    can_do_domain = remote_capabilities & can_do_domain;

    /* we already have only ONE protocol (which is preferred by us) in */
    /* remote_capability, so we don't have to check again with my_cap. */

#ifdef HAVE_HYDRA
    if (remote_capabilities & DOES_HYDRA)
      args->Hello.capabilities = DOES_HYDRA;
    else
#endif
    if (remote_capabilities & DOES_IANUS)
      args->Hello.capabilities = DOES_IANUS;
    else if (remote_capabilities & ZED_ZAPPER)
      args->Hello.capabilities = ZED_ZAPPER;
    else if (remote_capabilities & ZED_ZIPPER)
      args->Hello.capabilities = ZED_ZIPPER;
    else if (remote_capabilities & Y_DIETIFNA)
      args->Hello.capabilities = Y_DIETIFNA;
  }
  else
    args->Hello.capabilities = my_capabilities;

  strntcpy (args->Hello.my_name, system_name, 59);
  args->Hello.capabilities |= can_do_domain;

  if (can_do_domain && (aka->Domain != NULL))
  {
    if (strlen (system_name) + strlen (aka->Domain) > 57)
      args->Hello.my_name[57 - strlen (aka->Domain)] = '\0';

    sptr = args->Hello.my_name + strlen (args->Hello.my_name) + 1;
    strcpy (sptr, aka->Domain);
  }

  if (n_getpassword (&remote_addr) > 0)
    strncpy ((char *) (args->Hello.my_password), remote_password, 8);

  if ((matrix_mask & TAKE_REQ) &&
      ((args->sender == 0)      /* r. hoerner */
       || (QueryNodeFlag (on_our_nickel, FREEREQ, &remote_addr))))
    args->Hello.capabilities |= WZ_FREQ;
  else
    matrix_mask &= ~TAKE_REQ;   /* r. hoerner: response_to_file_requests! */

  remote_pickup = 1;

  /* ------------------------------------------------------------------ */
  /* Send the packet.                                                   */
  /* Load outbound buffer quickly, and get modem busy sending.          */
  /* ------------------------------------------------------------------ */

  SENDBYTE (0x1f);
  sptr = (char *) (&args->Hello);
  SENDCHARS (sptr, 128, 1);

  /* ------------------------------------------------------------------ */
  /* Calculate CRC while modem is sending its buffer                    */
  /* ------------------------------------------------------------------ */

  for (crc = 0, i = 0; i < 128; i++)
    crc = xcrc (crc, (byte) sptr[i]);

  args->crc = crc;
  return (SH3);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SH3 | SendCRC  |                         | Clear Input Buffer      | SH4 |
   |     |          |                         | Send two-byte CRC of pkt|     |
   |     |          |                         | MSB followed by LSB     |     |
   |     |          |                         | Start 40 second timer   |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SHSendCRC (YHARGSP args)
{
  CLEAR_INBOUND ();

  SENDBYTE ((unsigned char) (args->crc >> 8));
  SENDBYTE ((unsigned char) (args->crc & 0xff));

  args->timer1 = timerset (40 * PER_SECOND);
  return (SH4);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SH4 | GetResp  | 40 second timer expires | Failed to send packet   | exit|
   |     |          | or carrier lost         |                         |     |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | ACK received            | Successful transmission | exit|
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | '?' received            | Error, bump retry count | SH5 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | ENQ received            | Out of sync, retry      | SH5 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | other character recvd   | Debris, keep watching   | SH4 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SHGetResp (YHARGSP args)
{
  int i;

  while ((i = TIMED_READ (5)) != ACK)
  {
    if (timeup (args->timer1))
    {
      status_line ("!%s", MSG_TXT (M_TIMEOUT));
      css_upd ();
      return (SHexit);
    }

    if (!CARRIER)
    {
      status_line (MSG_TXT (M_NO_CARRIER));
      return (SHexit);
    }

    if (got_ESC ())
    {
      modem_hangup ();
      status_line (MSG_TXT (M_KBD_MSG));
      return (SHexit);
    }

    switch (i)
    {
    case '?':
      status_line (MSG_TXT (M_DRATS));
      /* Fall through into ENQ case */

    case ENQ:
      return (SH5);

    default:
      break;
    }
  }

  args->result = 1;
  return (SHexit);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | SH5 | BumpCnt  | Retry count >= 10       | Failed to send packet   | exit|
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Count < 10              | Clear inbound buffer    | SH2 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
SHBumpCnt (YHARGSP args)
{
  if (++(args->retries) >= 10)
  {
    status_line (MSG_TXT (M_FUBAR_MSG));
    return (SHexit);
  }

  CLEAR_INBOUND ();
  return (SH2);
}

/* ------------------------------------------------------------------------ */
/* RECEIVE HELLO PACKET                                                     */
/* ------------------------------------------------------------------------ */

int
Recv_Hello_Packet (int Sender)
{
  YHARGS args;
  int res;

  args.result = 0;
  args.sender = Sender;
  res = state_machine ((STATEP) Rcv_Hello, &args, 2);
  return (res);
}

int
RHInit (YHARGSP args)
{
  happy_compiler = args->result;

  return (RH1);
}

/* This routine is called by the state machine when the '??exit'
 * state is seen. Its return value is what the state machine
 * will return to its caller as the result of the function. */

int
RHExit (YHARGSP args)
{
  if (args->result == 0)
    b_init ();
  return (args->result);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH1 | SendENQ  |                         | Start 2 minute timer    | RH2 |
   |     |          |                         | Send an ENQ character   |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */
int
RHSendENQ (YHARGSP args)
{
  do_status (M_STATUS_YOOHOO2U2, NULL);
  SENDBYTE (ENQ);               /* Let the other system know we heard YooHoo */
  args->timer1 = timerset (2 * PER_MINUTE);  /* No more than 2 mins! */
  args->timer2 = 0L;
  args->retries = 0;
  return (RH2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH2 | WaitHedr | 2 minute timer expires  | Report failure          | exit|
   |     |          | or carrier lost         |                         |     |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Received Hex 1f         | Got header, get packet  | RH5 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Received other char     | Debris, throw away      | RH3 |
   |     |          |                         | Start 10 sec timer      |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHWaitHedr (YHARGSP args)
{
  int i;

  while ((i = TIMED_READ (5)) != 0x1f)
  {
    if (!CARRIER)
    {
      status_line (MSG_TXT (M_NO_CARRIER));
      return (RHexit);
    }

    if (got_ESC ())
    {
      status_line (MSG_TXT (M_KBD_MSG));
      return (RHexit);
    }

    if (timeup (args->timer1))
    {
      status_line ("!%s", MSG_TXT (M_TIMEOUT));
      css_upd ();
      return (RHexit);
    }

    if (i != 0xff)
    {
      if (args->timer2 == 0L)
        args->timer2 = timerset (10 * PER_SECOND);
      return (RH3);
    }
  }

  return (RH5);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH3 | TossJunk | 10 sec timer expires    | Too much noise          | RH4 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Received Hex 1f         | Got header, get packet  | RH5 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Input buffer empty      | Try to resynch          | RH4 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Carrier lost            | Report failure          | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHTossJunk (YHARGSP args)
{
  while (!timeup (args->timer2))
  {
    if (got_ESC ())
    {
      status_line (MSG_TXT (M_KBD_MSG));
      return (RHexit);
    }

    if (!CHAR_AVAIL ())
      return (RH4);

    if (TIMED_READ (0) == 0x1f)
      return (RH5);

    if (!CARRIER)
    {
      status_line (MSG_TXT (M_NO_CARRIER));
      return (RHexit);
    }
  }

  return (RH4);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH4 | ReSynch  |                         | Clear input buffer      | RH2 |
   |     |          |                         | Send ENQ                |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHReSynch (YHARGSP args)
{
  CLEAR_INBOUND ();
  SENDBYTE (ENQ);
  args->timer2 = 0L;
  return (RH2);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH5 | HdrSetup |                         | Initialize CRC          |     |
   |     |          |                         | Set 30 second timer     | RH6 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHHdrSetup (YHARGSP args)
{
  memset ((char *) &args->Hello, 0, sizeof (struct _Hello));

  args->crc = 0;
  args->count = 0;
  args->timer3 = timerset (30 * PER_SECOND);
  return (RH6);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH6 | GetHChar | 30 sec timer expires or |                         |     |
   |     |          | carrier lost            | Report failure          | exit|
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | Character received      | Process character       | RH7 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | 10 seconds with no char | Error, try resync       | RH9 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHGetHChar (YHARGSP args)
{
  long localtimer;

  localtimer = timerset (10 * PER_SECOND);

  while (PEEKBYTE () < 0)
  {
    if (timeup (args->timer3))
    {
      status_line ("!%s", MSG_TXT (M_TIMEOUT));
      css_upd ();
      return (RHexit);
    }

    if (timeup (localtimer))
      return (RH9);

    if (got_ESC ())
    {
      status_line (MSG_TXT (M_KBD_MSG));
      return (RHexit);
    }

    if (!CARRIER)
    {
      status_line (MSG_TXT (M_NO_CARRIER));
      return (RHexit);
    }

    time_release ();
  }

  return (RH7);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH7 | StoHChar | Buffer and CRC filled   | Compare CRC             | RH8 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | More characters needed  | Reset 30 sec timer      | RH6 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHStoHChar (YHARGSP args)
{
  int c;
  int i;
  char *sptr = (char *) &(args->Hello);

  while (PEEKBYTE () >= 0)
  {
    c = TIMED_READ (0);
    switch (i = args->count)
    {
    case 128:
      args->hiscrc = c << 8;
      args->count++;
      break;

    case 129:
      args->hiscrc += c;
      return (RH8);

    default:
      sptr[i] = (char) c;
      args->crc = xcrc (args->crc, (byte) c);
      args->count++;
      break;
    }
  }

  args->timer3 = timerset (30 * PER_SECOND);
  return (RH6);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH8 | CheckCRC | CRC matches             | Finish Receive          | RH10|
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | CRC doesn't match       | Handle error            | RH9 |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHCheckCRC (YHARGSP args)
{
  if (args->crc == args->hiscrc)
    return (RH10);

  status_line ("!%s", MSG_TXT (M_CRC_MSG));
  return (RH9);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH9 | CountERR | Less than 10 errors     | Send '?' (0x3f)         | RH2 |
   |     |          +- - - - - - - - - - - - -+- - - - - - - - - - - - -+- - -|
   |     |          | 10 errors               | Hang up, report failure | exit|
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHCountERR (YHARGSP args)
{
  if ((args->retries++) < 10)
  {
    CLEAR_INBOUND ();
    SENDBYTE ('?');
    return (RH2);
  }

  status_line (MSG_TXT (M_FUBAR_MSG));
  return (RHexit);
}

/*
   .-----+----------+-------------------------+-------------------------+-----.
   | RH10| HelloOK  |                         | Clear inbound buffer    | exit|
   |     |          |                         | Send ACK                |     |
   `-----+----------+-------------------------+-------------------------+-----'
 */

int
RHHelloOK (YHARGSP args)
{
  char *p;
  ADDR his_boss;
  int j;
  int logit = TRUE;
  AKA *aka;

  if (alias == NULL)
    return (RHexit);

  for (aka = alias, j = 0; (j < assumed) && (aka != NULL); aka = aka->next, j++) ;

  if (aka == NULL)
    aka = alias;

  /* The idea for removing junk characters came from Holger Schurig */
  /* Get rid of junk characters */
  for (p = args->Hello.my_name; *p != '\0'; p++)
    if (*p < ' ')
      *p = ' ';

  /* Get rid of junk characters */
  for (p = args->Hello.sysop; *p != '\0'; p++)
    if (*p < ' ')
      *p = ' ';

  remote_addr.Zone = args->Hello.my_zone;
  remote_addr.Net = args->Hello.my_net;
  remote_addr.Node = args->Hello.my_node;
  remote_addr.Point = args->Hello.my_point;

  if ((args->Hello.capabilities & DO_DOMAIN) && (my_addr.Domain != NULL))
    remote_addr.Domain = find_domain (&(args->Hello.my_name[strlen (args->Hello.my_name) + 1]));
  else
    remote_addr.Domain = NULL;

  args->Hello.my_name[42] = '\0';
  strcpy (remote_site, args->Hello.my_name);  /* TJW 960528 SRIF data */

  args->Hello.sysop[19] = '\0';
  strcpy (remote_sysop, args->Hello.sysop);  /* TJW 960528 SRIF data */

  remote_capabilities = args->Hello.capabilities;

  /* TJW 960721                                                         */
  /* if I am sender and remote side has chosen ONE protocol, this step  */
  /* is not necessary - but if I am sender and remote side sends back   */
  /* ALL protocols it can do (and did not choose), I have to choose.    */
  /* if I am receiver, I choose now from set that sender gave me,       */
  /* according to my capabilities.                                      */
  /* This is according to FTS-0007 with addition of Hydra as 1st choice. */

#ifdef HAVE_HYDRA
  if (remote_capabilities & my_capabilities & DOES_HYDRA)
    remote_capabilities &= ~(DOES_IANUS | ZED_ZAPPER | ZED_ZIPPER | Y_DIETIFNA);
  else
#endif
  if (remote_capabilities & my_capabilities & DOES_IANUS)
    remote_capabilities &= ~(DOES_HYDRA | ZED_ZAPPER | ZED_ZIPPER | Y_DIETIFNA);
  else if (remote_capabilities & my_capabilities & ZED_ZAPPER)
    remote_capabilities &= ~(DOES_HYDRA | DOES_IANUS | ZED_ZIPPER | Y_DIETIFNA);
  else if (remote_capabilities & my_capabilities & ZED_ZIPPER)
    remote_capabilities &= ~(DOES_HYDRA | DOES_IANUS | ZED_ZAPPER | Y_DIETIFNA);
  else if (remote_capabilities & my_capabilities & Y_DIETIFNA)
    remote_capabilities &= ~(DOES_HYDRA | DOES_IANUS | ZED_ZAPPER | ZED_ZIPPER);

  his_boss = remote_addr;
  his_boss.Point = 0;

  if (nodefind (&his_boss, 0) && !remote_addr.Zone)
    remote_addr.Zone = found_zone;

  status_line ("*%s (%s)", args->Hello.my_name, Full_Addr_Str (&remote_addr));

  if ((pvtnet >= 0) &&
      ((remote_addr.Zone == aka->Zone) || (remote_addr.Zone == 0)) &&
      (remote_addr.Net == boss_addr.Net) && (remote_addr.Node == boss_addr.Node) &&
      (remote_addr.Point > 0))
  {
    remote_addr.Net = pvtnet;
    remote_addr.Node = args->Hello.my_point;
    remote_addr.Point = 0;
  }

  log_product (args->Hello.product, args->Hello.product_maj, args->Hello.product_min);
  /* TJW 960528 above call sets also SRIF remote_mailer */

  if (args->Hello.sysop[0])
    status_line (":Sysop : %s", args->Hello.sysop);

  if ((pvtnet >= 0) && (remote_addr.Point > 0))
  {
    remote_addr.Point = 0;
    remote_addr.Node = (unsigned short) -1;
  }

  if ((j = n_getpassword (&remote_addr)) > 0)
  {
    if ((j = n_password (&remote_addr, (char *) args->Hello.my_password, remote_password, &logit)) != 0)
    {
      if ((j == 1) || ((j == 2) && (args->sender == 0)))
        goto blew_it;
      else
        status_line (MSG_TXT (M_PASSWORD_OVERRIDE));
    }

    strcpy (password, remote_password);  /* TJW 960528 SRIF */
  }
  else if (j < 0)
  {
    status_line (MSG_TXT (M_NUISANCE_CALLER));

  blew_it:

    modem_hangup ();            /* He'll never get it right */
    return (RHexit);
  }

  CLEAR_INBOUND ();
  SENDBYTE (ACK);

  args->result = 1;
  return (RHexit);
}

/* $Id: yoohoo.c,v 1.5 1999/03/23 22:40:13 mr Exp $ */
