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
 * Filename    : $Source: E:/cvs/btxe/src/mailroot.c,v $
 * Revision    : $Revision: 1.13 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:30 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Mail Control Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"
#include "fidomail.h"

char *estring (int, int);
static int scan_netmail (void);


/* MMP 960419 A call to movelogs() has been added for every return statement
 * in do_mail(). */

int
do_mail (ADDRP baddr, int type) /* 1 = manual, 0 = normal, -1 = CM */
{
  int i;
  int ret = 0;                  /* TJW 960526 return value */
  char junk[128];
  short CM;
  CALLHIST CallInfo;
  long callstart = 0;
  long callend;

  /* VRP 990826 start */

  int sess_result = mission_ok;
  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;
  int wday, hours, mins, secs, ths;

  /* VRP 990826 end */

  memset (&CallInfo, 0, sizeof (CallInfo));
  set_prior (PRIO_MODEM);       /* TJW 960526 */

  b_init ();
  caller = 1;

  /* MR 970803 already done in b_init:
   * got_packet = got_arcmail = got_mail = got_fax = sent_mail = 0; */

  no_WaZOO_Session = QueryNodeFlag (no_WaZOO, NOWAZOO, baddr);  /* r. hoerner */
  no_EMSI_Session = QueryNodeFlag (no_EMSI, NOEMSI, baddr);  /* r. hoerner */

  sprintf (junk, "%s", Full_Addr_Str (baddr));

  called_addr = remote_addr = *baddr;  /* structure assignment */

  if (!net_params)
  {
    status_line (MSG_TXT (M_INSUFFICIENT_DATA));
    set_xy ("");
    goto end_movelogs;
  }

  if (!nodeproc (junk))         /* sets "newnodedes" struct */
    goto end_movelogs;

  /* VRP hidden line support start */

  /*
   * Here we must deside to use main line or one of hidden one
   *
   */

  for (hidden = hidden1; hidden; hidden = hidden->next)
  {
    if (SameAddress ((ADDR *) hidden, baddr))
    {
      if (hidden->current == NULL)
      {
        /*
         * Use main (nodelist) line - no changes needed if is online now
         * In other case - try to find appropriate hidden line
         *
         */

        /* if any "Override" statement: */
        GetAltPhone ((char *) &newnodedes.PhoneNumber, baddr);
        GetAltModemflag ((word *) & newnodedes.ModemType, baddr);
        GetAltFidoFlag ((word *) & newnodedes.NodeFlags, baddr);
        if (GetAltOnLineTime (&newnodedes.online_start, &newnodedes.online_end, baddr))
        {
          newnodedes.NodeFlags |= B_CM;
        }

        /*
         * The main line isn't online now - try to find hidden one
         *
         */

        if (!(newnodedes.NodeFlags & B_CM) &&
            !IsOnLine (newnodedes.online_start, newnodedes.online_end))
        {
          hidden->current = hidden->hidden_line1;
        }
      }

      if (hidden->current != NULL)
      {
        /*
         * Use hidden line - we must find appropriate line which 
         * is now online
         *
         */

        dostime (&wday, &hours, &mins, &secs, &ths);
        wday = (short) weekdays[wday % 7];
        hidden_line = hidden->current;

        while (hidden_line)
        {
          status_line (">%d:%d/%d.%d - days %d",
                       hidden->Zone, hidden->Net, hidden->Node,
                       hidden->Point, hidden_line->days);

          if ((hidden_line->days & wday) &&
              (IsOnLine (hidden_line->start, hidden_line->end) ||
               (hidden_line->fido & B_CM)))
          {
            hidden->current = hidden_line;

            strcpy (newnodedes.PhoneNumber, hidden_line->num);
            newnodedes.ModemType = (byte) hidden_line->modem;
            newnodedes.NodeFlags = hidden_line->fido;
            newnodedes.online_start = hidden_line->start;
            newnodedes.online_end = hidden_line->end;

            break;
          }
          hidden_line = hidden_line->next;
        }

        if (hidden_line == NULL)
        {
          for (hidden_line = hidden->hidden_line1;
               hidden_line != hidden->current;
               hidden_line = hidden_line->next)
          {
            if ((hidden_line->days & wday) &&
                (IsOnLine (hidden_line->start, hidden_line->end) ||
                 (hidden_line->fido & B_CM)))
            {
              hidden->current = hidden_line;

              strcpy (newnodedes.PhoneNumber, hidden_line->num);
              newnodedes.ModemType = (byte) hidden_line->modem;
              newnodedes.NodeFlags = hidden_line->fido;
              newnodedes.online_start = hidden_line->start;
              newnodedes.online_end = hidden_line->end;

              break;
            }
          }
        }
      }

      break;
    }
  }

  if (hidden == NULL)
  {
    /* 
     * No hidden line found - use the main one
     *
     */

    GetAltPhone ((char *) &newnodedes.PhoneNumber, baddr);
    GetAltModemflag ((word *) & newnodedes.ModemType, baddr);
    GetAltFidoFlag ((word *) & newnodedes.NodeFlags, baddr);
    if (GetAltOnLineTime (&newnodedes.online_start, &newnodedes.online_end, baddr))
    {
      newnodedes.NodeFlags |= B_CM;
    }
  }

  /* VRP hidden line support end */

  if (!do_dial_strings (type))  /* r. hoerner: moved, "bittype" */
  {
    status_line (">don't know how to dial to %s", Full_Addr_Str (baddr));
    ret = 0;
    goto end_movelogs;
  }

  /* else: either no matching "ModemTrans", then we could use "PreDial", *
   * or "ModemTrans" matched and we found a predial string there         */

  // status_line (MSG_TXT (M_PROCESSING_NODE),
  // Full_Addr_Str (baddr), newnodedes.SystemName);

  if (type > 0)                 /* manual dial */
  {
    if (flag_file (TEST_AND_SET, &called_addr, 1))
    {
      if (CARRIER)
        modem_hangup ();
      goto end_movelogs;
    }

    if (CARRIER ||              /* called manually maybe? */
        (ExtMailMask && (newnodedes.ModemType == (byte) ExtMailMask)))
      goto process_the_damned_mail;  /* yup, just do some mail */

    try_2_connect ((char *) newnodedes.PhoneNumber, &called_addr);
  }
  else
  {
    if ((type == 0) && (cur_event >= 0))  /* normal dial */
    {
      /* ------------------------------------------
       * If this event is supposed to be only local
       * ------------------------------------------ */

      /* TJW 970304: this is OLD behaviour and not appropriate for XE !!!!!!!!
       * so I commented this out to see what happens ...
       *
       *  if (e_ptrs[cur_event].behavior & MAT_LOCAL)
       *  {
       *    if (e_ptrs[cur_event].node_cost < 0)
       *    {
       *      if ((int) newnodedes.RealCost < -e_ptrs[cur_event].node_cost)
       *        goto dial_aborted;
       *    }
       *    else
       *    {
       *      if ((int) newnodedes.RealCost > e_ptrs[cur_event].node_cost)
       *        goto dial_aborted;
       *    }
       *  }
       */

      /* --------------------------------------------
       * If this is a "receive only" event then abort,
       * if not a "CRASH" event, too.
       * -------------------------------------------- */

      if ((e_ptrs[cur_event].behavior & MAT_NOOUT)  /* r.hoerner 1/12/97 */
          && !(e_ptrs[cur_event].behavior & MAT_HIPRICM))
        goto dial_aborted;

      if (newnodelist || version7)
      {
        CM = newnodedes.NodeFlags & B_CM;

        /* --------------------------------------------
         * If event is no mail-to-all event and Non-CM
         * -------------------------------------------- */

        if (!(e_ptrs[cur_event].behavior & MAT_NOMAIL24) && !CM
            && !IsOnLine (newnodedes.online_start, newnodedes.online_end))  /* VRP 990820 */
          goto dial_aborted;

        /* ------------------------------------------------
         * If this event is supposed to send to Non-CM only
         * ------------------------------------------------ */

        if ((e_ptrs[cur_event].behavior & MAT_NOCM) && CM)
          goto dial_aborted;
      }
    }

    /* ------------------------------------------------
     * If we cannot set our BSY flag
     * ------------------------------------------------ */

    if (flag_file (TEST_AND_SET, &called_addr, 1))
    {
      if (CARRIER)
        modem_hangup ();
      goto end_movelogs;
    }

    if (un_attended)
    {
      sb_move (holdwin, 1, 0);
      sb_wa (holdwin, colors.calling, HOLDWIN_SIZE - 2);  /* TJW 960503 */
    }

    if (!blank_on_key)
      screen_blank = 0;

    if (ExtMailMask && (newnodedes.ModemType == (byte) ExtMailMask))
      goto process_the_damned_mail;

    i = try_1_connect ((char *) newnodedes.PhoneNumber, &called_addr);

    /* ------------------------------------------------
     * If we could not get a connection, 'i' means
     *  1 = IGNORE
     *  0 = FAILURE
     * <0 = ERROR (WAS_BUSY == -2)
     * ------------------------------------------------ */

    /* VRP hidden line support start */

    /*
     * Here we must switch to next hidden line if any
     *
     */

    if (i == FAILURE || i == WAS_BUSY || i == USER_BREAK)
    {
      for (hidden = hidden1; hidden; hidden = hidden->next)
      {
        if (SameAddress ((ADDR *) hidden, &called_addr))
        {
          if (hidden->current == NULL)
          {
            hidden->current = hidden->hidden_line1;
          }
          else
          {
            hidden->current = hidden->current->next;
          }
        }
      }
    }

    /* VRP hidden line support end */

    if (i <= 0)                 /* MR 970523 was "i < 0"                            */
    {
      if (chg_call)
      {
        change_call_to_normal (&called_addr);
        chg_call = 0;
      }

      if (un_attended)
      {
        sb_move (holdwin, 1, 0);
        sb_wa (holdwin, colors.hold, HOLDWIN_SIZE - 2);  /* TJW 960503 */
      }

      modem_init ();

      status_line (MSG_TXT (M_END_OF_ATTEMPT));
      flag_file (CLEAR_FLAG, &called_addr, 1);

      ret = i;
      goto end_movelogs;
    }
  }

process_the_damned_mail:

  if (CARRIER ||                /* if we did,        */
      (ExtMailMask && (newnodedes.ModemType == (byte) ExtMailMask)))
  {
    /* CFS 961225 - we don't reset the counter here if UseRingingCost
       is used. The counter is running since either the dial or the
       last ringing. Exception is if the counter isn't started for
       some reason */
    if (!UseRingingCost || callstart == 0)
    {
      callstart = unix_time (NULL);
      // status_line (" setting1 callstart to %08lx", callstart);
    }
    else
      callstart = cfscallstart;
#if defined(OS_2) && !defined(__EMX__)
    UpdateCID = 0;
#endif
    sess_result = b_session (1, &CallInfo);  /* do a mail session  *//* VRP 990826 */

    /* VRP 990913 start */

    if (CallInfo.calltype == CT_MAILER || CallInfo.calltype == CT_EXTMAIL)
      CallInfo.h.m.success = sess_result;

    /* VRP 990913 end */

    callend = unix_time (NULL);
    // status_line (" setting1 callend to %08lx", callend);

    b_session_cleanup (sess_result);  /* VRP 990826 */

    hist.last_Elapsed = callend - callstart;
    hist.callcost += cost_of_call (1, callstart, callend, &CallInfo);
    ++hist.connects;

    if (CallInfo.calltype == CT_MAILER)
      SaveCallInfo (CallInfo);

    if ((SaveExtSession & SAVE_EXT_OUT) && CallInfo.calltype == CT_EXTMAIL)  /* VRP 990913 */
      SaveCallInfo (CallInfo);

    write_stats ();

    if (un_attended && (got_arcmail || got_packet || got_mail))
    {
      bad_call (baddr, -1);
      receive_exit ();
    }

    if (un_attended)
    {
      sb_move (holdwin, 1, 0);
      sb_wa (holdwin, colors.hold, HOLDWIN_SIZE - 2);  /* TJW960503 */
    }

    ret = 1;
    goto end_movelogs;
  }
  else
  {
    if (chg_call)
    {
      change_call_to_normal (&called_addr);
      chg_call = 0;
    }

    status_line (MSG_TXT (M_END_OF_ATTEMPT));
    flag_file (CLEAR_FLAG, &called_addr, 1);
    modem_init ();              /* MR 970425 */
  }

  if (un_attended)
  {
    sb_move (holdwin, 1, 0);
    sb_wa (holdwin, colors.hold, HOLDWIN_SIZE - 2);  /* TJW960503 */
  }

  write_stats ();
  ret = 2;
  goto end_movelogs;

dial_aborted:

  /* r.hoerner: i like to test, whether this really can happen.
     I guess: no! */
  status_line ("!Emergency brake: No dialout in this event");

end_movelogs:

#if defined(OS_2) && !defined(__EMX__)
  UpdateCID = 1;
#endif
  movelogs ();
  do_today (1);
  set_prior (PRIO_REGULAR);     /* TJW 960526 */
  return ret;
}


int
handle_inbound_mail (int answer_now)
{
  CALLHIST CallInfo;
  long lEndTime;                /* master timeout           */
  long callstart;
  long callend;
  int mr;                       /* Modem response           */
  int iRingCount = 0;           /* for counting 'em         */
  int answered = 0;             /* already answered phone ? */
  int i;
  int sess_result = mission_ok; /* VRP 990915 */

  /* MR 970523 removed interval timer, master timeout is enough */

  memset (&CallInfo, 0, sizeof (CallInfo));
  no_WaZOO_Session = no_WaZOO;
  no_EMSI_Session = no_EMSI;
  caller = 0;

  memset ((char *) &remote_addr, 0, sizeof (remote_addr));
  remote_addr.Domain = NULL;    /* only Microsoft can explain this  */

  lEndTime = timerset (2 * PER_MINUTE);  /* Master timeout of 2 minutes      */

  /* MR 970311 removed those ugly gotos.... */

  do
  {
    /* Do we have carrier, RING, or NO DIAL TONE? */
    if (timeup (lEndTime) || ((answer_now <= 0) && (iRingCount == 0) &&
                              !(server_mode && CARRIER) && !CHAR_AVAIL ()))
    {
      time_release ();
#if defined(OS_2) && !defined(__EMX__)
      UpdateCID = 1;
#endif
      return (0);               /* No, nothing to do                */
    }

    mail_only = 1;

    if (cur_event >= 0)
      if (e_ptrs[cur_event].behavior & MAT_BBS)
        mail_only = 0;

    if (answer_now <= 0)
      while (!timeup (lEndTime) && !CHAR_AVAIL () && !KEYPRESS ())
      {
        if (answered)           /* if we answered the phone, count down seconds */
        {
          int tmp = (lEndTime - timerset (0)) / PER_SECOND;

          do_status (M_READY_ANSWCOUNT, &tmp);  /* MR 972412 */
        }

        time_release ();        /* wait for another result          */
      }

    // MR 970523 not needed, modem_response checks for ESC
    //
    //    if (KEYPRESS ())                  /* If aborted by user,              */
    //    {
    // #ifdef OS_2 /*  TS 970328, CFS please have a look if this is intentional */
    //      UpdateCID=1;
    // #endif
    //      return (1);                     /* get out                          */
    //    }

    if (server_mode && CARRIER)
    {
      mr = CONNECTED;
#if defined(OS_2) && !defined(__EMX__)
      UpdateCID = 0;
#endif
    }
    else
    {
      if (answer_now > 0)
      {
        mr = RINGING;           /* say it rang                      */
        iRingCount = ring_wait; /* we want to answer right now      */
        answer_now = -1;
      }
      else
        mr = modem_response (5 * PER_SECOND);

      if ((mr == RINGING) && (modem.answer != NULL))  /* saw RING        */
      {
        ++iRingCount;           /* no need to check an timer, we've only got 10secs */

#if defined(OS_2) && !defined(__EMX__)
        UpdateCID = 0;
#endif
        /* Unblank on the first ring */
        if ((answer_now < 0) || (iRingCount == 1))
        {
          if (!blank_on_key)
            screen_blank = 0;

          if (un_attended)
            sb_show ();
        }

        /* Answer if we've reached the correct ring number */
        if (iRingCount >= ring_wait)
        {
          char *response = NULL;
          int userbreak = 0;
          long t = timerset (PER_SECOND);

          free (CurrentCID);
          CurrentCID = NULL;

          do
          {
            if (fpnumber[0] != '\0' && CurrentCID == NULL)
            {
              CurrentCID = Get_CallerID (fpnumber);

              if (rejected_call (CurrentCID))
              {
                time_release ();
                return (0);
              }
            }

            if (CHAR_AVAIL ())
              response = get_response (t, 1, &userbreak);
            else
              response = NULL;
          }
          while (response && !(timeup (t)));

          set_banner ();
          CLEAR_INBOUND ();

          // SMS 980823 begin
          for (i = 0; i < num_ext_mail; i++)
          {
            if (stricmp ("RING", ext_mail_string[i]) == 0)
            {
              if (CurrentCID != NULL)
                strcpy (mdm_reliable, CurrentCID);
              else
                mdm_reliable[0] = '\0';
              UUCPexit (lev_ext_mail[i], 0);
              i = num_ext_mail + 1;
            }
          }

          if (i == num_ext_mail)
          {
            // no "ExtrnMail RING" => answer modem
            modem_answer ();
          }
          // SMS 980823 end

          answered = 1;

          /* MR 970523 restart master timeout */
          lEndTime = timerset (60 * PER_SECOND);
        }
        else
        {
          /* Otherwise wait up to 10 seconds for something to happen */
          /* MR 971223 answer count down */
          int tmp = ring_wait - iRingCount;  /* int, using printf %d */

          do_status (M_READY_ANSWCOUNT, &tmp);

          /* MR 970523 restart master timeout */
          lEndTime = timerset (5 * PER_SECOND);
        }
      }
      else if (mr == FAILURE)   /* If we got "No Carrier"           */
      {
        modem_hangup ();        /* MR 970523 hangup                 */
        modem_init ();          /*           and reinit modem       */
        time_release ();
#if defined(OS_2) && !defined(__EMX__)
        UpdateCID = 1;
#endif
        return (0);             /* Nothing happened...              */
      }

    }
  }                             /* then proceed along               */
  while ((mr != CONNECTED) && !(mr & FAX));

  /* MR 970311 end of rewrite... more to come ;) */

  newnodedes.RealCost = 0;      /* TJW 961008 init costs for BBS/FAX callers */
  start_hist = hist;
  callstart = unix_time (NULL);

  // status_line (" setting2 callstart to %08lx", callstart);

  set_prior (PRIO_MODEM);       /* TJW 960526 */

  if (!blank_on_key)
    screen_blank = 0;

  if (un_attended)
    sb_show ();

  /* TJW 960625 will be Inc-Fail if this was all */
  last_type (6, (ADDR *) alias);

  if (mr & FAX)
  {
    int gotfax = 1;
    int i;

    if (!fax_in)
    {
      /* We have a FAX result code but are not configured to
       * receive any. See if the user has set up a fax exit
       * using the external mail strings */

      for (i = 0; i < num_ext_mail; i++)
      {
        if (stricmp (saved_response, ext_mail_string[i]) == 0)
        {
          last_type (5, (ADDR *) alias);
          UUCPexit (lev_ext_mail[i], 0);
          goto end_fax;
        }
      }

      status_line ("!FAX: no 'FaxInDir'. Cannot receive");
    }
    else
    {
      last_type (5, (ADDR *) alias);
      gotfax = faxreceive (FAX, &CallInfo);  /* Try to get a FAX                 */
    }

  end_fax:
    modem_hangup ();            /* Then hang up                     */

    callend = unix_time (NULL);
    // status_line (" setting2a callend to %08lx", callend);

    hist.last_Elapsed = callend - callstart;
    hist.callcost += cost_of_call (0, callstart, callend, &CallInfo);

    if (gotfax)                 /* If we got one,                   */
    {
      hist.fax_calls++;         /* TJW 960903 update faxcnt in hist */

      /* AW 970531 save FAX callinfo   */
      if (fax_in && CallInfo.calltype == CT_FAX)
        SaveCallInfo (CallInfo);

      got_fax = 1;              /* Say we got mail                  */
      receive_exit ();          /* And try to exit                  */
    }

    modem_init ();              /* MR 970425 and reinitialize it    */
  }
  else
  {
    long t = timerset (1000);   /* TE 131097: some modems are quite */

    while ((!CARRIER) && (!timeup (t)))  /* slow turning DCD on, e.g. VMO  */
      time_release ();

    if (CARRIER)                /* if we have a carrier,            */
    {
      sess_result = b_session (0, &CallInfo);  /* do a mail session *//* VRP 990913 */

      /* VRP 990913 start */

      if (CallInfo.calltype == CT_MAILER || CallInfo.calltype == CT_EXTMAIL)
        CallInfo.h.m.success = sess_result;

      /* VRP 990913 end */

      callend = unix_time (NULL);
      // status_line (" setting2b callend to %08lx", callend);

      b_session_cleanup (mission_ok);  /* VRP 990824 */

      hist.last_Elapsed = callend - callstart;
      hist.callcost += cost_of_call (0, callstart, callend, &CallInfo);

      if (CallInfo.calltype == CT_MAILER)
        SaveCallInfo (CallInfo);

      if ((SaveExtSession & SAVE_EXT_IN) && CallInfo.calltype == CT_EXTMAIL)  /* VRP 990913 */
        SaveCallInfo (CallInfo);

      /* We got inbound mail */
      if (got_arcmail || got_packet || got_mail)
      {                         /* r. hoerner:                    */
        if (DelBadCall)         /* if "undialable" site called us */
        {
          bad_call (&remote_addr, -1);  /* then delete badcall status */
          for (i = 0; i < num_rakas; i++)  /* do this to all of his AKAs too */
            bad_call (&remote_akas[i], -1);  /* AG 990120 */
        }

        receive_exit ();
      }
    }
    else
    {
      modem_hangup ();          /* Try to reset modem               */

      callend = unix_time (NULL);
      // status_line (" setting2c callend to %08lx", callend);

      modem_init ();            /* MR 970424 and reinitialize it    */

      hist.last_Elapsed = callend - callstart;
      hist.callcost += cost_of_call (0, callstart, callend, &CallInfo);
    }
  }

  mailer_banner ();
  do_today (1);
  set_prior (PRIO_REGULAR);     /* TJW 960526 */

  return (1);
}

void
receive_exit ()
{
  char junk1[150];
  int i;
  BINK_EVENT evt;
  int gotany;                   /* MMP 960419 */

  if (cur_event >= 0)
    evt = e_ptrs[cur_event];

  /* MMP 960419/960507 Create BTMAIL.IN in flags directory */

  gotany = got_fax || got_mail || got_packet || got_arcmail || got_tic;  /* VRP 990918 */

  if (gotany && MailFlag)
  {
    FILE *fp;

    if ((MailFlag[0] == '%') && (MailFlag[1] == 's'))
      sprintf (junk1, MailFlag, flag_dir ? flag_dir : "");
    else
      strcpy (junk1, MailFlag);

    fp = fopen (junk1, write_binary);
    if (fp != NULL)
      fclose (fp);
  }

  /* VRP 990920 start */

  if (got_packet && NetMailFlag)
  {
    FILE *fp;

    if ((NetMailFlag[0] == '%') && (NetMailFlag[1] == 's'))
      sprintf (junk1, NetMailFlag, flag_dir ? flag_dir : "");
    else
      strcpy (junk1, NetMailFlag);

    fp = fopen (junk1, write_binary);
    if (fp != NULL)
      fclose (fp);
  }

  if (got_arcmail && ArcMailFlag)
  {
    FILE *fp;

    if ((ArcMailFlag[0] == '%') && (ArcMailFlag[1] == 's'))
      sprintf (junk1, ArcMailFlag, flag_dir ? flag_dir : "");
    else
      strcpy (junk1, ArcMailFlag);

    fp = fopen (junk1, write_binary);
    if (fp != NULL)
      fclose (fp);
  }

  if (got_tic && TicFlag)
  {
    FILE *fp;

    if ((TicFlag[0] == '%') && (TicFlag[1] == 's'))
      sprintf (junk1, TicFlag, flag_dir ? flag_dir : "");
    else
      strcpy (junk1, TicFlag);

    fp = fopen (junk1, write_binary);
    if (fp != NULL)
      fclose (fp);
  }

  /* VRP 990920 end */

  if ((aftermail != NULL) && gotany)
  {
    strcpy (junk1, aftermail);
    if (cur_event >= 0)
      strcat (junk1, evt.cmd);

    // HJK - 98/02/25 - Some changes to 'fast' aftermail
    if (aftermail[0] == '#')
    {
      memmove (&junk1[0], &junk1[1], strlen (junk1));
      i = ExecSession ((unsigned char *) junk1);
      if (i == 0)               /* this is: with OS_2 and _WIN32 */
      {
        status_line (MSG_TXT (M_LAUNCH_AFTER_MAIL));
#ifdef BINKLEY_SOUNDS
        Make_Sound (fnMailSound);
#endif
        goto aftermail_done;
      }

      aftermail[0] = ' ';
    }

    status_line (MSG_TXT (M_AFTERMAIL));
#ifdef BINKLEY_SOUNDS
    Make_Sound (fnMailSound);
#endif

    if (!quickaftermail)
      modem_busy ();

    screen_clear ();
    vfossil_cursor (1);
    strcpy (junk1, aftermail);

    if (cur_event >= 0)
      strcat (junk1, evt.cmd);

    close_up (0);
    b_spawn (junk1);
    come_back ();
    status_line (MSG_TXT (M_OK_AFTERMAIL));

    if (!quickaftermail)
    {
      modem_init ();            /* MR 970310 */
      xmit_reset (0, 0);
    }

    IPC_SetStatus (MSG_TXT (M_MCP_WAITING));
    waitfor_line = timerset (PER_MINUTE);

  aftermail_done:;
  }

  if (cur_event >= 0)
  {
    int errlvl = 0;
    int exitlvl = 0;

    if (got_arcmail && (evt.errlevel[2]))
    {
      exitlvl = evt.errlevel[2];
      if (errlvlshell[exitlvl - 1] == NULL)
      {
        errlvl = exitlvl;
        status_line (MSG_TXT (M_EXIT_COMPRESSED), errlvl);
      }
      else
      {
        status_line (MSG_TXT (M_SHELL_COMPRESSED), exitlvl);
        errl_shell (exitlvl);
        got_arcmail = got_mail = got_packet = 0;
      }
    }

    for (i = 0; i < 6; i++)
    {
      if (user_exits[i])
      {
        user_exits[i] = 0;
        exitlvl = evt.errlevel[i + 3];

        if (errlvlshell[exitlvl - 1] == NULL)
        {
          if (errlvl == 0)
          {
            errlvl = exitlvl;
            status_line (MSG_TXT (M_EXIT_AFTER_EXTENT),
                         &(evt.err_extent[i][0]), errlvl);
          }
        }
        else
        {
          status_line (MSG_TXT (M_SHELL_AFTER_EXTENT),
                       &(evt.err_extent[i][0]), exitlvl);
          errl_shell (exitlvl);
        }
      }
    }

#ifdef BINKLEY_SOUNDS
    /* If we got an exit level, it's still set even if we shelled.
       So use its presence to determine whether to make noise */

    if (exitlvl)
      Make_Sound (fnFileSound);
#endif

    /*  See if we still want to exit */
    if (errlvl)
      errl_exit (errlvl);
  }

  if (got_fax && cur_event >= 0 && (evt.faxerrlevel))
  {
#ifdef BINKLEY_SOUNDS
    Make_Sound (fnFAXSound);
#endif
    /* MR 961123 */
    got_fax = 0;
    errl_exit_shell (evt.faxerrlevel, MSG_TXT (M_EXIT_FAX), MSG_TXT (M_SHELL_FAX));
  }

  if ((got_fax || got_mail || got_packet) &&
      (cur_event >= 0) && (evt.errlevel[1]))
  {
#ifdef BINKLEY_SOUNDS
    Make_Sound (fnMailSound);
#endif
    /* MR 961123 */
    got_fax = got_mail = got_packet = got_tic = 0;  /* VRP 990918 */
    errl_exit_shell (evt.errlevel[1], MSG_TXT (M_EXIT_AFTER_MAIL),
                     MSG_TXT (M_SHELL_AFTER_MAIL));
  }

  got_arcmail = 0;
  got_packet = 0;
  got_mail = 0;
  got_fax = 0;
  got_tic = 0;                  /* VRP 990918 */
}

/* MR 961123 a little new function... */

void
errl_exit_shell (int errlvl, char *txt_exit, char *txt_shell)
{
  if (errlvlshell[errlvl - 1] == NULL)
  {
    status_line (txt_exit, errlvl);
    errl_exit (errlvl);
  }
  else
  {
    status_line (txt_shell, errlvl);
    errl_shell (errlvl);
  }
}

void
errl_exit (int n)
{
  write_sched ();
  status_line (MSG_TXT (M_BINK_END), ANNOUNCE);
  modem_busy ();
  b_sbexit ();
  gotoxy (0, SB_ROWS - 1);      /* TJW 960517 */
  cputs ("\n");                 /* TJW 960517 */

  if (vfossil_installed)
    vfossil_close ();

  if (!share)
    MDM_DISABLE ();

  closelogs ();                 /* TJW 960527 */
  no_longer_alive ();           /* MR  961123 */
  set_prior (PRIO_NORMAL);      /* SET PRIORITY BACK TO NORMAL (r. hoerner) */
  exit (n);
}

void
errl_shell (int n)
{
#ifdef  OS_2                    /*  TS 970206   */
  HEV hev = 0;
  APIRET ulrc = 0;

#endif
#ifdef _WIN32
  HANDLE hsem;
  DWORD rc;

#endif

  write_sched ();

  if (!strnicmp ("start ", errlvlshell[n - 1], 6))
  {
    status_line (MSG_TXT (M_START_ERRLVL_SESSION), n);
    close_up (0);
    b_spawn (errlvlshell[n - 1]);  /*  TS 970404, was `system ()'  */
    come_back ();
    IPC_SetStatus (MSG_TXT (M_MCP_WAITING));
    return;
  }
#ifdef OS_2
  else if (!strnicmp ("post ", errlvlshell[n - 1], 5))
  {
    PCHAR sem_name;

    status_line (MSG_TXT (M_POSTING_SEMAPHORE), n);
    sem_name = strlwr (strdup (errlvlshell[n - 1]));
    ulrc = DosCreateEventSem ((PSZ) strstr (sem_name, "\\sem32\\"), &hev, 0, 0);

    if (ulrc == ERROR_DUPLICATE_NAME)
      if (!DosOpenEventSem ((PSZ) strstr (sem_name, "\\sem32\\"), &hev))
      {
        DosPostEventSem (hev);
        DosCloseEventSem (hev);
      }

    if (ulrc == NO_ERROR)
    {
      status_line (MSG_TXT (M_NO_SEMAPHORE), strstr (sem_name, "\\sem32\\"));
      DosCloseEventSem (hev);
    }

    free (sem_name);
    return;
  }
#endif
#ifdef _WIN32                   /* AS 970316 - very late :-), sem handling for Win32 */
  else if (!strnicmp ("post ", errlvlshell[n - 1], 5))
  {
    status_line (MSG_TXT (M_POSTING_SEMAPHORE), n);

    /* instead of a semaphore, we use an Win32 Event. */
    hsem = CreateEvent (NULL, TRUE, TRUE, errlvlshell[n - 1] + 5);
    rc = GetLastError ();

    if (hsem == NULL)
      status_line (":Error creating semaphore %s", errlvlshell[n - 1] + 5);
    else
    {
      if (rc == ERROR_ALREADY_EXISTS)
      {
        status_line (">Semaphore %s already exists, flagging it", errlvlshell[n - 1] + 5);
        SetEvent (hsem);
      }
    }

    return;
  }
#endif
  else if (!strnicmp ("poll ", errlvlshell[n - 1], 5))  /* JNK 970224 */
  {
    ADDR addr;
    char *c, *s, name[31], *HoldName, junk[255];
    int i, fail;
    FILE *f;

    c = s = errlvlshell[n - 1];
    for (;;)                    /* we go forever */
    {
      i = 0;
      fail = 1;

      while (!isdigit (*c))     /* go to address */
        c++;

      while (((*c) != ' ') && ((c - s) < (char) strlen (s)))  /* copy address */
        name[i++] = *(c++);
      name[i] = '\0';

      if (find_address (name, &addr))  /* try to cath address */
        if (nodefind (&addr, 0))  /* is this node listed? */
          if (!flag_file (TEST_AND_SET, &addr, 0))  /* is this node busy? */
          {
            HoldName = HoldAreaNameMunge (&addr);
            sprintf (junk, "%s%s.clo", HoldName, Hex_Addr_Str (&addr));
            f = fopen (junk, append_binary);
            if (f != NULL)      /* try to create/open poll_file */
            {
              fclose (f);
              fail = 0;
            }

            flag_file (CLEAR_FLAG, &addr, 0);
          }

      if (fail == 0)
        status_line (MSG_TXT (M_ERRLVL_POLL), name);  /* all okidoki */
      else
        status_line (MSG_TXT (M_ERRLVL_POLL_FAIL), name);  /* tough luck */

      if ((c - s) == (char) strlen (s))  /* is the end already there? */
        break;
    }

    global_rescan ();           /* force mailer-rescan */
  }
  else if (strnicmp ("none", errlvlshell[n - 1], 4))  /* MR 961123 */
  {
    modem_busy ();
    screen_clear ();
    vfossil_cursor (1);
    close_up (0);
    b_spawn (errlvlshell[n - 1]);
    come_back ();
    status_line (MSG_TXT (M_RETURN_ERRLVL_SHELL));
    modem_init ();              /* MR 970310 */
    xmit_reset (0, 0);
    waitfor_line = timerset (PER_MINUTE);
  }
}

long
random_time (int x)
{
  int i;

  if (x == 0)
    return (0L);

  /* Number of seconds to delay is random based on x +/- 50% */
  i = (rand () % (x + 1)) + (x / 2);

  return (timerset ((unsigned int) (i * PER_SECOND)));  /*  TS970519    */
}

char *
HoldAreaNameMunge (ADDRP maddr)
{
  static char munged[PATHLEN];  /* TJW 960526 */
  register char *p, *q;
  int i;

  if (maddr == NULL)            /* if invalid address pointer   */
    return (BINKpath);          /* return at least a valid path */

  if ((maddr->Domain != my_addr.Domain) && (maddr->Domain != NULL))
  {
    /* CEH 19990514: this is no longer needed */
    /* *domain_loc = '\0'; */

    strcpy (munged, domain_area);
    q = &(munged[strlen (munged)]);
    for (i = 0; domain_name[i] != NULL; i++)
    {
      if (domain_name[i] == maddr->Domain)
      {
        if (domain_abbrev[i] != NULL)
        {
          p = domain_abbrev[i];
          while (*p)
            *q++ = *p++;
          sprintf (q, ".%03x" DIR_SEPS, maddr->Zone);

          return (munged);
        }
      }
    }
    /* if we are here, then no matching domain name was found. */
    /* let's take our Hold directory and return a name, that   */
    /* will allow to store the coming files parallel to that   */
    /* directory. In other words:                              */
    /* If our Hold-directory is  c:\binkley\outbound           */
    /* then return an name as    c:\binkley\outbound.000       */
    /*                                             (r.hoerner) */
  }

  strcpy (munged, hold_area);
  q = &(munged[strlen (munged)]);
  if (maddr->Zone != alias->Zone)
  {
    --q;
    sprintf (q, ".%03x" DIR_SEPS, maddr->Zone);
  }

  return (munged);
}

void
mailer_banner ()
{
  if (un_attended)
  {
    char junk[256];

    vfossil_cursor (0);
    // clear_filetransfer ();

    show_cs_port (cur_baud.rate_value, FALSE);  /*  TS 961122   */
    sprintf (junk, "%-2d", cur_event + 1);
    sb_move_puts (settingswin, SET_EVNT_ROW, SET_COL, junk);
  }

  set_baud (max_baud.rate_value, 0);
}

void
clear_filetransfer ()
{
  if (un_attended)
    sb_fillc (filewin, ' ');
}

char *
estring (int e, int how_big)
{
  char j[30];
  char *p, *q;
  static char ebuf[50];

  *(p = &ebuf[0]) = '\0';
  if (e >= 0)
  {
    if (e_ptrs[e].behavior & MAT_BBS)
    {
      *p++ = 'B';
      if (how_big)
        *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_CM)
    {
      *p++ = 'C';
      if (how_big)
        *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_DYNAM)
    {
      *p++ = 'D';
      if (how_big)
        *p++ = ' ';
    }

    if (how_big && (e_ptrs[e].behavior & MAT_FORCED))
    {
      *p++ = 'F';
      *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_HIPRICM)
    {
      *p++ = 'H';
      if (how_big)
        *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_NOCM)
    {
      *p++ = 'K';
      if (how_big)
        *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_LOCAL)
    {
      *p++ = 'L';
      if (how_big)
      {
        switch (e_ptrs[e].extramask & EXTR_LBHVMASK)
        {
        case EXTR_LINDEXCOST:
          *p++ = 'I';
          break;

        case EXTR_LMINUTECOST:
          *p++ = 'C';
          break;

        case EXTR_LTOTALCOST:
          *p++ = 'T';
          break;

        default:
          *p++ = '?';
          break;
        }

        switch (e_ptrs[e].extramask & EXTR_LRELMASK)
        {
        case EXTR_LEQUALS:
          sprintf (j, "=%d ", e_ptrs[e].node_cost);
          break;

        case EXTR_LLESS:
          sprintf (j, "<%d ", e_ptrs[e].node_cost);
          break;

        case EXTR_LGREATER:
          sprintf (j, ">%d ", e_ptrs[e].node_cost);
          break;

        default:
          sprintf (j, "??? ");
          break;
        }

        q = j;
        while (*q)
          *p++ = *q++;
      }
    }

    if (how_big && (e_ptrs[e].behavior & MAT_NOMAIL24))
    {
      *p++ = 'M';
      *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_NOREQ)
    {
      *p++ = 'N';
      if (how_big)
        *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_OUTONLY)
    {
      *p++ = 'S';
      if (how_big)
        *p++ = ' ';
    }

    if (e_ptrs[e].behavior & MAT_NOOUT)
    {
      *p++ = 'R';
      if (how_big)
        *p++ = ' ';
    }

    if (how_big && (e_ptrs[e].behavior & MAT_NOOUTREQ))
      *p++ = 'X';

    if (how_big && (e_ptrs[e].behavior & MAT_NOPICKUP))
      *p++ = 'P';

    if (how_big && (e_ptrs[e].extramask & EXTR_NOSOUND))
      *p++ = 'Y';
  }

  *p = '\0';
  return (ebuf);
}

void
do_status (int mnum, int *addData)
{
  char junk[256];
  char junk2[256];
  char *str = MSG_TXT (mnum);
  char *mcp_str = NULL;

  switch (mnum)
  {
  case M_FAX_RECV:
    mcp_str = MSG_TXT (M_MCP_FAX_RECV);
    break;

  case M_READY_INIT:
    mcp_str = MSG_TXT (M_MCP_INIT);
    break;

  case M_READY_WAITING:
    mcp_str = MSG_TXT (M_MCP_WAITING);
    break;

  case M_READY_HANGUP:
    mcp_str = MSG_TXT (M_MCP_HANGUP);
    break;

  case M_READY_ANSWCOUNT:
    mcp_str = MSG_TXT (M_MCP_ANSWERING);
    break;

  case M_READY_ANSWERING:
    mcp_str = MSG_TXT (M_MCP_ANSWERING);
    break;

  case M_READY_CONNECT:
    mcp_str = MSG_TXT (M_MCP_CONNECT);
    break;

  case M_READY_RESCAN:
    mcp_str = MSG_TXT (M_MCP_RESCAN);
    break;

  case M_READY_READING:
    mcp_str = MSG_TXT (M_MCP_READING);
    break;
  }

  if (mcp_str != NULL)
    IPC_SetStatus (mcp_str);

  sprintf (junk, "%-2d/%-6.6s", cur_event + 1, estring (cur_event, 0));
  sb_move_puts (settingswin, SET_EVNT_ROW, SET_COL, junk);

  if (addData != NULL)          /* MR 971223 */
  {
    sprintf (junk2, str, *addData);
    str = junk2;
  }

  sprintf (junk, "%-10s", str); /* TJW 960514 */
  sb_move_puts (settingswin, SET_STAT_ROW, SET_COL, junk);
  show_os_memfree ();
  sb_show ();
}

void
do_session (int mnum)
{
  char junk[16];

  sprintf (junk, "%-10s", MSG_TXT (mnum));
  sb_move_puts (settingswin, SET_SESSION_ROW, SET_COL, junk);
  sb_show ();
}

void
do_protocol (int mnum)
{
  char junk[16];

  sprintf (junk, "%-10s", MSG_TXT (mnum));
  sb_move_puts (settingswin, SET_PROTOCOL_ROW, SET_COL, junk);
  show_os_memfree ();
  sb_show ();
}

void
list_next_event ()
{
  int i, count;
  char *p;
  char j1[100], j2[100];


  if (cur_event >= 0 || next_event >= 0)
    clear_filetransfer ();

  if (cur_event >= 0)
  {
    p = estring (cur_event, 1);
    sprintf (j1, "%s%s%s", e_ptrs[cur_event].ename,
             strlen (e_ptrs[cur_event].ename) > 0 ? " / " : "", p);
    sprintf (j2, MSG_TXT (M_CUR_EVENT), cur_event + 1, j1);
    sb_move_puts (filewin, 0, 0, j2);
  }

  if (next_event >= 0)
  {
    i = time_to_next (0);       /* MR 970803 must be first, sets up next_event */
    p = estring (next_event, 1);
    sprintf (j1, "%s%s%s", e_ptrs[next_event].ename,
             strlen (e_ptrs[next_event].ename) > 0 ? " / " : "", p);
    sprintf (j2, MSG_TXT (M_NEXT_EVENT), next_event + 1, j1, i / 60, i % 60);
    sb_move_puts (filewin, 1, 0, j2);
  }

  count = scan_netmail ();
  if (count == 0)
    sprintf (j1, "%-5s", "-");
  else
    sprintf (j1, "%-5d", count);
  sb_move_puts (settingswin, SET_MAIL_ROW, SET_COL, j1);

  sb_show ();
}

static int
scan_netmail (void)             /* returns number of unread netmails */
{
  int count = 0;

  if (netmail == NULL)
    return count;

  if (*netmail == '$')
  {
    count = SquishScan (netmail + 1);

    if (count == -1)
    {
      free (netmail);
      netmail = NULL;
    }
  }
  else
  {
    int i, k;
    short iReadT[2];
    FILE *fpt;
    struct FILEINFO dta;
    char j[PATHLEN];

    strcpy (j, netmail);
    strcat (j, "LASTREAD");     /* Full path to LASTREAD  */

    fpt = share_fopen (j, read_binary, DENY_WRITE);  /* Open the file          */
    if (fpt != NULL)
    {
      /*
       * Try to read two records. If there are two, the first record is the
       * current pointer and the second one is the last-read. If there is
       * one record, it is the last-read.
       */

      i = fread (iReadT, sizeof (short), 2, fpt);  /* Get lastread ptr   */

      fclose (fpt);             /* Then close the file */
      if (i)
      {
        int f = 0;              /* "findfirst"        */

        k = iReadT[0];          /* Last msg read      */
        strcpy (j, netmail);
        strcat (j, "*.msg");    /* Wildcard for .MSG  */

        while (!dfind (&dta, j, f))  /* If there are any,  */
        {
          f = 1;
          if (atoi (dta.name) > k)  /* See if one's more  */
          {
            FILE *fn;
            FIDOMSG fidomsg;
            char fname[128];

            sprintf (fname, "%s%s", netmail, dta.name);

            fn = share_fopen (fname, read_binary, DENY_WRITE);
            if (fn)
            {
              i = fread ((void *) &fidomsg, 1, sizeof (fidomsg), fn);
              fclose (fn);

              if (i == sizeof (fidomsg))
              {
                /* not interested in self written mails */
                if ((fidomsg.attribute & FM_LOCAL_MSG)
                    || (fidomsg.attribute & FM_MSG_SENT))
                  continue;

                /* not interested in mails for 3rd parties */
                if (fidomsg.attribute & FM_INTRANSIT)
                  continue;

                /* not interested mails already read       */
                if (!(fidomsg.attribute & FM_MSG_RECVED))
                  count++;
              }                 /* if      */
            }                   /* if (fn) */
            else
              count++;
          }                     /* if (atoi) */
        }                       /* while (dfind) */

        if (f)
          dfind (&dta, NULL, 2);
      }                         /* If any records read from LASTREAD */
    }                           /* If we were able to open LASTREAD  */
  }                             /* If the user specified NetMail     */

  return count;
}


/* AW 980521                                                             */
/* created this function, because with maximum optimisation switches the */
/* Watcom compiler traps on the old code sequence:                       */
/*
 *  if (exact_mdm_type)
 *    match = (m->mdm == newnodedes.ModemType) ? 1 : 0;
 *  else
 *    match = (m->mdm  & newnodedes.ModemType) ? 1 : 0;
 *
 * It seems to be nice anyway to have such function, because this sequence is
 * duplicated all over the sources. */

int
ModemType_matches (byte mdm, byte b)
{
  if (exact_mdm_type)           /* binkley default */
    return (mdm == b) ? 1 : 0;

  return (mdm & b) ? 1 : 0;
}

MDM_TRNS *
find_mdm_trans (byte modembyte)
{
  MDM_TRNS *m = mm_head;

  while (m)
  {
    if (ModemType_matches (m->mdm, modembyte))
      return m;

    m = m->next;
  }

  return NULL;
}

char *
get_modem_flags (char *s)
{
  MDM_TRNS *m = mm_head;
  int flag = 0;

  s[0] = '\0';
  while (m)
  {
    if (ModemType_matches (m->mdm, newnodedes.ModemType) && m->ascii[0])
    {
      if (flag)
        strcat (s, ",");
      else
        flag = 1;
      strcat (s, m->ascii);
    }

    m = m->next;
  }

  return s;
}

/* $Id: mailroot.c,v 1.13 1999/09/27 20:51:30 mr Exp $ */
