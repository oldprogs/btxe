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
 * Filename    : $Source: E:/cvs/btxe/src/emsi.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:25 $
 * State       : $State: Exp $
 * Orig. Author: Tobias Burchhardt
 *
 * Description : BinkleyTerm EMSI-Handshake Processor
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#define EMSI_DATALOSS_CHECKER   /* this shouldn't do any harm */

#undef  EMSI_INQ_RETRANSMIT     /* this has yet to be tested ... */
#undef  EMSI_DAT_RETRANSMIT     /* this has yet to be tested ... */
#undef  EMSI_GETFAST            /* this has yet to be tested ... */
#undef  EMSI_SMFIX              /* this has yet to be tested ... */

static int LOCALFUNC Send_EMSI_Packet (int);
static int LOCALFUNC Recv_EMSI_Packet (int);
static int LOCALFUNC get_emsi (char *);
static char *LOCALFUNC b7strcat (char *, char *);
static int LOCALFUNC recode_from_b7 (char *, int);
static AKA *LOCALFUNC match_aka (ADDRP);
static char *LOCALFUNC get_bracket (char *);
static char *LOCALFUNC cat_bracket (char *, char *);
static int LOCALFUNC get_level (ADDRP addr, ADDRP myaka);
static int LOCALFUNC Encode_EMSI_Header (char *, int, char **, long);
static int LOCALFUNC Decode_EMSI_Header (char *, int, int);
static int highest_level, hl_aka;  /* Used for passwording */
static void LOCALFUNC CheckSmartBiDi (int Sender,
                                      long *noHydra,
                                      long *noJanus,
                                      long noPickup);
static long LOCALFUNC GetTZUTC (char *);

const char PASSWORD_FAILURE[] = "==! password failure !==";
const char CHECK_SETUP[] = "==! check your setup !==";

static int wrong_password, nrTries;

/*--------------------------------------------------------------------------*/
/* EMSI SENDER    (used when I am the CALLING system)                       */
/*--------------------------------------------------------------------------*/

int
EMSI_Sender (void)
{
  char *sptr;
  int result;

  nrTries = 0;
  wrong_password = 0;
  highest_level = num_rakas = 0;
  hl_aka = -1;
  sptr = NULL;

  if (debug_emsi)
    log_emsi ('C', NULL, 0L);

  while (nrTries < 2)
  {
    /* -------------------------------------------------------------------- */
    /* Clean up any mess that may be around                                 */
    /* -------------------------------------------------------------------- */
    CLEAR_INBOUND ();
    XON_DISABLE ();

    if (!Send_EMSI_Packet (1))
    {
      sptr = MSG_TXT (M_HE_HUNG_UP);
      status_line (sptr);
      return (0);
    }

    if (wrong_password)         /* if second run  */
      break;                    /* hangup via DTR */

    result = Recv_EMSI_Packet (1);

    if (result)
      return (result);          /* return SUCCESS */

    nrTries++;
  }

  // TJW970518 why should we hangup here ????
  //  LOWER_DTR ();
  //  timer (2);

  return (0);                   /* this is: return ERROR */
}

/*--------------------------------------------------------------------------*/
/* EMSI RECEIVER  (Used when I am the CALLED system)                        */
/*--------------------------------------------------------------------------*/

int
EMSI_Receiver (void)
{
  int result;

  nrTries = 0;
  wrong_password = 0;
  highest_level = num_rakas = 0;
  hl_aka = -1;

  if (debug_emsi)
    log_emsi ('A', NULL, 0L);

  /* -------------------------------------------------------------------- */
  /* Clean up any mess that may be around                                 */
  /* -------------------------------------------------------------------- */

  CLEAR_OUTBOUND ();
  XON_DISABLE ();

  if (Recv_EMSI_Packet (0) == 0)
    return (0);                 /* return ERROR  */

  result = Send_EMSI_Packet (0);

  if (result == 0)              /* if ERROR      */
    b_init ();

  // TJW970518: we should NOT hangup if we are the called system and only
  //            a single AKA of caller has a PW failure, but there are
  //            other AKAs left with no PW failure
  //
  //    if (wrong_password)
  //    {
  //      timer (2);
  //      LOWER_DTR ();
  //      timer (2);
  //    }

  return (result);              /* return SUCCESS or ERROR */
}                               /* EMSI Receiver */

/*--------------------------------------------------------------------------*/
/* Disable Janus and Hydra in case of upcoming one-way transfer             */
/*--------------------------------------------------------------------------*/

void LOCALFUNC
CheckSmartBiDi (int Sender, long *noHydra, long *noJanus, long noPickup)
{
  /* --------------------------------------------------------------------- */
  /* calculate loc_tot_moh for remote's AKAs (I am not Sender) or          */
  /* for called address (I am Sender and don't know all remote AKAs        */
  /* --------------------------------------------------------------------- */
  long tot_moh = 0L;

  if (Sender)
    tot_moh = totalsize (&remote_addr, 0);
  else
  {
    int i;

    for (i = 0; i < num_rakas; i++)
      tot_moh += totalsize (&remote_akas[i], 0);
  }

  /* ---------------------------------------------------------------------
   * R.hoerner 2.2.97:
   * switch off bidi-protocols in case we can savely assume an upcoming
   * one-way xfer. ZModem will be the fastest protocol in that case.
   *
   * !Sender:    we know all addresses and know we have 0 byte for "him"
   * smart_bidi: we are the sender and are configured to only take into
   *             account what we have for the called address.
   * noPickup:   we'll deny reception
   * TJW 970216: this is no good if you want to chat using hydra-chat, so
   *             I modified behaviour to be able to completely switch off
   *             this feature
   * ---------------------------------------------------------------------*/

  if (tot_moh == 0L && smart_bidi && (!Sender || smart_bidi_out || noPickup))
  {
    status_line (MSG_TXT (M_DISABLING_BIDI));  /* VRP 990909 */
    *noJanus = 1L;
    *noHydra = 1L;
  }

  TxStats.tot_moh = tot_moh;
}

/*--------------------------------------------------------------------------*/
/* SEND EMSI PACKET                                                         */
/*--------------------------------------------------------------------------*/

static int LOCALFUNC
Send_EMSI_Packet (int Sender)
{
  int i;
  char *sptr = NULL;
  long T1, T2;
  word crc, len;
  word tries = 0;
  char *junkbuff, *uni;
  char *emsi_output;

  /* Txbuf is (8192+16) in length
   * junkbuff wants to be 4096
   * uni wants to be 256
   * We reserve the last 1024+16 for lower level functions
   *
   * If you change this way of doing things, you have to change
   * Encode_EMSI_Header as well!!! */

  junkbuff = bufp->emsi._junkbuff;  /* TJW 960416 */
  uni = bufp->emsi._uni;        /* TJW 960416 */

  /* -------------------------------------------------------------------- */
  /* Setup EMSI structure                                                 */
  /* -------------------------------------------------------------------- */
  emsi_output = NULL;
  len = Encode_EMSI_Header (junkbuff, Sender, &emsi_output, (long) sizeof (junkbuff));

  if (emsi_output != NULL)
    junkbuff = emsi_output;

  if (len)                      /* r. hoerner */
    if (debug_emsi)
      log_emsi ('S', junkbuff, len);

  /* -------------------------------------------------------------------- */
  /* Disable handshaking and ^C/^K handling                               */
  /* -------------------------------------------------------------------- */
  XON_DISABLE ();

  /* -------------------------------------------------------------------- */
  /* Send the packet.                                                     */
  /* Load outbound buffer quickly, and get modem busy sending.            */
  /* -------------------------------------------------------------------- */

  do_status (M_STATUS_EMSISEND, NULL);  // was: M_EMXMIT
  // or:  set_xy (MSG_TXT (M_EMSEND));

  /* -------------------------------------------------------------------- */
  /* Send the packet.                                                     */
  /* Load outbound buffer quickly, and get modem busy sending.            */
  /* -------------------------------------------------------------------- */

  tries = 0;
  T1 = timerset (PER_MINUTE);

xmit_packet:

  if (tries++ > 6)
  {
    sprintf (uni, "!%s", MSG_TXT (M_TIMEOUT));
    css_upd ();
    sptr = uni;
    goto no_response;
  }

  sprintf (uni, "%s%04X", &(emsistr[EMSI_DAT][2]), len);
  crc = crc_block ((unsigned char *) uni, strlen (uni));
  SENDCHARS ("**", 2, 1);
  SENDCHARS (uni, strlen (uni), 1);
  SENDCHARS (junkbuff, len, 1);

  /* -------------------------------------------------------------------- */
  /* Calculate CRC while modem is sending its buffer                      */
  /* -------------------------------------------------------------------- */
  for (i = 0; i < (int) len; i++)
    crc = xcrc (crc, junkbuff[i]);

  CLEAR_INBOUND ();
  ///////////////// TJW979724 does this make sense ???

  sprintf (uni, "%04X\r", crc);
  SENDCHARS (uni, strlen (uni), 1);

  /* wait_response: */

  T2 = timerset (20 * PER_SECOND);

  while (!timeup (T1) && CARRIER)
  {
    if (timeup (T2))
      goto xmit_packet;

    if (!CHAR_AVAIL ())
    {
      if (got_ESC ())
      {
        modem_hangup ();
        sptr = MSG_TXT (M_KBD_MSG);
        goto no_response;
      }
      time_release ();
      continue;
    }

    i = TIMED_READ (0);
    switch (i)
    {
    case '*':
      strcpy (uni, "*");

      if (!get_emsi (uni))
        break;

      if (!stricmp (uni, emsistr[EMSI_REQ]))
      {
#ifdef EMSI_INQ_RETRANSMIT
        /* TJW970305 added re-transmitting EMSI_INQ if other side  */
        /* maybe didn't get it correctly the first time and didn't */
        /* even enter EMSI handshake yet ...                       */
        sprintf (uni, "%s\r", emsistr[EMSI_INQ]);
        SENDCHARS (uni, strlen (uni), 1);  /* Kick the other guy. */
        status_line (">EMSI: EMSI_INQ retransmit");
        /* end of TJW970305 changes                                */
#endif

#ifdef EMSI_DAT_RETRANSMIT
        /* TJW 970309 experimental change: retransmit EMSI_DAT */
        goto xmit_packet;
#else
        /* original code: */
        continue;
#endif
      }

      if (!stricmp (uni, emsistr[EMSI_ACK]))
      {                         /* AW 980521 fix memory leak   */
        if (emsi_output)        /* we need to free the buffer! */
          free (emsi_output);
        return (1);
      }

      //////////////////// TJW970724
      // this is a problem in FSC-0056 (!!!) :
      // if an unknown EMSI sequence is received, tries will be
      // incremented and there will be a "timeout" if tries goes
      // beyond 6 (after xmit_packet). This may happen before any
      // valid EMSI_DAT sequence is received ...
      // So until FSC0056 is reviewed, we just ignore unknown EMSI
      // sequences by doing a "break" instead of "goto xmit_packet".
      // goto xmit_packet; // original FSC-0056 compliant code
      break;                    // TJW970724 fixes Xenia EMSI_MD5 "timeout" problem

    default:
      if (i <= 0)               /* MB *//* Could just be line noise if > 0 */
        time_release ();        /*PLF Sun  12-01-1991  04:21:14 */
      break;
    }
  }

  if (!CARRIER)
    sptr = MSG_TXT (M_NO_CARRIER);
  else
  {
    sprintf (uni, "!EMSI-Send: %s", MSG_TXT (M_TIMEOUT));
    sptr = uni;
  }

no_response:

  if (emsi_output)
    free (emsi_output);

  if (sptr != NULL)
    status_line (sptr);

  return (0);
}                               /* Send EMSI */

/*--------------------------------------------------------------------------*/
/* RECEIVE EMSI PACKET                                                      */
/*--------------------------------------------------------------------------*/

static int LOCALFUNC
Recv_EMSI_Packet (int Sender)
{
  int i;
  int c;
  int len;
  char *sptr;
  byte tries = 0;
  word crc, crc2;
  long T1, T2, T3;
  char *junkbuff, *uni, *help;
  char *emsi_input = NULL;
  int state = 0;
  NODESTRUC *np;

  /* Txbuf is (8192+16) in length
   * junkbuff wants to be 4096
   * uni wants to be 256
   * help wants to be 256
   * tmpstr wants to be 256
   * We reserve the last 1024+16 for lower level functions
   *
   * If you change this way of doing things, you have to change
   * Decode_EMSI_Header as well!!! */

  junkbuff = bufp->emsi._junkbuff;  /* TJW 960416 */
  uni = bufp->emsi._uni;        /* TJW 960416 */
  help = bufp->emsi._help;      /* TJW 960416 */
  /* tmpstr = &help[256]; */

  /* -------------------------------------------------------------------- */
  /* Display a message                                                    */
  /* -------------------------------------------------------------------- */

  do_status (M_STATUS_EMSIWAIT, NULL);  // was:  emsi_display(M_EMWAIT);
  // or:   set_xy (MSG_TXT (M_EMRECV));


  /* -------------------------------------------------------------------- */
  /* Get the EMSI structure                                               */
  /* -------------------------------------------------------------------- */

  remote_password = NULL;
  remote_pickup = 2;            /* Pickup for all akas */

  T1 = timerset (20 * PER_SECOND);
  T2 = timerset (PER_MINUTE);
  sptr = NULL;

request_it:

  if (tries++ > 6)
    goto timeout;

  if (sptr)
  {
    status_line (sptr);
    sptr = NULL;
  }

  status_line (">EMSI: request_it - try %d", tries);

  if (!Sender)
    sprintf (uni, "%s\r", emsistr[EMSI_REQ]);  /* EMSI_REQ if he called me  */
  else if (tries > 1)
    sprintf (uni, "%s\r", emsistr[EMSI_NAK]);  /* EMSI_NAK anytime after 1st */
  else
    goto watch_for_header;

  SENDCHARS (uni, strlen (uni), 1);  /* Kick the other guy.       */

new_timer:

  T1 = timerset (20 * PER_SECOND);

watch_for_header:

  while (!timeup (T2) && CARRIER)
  {
    if (timeup (T1))
      goto request_it;

    if (!CHAR_AVAIL ())
    {
      if (got_ESC ())
      {
        modem_hangup ();
        sptr = MSG_TXT (M_KBD_MSG);
        goto receive_failed;
      }

      time_release ();
      continue;
    }

    i = TIMED_READ (0);
    switch (i)
    {
    case '*':
      strcpy (uni, "*");
      if (!get_emsi (uni))
        break;
      if (!stricmp (uni, emsistr[EMSI_HBT]))
        goto new_timer;
      if (!stricmp (uni, emsistr[EMSI_DAT]))
        goto receive_packet;
      break;

    default:
      if (i <= 0)               /* MB *//* Could just be line noise if > 0 */
        time_release ();        /*PLF Sun  12-01-1991  04:21:14 */
      break;
    }
  }

  if (timeup (T2))
    goto timeout;

  if (!CARRIER)
  {
    sptr = MSG_TXT (M_NO_CARRIER);
    goto receive_failed;
  }

receive_packet:

  status_line (">EMSI: receive packet");

  do_status (M_STATUS_EMSIRECV, NULL);  //was: emsi_display(M_EMRCPT);

  T3 = timerset (PER_MINUTE);
  if (((int) (uni[0] = (char) TIMED_READ (3)) < 0) ||
      ((int) (uni[1] = (char) TIMED_READ (3)) < 0) ||
      ((int) (uni[2] = (char) TIMED_READ (3)) < 0) ||
      ((int) (uni[3] = (char) TIMED_READ (3)) < 0))
  {
    sptr = MSG_TXT (M_EMSHORT);
    goto emsi_error;
  }

  if (!CARRIER)
  {
    sptr = MSG_TXT (M_NO_CARRIER);
    goto receive_failed;
  }

  uni[4] = '\0';
  sscanf (uni, "%x", &len);

  sprintf (help, "%s%s", &(emsistr[EMSI_DAT][2]), uni);
  crc = crc_block ((unsigned char *) help, strlen (help));

  memset (junkbuff, 0, 4096);

  if (len > 4096)               /* then we could get a problem.. (r. hoerner) */
  {                             /* 'cause our input buffer is only 4096 byte! */
    emsi_input = calloc (1, len);  /* get another buffer to receive EMSI */
    if (emsi_input != NULL)     /* if this works: go on with EMSI    */
      junkbuff = emsi_input;
    else
    {
      sptr = MSG_TXT (M_MEM_ERROR);  /* else abort: useless to try again! */
      goto receive_failed;
    }
  }

  sptr = junkbuff;

  for (i = 0; i < len; i++)
  {
    while (PEEKBYTE () < 0)
    {
      if (timeup (T2) || timeup (T3))
        goto timeout;

      if (got_ESC ())
      {
        sptr = MSG_TXT (M_KBD_MSG);
        goto receive_failed;
      }

      if (!CARRIER)
      {
        sptr = MSG_TXT (M_NO_CARRIER);
        goto receive_failed;
      }

      time_release ();
    }

    c = TIMED_READ (0);

#ifdef EMSI_DATALOSS_CHECKER
    /* TJW 970305: data loss checker */
    if (c == '}' && i > ((len * 7) >> 3))
      state = 1;                /* we maybe have read last } of EMSI now */
    else if (c == '{')          /* no. it wasn't the last one - reset! */
      state = 0;
    else if (state >= 1)
    {
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
        state++;                /* we have read one more hex digit (EMSI CRC!?) */
      else
        state = 0;
      if (state >= 2)
      {
        /* we have read }X... (X=hex digit) - seems like */
        /* we had lost some incoming data and therefore  */
        /* read too much ahead!                          */
        status_line ("!EMSI: data loss, end detected: %d, len expected: %d!",
                     i, len);
        sptr = MSG_TXT (M_EMSHORT);
        goto emsi_error;
      }
    }
    /* end data loss checker */
#endif

    *sptr++ = (char) c;
    crc = xcrc (crc, (byte) c);
  }

  if (((int) (uni[0] = (char) TIMED_READ (3)) < 0) ||
      ((int) (uni[1] = (char) TIMED_READ (3)) < 0) ||
      ((int) (uni[2] = (char) TIMED_READ (3)) < 0) ||
      ((int) (uni[3] = (char) TIMED_READ (3)) < 0))
  {
    sptr = MSG_TXT (M_EMSHORT);
    goto emsi_error;
  }

  TIMED_READ (1);               /* What am I throwing away here? */

  if (!CARRIER)
  {
    sptr = MSG_TXT (M_NO_CARRIER);
    goto receive_failed;
  }

  uni[4] = '\0';
  sscanf (uni, "%hx", &crc2);

  do_status (M_STATUS_EMSIRCVD, NULL);

  if (crc == crc2)
    goto process_packet;

  sprintf (junkbuff, "!EMSI-Recv: %s", MSG_TXT (M_CRC_MSG));
  sptr = junkbuff;

emsi_error:

  if (timeup (T2))
    goto timeout;

  CLEAR_INBOUND ();
  goto request_it;

process_packet:

  if (strnicmp (junkbuff, "{EMSI}", 6))
  {
    sptr = MSG_TXT (M_EMSTART);
    goto emsi_error;
  }

  if (debug_emsi)               /* r. hoerner */
    log_emsi ('R', junkbuff, len);

  if (!Decode_EMSI_Header (junkbuff, len, Sender))  /*0: "nuisance caller" */
    goto receive_failed2;       /* TJW 960615 bugfix  */

  if (hl_aka >= 0)
    n_getpassword (&remote_akas[hl_aka]);
  /* Updated 'assumed', 'CURRENT' to match remote */

  /* Extra Inbound Dir */
  if ((np = QueryNodeStruct (&remote_akas[hl_aka], 0)) != NULL)
    if (np->ExtraDirIn)
    {
      CURRENT = SPECIAL;
      strcpy (CURRENT.sc_Inbound, np->ExtraDirIn);
    }

  CLEAR_INBOUND ();
  CLEAR_OUTBOUND ();

  sprintf (uni, "%s\r", emsistr[EMSI_ACK]);
  SENDCHARS (uni, strlen (uni), 1);  /* Let the other system know */
  SENDCHARS (uni, strlen (uni), 1);  /* we got it !               */
  T3 = timerset (10 * PER_SECOND);

  while (!OUT_EMPTY ())         /* Flush the buffer          */
  {
    if (timeup (T3))
      goto timeout;

    if (!CARRIER)
    {
      sptr = MSG_TXT (M_NO_CARRIER);  /* TJW 960615 bugfix */
      goto receive_failed;
    }

    time_release ();
  }

  if (debugging_log)
    show_our_capabilities ("EMSI ");

  if (emsi_input)
    free (emsi_input);

  return (1);

timeout:

  sprintf (uni, "!EMSI: %s", MSG_TXT (M_TIMEOUT));
  sptr = uni;

receive_failed:

  if (sptr)
    status_line (sptr);

receive_failed2:

  b_init ();
  if (emsi_input)
    free (emsi_input);
  return (0);
}                               /* Recv EMSI */

static int LOCALFUNC
get_emsi (char *s)
{
  long T1;
  char *sptr, c;
  int i;

  status_line (">EMSI: get_emsi");

#ifdef EMSI_GETFAST
  T1 = timerset (2 * PER_SECOND);  /* SM 970402 */
#else
  T1 = timerset (5 * PER_SECOND);  /* original code */
#endif

  sptr = s + 1;
  memset (sptr, 0, EMSI_LEN + 1);
  while (CARRIER)
  {
    while (PEEKBYTE () < 0)
    {
      if (timeup (T1))
        goto timeout;

      if (got_ESC ())
      {
        status_line (MSG_TXT (M_KBD_MSG));
        goto receive_failed;
      }

      if (!CARRIER)
      {
        status_line (MSG_TXT (M_NO_CARRIER));
        goto receive_failed;
      }

      time_release ();
    }

    c = (char) TIMED_READ (0);
    if (c == '\r')
    {
      status_line (" EMSI: '%s'", s);
      return (1);
    }

    *sptr++ = (char) c;

    for (i = 0; i < EMSI_MAX; i++)
    {
      if (!strnicmp (s, emsistr[i], strlen (emsistr[i])))
      {
        status_line (">EMSI: '%s'", s);
        return (1);
      }
    }

    if ((int) strlen (s) > EMSI_LEN)
    {
#ifdef EMSI_SMFIX
      /* TJW: does this code make sense and why ??? : */
      memmove (s, s + 1, EMSI_LEN - 1);
      /* wouldn't be strcpy(s,s+1) be better, \0 see init ... ??? */
      sptr--;
#else
      status_line (" EMSI: '%s'", s);
      return (1);
#endif
    }
  }
timeout:

  status_line ("!%s", MSG_TXT (M_TIMEOUT));
  css_upd ();

receive_failed:

  return (0);
}

static char *LOCALFUNC
b7strcat (char *d, char *s)
{
  int i;
  int j = strlen (s);
  char uni[4], *p, *c;

  p = &d[strlen (d)];
  c = s;
  for (i = 0; i < j; c++, i++)
  {
    if (*c == '\\')
    {
      *p++ = '\\';
      *p++ = '\\';
      continue;
    }

    /*old if((*c>=0)&&(*c<=0x7F)) */

    if (!(*c & 0x80))
    {
      *p++ = *c;
      continue;
    }

    *p++ = '\\';                /* Do coding only for 8 bit chars */
    sprintf (uni, "%02X", (unsigned char) *c);
    *p++ = uni[0];
    *p++ = uni[1];
    continue;
  }

  *p++ = '\0';
  return (d);
}

static int LOCALFUNC
recode_from_b7 (char *s, int len)
{
  int i, j;
  char *d, uni[4];
  unsigned short int c;

  d = s;
  for (j = i = 0; i < len; j++)
  {
    if (*s != '\\')
    {
      *d++ = *s++;
      i++;
      continue;
    }

    if (*++s == '\\')
    {
      *d++ = *s++;
      i += 2;
      continue;
    }

    uni[0] = *s++;
    uni[1] = *s++;
    uni[2] = '\0';
    sscanf (uni, "%hx", &c);
    *d++ = (char) c;
    i += 3;
  }

  *d++ = '\0';
  return (j);                   /* New length */
}

static int LOCALFUNC
get_level (ADDRP addr, ADDRP myaka)
{
  int level = -1;

  if ((addr->Domain == myaka->Domain) || !addr->Domain || !myaka->Domain)
    level++;
  else
    return (level);

  if (addr->Zone == myaka->Zone)
    level++;
  else
    return (level);

  if (addr->Net == myaka->Net)
    level++;
  else
    return (level);

  if (addr->Node == myaka->Node)
    level++;
  else
    return (level);

  /* TJW 960908 point comparison was missing -> bug in EMSI dupe check */

  if (addr->Point == myaka->Point)
    level++;

  return (level);
}

static AKA *LOCALFUNC
match_aka (ADDRP addr)
{
  int level = -1;
  int i, j;
  AKA *aka, *assumed_aka;

  assumed = 0;
  assumed_aka = NULL;

  for (i = 0, aka = alias; aka != NULL; i++, aka = aka->next)
  {
    j = get_level (addr, (ADDR *) aka);
    if (j > level)
    {
      assumed = i;
      level = j;
      assumed_aka = aka;
    }
  }

  if (assumed_aka == NULL)
    assumed_aka = alias;

  return (assumed_aka);
}

/*--------------------------------------------------------------------------*/
/* Find end of EMSI field                                                   */
/*--------------------------------------------------------------------------*/

static char *LOCALFUNC
get_bracket (char *p)
{
  register char c;
  register char x = *(p - 1);   /* Save starting character */
  char *q = p;

  /* Convert starting bracket to expected ending bracket */
  if (x == '{')
    x = '}';
  else if (x == '[')
    x = ']';

  while ((c = *p++) != '\0')
  {
    /* If this is an ending bracket */
    if (c == '}' || c == ']')
    {
      /* And it's not escaped */
      if (*p != c)
      {
        /* And it's the right one, get out. */
        if (c == x)
          break;
      }
      else
      {
        /* If it is escaped, copy the end of the string over the esc */
        strcpy (q, p);
      }
    }

    q = p;
  }

  return (q);
}

static char *LOCALFUNC
cat_bracket (char *d, char *c)  /* MB 93-11-27 */
{
  char x, *p, *q;

  q = p = bufp->emsi._pq;       /* TJW 960416 */

  while ((x = *c++) != '\0')
  {
    if (*c && (x == '}' || x == ']'))
      *p++ = x;                 /* We have to escape } or ] by doing it twice */
    *p++ = x;
  }
  *p++ = '\0';
  return (b7strcat (d, q));
}

/* TJW 960720 made EMSI FSC-0056 compliant: caller chooses protocol preference
 * not called mailer as it was implemented in Binkley until now. */

static int LOCALFUNC
Decode_EMSI_Header (char *junkbuff, int len, int Sender)
{
  static char *addons[] =
  {"{IDENT}", "{TRX#}", "{MOH#}", "{TZUTC}", NULL};
  int prodcodeT;
  int j;
  int addonnr;
  int i = 0;
  int cur = 0;
  int akacnt = 0;
  long do_timesync = 0;         /* TJW 960701 for emsi timesync     */
  int called_aka_presented = 0;
  int logit = TRUE;
  int search_protocol = TRUE;   /* TJW 960720 for callers 1. choice */
  char *sptr;
  char *p;
  char *name;
  char *loc;
  char *uni = bufp->emsi._uni;  /* TJW 960416 Look at   */
  char *help = bufp->emsi._help;  /*  Recv_EMSI_Packet to */
  char *tmpstr = bufp->emsi._tmpstr;  /*  see why this is OK  */
  char test;                    /* MMP 960419 - used when parsing TRX */
  ADDR addr[ALIAS_CNT];
  ADDR tmp;
  time_t his_trx = (time_t) 0;
  long diff_trx2 = 0;
  long remoteTZUTC = -1;

  diff_trx = (time_t) 0;
  direct_zap = 0;

  recode_from_b7 (junkbuff, len);
  sptr = &junkbuff[7];

  memset (&remote_addr, 0, sizeof (remote_addr));
  memset (&addr, 0, sizeof (addr));

  if (Sender)
    addr[i++] = called_addr;
  else
    called_aka_presented = 1;   // MR 970506 added

  while (*sptr != '}')
  {
    while (*sptr == ' ')
      sptr++;

    /* Copy the address string into temp string. */
    p = tmpstr;
    while (*sptr && *sptr != '}' && *sptr != ' ')
      *p++ = *sptr++;

    *p = '\0';
    if (p == tmpstr)
      break;

    find_address (tmpstr, &tmp);
    status_line (">EMSI: parsed %s and got addr[%d]: '%s'", tmpstr, i, Full_Addr_Str (&tmp));
    status_line (">EMSI: checking addr '%s' for dupe", Full_Addr_Str (&tmp));

    for (j = 0; j < i; j++)
    {
      if (SameAddress (&addr[j], &tmp))
      {
        if (j == 0 && Sender)
        {
          addr[0] = tmp;
          called_aka_presented = 1;
        }

        status_line (">EMSI: dupe AKA removed: '%s'", Full_Addr_Str (&tmp));
        break;
      }
    }

    if (j == i)
    {
      status_line (">EMSI: using addr: '%s'", Full_Addr_Str (&tmp));
      addr[i] = tmp;
    }
    else
      continue;

    if (i < (ALIAS_CNT - 1))
      i++;
    else
    {
      status_line (MSG_TXT (M_ADDRS_SKIPPED),
                   addr[i].Zone, addr[i].Net, addr[i].Node,
                   addr[i].Point, addr[i].Domain);
      break;
    }
  }

  akacnt = i;
  // status_line (">EMSI: akacnt = %d", akacnt);

  while (*sptr++ != '{') ;

  for (i = 0; *sptr != '}'; sptr++)
    if (i < 16)
      password[i++] = *sptr;

  password[i] = '\0';

  sptr += 2;                    /* skip "}{" */

  while (*sptr != '{')          /* until begin of next field */
  {
    i = 0;
    while ((*sptr != ',') && (*sptr != '}'))
      uni[i++] = *sptr++;

    uni[i] = '\0';
    sptr++;                     /* skip ',' or '}' */

    if (!stricmp (uni, "NPU"))
      remote_pickup = 0;
    else if (!stricmp (uni, "PUP"))
      remote_pickup = 1;
    else if (!stricmp (uni, "PUA"))
      remote_pickup = 2;
#ifdef HAVE_HYDRA
    else if (!stricmp (uni, "RH1"))
      do_RH1hydra = 1;
#endif
  }

  CURRENT = DEFAULT;
  for (i = 0; i < akacnt; i++)
  {
    tmp = addr[i];

    status_line (">EMSI: checking addr: '%s'", Full_Addr_Str (&tmp));
    if ((j = n_getpassword (&tmp)) > 0)
    {
      // TJW970518: sorry, but the DBridge fix is not possible any more as the
      //            product-code is not known yet - but the password processing
      //            MUST be done here ...
      //
      // if (Sender && *remote_password && !*password && (prodcodeT == isDBRIDGE))
      //   j = 0;          /* work-around for a D'Bridge(tm)-Bug */
      // else
      j = n_password (&tmp, password, remote_password, &logit);

      status_line (">EMSI: result of pw check: '%d'", j);

      if (j == 0)
      {
        if (highest_level < 2)
        {
          highest_level = 2;
          hl_aka = num_rakas;
        }
      }
      else
      {
        /* Password doesn't match.
         *  - Result of 1 means both of us had one.
         *  - Result of 2 means we had one and the other side didn't.
         *
         * If we are the sender we will accept this mismatch,
         * but only on the first aka. */

        if ((!Sender) || (num_rakas != 0))
        {
          wrong_password = 1;
          continue;
        }
        else
          status_line (MSG_TXT (M_PASSWORD_OVERRIDE));

        if (highest_level < 1)
        {
          highest_level = 1;
          hl_aka = num_rakas;
        }
      }
    }
    else if (j < 0)
    {
      status_line (">EMSI: addr '%s' will be curmudgeoned", Full_Addr_Str (&tmp));
      cur = 1;
      wrong_password = 1;
      continue;
    }
    else
    {                           /* j == 0 */
      if (highest_level < 1)
      {
        highest_level = 1;
        hl_aka = num_rakas;
      }
    }

    do_timesync = QueryNodeFlag (do_timesync, TIMESYNC, &tmp);
    remote_akas[num_rakas++] = tmp;
  }

  // !!!!!! here we must ensure that remote_addr has no PW failure !!!

  if (called_aka_presented)     /* always true if we are _not_ sender */
  {
    if (num_rakas > 0)
      remote_addr = remote_akas[0];
    else;
    // remote_addr will stay 0:0/0.0 in this
    // case - this is VERY IMPORTANT !!!
  }
  else
  {
    /* only happens, if called system did not present the AKA we called */
    if (num_rakas > 1)          /* choose it's main aka, if no PW failure  */
      remote_addr = remote_akas[1];
    else if (num_rakas > 0)     /* choose the called aka, if no PW failure */
      remote_addr = remote_akas[0];
    else;
    // remote_addr will stay 0:0/0.0 in this
    // case - this is VERY IMPORTANT !!!
  }

  // status_line(" Highest_Level = %d", highest_level);
  // status_line(" HL-Aka = %d", hl_aka);

  /* ------------------------------------------------------------------
   * Now we know the remote's addresses. Either we CALLED, then we know
   * the allowed protocols, or we are ANSWERING, then we have to set
   * our flags NOW. (r. hoerner)
   * ----------------------------------------------------------------- */

  if (!Sender)
  {
    long noPickup, noJanus, noHydra, noZip;

    noJanus = QueryNodeFlag (no_janus, NOJANUS, &remote_addr);
    noHydra = QueryNodeFlag (no_hydra, NOHYDRA, &remote_addr);
    noZip = QueryNodeFlag (no_zapzed, NOZED, &remote_addr);
    noPickup = QueryNodeFlag (no_pickup, NOPICK, &remote_addr);

    CheckSmartBiDi (Sender, &noHydra, &noJanus, noPickup);

    if (noJanus)
      my_capabilities &= ~DOES_IANUS;
#ifdef HAVE_HYDRA
    if (noHydra)
      my_capabilities &= ~DOES_HYDRA;
#endif
    if (noZip)
      my_capabilities &= ~ZED_ZAPPER;

    my_capabilities |= ZED_ZIPPER;
  }

  /* ------------------------------------------------------------------
   * that's it.
   * ----------------------------------------------------------------- */

  sptr++;                       /* skip '{' */
  remote_capabilities = WZ_FREQ;

  while (*sptr != '}')
  {
    i = 0;
    while ((*sptr != ',') && (*sptr != '}'))
      uni[i++] = *sptr++;

    uni[i] = '\0';
    if (*sptr != '}')
      sptr++;

    if (search_protocol)
    {
      if (!stricmp (uni, "JAN"))
        remote_capabilities |= DOES_IANUS;
#ifdef HAVE_HYDRA
      else if (!stricmp (uni, "HYD"))
        remote_capabilities |= DOES_HYDRA;
#endif /* HAVE_HYDRA */
      else if (!stricmp (uni, "DZA"))
      {
        if (!no_dirzap)
        {
          remote_capabilities |= ZED_ZAPPER;
          direct_zap = 1;
        }
      }
      else if (!stricmp (uni, "ZAP"))
        remote_capabilities |= ZED_ZAPPER;
      else if (!stricmp (uni, "ZMO"))
        remote_capabilities |= ZED_ZIPPER;

      if (debugging_log)        /* r. hoerner: debug */
      {
        char junk[8];

        if (!stricmp (uni, "JAN"))
          strcpy (junk, "Janus");
        else if (!stricmp (uni, "HYD"))
          strcpy (junk, "Hydra");
        else if (!stricmp (uni, "DZA"))
          strcpy (junk, "DirZap");
        else if (!stricmp (uni, "ZAP"))
          strcpy (junk, "ZedZap");
        else if (!stricmp (uni, "ZMO"))
          strcpy (junk, "ZedZip");
        else
          strcpy (junk, "????");

        status_line (">Remote has %s", junk);
      }
    }

    if (!stricmp (uni, "NRQ"))
      remote_capabilities &= ~WZ_FREQ;
    if (remote_capabilities & my_capabilities)
      search_protocol = FALSE;
  }

  sptr += 2;
  sscanf (sptr, "%x", &prodcodeT);

  while (*sptr++ != '{') ;

  i = 0;
  while (*sptr != '}')
    uni[i++] = *sptr++;

  sptr += 2;
  uni[i++] = ' ';
  while (*sptr != '}')
    uni[i++] = *sptr++;

  sptr += 2;
  uni[i++] = '/';
  while (*sptr != '}')
    uni[i++] = *sptr++;

  if (uni[--i] != '/')
    i++;

  uni[i] = '\0';
  sptr++;

  for (;;)
  {
    if (*sptr != '{')
      break;

    for (addonnr = 0; addons[addonnr] != NULL; addonnr++)
    {
      if (!strnicmp (sptr, addons[addonnr], strlen (addons[addonnr])))
        break;
    }

    if (!addons[addonnr])       /* no known addon */
      addonnr = 0;
    else
      addonnr++;

    switch (addonnr)
    {
    case 1:                    /* {IDENT} system name */
      sptr += 9;
      name = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';

      strcpy (remote_site, name);  /* mtt 96-04-28 */

      sprintf (help, "%s %s (%s)", MSG_TXT (M_EMSI_SYSTEM),
               name, Full_Addr_Str (&remote_addr));
      status_line ("%s", help);
      if (akacnt > 1)
      {
        char *pa;
        int len1, len2;

        strcpy (help, MSG_TXT (M_EMSI_AKA));
        strcat (help, " ");
        for (j = i = 0, len1 = 5; i < akacnt; i++)
        {
          if (!memcmp (&addr[i], &remote_addr, sizeof (ADDR)))
            continue;

          pa = Full_Addr_Str (&addr[i]);
          len2 = strlen (pa);

          if ((j >= 1) && (len1 + len2 > 63))
          {
            /* TJW 960528 output if it would get too long */
            status_line ("%s", help);
            strcpy (help, MSG_TXT (M_EMSI_EMPTY));
            strcat (help, " ");
            j = 0;
            len1 = 5;
          }

          strcat (help, pa);
          strcat (help, " ");
          len1 = strlen (help);
          j++;
        }

        if (j)
          status_line ("%s", help);
      }

      sptr += 2;
      status_line ("%s %s", MSG_TXT (M_REMOTE_USES), uni);

      strcpy (remote_mailer, uni);  /* mtt 96-04-28 */

      loc = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';
      *uni = '\0';
      if (loc[0])
      {
        sprintf (uni, " from %s", loc);
        strcpy (remote_location, loc);  /* mtt 96-04-28 */
      }

      sptr += 2;
      name = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';
      if (name[0])
      {
        status_line ("%s %s%s", MSG_TXT (M_EMSI_SYSOP), name, uni);
        strcpy (remote_sysop, name);
        strcpy (lastfound_SysopName, name);
      }

      sptr += 2;                /* phone */
      name = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';

      sptr += 2;
      sptr = get_bracket (sptr);  /* baud */
      *sptr = '\0';

      sptr += 2;                /* flags */
      loc = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';

      if (loc[0])               /* mtt 96-04-29 */
        status_line ("%s %s", MSG_TXT (M_EMSI_FLAGS), loc);

      if (name[0] && strcmp (name, "-Unpublished-"))
      {
        status_line ("%s %s", MSG_TXT (M_EMSI_PHONE), name);
        strcpy (remote_phone, name);
      }

      while (*sptr++ != '}') ;
      break;

    case 2:                    /* {TRX#}  tranx */
      sptr += 8;
      name = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';
      while (*sptr++ != '}') ;

      /* MMP 960419 Parse the TRX, and store the difference in the
       * global variable (argh!) diff_trx, so it can be used at the end
       * of the session to adjust the clock. Adjusting the clock now
       * would make the time stamps in the log look wierd. It might also
       * cause timer problems (i don't know). */

      if (strlen (name) != 8 || sscanf (name, "%lx%c", &his_trx, &test) != 1)
        his_trx = 0;

      break;

    case 3:                    /* {MOH#}  mail on hold */
      sptr += 8;
      name = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';
      sscanf (name, "%lX", &RxStats.tot_moh);
      status_line (MSG_TXT (M_REMOTE_MOH), RxStats.tot_moh);
      while (*sptr++ != '}') ;
      break;

    case 4:                    /* {TZUTC}  diff from remote's timezone to UTC */
      sptr += 9;
      name = sptr;
      sptr = get_bracket (sptr);
      *sptr = '\0';
      remoteTZUTC = GetTZUTC (name);
      while (*sptr++ != '}') ;
      break;

    default:                   /* any other addon */
      name = ++sptr;
      while (*sptr != '}')
        sptr++;                 /* goto end of addon  string */
      *sptr++ = '\0';
      loc = ++sptr;
      sptr = get_bracket (sptr);  /* MB  new  */
      *sptr++ = '\0';
      status_line ("%s %s: {%s}", MSG_TXT (M_EMSI_ADDON), name, loc);
      break;
    }
  }

  if (his_trx)                  /* remote has sent his TRX# timestamp */
  {
    char timestr[80];
    long hisTZUTC = -1;
    NODESTRUC *np;

    if ((np = QueryNodeStruct (&remote_addr, 0)) != NULL)
      hisTZUTC = np->tzutc;     /* TZUTC info from config              */

    if (hisTZUTC == -1 && remoteTZUTC != -1)
      hisTZUTC = remoteTZUTC;   /* or TZUTC info from remote           */

    if (hisTZUTC == -1)
      hisTZUTC = TIMEZONE;      /* default remote TZUTC = my own TZUTC */

    /* we now have:            AW 980208                                   */
    /*      hisTZUTC                       his time diff from UTC          */
    /*      his_trx                        his local time with SET TZ=UTC0 */
    /*      TIMEZONE                       our time diff from UTC          */
    /*      etm                            our time (with our correct TZ!) */
    /* we can compute:                                                     */
    /*      his_trx+hisTZUTC               his unix time (UTC)             */
    /*      his_trx+hisTZUTC+TIMEZONE      his time (as with our TZ)       */
    /*      his_trx+hisTZUTC+TIMEZONE-etm  our real time diff              */

    his_trx += (time_t) TIMEZONE;  /* his local time for the logfile */
    strftime (timestr, 80, "%a, %d %b %Y %H:%M:%S", unix_localtime (&his_trx));
    status_line (MSG_TXT (M_EMSI_REMTIME), timestr, (int) (-hisTZUTC / 3600), abs ((int) (hisTZUTC % 60)));

    strftime (timestr, 80, "%a, %d %b %Y %H:%M:%S", unix_localtime (&etm));
    status_line (MSG_TXT (M_EMSI_LOCTIME), timestr, (int) (-TIMEZONE / 3600), abs ((int) (TIMEZONE % 60)));

    his_trx += (time_t) (hisTZUTC - TIMEZONE);  /* make his time normal UTC */
    diff_trx2 = (long) (his_trx - etm);
    status_line (MSG_TXT (M_EMSI_UTCDIFF), diff_trx2);
  }
  else
    diff_trx2 = 0;

  if (!remote_pickup)
    status_line (MSG_TXT (M_REMOTE_REFUSED_MAIL_PU));

  if (!num_rakas)
  {
    if (cur)
    {
      sptr = MSG_TXT (M_NUISANCE_CALLER);
      status_line (sptr);       /* TJW 960615 bugfix */
    }

    /*                             r. hoerner                  */
    if (!Sender)                /* if other side pays the bill */
      if (wrong_password)       /* and has a password failure  */
        if (nrTries < 2)        /* then allow a second run     */
          return (1);           /* to tell it (via EMSI_Send)  */

    modem_hangup ();            /* TJW was: LOWER_DTR (); */
    return (0);
  }

  if (do_timesync)              /* if we had our timesync node in EMSI AKAs ... */
    diff_trx = diff_trx2;

  /* update TxStats.tot_moh for remote's AKAs (if I am Sender)  */
  if (Sender)
  {
    TxStats.tot_moh = 0L;
    for (i = 0; i < num_rakas; i++)
      TxStats.tot_moh += totalsize (&remote_akas[i], 0);
  }

  return (1);
}

/* TJW 960720 made EMSI FSC-0056 compliant: caller chooses protocol preference
 * not called mailer as it was implemented in Binkley until now. */

static int LOCALFUNC
Encode_EMSI_Header (char *junkbuff, int Sender, char **emsi_output, long max_len)
{
  long i, j;
  int capabilities = 0, ret;
  char *uni;
  long noPickup, freeReq, noJanus, noHydra, noZip;
  AKA *aka;

  freeReq = QueryNodeFlag (on_our_nickel, FREEREQ, &remote_addr);
  noPickup = QueryNodeFlag (no_pickup, NOPICK, &remote_addr);
  noJanus = QueryNodeFlag (no_janus, NOJANUS, &remote_addr);
  noHydra = QueryNodeFlag (no_hydra, NOHYDRA, &remote_addr);
  noZip = QueryNodeFlag (no_zapzed, NOZED, &remote_addr);

  /* calcs MOH, too */
  CheckSmartBiDi (Sender, &noHydra, &noJanus, noPickup);

  if (Sender && !freeReq)
    matrix_mask &= ~TAKE_REQ;   /* r. hoerner: IF (sender && !on_our_nickel) */

  uni = bufp->emsi._uni;        /* TJW 960416 See why this is OK in Send_EMSI_Packet */

  /* ---------------------------------------------------------------------- */
  /* the following fragment tries to circumvent a miss in the original code */
  /* We now have (theoreticaly) an unlimited amount of AKA space, but the   */
  /* complete addresses must fit in a 4096 byte buffer. This will fail ;)   */
  /* Therefore we 1st count the AKAs. Then we assume a necessary space for  */
  /* them. Then we either continue (enough space) or we request memory. If  */
  /* we can allocate memory: all fine. Else: stop transmitting the AKAs when */
  /* our buffer is "enough" filled (here: after max_len byte).              */
  /*                                                             r. hoerner */
  /* ---------------------------------------------------------------------- */
  i = 0;
  /* count own AKAs */
  for (aka = alias; aka != NULL; aka = aka->next, i++) ;

  /* ---------------------------------------------------------------------- */
  /* a valid fido standard address is limited to this mask (the domain is   */
  /* not limited: it is simply not required)                                */
  /*           xxx:xxxxx/xxxxx.xxxxx@xxxxxxxxxxxx  : 33 byte per AKA        */
  /*           256:65535/65535.65535@fidonet.orgx    + 1 byte for ' '       */
  /* ---------------------------------------------------------------------- */
  if (i > ((max_len - 512) / 34))  /* 100 AKAs: 3400 Byte */
  {
    unsigned long needed = i * 34 + 512;
    unsigned int get;

    if (needed > 65535L)        /* EMSI limit */
      needed = 65535L;
    get = (unsigned int) needed;  /* convert to 16 bit integer */

    *emsi_output = calloc (1, get);  /* request a new buffer       */
    if (*emsi_output)           /* else: stop after more then max_len-512 byte */
    {
      junkbuff = *emsi_output;
      max_len = needed;
    }
  }

  /* ---------------------------------------------------------------------- */
  /* Anyway: here it goes.                                                  */
  /* ---------------------------------------------------------------------- */

  status_line (">EMSI: Checking addr: %s", Full_Addr_Str ((ADDR *) & remote_addr));
  aka = match_aka (&remote_addr);  /* Update 'assumed' */
  status_line (">EMSI: matching AKA:  %s", Full_Addr_Str ((ADDR *) aka));

  strcpy (junkbuff, "{EMSI}{");
  b7strcat (junkbuff, Full_Addr_Str ((ADDR *) aka));

  i = 0;
  aka = alias;
  while ((aka != NULL) && (strlen (junkbuff) < (unsigned int) (max_len - 512)))
  {
    if (i != assumed)
    {
      if ((!Sender) && (AKA_matching))
      {
        for (j = 0; j <= num_rakas; j++)
        {
          if (aka->Zone == remote_akas[j].Zone)
          {
            b7strcat (junkbuff, " ");
            b7strcat (junkbuff, Full_Addr_Str ((ADDR *) aka));
            j = num_rakas;
          }
        }
      }
      else
      {
        b7strcat (junkbuff, " ");
        b7strcat (junkbuff, Full_Addr_Str ((ADDR *) aka));
      }
    }

    if (strlen (junkbuff) < (unsigned int) (max_len - 512))
      aka = aka->next;

    i++;
  }

  /* ---------------------------------------------------------------------- */

  if (strlen (junkbuff) >= (unsigned int) (max_len - 512))
    status_line (MSG_TXT (M_EMSI_FULL), Full_Addr_Str ((ADDR *) aka));

  /* ---------------------------------------------------------------------- */

  b7strcat (junkbuff, "}{");
  remote_password = NULL;
  if (Sender)
    ret = n_getpassword (&remote_addr);
  else
    ret = (hl_aka >= 0) ? n_getpassword (&remote_akas[hl_aka]) : 0;
  if (ret > 0)
  {
    /* r. hoerner:
     * note: nowhere in FTS-56 it is said that a password is limited to any
     * size */

    if (remote_password)
    {
      strncpy ((char *) uni, remote_password, 8);
      uni[8] = '\0';
      b7strcat (junkbuff, uni);
    }
  }

  b7strcat (junkbuff, "}{8N1");

  if (Sender)
  {
    if (!noPickup)
    {
      if (pickup_all)
        b7strcat (junkbuff, ",PUA");
      else
        b7strcat (junkbuff, ",PUP");
    }
    else
      b7strcat (junkbuff, ",NPU");
  }
  else
  {
    if (matrix_mask & NO_TRAFFIC)
      b7strcat (junkbuff, ",HAT");
    if (!(matrix_mask & TAKE_REQ))
      b7strcat (junkbuff, ",HRQ");
  }

#ifdef HAVE_HYDRA
  if (!noHydra && (Sender || (remote_capabilities & DOES_HYDRA)))  /* MR 970809 */
    b7strcat (junkbuff, ",RH1");  /* TJW 960528 RH1hydra / xHydra */
#endif

  b7strcat (junkbuff, "}{");

  /* ------------------------------------------------------------------------
   *  begin of protocol section
   * ---------------------------------------------------------------------- */

  /* ------------------------------------------------------------------------
   * If we are the CALLING system, turn on all our capabilities
   * except those, that are configured to be not available for the remote node
   * At least one ZModem protocol (ZMO) must be available for FSC-0056 !
   * ---------------------------------------------------------------------- */

  my_capabilities &= ~(DOES_IANUS | DOES_HYDRA | ZED_ZAPPER);  /* VRP 990909 */
  my_capabilities |= ZED_ZIPPER;

  if (!noJanus)
  {
    my_capabilities |= DOES_IANUS;
  }

#ifdef HAVE_HYDRA
  if (!noHydra)
  {
    my_capabilities |= DOES_HYDRA;
  }
#endif

  if (!noZip)
  {
    my_capabilities |= ZED_ZAPPER;
  }

  if (!no_dirzap)
  {
    my_capabilities |= DIR_ZAPPER;
  }

  /* ------------------------------------------------------------------------
   * If we are the ANSWERING system, choose the first choice from
   * the set that the caller gave us and only send this one back.
   * See also EMSI_Decoder.
   * ---------------------------------------------------------------------- */

  if (!Sender)
  {
    unsigned short caps = remote_capabilities & my_capabilities;

    if (caps & DOES_IANUS)
      b7strcat (junkbuff, "JAN,");
#ifdef HAVE_HYDRA
    else if (caps & DOES_HYDRA)
      b7strcat (junkbuff, "HYD,");
#endif
    else if (caps & ZED_ZAPPER)
    {
      if (direct_zap)
        b7strcat (junkbuff, "DZA,");
      else
        b7strcat (junkbuff, "ZAP,");
    }
    else if (caps & ZED_ZIPPER)
      b7strcat (junkbuff, "ZMO,");
    else
      b7strcat (junkbuff, "NCP,");

    /* r.hoerner 9 feb 97:
     * remove all but the actual working capability from my_capabilities. */

    my_capabilities &= ~(DOES_IANUS | DOES_HYDRA | ZED_ZAPPER | ZED_ZIPPER);

    if (caps & DOES_IANUS)
      my_capabilities |= DOES_IANUS;
#ifdef HAVE_HYDRA
    else if (caps & DOES_HYDRA)
      my_capabilities |= DOES_HYDRA;
#endif
    else if (caps & ZED_ZAPPER)
      my_capabilities |= ZED_ZAPPER;
    else if (caps & ZED_ZIPPER)
      my_capabilities |= ZED_ZIPPER;

    /* r.hoerner 9 feb 97:
     * remove all but the actual working capability from
     * remote_capabilities. Else b_wazoo.c might get confused if we
     * do smart_bidi. */

    remote_capabilities &= ~(DOES_IANUS | DOES_HYDRA | ZED_ZAPPER | ZED_ZIPPER);
    remote_capabilities |= (my_capabilities &
                            (DOES_IANUS | DOES_HYDRA |
                             ZED_ZAPPER | ZED_ZIPPER));
  }

  /* ------------------------------------------------------------------------
   *  We are CALLER: tell preferred protocols IF available for this remote
   * ---------------------------------------------------------------------- */

  if (Sender)
  {
    char *p;
    int did_protocol = 0;

    capabilities = my_capabilities;
    p = actprotpreference;

    if (*p)                     /* either "JAN", "HYD", "ZMO", "ZAP" do exist. */
    {
      /* --------------------------------------------------------------------
       * go through the list of "preferred" protocols and tell them in the
       * sequence they are preferred, but only if it the protocol is allowed,
       * too. don't tell a protocol twice (could be, 'cause it is user defined)
       * ------------------------------------------------------------------ */

      char i = 0;
      char *q;
      char *delims =
      {" ,;%"};

      q = strdup (p);
      p = strtok (q, delims);

      for (i = 0; (p != NULL) && (i < 4); i++, p = strtok (NULL, delims))
      {
        if ((strnicmp (p, "JAN", 3) == 0) && (capabilities & DOES_IANUS)
            && !(did_protocol & DOES_IANUS))
        {
          b7strcat (junkbuff, "JAN,");
          did_protocol |= DOES_IANUS;
        }
#ifdef HAVE_HYDRA
        else if ((strnicmp (p, "HYD", 3) == 0) && (capabilities & DOES_HYDRA)
                 && !(did_protocol & DOES_HYDRA))
        {
          b7strcat (junkbuff, "HYD,");
          did_protocol |= DOES_HYDRA;
        }
#endif
        else if ((strnicmp (p, "DZA", 3) == 0) && (capabilities & DIR_ZAPPER)
                 && !(did_protocol & DIR_ZAPPER))
        {
          b7strcat (junkbuff, "DZA,");
          did_protocol |= DIR_ZAPPER;
        }
        else if ((strnicmp (p, "ZAP", 3) == 0) && (capabilities & ZED_ZAPPER)
                 && !(did_protocol & ZED_ZAPPER))
        {
          b7strcat (junkbuff, "ZAP,");
          did_protocol |= ZED_ZAPPER;
        }
        else if ((strnicmp (p, "ZMO", 3) == 0) && (capabilities & ZED_ZIPPER)
                 && !(did_protocol & ZED_ZIPPER))
        {
          b7strcat (junkbuff, "ZMO,");
          did_protocol |= ZED_ZIPPER;
        }
      }
    }

    /* --------------------------------------------------------------------
     * Did we in fact tell anything??
     * ------------------------------------------------------------------- */
    if (!(did_protocol & ZED_ZIPPER))  /* No protocol configured  Xo(     */
      b7strcat (junkbuff, "ZMO,");  /* ZMO is a MUST in EMSI (fsc-56)  */
  }

  /* -----------------------------------------------------------------------
   *  end of protocol section
   * ---------------------------------------------------------------------- */

  if (!(matrix_mask & TAKE_REQ) || (Sender && !freeReq))
    b7strcat (junkbuff, "NRQ,");  /* ^^^ this is meant by this emsi flag */
  else
    capabilities |= WZ_FREQ;

  b7strcat (junkbuff, "ARC,XMA,FNC}");  /* MB 93-11-27 */
  sprintf (uni, "{%02X}", PRDCT_CODE);
  b7strcat (junkbuff, uni);
  sprintf (uni, "{%s}", PRD_SH_PRTY);
  cat_bracket (junkbuff, uni);
  sprintf (uni, "{%s}", PRDCT_VRSN);  /* TJW 960415 */
  b7strcat (junkbuff, uni);

  if (wrong_password)
    sprintf (uni, "{%s}", PASSWORD_FAILURE);
  else
    sprintf (uni, "{%s-%s%s%s %s}", version_type, version_base, version_modified, version_opts, compilerinfo);

  cat_bracket (junkbuff, uni);
  b7strcat (junkbuff, "{IDENT}{");

  if (wrong_password)
    sprintf (uni, "[%s]", PASSWORD_FAILURE);
  else
    sprintf (uni, "[%s]", system_name);

  cat_bracket (junkbuff, uni);
  if (wrong_password)
    sprintf (uni, "[%s]", CHECK_SETUP);
  else
    sprintf (uni, "[%s]", location);  /*  vvv  */

  cat_bracket (junkbuff, uni);
  if (wrong_password)
    sprintf (uni, "[%s]", PASSWORD_FAILURE);
  else
    sprintf (uni, "[%s]", sysop);

  cat_bracket (junkbuff, uni);
  sprintf (uni, "[%s]", myphone);
  cat_bracket (junkbuff, uni);
  sprintf (uni, "[%s]", mymaxbaud);
  cat_bracket (junkbuff, uni);
  sprintf (uni, "[%s]", myflags);
  cat_bracket (junkbuff, uni);  /* MB 93-11-27 */

  if (no_tranx)                 /* JNK 970312 */
    b7strcat (junkbuff, "}");
  else
  {
    b7strcat (junkbuff, "}{TZUTC}");  /* TJW 971031, AW 980208 send own TZ */
    sprintf (uni, "{[%2.02d%02d]}", (int) (-TIMEZONE / 3600), abs (TIMEZONE % 60));
    b7strcat (junkbuff, uni);
    b7strcat (junkbuff, "{TRX#}");
    unix_time (&etm);           /* r.hoerner: update etm NOW */
    sprintf (uni, "{[%08lX]}", etm - (time_t) TIMEZONE);
    b7strcat (junkbuff, uni);
  }

  b7strcat (junkbuff, "{MOH#}");  /* TJW 960704 send mail on hold info */
  sprintf (uni, "{[%lX]}", TxStats.tot_moh);
  b7strcat (junkbuff, uni);

  status_line (MSG_TXT (M_LOCAL_MOH), TxStats.tot_moh);
  return (strlen (junkbuff));
}

/* AW 980208 convert TZUTC string from emsi packet into time in seconds */
static long LOCALFUNC
GetTZUTC (char *s)
{
  int l;
  long tzutc = -1;
  char tmp[6];

  l = strlen (s);
  if (l == 4 || l == 5)
  {
    strcpy (tmp, s);
    tzutc = atoi (tmp + l - 2);
    tmp[l - 2] = '\0';
    tzutc = -(tzutc + 60 * atoi (tmp)) * 60;
  }

  /* printf("tz=%s, converted=%ld\n", s, tzutc); */
  return tzutc;
}

/* $Id: emsi.c,v 1.7 1999/09/27 20:51:25 mr Exp $ */
