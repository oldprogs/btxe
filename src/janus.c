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
 * Filename    : $Source: E:/cvs/btxe/src/janus.c,v $
 * Revision    : $Revision: 1.11 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/04/01 00:52:11 $
 * State       : $State: Exp $
 * Orig. Author: Rick Huebner
 *
 * Description :
 *   BinkleyTerm Janus revision 0.31, 11-2-89
 *   Full-duplex WaZOO file transfer protocol
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/* Private routines */

static void LOCALFUNC getfname (word);
static void LOCALFUNC sendpkt (byte *, int, int);
static void LOCALFUNC sendpkt32 (byte *, int, int);
static void LOCALFUNC txbyte (byte);
static long LOCALFUNC procfname (void);
static byte LOCALFUNC rcvpkt (void);
static void LOCALFUNC rxclose (word);
static void LOCALFUNC endbatch (void);
static void LOCALFUNC long_set_timer (long *, unsigned int);
static int LOCALFUNC long_time_gone (long *);
static int LOCALFUNC rcvrawbyte (void);
static int LOCALFUNC rxbyte (void);
static int LOCALFUNC get_filereq (byte);
static int record_reqfile (char *);  /* ** NOT ** LOCALFUNC!!! */
static int timeof_reqfile (long);  /* ** NOT ** LOCALFUNC!!! */
static byte LOCALFUNC get_reqname (byte);
static void LOCALFUNC mark_done (char *);

/* Private data. I know, proper software design says you shouldn't make data */
/* global unless you really need to.  In this case speed and code size make  */
/* it more important to avoid constantly pushing & popping arguments.        */

static char *GenericError = "!%s";
static char ReqTmp[PATHLEN];
static char *Rxbuf;             /* Address of packet reception buffer              */
static char *Txfname;           /* Full path of file we're sending                 */
static char *Rxfname;           /* Full path of file we're receiving               */
static byte *Rxbufptr;          /* Current position within packet reception buffer */
static byte *Rxbufmax;          /* Upper bound of packet reception buffer          */
static byte Do_after;           /* What to do with file being sent when we're done */
static byte WaitFlag;           /* Tells rcvrawbyte() whether or not to wait       */
static byte SharedCap;          /* Capability bits both sides have in common       */
static FILE *Txfile;            /* File handle of file we're sending               */
static FILE *Rxfile;            /* File handle of file we're receiving             */
static int ReqRecorded;         /* Number of files obtained by this request        */
static word TimeoutSecs;        /* How long to wait for various things             */
static int Rxblklen;            /* Length of data in last data block packet recvd  */
static long Txlen;              /* Total length of file we're sending              */
static long Rxlen;              /* Total length of file we're receiving            */
static long Rxfiletime;         /* Timestamp of file we're receiving               */
static long Diskavail;          /* 64Byte units available in upload directory      */
static int EMSI_flag;           /* Currently in EMSI-Session ?                     */
static int EMSI_aka;            /* Current aka to send mail to                     */
static int EMSI_raka;           /* Current aka to request files from               */

/*****************************************************************************/
/* Super-duper neato-whizbang full-duplex streaming ACKless batch file       */
/* transfer protocol for use in WaZOO mail sessions                          */
/*****************************************************************************/

MISSION_ERR
Janus (void)
{
  byte xstate;                  /* Current file transmission state                 */
  byte rstate;                  /* Current file reception state                    */
  byte pkttype;                 /* Type of packet last received                    */
  byte tx_inhibit;              /* Flag to wait and send after done receiving      */
  char *holdname;               /* Name of hold area                               */

  /* r.hoerner: old local variable "fsent": renamed to jsent (1/6/97) */
  byte jsent;                   /* Did we manage to send anything this session?    */
  byte sending_req;             /* Are we currently sending requested files?       */
  byte attempting_req;          /* Are we waiting for the sender to start our req? */
  byte req_started;             /* Has the sender started servicing our request?   */
  word blklen;                  /* Length of last data block sent                  */
  word txblklen;                /* Size of data block to try to send this time     */
  word txblkmax;                /* Max size of data block to send at this speed    */
  word goodneeded;              /* # good bytes to send before upping txblklen     */
  word goodbytes;               /* Number of good bytes sent at this block size    */
  word rpos_count;              /* Number of RPOS packets sent at this position    */
  long xmit_retry;              /* Time to retransmit lost FNAMEPKT or EOF packet  */
  long txpos = 0L;              /* Current position within file we're sending      */
  long rxpos = 0L;              /* Current position within file we're receiving    */
  long lasttx;                  /* Position within file of last data block we sent */
  long txstpos;                 /* Initial data position of file we're sending     */
  long rxstpos;                 /* Initial data position of file we're receiving   */
  long rpos_retry;              /* Time at which to retry RPOS packet              */
  long brain_dead;              /* Time at which to give up on other computer      */
  long rpos_sttime = 0;         /* Time at which we started current RPOS sequence  */
  long last_rpostime;           /* Timetag of last RPOS which we performed         */
  long last_blkpos;             /* File position of last out-of-sequence BLKPKT    */
  FILE *reqfile;                /* File handle for .REQ file                       */
  MISSION_ERR error = mission_ok;

  sprintf (ReqTmp, request_template, flag_dir, "janusreq", TaskNumber);

  set_prior (PRIO_JANUS);
  XON_DISABLE ();

  SharedCap = 0;

  /* ---------------------------------------------------------------------- */
  /* Allocate memory                                                        */
  /* ---------------------------------------------------------------------- */
  Txbuf = bufp->janus._Txbuf;
  Rxbuf = (char *) bufp->janus._Rxbuf;
  Txfname = Rxfname = NULL;
  if (((Txfname = malloc (PATHLEN)) == NULL)
      || ((Rxfname = malloc (PATHLEN)) == NULL))
  {
    status_line (MSG_TXT (M_MEM_ERROR));
    modem_hangup ();
    error = mission_error;
    goto freemem;
  }
  Rxbufmax = (byte *) (Rxbuf + JANUSRXBUFSIZE);

  /* ---------------------------------------------------------------------- */
  /* Initialize file transmission variables                                 */
  /* ---------------------------------------------------------------------- */
  if (!no_EMSI_Session)
  {
    EMSI_flag = TRUE;
    EMSI_aka = EMSI_raka = 0;
    called_addr = remote_akas[EMSI_aka];
  }
  else
    EMSI_flag = FALSE;

  tx_inhibit = FALSE;
  last_rpostime = last_blkpos = lasttx = txstpos = rxstpos = xmit_retry = 0L;
  long_set_timer (&brain_dead, 120);

  if (cur_baud.rate_value > 9600L)
  {
    TimeoutSecs = 30;
    txblkmax = BUFMAX;
  }
  else
  {
    TimeoutSecs = (unsigned int) (40960L / cur_baud.rate_value);
    if (TimeoutSecs < 30)
      TimeoutSecs = 30;
    txblkmax = (int) cur_baud.rate_value / 300 * 128;
    if (txblkmax > BUFMAX)
      txblkmax = BUFMAX;
  }

  txblklen = txblkmax;
  goodbytes = goodneeded = 0;
  Txfile = NULL;
  sending_req = jsent = FALSE;
  xstate = XSENDFNAME;
  getfname (INITIAL_XFER);

  /* ---------------------------------------------------------------------- */
  /* Initialize file reception variables                                    */
  /* ---------------------------------------------------------------------- */
  holdname = HoldAreaNameMunge (&called_addr);
  sprintf (Abortlog_name, "%s%s.z", holdname, Hex_Addr_Str (&called_addr));

  Diskavail = zfree (CURRENT.sc_Inbound);

  Rxbufptr = NULL;
  rpos_retry = rpos_count = 0;
  attempting_req = req_started = FALSE;
  rstate = RRCVFNAME;

  /* ---------------------------------------------------------------------- */
  /* Send and/or receive stuff until we're done with both                   */
  /* ---------------------------------------------------------------------- */
  do
  {                             /* while (xstate || rstate)  */

    /* ------------------------------------------------------------------- */
    /* If nothing useful (i.e. sending or receiving good data block) has   */
    /* happened within the last 2 minutes, give up in disgust              */
    /* ------------------------------------------------------------------- */
    if (long_time_gone (&brain_dead))
    {
      status_line (MSG_TXT (M_OTHER_DIED));  /* "He's dead, Jim." */
      TxStats.cur_errors++;
      RxStats.cur_errors++;
      goto giveup;
    }

    /* ------------------------------------------------------------------- */
    /* If we're tired of waiting for an ACK, try again                     */
    /* ------------------------------------------------------------------- */
    if (xmit_retry)
    {
      if (long_time_gone (&xmit_retry))
      {
        status_line ("!%s", MSG_TXT (M_TIMEOUT));
        css_upd ();
        xmit_retry = 0L;

        switch (xstate)
        {
        case XRCVFNACK:
          xstate = XSENDFNAME;
          break;

        case XRCVFRNAKACK:
          xstate = XSENDFREQNAK;
          break;

        case XRCVEOFACK:
          if (fseek (Txfile, txpos = lasttx, SEEK_SET))
          {
            got_error (MSG_TXT (M_SEEK_MSG), Txfname);
            TxStats.cur_errors++;
            goto giveup;
          }
          xstate = XSENDBLK;
          break;
        }
      }
    }

    /* ------------------------------------------------------------------- */
    /* Transmit next part of file, if any                                  */
    /* ------------------------------------------------------------------- */
    switch (xstate)
    {
    case XSENDBLK:
      if (tx_inhibit)
        break;

#ifdef GENERIC
      lasttx = txpos;
      Txbuf[0] = txpos & 0xff;
      Txbuf[1] = (txpos >> 8) & 0xff;
      Txbuf[2] = (txpos >> 16) & 0xff;
      Txbuf[3] = (txpos >> 24) & 0xff;
#else
      *((long *) Txbuf) = lasttx = txpos;
#endif

      blklen = fread (Txbuf + sizeof (txpos), sizeof (char), txblklen, Txfile);

      if (ferror (Txfile))
      {
        got_error (MSG_TXT (M_READ_MSG), Txfname);
        TxStats.cur_errors++;
        goto giveup;
      }

      txpos += blklen;
      sendpkt (Txbuf, sizeof (txpos) + blklen, BLKPKT);
      TxStats.FilePos = txpos;
      css_upd ();
      jsent = TRUE;

      if (txpos >= Txlen || blklen < txblklen)
      {
        long_set_timer (&xmit_retry, TimeoutSecs);
        xstate = XRCVEOFACK;
      }
      else
        long_set_timer (&brain_dead, 120);

      if (txblklen < txblkmax && (goodbytes += txblklen) >= goodneeded)
      {
        txblklen <<= 1;
        goodbytes = 0;
      }
      break;

    case XSENDFNAME:
      blklen = (int) (strchr (strchr ((char *) Txbuf, '\0') + 1, '\0') - (char *) Txbuf) + 1;
      Txbuf[blklen++] = OURCAP;
      sendpkt (Txbuf, blklen, FNAMEPKT);
      long_set_timer (&xmit_retry, TimeoutSecs);
      xstate = XRCVFNACK;
      break;

    case XSENDFREQNAK:
      sendpkt (NULL, 0, FREQNAKPKT);
      long_set_timer (&xmit_retry, TimeoutSecs);
      xstate = XRCVFRNAKACK;
      break;
    }

    /* ------------------------------------------------------------------- */
    /* Catch up on our reading; receive and handle all outstanding packets */
    /* ------------------------------------------------------------------- */
    while ((pkttype = rcvpkt ()) != 0)
    {
      if (pkttype != BADPKT)
        long_set_timer (&brain_dead, 120);

      switch (pkttype)
      {

        /* ------------------------------------------------------------- */
        /* File data block or munged block                               */
        /* ------------------------------------------------------------- */
      case BADPKT:
      case BLKPKT:
        if (rstate == RRCVBLK)
        {
          long t;

#ifdef GENERIC
          t = (long) Rxbuf[0] +
            ((long) Rxbuf[1] << 8) +
            ((long) Rxbuf[2] << 16) +
            ((long) Rxbuf[3] << 24);
#else
          t = *(long *) Rxbuf;
#endif
          if (pkttype == BADPKT || (t != rxpos))
          {
            if (pkttype == BLKPKT)
            {
              if (t < last_blkpos)
                rpos_retry = rpos_count = 0;
              last_blkpos = t;
            }
            if (long_time_gone (&rpos_retry))
            {

              /* ------------------------------------------------- */
              /* If we're the called machine, and we're trying to  */
              /* send stuff, and it seems to be screwing up our    */
              /* ability to receive stuff, maybe this connection   */
              /* just can't hack full-duplex.  Try waiting till    */
              /* the sending system finishes before sending our    */
              /* stuff to it                                       */
              /* ------------------------------------------------- */
              if (rpos_count > 4)
              {
                if (xstate && !isOriginator && !tx_inhibit)
                {
                  tx_inhibit = TRUE;
                  status_line (MSG_TXT (M_GOING_ONE_WAY));
                }
                rpos_count = 0;
              }

              if (++rpos_count == 1)
                unix_time ((time_t *) & rpos_sttime);

              status_line (MSG_TXT (M_J_BAD_PACKET), rxpos);

#ifdef GENERIC
              Rxbuf[0] = rxpos & 0xff;
              Rxbuf[1] = (rxpos >> 8) & 0xff;
              Rxbuf[2] = (rxpos >> 16) & 0xff;
              Rxbuf[3] = (rxpos >> 24) & 0xff;
              Rxbuf[4] = rpos_sttime & 0xff;
              Rxbuf[5] = (rpos_sttime >> 8) & 0xff;
              Rxbuf[6] = (rpos_sttime >> 16) & 0xff;
              Rxbuf[7] = (rpos_sttime >> 24) & 0xff;
#else
              *((long *) Rxbuf) = rxpos;
              *((long *) (Rxbuf + sizeof (rxpos))) = rpos_sttime;
#endif
              sendpkt ((byte *) Rxbuf, sizeof (rxpos) + sizeof (rpos_sttime), RPOSPKT);
              long_set_timer (&rpos_retry, TimeoutSecs / 2);
            }
          }
          else
          {
            last_blkpos = rxpos;
            rpos_retry = rpos_count = 0;
            fwrite (Rxbuf + sizeof (rxpos), sizeof (char),
                    Rxblklen -= sizeof (rxpos), Rxfile);

            if (ferror (Rxfile))
            {
              got_error (MSG_TXT (M_WRITE_MSG), Rxfname);
              RxStats.cur_errors++;
              goto giveup;
            }

            Diskavail -= Rxblklen / 64;
            rxpos += Rxblklen;
            RxStats.FilePos = rxpos;
            css_upd ();

            if (rxpos >= Rxlen)
            {
              long Rxtime;

              rxclose (GOOD_XFER);
              RxStats.cur_mxfrd += Rxlen;
              RxStats.cur_fxfrd++;
              RxStats.FilePos = RxStats.FileLen = -1L;
              css_upd ();
              Rxlen -= rxstpos;
              Rxtime = throughput (1, 0, Rxlen) / PER_SECOND;
              status_line ("%s-J%s %s", MSG_TXT (M_FILE_RECEIVED),
                           (SharedCap & CANCRC32) ? "/32" : " ", Rxfname);
              update_files (0, Rxfname, Rxlen, Rxtime, &(RxStats.cur_errors));
              rstate = RRCVFNAME;
            }
          }
        }

        if (rstate == RRCVFNAME)
          sendpkt (NULL, 0, EOFACKPKT);
        break;

        /* ------------------------------------------------------------- */
        /* Name and other data for next file to receive                  */
        /* ------------------------------------------------------------- */
      case FNAMEPKT:
        if (rstate == RRCVFNAME)
          rxpos = rxstpos = procfname ();
        if (!Rxfname[0] && get_filereq (req_started))
        {
          sendpkt ((byte *) Rxbuf, strlen (Rxbuf) + 2, FREQPKT);
          attempting_req = TRUE;
          req_started = FALSE;
        }
        else
        {
          if (attempting_req)
          {
            attempting_req = FALSE;
            req_started = TRUE;
          }
#ifdef GENERIC
          Rxbuf[0] = rxpos & 0xff;
          Rxbuf[1] = (rxpos >> 8) & 0xff;
          Rxbuf[2] = (rxpos >> 16) & 0xff;
          Rxbuf[3] = (rxpos >> 24) & 0xff;
          Rxbuf[4] = SharedCap;
#else
          *((long *) Rxbuf) = rxpos;
          Rxbuf[sizeof (rxpos)] = (char) SharedCap;
#endif
          sendpkt ((byte *) Rxbuf, sizeof (rxpos) + 1, FNACKPKT);

          if (rxpos > -1)
            rstate = (byte) ((Rxfname[0]) ? RRCVBLK : RDONE);
          else
            status_line (MSG_TXT (M_REFUSING), Rxfname);

          if (!rstate)
            tx_inhibit = FALSE;

          if (!(xstate || rstate))
            goto breakout;
        }
        break;

        /* ------------------------------------------------------------- */
        /* ACK to filename packet we just sent                           */
        /* ------------------------------------------------------------- */
      case FNACKPKT:
        if (xstate == XRCVFNACK)
        {
          xmit_retry = 0L;
          if (Txfname[0])
          {
#ifdef GENERIC
            SharedCap = (Rxblklen > sizeof (long)) ? Rxbuf[4] : 0;

            txpos = (long) Rxbuf[0] +
              ((long) Rxbuf[1] << 8) +
              ((long) Rxbuf[2] << 16) +
              ((long) Rxbuf[3] << 24);
#else
            SharedCap = (byte) ((Rxblklen > sizeof (long)) ? Rxbuf[sizeof (long)] : 0);

            txpos = *((long *) Rxbuf);
#endif
            if (txpos > -1L)
            {
              if (txpos)
                status_line (MSG_TXT (M_SYNCHRONIZING), txpos);

              if (fseek (Txfile, txstpos = txpos, SEEK_SET))
              {
                got_error (MSG_TXT (M_SEEK_MSG), Txfname);
                TxStats.cur_errors++;
                goto giveup;
              }

              xstate = XSENDBLK;
            }
            else
            {
              status_line (MSG_TXT (M_REMOTE_REFUSED), Txfname);
              if (sending_req)
              {
                if (!(sending_req = get_reqname (FALSE)))
                  getfname (GOOD_XFER);
              }
              else
              {
                Do_after = NOTHING_AFTER;
                getfname (GOOD_XFER);
              }

              xstate = XSENDFNAME;
            }
          }
          else
          {
            sent_mail = 1;
            xstate = XDONE;
            fsent++;            /* fsent or jsent ?????????? */
          }
        }

        if (!(xstate || rstate))
          goto breakout;

        break;

        /* ------------------------------------------------------------- */
        /* Request to send more stuff rather than end batch just yet     */
        /* ------------------------------------------------------------- */
      case FREQPKT:
        if (xstate == XRCVFNACK)
        {
          static char addr_str[100];

          xmit_retry = 0L;
          if (alias->Point != 0)
            sprintf (addr_str, "%08hx", alias->Point);
          else
            sprintf (addr_str, "%04hx%04hx", alias->Net, alias->Node);

          SharedCap = *(strchr (Rxbuf, '\0') + 1);
          sprintf ((char *) Txbuf, request_template, CURRENT.sc_Inbound,
                   addr_str, TaskNumber);

          reqfile = fopen ((char *) Txbuf, write_ascii);
          if (reqfile == (FILE *) NULL)
          {
            got_error (MSG_TXT (M_OPEN_MSG), (char *) Txbuf);
            TxStats.cur_errors++;
          }
          else
          {
            fputs (Rxbuf, reqfile);
            fputs ("\n", reqfile);
            fclose (reqfile);
          }

          unlink (ReqTmp);
          ReqRecorded = 0;      /* counted by record_reqfile */

          fsent += respond_to_file_requests (1, record_reqfile, timeof_reqfile);
          // fsent or jsent ???????????

          CURRENT.rq_Limit -= ReqRecorded;
          if ((sending_req = get_reqname (TRUE)) != 0)
            xstate = XSENDFNAME;
          else
            xstate = XSENDFREQNAK;
        }
        break;

        /* ------------------------------------------------------------- */
        /* Our last file request didn't match anything; move on to next  */
        /* ------------------------------------------------------------- */
      case FREQNAKPKT:
        attempting_req = FALSE;
        req_started = TRUE;
        sendpkt (NULL, 0, FRNAKACKPKT);
        break;

        /* ------------------------------------------------------------- */
        /* ACK to no matching files for request error; try to end again  */
        /* ------------------------------------------------------------- */
      case FRNAKACKPKT:
        if (xstate == XRCVFRNAKACK)
        {
          xmit_retry = 0L;
          getfname (GOOD_XFER);
          xstate = XSENDFNAME;
        }
        break;

        /* ------------------------------------------------------------- */
        /* ACK to last data block in file                                */
        /* ------------------------------------------------------------- */
      case EOFACKPKT:
        if (xstate == XRCVEOFACK || xstate == XRCVFNACK)
        {
          xmit_retry = 0L;
          if (xstate == XRCVEOFACK)
          {
            long Txtime;

            Txlen -= txstpos;
            Txtime = throughput (1, 1, Txlen) / PER_SECOND;

            status_line ("%s-J%s %s", MSG_TXT (M_FILE_SENT),
                         (SharedCap & CANCRC32) ? "/32" : " ", Txfname);

            update_files (1, Txfname, Txlen, Txtime, &(TxStats.cur_errors));
            TxStats.cur_mxfrd += Txlen;
            TxStats.cur_fxfrd++;
            TxStats.FilePos = TxStats.FileLen = -1L;
            css_upd ();
            if (sending_req)
            {
              if (!(sending_req = get_reqname (FALSE)))
                getfname (GOOD_XFER);
            }
            else
              getfname (GOOD_XFER);
          }
          xstate = XSENDFNAME;
        }
        break;

        /* ------------------------------------------------------------- */
        /* Receiver says "let's try that again."                         */
        /* ------------------------------------------------------------- */
      case RPOSPKT:
        if (xstate == XSENDBLK || xstate == XRCVEOFACK)
        {
          long t;

#ifdef GENERIC
          t = (long) Rxbuf[4] +
            ((long) Rxbuf[5] << 8) +
            ((long) Rxbuf[6] << 16) +
            ((long) Rxbuf[7] << 24);
#else
          t = *((long *) (Rxbuf + sizeof (txpos)));
#endif
          if (t != last_rpostime)
          {
            last_rpostime = t;
            xmit_retry = 0L;
            CLEAR_OUTBOUND ();

#ifdef GENERIC
            lasttx = (long) Rxbuf[0] +
              ((long) Rxbuf[1] << 8) +
              ((long) Rxbuf[2] << 16) +
              ((long) Rxbuf[3] << 24);
#else
            lasttx = *((long *) Rxbuf);
#endif

            if (fseek (Txfile, txpos = lasttx, SEEK_SET))
            {
              got_error (MSG_TXT (M_SEEK_MSG), Txfname);
              TxStats.cur_errors++;
              goto giveup;
            }

            status_line (MSG_TXT (M_SYNCHRONIZING), txpos);
            txblklen >>= 2;
            if (txblklen < 64)
              txblklen = 64;

            goodbytes = 0;
            goodneeded += 1024;
            if (goodneeded > 8192)
              goodneeded = 8192;

            xstate = XSENDBLK;
          }
        }
        break;

        /* ------------------------------------------------------------- */
        /* Debris from end of previous Janus session; ignore it          */
        /* ------------------------------------------------------------- */
      case HALTACKPKT:
        break;

        /* ------------------------------------------------------------- */
        /* Abort the transfer and quit                                   */
        /* ------------------------------------------------------------- */
      default:
        status_line (MSG_TXT (M_UNKNOWN_PACKET), pkttype);
        /* fallthrough */

      case HALTPKT:
      giveup:
        status_line (MSG_TXT (M_SESSION_ABORT));
        if (Txfname[0])
          getfname (ABORT_XFER);

        if (rstate == RRCVBLK)
          rxclose (FAILED_XFER);

        error = mission_aborted;
        goto abortxfer;

      }                         /* switch (pkttype)  */
    }                           /* while (pkttype)   */
  }
  while (xstate || rstate);

  /* ---------------------------------------------------------------------- */
  /* All done; make sure other end is also finished (one way or another)    */
  /* ---------------------------------------------------------------------- */
breakout:

  if (!jsent)
    status_line (MSG_TXT (M_NOTHING_TO_SEND), Full_Addr_Str (&called_addr));

abortxfer:

  endbatch ();

  /* ---------------------------------------------------------------------- */
  /* Release allocated memory                                               */
  /* ---------------------------------------------------------------------- */

freemem:

  if (Txfname)
    free (Txfname);
  if (Rxfname)
    free (Rxfname);
  set_prior (PRIO_MODEM);

  // CEH 980803
  // if there are still errors counted in ?xStats.cur_errors (when a file has not
  // been received completely, so that update_file has not been called):

  TxStats.tot_errors += TxStats.cur_errors;
  RxStats.tot_errors += RxStats.cur_errors;
  hist.err_out += TxStats.cur_errors;
  hist.err_in += RxStats.cur_errors;
  TxStats.cur_errors = 0;
  RxStats.cur_errors = 0;

  return error;
}

/*****************************************************************************/
/* Get name and info for next file to be transmitted, if any, and build      */
/* FNAMEPKT.  Packet contents as per ZModem filename info packet, to allow   */
/* use of same method of aborted-transfer recovery.  If there are no more    */
/* files to be sent, build FNAMEPKT with null filename.  Also open file and  */
/* set up for transmission.  Set Txfname, Txfile, Txlen.  Txbuf must not be  */
/* modified until FNACKPKT is received.                                      */
/*****************************************************************************/
static void LOCALFUNC
getfname (word xfer_flag)
{
  static byte floflag, bad_xfers;
  static char outboundname[PATHLEN];
  static long floname_pos;
  static FILE *flofile;
  char *holdname;

  register char *p;
  int i;
  long curr_pos;
  struct stat f;

  /* ---------------------------------------------------------------------- */
  /* Initialize static variables on first call of the batch                 */
  /* ---------------------------------------------------------------------- */
  if (xfer_flag == INITIAL_XFER)
  {
    if (EMSI_flag)
      status_line (MSG_TXT (M_EMSI_PROC_NODE),
                   Full_Addr_Str (&remote_akas[EMSI_aka]),
                   HoldAreaNameMunge (&remote_akas[EMSI_aka]));

    floflag = outboundname[0] = '\0';
    flofile = NULL;
  }
  else
    /* -------------------------------------------------------------------- */
    /* If we were already sending a file, close it and clean up             */
    /* -------------------------------------------------------------------- */
  if (Txfile != NULL)
  {
    fclose (Txfile);
    Txfile = NULL;

    /* ------------------------------------------------------------------- */
    /* If xfer completed, do post-xfer cleanup                             */
    /* ------------------------------------------------------------------- */
    if (xfer_flag == GOOD_XFER)
    {
      /* ---------------------------------------------------------------- */
      /* Perform post-xfer file massaging if neccessary                   */
      /* ---------------------------------------------------------------- */
      switch (Do_after)
      {
      case DELETE_AFTER:
      case SHOW_DELETE_AFTER:
        status_line (MSG_TXT (M_UNLINKING_MSG), Txfname);
        unlink (Txfname);
        break;

      case TRUNC_AFTER:
        status_line (MSG_TXT (M_TRUNC_MSG), Txfname);
        Txfile = fopen (Txfname, write_binary);
        if (Txfile == NULL)
          got_error (MSG_TXT (M_TRUNC_MSG), Txfname);
        else
          fclose (Txfile);
        Txfile = NULL;
        break;
      }

      /* ---------------------------------------------------------------- */
      /* If processing .?LO file, flag filename as sent (name[0] = '~')   */
      /* ---------------------------------------------------------------- */
    skipname:

      if (floflag)
      {
        curr_pos = ftell (flofile);
        if (curr_pos == -1L)
          got_error (MSG_TXT (M_SEEK_MSG), outboundname);

        if (fseek (flofile, floname_pos, SEEK_SET))
          got_error (MSG_TXT (M_SEEK_MSG), outboundname);

        if (fputc (Txfname[0] = '~', flofile) == EOF)
          got_error (MSG_TXT (M_WRITE_MSG), outboundname);

        if (fseek (flofile, curr_pos, SEEK_SET))
          got_error (MSG_TXT (M_SEEK_MSG), outboundname);
      }
    }
    else
    {
    abort:
      ++bad_xfers;
    }
  }

  /* ---------------------------------------------------------------------- */
  /* Find next file to be sent and build FNAMEPKT.  If reading .FLO-type    */
  /* file get next entry from it; otherwise check for next .OUT/.FLO file   */
  /* ---------------------------------------------------------------------- */

  if (EMSI_flag)
    holdname = HoldAreaNameMunge (&remote_akas[EMSI_aka]);
  else
    holdname = HoldAreaNameMunge (&called_addr);

next_aka:

  if (!floflag)
  {
    /* ------------------------------------------------------------------- */
    /* If first getfname() for this batch, init filename to .OUT           */
    /* ------------------------------------------------------------------- */
    if (!outboundname[0])
    {
      ADDRP addrp;

      if (!EMSI_flag)
        addrp = &called_addr;
      else
        addrp = &remote_akas[EMSI_aka];

      sprintf (outboundname, "%s%s.out", holdname, Hex_Addr_Str (addrp));

      *ext_flags = 'o';
    }
    /* ------------------------------------------------------------------- */
    /* Increment outbound filename until match found or all checked        */
    /* .OUT->.DUT->.CUT->.HUT->.FLO->.DLO->.CLO->.HLO->null name           */
    /* ------------------------------------------------------------------- */
    else
    {
    nxtout:
      p = strchr (outboundname, '\0') - 3;
      for (i = 0; i < NUM_FLAGS; ++i)
        if (ext_flags[i] == *p)
          break;
      if (i < NUM_FLAGS - 1)
      {
        *p = ext_flags[i + 1];
#ifndef JACK_DECKER
        if (isOriginator && *p == 'h')
          goto nxtout;
#endif
      }
      else
      {
        /* ------------------------------------------------------------- */
        /* Finished ?,D,C,H sequence; wrap .OUT->.FLO, or .FLO->done     */
        /* ------------------------------------------------------------- */
        if (!floflag)
        {
          *p++ = *ext_flags = 'f';
          *p++ = 'l';
          *p = 'o';
          ++floflag;
        }
        else
        {
          outboundname[0] = Txfname[0] = Txbuf[0] = Txbuf[1] = floflag = '\0';
          if (EMSI_flag && (++EMSI_aka < num_rakas))
          {
            holdname = HoldAreaNameMunge (&remote_akas[EMSI_aka]);
            status_line (MSG_TXT (M_EMSI_PROC_NODE),
                         Full_Addr_Str (&remote_akas[EMSI_aka]),
                         holdname);
            goto next_aka;
          }
        }
      }
    }

    /* ------------------------------------------------------------------- */
    /* Check potential outbound name; if file doesn't exist keep looking   */
    /* ------------------------------------------------------------------- */

    if (outboundname[0])
    {
      if (!dexists (outboundname))
        goto nxtout;
      if (floflag)
        goto rdflo;
      strcpy (Txfname, outboundname);
      /* ---------------------------------------------------------------- */
      /* Start FNAMEPKT using .PKT alias                                  */
      /* ---------------------------------------------------------------- */
      invent_pkt_name ((char *) Txbuf);
      Do_after = DELETE_AFTER;
    }
    /* ------------------------------------------------------------------- */
    /* Read and process next entry from .?LO-type file                     */
    /* ------------------------------------------------------------------- */
  }
  else
  {

  rdflo:
    /* ------------------------------------------------------------------- */
    /* Open .?LO file for processing if neccessary                         */
    /* ------------------------------------------------------------------- */
    if (!flofile)
    {
      bad_xfers = 0;
      flofile = share_fopen (outboundname, read_binary_plus, DENY_WRITE);
      if (flofile == (FILE *) NULL)
      {
        got_error (MSG_TXT (M_OPEN_MSG), outboundname);
        goto nxtout;
      }
    }

    floname_pos = ftell (flofile);
    if (floname_pos == -1L)
      got_error (MSG_TXT (M_SEEK_MSG), outboundname);

    if (fgets (p = Txfname, PATHLEN, flofile))
    {
      /* ---------------------------------------------------------------- */
      /* Got an attached file name; check for handling flags, fix up name */
      /* ---------------------------------------------------------------- */
      while (*p > ' ')
        ++p;
      *p = '\0';
      switch (Txfname[0])
      {
      case '\0':
      case '~':
      case ';':
        goto rdflo;

      case TRUNC_AFTER:
      case DELETE_AFTER:
      case SHOW_DELETE_AFTER:
        Do_after = Txfname[0];
        strcpy (Txfname, Txfname + 1);
        break;

      default:
        Do_after = NOTHING_AFTER;
        break;
      }

      /* ---------------------------------------------------------------- */
      /* Start FNAMEPKT with simple filename                              */
      /* ---------------------------------------------------------------- */
      while (p >= Txfname && *p != DIR_SEPC && *p != ':')
        --p;

      strcpy ((char *) Txbuf, ++p);
    }
    else
    {
      /* ---------------------------------------------------------------- */
      /* Finished reading this .?LO file; clean up and look for another   */
      /* ---------------------------------------------------------------- */
      fclose (flofile);
      flofile = NULL;
      if (!bad_xfers)
        unlink (outboundname);

      goto nxtout;
    }
  }

  /* ---------------------------------------------------------------------- */
  /* If we managed to find a valid file to transmit, open it, finish        */
  /* FNAMEPKT, and print nice message for the sysop.                        */
  /* ---------------------------------------------------------------------- */
  if (Txfname[0])
  {
    if (xfer_flag == ABORT_XFER)
      goto abort;

    status_line (MSG_TXT (M_SENDING), Txfname);
    Txfile = share_fopen (Txfname, read_binary, DENY_WRITE);
    if (Txfile == NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), Txfname);
      goto skipname;
    }

    if (isatty (fileno (Txfile)))  /* Check for character devices */
    {
      errno = 1;
      got_error (MSG_TXT (M_DEVICE_MSG), Txfname);
      fclose (Txfile);          /* return errors if it is the case */
      goto skipname;
    }

    unix_stat_noshift (Txfname, &f);
    sprintf (strchr ((char *) Txbuf, '\0') + 1, "%lu %lo %o", Txlen = f.st_size, f.st_mtime, f.st_mode);

    p = strchr (Txfname, '\0');
    while (p >= Txfname && *p != ':' && *p != DIR_SEPC)
      --p;

    strcpy (TxStats.fname, fname8p3 (++p));
    TxStats.FileLen = Txlen;
    css_upd ();

    throughput (0, 1, 0L);
  }
}

/*****************************************************************************/
/* Build and send a packet of any type.                                      */
/* Packet structure is: PKTSTRT,contents,packet_type,PKTEND,crc              */
/* CRC is computed from contents and packet_type only; if PKTSTRT or PKTEND  */
/* get munged we'll never even find the CRC.                                 */
/*****************************************************************************/
static void LOCALFUNC
sendpkt (register byte * buf, int len, int type)
{
  register word crc;

  if ((SharedCap & CANCRC32) && type != FNAMEPKT)
    sendpkt32 (buf, len, type);
  else
  {
    BUFFER_BYTE (DLE);
    BUFFER_BYTE (PKTSTRTCHR ^ 0x40);

    crc = 0;
    while (--len >= 0)
    {
      txbyte (*buf);
      crc = xcrc (crc, ((word) (*buf++)));
    }

    BUFFER_BYTE ((byte) type);
    crc = xcrc (crc, type);

    BUFFER_BYTE (DLE);
    BUFFER_BYTE (PKTENDCHR ^ 0x40);

    txbyte ((byte) (crc >> 8));
    txbyte ((byte) (crc & 0xFF));

    UNBUFFER_BYTES ();
  }
}

/*****************************************************************************/
/* Build and send a packet using 32-bit CRC; same as sendpkt in other ways   */
/*****************************************************************************/
static void LOCALFUNC
sendpkt32 (register byte * buf, register int len, int type)
{
  unsigned long crc32;

  BUFFER_BYTE (DLE);
  BUFFER_BYTE (PKTSTRTCHR32 ^ 0x40);

  crc32 = 0xFFFFFFFFUL;
  while (--len >= 0)
  {
    txbyte (*buf);
    crc32 = Z_32UpdateCRC (((word) * buf), crc32);
    ++buf;
  }

  BUFFER_BYTE ((byte) type);
  crc32 = Z_32UpdateCRC (type, crc32);

  BUFFER_BYTE (DLE);
  BUFFER_BYTE (PKTENDCHR ^ 0x40);

  txbyte ((byte) (crc32 >> 24));
  txbyte ((byte) ((crc32 >> 16) & 0xFF));
  txbyte ((byte) ((crc32 >> 8) & 0xFF));
  txbyte ((byte) (crc32 & 0xFF));

  UNBUFFER_BYTES ();
}

/*****************************************************************************/
/* Transmit cooked escaped byte(s) corresponding to raw input byte.  Escape  */
/* DLE, XON, and XOFF using DLE prefix byte and ^ 0x40. Also escape          */
/* CR-after-'@' to avoid Telenet/PC-Pursuit problems.                        */
/*****************************************************************************/
static void LOCALFUNC
txbyte (register byte c)
{
  static byte lastsent;

  switch (c)
  {
  case CR:
    if (lastsent != '@')
      goto sendit;
    /* fallthrough */

  case DLE:
  case XON:
  case XOFF:
    BUFFER_BYTE (DLE);
    c ^= 0x40;
    /* fallthrough */

  default:

  sendit:

    BUFFER_BYTE (lastsent = c);
  }
}

/*****************************************************************************/
/* Process FNAMEPKT of file to be received.  Check for aborted-transfer      */
/* recovery and solve filename collisions.    Check for enough disk space.   */
/* Return initial file data position to start receiving at, or -1 if error   */
/* detected to abort file reception.  Set Rxfname, Rxlen, Rxfile.            */
/*****************************************************************************/
static long LOCALFUNC
procfname (void)
{
  register char *p;
  register char *openmode;
  char linebuf[128], *fileinfo, *badfname;
  long filestart, bytes;
  FILE *abortlog;
  struct stat f;
  int i;

  /* ---------------------------------------------------------------------- */
  /* Initialize for file reception                                          */
  /* ---------------------------------------------------------------------- */
  badfname = NULL;
  Rxfname[0] = Resume_WaZOO = 0;

  /* ---------------------------------------------------------------------- */
  /* Save info on WaZOO transfer in case of abort                           */
  /* ---------------------------------------------------------------------- */
  f_fancy_str (Rxbuf);
  strcpy (Resume_name, Rxbuf);
  fileinfo = strchr (Rxbuf, '\0') + 1;
  p = strchr (fileinfo, '\0') + 1;
  SharedCap = (byte) ((Rxblklen > p - Rxbuf) ? *p & OURCAP : 0);

  /* ---------------------------------------------------------------------- */
  /* If this is a null FNAMEPKT, return OK immediately                      */
  /* ---------------------------------------------------------------------- */
  if (!Rxbuf[0])
    return 0L;

  /* Find transmitted simple filename */
  p = Rxbuf + strlen ((char *) Rxbuf) - 1;

  while (p >= Rxbuf && *p != '\\' && *p != '/' && *p != ':')
    p--;

  strcpy (linebuf, ++p);
  strlwr (linebuf);
  p = check_netfile (linebuf);
  status_line ("#%s %s %s", MSG_TXT (M_RECEIVING), (p) ? p : " ", Rxbuf);

  /* ---------------------------------------------------------------------- */
  /* Extract and validate filesize                                          */
  /* ---------------------------------------------------------------------- */
  Rxlen = -1;
  Rxfiletime = 0;
  if (sscanf (fileinfo, "%ld %lo", &Rxlen, &Rxfiletime) < 1 || Rxlen < 0)
  {
    status_line (MSG_TXT (M_NO_LENGTH));
    return -1L;
  }

  sprintf (Resume_info, "%ld %lo", Rxlen, Rxfiletime);

  /* ---------------------------------------------------------------------- */
  /* Check if this is a failed WaZOO transfer which should be resumed       */
  /* ---------------------------------------------------------------------- */
  if (dexists (Abortlog_name))
  {
    abortlog = fopen (Abortlog_name, read_ascii);
    if (abortlog == (FILE *) NULL)
      got_error (MSG_TXT (M_OPEN_MSG), Abortlog_name);
    else
    {
      while (!feof (abortlog))
      {
        linebuf[0] = '\0';

        if (!fgets (p = linebuf, sizeof (linebuf), abortlog))
          break;

        while (*p >= ' ')
          ++p;

        *p = '\0';
        p = strchr (linebuf, ' ');
        *p = '\0';

        if (!stricmp (linebuf, Resume_name))
        {
          p = strchr ((badfname = ++p), ' ');
          *p = '\0';
          if (!stricmp (++p, Resume_info))
          {
            ++Resume_WaZOO;
            break;
          }
        }
      }

      fclose (abortlog);
    }
  }

  /* ---------------------------------------------------------------------- */
  /* Open either the old or a new file, as appropriate                      */
  /* ---------------------------------------------------------------------- */
  p = strchr (strcpy (Rxfname, CURRENT.sc_Inbound), '\0');

  if (Resume_WaZOO)
  {
    strcpy (p, badfname);
    if (dexists (Rxfname))
      openmode = read_binary_plus;
    else
      openmode = write_binary;
  }
  else
  {
    strcpy (p, Rxbuf);

    /* ------------------------------------------------------------------- */
    /* If the file already exists:                                         */
    /* 1) And the new file has the same time and size, skip it             */
    /* 2) And OVERWRITE is turned on, delete the old copy                  */
    /* 3) Else create a unique file name in which to store new data        */
    /* ------------------------------------------------------------------- */
    if (unix_stat_noshift (Rxfname, &f) != -1)
    {                           /* If file already exists...      */
      if (Rxlen == f.st_size && (time_t) Rxfiletime == f.st_mtime)
      {
        status_line (MSG_TXT (M_ALREADY_HAVE), Rxfname);
        return -1L;
      }

      i = strlen (Rxfname) - 1;
      if ((!overwrite) || (is_arcmail (Rxfname, i)))
      {
        unique_name (Rxfname);
        status_line (MSG_TXT (M_RENAME_MSG), Rxfname);
      }
      else
        unlink (Rxfname);
    }

    openmode = write_binary;
  }

  Rxfile = fopen (Rxfname, openmode);
  if (Rxfile == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), Rxfname);
    return -1L;
  }

  if (isatty (fileno (Rxfile))) /* Check for character devices */
  {
    errno = 1;
    got_error (MSG_TXT (M_DEVICE_MSG), Rxfname);
    fclose (Rxfile);            /* Return errors if it is the case */
    return -1L;
  }

  /* ---------------------------------------------------------------------- */
  /* Determine initial file data position                                   */
  /* ---------------------------------------------------------------------- */
  if (Resume_WaZOO)
  {
    unix_stat_noshift (Rxfname, &f);  /* maybe unix_stat also OK */
    status_line (MSG_TXT (M_SYNCHRONIZING), filestart = f.st_size);
    p = Rxbuf;

    if (fseek (Rxfile, filestart, SEEK_SET))
    {
      got_error (MSG_TXT (M_SEEK_MSG), Rxfname);
      fclose (Rxfile);
      return -1L;
    }
  }
  else
    filestart = 0L;

  /* ---------------------------------------------------------------------- */
  /* Check for enough disk space                                            */
  /* ---------------------------------------------------------------------- */
  bytes = Rxlen - filestart + 10240;
  if (bytes / 64 > Diskavail)
  {
    status_line (MSG_TXT (M_OUT_OF_DISK_SPACE));
    fclose (Rxfile);
    return -1L;
  }

  /* ---------------------------------------------------------------------- */
  /* Print status message for the sysop                                     */
  /* ---------------------------------------------------------------------- */

  strcpy (RxStats.fname, fname8p3 (p));
  RxStats.FileLen = Rxlen;
  css_upd ();

  throughput (0, 0, 0L);

  return filestart;
}

/*****************************************************************************/
/* Receive, validate, and extract a packet if available.  If a complete      */
/* packet hasn't been received yet, receive and store as much of the next    */
/* packet as possible.    Each call to rcvpkt() will continue accumulating a */
/* packet until a complete packet has been received or an error is detected. */
/* Rxbuf must not be modified between calls to rcvpkt() if NOPKT is returned. */
/* Returns type of packet received, NOPKT, or BADPKT.  Sets Rxblklen.        */
/*****************************************************************************/
static byte LOCALFUNC
rcvpkt ()
{
  static byte rxcrc32;
  static word crc;
  static unsigned long crc32;
  register byte *p;
  register int c;
  int i;
  unsigned long pktcrc;

  /* ---------------------------------------------------------------------- */
  /* Abort transfer if operator pressed ESC                                 */
  /* ---------------------------------------------------------------------- */

  if (got_ESC ())
  {
    status_line (GenericError, MSG_TXT (M_KBD_MSG));
    return HALTPKT;
  }

  /* ---------------------------------------------------------------------- */
  /* If not accumulating packet yet, find start of next packet              */
  /* ---------------------------------------------------------------------- */
  WaitFlag = FALSE;
  p = Rxbufptr;
  if (!p)
  {
    do
      c = rxbyte ();
    while (c >= 0 || c == PKTEND);

    switch (c)
    {
    case PKTSTRT:
      rxcrc32 = FALSE;
      p = (byte *) Rxbuf;
      crc = 0;
      break;

    case PKTSTRT32:
      rxcrc32 = TRUE;
      p = (byte *) Rxbuf;
      crc32 = 0xFFFFFFFFUL;
      break;

    case NOCARRIER:
      status_line (GenericError, &(MSG_TXT (M_NO_CARRIER)[1]));
      return HALTPKT;

    default:
      return NOPKT;
    }
  }

  /* ---------------------------------------------------------------------- */
  /* Accumulate packet data until we empty buffer or find packet delimiter  */
  /* ---------------------------------------------------------------------- */
  if (rxcrc32)
  {
    while ((c = rxbyte ()) >= 0 && p < Rxbufmax)
    {
      *p++ = (byte) c;
      crc32 = Z_32UpdateCRC (c, crc32);
    }
  }
  else
  {
    while ((c = rxbyte ()) >= 0 && p < Rxbufmax)
    {
      *p++ = (byte) c;
      crc = xcrc (crc, c);
    }
  }

  /* ---------------------------------------------------------------------- */
  /* Handle whichever end-of-packet condition occurred                      */
  /* ---------------------------------------------------------------------- */
  switch (c)
  {
    /* ------------------------------------------------------------------- */
    /* PKTEND found; verify valid CRC                                      */
    /* ------------------------------------------------------------------- */
  case PKTEND:
    WaitFlag = TRUE;
    pktcrc = 0;
    for (i = (rxcrc32) ? 4 : 2; i; --i)
    {
      if ((c = rxbyte ()) < 0)
        break;
      pktcrc = (pktcrc << 8) | c;
    }

    if (!i)
    {
      if ((rxcrc32 && pktcrc == crc32) || pktcrc == crc)
      {
        /* ---------------------------------------------------------- */
        /* Good packet verified; compute packet data length and       */
        /* return packet type                                         */
        /* ---------------------------------------------------------- */
        Rxbufptr = NULL;
        Rxblklen = (int) (--p - (byte *) Rxbuf);
        return *p;
      }
    }
    /* fallthrough */

    /* ------------------------------------------------------------------- */
    /* Bad CRC, carrier lost, or buffer overflow from munged PKTEND        */
    /* ------------------------------------------------------------------- */
  default:
    if (c == NOCARRIER)
    {
      status_line (GenericError, &(MSG_TXT (M_NO_CARRIER)[1]));
      return HALTPKT;
    }
    else
    {
      Rxbufptr = NULL;
      return BADPKT;
    }

    /* ------------------------------------------------------------------- */
    /* Emptied buffer; save partial packet and let sender do something     */
    /* ------------------------------------------------------------------- */
  case BUFEMPTY:
    time_release ();            /* Also give other tasks a chance */
    Rxbufptr = p;
    return NOPKT;

    /* ------------------------------------------------------------------- */
    /* PKTEND was trashed; discard partial packet and prep for next one    */
    /* ------------------------------------------------------------------- */
  case PKTSTRT:
    rxcrc32 = FALSE;
    Rxbufptr = (byte *) Rxbuf;
    crc = 0;
    return BADPKT;

  case PKTSTRT32:
    rxcrc32 = TRUE;
    Rxbufptr = (byte *) Rxbuf;
    crc32 = 0xFFFFFFFFUL;
    return BADPKT;
  }
}

/*****************************************************************************/
/* Close file being received and perform post-reception aborted-transfer     */
/* recovery cleanup if neccessary.                                           */
/*****************************************************************************/
static void LOCALFUNC
rxclose (word xfer_flag)
{
  register char *p;
  char namebuf[PATHLEN], linebuf[128];
  byte c;
  FILE *abortlog, *newlog;
  struct utimbuf utimes;

  /* ---------------------------------------------------------------------- */
  /* Close file we've been receiving                                        */
  /* ---------------------------------------------------------------------- */
  fclose (Rxfile);
  if (Rxfiletime > 0)           /* utime doesn't like negative numbers */
  {
    utimes.UT_ACTIME = utimes.modtime = Rxfiletime;
    unix_utime_noshift (Rxfname, (UTIMBUF *) & utimes);
  }

  /* ---------------------------------------------------------------------- */
  /* If we completed a previously-aborted transfer, kill log entry & rename */
  /* ---------------------------------------------------------------------- */
  if (xfer_flag == GOOD_XFER && Resume_WaZOO)
  {
    abortlog = fopen (Abortlog_name, read_ascii);
    if (abortlog == (FILE *) NULL)
      got_error (MSG_TXT (M_OPEN_MSG), Abortlog_name);
    else
    {
      c = 0;
      strcpy (strchr (strcpy (namebuf, Abortlog_name), '\0') - 1, "TMP");
      newlog = fopen (namebuf, write_ascii);

      if (newlog == (FILE *) NULL)
      {
        got_error (MSG_TXT (M_OPEN_MSG), namebuf);
        fclose (abortlog);
      }
      else
      {
        while (!feof (abortlog))
        {
          linebuf[0] = '\0';
          if (!fgets (p = linebuf, sizeof (linebuf), abortlog))
            break;
          while (*p > ' ')
            ++p;
          *p = '\0';

          if (stricmp (linebuf, Resume_name))
          {
            *p = ' ';
            fputs (linebuf, newlog);
            if (ferror (newlog))
            {
              got_error (MSG_TXT (M_WRITE_MSG), namebuf);
              break;
            }

            ++c;
          }
        }

        fclose (abortlog);
        fclose (newlog);
        unlink (Abortlog_name);

        if (c)
        {
          if (rename (namebuf, Abortlog_name))
            got_error (MSG_TXT (M_RENAME_MSG), namebuf);
        }
        else
          unlink (namebuf);
      }
    }

    status_line (MSG_TXT (M_FINISHED_PART), Resume_name);
    unique_name (strcat (strcpy (namebuf, CURRENT.sc_Inbound), Resume_name));

    if (rename (Rxfname, namebuf))
      got_error (MSG_TXT (M_RENAME_MSG), Rxfname);
    else
      strcpy (Rxfname, namebuf);
  }
  else if (xfer_flag == FAILED_XFER && !Resume_WaZOO)
  {
    /* --------------------------------------------------------------------- */
    /* If transfer failed and was not an attempted resumption, log for later */
    /* --------------------------------------------------------------------- */

    status_line (MSG_TXT (M_SAVING_PART), Rxfname);
    unique_name (strcat (strcpy (namebuf, CURRENT.sc_Inbound), "BadWaZOO.001"));

    if (rename (Rxfname, namebuf))
      got_error (MSG_TXT (M_RENAME_MSG), Rxfname);

    abortlog = fopen (Abortlog_name, append_ascii);
    if (abortlog == (FILE *) NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), Abortlog_name);
      unlink (namebuf);
    }
    else
    {
      fprintf (abortlog, "%s %s %s\n", Resume_name, namebuf + strlen (CURRENT.sc_Inbound), Resume_info);
      if (ferror (abortlog))
        got_error (MSG_TXT (M_WRITE_MSG), Abortlog_name);
      fclose (abortlog);
    }
  }
}

/*****************************************************************************/
/* Try REAL HARD to disengage batch session cleanly                          */
/*****************************************************************************/
static void LOCALFUNC
endbatch (void)
{
  register int done, timeouts;
  long timeval, brain_dead;

  /* ---------------------------------------------------------------------- */
  /* Tell the other end to halt if it hasn't already                        */
  /* ---------------------------------------------------------------------- */
  done = timeouts = 0;
  long_set_timer (&brain_dead, 120);
  sendpkt (NULL, 0, HALTPKT);
  long_set_timer (&timeval, TimeoutSecs);

  /* ---------------------------------------------------------------------- */
  /* Wait for the other end to acknowledge that it's halting                */
  /* ---------------------------------------------------------------------- */
  while (!done)
  {
    if (long_time_gone (&brain_dead))
      break;

    switch (rcvpkt ())
    {
    case NOPKT:
    case BADPKT:
      if (long_time_gone (&timeval))
      {
        if (++timeouts > 2)
          ++done;
        else
          goto reject;
      }
      break;

    case HALTPKT:
    case HALTACKPKT:
      ++done;
      break;

    default:
      timeouts = 0;
    reject:
      if (!CARRIER)
        return;

      sendpkt (NULL, 0, HALTPKT);
      long_set_timer (&timeval, TimeoutSecs);
      break;
    }
  }

  /* ---------------------------------------------------------------------- */
  /* Announce quite insistently that we're done now                         */
  /* ---------------------------------------------------------------------- */
  for (done = 0; done < 10; ++done)
    sendpkt (NULL, 0, HALTACKPKT);

  while (!OUT_EMPTY () && CARRIER)
    time_release ();
}

/*****************************************************************************/
/* Compute future timehack for later reference                               */
/*****************************************************************************/
static void LOCALFUNC
long_set_timer (long *Buffer, unsigned int Duration)
{

  unix_time ((time_t *) Buffer);
  *Buffer += (long) Duration;
}

/*****************************************************************************/
/* Return TRUE if timehack has been passed, FALSE if not                     */
/*****************************************************************************/
static int LOCALFUNC
long_time_gone (long *TimePtr)
{

  return (unix_time (NULL) > (time_t) * TimePtr);
}

/*****************************************************************************/
/* Receive cooked escaped byte translated to avoid various problems.         */
/* Returns raw byte, BUFEMPTY, PKTSTRT, PKTEND, or NOCARRIER.                */
/*****************************************************************************/
static int LOCALFUNC
rxbyte (void)
{
  register int c, w;

  if ((c = rcvrawbyte ()) == DLE)
  {
    w = WaitFlag++;
    if ((c = rcvrawbyte ()) >= 0)
    {
      switch (c ^= 0x40)
      {
      case PKTSTRTCHR:
        c = PKTSTRT;
        break;

      case PKTSTRTCHR32:
        c = PKTSTRT32;
        break;

      case PKTENDCHR:
        c = PKTEND;
        break;
      }
    }

    WaitFlag = (byte) w;
  }

  return c;
}

/*****************************************************************************/
/* Receive raw non-escaped byte.  Returns byte, BUFEMPTY, or NOCARRIER.      */
/* If waitflag is true, will wait for a byte for Timeoutsecs; otherwise      */
/* will return BUFEMPTY if a byte isn't ready and waiting in inbound buffer. */
/*****************************************************************************/
static int LOCALFUNC
rcvrawbyte (void)
{
  long timeval;

  if ((int) PEEKBYTE () >= 0)
    return MODEM_IN ();

  if (!CARRIER)
    return NOCARRIER;

  if (!WaitFlag)
  {
    time_release ();            /* TJW 970526 anti-cpu-hogging ! */
    return BUFEMPTY;
  }

  timeval = unix_time (NULL) + TimeoutSecs;

  while ((int) PEEKBYTE () < 0)
  {
    if (!CARRIER)
      return NOCARRIER;
    if (unix_time (NULL) > (time_t) timeval)
      return BUFEMPTY;
    time_release ();
  }

  return MODEM_IN ();
}

#if 0                           /* replaced by throughput in misc.c ! */
/*****************************************************************************/
/* Compute and print throughput                                              */
/*****************************************************************************/
long
through (long *byts, long *strtd)
{
  long bytes = *byts;           /* TJW 960513 */
  long started = *strtd;        /* TJW 960513 */
  long stopped;                 /* TJW 960513 */
  long elapsed;                 /* TJW 960513 */
  long cps, lowcps, highcps;    /* TJW 960513 */
  long efficiency;              /* TJW 960513 */

  stopped = unix_time (NULL);
  /* The next line tests for day wrap without the date rolling over */
  if (stopped < started)
    stopped += 86400L;

  elapsed = stopped - started;
  if (elapsed == 0L)
    elapsed = 1L;

  cps = (long) (bytes / (unsigned long) elapsed);

  /* TJW 970407 changed to calculate with 8 bits/byte (sync) instead of */
  /* 10 bits/byte (async 8n1)                                           */
  efficiency = (cps * (8 * 100L)) / ((long) cur_baud.rate_value);

  lowcps = (long) (bytes / ((unsigned long) (stopped - started + 1)));
  if (stopped - started > 1)
    highcps = (long) (bytes / ((unsigned long) (stopped - started - 1)));
  else
    highcps = cps;
  status_line (MSG_TXT (M_CPS_MESSAGE), cps, lowcps, highcps, bytes, efficiency);
  return elapsed;
}
#endif

/*****************************************************************************/
/* Get next file to request, if any                                          */
/*****************************************************************************/
static int LOCALFUNC
get_filereq (byte req_started)
{
  char reqname[PATHLEN], linebuf[128], *holdname;
  register char *p;
  int gotone = FALSE;
  FILE *reqfile;

next_aka:

  if (!EMSI_flag)
  {
    holdname = HoldAreaNameMunge (&called_addr);
    sprintf (reqname, "%s%s.req", holdname, Hex_Addr_Str (&called_addr));
  }
  else
  {
    holdname = HoldAreaNameMunge (&remote_akas[EMSI_raka]);
    sprintf (reqname, "%s%s.req", holdname, Hex_Addr_Str (&remote_akas[EMSI_raka]));
  }

  if (req_started)
    mark_done (reqname);

  if (dexists (reqname))
  {
    if (!(remote_capabilities & WZ_FREQ))
      status_line (MSG_TXT (M_FREQ_DECLINED));
    else if (!(SharedCap & CANFREQ))
      status_line (MSG_TXT (M_REMOTE_CANT_FREQ));
    else
    {
      cleanup_req (reqname);    /* TJW 961028 remove $xxxxx size info */
      reqfile = fopen (reqname, read_ascii);
      if (reqfile == (FILE *) NULL)
        got_error (MSG_TXT (M_OPEN_MSG), reqname);
      else
      {
        while (!feof (reqfile))
        {
          linebuf[0] = '\0';
          if (!fgets (p = linebuf, sizeof (linebuf), reqfile))
            break;
          while (*p >= ' ')
            ++p;
          *p = '\0';
          if (linebuf[0] != ';')
          {
            strcpy (Rxbuf, linebuf);
            *(strchr (Rxbuf, '\0') + 1) = SharedCap;
            gotone = TRUE;
            break;
          }
        }

        fclose (reqfile);
        if (!gotone)
          unlink (reqname);
      }
    }
  }

  if (!gotone && EMSI_flag)
  {
    if (++EMSI_raka < num_rakas)
      goto next_aka;
  }

  return gotone;
}

/*****************************************************************************/
/* Record names of files to send in response to file request; callback       */
/* routine for respond_to_file_requests()                                    */
/*****************************************************************************/
static int
record_reqfile (char *fname)
{
  FILE *pFileT;

  pFileT = fopen (ReqTmp, append_ascii);
  if (pFileT == (FILE *) NULL)
    got_error (MSG_TXT (M_OPEN_MSG), ReqTmp);
  else
  {
    fputs (fname, pFileT);
    fputs ("\n", pFileT);

    if (ferror (pFileT))
      got_error (MSG_TXT (M_WRITE_MSG), ReqTmp);

    fclose (pFileT);
    ++ReqRecorded;
    return TRUE;
  }

  return FALSE;
}

/*****************************************************************************/
/* Estimate transfer time for requested file(s); callback                    */
/* routine for respond_to_file_requests()                                    */
/*****************************************************************************/
static int
timeof_reqfile (long filesize)
{
  long i;

  i = remaining (filesize, JANUS_EFFICIENCY);

  if (i > 65535)
    i = 65535;

  /* Since actual transfers don't occur while in file request code, we have
   * to "reverse engineer" the testing for end-time. */

  if (CURRENT.time_Limit != 0)
  {
    if (((long) unix_time (NULL) + i - freq_accum.time) > CURRENT.time_Limit)
      return (int) i;

    freq_accum.time -= (int) i; /* Add time to (now-start) interval */
    return 0;
  }
  return (int) i;
}

/*****************************************************************************/
/* Get next file which was requested, if any                                 */
/*****************************************************************************/
static byte LOCALFUNC
get_reqname (byte first_req)
{
  register char *p;
  byte gotone = FALSE;
  FILE *pFileT;
  struct stat f;

  if (!first_req)
  {
    fclose (Txfile);
    Txfile = NULL;
    mark_done (ReqTmp);
  }

  if (dexists (ReqTmp))
  {
    pFileT = fopen (ReqTmp, read_ascii);
    if (pFileT == (FILE *) NULL)
      got_error (MSG_TXT (M_OPEN_MSG), ReqTmp);
    else
    {
      while (!feof (pFileT))
      {
        Txfname[0] = '\0';
        if (!fgets (p = Txfname, PATHLEN, pFileT))
          break;

        while (*p >= ' ')
          ++p;
        *p = '\0';

        if (Txfname[0] != ';')
        {
          status_line (MSG_TXT (M_SENDING), Txfname);
          Txfile = share_fopen (Txfname, read_binary, DENY_WRITE);
          if (Txfile == NULL)
          {
            got_error (MSG_TXT (M_OPEN_MSG), Txfname);
            continue;
          }

          while (p >= Txfname && *p != DIR_SEPC && *p != ':')
            --p;

          strcpy ((char *) Txbuf, ++p);
          unix_stat_noshift (Txfname, &f);
          sprintf (strchr ((char *) Txbuf, '\0') + 1, "%lu %lo %o", Txlen = f.st_size, f.st_mtime, f.st_mode);

          strcpy (TxStats.fname, fname8p3 (p));
          TxStats.FileLen = Txlen;
          css_upd ();
          throughput (0, 1, 0L);
          gotone = TRUE;
          break;
        }
      }

      fclose (pFileT);
      if (!gotone)
        unlink (ReqTmp);
    }
  }

  return gotone;
}

/*****************************************************************************/
/* Mark first unmarked line of file as done (comment it out)                 */
/*****************************************************************************/
static void LOCALFUNC
mark_done (char *fname)
{
  char linebuf[128];
  FILE *fp;
  long pos;

  if (dexists (fname))
  {
    fp = fopen (fname, read_binary_plus);
    if (fp == (FILE *) NULL)
      got_error (MSG_TXT (M_OPEN_MSG), fname);
    else
    {
      while (!feof (fp))
      {
        pos = ftell (fp);
        if (pos == -1L)
          got_error (MSG_TXT (M_SEEK_MSG), fname);

        if (!fgets (linebuf, sizeof (linebuf), fp))
          break;

        if (linebuf[0] != ';')
        {
          if (fseek (fp, pos, SEEK_SET))
            got_error (MSG_TXT (M_SEEK_MSG), fname);

          fputc (';', fp);
          if (ferror (fp))
            got_error (MSG_TXT (M_WRITE_MSG), fname);

          break;
        }
      }

      fclose (fp);
    }
  }
}

/* $Id: janus.c,v 1.11 1999/04/01 00:52:11 mr Exp $ */
