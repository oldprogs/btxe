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
 * Filename    : $Source: E:/cvs/btxe/src/b_wazoo.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:20 $
 * State       : $State: Exp $
 *
 * Description : BinkleyTerm WaZOO/EMSI Sendmail Processor
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

extern int WaZOO_callback (char *);
extern int WaZOO_time (long);


static MISSION_ERR
ZedZapZip (int originator, int noPick)
{
  MISSION_ERR error = mission_ok;

  session_method = ZMODEM_SESSION;

  /* -------------------------------------------------------------------- */
  /* ORIGINATOR: send/receive/send                                        */
  /* -------------------------------------------------------------------- */
  if (originator)
  {
    int zmodem;
    int anysent;

    status_line (">Calling send_WaZOO ...");
    error = send_WaZOO (ZMODEM_SESSION);
    status_line (">Returned from send_WaZOO, error=%d.", error);

    if (error != mission_ok)
      status_line ("!Problems: Leaving ZedZapZip ...");
    else if (!CARRIER)
    {
      status_line ("!NoCarrier: Leaving ZedZapZip ...");
      error = mission_aborted;
    }
    else if (noPick)
      status_line (">NoPickup: Leaving ZedZapZip ...");
    else
    {
      status_line (">Calling get_Zmodem ...");

      if (get_Zmodem (CURRENT.sc_Inbound, NULL) == NOTHING_RECVED)
      {
        status_line (">Returned from get_Zmodem.");
        status_line (">Nothing Received: Leaving ZedZapZip ...");
        error = mission_error;
      }
      else
      {
        status_line (">Returned from get_Zmodem.");

        if (!CARRIER)
        {
          status_line (">NoCarrier: Leaving ZedZapZip ...");
          // error = mission_aborted; // TJW: is this an error in ANY case ?!?
        }
        else
        {
          status_line (">Calling respond_to_freqs ...");

          anysent = respond_to_file_requests (0, WaZOO_callback, WaZOO_time);
          fsent += anysent;

          status_line (">Returned from respond_to_freqs, sent:%d.", anysent);

          zmodem = anysent ? ZMODEM_ENDBATCH : ZMODEM_END;

          status_line (">Calling Send_Zmodem to finish (%d) ...", zmodem);

          // TJW: commented out, because this often gave errors and REQ files
          // were not delete. Yet to be cleared !!!!!
          // if (
          Send_Zmodem (NULL, NULL, zmodem, DO_WAZOO);
          // == CANNOT_SEND)
          //  error = mission_error; // TJW: is this an error in ANY case ?!? 

          status_line (">Returned from Send_Zmodem.");

          zmodem_state = ZMODEM_INIT;
          mail_finished = 1;
        }
      }
    }
  }

  /* -------------------------------------------------------------------- */
  /* CALLED SYSTEM: receive/send/receive                                  */
  /* -------------------------------------------------------------------- */

  else
  {
    status_line (">Calling get_Zmodem ...");

    if (get_Zmodem (CURRENT.sc_Inbound, NULL) == NOTHING_RECVED)
    {
      status_line (">Returned from get_Zmodem.");
      status_line (">Nothing Received: Leaving ZedZapZip ...");
      error = mission_error;
    }
    else
    {
      status_line (">Returned from get_Zmodem.");

      if (!CARRIER)
      {
        status_line ("!NoCarrier: Leaving ZedZapZip ...");
        error = mission_aborted;
      }
      else
      {
        if (remote_pickup != 0) /* remote doesn't reject pickups now */
        {
          status_line (">Calling send_WaZOO ...");
          error = send_WaZOO (ZMODEM_SESSION);
          status_line (">Returned from send_WaZOO.");
        }

        if (error != mission_ok)
          status_line ("!Problems: Leaving ZedZapZip ...");
        else if (!CARRIER)
        {
          status_line ("!NoCarrier: Leaving ZedZapZip ...");
          error = mission_aborted;
        }
        else if (!made_request)
          status_line (">NotMadeRequest: Leaving ZedZapZip ...");
        else
        {
          status_line (">Calling get_Zmodem ...");

          if (get_Zmodem (CURRENT.sc_Inbound, NULL) == NOTHING_RECVED)
            error = mission_error;

          status_line (">Returned from get_Zmodem.");
        }
      }
    }
  }

  XON_DISABLE ();               // HJK 980807

  return error;
}


void
show_session (char *text, ADDRP remote)
{
  char addrstr[25];
  char j[200];

  if (remote == NULL)
    sprintf (addrstr, "unknown system");  // MR 980118 remote might be NULL

  else
  {
    if (remote->Point == 0)
    {
      sprintf (addrstr, "%d:%d/%d",
               remote->Zone, remote->Net, remote->Node);
    }
    else
    {
      sprintf (addrstr, "%d:%d/%d.%d",
               remote->Zone, remote->Net, remote->Node, remote->Point);
    }
  }

  sprintf (j, text, addrstr);
  IPC_SetStatus (j);
}

/* ------------------------------------------------------------------------ */
/* WaZOO/EMSI                                                               */
/* ------------------------------------------------------------------------ */

MISSION_ERR
WaZOO (int originator)
{
  char j[200];
  char k[100];
  char fname[PATHLEN];
  char fname2[PATHLEN];
  char *HoldName;
  ADDR saved_addr;
  ADDRP addrp;
  int i = 0;
  int akas = 0;
  MISSION_ERR error = mission_ok;

  long noZed = QueryNodeFlag (no_zapzed, NOZED, &remote_addr);  /*r. hoerner */
  long noZip = QueryNodeFlag (no_zipzed, NOZIP, &remote_addr);  /*j. n. klug */
  long noJan = QueryNodeFlag (no_janus, NOJANUS, &remote_addr);  /*r. hoerner */
  long noHyd = QueryNodeFlag (no_hydra, NOHYDRA, &remote_addr);  /*r. hoerner */
  long noPick = QueryNodeFlag (no_pickup, NOPICK, &remote_addr);  /*r. hoerner */

  noPick = noPick || !pickup_ok;  /* cfg-flag + event def */

  /* -------------------------------------------------------------------- */
  /* Initialize WaZOO/EMSI                                                */
  /* -------------------------------------------------------------------- */

  made_request = got_arcmail = got_packet = got_tic = 0;  /* VRP 990918 */

  Netmail_Session = 1;

  isOriginator = originator;

  if (originator &&
      ((remote_addr.Zone != called_addr.Zone) ||
       (remote_addr.Net != called_addr.Net) ||
       (remote_addr.Node != called_addr.Node) ||
       (remote_addr.Point != called_addr.Point)))
  {
    sprintf (k, "%s", Full_Addr_Str (&remote_addr));
    sprintf (j, MSG_TXT (M_CALLED), Full_Addr_Str (&called_addr), k);
    status_line (j);
  }
  else if (!originator)
    called_addr = remote_addr;

  saved_addr = called_addr;

  show_session (MSG_TXT (M_MCP_MAIL_XFER), &remote_addr);

  /* If we are using an external mail agent, call it now. */

  if (remote_pickup == -2)
  {
    num_rakas = akas = 1;
    remote_akas[0] = called_addr;
    status_line (">External Mail Session");
    error = send_WaZOO (ZMODEM_SESSION);
  }
  else if (!CARRIER)            /* We need to have DCD here. */
  {
    error = mission_aborted;
  }
  else
  {
    switch (remote_pickup)
    {
    case -1:                   /* WaZOO Session */
      akas = 1;
      status_line (">WaZOO Session");
      remote_akas[0] = called_addr;
      break;

    case 0:                    /* TJW 960610 */
      status_line (">EMSI, no pickup");
      akas = 0;
      break;

    case 1:
      status_line (">EMSI, no pickup-all");
      if (num_rakas >= 1)
      {
        akas = 1;
        remote_akas[0] = called_addr;
      }
      break;

    case 2:
      status_line (">EMSI, pickup-all");
      akas = num_rakas;
      break;

    default:
      akas = 0;
      break;
    }

    /* Flag all AKA's received */

    for (i = 0; i < akas; i++)
    {
      status_line (">Setting flag_file for %s", Full_Addr_Str (&remote_akas[i]));
      if (flag_file (TEST_AND_SET, &remote_akas[i], 1))
      {
        int n;

        status_line (">Failed to set flag_file for %s", Full_Addr_Str (&remote_akas[i]));

        /* Couldn't flag it, shift everything down one */

        akas--;
        for (n = i + 1; n <= akas; n++)
          remote_akas[n - 1] = remote_akas[n];

        /* Do this loop again since we just replaced this remote_aka */
        i--;
      }
    }

    num_rakas = akas;

    /* TJW 960721                                                  */
    /* remote_capability contains only ONE protocol we can also do */
    /* further, it is already checked if bidi protocols are ok     */

    /* Let's look into the BiDi protocols first */

    /* We'll have to call them even when remote set nopickup, otherwise */
    /* we wouldn't get mail, either [AG 990926] */

    if (debugging_log)
      show_our_capabilities ("WAZOO");

#ifdef HAVE_HYDRA
    if ((unsigned) (remote_capabilities & DOES_HYDRA) && !noHyd)
    {
      status_line ("%s %sHydra", MSG_TXT (M_WAZOO_METHOD), do_RH1hydra ? "x" : "");
      do_protocol (do_RH1hydra ? M_STATUS_XHYDRA : M_STATUS_HYDRA);
      error = send_WaZOO (HYDRA_SESSION);
    }
    else
#endif
    if ((unsigned) (remote_capabilities & DOES_IANUS) && !noJan)
    {
      status_line ("%s Janus", MSG_TXT (M_WAZOO_METHOD));
      do_protocol (M_STATUS_JANUS);
      session_method = JANUS_SESSION;
      error = Janus ();
    }
    else if (!(akas <= 0) && (remote_pickup != 0))  /* TJW 960610 fixes nopickup */
    {
      if ((remote_capabilities & ZED_ZAPPER) && !noZed)
      {
        if (direct_zap)
          status_line ("%s DirZap", MSG_TXT (M_WAZOO_METHOD));
        else
          status_line ("%s ZedZap", MSG_TXT (M_WAZOO_METHOD));
        do_protocol (M_STATUS_ZEDZAP);
        error = ZedZapZip (originator, noPick);
      }
      else if ((remote_capabilities & ZED_ZIPPER) && !noZip)
      {
        status_line ("%s ZedZip", MSG_TXT (M_WAZOO_METHOD));
        do_protocol (M_STATUS_ZEDZIP);
        error = ZedZapZip (originator, noPick);
      }
      else if (remote_pickup == -1)
      {
        long noDiet = QueryNodeFlag (no_dietifna, NODIET, &called_addr);

        if (originator && noDiet)  /* if we don't like OUTgoing FTS-1 */
        {
          status_line ("%s: NoDietIFNA", MSG_TXT (M_NO_COMMON_PROTO));
          error = mission_error;
        }
        else
        {
          do_protocol (M_STATUS_FTS1);
          status_line ("%s DietIFNA", MSG_TXT (M_WAZOO_METHOD));
          session_method = FTSC_SESSION;
          if (originator)
            error = FTSC_sender (1);
          else
            error = FTSC_receiver (1);
        }
      }
    }
  }

  modem_hangup ();              /* r. hoerner: don't charge to telecom any more! */

  called_addr = saved_addr;

  if (made_request)
  {
    int delreq;

    if (error == mission_ok)
    {
      delreq = 1;
    }
    else
    {
      delreq = 0;
      status_line (MSG_TXT (M_REACTIVATING_REQ), error);
    }

    for (i = 0; i < akas; i++)
    {
      addrp = &remote_akas[i];

      /* delete RQFs at end of session if session was successful */
      /* else rename to REQ to activate them again               */

      HoldName = HoldAreaNameMunge (addrp);
      sprintf (fname, "%s%s.rqf", HoldName, Hex_Addr_Str (addrp));
      sprintf (fname2, "%s%s.req", HoldName, Hex_Addr_Str (addrp));

      if (delreq)
        unlink (fname);         /* delete completed REQ now   */
      else
        rename (fname, fname2); /* reactivate incomplete REQ  */
    }
  }

  for (i = 0; i < akas; i++)
    flag_file (CLEAR_FLAG, &remote_akas[i], 1);

  status_line (MSG_TXT (M_WAZOO_END));

  return error;
}                               /* WaZOO                      */


/* ------------------------------------------------------------------------ */
/* RESPOND TO FILE REQUEST                                                  */
/* ------------------------------------------------------------------------ */
int
respond_to_file_requests (int janusflag, int (*callback) (char *), int (*calltime) (long))
{
  char req[PATHLEN];
  struct stat buf;
  int sentfiles = 0;
  FILE *fp;
  char *s;
  ADDR foo;
  AKA *aka;
  int result = 0;               /* = nr of files succesfully sent */

  /* this is for callback */
  if (!(matrix_mask & TAKE_REQ))
    goto done;

  if (SRIF_commandline[0])      /* mtt 960429 */
    return (invoke_SRIF (janusflag, callback, calltime));

  /* else */

  for (aka = alias; aka != NULL; aka = aka->next)
  {
    memcpy (&foo, aka, sizeof (ADDR));

    /* For a point, massage the address to get the right .REQ filename */

    if (foo.Point != 0)
    {
      foo.Node = foo.Point;
      foo.Point = 0;
      foo.Net = (pvtnet > 0) ? (word) pvtnet : 0;
    }

    sprintf (req, request_template, CURRENT.sc_Inbound, Hex_Addr_Str (&foo), TaskNumber);

    if (!unix_stat (req, &buf))
    {
      if ((fp = fopen (req, read_ascii)) == NULL)
      {
        got_error (MSG_TXT (M_OPEN_MSG), req);
        /* TJW960126 was: goto done; */
        continue;               /* if this one fails, try next REQ file */
      }

      while (!feof (fp))
      {
        req[0] = 0;
        if ((fgets (req, 79, fp) == NULL) || !CARRIER)
          break;

        /* ; as the first char is a comment */
        if (req[0] == ';')
          continue;

        /* Get rid of the newline at the end */
        s = (char *) (req + strlen (req) - 1);
        while ((s >= req) && isspace (*s))
          *s-- = '\0';

        if (req[0] == '\0')
          continue;

        if ((sentfiles = n_frproc (req, callback, calltime)) < 0)
          break;                /* break on error */

        result += sentfiles;
      }

      fclose (fp);

      sprintf (req, request_template, CURRENT.sc_Inbound, Hex_Addr_Str (&foo), TaskNumber);
      unlink (req);

      if (!CARRIER)
        break;                  /* this one is needed (r.hoerner 97/01/27) */

      /* once we have satisfied a request, we can get out of here */
      /* TJW960126: but why should we ??? commented out! */
      /* break; */
    }
  }

done:
  return result;
}

/* $Id: b_wazoo.c,v 1.7 1999/09/27 20:51:20 mr Exp $ */
