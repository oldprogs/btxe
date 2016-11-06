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
 * Filename    : $Source: E:/cvs/btxe/src/mdm_proc.c,v $
 * Revision    : $Revision: 1.8 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:33 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Modem Handler Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static int LOCALFUNC dial_modem (char *, ADDRP);
static void LOCALFUNC phone_translate (char *, char *);
static int LOCALFUNC parse_response (char *);
static void LOCALFUNC empty_delay (void);
static void LOCALFUNC MNP_Filter (void);
static void LOCALFUNC afterconnect (void);
static int LOCALFUNC wait_ok (int t);
static int LOCALFUNC get_cause_value (int);
static void LOCALFUNC get_dce (char *);

static char response_string[128];
static long last_cost = 0;

/* r. hoerner
 * the user may configure either nothing or "bittype" modem flags
 * here we have to distinguish between those two types */

int
do_dial_strings (int type)
{
  MDM_TRNS *m;
  int match = 1;
  char junk[80];

  modem.predial = modem.normprefix;
  modem.postdial = modem.normsuffix;

  if (debugging_log)
    status_line (">remote modemflags '%s'", get_modem_flags (junk));

  if ((m = find_mdm_trans (newnodedes.ModemType)) != NULL)
  {
    modem.predial = m->pre;
    modem.postdial = m->suf;
    status_line (">match: 1");
  }

  if (modem.predial == NULL)
    match = 0;
  else if (strlen (modem.predial) == 0)
    match = 0;
  else
    match = 1;

  if ((type > 0) && !match)
  {
    modem.predial = modem.normprefix;
    modem.postdial = modem.normsuffix;
    match = 1;
  }

  status_line (">Dialstring: '%s'<number>'%s'", modem.predial, modem.postdial);

  return (match);
}


void
try_2_connect (char *phnum, ADDRP addr)
{
  long t1;
  int j;
  int k;
  char junk[128];

  /* do polltries or till keypress */
  for (j = 0; (j < poll_tries); j++)
  {
    show_session (MSG_TXT (M_MCP_DIALING), addr);

    k = dial_modem (phnum, addr);

    if (un_attended || doing_poll)
    {
      ++hist.calls_made;
      do_today (0);
    }

    if (k > FAILURE)            /* CARRIER                          */
    {
      break;
    }
    else if (k == USER_BREAK || (k == FAILURE && !doing_poll))
    {
      if (!CARRIER)             /* Abort if no carrier              */
      {
        status_line (MSG_TXT (M_CONNECT_ABORTED));
        modem_hangup ();
      }
      break;
    }
    else if (k == WAS_INCOMING) /* r. hoerner: no dialtone          */
    {                           /*             or call collision    */
      do_today (M_STATUS_OFFLINE);
      break;
    }

    modem_hangup ();

    if (ReDialTime)
      t1 = longtimerset (ReDialTime);  /* r. hoerner */
    else
      t1 = longtimerset (2 * PER_SECOND);

    while (!longtimeup (t1) && !KEYPRESS ())
    {
      long t = timerset (PER_SECOND);

      sprintf (junk, "%s%3ld",
               MSG_TXT (M_READY_WAITING),
               (t1 - longtimerset (0)) / PER_SECOND);
      sb_move_puts (settingswin, SET_STAT_ROW, SET_COL, junk);
      sb_show ();

      while (!timeup (t))
        time_release ();
    }

    // HJK 980205 - Quit polling when ESC has been pressed
    if (KEYPRESS ())
      if (got_ESC ())
        break;
  }

  modem.predial = modem.normprefix;
  modem.postdial = modem.normsuffix;

  do_status (M_READY_WAITING, NULL);
}

int
try_1_connect (char *phnum, ADDRP addr)
{
  int k;

  show_session (MSG_TXT (M_MCP_DIALING), addr);

  /*  2 = CONNECTED */
  /* (1 = IGNORE)   */
  /*  0 = FAILURE   */
  /* <0 = ERROR     */
  if ((k = dial_modem (phnum, addr)) <= FAILURE)
  {
    /* get the cause-string at eg. "Busy/Cause=34Be" */

    if (k == USER_BREAK)
    {
      status_line (MSG_TXT (M_CONNECT_ABORTED));
    }
    else if (ModemReject)
    {
      if (strstr (response_string, ModemReject))
        chg_call = 1;
      else
        chg_call = get_cause_value (k);  /* the cause-line via AT??? command */
    }

    if (k != WAS_INCOMING)      /* NO DIALTONE needs express service */
      modem_hangup ();
    else
      do_today (M_STATUS_OFFLINE);
  }

  if (un_attended || doing_poll)
  {
    ++hist.calls_made;
    do_today (0);
  }

  modem.predial = modem.normprefix;
  modem.postdial = modem.normsuffix;

  do_status (M_READY_WAITING, NULL);
  return (k);
}

static void LOCALFUNC
phone_translate (char *number, char *translated)
{
  PN_TRNS *p;
  char *c, *c2;

  strcpy (translated, number);
  for (p = pn_head; p != NULL; p = p->next)
  {
    if (strncmp (p->num, number, (unsigned int) (p->len)) == 0)
    {
      sprintf (translated, "%s%s%s", p->pre, &(number[p->len]), p->suf);
      break;
    }
  }

  if (cIPRepChar)               /* TE 971014 */
    for (c = translated; *c; c++)
      if (*c == cIPRepChar)
      {
        for (c2 = c + strlen (c); c2 > c; c2--)
          *(c2 + 1) = *c2;
        *c = '\\';
        *(c + 1) = '.';
      }

  status_line (">phone string: %s", translated);  /* r. hoerner */
}

static int LOCALFUNC
dial_modem (char *numbertocall, ADDRP addrp)
{
  int count, resp;
  int retval = FAILURE;
  int did_print;
  int userbreak;
  long t;
  long ring_counter;
  char modemstr[256];
  char translated_string[128];
  char *connect_response = NULL;
  char *translated = translated_string;

  /* the problem with the former code was that, that you could not gain
   * the modem response without changing the translated string.
   * r. hoerner */

  P_ANUMS altnums = anum_head;
  char *number = numbertocall;
  int dialmsg = M_DIALING_NUMBER;
  ADDR addr;

  if (addrp == NULL)
  {
    sprintf (newnodedes.SystemName, "unknown system");
    addr.Zone = 0;
    addr.Net = 0;
    addr.Node = 0;
    addr.Point = 0;
    addr.Domain = NULL;
  }
  else
    addr = *addrp;

  janus_OK = 0;
  ARQ_lock = 0;

  CLEAR_INBOUND ();

  if (numbertocall == NULL)     /* e.g. Hold nodes have 0-length number! */
  {
    status_line ("!number to dial is NULL");
    return (FAILURE);           /* TJW970305 was: WAS_INCOMING */
  }

  if (!strlen (numbertocall))
  {
    status_line ("!No number to dial");
    return (FAILURE);           /* TJW970305 was: WAS_INCOMING */
  }

  for (;;)
  {
    phone_translate (number, translated);
    if (translated[0] == '\"')  /* If it's a script          */
    {
      retval = do_script (translated);  /* then do it that way       */
      bottom_line (0);          /* clean up the bottom line  */
      return (retval);          /* return result             */
    }

    status_line (MSG_TXT (dialmsg), translated, Full_Addr_Str (&addr),
                 newnodedes.SystemName);

    if (!no_collide && CHAR_AVAIL ())  /* if we have something on line */
      return (WAS_INCOMING);    /* don't hang up on the guy!    */

    mdm_cmd_string (modem.dial_setup, 1);

    if (!no_collide && CHAR_AVAIL ())  /* Is there something? return */
      return (WAS_INCOMING);

    do_status (M_READY_DIALING, NULL);  /* now it is true */

    sprintf (modemstr, "%s%s%s", modem.predial == NULL ? "" : modem.predial,
             translated == NULL ? "" : translated,
             modem.postdial == NULL ? "" : modem.postdial);
#if defined(OS_2) && !defined(__EMX__)
    UpdateCID = 0;
    strcpy (CIDFromCAPI, translated);  /* So it can be added to CONNECT */
#endif
    mdm_cmd_string (modemstr, 0);

    if (no_collide)
      CLEAR_INBOUND ();         /* Throw out all echo to now */

    mdm_cmd_char ('|');         /* terminate the string      */

    cfscallstart = unix_time (NULL);
    count = MaxRingWait;
    ring_counter = ring_tries;
    resp = IGNORE;
    did_print = 0;

    if (ring_counter < 1)
      ring_counter = 1;         /* force connect_response */

    if (count < 1)
      count = 1;                /* to be set!             */

    while ((resp == IGNORE || resp == RING || resp == COSTRESET)
           && count
           && ring_counter)
    {
      userbreak = 1;            /* MR 970515 allow user breaks */
      connect_response = get_response (timerset (PER_SECOND), 0, &userbreak);

      if (userbreak)            /* MR 970515 did user break?   */
        resp = USER_BREAK;
      else
        resp = parse_response (connect_response);

      do_status (M_READY_DIALCOUNT, &count);

      if (resp == RING || (!UseModemRing && UseRingingCost && resp == COSTRESET))
      {
        status_line (">ring_counter : %u", ring_counter--);
        if (loglevel < 6)       /* r.hoerner: else already printed! */
          status_line (" '%s'", connect_response);
        did_print = 1;
      }
      else
        did_print = 0;

      if (resp == COSTRESET)
        cfscallstart = unix_time (NULL);

      count--;
    }

    if (!did_print && *connect_response != '\0')
      status_line (" '%s'", connect_response);

    if (resp == CONNECTED)
    {
      get_dce (connect_response);
      if (modem.afterconnect == NULL)
        MNP_Filter ();
      else
        afterconnect ();
    }

    switch (resp)               /* r. hoerner: rearranged to switch() format 970108 */
    {                           /* and handle "RING" response as INCOMING */
    case IGNORE:               /*  Trash seen 'til timeout  */
    case FAILURE:
      return (FAILURE);         /* Error seen                */

    case USER_BREAK:
      return (USER_BREAK);

    case RINGING:              /* "RING" seen,impossible  */
    case INCOMING:
      return (WAS_INCOMING);    /* "NO DIALTONE" seen      */

    case RING:
      modem_hangup ();          /* "RINGING" 'til MaxRingWait */

    case DIALRETRY:
      // JH, 980817
      // before checking for AltNumbers, check for a reject:
      if (ModemReject)
      {
        if (strstr (response_string, ModemReject))
          chg_call = 1;
        else
          /* the cause-line via AT??? command */
          chg_call = get_cause_value (WAS_BUSY);
      }

      /* "BUSY" and no alternate -or- rejected */
      if ((altnums == NULL) || chg_call)
        return (WAS_BUSY);      /* number: tell it        */

      number = NULL;            /* precondition next number */
      /* Scan alternate number list for another match */
      while (altnums != NULL)
      {
        status_line (">AltNum-1:%s, AltNum-2:%s",
                     altnums->num, altnums->alt);

        if (!stricmp (altnums->num, numbertocall))
        {
          number = altnums->alt;
          altnums = altnums->next;
          dialmsg = M_DIALING_ALTNUM;
          CLEAR_INBOUND ();
          break;
        }
        else
          altnums = altnums->next;
      }

      if (number == NULL)       /* no alternate number found: */
        return (WAS_BUSY);      /* return BUSY, else dial */
      break;

    default:
      t = timerset (2 * PER_SECOND);  /* Wait up to 2 seconds */
      // HJK 98/02/24 - Now primarily waits for carrier, better for FDsystem connects
      while (!timeup (t) && !CARRIER)
      {
        if (CHAR_AVAIL ())      /* some life      */
          break;                /* leave early...      */
        else
          time_release ();
      }
      return (CARRIER);         /* Carrier should be on now  */
    }

    /* If we get here we want out of the loop.

     * No, we DO want to CONTINUE!
     * AltNums could be found!      (r.hoerner)
     *
     * break; */
  }

#ifndef __IBMC__                /* VAC 3.00     complains           */
#ifndef __WATCOMC__             /* watcom 10.0b complains           */
#ifndef __TURBOC__              /* BC 3.1 does the same             */
  return (retval);              /* This line will never be reached! */
#endif
#endif
#endif
}

char *
get_response (long end_time, int show, int *user)  /* show = 0 or 1 */
{                               /* user = 0 or 1 */
  char *p = response_string;    /* points to character cell  */
  char c;                       /* current modem character   */
  int count = 0;                /* count of characters       */

#ifdef OS_2
  int x = 0;

#endif
  int userbreak = 0;            /* MR 970515                 */

  if (NoModem)
    return ("OK");

  /* until we have 127 chars, or ran out of time but have no chars */
  /* left in buffer (MR 970515)                                    */

  while ((count < 127) && !(timeup (end_time) && !CHAR_AVAIL ()))
  {
    /* Restyled next routine to try to make Win32 version not to
     * hang anymore sometimes when waiting for modem response
     * - HJK 98/04/29 */

    if (*user)                  /* if user is able to interrupt */
    {
      if (KEYPRESS ())          /* if user gets impatient */
      {
        if (got_ESC ())
        {
          userbreak = 1;
          break;
        }
      }
    }

    if (!CHAR_AVAIL ())         /* if nothing ready yet,     */
    {
      time_release ();
      continue;                 /* just process timeouts     */
    }

    c = (char) (MODEM_IN () & 0xff);  /* get a character           */
    print2modemwin (MW_RCVD, c);  /* MR 970216 print 2 modemwin */

    if (c == '\r' || c == '\n') /* if a line ending          */
    {
      if (count != 0)           /* and we have something,    */
      {
        if (CHAR_AVAIL () && (PEEKBYTE () == '\r' || PEEKBYTE () == '\n'))
          MODEM_IN ();

        break;                  /* get out                   */
      }
      else
        continue;               /* otherwise just keep going */
    }

    if (count || (c != ' '))    /* r. hoerner: ignore leading blanks */
    {
      *p++ = c;                 /* store the character       */
      ++count;                  /* increment the counter     */
    }
  }

  *user = userbreak;
  *p = '\0';                    /* terminate the new string  */

#if defined(OS_2) && !defined(__EMX__)
  for (x = 0; x < AddCIDToNumber; x++)
  {
    if (strstr (response_string, AddCIDTo[x]) == response_string)
    {
      strcat (response_string, "/ID=");
      strcat (response_string, CIDFromCAPI);
      break;
    }
  }
#endif

  perform_rep (response_string, 1);
  if (strlen (response_string) > 0)
  {
    if (show || (loglevel > 5)) /* hoerner: enable debug */
    {
      if (count)
      {
        if ((strnicmp (response_string, "AT", 2))  /* r. hoerner: no ECHOs  */
            && (strnicmp (response_string, "OK", 2)))  /*             no "OK"s  */
          status_line (" '%s'", response_string);  /* pop rest to screen    */
        else
          status_line (">'%s'", response_string);  /* r. hoerner */
      }
    }
  }

  strcpy (fpnumber, response_string);  /* store response for FreePoll  CE 960423 */
  return (response_string);     /* return the pointer        */
}

static int LOCALFUNC
parse_response (char *response)
{
  char *p;                      /* temp character pointer    */
  register int i;               /* array pointer             */

  if (!mdm_resps)               /* If we have no strings,    */
    return (IGNORE);            /* always ignore...          */

  if (strlen (response) == 0)
    return (IGNORE);

  for (i = 0; i < resp_count; i++)  /* scan through array   */
  {
    p = mdm_resps[i].resp;      /* point at possible response */

    /* Modified by Paul Edwards to make it case-sensitive, so that
       modems that produce extended information such as "Error
       Correction" do not get a match on the "Error" bit. */

    if (strncmp (response, p, strlen (p)) == 0)
    {
      /* We matched this result. Return its disposition */

      status_line (">response: '%s'", response);
      status_line (">matches: '%s'", mdm_resps[i].resp);  /* r. hoerner */
      switch (mdm_resps[i].disp)  /* more debug infos */
      {
      case FAILURE:
        status_line (">result: 'ModemFailure'");
        break;

      case IGNORE:
        status_line (">result: 'ModemIgnore'");
        break;

      case RINGING:
        status_line (">result: 'ModemRinging'");
        break;

      case INCOMING:
        status_line (">result: 'ModemIncoming'");
        break;

      case DIALRETRY:
        status_line (">result: 'ModemRetry'");
        break;

      case RING:
        status_line (">result: 'ModemRing'");
        break;

      case CONNECTED:
        status_line (">result: 'ModemConnect'");
        break;

      case FAX:
        status_line (">result: 'ModemFax'");
        break;

      case ZYXFAX:
        status_line (">result: 'ModemZyxfax'");
        break;

      case COSTRESET:
        status_line (">result: 'ModemCostReset'");
        break;

      default:
        status_line (">result: 'unknown'");
        break;
      }

      return ((int) (mdm_resps[i].disp));
    }
  }

  return (IGNORE);              /* ignore all unknowns       */
}

/* routine is called from "handle_inbound_mail()" and "script_dial()" only */
int
modem_response (int ths)
{
  long end_time;                /* holds time at end of 2min */
  char *response = "";          /* pointer to modem response */
  int result = IGNORE;          /* result code               */
  int ring_count = 0;           /* # of RINGING responses    */
  int userbreak;

  end_time = timerset ((unsigned int) ths);  /* arm the timeout          */

  while ((result == IGNORE)     /* until success or failure, */
         && (!timeup (end_time)))  /* or out of time,           */
  {
    /* See if we've been through this code enough times to reach
     *  the user-defined failure threshold. For this purpose,
     *  we crudely assume that all IGNORE cases are rings.
     *
     *  Henry Clark and Ron Bemis get credit for this one. */

    if (ring_count++ >= ring_tries)
    {
      response = "NO ANSWER";
      /* move the call to parse_response up, so that we get the
       * case correct before we mess with it with fancy_str */
      result = parse_response (response);
      if (!nofancystrings)
        fancy_str (response);
      status_line ("#%s", response);
      break;
    }

    /* We're still here. So get the next modem response
     *  (or time out trying). */

    userbreak = 1;              /* MR 970515 allow user breaks */
    response = get_response (end_time, 1, &userbreak);  /* get a response */

    if (userbreak)              /* MR 970515 did user break?   */
      result = FAILURE;
    else
      result = parse_response (response);  /* parse, determine status     */

    if (result == RING)         /* while "RINGING" seen        */
      result = IGNORE;          /* do nothing but wait         */

    time_release ();
  }

  if (result == CONNECTED)      /* Got to be a CONNECT msg */
  {
    get_dce (response);

    if ((fax_in) && !strnicmp (response + 7, "FAX", 3))
      return (ZYXFAX);

    if (modem.afterconnect == NULL)
      MNP_Filter ();
    else
      afterconnect ();
  }
  else if (result & FAX)        /* FAX is a special case     */
    strcpy (saved_response, response);

  return (result);              /* timeout or failure or OK  */
}

void
mdm_cmd_string (char *mdm_cmd, int dospace)
{
  register char *c;
  register int escaped = 0;

  if (NoModem)
    return;

  if (mdm_cmd == NULL)          /* defense from badness      */
    return;

  if (strlen (mdm_cmd) == 0)    /* defense from useless      */
    return;

  status_line (">sending \"%s\" to modem", mdm_cmd);  /* r. hoerner */

  for (c = mdm_cmd; *c; c++)
  {
    if (escaped)
    {
      print2modemwin (MW_SENT, *c);  /* MR 970216  */
      SENDBYTE (*c);
      escaped = 0;
      continue;
    }

    if (*c == '\\')
    {
      escaped = 1;
      continue;
    }

    if (!isspace (*c) || dospace)  /* don't output spaces       */
      mdm_cmd_char (*c);        /* output the next character */
  }
}

static void LOCALFUNC
empty_delay ()
{
  long t;

  if (NoModem)
    return;
  t = timerset (500);
  while ((!OUT_EMPTY ()) && (!timeup (t)))
    time_release ();            /* wait for output to finish */

  if (!OUT_EMPTY ())
  {
    MDM_DISABLE ();
    Cominit (port_ptr, buftmo);
    program_baud ();
    RAISE_DTR ();
    CLEAR_OUTBOUND ();
    CLEAR_INBOUND ();
    sb_dirty ();
    sb_show ();
  }
}

void
mdm_cmd_char (int outchr)
{
  print2modemwin (MW_SENT, outchr);  /* MR 970216  */

  switch (outchr)
  {
  case '-':                    /* if it's a dash (phone no) */
    return;                     /* ignore it                 */

  case '|':                    /* if the CR character,      */
    outchr = CR;                /* substitute a real CR here */
    break;

#if 0                           /* TJW 960926 I see really NO sense in this, so I commented this */
    /* out because it often makes problems like in AT+FCLASS=2.0 ... */
    /* if somebody KNOWS for what this MAKES sense, please tell me!  */

  case '.':                    /* Substitute ',' for '.'    */
    outchr = ',';               /* for compatibility         */
    break;

#endif

  case '~':                    /* if the "delay" character, */
    empty_delay ();             /* wait for buffer to clear, */
    timer (10);                 /* then wait 1 second        */
    return;                     /* and return                */

  case '^':                    /* Raise DTR                 */
    empty_delay ();             /* wait for buffer to clear, */
    RAISE_DTR ();               /* Turn on DTR               */
    return;                     /* and return                */

  case 'v':                    /* Lower DTR                 */
    empty_delay ();             /* wait for buffer to clear, */
    LOWER_DTR ();               /* Turn off DTR              */
    return;                     /* and return                */

  case '`':                    /* Short delay               */
    timer (1);                  /* short pause, .1 second    */
    return;                     /* and return                */

  default:
    break;
  }

  SENDBYTE ((unsigned char) outchr);  /* write the character */

  if (outchr == CR)             /* if it was a CR,           */
  {
    print2modemwin (MW_SENT, outchr);  /* MR 970216  */
    empty_delay ();
    timer (1);                  /* allow .1 sec line quiet   */
  }
  else if (slowmodem)
    timer (1);                  /* wait .1 sec for output    */
}


/* MR 970416 changes start here */

void
modem_init ()                   /* initialize the modem */
{
  int i;
  int gotok;
  int err_cnt;
  int err_cnt_2 = 0;
  int init_cnt;
  char **init;

  do_status (M_READY_INIT, NULL);
  CLEAR_OUTBOUND ();
  CLEAR_INBOUND ();
  set_baud (max_baud.rate_value, 0);

  if (un_attended)
  {
    init_cnt = modem.init_cnt;
    init = modem.init;
  }
  else
  {
    init_cnt = modem.terminit_cnt;
    init = modem.terminit;
  }

  for (i = 0; i < init_cnt; i++)
  {
    err_cnt = 0;

    do
    {
      set_prior (PRIO_MODEM);   /* always high */

      if (i == 0)
        mdm_cmd_string (modem.init_setup, 1);

      status_line (">Initializing Modem '%s'", init[i]);
      mdm_cmd_string (init[i], 0);
      gotok = wait_ok (5 * PER_SECOND);
      set_prior (PRIO_REGULAR);
    }
    while (!gotok && err_cnt++ < 2);

    /* If we didn't get OK, spawn the SpawnNoOK command */
    if (!gotok)
    {
      if (++err_cnt_2 > 1)
        return;                 /* MR 970502 give up, maybe even exit???     */
      else if (spawnnook && un_attended)
      {
        status_line (MSG_TXT (M_SPAWN_NO_OK_COMMAND));
        screen_clear ();
        vfossil_cursor (1);
        close_up (0);
        b_spawn (spawnnook);
        come_back ();
        i = -1;                 /* MR 970502 restart with first init command */
      }
    }

    timer (5);
    CLEAR_INBOUND ();
  }

  do_status (M_READY_WAITING, NULL);
}


void
modem_answer ()                 /* answer the phone */
{

  int i, answer_setup_cnt;
  int gotok;
  int err_cnt;
  int err_cnt_2 = 0;

  do_status (M_READY_ANSWERING, NULL);  /* DR 971022 */

  /* AW 980522 prevents Watcom compiler crash */
  answer_setup_cnt = modem.answer_setup_cnt;

  for (i = 0; i < answer_setup_cnt; i++)
  {
    err_cnt = 0;

    do
    {
      mdm_cmd_string (modem.answer_setup[i], 0);
      gotok = wait_ok (PER_SECOND);
    }
    while (!gotok && err_cnt++ < 2);

    /* If we didn't get OK, spawn the SpawnNoOK command */
    if (!gotok)
    {
      if (++err_cnt_2 > 1)
        return;                 /* MR 970502 give up, maybe even exit???     */
      else if (spawnnook && un_attended)
      {
        status_line (MSG_TXT (M_SPAWN_NO_OK_COMMAND));
        screen_clear ();
        vfossil_cursor (1);
        close_up (0);
        b_spawn (spawnnook);
        come_back ();
        i = -1;                 /* MR 970502 restart with first init command */
      }
    }
  }

  mdm_cmd_string (modem.answer, 0);
}


void
modem_busy ()                   /* busy the phone */
{
  mdm_cmd_string (modem.busy, 0);
}


void
modem_reject ()                 /* reject a call */
{
  mdm_cmd_string (modem.reject, 0);
}


void
modem_hangup ()                 /* hang up the phone */
{
  long t1;
  int err_cnt = 0;
  int userbreak = 0;

  do_status (M_READY_HANGUP, NULL);  /* always, 'cause of MCP (r.hoerner 1/7/97) */
  CLEAR_OUTBOUND ();
  set_baud (max_baud.rate_value, 0);
  CLEAR_INBOUND ();

  do
  {
    mdm_cmd_string (modem.hangup, 0);
  }
  while (CARRIER && (err_cnt++ < 2));

  t1 = timerset ((unsigned int) PER_SECOND);

  while (!timeup (t1))          /* wait for modem response */
  {
    if (get_response (t1, 0, &userbreak) != '\0')
      break;

    time_release ();
  }

  CLEAR_OUTBOUND ();
  CLEAR_INBOUND ();

  if (!un_attended)
    status_line (MSG_TXT (M_MODEM_HANGUP));  /* Tell what we've done */

  ARQ_lock = 0;                 /* Re-init lockbaud */
  set_xy ("");
}

long
modem_getcost (long oldcost)
{
  if (cFosCost)
    return last_cost;
  else
    return oldcost;
}

void
modem_aftercall (void)
{
  char *answer;
  int i;
  int userbreak = 0;
  long lwait;

  if (modem.aftercall && (remote_pickup != -2))
  {
    mdm_cmd_string (modem.init_setup, 1);
    mdm_cmd_string (modem.aftercall, 1);

    lwait = timerset (3 * PER_SECOND);
    for (i = 0; i < lines_aftercall && !timeup (lwait); i++)
    {
      answer = get_response (lwait, 1, &userbreak);
      if (!stricmp (answer, "NO CARRIER"))
      {
        CLEAR_INBOUND ();
        break;
      }

      lwait = timerset (PER_SECOND);
    }
  }

  if (cFosCost)
  {
    /* we are expecting to see a string like this:              */
    /* "D   R:2      T:4    C:1         X75            0123456" */
    /*  0         1         2                                   */
    /*  012345678901234567890123456789                          */

    last_cost = 0;              /* MR 971223 we should reset the last costs */

    mdm_cmd_string (modem.init_setup, 1);
    mdm_cmd_string ("ATi1|", 0);

    lwait = timerset (3 * PER_SECOND);
    for (i = 0; i < 3 && !timeup (lwait); i++)
    {
      answer = get_response (lwait, 1, &userbreak);

      if (!stricmp (answer, "NO CARRIER"))
      {
        CLEAR_INBOUND ();
        break;
      }
      else if (strlen (answer) > 50)
      {
        if ((answer[20] == 'C')
            && (answer[21] == ':')
            && isdigit (answer[22]))
        {
          long tcost;

          answer += 22;

          if (sscanf (answer, "%ld", &tcost) == 1)
            last_cost = tcost * cost_unit;

          status_line (">cFosCost result: %ld=%ld*%i",
                       last_cost, tcost, cost_unit);
        }
      }

      lwait = timerset (PER_SECOND);
    }
  }
}

static int LOCALFUNC
wait_ok (int t)
{
  if (!modem.no_okcheck)
  {
    int userbreak = 0;
    long t1;

    t1 = timerset ((unsigned int) t);

    while (!timeup (t1))
    {
      if (strncmp (get_response (t1, 0, &userbreak), "OK", 2) == 0)
        return 1;
    }

    return 0;
  }

  return 1;
}


/* MR 970416 changes end here */


/*
 * MR 970310 was:
 *
 * char ch0,ch1,ch2;
 *
 * t = timerset (5 * PER_SECOND);
 * gotok = 0;
 * ch0 = ch1 = '\0';
 * if (spawnnook == NULL)
 *   get_response (t, 0);
 * else
 *   while (!timeup (t))
 *     if (CHAR_AVAIL ())
 *     {
 *       t = timerset (PER_SECOND);
 *       ch2 = ch1, ch1 = ch0, ch0 = (char) (MODEM_IN () & 0xFF);
 *       if (ch2 == 'O' && ch1 == 'K' && ch0 == '\r')
 *         gotok = 1;
 *     }
 *     else
 *     {
 *       time_release ();
 *     }
 */

void
send_break (int t)
{
  long t1;

  t1 = timerset ((unsigned int) t);
  do_break (1);

  while (!timeup (t1))
    time_release ();

  do_break (0);
}


/* We get here right after the CONNECT message. It could happen
 * so quickly that we don't even have DCD set. (On a 33MHz 386
 * and a T2500, that happens!)
 *
 * So: this routine waits up to 3 seconds for a carrier.
 *
 * It then eats anything that looks like an MNP string, with
 * a total time allowed of 10 seconds (for streaming garbage)
 * and maximum inter-character delay of 2 seconds. */

static void LOCALFUNC
MNP_Filter ()
{
  long t, t1;
  int c;
  int logged = 0;

  if (noMNPFilter)              /* r. hoerner */
    return;

  t = timerset (3 * PER_SECOND);  /* at most a three second delay  */
  t1 = timerset (50);           /* From same point, .5 sec for LF */

  while (!CARRIER && !timeup (t))
    time_release ();            /*PLF Sun  12-01-1991  04:28:38 */

  /* Look for terminator character on connect string. If we see one,
   * eat it. Only wait up to .5 sec for it, though. (Note that if
   * we delayed for DCD, we count that time, since the LF should
   * have already been coming in from the modem) */

  do
  {
    c = PEEKBYTE ();
    if (c != -1)
      break;
  }
  while (!timeup (t1));

  if (c == LF)
    MODEM_IN ();

  /* If this was a MNP connection, bail out right now. */

  for (c = 0; c < cMNP; c++)
  {
    if ((strnicmp (mdm_reliable, pchMNP[c], strlen (pchMNP[c]))) == 0)
      return;
  }

  t1 = timerset (10 * PER_SECOND);  /* 10 second drop dead timer    */
  t = timerset (2 * PER_SECOND);  /* at most a 2 second delay     */

  while (CARRIER && !timeup (t))
  {
    if (got_ESC ())
    {                           /* Manual abort?        */
      modem_hangup ();          /* MR 970310 was: LOWER_DTR (); Yes, drop carrier */
      return;
    }

    if (timeup (t1))
      break;

    if ((c = PEEKBYTE ()) != -1)
    {
      TIMED_READ (0);

      /* If we get an MNP or v.42 character,
       * eat it and wait for clear line */

      if ((c != 0) && ((strchr (BadChars, (c | 0x80)) != NULL)
                       || (strchr (BadChars, (c & 0x7f)) != NULL)))
      {
        t = timerset (5 * PER_SECOND);
        if ((logged++) == 0)
          status_line (MSG_TXT (M_FILTER));
      }
    }
    else
      time_release ();          /*PLF Sun  12-01-1991  04:28:38 */
  }
}

static void LOCALFUNC
afterconnect ()
{
  int i = 0;
  int userbreak = 0;
  long lwait;

  mdm_cmd_string (modem.afterconnect, 0);

  lwait = timerset (3 * PER_SECOND);
  for (i = 0; i < lines_afterconnect && !timeup (lwait); i++)
  {
    if (!stricmp (get_response (lwait, 1, &userbreak), "NO CARRIER"))
    {
      CLEAR_INBOUND ();
      break;
    }
    lwait = timerset (PER_SECOND);
  }

  return;
}

/* get a cause-value via AT???-command (for e.g. ZyXEL 2864I) */
static int LOCALFUNC
get_cause_value (int k)
{
  int i = 0;
  int reject = 0;
  int userbreak = 0;
  long lwait;

  if (modem.aftercallout && k == 0)
  {
    mdm_cmd_string (modem.aftercallout, 0);

    lwait = timerset (3 * PER_SECOND);
    for (i = 0; i < lines_aftercallout && !timeup (lwait); i++)
    {
      get_response (lwait, 0, &userbreak);
      if (strstr (response_string, ModemReject) != NULL && !reject)
      {
        CLEAR_INBOUND ();
        reject = 1;
      }

      lwait = timerset (PER_SECOND);
    }
  }

  return (reject);
}


static void LOCALFUNC
get_dce (char *response)
{
  unsigned long baudrate;
  char *c;                      /* miscellaneous pointer     */
  int i;

  mdm_reliable[0] = '\0';       /* Start with nothing        */

  status_line (">get_dce(%s)", response);

  strcpy (saved_response, response);
  c = skip_blanks (saved_response + 7);  /* get past the blanks       */

  if (*c == '\0')               /* if nothing there,         */
  {
    baudrate = 300L;            /* say that it's 300 baud    */
    if (assumebaud)
      baudrate = assumebaud;    /* r. hoerner say it's "assumed baudrate" */
  }
  else
  {
    baudrate = (unsigned long) atol (c);  /* else do fallback */

    /* For 1200/75 split speed modems and "Connect 212" */
    if ((baudrate == 1275) || (baudrate == 7512)
        || (baudrate == 75) || (baudrate == 212) || (baudrate == 12))
      baudrate = 1200L;

    /* For "Connect 103" */
    if (baudrate == 103)
      baudrate = 300L;
  }

  realspeed = baudrate;
  ARQ_lock = 0;

  while (isdigit (*c))          /* Get past digits           */
    ++c;
  c = skip_blanks (c);          /* Get rid of blanks         */
  if (*c != '\0')               /* We have "reliable" info.  */
  {
    strcpy (mdm_reliable, c);   /* Copy in the info  */
    can_Janus (baudrate, mdm_reliable);  /* Set the flag for Janus    */
    for (i = 0; i < ARQs; i++)
    {
      if ((strnicmp (mdm_reliable, ARQ[i], strlen (ARQ[i]))) == 0)
      {
        ARQ_lock = 1;
        break;
      }
    }
  }

  if (baudrate)
    set_baud (baudrate, 1);

  status_line (">baudrate : %u", baudrate);

  connectrate = cur_baud.rate_value;  /* MMP For cost log */
}

/* $Id: mdm_proc.c,v 1.8 1999/09/27 20:51:33 mr Exp $ */
