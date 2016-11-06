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
 * Filename    : $Source: E:/cvs/btxe/src/ftsc.c,v $
 * Revision    : $Revision: 1.9 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:26 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm FTSC Mail Session Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int FTSC_callback (char *);
int FTSC_time (long);
static int LOCALFUNC FTSC_sendmail (void);
static int LOCALFUNC FTSC_recvmail (int);

static long NoSealink;


MISSION_ERR
FTSC_sender (int wz)
{
  int j;
  long freeReq;
  int done;
  long t1;
  MISSION_ERR error = mission_ok;

  XON_DISABLE ();
  first_block = 0;

  if (!wz)
  {
    status_line (MSG_TXT (M_SEND_FALLBACK));
    first_block = 1;
    who_is_he = 0;
    status_line ("*%s (%s)",
                 newnodedes.SystemName, Full_Addr_Str (&remote_addr));
  }

  freeReq = QueryNodeFlag (on_our_nickel, FREEREQ, &remote_addr);  /* r. hoerner */
  Netmail_Session = 1;

  if (FTSC_sendmail ())
    error = mission_error;

  t1 = timerset (45 * PER_SECOND);
  done = 0;

  /* See what the receiver would like us to do */

  while ((!timeup (t1)) && CARRIER && !done)
  {
    if ((j = PEEKBYTE ()) >= 0)
    {
      CLEAR_INBOUND ();

      switch (j)
      {
      case TSYNC:
        if (FTSC_recvmail (1))
        {
          error = mission_error;
          done = 1;
        }
        else
          t1 = timerset (45 * PER_SECOND);
        break;

      case SYN:
        if (freeReq)
          SEA_recvreq ();
        else
        {
          SENDBYTE (CAN);
          status_line (MSG_TXT (M_REFUSING_IN_FREQ));
        }
        t1 = timerset (45 * PER_SECOND);
        break;

      case ENQ:
        SEA_sendreq ();
        done = 1;
        break;

      case NAK:
      case 'C':
        SENDBYTE (EOT);
        t1 = timerset (45 * PER_SECOND);
        break;

      default:
        SENDBYTE (SUB);
        break;
      }
    }
    else
    {
      time_release ();
    }
  }

  if (!CARRIER)
  {
    status_line (MSG_TXT (M_NO_CARRIER));
    CLEAR_INBOUND ();
    first_block = 0;
  }
  else
  {
    if (timeup (t1) && !done)
    {
      if (FTSC_recvmail (1))
        error = mission_error;
      status_line (MSG_TXT (M_TOO_LONG));
    }

    first_block = 0;
    t1 = timerset (PER_SECOND);

    while (!timeup (t1))
      time_release ();

    if (!wz)
      status_line (MSG_TXT (M_0001_END));
  }

  return error;
}


MISSION_ERR
FTSC_receiver (int wz)
{
  int havemail;
  long done;
  unsigned int i;
  long t1, t2;
  struct stat buf;
  ADDR tmp;
  AKA *aka;
  char fname[PATHLEN];
  char *HoldName;
  struct FILEINFO dta;
  MISSION_ERR error = mission_ok;

  first_block = 0;
  XON_DISABLE ();

  if (!wz)
  {
    first_block = 1;
    status_line (MSG_TXT (M_RECV_FALLBACK));
    who_is_he = 1;
  }

  Netmail_Session = 1;

  CLEAR_INBOUND ();

  /* Save the state of pickup for now */

  done = QueryNodeFlag (no_pickup, NOPICK, &called_addr);  /*r. hoerner */
  no_pickup = 0;

  if (FTSC_recvmail (0))
  {
    /* Restore the state of pickup */

    no_pickup = done;
    first_block = 0;
    error = mission_error;

    if (!wz)
      status_line (MSG_TXT (M_0001_END));
  }
  else
  {

    /* Restore the state of pickup */

    no_pickup = done;
    remote_addr = called_addr;

    show_session (MSG_TXT (M_MCP_MAIL_XFER), &remote_addr);

    HoldName = HoldAreaNameMunge (&called_addr);

    /* Now see if we should send anything back to him */

    sprintf (fname, "%s%s.?ut", HoldName, Hex_Addr_Str (&remote_addr));
    havemail = !dfind (&dta, fname, 0);

    if (!havemail)
    {
      sprintf (fname, "%s%s.?lo", HoldName, Hex_Addr_Str (&remote_addr));
      havemail = !dfind (&dta, fname, 0);
    }

    if (!havemail)
    {
      for (aka = alias; aka != NULL; aka = aka->next)
      {
        sprintf (fname, "%s%s.req", CURRENT.sc_Inbound, Hex_Addr_Str ((ADDR *) aka));
        havemail = !dfind (&dta, fname, 0);
        if (havemail)
          break;
      }
    }

    if (!havemail)
    {
      status_line (MSG_TXT (M_NOTHING_TO_SEND), Full_Addr_Str (&remote_addr));
    }
    else
    {
      /* Release any resource involved in finding the mail */

      dfind (&dta, NULL, 2);
      status_line (MSG_TXT (M_GIVING_MAIL), Full_Addr_Str (&remote_addr));

      /* Send the TSYNC's until we get a C or NAK or CAN back */

      t1 = timerset (30 * PER_SECOND);  /* set 30 second timeout */
      done = 0;

      while (!timeup (t1) && CARRIER && !done)  /* till then or CD lost  */
      {
        SENDBYTE (TSYNC);

        t2 = timerset (3 * PER_SECOND);
        while (CARRIER && (!timeup (t2)) && !done)
        {
          switch (TIMED_READ (0))
          {
          case 'C':
          case NAK:
            done = 1;
            if (FTSC_sendmail ())
              error = mission_error;
            break;

          case CAN:
            done = 1;
            status_line (MSG_TXT (M_REFUSE_PICKUP), Full_Addr_Str (&remote_addr));
            break;

          default:
            time_release ();
          }
        }
      }
    }

    first_block = 0;

    if (!wz)                    /* All done if this is WaZOO */
    {

      /* Now see if we want to request anything */

      tmp = remote_addr;

      /* For a point, massage the address to get the right .REQ filename */

      if (tmp.Point != 0)
      {
        tmp.Node = tmp.Point;
        tmp.Point = 0;
        tmp.Net = (pvtnet > 0) ? (unsigned int) pvtnet : 0;
      }

      sprintf (fname, "%s%s.req", HoldName, Hex_Addr_Str (&tmp));
      if (!unix_stat (fname, &buf))
      {
        /* Send the SYN character and wait for an ENQ or CAN */

        t1 = timerset (30 * PER_SECOND);  /* set 30 second timeout */
        done = 0;

        while (!timeup (t1) && CARRIER && !done)  /* till then or CD lost  */
        {
          SENDBYTE (SYN);

          t2 = timerset (5 * PER_SECOND);
          while (CARRIER && (!timeup (t2)) && !done)
          {
            i = (unsigned) TIMED_READ (0);

            switch (i)
            {
            case ENQ:
              SEA_sendreq ();
              break;

            case CAN:
              done = 1;
              break;

            case 'C':
            case NAK:
              SENDBYTE (EOT);
              break;

            case SUB:
              SENDBYTE (SYN);
              break;

            default:
              time_release ();
            }
          }
        }
      }

      /* Finally, can he request anything from us */

      if (!QueryNodeFlag (no_requests, NOREQ, &remote_addr))  /* r. hoerner */
        SEA_recvreq ();

      status_line (MSG_TXT (M_0001_END));
    }
  }

  return error;
}


static int LOCALFUNC
FTSC_sendmail ()
{
  FILE *fp;
  char fname[PATHLEN];
  char s[PATHLEN];
  char *sptr;
  char *HoldName;
  int c;
  int i;
  struct stat buf;
  struct _pkthdr *tmppkt;
  time_t t1;
  struct tm *tm1;
  AKA *aka;

  XON_DISABLE ();

  if (alias == NULL)
    return 1;

  n_getpassword (&called_addr); /* Update "assumed" */

  for (aka = alias, i = 0; (i < assumed) && (aka != NULL); aka = aka->next, i++) ;

  if (aka == NULL)
    aka = alias;

  sptr = s;

  NoSealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);

  /* r. hoerner */
  /* ------------------------------------------------------------------ */
  /* Send all waiting ?UT files (mail packets)                          */
  /* ------------------------------------------------------------------ */

  *ext_flags = 'o';
  HoldName = HoldAreaNameMunge (&called_addr);
  for (c = 0; c < NUM_FLAGS; c++)
  {
#ifndef JACK_DECKER
    if ((caller && !SendHoldIfCaller) && (ext_flags[c] == 'h'))  /* VRP 990824 */
      continue;
#endif
    sprintf (fname, "%s%s.%cut",
             HoldName, Hex_Addr_Str (&called_addr), ext_flags[c]);

    if (!unix_stat (fname, &buf))
      break;
  }                             /* for */

  /* -- Build a dummy PKT file name -- */

  invent_pkt_name (s);

  status_line (MSG_TXT (M_PACKET_MSG));

  if (c == NUM_FLAGS)
  {
    sprintf (fname, "%s%s.out",
             HoldName, Hex_Addr_Str (&called_addr));
    if ((fp = fopen (fname, write_binary)) == NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), fname);
      return (1);
    }

    t1 = unix_time (NULL);
    tm1 = unix_localtime (&t1);

    tmppkt = (struct _pkthdr *) calloc (1, sizeof (struct _pkthdr));

    if (tmppkt == NULL)
    {
      status_line (MSG_TXT (M_MEM_ERROR));
      fclose (fp);
      return (1);
    }

    tmppkt->orig_node = (int) aka->Node;
    tmppkt->dest_node = called_addr.Node;
    tmppkt->ver = PKTVER;
    tmppkt->orig_net = (int) aka->Net;
    tmppkt->dest_net = called_addr.Net;
    tmppkt->product = PRDCT_CODE;
    tmppkt->orig_zone = (int) aka->Zone;
    tmppkt->dest_zone = called_addr.Zone;

    if (n_getpassword (&called_addr) > 0)
    {
      if (remote_password != NULL)
      {
        strupr (remote_password);
        strncpy ((char *) (tmppkt->password), remote_password, 8);
      }
    }

    if (((called_addr.Domain != NULL)
         && (called_addr.Domain != aka->Domain)
         && (my_addr.Domain != NULL))
        || (aka->Point != 0))
    {
      /* Make it a type 2.2 packet instead */
      tmppkt->year = aka->Point;
      tmppkt->month = called_addr.Point;
      tmppkt->day = 0;
      tmppkt->hour = 0;
      tmppkt->minute = 0;
      tmppkt->second = 0;
      tmppkt->rate = 2;
      if (aka->Domain != NULL)
      {
        for (i = 0; domain_name[i] != NULL; i++)
        {
          if (domain_name[i] == aka->Domain)
            break;
        }

        if (i < DOMAINS)
          strncpy ((char *) tmppkt->B_fill2, domain_abbrev[i], 8);
      }

      for (i = 0; (i < DOMAINS) && (domain_name[i] != NULL); i++)
      {
        if (domain_name[i] == called_addr.Domain)
          break;
      }

      if (i < DOMAINS)
      {
        if (domain_name[i] != NULL)
          strncpy ((char *) &(tmppkt->B_fill2[8]), domain_abbrev[i], 8);
      }
    }
    else
    {
      tmppkt->year = tm1->tm_year + ((tm1->tm_year < 70) ? 2000 : 1900);  /* TJW 971020 bugfix */
      /* AG 990120 Y2K fix */
      tmppkt->month = tm1->tm_mon;
      tmppkt->day = tm1->tm_mday;
      tmppkt->hour = tm1->tm_hour;
      tmppkt->minute = tm1->tm_min;
      tmppkt->second = tm1->tm_sec;
      tmppkt->rate = 0;
    }

    fwrite ((char *) tmppkt, sizeof (struct _pkthdr), 1, fp);

    free (tmppkt);
    fwrite ("\0\0", 2, 1, fp);
    fclose (fp);
  }
  else
  {
    if ((fp = fopen (fname, read_binary_plus)) == NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), fname);
      return (1);
    }

    tmppkt = (struct _pkthdr *) calloc (1, sizeof (struct _pkthdr));

    if (tmppkt == NULL)
    {
      status_line (MSG_TXT (M_MEM_ERROR));
      fclose (fp);
      return (1);
    }

    if (fread (tmppkt, 1, sizeof (struct _pkthdr), fp) < sizeof (struct _pkthdr))
    {
      got_error (MSG_TXT (M_READ_MSG), fname);
      free (tmppkt);
      fclose (fp);
      return (1);
    }

    if (n_getpassword (&called_addr) > 0)
    {
      if (remote_password != NULL)
      {
        strupr (remote_password);
        strncpy ((char *) (tmppkt->password), remote_password, 8);
      }
    }

    /* Make sure the zone info is in there */

    tmppkt->orig_node = (int) aka->Node;
    tmppkt->orig_net = (int) aka->Net;
    tmppkt->orig_zone = (int) aka->Zone;
    tmppkt->dest_zone = called_addr.Zone;

    if ((called_addr.Domain != NULL) &&
        (called_addr.Domain != aka->Domain) &&
        (my_addr.Domain != NULL))
    {
      /* Make it a type 2.2 packet instead */

      tmppkt->year = aka->Point;
      tmppkt->month = called_addr.Point;
      tmppkt->day = 0;
      tmppkt->hour = 0;
      tmppkt->minute = 0;
      tmppkt->second = 0;
      tmppkt->rate = 2;

      if (aka->Domain != NULL)
      {
        for (i = 0; domain_name[i] != NULL; i++)
        {
          if (domain_name[i] == aka->Domain)
            break;
        }

        if (i < DOMAINS)
          strncpy ((char *) tmppkt->B_fill2, domain_abbrev[i], 8);
      }

      for (i = 0; domain_name[i] != NULL; i++)
      {
        if (domain_name[i] == called_addr.Domain)
          break;
      }

      if (i < DOMAINS)
        strncpy ((char *) &(tmppkt->B_fill2[8]), domain_abbrev[i], 8);
    }

    fseek (fp, 0L, SEEK_SET);
    fwrite (tmppkt, 1, sizeof (struct _pkthdr), fp);

    fclose (fp);
    free (tmppkt);
  }

  if (NoSealink)
    i = (Telink_Send_File (fname, s) != FILE_SENT_OK);
  else
    i = (SEAlink_Send_File (fname, s) != FILE_SENT_OK);

  /* r. hoerner */
  if (i)
  {
    if (c == NUM_FLAGS)
      unlink (fname);
    return 1;
  }

  fsent++;

  /* Delete the sent packet */
  unlink (fname);

  /* ------------------------------------------------------------------ */
  /* Send files listed in ?LO files (attached files)                    */
  /* ------------------------------------------------------------------ */

  *ext_flags = 'f';
  status_line (" %s %s", MSG_TXT (M_OUTBOUND), MSG_TXT (M_FILE_ATTACHES));

  if (!do_FLOfile (ext_flags, FTSC_callback))
    return 1;

  /* ------------------------------------------------------------------ */
  /* Send our File requests to other system if it's a WaZOO             */
  /* ------------------------------------------------------------------ */

  if (requests_ok && remote_capabilities)
  {
    sprintf (fname, "%s%s.req", HoldName, Hex_Addr_Str (&called_addr));
    if (!unix_stat (fname, &buf))
    {
      if (!(((unsigned) remote_capabilities) & WZ_FREQ))
        status_line (MSG_TXT (M_FREQ_DECLINED));
      else
      {
        status_line (MSG_TXT (M_MAKING_FREQ));

        if (FTSC_callback (fname) == FILE_SENT_OK)
        {
          status_line ("!Deleting %s", fname);
          unlink (fname);
          fsent++;
        }
      }
    }
  }

  /* ------------------------------------------------------------------ */
  /* Process WaZOO file requests from other system                      */
  /* ------------------------------------------------------------------ */

  fsent += respond_to_file_requests (0, FTSC_callback, FTSC_time);

  /* Now close out the file attaches */
  sent_mail = 1;
  *sptr = 0;
  status_line (" %s %s %s", MSG_TXT (M_END_OF), MSG_TXT (M_OUTBOUND), MSG_TXT (M_FILE_ATTACHES));
  Batch_Send (NULL);
  return 0;
}

static int LOCALFUNC
FTSC_recvmail (int outbound_session)
{
  char fname[PATHLEN];
  char fname1[PATHLEN];
  char fname2[PATHLEN];
  struct _pkthdr tmppkt;
  FILE *fp, *fp1;
  int done;
  int i = 0;
  int j;
  int logit = TRUE;
  char *starting_inbound;
  NODESTRUC *np;

  status_line (MSG_TXT (M_RECV_MAIL));

  if (!CARRIER)
  {
    status_line (MSG_TXT (M_NO_CARRIER));
    CLEAR_INBOUND ();
    return (1);
  }

  XON_DISABLE ();

  done = 0;

  /* If we don't want to pickup stuff */

  if (QueryNodeFlag (no_pickup, NOPICK, &remote_addr))  /*r. hoerner */
  {
    status_line (MSG_TXT (M_NO_PICKUP));
    SENDBYTE (CAN);
  }
  else
  {
    status_line (" %s %s", MSG_TXT (M_INBOUND), MSG_TXT (M_MAIL_PACKET));

    /* Invent a dummy name for the packet */

    invent_pkt_name (fname1);

    /* Receive the packet with special netmail protocol */

    CLEAR_INBOUND ();

    starting_inbound = CURRENT.sc_Inbound;

    if (Xmodem_Receive_File (CURRENT.sc_Inbound, fname1) == FILE_RECV_OK)
      got_packet = 1;

    sprintf (fname, "%s%s", starting_inbound, fname1);

    /* Check the password if there is one */

    /* Extra Inbound Dir */
    if ((np = QueryNodeStruct (&remote_addr, 0)) != NULL)
      if (np->ExtraDirIn)
        strcpy (CURRENT.sc_Inbound, np->ExtraDirIn);

    if ((!remote_capabilities) && (!outbound_session))
      i = n_getpassword (&remote_addr);

    if (i < 0)
    {
      status_line (MSG_TXT (M_NUISANCE_CALLER));
      modem_hangup ();          /* Hang up right now      */
      goto bad_caller;
    }

    if (i != 0)
    {
      if (remote_password != NULL)
      {
        got_packet = 0;

        if ((fp = fopen (fname, read_binary_plus)) == NULL)
        {
          got_error (MSG_TXT (M_OPEN_MSG), fname);
          status_line (MSG_TXT (M_PWD_ERR_ASSUMED));
          return (1);
        }

        if (fread (&tmppkt, 1, sizeof (struct _pkthdr), fp) < sizeof (struct _pkthdr))
        {
          got_error (MSG_TXT (M_OPEN_MSG), fname);
          status_line (MSG_TXT (M_PWD_ERR_ASSUMED));
          fclose (fp);
          return (1);
        }

        fclose (fp);

        if (n_password (&remote_addr, (char *) (tmppkt.password), remote_password, &logit))
        {
        bad_caller:
          strcpy (fname1, fname);
          j = (int) strlen (fname) - 3;
          strcpy (&(fname[j]), "Bad");

          if (rename (fname1, fname))
            status_line (MSG_TXT (M_CANT_RENAME_MAIL), fname1);
          else
            status_line (MSG_TXT (M_MAIL_PACKET_RENAMED), fname);

          return (1);
        }
      }

      got_packet = 1;
    }

    /*
     *  See if things changed after the fact. If so, we want to move
     *  the mail packet from the non-secured directory into the
     *  secured one. This is slightly tricky. Start with a simple rename
     *  and if that doesn't work (it might not if we're spanning drives)
     *  do a simple copy/unlink.
     *
     *  Steal resources such as 'done' and 'bufp->_all' wherever that makes sense.
     *                                     ^----------^ TJW 960416
     */

    if (strcmp (starting_inbound, CURRENT.sc_Inbound) != 0)
    {
      strcpy (fname2, CURRENT.sc_Inbound);
      strcat (fname2, fname1);

      /*  Try the easy case first. A straight rename. */

      done = 1;

      if (rename (fname2, fname))
      {
        /*     If we get here, the straight rename didn't work. Let's
         *     do a copy. Use bufp->_all since while we are here, we're
         *     not doing any file transfers.
         */
        done = 0;               /* default is failure till files are open */

        if ((fp = fopen (fname, read_binary_plus)) == NULL)
        {
          got_error (MSG_TXT (M_OPEN_MSG), fname);
        }
        else if ((fp1 = fopen (fname2, write_binary)) == NULL)
        {
          fclose (fp);          /* CEH 971225 */
          got_error (MSG_TXT (M_OPEN_MSG), fname2);
        }
        else
        {
          /* Here both packets are open */
          /* Steal bufp->_all because nobody's using it now */

          done = 1;             /* default is success now */

          while ((j = fread (bufp->_all, 1, WAZOOMAX, fp)) > 0)
          {                     /* ^v TJW 960416 */
            if (fwrite (bufp->_all, j, 1, fp1) != 1)
            {
              got_error (MSG_TXT (M_WRITE_MSG), fname2);
              done = 0;         /* only possible failure = write err */
              break;
            }
          }

          fclose (fp1);
          fclose (fp);          /* CEH 971225 */

          if (done == 0)        /* Figure out which file to delete */
            unlink (fname2);
          else
            unlink (fname);
        }
      }

      if (done == 0)
        status_line (MSG_TXT (M_CANT_RENAME_MAIL), fname);
      else
        status_line (MSG_TXT (M_MAIL_PACKET_RENAMED), fname2);
    }

    /*
     * If this was an inbound session, we need to set up the
     * node flags for the node.
     */
    got_mail = got_packet;
    if (!outbound_session)
    {
      if (flag_file (TEST_AND_SET, &remote_addr, 1))
        return (1);
      called_addr = remote_addr;
    }

    done = 0;

    /* Now receive the files if possible */

    status_line (" %s %s", MSG_TXT (M_INBOUND), MSG_TXT (M_FILE_ATTACHES));
    done = (Batch_Receive (CURRENT.sc_Inbound) != FILE_RECV_OK);
  }

  status_line (" %s %s %s", MSG_TXT (M_END_OF), MSG_TXT (M_INBOUND), MSG_TXT (M_FILE_ATTACHES));
  CLEAR_INBOUND ();
  return (done);
}

int
FTSC_callback (char *sptr)
{
  if (Batch_Send (sptr) != FILE_SENT_OK)
    return CANNOT_SEND;

  return FILE_SENT_OK;
}

int
FTSC_time (long filesize)
{
  long ltemp;

  ltemp = remaining (filesize, 94L);
  return (ltemp < 20L) ? 20 : (int) ltemp;
}

/* $Id: ftsc.c,v 1.9 1999/09/27 20:51:26 mr Exp $ */
