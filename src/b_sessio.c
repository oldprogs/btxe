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
 * Filename    : $Source: E:/cvs/btxe/src/b_sessio.c,v $
 * Revision    : $Revision: 1.14 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:19 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Mail Session Initiator
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
show_nodes_name (char *addinfo)
{
  if (un_attended)
  {
    int sysopname = strlen (lastfound_SysopName);

    if (sysopname || addinfo)
    {
      char temp[80];

      if (sysopname)
      {
        fancy_str (lastfound_SysopName);
        sprintf (temp, "%s, %s ", Full_Addr_Str (&remote_addr),
                 lastfound_SysopName);
      }
      else
        sprintf (temp, "%s ", MSG_TXT (M_READY_CONNECT));

      if (addinfo)
        strcat (temp, addinfo);

      strcpy (GenStats.remote, temp);
      css_upd ();
    }
  }
}

MISSION_ERR
b_session (int was_initiator, CALLHIST * data)
{
  ADDR tmp;
  ADDRP addrp;
  int i;
  int uucpe;
  long noChat, noZed, noJan, noHyd, noWazoo, noEmsi, noDiet;
  int realHydraChat;
  int retry = 0;
  MISSION_ERR error = mission_ok;

  isOriginator = was_initiator;
  start_hist = hist;
  direct_zap = 0;
  CURRENT = DEFAULT;
  ReqSkipFile = 0;
  mail_finished = FALSE;
  diff_trx = 0;                 /* MMP 960419 If non-zero, clock will be set */
  is_protected = FALSE;         /* TJW 960526 / MTT 9605 for SRIF */
  is_listed = FALSE;            /* TJW 960526 / MTT 9605 for SRIF */
#ifdef HAVE_HYDRA
  do_RH1hydra = FALSE;          /* TJW 960528 RH1 / xHydra */
#endif
  freq_accum.bytes = 0L;
  freq_accum.files = 0;
  freq_accum.time = 0L;

  data->outgoing = was_initiator;
  data->task = TaskNumber;
  data->calltype = CT_MAILER;
  data->subclass = SC_FAILURE;

  do_random_system ();
  chat_prepare ();
  unix_time (&etm);

  strcpy (remote_sysop, "Unknown sysop name");  /* TJW 960528 */
  strcpy (remote_site, "Unknown site name");  /* TJW 960528 */
  strcpy (remote_location, "Unknown location");  /* TJW 960528 */
  strcpy (remote_phone, "Unknown phone");  /* TJW 960528 */
  strcpy (remote_mailer, "Unknown mailer");  /* TJW 960528 */
  strcpy (password, "");        /* TJW 960528 */
  strcpy (lastfound_SysopName, "(unknown system)");

  if (un_attended)
    do_status (M_READY_CONNECT, NULL);

  /* TJW 960526 code for removing REQs and Response packets deleted  */
  /* this makes NO sense here, because the correct inbound directory */
  /* (depending on session security) is not yet known at this place. */

  tmp.Zone = tmp.Net = tmp.Node = tmp.Point = 0;
  tmp.Domain = NULL;

  /* we must, if we want node dependend flags AND global flags, use temporary
   * flags that are set according to:
   *  * if flag is set *ON CURRENT NODE* then use this flag as current flag.
   *  * ELSE if flag is set *GLOBALLY* use this flag as current flag
   *  * ELSE dont use it, cause it isn't set at all.
   *
   * Therefore all usage of real global flags ("no_janus") has been replaced
   * by the usage of the corresponding temp-flag. "QueryNodeFlag" resides in
   * misc.c
   * r. hoerner */

  if (was_initiator)            /* QueryNodeFlag returns global variable if */
    addrp = &called_addr;
  else
    addrp = NULL;

  noDiet = QueryNodeFlag (no_dietifna, NODIET, addrp);  /*r. hoerner */
  noWazoo = QueryNodeFlag (no_WaZOO, NOWAZOO, addrp);  /*r. hoerner */
  noEmsi = QueryNodeFlag (no_EMSI, NOEMSI, addrp);  /*r. hoerner */
  noZed = QueryNodeFlag (no_zapzed, NOZED, addrp);  /*r. hoerner */
  noJan = QueryNodeFlag (no_janus, NOJANUS, addrp);  /*r. hoerner */
  noHyd = QueryNodeFlag (no_hydra, NOHYDRA, addrp);  /*r. hoerner */
  noChat = QueryNodeFlag (nohydrachat, NOHYCHAT, addrp);  /*r. hoerner */

  realHydraChat = nohydrachat;  /* save real global state */
  nohydrachat = noChat;         /* change global state */
  no_WaZOO_Session = noWazoo;
  no_EMSI_Session = noEmsi;
  remote_capabilities = my_capabilities = 0;

  my_capabilities = noZed ? 0 : ZED_ZAPPER;  /* r. hoerner */
  my_capabilities |= ZED_ZIPPER;

  if (was_initiator)
    my_capabilities |= noDiet ? 0 : Y_DIETIFNA;  /* r. hoerner */
  else
    my_capabilities |= Y_DIETIFNA;  /* always allow incoming FTS-1 */

  if ((janus_baud >= cur_baud.rate_value) || (janus_OK))
  {
    if (!noJan)
      my_capabilities |= DOES_IANUS;
#ifdef HAVE_HYDRA
    if (!noHyd)
      my_capabilities |= DOES_HYDRA;
#endif
  }
  if (!protpreference)
  {
    *actprotpreference = '\0';

    if ((janus_baud >= cur_baud.rate_value) || (janus_OK))
    {
#ifdef HAVE_HYDRA
      if (!no_hydra)
        strcat (actprotpreference, "HYD,");
#endif
      if (!no_janus)
        strcat (actprotpreference, "JAN,");
    }

    if (!no_zapzed)
      strcat (actprotpreference, "ZAP,ZMO");
  }
  else
    strcpy (actprotpreference, protpreference);

  flag_file (SET_SESSION_FLAG, &called_addr, 0);
  clear_filetransfer ();        /* r.hoerner 1/13/97 */
  css_init ();

  fsent = 0;
  SyncHydraSession = 0;
  bttask = 0;
  zmodem_state = ZMODEM_INIT;
  session_method = 0;

  if (was_initiator)
  {

    /*
     *    OUTBOUND -- meaning call was made by us.
     */

    last_type (1, &remote_addr);

    /* See if this address is serviced by an external mail
       agent. If so, call WaZOO with a special argument. */

    if (ExtMailMask && (newnodedes.ModemType == (byte) ExtMailMask))
    {
      /* VRP 990913 start */
      strcpy (remote_site, newnodedes.SystemName);
      strcpy (remote_location, newnodedes.MiscInfo);
      strcpy (remote_mailer, ExtMailAgent);
      realspeed = 0;
      /* VRP 990913 start */

      data->calltype = CT_EXTMAIL;
      data->subclass = 0;
      no_EMSI_Session = TRUE;
      remote_pickup = -2;
      remote_capabilities = my_capabilities = 0;
      do_status (M_STATUS_SESSION, NULL);
      do_session (M_STATUS_EXTERN);  /* r. hoerner */
      error = WaZOO (1);
    }
    else if (Whack_CR () == 0)
    {
      do
      {
        switch (i = CallerSendSync ())
        {
        case CALLED_FTSC:
          if (noDiet)
          {
            no_EMSI_Session = no_WaZOO_Session = TRUE;
            status_line ("%s: NoDietIFNA", MSG_TXT (M_NO_COMMON_PROTO));
            ++retry;
          }
          else
          {
            data->subclass = SC_FTS1;
            no_EMSI_Session = TRUE;
            session_method = FTSC_SESSION;  /* xmodem/sealink/telink */
            do_status (M_STATUS_SESSION, NULL);
            do_session (M_STATUS_FTS1);
            do_protocol (M_STATUS_XMODEM);
            show_nodes_name (NULL);
            error = FTSC_sender (0);
            retry = 0;
          }
          break;

        case CALLED_YOOHOO:
          if (YooHoo_Sender ())
          {
            show_nodes_name (NULL);
            if ((remote_capabilities & my_capabilities) == 0)
            {
              no_EMSI_Session = no_WaZOO_Session = TRUE;
              status_line (MSG_TXT (M_NO_COMMON_PROTO));
              ++retry;
            }
            else
            {
              data->subclass = SC_WAZOO;
              no_EMSI_Session = TRUE;
              remote_pickup = -1;
              do_status (M_STATUS_SESSION, NULL);
              do_session (M_STATUS_YOOHOO);
              error = WaZOO (1);
              retry = 0;
            }
          }
          break;

        case CALLED_EMSI:
          if (EMSI_Sender ())
          {
            if (lastfound_SysopName[0] == '(')  /* not found in NL */
            {
              char c = remote_sysop[43];

              remote_sysop[43] = 0;
              sprintf (lastfound_SysopName, "%s (EMSI)", remote_sysop);
              remote_sysop[43] = c;
            }

            show_nodes_name (NULL);

            if ((remote_capabilities & my_capabilities) == 0)
            {
              no_EMSI_Session = no_WaZOO_Session = TRUE;
              status_line (MSG_TXT (M_NO_COMMON_PROTO));
              ++retry;
            }
            else
            {
              data->subclass = SC_EMSI;
              do_status (M_STATUS_SESSION, NULL);
              do_session (M_STATUS_EMSI);
              error = WaZOO (1);
              retry = 0;
            }
          }
          break;

        default:
          ++retry;
          /* AW980603 missing ++retry caused endless loop on */
          /* certain failed handshakes                       */
          break;

        }                       /* end switch   */
      }                         /* end do       */
      while (retry && (retry < 5));  /* only 5 tries */
    }                           /* end if       */
  }
  else
  {
    /*
     *    INBOUND -- meaning we were connected to by another system.
     */

    do
    {
      b_init ();

      /* String following "Connect [xxxx]" is a special case of
         an external mail string. */

      for (uucpe = 0, i = 0; i < num_ext_mail; i++)
      {
        if (stricmp (saved_response, ext_mail_string[i]) == 0)
        {
          uucpe = i + RECVED_UUCP;
          break;
        }
      }

      if (uucpe != 0)
        i = uucpe;
      else
        i = CalledRecvSync ();

      switch (i)
      {
      case RECVED_BBS:         /* ESCape or Timeout      */
        if (CARRIER && !mail_only)
        {
          data->calltype = CT_BBS;
          data->subclass = 0;
          last_type (3, &tmp);
          flag_file (CLEAR_SESSION_FLAG, &called_addr, 0);
          BBSexit ();
          modem_hangup ();
        }
        else
          status_line (MSG_TXT (M_REJECTING_HUMAN_CALLER));
        break;

      case RECVED_FTSC:        /* FTSC Netmail Session   */
        data->subclass = SC_FTS1;
        if (CARRIER)
        {
          ++hist.mail_calls;
          tmp.Zone = (unsigned short) -1000;
          last_type (2, &tmp);
          tmp.Zone = 0;
          no_EMSI_Session = TRUE;
          session_method = FTSC_SESSION;  /* xmodem/sealink/telink  */
          do_status (M_STATUS_SESSION, NULL);
          do_session (M_STATUS_FTS1);
          do_protocol (M_STATUS_XMODEM);
          error = FTSC_receiver (0);
          retry = 0;
        }
        break;

      case RECVED_WAZOO:       /* WaZOO Netmail Session  */
        if (YooHoo_Receiver ())
        {
          show_nodes_name (NULL);
          if ((remote_capabilities & my_capabilities) == 0)
          {
            no_WaZOO_Session = no_EMSI_Session = TRUE;
            status_line (MSG_TXT (M_NO_COMMON_PROTO));
            ++retry;
          }
          else
          {
            data->subclass = SC_WAZOO;
            remote_pickup = -1;
            no_EMSI_Session = TRUE;
            ++hist.mail_calls;
            last_type (1, &remote_addr);
            do_status (M_STATUS_SESSION, NULL);
            do_session (M_STATUS_YOOHOO);
            error = WaZOO (0);  /* do WaZOO!!!            */
            retry = 0;
          }
        }
        break;

      case RECVED_EMSI:        /* EMSI Netmail Session   */
        if (EMSI_Receiver ())
        {
          if (lastfound_SysopName[0] == '(')  /* not found in NL        */
          {
            char c = remote_sysop[43];

            remote_sysop[43] = 0;
            sprintf (lastfound_SysopName, "%s (EMSI)", remote_sysop);
            remote_sysop[43] = c;
          }

          show_nodes_name (NULL);

          if ((remote_capabilities & my_capabilities) == 0)
          {
            no_WaZOO_Session = no_EMSI_Session = TRUE;
            status_line (MSG_TXT (M_NO_COMMON_PROTO));
            ++retry;
          }
          else
          {
            data->subclass = SC_EMSI;
            ++hist.mail_calls;
            last_type (1, &remote_addr);
            do_status (M_STATUS_SESSION, NULL);
            do_session (M_STATUS_EMSI);
            error = WaZOO (0);  /* do EMSI!!!             */
            retry = 0;
          }
        }
        break;

      default:
        if (CARRIER)            /* See if ext mail */
        {
          if ((i >= RECVED_UUCP) && (i < (RECVED_UUCP + num_ext_mail)))
          {
            /* VRP 990913 start */
            strcpy (remote_site, newnodedes.SystemName);
            strcpy (remote_location, newnodedes.MiscInfo);
            strcpy (remote_mailer, "External mailer");
            realspeed = 0;
            /* VRP 990913 start */

            data->calltype = CT_EXTMAIL;
            data->subclass = 0;
            last_type (4, &tmp);
            flag_file (CLEAR_SESSION_FLAG, &called_addr, 0);
            do_status (M_STATUS_SESSION, NULL);
            do_session (M_STATUS_EXTERN);
            UUCPexit (lev_ext_mail[i - RECVED_UUCP], 1);
          }
          else                  /* MR 972504 what else should we do??? */
            status_line (MSG_TXT (M_NO_COMMON_PROTO));
        }
        else
          status_line (MSG_TXT (M_NO_CARRIER));

        modem_hangup ();        /* reset modem            */
        retry = 0;
        break;

      }                         /* end switch             */
    }                           /* end do                 */
    while (retry && (retry < 5));  /* only 5 tries           */
  }                             /* end if (was_initiator) */

  if (retry)                    /* we tried too often     */
  {
    modem_hangup ();            /* reset modem            */
    error = mission_error;
  }

  nohydrachat = realHydraChat;  /* restore global state   */
  session_method = 0;
  data->h.m.passworded = is_protected;

  memcpy (data->h.m.name, remote_sysop, strlen (remote_sysop) > 39 ? 39 : strlen (remote_sysop));
  memcpy (data->h.m.system, remote_site, strlen (remote_site) > 39 ? 39 : strlen (remote_site));
  memcpy (data->h.m.location, remote_location, strlen (remote_location) > 39 ? 39 : strlen (remote_location));
  memcpy (data->h.m.phone, remote_phone, strlen (remote_phone) > 24 ? 24 : strlen (remote_phone));
  memcpy (data->h.m.mailer, remote_mailer, strlen (remote_mailer) > 39 ? 39 : strlen (remote_mailer));
  data->h.m.name[39] = data->h.m.system[39] = data->h.m.location[39] =
    data->h.m.phone[24] = data->h.m.mailer[39] = 0;
  return error;
}

void
b_session_cleanup (int sess_result)  /* VRP 990826 */
{
  ADDR tmp;
  char junk[256];
  AKA *aka;
  int res;                      /* VRP 990826 */

  if (CARRIER)
    modem_hangup ();

  chat_exit ();                 /* TJW 960706 close hydra chat window if open */
  css_done ();                  /*            close CSS                       */
  do_today (1);                 /* TJW 960903 update / show bmf call counters */
  timesync ();                  /* MMP 96....                                 */
  modem_aftercall ();           /* MR  970420 get modem statistics            */
  modem_init ();                /*            and initialize it               */

  next_rescan = 0L;
  next_time = 0L;

#if defined(OS_2) && !defined(__EMX__)
  CIDFromCAPI[0] = 0;           /* Remove caller-id so it isn't used for next call... */
  UpdateCID = 1;                /* ...And allow CAPI thread to update it */
#endif

  /* Remove the .REQ (*.Rxx) and .RSP files                       */
  /* TJW 960526 this was moved down to here (end of session), see */
  /* comment at start of session, too.                            */

  if (alias != NULL)
  {
    for (aka = alias; aka != NULL; aka = aka->next)
    {
      memcpy (&tmp, aka, sizeof (ADDR));

      /* For a point, massage the address to get right .REQ filename */

      if (tmp.Point != 0)
      {
        tmp.Node = tmp.Point;
        tmp.Point = 0;
        tmp.Net = (pvtnet > 0) ? (word) pvtnet : 0;
      }

      sprintf (junk, request_template, CURRENT.sc_Inbound, Hex_Addr_Str (&tmp), TaskNumber);
      unlink (junk);

      if (!pktrsp)              /* MB 93-12-12  PKTRSP */
      {
        sprintf (junk, "%s.rsp", Hex_Addr_Str (&tmp));
        unlink (junk);
      }
    }

    /* remove old response PKTs */

    if (pktrsp)                 /* MB 93-12-12  PKTRSP */
    {
      struct FILEINFO dta;
      char fname[PATHLEN];

      sprintf (junk, "%s%02x*.pkt", flag_dir, TaskNumber);
      if (!dfind (&dta, junk, 0))
      {
        do
        {
          sprintf (fname, "%s%s", flag_dir, dta.name);
          unlink (fname);
        }
        while (!dfind (&dta, junk, 1));

        dfind (&dta, NULL, 2);
      }
    }

    saved_response[0] = '\0';
    no_WaZOO_Session = no_WaZOO;
    no_EMSI_Session = no_EMSI;
    /* ensure flag's gone */
    flag_file (CLEAR_FLAG, &called_addr, 1);
    flag_file (CLEAR_SESSION_FLAG, &called_addr, 0);

    /* VRP 990826 start */

    if (sess_result != mission_ok && (isOriginator && RestorePoll))
    {
      flag_file (TEST_AND_SET, &called_addr, 0);
      res = mail_addsend (&called_addr, "", RestorePollFlavour);
      flag_file (CLEAR_FLAG, &called_addr, 0);
      status_line (">Poll to %d:%d/%d.%d restored. Flavour: %slo. Result: %d",
                   called_addr.Zone, called_addr.Net, called_addr.Node,
                   called_addr.Point, RestorePollFlavour, res);
    }

    /* VRP 990826 end */
  }                             /* end of if (alias != NULL) */

  global_rescan ();             /* TJW 960916 force global rescan */

#ifdef HEAPCHECK
  heapchk (2);
#endif
}

void
b_init ()
{
  got_arcmail = 0;
  got_packet = 0;
  got_mail = 0;
  got_fax = 0;
  sent_mail = 0;
  remote_addr.Zone = 0;
  remote_addr.Net = 0;
  remote_addr.Node = 0;
  remote_addr.Point = 0;
  remote_addr.Domain = NULL;
  assumed = 0;
  Netmail_Session = 0;
  remote_capabilities = 0;
  mail_finished = FALSE;
  CURRENT = DEFAULT;

#ifdef HEAPCHECK
  heapchk (1);
#endif
}

/* Send banner-type lines to remote. Since strange modems like Courier HST
 * have a propensity for thinking they are connected when talking to a dial
 * tone (but do leave CTS down just to screw us up) we have to use a timeout
 * to figure out if we are in trouble, and if so, we drop DTR to make the
 * bogus carrier go away.
 *
 * This routine is used in modules B_BBS.C and RECVSYNC.C, both of which
 * are called from this module only. */

int
SendBanner (char far * string, int forcedsend)
{
  long t1;
  char *p, d;
  int i;

  if (string == NULL)
    return (1);                 /* say: success */

  t1 = timerset (15 * PER_SECOND);  /* 15 secs to send banner */

  p = string;
  i = strlen (p);

  while (!timeup (t1) && CARRIER && i)
  {
    if (got_ESC ())             /* Manual abort?     */
      break;                    /* Use failure logic */

    if (!forcedsend)            /* if BBSwelcome and so on.. */
    {
      d = PEEKBYTE ();
      if ((d == '*')            /* if other side sends EMSI (r.hoerner) */
          || (d == (char) TSYNC)  /* ... or TSYNC .. */
          || (d == (char) YOOHOO))  /* .. or YOOHOO .. */
        return (1);             /* leave early    */
    }

    i = strlen (p);
    if (i > 128)
      i = 128;

    SENDCHARS (p, i, 1);        /* Kick the other guy.       */
    p += i;
    time_release ();
  }

  if (!i)
    return (1);

  /* If we get here we had trouble. Hangup the modem. */

  modem_hangup ();
  return (0);
}

static void
freeze (void)
{
  modem_busy ();
  IPC_SetStatus (MSG_TXT (M_MCP_FROZEN));
  close_up (0);
  closelogs ();
}

static void
unfreeze (void)
{
  openlogs (0);
  come_back ();
  modem_init ();
  IPC_SetStatus (MSG_TXT (M_MCP_WAITING));
}

int
flag_file (int function, ADDRP address, int do_stat)
{
  static ADDR last_set[ALIAS_CNT];
  static int had_to_punt[ALIAS_CNT];
  static int last_count;
  FILE *fptr;
  char *HoldName;
  char flagname[PATHLEN];
  char tmpname[PATHLEN];
  char BSYname[15];
  int i, j;

  HoldName = HoldAreaNameMunge (address);

  switch (function)
  {
  case INITIALIZE:
    last_count = 0;
    /* Fall through for cleanup purposes */

  case CLEAR_SESSION_FLAG:
    /* At the end of a session, delete the task file */
    if (flag_dir)
    {
      sprintf (flagname, "%stask.%02x", flag_dir, TaskNumber);
      unlink (flagname);
    }
    return (FALSE);

  case SET_SESSION_FLAG:
    /* At the start of a session, set up the task number */
    if (flag_dir)
    {
      sprintf (flagname, "%stask.%02x", flag_dir, TaskNumber);
      fptr = fopen (flagname, write_binary);
      if (fptr)
        fclose (fptr);
    }
    return (FALSE);

  case TEST_AND_SET:
    status_line (">set_flagfile for %s", Full_Addr_Str (address));

    /*  First see if we already HAVE this lock! If so, return now. */
    for (i = 0; i < last_count; i++)
    {
      if (memcmp (&last_set[i], address, sizeof (ADDR)) == 0)
        return (FALSE);
    }

    /* TJW 960503 "MakeDir" part 1
     * create domain / zone outbound directory if not already existing */

#ifdef MAKEDIR
    if (makedir)
    {
#endif
      strcpy (tmpname, HoldName);
      tmpname[strlen (tmpname) - 1] = '\0';  /* remove backslash */
      status_line (">MakeDir: Attempting to create %s", tmpname);
      mkdir (tmpname);
      /* create directory. if it is already there, this is not necessary, but
       * also does no harm. if it is not there yet, it will be there NOW. */
#ifdef MAKEDIR
    }
#endif

    /* Next determine the directory in which we will create the flagfile.
     * Also, the name of the file. */

    if (address->Point != 0)
    {
      sprintf (flagname, "%s%04hx%04hx.pnt" DIR_SEPS,
               HoldName, address->Net, address->Node);
      sprintf (BSYname, "%08hx.bsy", address->Point);

      /* TJW 960429 "MakeDir" part 2
       * create point directory if not already existing */

#ifdef MAKEDIR
      if (makedir)
      {
#endif
        strcpy (tmpname, flagname);
        tmpname[strlen (tmpname) - 1] = '\0';  /* remove backslash */
        status_line (">MakeDir: Attempting to create %s", tmpname);
        mkdir (tmpname);
        /* create directory. if it is already there, this is not necessary, but
         * also does no harm. if it is not there yet, it will be there NOW. */
#ifdef MAKEDIR
      }
#endif
    }
    else
    {
      strcpy (flagname, HoldName);
      sprintf (BSYname, "%04hx%04hx.bsy", address->Net, address->Node);
    }

    /* File opens are destructive by nature. Therefore use a file name
     * that's unique to us. Create it in the chosen target. If we can't
     * do that, try to use the flag directory. */

    had_to_punt[last_count] = FALSE;
    sprintf (tmpname, "%sbinkbsy.%02x", flagname, TaskNumber);
    fptr = fopen (tmpname, write_binary);

    if ((fptr == NULL) && (flag_dir))
    {
      had_to_punt[last_count] = TRUE;
      strcpy (flagname, flag_dir);
      sprintf (tmpname, "%sbinkbsy.%02x", flagname, TaskNumber);
      fptr = fopen (tmpname, write_binary);
    }

    /* Now we've done all we can. The file is either open in the
     * appropriate outbound or it's in the flag directory.
     * If neither option worked out, go away. There's nothing to do. */

    if (fptr == NULL)
    {
      if (do_stat)
        status_line (MSG_TXT (M_FAILED_CREATE_FLAG), tmpname);
      return (TRUE);
    }
    fclose (fptr);

    /* Now the test&set. Attempt to rename the file to a value specific
     * to the remote node's address. If we succeed, we have the lock.
     * If we do not, delete the temp file. */

    /* 970504 see misc.c :: sem_tas() - duplicate code !!!!!!!!! */

    strcat (flagname, BSYname); /* Add the .BSY file name */
    i = rename (tmpname, flagname);
    if (i && maxbsyage)
    {
      struct stat st;
      time_t age;

      if (!unix_stat (flagname, &st))
      {
        if (unix_time (NULL) > st.st_mtime + 60L * maxbsyage)
        {
          age = (unix_time (NULL) - st.st_mtime) / 60UL;
          status_line (MSG_TXT (M_OLD_BSY_IGNORED), flagname, age);
          remove (flagname);
          i = rename (tmpname, flagname);
        }
      }
    }

    if (!i)
    {
      if (do_stat)
        status_line (MSG_TXT (M_CREATED_FLAGFILE), flagname);
      last_set[last_count++] = *address;
      return (FALSE);
    }

    if (do_stat)
      status_line (MSG_TXT (M_THIS_ADDRESS_LOCKED), Full_Addr_Str (address));

    unlink (tmpname);
    return (TRUE);

  case CLEAR_FLAG:

    status_line (">clear_flagfile for %s", Full_Addr_Str (address));

    /* Make sure we need to clear something.
     * Zone should be something other than -1 if that's the case. */

    if (!last_count)
      return (TRUE);

    /* Next compare what we want to clear with what we think we have. */

    for (i = 0; i < last_count; i++)
      if (!memcmp (&last_set[i], address, sizeof (ADDR)))
        break;

    if (i >= last_count)        /* not yet set ??? */
    {
      if (do_stat)
        status_line (MSG_TXT (M_BAD_CLEAR_FLAGFILE),
                     Full_Addr_Str (address));
      return (TRUE);
    }

    /* We match. Recalculate the directory. Yeah, that's redundant
     * code, but it saves static space. */

    if (address->Point != 0)
    {
      sprintf (flagname, "%s%04hx%04hx.pnt" DIR_SEPS,
               HoldName, address->Net, address->Node);
      sprintf (BSYname, "%08hx.bsy", address->Point);

#ifdef MAKEDIR
      if (makedir)
      {                         /* TJW 960429 makedir part 3a */
#endif
        strcpy (tmpname, flagname);
        tmpname[strlen (tmpname) - 1] = '\0';  /* remove backslash */
#ifdef MAKEDIR
      }
#endif
    }
    else
    {
      strcpy (flagname, HoldName);
      sprintf (BSYname, "%04hx%04hx.bsy", address->Net, address->Node);
    }

    if (had_to_punt[i])
      strcpy (flagname, flag_dir);
    strcat (flagname, BSYname);

    had_to_punt[i] = 0;
    j = i++;

    for (; i < last_count; j++, i++)
    {
      last_set[j] = last_set[i];
      had_to_punt[j] = had_to_punt[i];
    }

    last_count--;

    if (!unlink (flagname))
    {
      if (do_stat)
        status_line (MSG_TXT (M_CLEARED_FLAGFILE), flagname);

#ifdef MAKEDIR
      if (makedir)
      {                         /* TJW 960429 makedir part 3b */
#endif
        if (address->Point)
        {
          status_line (">MakeDir: Attempting to remove %s", tmpname);
          rmdir (tmpname);      /* try to remove point dir */
        }
        if (strchr (HoldName, '.') != NULL)
          /* RHo NEVER remove \outbound\ ! */
        {
          strcpy (tmpname, HoldName);
          tmpname[strlen (tmpname) - 1] = '\0';  /* remove backslash */
          status_line (">MakeDir: Attempting to remove %s", tmpname);
          rmdir (tmpname);      /* try to remove zone dir */
        }
#ifdef MAKEDIR
      }
#endif

      return (TRUE);
    }

    if (do_stat)
      status_line (MSG_TXT (M_FAILED_CLEAR_FLAG), flagname);
    return (FALSE);

  default:
    break;
  }

  return (TRUE);
}

void
forcexitcheck ()
{
  struct stat buf;
  struct FILEINFO dta;
  char fname[PATHLEN];          /* TJW 960526 static -> dynamic */
  static long exit_time = 0L;

  if (!flag_dir)
    return;

  if (exit_time && !longtimeup (exit_time))
    return;

  if (forcexit)
  {
    sprintf (fname, "%sforcexit.%02x", flag_dir, TaskNumber);

    if (!unix_stat (fname, &buf))
    {
      unlink (fname);
      screen_blank = FALSE;
      errl_exit_shell (forcexit, MSG_TXT (M_FORCED_EXIT),
                       MSG_TXT (M_FORCED_SHELL));  /* MR 961123 */
    }
  }

  sprintf (fname, "%sbtexit??.??", flag_dir);

  if (!dfind (&dta, fname, 0))
  {
    do
    {
      int sExit = 0, sTask = 0;
      int cScanned = 0;

      cScanned = sscanf (dta.name, "btexit%02x.%02x", &sExit, &sTask);
      if (cScanned && (sTask == TaskNumber))
      {
        sprintf (fname, "%s%s", flag_dir, dta.name);
        unlink (fname);
        screen_blank = FALSE;
        dfind (&dta, NULL, 2);
        errl_exit_shell (sExit, MSG_TXT (M_FORCED_EXIT),
                         MSG_TXT (M_FORCED_SHELL));  /* MR 961123 */
        break;
      }
    }
    while (!dfind (&dta, fname, 1));

    dfind (&dta, NULL, 2);
  }

  /* Check for BTFREEZE.xx */
  sprintf (fname, "%sbtfreeze.%02x", flag_dir, TaskNumber);
  if (!unlink (fname))
  {
    FILE *fp;

    sprintf (fname, "%sbtfrozen.%02x", flag_dir, TaskNumber);
    if ((fp = fopen (fname, write_binary)) != NULL)
    {
      fclose (fp);
      status_line (MSG_TXT (M_FROZEN), fname);
      freeze ();                /* closes SNServer-Pipe */
      while (!unix_stat (fname, &buf))
        time_release ();

      status_line (MSG_TXT (M_UNFROZEN));
      unfreeze ();              /* re-opens SNServer-Pipe */
    }
  }

  exit_time = longtimerset (readflagtime * PER_SECOND);  /* Don't check for 15 seconds */
}


/* TJW 960616 misc BTRESCAN.FLG / BSY changes */
int
forcerescancheck ()
{
  struct stat buf;
  char fname[PATHLEN];          /* TJW 960526 static -> dynamic */
  int dorescan = FALSE;

  // static time_t flg_time = (time_t) 0;

  if (next_time && !longtimeup (next_time))
    return (0);                 /* no rescan */

  if (!flag_dir)
    return (0);                 /* no rescan */

  sprintf (fname, "%sbtrescan.flg", flag_dir);

  if (!unix_stat (fname, &buf))
  {
    if (last_rescan_tstamp < buf.st_mtime)
    {                           /* was file modified/touched ?     */
      // flg_time = buf.st_mtime;  /* remember new timestamp       */
      dorescan = TRUE;          /* rescan!                         */
    }
  }

  sprintf (fname, "%sbtrescan.%02x", flag_dir, TaskNumber);
  dorescan = dorescan || !unix_stat (fname, &buf);

  next_time = longtimerset (readflagtime * PER_SECOND);  /* 15 seconds before next */

  return dorescan;              /* do or do not rescan */
}


/* CE 980227 check for BTENDSES.xx to terminate active sessions */
void
forcebttaskcheck (void)
{
  struct stat buf;
  char fname[PATHLEN];
  static long exit_time = 0L;

  if (!flag_dir)
    return;

  if (exit_time && !longtimeup (exit_time))
    return;

  sprintf (fname, "%sbtendses.%02x", flag_dir, TaskNumber);

  if (!unix_stat (fname, &buf))
  {
    unlink (fname);
    if (!bttask)
      status_line (MSG_TXT (M_DELETE_BTTASK), fname);
    bttask = 1;
    return;
  }

  // HJK 98/07/26
  exit_time = longtimerset (15 * PER_SECOND);  /* Don't check for 15 seconds */

  return;
}


/* CheckPath checks most important pathes of there existens while Binkley */
/* wait for an new caller. Default of checking is off.                    */

void
checkpath (void)
{
  char fname[PATHLEN];
  static long check_time = 0L;
  FILE *fp;

  if (!check_path)              /* check enabled? */
    return;

  if (check_time && !timeup (check_time))  /* check only in time intervals */
    return;

  sprintf (fname, "%spath_chk.%02x", hold_area, TaskNumber);
  fp = fopen (fname, write_binary);  /* check Outbound */
  if (fp)
    fclose (fp);
  else
    binkexit (check_path_errorlevel);
  /* if not exist, stop Binkley with errorlevel XXX immediately */

  sprintf (fname, "%spath_chk.%02x", net_info, TaskNumber);
  fp = fopen (fname, write_binary);  /* check nodelist dir */
  if (fp)
    fclose (fp);
  else
    binkexit (check_path_errorlevel);

  sprintf (fname, "%spath_chk.%02x", flag_dir, TaskNumber);
  fp = fopen (fname, write_binary);  /* check flag dir */
  if (fp)
    fclose (fp);
  else
    binkexit (check_path_errorlevel);

  sprintf (fname, "%spath_chk.%02x", DEFAULT.sc_Inbound, TaskNumber);
  fp = fopen (fname, write_binary);  /* check default inbound */
  if (fp)
    fclose (fp);
  else
    binkexit (check_path_errorlevel);

  sprintf (fname, "%spath_chk.%02x", KNOWN.sc_Inbound, TaskNumber);
  fp = fopen (fname, write_binary);  /* check known inbound */
  if (fp)
    fclose (fp);
  else
    binkexit (check_path_errorlevel);

  sprintf (fname, "%spath_chk.%02x", PROT.sc_Inbound, TaskNumber);
  fp = fopen (fname, write_binary);  /* check protected inbound */
  if (fp)
    fclose (fp);
  else
    binkexit (check_path_errorlevel);

  check_time = timerset (check_path_time * PER_SECOND);  /* Don't show for at least 30 seconds */
  return;
}

/* $Id: b_sessio.c,v 1.14 1999/09/27 20:51:19 mr Exp $ */
