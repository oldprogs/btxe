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
 * Filename    : $Source: E:/cvs/btxe/src/mailer.c,v $
 * Revision    : $Revision: 1.11 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:28 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Mail Control Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static int LOCALFUNC poll_node (ADDRP);
static int LOCALFUNC call_node (ADDRP, int);
static void LOCALFUNC bbs_reader (void);
static void LOCALFUNC check_timer1 (long *);
static void LOCALFUNC check_timers (long *, long *, long *);
static void LOCALFUNC init_timers (long *, long *, long *);
static void LOCALFUNC show_waiting (void);

/*--------------------------------------------------------------------------*/
/* r.hoerner: i moved some parts of unattended() to separate routines to    */
/*            make unattended() more readable and maintainable              */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*             show "living"                                      r.hoerner */
/*--------------------------------------------------------------------------*/
void
display_life (void)
{
  static long t = 0L;
  static int i = -1;

  if (!screen_blank && un_attended && wholewin != NULL)
  {
    if (timeup (t))
    {
      char j[8];

      t = timerset ((int) (PER_SECOND / 4));
      ++i;
      i &= 3;

      if (MSG_TXT (M_TICKER)[4] == '2')
        sprintf (j, "%c%02x%c", MSG_TXT (M_TICKER)[i], TaskNumber, MSG_TXT
                 (M_TICKER)[i + 5]);
      else
        sprintf (j, "%c %02x", MSG_TXT (M_TICKER)[i], TaskNumber);

      sb_move_puts (wholewin, 0, 0, j);
      if (!no_time_display)     /* HJK 99/01/22 - only if we can display it */
        timedisplay ();         /* AG 990119 show the correct time */
      sb_show ();
    }
  }
  else
    t = 0L;
}

/*--------------------------------------------------------------------------*/
/*             show "waiting"                                               */
/*--------------------------------------------------------------------------*/
static void LOCALFUNC
show_waiting (void)
{
  do_status (M_READY_WAITING, NULL);
  list_next_event ();
}

/*--------------------------------------------------------------------------*/
/* check these three timers                                                 */
/*--------------------------------------------------------------------------*/
static void LOCALFUNC
check_timers (
               long *init_timer,
               long *rescan_timer,
               long *blank_timer)
{
  if (longtimeup (*init_timer))
  {
    modem_init ();
    *init_timer = longtimerset (reinittime);  /* mtt 960416 */
  }

  if (longtimeup (*rescan_timer))  /* TJW 960526 */
  {
    set_up_outbound (0);
    *rescan_timer = longtimerset (readholdtime);
    more_mail = 1;
  }

  if (longtimeup (*blank_timer))
  {
    *blank_timer = longtimerset (blank_time);
    if (do_screen_blank)
      screen_blank = 1;
  }
  if (screen_blank && do_screen_blank)
    sb_show ();
}

/*--------------------------------------------------------------------------*/
/*             re-init these three timers                                   */
/*--------------------------------------------------------------------------*/
static void LOCALFUNC
init_timers (
              long *init_timer,
              long *rescan_timer,
              long *blank_timer)
{
  *blank_timer = longtimerset (blank_time);  /* start "blank_time" timer */
  *init_timer = longtimerset (reinittime);  /* start "reinittime" timer */
  *rescan_timer = longtimerset (readholdtime);  /* start "readholdtime" timer */

}

/*--------------------------------------------------------------------------*/
/*             check t1 timer                                               */
/*--------------------------------------------------------------------------*/
static void LOCALFUNC
check_timer1 (long *t1)
{
  // display_life();
  if (timeup (*t1))
  {
    show_waiting ();
    *t1 = timerset ((unsigned int) PER_MINUTE);  /* Set a 1 min timer  */
    IPC_Ping ();
  }
}

/*--------------------------------------------------------------------------*/
/*             load BINKLEY.BAN                                             */
/*--------------------------------------------------------------------------*/
static void LOCALFUNC
load_bannerfile (void)
{
  FILE *stream;
  char tmpfname[PATHLEN];       /* MR 961122 */

  if (BBSwelcome != NULL)
    return;

  /* MR 961122 load banner file from binks home directory and not from the
   *           current directory!
   */

  sprintf (tmpfname, "%s%s", BINKpath, PRDCT_PRFXlower ".ban");
  stream = share_fopen (tmpfname, read_binary, DENY_WRITE);

  if (stream != NULL)
  {
    size_t len;
    long filesize;

    fseek (stream, 0L, SEEK_END);
    filesize = ftell (stream);
    fseek (stream, 0L, SEEK_SET);
    len = (size_t) filesize;

    BBSwelcome = calloc (1, len);
    if (BBSwelcome)
      fread (BBSwelcome, len, 1, stream);
    fclose (stream);
  }
  else
  {
    BBSwelcome = NULL;
  }
}

int
unattended (void)
{
  int no_inbound = 0;
  short j;
  int m;
  long t1;                      /* used for the timeouts  */
  long init_timer, t = -1L;
  long blank_timer;
  long rescan_timer;            /* TJW 960526 ReadHoldTime */
  unsigned int middle_timer;
  struct _key_fnc_hdr *OldKeyFncHdr;
  word TempScanCode;
  int done = 1;                 /* if we exit with this, get out of BT      */
  int did_rescan = 0;           /* TJW 960817 rescan before dynamic end */
  int old_loglevel = loglevel;
  char junk1[150];              /* HJK 98/03/06 - For ExecSession */
  int i;

  un_attended = 1;

  comm_bits = BITS_8;
  parity = NO_PARITY;
  stop_bits = STOP_1;
  program_baud ();

  OldKeyFncHdr = KbMapSet (&UnattendedKeyFncHdr);

  ctrl_modemwin (show_modemwin);  /* MR 970216 */
  read_in_log ();
  /*
   * r.hoerner 03/12/97
   * screen_clear ();
   */
  sb_dirty ();
  mailer_banner ();
  load_bannerfile ();

#ifdef BINKLEY_SOUNDS
  Make_Sound (fnStartSound);
#endif

  /* Initialize the random number generator */
  j = (int) unix_time (NULL);
  srand ((unsigned int) j);

  status_line (MSG_TXT (M_BINK_BEGIN), ANNOUNCE);
#ifdef DOS16
  /*  TS 960722 fossil announcement   */
  fossil_announce (1);
#endif
  set_xy ("");

  set_prior (PRIO_MODEM);
  XON_DISABLE ();
  set_prior (PRIO_REGULAR);

  /* See if someone has set the forced-exit semaphore */
  forcexitcheck ();
  forcebttaskcheck ();
  checkpath ();                 /* check network environmet for valid path */

  /* Turn off forced events */
  if (noforce)
  {
    find_event ();
    noforce = 0;
  }

  if (redo_dynam)
  {
    for (j = 0; j < num_events; j++)
    {
      e_ptrs[j].behavior &= ~MAT_SKIP;
    }
    redo_dynam = 0;
  }

  /*
   * See if we should exit before initializing the modem (and therefore
   * possibly letting a call sneak through)
   */

  find_event ();
  do_status (M_READY_INIT, NULL);

  /* Make sure we have all necessary parameters and that the nodelist
   * index gets read in. If not, then we must exit right now.
   */

  if (!net_params || !nodefind (&boss_addr, 0))
    if (boss_addr.Net != 0xffff)
    {
      status_line (MSG_TXT (M_MISCONFIGURED));
      errl_exit (254);
    }

  list_next_event ();
  display_life ();

  if (!CARRIER)
  {
    modem_init ();              /* Reinit modem *//* MR 970310 */
    init_timer = longtimerset (reinittime);  /* mtt 960416 */
  }

  waitfor_line = timerset (PER_MINUTE);  /* no collisions for 1 min     */

#ifdef HEAPCHECK
  heapchk (0);
#endif

top_of_mail:

  un_attended = 1;
  no_inbound = 0;
  m = 1;

  /* As long as we don't press a key */

bad_char:

  more_mail = 1;

  while (!(KEYPRESS () || ctrlc_ctr))
  {
    forcexitcheck ();           /* Check for forced exit semaphore */
    forcebttaskcheck ();
    checkpath ();               /* check network environmet for valid path */
    find_event ();              /* See if we need to move into the next event */

    if (m)                      /* Show that we are ready */
    {
      m = 0;
      show_waiting ();
      init_timers (&init_timer, &rescan_timer, &blank_timer);
      t1 = timerset ((unsigned int) PER_MINUTE);
    }
    else
    {
      check_timer1 (&t1);
      check_timers (&init_timer, &rescan_timer, &blank_timer);
    }

    /*  variable 'no_inbound' will be TRUE if we are manually dialing out
     *  or if we are in an event where we do not want incoming stuff.
     */

    if (cur_event >= 0)
      no_inbound = (e_ptrs[cur_event].behavior & MAT_OUTONLY);
    else
      no_inbound = 1;

    if (t != -1L)
      middle_timer = (cur_event >= 0) ? e_ptrs[cur_event].wait_time : 5;
    else
      middle_timer = 10;

    if (middle_timer == 0 || middle_timer > 655)
      middle_timer = 120;

    t = random_time (middle_timer);

    while (!timeup (t) && !KEYPRESS () && !ctrlc_ctr)
    {
      forcexitcheck ();
      forcebttaskcheck ();
      checkpath ();             /* check network environmet for valid path */

      if (forcerescancheck ())
      {
        set_up_outbound (0);    /* rescan outbound */
        more_mail = 1;
        m = 1;
        /* TJW 961118 commented out, I think this was our powerdialer!
           status_line(" Jumping out because of forcerescancheck() !");
           goto immed_call;
         */
      }

      show_alive ();            /* RDH */

      find_event ();
      time_release ();

      check_timer1 (&t1);

      /* If we want inbound, see if there is any.
         If we send anything, clean up afterwards ... */

      if (!no_inbound)
      {
        /* r. hoerner
         * i removed "lstart", 'cause with this the timer "t" in fact run half
         * as fast as expected and desired (it needed 2 seconds per click)
         */
        m = handle_inbound_mail (0);
        if (m)
        {
          xmit_sameplace ();
          init_timers (&init_timer, &rescan_timer, &blank_timer);
          t1 = timerset ((unsigned int) PER_MINUTE);
          m = 0;
          continue;
        }
      }
      else if (PEEKBYTE () != (short) 0xffff)  /* READ the modem! */
      {
        int userbreak = 0;

        get_response (timerset (PER_SECOND), 0, &userbreak);
      }

      check_timers (&init_timer, &rescan_timer, &blank_timer);
    }

  immed_call:

    forcexitcheck ();           /* Check for forced exit semaphore */
    forcebttaskcheck ();
    checkpath ();               /* check network environmet for valid path */
    find_event ();              /* See if we need to move into the next event */

    /* If we have pressed a key, get out */

    if (KEYPRESS () || ctrlc_ctr)
      break;

    /* If we are not in an event, or if this is a no-outbound
       event, loop again */

    if ((cur_event < 0) || (e_ptrs[cur_event].behavior & MAT_NOOUT))
    {
      time_release ();
      continue;
    }

    if (more_mail)
    {
      set_up_outbound (0);      /* CE 011197 rescan before dialing */
      more_mail = xmit_next (&next_addr);
    }

    if (more_mail)
      m = call_node (&next_addr, no_inbound);
    else
    {
      /* No more mail to do, was it dynamic? */
      if (e_ptrs[cur_event].behavior & MAT_DYNAM)
      {
        if (!did_rescan)
        {
          int maxwait = 15;     /* emergency time out after 15 seconds */

          while (maxwait-- && (set_up_outbound (1) != 2))
            /* force and wait for rescan */
            timer (1);          /* wait a second */
          if (!maxwait)
            goto top_of_mail;

          did_rescan = 1;
          more_mail = 1;
          goto immed_call;
        }

        if (!blank_on_key)
          screen_blank = 0;

        e_ptrs[cur_event].behavior |= MAT_SKIP;
        status_line (":%s %s %d", MSG_TXT (M_END_OF),
                     MSG_TXT (M_DYNAMIC_EVENT), cur_event + 1);
        t = -1L;
        did_rescan = 0;
        goto top_of_mail;
      }
    }
  }

  screen_blank = 0;
  sb_show ();

  /* Eat the character we pressed */
  if (ctrlc_ctr || !KEYPRESS ())
  {
    /*
     * Be serious, there had to be a key pressed or we wouldn't be here I
     * know it sounds silly, but ^C will sometimes do crap like this
     */
    status_line (MSG_TXT (M_EXIT_REQUEST));
  }
  else
  {
    blank_timer = longtimerset (blank_time);  /* Set a timer  */
    j = (short) KbRemap (TempScanCode = FOSSIL_CHAR ());

    if (((unsigned short) j & F_UN_BSE) == F_UN_BSE)
    {
      switch ((unsigned short) j)
      {
      case F_UN_FUNKEY1:
      case F_UN_FUNKEY2:
      case F_UN_FUNKEY3:
      case F_UN_FUNKEY4:
      case F_UN_FUNKEY5:
      case F_UN_FUNKEY6:
      case F_UN_FUNKEY7:
      case F_UN_FUNKEY8:
      case F_UN_FUNKEY9:
      case F_UN_FUNKEY10:
        /* MR 961123 use ErrLevelShell for function keys, too */
        j = (1 + (short) ((unsigned short) j - (unsigned short)
                          F_UN_FUNKEY1)) * 10;
        errl_exit_shell (j, MSG_TXT (M_FUNCTION_KEY), MSG_TXT (M_FUNCTION_KEY));  /* MR 961123 */
        break;

      case F_UN_BLANKSCREEN:
        if (do_screen_blank)
        {
          screen_blank = 1;
          sb_show ();
        }
        break;

      case F_UN_CLEARHIST:
        {
          long hist_date;

          hist_date = hist.which_date;
          memset (&hist, 0, sizeof (HISTORY));
          start_hist = hist;
          hist.which_date = hist_date;
          do_today (1);
        }
        break;

      case F_UN_MSGEDITOR:
        bbs_reader ();
        m = 1;
        break;

      case F_UN_GETFILE:
        j = sb_popup (10, 5, 9, 70, Do_Get, 1);  /* RDH *** 26.May.1996 (2 Lines more please ) */
        if (j < 0)
        {
          status_line (MSG_TXT (M_NO_GET));
        }
        else if (j > 0)
        {
          set_up_outbound (1);
          m = 1;
          more_mail = 1;
        }
        break;

      case F_UN_REINITMODEM:
        /* The idea for this key came from Holger Schurig */
        modem_init ();          /* MR 970310 */
        m = 1;
        /* TJW 960421 thrown out outbound rescan, see fn below / Alt-O */
        break;

      case F_UN_RESCANOUTBOUND:  /* TJW 960421 Alt-O */
        set_up_outbound (1);
        m = 1;
        more_mail = 1;
        break;

      case F_UN_SHELL:
        status_line (MSG_TXT (M_SHELLING));
        modem_busy ();
        gotoxy (0, SB_ROWS - 1);
        cputs ("\n");
        vfossil_cursor (1);
        cputs (MSG_TXT (M_TYPE_EXIT));
        change_prompt ();
        close_up (0);
        b_spawn (NULL);
        come_back ();
        status_line (MSG_TXT (M_BINKLEY_BACK));
        modem_init ();
        set_up_outbound (0);
        IPC_SetStatus (MSG_TXT (M_MCP_WAITING));
        m = 1;
        break;

      case F_UN_KILLNODESMAIL:
        if (sb_popup (10, 5, 4, 70, Do_Kill, 1))
        {
          status_line (MSG_TXT (M_NO_KILL));
        }
        else
        {
          set_up_outbound (1);
          m = 1;
          more_mail = 1;
        }
        break;

      case F_UN_POLLBOSS:
        poll_node (&boss_addr);
        m = 1;
        break;

      case F_UN_ANSWER:
        modem_answer ();
        break;

      case F_UN_POLLNODE:
        poll_node ((ADDRP) NULL);
        m = 1;
        break;

      case F_UN_POLLPKT:
        j = sb_popup (10, 5, 4, 70, Do_Poll_Packet, 1);
        if (j > 0)
        {
          set_up_outbound (1);
          m = 1;
          more_mail = 1;
        }
        break;

      case F_UN_QUITTHISEVENT:
        if (cur_event >= 0)
          e_ptrs[cur_event].behavior |= MAT_SKIP;
        goto top_of_mail;

      case F_UN_RESTARTEVENTS:
        for (j = 0; j < num_events; j++)
        {
          /* Don't redo forced events */
          if (!(e_ptrs[j].behavior & MAT_FORCED))
          {
            e_ptrs[j].last_ran_date = -1;
            e_ptrs[j].behavior &= ~MAT_SKIP;
          }
        }
        goto top_of_mail;

      case F_UN_SENDFILE:
        j = sb_popup (10, 5, 6, 70, Do_Send, 1);
        if (j < 0)
        {
          status_line (MSG_TXT (M_NO_SEND));
        }
        else if (j > 0)
        {
          set_up_outbound (1);
          m = 1;
          more_mail = 1;
        }
        break;

      case F_UN_TERMINALMODE:
        status_line (MSG_TXT (M_ENTER_TERMINAL_MODE));
        done = 0;               /* We won't exit now */
        goto mail_done;

#ifdef BBS_INTERNAL
      case F_UN_BBSLOCALMODE:
        status_line (":Entering internal BBS (local mode)");
        bbsinternal (1);
        status_line (":Returned from BBS (local mode)");
        break;
#endif

      case F_UN_REPAINTSCREEN:
        screen_clear ();
        sb_dirty ();
        sb_show ();
        break;

      case F_UN_HELPSCREEN:
        mailer_help ();
        m = 1;
        break;

      case F_UN_EXITBINK:
        status_line (MSG_TXT (M_EXIT_REQUEST));
        goto mail_done;

      case F_UN_ZOOM:
        if (mail_top == NULL)
        {
          xmit_reset (0, 0);
          if (mail_top == NULL)
            break;
          m = 1;
          more_mail = 1;
          next_mail = mail_top;
          xmit_window (next_mail);
        }

        no_time_display = 1;    /* HJK 99/01/24 - Turn off timedisplay */
        if (sb_popup (1, 0, (short) (SB_ROWS - 2), 80, Do_Zoom, 0))
        {
          m = 1;
          more_mail = 1;
          next_mail = mail_top;
          xmit_window (next_mail);
        }

        no_time_display = 0;    /* HJK 99/01/24 - Turn on timedisplay */
        set_up_outbound (1);
        break;

      case F_UN_EVENTOVRD:     /* CFS 080397 - Event behaviour override screen */
        sb_popup (8, 0, 14, 80, Do_Event, 0);
        break;

      case F_UN_CALLHIST:      /* CFS 150397 - Last calls history screen */
        no_time_display = 1;    /* HJK 990124 - Turn off timedisplay */
        sb_popup (1, 0, ((SB_ROWS - 3) / 3) * 3 + 2, SB_COLS,
                  Do_CallHistory, 0);
        no_time_display = 0;    /* HJK 990124 - Turn on timedisplay */

        break;

      case F_UN_STATS:         /* CFS 160497 - Stats screen */
        no_time_display = 1;    /* HJK 990124 - Turn off timedisplay */
        sb_popup (1, 0, SB_ROWS - 2, 80, Do_Stats, 0);
        no_time_display = 0;    /* HJK 990124 - Turn on timedisplay */
        break;

      case F_UN_SHELL1:
      case F_UN_SHELL2:
      case F_UN_SHELL3:
      case F_UN_SHELL4:
      case F_UN_SHELL5:
      case F_UN_SHELL6:
      case F_UN_SHELL7:
      case F_UN_SHELL8:
      case F_UN_SHELL9:
        j = (short) ((unsigned short) j - (unsigned short) F_UN_SHELL1);
        if (shells[j] != NULL)
        {
          strcpy (junk1, shells[j]);
          /* HJK - 98/03/06 - Allow fast shell via ExecSession */
          if (shells[j][0] == '#')
          {
            memmove (&junk1[0], &junk1[1], strlen (junk1));
            i = ExecSession ((unsigned char *) junk1);
            if (i == 0)         /* this is: with OS_2 and _WIN32 */
            {
              status_line (MSG_TXT (M_LAUNCH_KEYBOARD_SHELL), j + 1);
              goto un_shell_done;
            }
            shells[j][0] = ' ';
          }

          status_line (MSG_TXT (M_KEYBOARD_SHELL), j + 1);
          modem_busy ();
          gotoxy (0, SB_ROWS - 1);
          vfossil_cursor (1);
          close_up (0);
          b_spawn (shells[j]);
          come_back ();
          status_line (MSG_TXT (M_END_KEYBOARD_SHELL));
          m = 1;
          modem_init ();        /* MR 970310 */
          set_up_outbound (0);
          waitfor_line = timerset (PER_MINUTE);
        }
        else
          status_line (MSG_TXT (M_NO_KEYBOARD_SHELL), j + 1);
      un_shell_done:
        break;

      case F_PEND_PGUP:
        if (next_mail == NULL)
          next_mail = mail_top;

        if (next_mail != NULL)
        {
          for (j = 1; j < SB_ROW_HOLD; j++)
          {
            if (next_mail->ptrval.ptrnp.prev != NULL)
              next_mail = next_mail->ptrval.ptrnp.prev;
          }

          xmit_window (next_mail);
        }
        break;

      case F_PEND_PGDN:
        if (next_mail == NULL)
          next_mail = mail_top;

        if (next_mail != NULL)
        {
          for (j = 1; j < SB_ROW_HOLD; j++)
          {
            if (next_mail->ptrval.ptrnp.next != NULL)
              next_mail = next_mail->ptrval.ptrnp.next;
          }

          xmit_window (next_mail);
        }
        break;

      case F_PEND_UPAR:
        if (next_mail == NULL)
          next_mail = mail_top;

        if (next_mail != NULL)
        {
          if (next_mail->ptrval.ptrnp.prev != NULL)
            next_mail = next_mail->ptrval.ptrnp.prev;
          xmit_window (next_mail);
        }
        break;

      case F_PEND_DNAR:
        if (next_mail == NULL)
          next_mail = mail_top;

        if (next_mail != NULL)
        {
          if (next_mail->ptrval.ptrnp.next != NULL)
            next_mail = next_mail->ptrval.ptrnp.next;
          xmit_window (next_mail);
        }
        break;

      case F_PEND_HOME:
        next_mail = mail_top;
        xmit_window (next_mail);
        break;

      case F_PEND_END:
        if (next_mail == NULL)
          next_mail = mail_top;

        if (next_mail != NULL)
        {
          while (next_mail->ptrval.ptrnp.next != NULL)
            next_mail = next_mail->ptrval.ptrnp.next;
        }

        xmit_window (next_mail);
        break;

      case F_UN_CALLRIGHTNOW:
        if (cur_event >= 0)
          if (e_ptrs[cur_event].behavior & MAT_NOOUT)
          {
            status_line (MSG_TXT (M_NO_CALLS_NOW));
            goto immed_call;
          }

        status_line (MSG_TXT (M_IMMEDIATE_CALL));
        m = 0;
        more_mail = 1;
        goto immed_call;

      case F_CALLWIN_DNAR:
        callwin_dnar ();
        break;

      case F_CALLWIN_UPAR:
        callwin_upar ();
        break;

      case F_CALLWIN_PGDN:
        callwin_pgdn ();
        break;

      case F_CALLWIN_PGUP:
        callwin_pgup ();
        break;

      case F_CALLWIN_TOP:
        callwin_top ();
        break;

      case F_CALLWIN_END:
        callwin_end ();
        break;

      case F_UN_RAS:           /* TS 971014    recent activity search  */
        sb_popup (3, 5, 4, 70, Do_Ras, 0);
        break;

      case F_UN_TOGGLEDEBUG:
        debugging_log = !debugging_log;
        if (!debugging_log)
          loglevel = old_loglevel;
        else
        {
          old_loglevel = loglevel;
          loglevel = 6;
        }
        status_line (" debug mode now %s", debugging_log ? "enabled" :
                     "disabled");
        break;

      case F_UN_TOGGLEMODEMW:
        show_modemwin = !show_modemwin;
        ctrl_modemwin (show_modemwin);
        break;

      default:
        status_line (MSG_TXT (M_JUNK_CHARACTER));
        m = 1;
        break;
      }
    }
    else
    {
      switch (toupper (j & 0xff))
      {
      case 3:
        status_line (MSG_TXT (M_EXIT_REQUEST));
        goto mail_done;

      case 0x20:
        m = 1;
        break;

      default:
        status_line (MSG_TXT (M_JUNK_CHARACTER));
        m = 1;
        break;
      }
    }

    goto bad_char;
  }

mail_done:

  write_sched ();
  status_line (MSG_TXT (M_BINK_END), ANNOUNCE);
  un_attended = 0;

  set_prior (PRIO_MODEM);
  XON_ENABLE ();
  set_prior (PRIO_REGULAR);
  KbMapSet (OldKeyFncHdr);

  return (done);
}

static void LOCALFUNC
bbs_reader (void)
{
  if (BBSreader != NULL)
  {
    vfossil_cursor (1);
    status_line (MSG_TXT (M_DISABLE_MODEM));
    modem_busy ();
    status_line (MSG_TXT (M_BEGIN_MESSAGE_READER));
    close_up (0);
    screen_clear ();            /* DR 971022 */
    b_spawn (BBSreader);
    come_back ();
    status_line (MSG_TXT (M_END_MESSAGE_READER));
    // set_up_outbound (0);
    set_up_outbound (1);        /* DR 971022 */
    more_mail = 1;
    status_line (MSG_TXT (M_ENABLE_MODEM));
    modem_init ();              /* MR 970310 */
    waitfor_line = timerset (PER_MINUTE);
  }
  else
  {
    set_xy (NULL);
    status_line (MSG_TXT (M_NO_MESSAGE_READER));
    set_xy (NULL);
  }
}

/*
 * Poll a node. If address is supplied, use it. Otherwise,
 * make a popup and ask for it.
 */

static int LOCALFUNC
poll_node (ADDRP address)
{
  char junk[128];

  if (address == (ADDRP) NULL)
  {
    status_line (MSG_TXT (M_POLL_MODE));
    if (!sb_popup (10, 5, 4, 70, Do_Poll, 1))
    {
      status_line (MSG_TXT (M_NODE_NOT_FOUND));
      return (-1);
    }
  }
  else
    next_addr = *address;

  doing_poll = 1;
  clear_filetransfer ();
  sprintf (junk, "%s %s", MSG_TXT (M_CURRENTLY_POLLING),
           Full_Addr_Str (&next_addr));
  sb_move_puts (filewin, 0, 0, junk);

  if ((do_mail (&next_addr, 1) == 1) && sent_mail)
  {
    next_mail = find_mail (&next_addr);
    bad_call (&next_addr, -1);
    xmit_delete ();
  }

  doing_poll = 0;
  status_line (MSG_TXT (M_POLL_COMPLETED));
  mailer_banner ();
  clear_filetransfer ();
  // HJK - 99/01/24 - Too much modeminits, so removed this one
  //modem_init ();                /* MR 970310 */
  return (0);
}

/*
 * Call a node. If we get through, do call accounting. If
 * we have a dialout collision, try to get the inbound.
 */

static int LOCALFUNC
call_node (ADDRP address, int no_inbound)
{
  int m;

  /* save the next call in the list in case we exit */

  if ((next_mail == NULL) || (next_mail->ptrval.ptrnp.next == NULL))
    hist.next_addr = mail_top->mail_addr;
  else
    hist.next_addr = next_mail->ptrval.ptrnp.next->mail_addr;

  /* If this is a CM call, tell do_mail not to pay
     attention to the nodelist flags! */

  if ((e_ptrs[cur_event].behavior & (MAT_CM | MAT_HIPRICM))
      && (next_mail->mailtypes & MAIL_CRASH))
    m = -1;
  else
    m = 0;

  // waitfor_line=timerset (PER_MINUTE); /* MR 970717 this always disabled caller collision recovery !!! */
  m = do_mail (address, m);

  /*
   * If there was a session, handle bad-call accounting
   * based on whether the session was a complete success
   * or not. Then take an exit if appropriate, based on
   * what actually arrived here.
   */
  switch (m)
  {
  case 1:                      /* There was a good session */
    if (!sent_mail)
    {
      /* We connected but the transfer didn't work */
      bad_call (address, 1);
    }
    else
    {
      /* We got through, so delete his status file */
      bad_call (address, -1);
      xmit_delete ();
    }

    /* Take an exit if we got something in */
    if (got_arcmail || got_packet || got_mail)
    {
      set_up_outbound (1);      /* TJW 960722 only setup outbd if mail proc'd */
      receive_exit ();
    }
    mailer_banner ();
    break;

  case 2:                      /* Nothing happened */
    bad_call (address, 2);

    // TJW970530 does this make sense ??? :
    /* Take an exit if we got something in */
    if (got_arcmail || got_packet || got_mail)
    {
      set_up_outbound (1);      /* TJW 960722 only setup outbd if mail proc'd */
      receive_exit ();
    }
    mailer_banner ();
    break;

  case WAS_INCOMING:
    status_line (MSG_TXT (M_INCOMING_CALL));

    /* Call collision or NO-DIAL-TONE. Try to answer the
       call in time to catch the remote. Exception: if it
       has been less than a minute since unbusying the line. */

    if (!no_inbound && timeup (waitfor_line))
    {
      m = handle_inbound_mail (1);
      if (m)
      {
        xmit_sameplace ();
        do_status (M_READY_WAITING, NULL);
        list_next_event ();
        do_today (1);
      }
    }
    break;

  case WAS_BUSY:               /* node was busy */
    bad_call (address, 2);
    break;

  case FAILURE:
    bad_call (address, 2);
    break;

  default:
    break;
  }

  return (m);
}

/* $Id: mailer.c,v 1.11 1999/09/27 20:51:28 mr Exp $ */
