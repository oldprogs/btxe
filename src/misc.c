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
 * Filename    : $Source: E:/cvs/btxe/src/misc.c,v $
 * Revision    : $Revision: 1.20 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:34 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello with code from several other authors
 *
 * Description : Miscellaneous routines used by BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static void LOCALFUNC fill_in_status (char *);
static int LOCALFUNC find_addr (char *, ADDR *, char *);
int zoom_popup (BINK_SAVEP p, int x);
static char *LOCALFUNC store_TZ_helper (char *);

unsigned short
get_filetype (char *file)
{
  char *x, *y, st[4];

  x = strchr (file, '.');
  if (!x)
    return AT_OTHER;

  x++;
  memcpy (st, x, 3);
  st[4] = 0;
  for (y = st; *y; y++)
    *y = tolower (*y);

  if (!strcmp (st, "pkt") || !strcmp (st + 1, "ut"))
    return AT_NETMAIL;

  st[3] = 0;

  if (!strcmp (st, "mo") || !strcmp (st, "tu") || !strcmp (st, "we") ||
      !strcmp (st, "th") || !strcmp (st, "fr") || !strcmp (st, "sa") ||
      !strcmp (st, "su"))
    return AT_ECHOMAIL;

  return AT_OTHER;
}

int
islocalregion (word Zone, word Net)
{
  AKREGION *pr;
  char N[12];

  sprintf (N, "%hu", Net);
  for (pr = regions; pr != NULL; pr = pr->next)
    if (pr->Zone == Zone && strstr (N, pr->Region) == N)
      return 1;
  return 0;
}

void
do_random_system (void)
{
  word g, r;

  if (!RandomGroups)
    return;

  g = rand () % RandomGroups;   /* Choose group */
  if (!RandomSystemInfo[g]->sysopcount)
    sysop = def_sysop;
  else
  {
    r = rand () % RandomSystemInfo[g]->sysopcount;
    sysop = RandomSystemInfo[g]->sysop[r];
  }

  if (!RandomSystemInfo[g]->systemcount)
    system_name = def_system_name;
  else
  {
    r = rand () % RandomSystemInfo[g]->systemcount;
    system_name = RandomSystemInfo[g]->system[r];
  }

  if (!RandomSystemInfo[g]->locationcount)
    location = def_location;
  else
  {
    r = rand () % RandomSystemInfo[g]->locationcount;
    location = RandomSystemInfo[g]->location[r];
  }

  top_line ();
}

int
set_baud (unsigned long baudrate, int log)
{
  register int i;
  unsigned long search_rate;

  search_rate = (baudrate > max_baud.rate_value) ? max_baud.rate_value : baudrate;

  for (i = 0; pbtypes[i].rate_value; i++)
  {
    if (pbtypes[i].rate_value >= search_rate)
    {
      if ((pbtypes[i].rate_value != search_rate) && (search_rate <= 2400))
        break;

      if (baud != i)            /* same as what we have?     */
      {
        if (log && !un_attended)
          status_line (MSG_TXT (M_SETTING_BAUD), baudrate);
        baud = i;               /* need this for ALT-B       */
        program_baud ();
        cur_baud = pbtypes[i];
      }

      cur_baud.rate_value = baudrate;
      show_cs_port (baudrate, CARRIER);
      sb_show ();
      return (1);
    }
  }

  return (0);
}

void
program_baud (void)
{
  unsigned long rate_mask;

  set_prior (PRIO_MODEM);

  if ((lock_baud && (pbtypes[baud].rate_value >= lock_baud))
      || (ARQ_lock != 0))
    rate_mask = max_baud.rate_mask;
  else
    rate_mask = pbtypes[baud].rate_mask;

  MDM_ENABLE (rate_mask);

  set_prior (PRIO_REGULAR);

}

static struct tm *tp;
static time_t ltime;

void
log_screen (char *timestr_scr, char *str)
{
  char tmp[300];

  sprintf (tmp, stat_str, str[0], timestr_scr, &str[1]);
  fill_in_status (tmp);

#if defined(OS_2) | defined(_WIN32)
  stripcolors (tmp);
  SnoopWrite (hsnoop, (PSZ) tmp);
#endif
}

/* This is a quick & dirty function to perform
 * case-insensitive substring searching */

char *
strstrci (char *string, char *substring)
{
  char *s1, *s2, *f, *result;

  if (string == NULL)
    return NULL;
  if (substring == NULL)
    return NULL;

  s1 = (char *) malloc (strlen (string) + 1);
  if (s1 == NULL)
    return NULL;

  s2 = (char *) malloc (strlen (substring) + 1);
  if (s2 == NULL)
  {
    free (s1);
    return NULL;
  }

  strcpy (s1, string);
  strupr (s1);
  strcpy (s2, substring);
  strupr (s2);

  f = strstr (s1, s2);
  if (!f)
    result = NULL;
  else
    result = (char *) ((unsigned long) string
                       + ((unsigned long) f - (unsigned long) s1));

  /* this really weird construction removes a warning with BC3.1 */
  /* original code was: result=(char *)(string+(f-s1));          */
  /* r.hoerner 26/01/97 */

  free (s1);
  free (s2);

  return result;
}

void
parse_stringrep (char *par, int frommodem)
{
  char *c;
  int norep = 0;
  struct SStringRep *newrep;
  newrep = (struct SStringRep *) malloc (sizeof (struct SStringRep));

  if (!newrep)
    return;

  SReplacements = (struct SStringRep **) realloc (SReplacements, (1 +
                                                      definedreplacements) *
                                              sizeof (struct SStringRep *));

  SReplacements[definedreplacements] = newrep;

  if (*par == '"')
  {
    c = ++par;
    while (*c && (*c) != '"')
      c++;
  }
  else
    c = skip_to_blank (par);

  if (*c == '\0')
    norep = 1;
  else
    *c = '\0';

  newrep->change = ctl_string (par);

  if (norep)
    newrep->replacement = ctl_string ("");
  else
    newrep->replacement = ctl_string (c + 1);

  newrep->frommodem = frommodem;
  definedreplacements++;
}

/* This function replaces the strings. The replacement is done 'in-place'
 * and there must be enough room for any extra chars! CFS 961021 */

void
perform_rep (char *string, int isfrommodem)
{
  char *temp, *c, *t2, *search;
  struct SStringRep *current;
  unsigned i;
  int left = 0, right = 0;

  temp = (char *) malloc (E_INPUT_LEN);

  if (!temp)
    return;

  for (i = 0; i < definedreplacements; i++)
  {
    current = SReplacements[i];
    if (current->frommodem != isfrommodem)
      continue;

    t2 = (char *) malloc (strlen (current->change) + 1);
    if (t2 == NULL)
      break;                    /* r. hoerner */

    strcpy (t2, current->change);

    if (t2[0] == '[')
    {
      strcpy (t2, t2 + 1);
      left = 1;
    }
    else
      left = 0;

    if (t2[strlen (t2) - 1] == ']')
    {
      t2[strlen (t2) - 1] = '\0';
      right = 1;
    }
    else
      right = 0;

    search = string;

    while ((c = strstrci (search, t2)) != NULL)
    {
      if ((left == 0 || (left == 1 && (c == string || *(c - 1) == ' '))) &&
          (right == 0 || (right == 1 && (c[strlen (t2)] == '\0'))))
      {
        if (left == 1 && c == string + 1 && *(c - 1) == ' ')
          *(c - 1) = 0;         /* SM 970103 */
        else
          *c = 0;
        strcpy (temp, string);
        strcat (temp, current->replacement);
        strcat (temp, c + strlen (t2));
        strcpy (string, temp);
      }

      /* Continue from last match after replacement */
      search = c + strlen (current->replacement);
    }

    free (t2);
  }

  free (temp);
}

void _cdecl
status_line (char *fmt,...)
{
  va_list arg_ptr;

  static char *specifiers = "!*+:# >&";
  long this_loglevel;
  char timestr_scr[40];
  char *p;
  char hundredths[4] = "\0";    /* CFS 961121 */
  int wday, hours, mins, secs, hths;

  va_start (arg_ptr, fmt);
  if (fmt == NULL || !fmt[0] || !fmt[1])
  {
    va_end (arg_ptr);
    return;                     /* TJW 960708:                   */
  }

  vsprintf (e_input, fmt, arg_ptr);  /* this is DANGEROUS if too long */
  e_input[E_INPUT_LEN - 1] = '\0';  /* avoid the worst of "open end" */

  perform_rep (e_input, 0);

  this_loglevel = strchr (specifiers, e_input[0]) - strchr (specifiers, '!');

  /* CEH/971005: Count all !-lines */
  if (!this_loglevel)
    hist.total_errors++;

  /* Paul Edwards: protect against control characters in the string */
  for (p = e_input; *p != '\0'; p++)
  {
    if (iscntrl (*p))
    {
      if (*p == '\007' && gong_allowed)
        WRITE_ANSI ('\007');    /* TJW 960706 chat request gong */
      *p = '.';
    }
  }

  /* AW990318 get time and date at the same time, use hours/mins/secs/hths */
  /* together instead of tp->xxx and hths */
  unix_time (&ltime);
  dostime (&wday, &hours, &mins, &secs, &hths);

  if (((e_input[0] != '>') && (e_input[0] != '&')) || (debugging_log))
  {
    sprintf (timestr_scr, "%02i:%02i:%02i", hours, mins, secs);
    log_screen (timestr_scr, e_input);
  }

  /* TJW 960522 negative loglevels log only with CARRIER_DETECT=1 */
  if ((status_log != NULL) && (this_loglevel <= abs (loglevel)))
  {
    if (loglevel >= 0 || CARRIER)
    {
      tp = unix_localtime (&ltime);

      /* CFS 961121 - removed Paul Edward's conditional stuff to log
       * hundredths. See NOHUNDREDTHS keyword */

      if (!nologhundredths)
        sprintf (hundredths, ".%02i", hths);

      stripcolors (e_input);    /* CFS 970216 */
      fprintf (status_log, "%c %02i %s %02i:%02i:%02i%s %s %s\n",
               e_input[0],
               tp->tm_mday, mtext[tp->tm_mon],
               tp->tm_hour, tp->tm_min, tp->tm_sec, hundredths,
               un_attended ? "BINK" : "TERM",
               &e_input[1]);

      if (immed_update)
      {
        fflush (status_log);
        real_flush (fileno (status_log));
        need_update = 0;
      }
      else
        need_update = 1;
    }
  }

  va_end (arg_ptr);
}

/*--------------------------------------------------------------------------*/
/* THROUGHPUT                                                               */
/* Print throughput message at end of transfer                              */
/* opt = 0 -> start timer, = 1 -> stop + display, = 2 update cps rates      */
/* dir = 0 -> Rx,    = 1 -> Tx                                              */
/*--------------------------------------------------------------------------*/

long
throughput (int opt, int dir, unsigned long bytes)
{
  static long started[2] =
  {
    0L, 0L
  };

  static long stopped[2] =
  {
    0L, 0L
  };

  static long elapsed[2] =
  {
    0L, 0L
  };

  long cps;                     /* TJW 960513 */
  long efficiency;              /* TJW 960513 */
  long tnow = longtimerset (0);

  if (!opt)
    started[dir] = tnow;
  else if (started[dir])
  {
    stopped[dir] = tnow;

    /* The next line tests for week wrap without the date rolling over */
    if (stopped[dir] < started[dir])
      stopped[dir] += PER_WEEK;

    elapsed[dir] = stopped[dir] - started[dir];

    if (elapsed[dir] == 0L)
      elapsed[dir] = 1;

    cps = (long) (PER_SECOND * bytes / (unsigned long) elapsed[dir]);

    /* TJW 970407: changed to calculate with 8bits/byte (sync assumed) */
    /*          instead of 10bits/byte (8 data + 1 start + 1 stop bit) */
    efficiency = (cps * (8 * 100L)) / ((long) cur_baud.rate_value);

    if (opt == 1)
    {
      // status_line (" bytes=%ld elapsed=%ld", bytes, elapsed[dir]);
      status_line ((char *) MSG_TXT (M_CPS_MESSAGE), cps, bytes, efficiency);
    }
    else if (opt == 2)
    {
      if (cps <= 99999)
      {
        if (dir)
          TxStats.cur_cps = cps;
        else
          RxStats.cur_cps = cps;
        css_upd ();
      }
    }
  }

  if (elapsed[dir] < PER_SECOND)
    elapsed[dir] = PER_SECOND;
  return elapsed[dir];
}                               /* throughput */

static void LOCALFUNC
fill_in_status (char *stat_line)
{
  BINK_SAVEP tmp = NULL;
  int i;

  unix_time (&ltime);
  tp = unix_localtime (&ltime);

  if (callwin)                  /*PLF Sun  12-01-1991  07:25:27 */
  {
    if (strlen (callwin->endbuff))
      memmove (callwin->buffer,
               callwin->buffer + callwin->linesize,
               (size_t) (callwin->endbuff - callwin->buffer));
    else if (strlen (callwin->buffer))
    {
      callwin->lastshown += callwin->linesize;
      callwin->lastline += callwin->linesize;
    }

    /* TJW: not colstrlen !!! */
    i = min (strlen (stat_line), (unsigned int) callwin->linesize - 2);
    /* i is now string length (without \0 char at end) or max. line    */
    /* length (also without the two \0 chars already there from calloc */

    memcpy (callwin->lastline, stat_line, i);
    /* we needn't copy nor set last char to \0 as it is already there! */

    /* ----------------------------------------------------------------------
     * r. hoerner
     * ----------------------------------------------------------------------
     * I hated to see popup windows been scrolled to the desert when some
     * important message is displayed at the status window.
     * what i did to stop it:
     * in sbuf.c I save the pointer to the first popup window ("popped_up").
     * "first" means: if a second window pops up, nothing more is saved.
     * i added two functions "sb_hide_popup" and "sb_show_popup" (sbuf.c).
     * ----------------------------------------------------------------------
     * a) if popped_up is valid then save the same coordinates again, ie: save
     *    the content of the window that is popped up and restore the status
     *    window with the bytes saved in the (first) popped window: do, as if
     *    the popup window has been closed (it isn't!).
     *
     *    this is done in sb_hide_popup();
     *
     * b) scroll and print to the status window.
     *
     * c) update the popped window's save area (this is: where the original
     *    status window content is saved) with the actual status window content.
     *    repaint the popup window.
     *
     *    c) is done in sb_show_popup();
     * -------------------------------------------------------------------- */

    if (popped_up != NULL)      /* at least 1 popup window on screen */
    {
      tmp = sb_hide_popup (popped_up);
    }

    sb_scrl (callwin, 1);
    sb_move_puts (callwin, callwin->lines - 1, 0, callwin->lastshown);
    /* timedisplay();    AG 990119 not needed anymore */

    if (popped_up != NULL)      /* at least 1 popup window on screen */
      sb_show_popup (popped_up, tmp);

    sb_show ();

    /* --------------------------------------------------------------------
     * if in zoomed outbound: show message in a popup message box, too
     * -------------------------------------------------------------------- */

    if (inzoomwindow)
      sb_popup (10, 0, 3, SB_COLS, zoom_popup, PER_SECOND);
  }

  IPC_Ping ();
}

int
zoom_popup (BINK_SAVEP p, int x)
{
  if (p == NULL)
    return 0;

  sb_fill (p->region, ' ', colors.popup);
  sb_box (p->region, boxtype, colors.popup);
  sb_move_puts (p->region, 1, 1, callwin->lastline);
  wait_for_key (x);
  return (0);
}

void
timedisplay (void)              /* TJW 960624 display date / time in settings window */
{
  char junk[64];

  unix_time (&ltime);
  tp = unix_localtime (&ltime);
  sprintf (junk, "%02d/%02d/%02d %s %02d:%02d",
           tp->tm_year % 100, tp->tm_mon + 1, tp->tm_mday,
           wkday[tp->tm_wday],
           tp->tm_hour, tp->tm_min);

  sb_move_puts (settingswin, SET_TIME_ROW, SET_TIME_COL, junk);
}

void
set_xy (char *string)
{
  if (screen_blank && do_screen_blank)
    return;

  WRITE_ANSI ('\r');
  WRITE_ANSI ('\n');
  scr_printf (string);
}

void _cdecl
time_release (void)
{
  if (!(debugflags & DEBUGFLAGS_NODISPLAYLIFE))
    display_life ();            /* MR 970523 */

  if (!(debugflags & DEBUGFLAGS_NOFLUSH))
  {
    if (need_update)
    {
      if (status_log != NULL)   /*WRA */
      {
        fflush (status_log);
        real_flush (fileno (status_log));
      }

      need_update = 0;
    }
  }

#ifdef DOS16
  dos_break_off ();             /* Turn off ^C trapping */
#endif

  if (!(debugflags & DEBUGFLAGS_NOIDLE))
    (*mtask_idle) ();

}

void
timer (int interval)
{
  long timeout;

  timeout = longtimerset ((long) (interval * 10));
  while (!longtimeup (timeout))
    time_release ();
}

time_t
age_seconds (time_t old)
{
  time_t now = unix_time (NULL);

  if (now >= old)
    return (now - old);
  else
    return ((time_t) 0);
}

void
big_pause (int secs)
{
  long timeout;

  timeout = longtimerset ((long) (secs * PER_SECOND));
  while (!longtimeup (timeout))
  {
    if (CHAR_AVAIL ())
      break;
    time_release ();
  }
}

void
scr_printf (char *string)
{
  if (string != NULL)
  {
    if (vfossil_installed)
      VioWrtTTY ((PCH) string, (USHORT) strlen (string), (HVIO) 0L);
    else
      while (*string != 0)
        WRITE_ANSI (*string++);
  }
}

void
send_can (void)
{
  int i;

  CLEAR_OUTBOUND ();
  CLEAR_INBOUND ();

  for (i = 0; i < 10; i++)
    SENDBYTE (CAN);
  for (i = 0; i < 10; i++)
    SENDBYTE (BKS);
}

void
invent_pkt_name (char string[])
{
  struct tm *ptm;
  time_t t;
  static int counter = 0;

  unix_time (&t);

  /* TJW 970408 bugfix: beware of double pkt names if */
  /* this function is called twice in the same second */
  counter++;
  t += counter;
  ptm = unix_localtime (&t);
  sprintf (string, "%02i%02i%02i%02i.pkt",
           ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  /* TJW: we also just could use sprintf(string, "%ld.pkt", t)    !? */
}

char *
fname8p3 (char *longpath)
{
  static char fname[13];
  char *p;

  if ((p = strrchr (longpath, DIR_SEPC)) != NULL)
    strntcpy (fname, p + 1, 13);
  else
    strntcpy (fname, longpath, 13);

  return fname;
}

int
is_user (char *p)
{
  char *q;
  char far *r;
  int i, j;

  if (cur_event < 0)
    return (0);

  q = strchr (p, '.');
  if (q != NULL)
  {
    ++q;
    for (i = 0; i < 6; i++)
    {
      r = &(e_ptrs[cur_event].err_extent[i][0]);
      for (j = 0; j < 3; j++)
      {
        if ((tolower (q[j]) != tolower (r[j])) && (q[j] != '?'))
          break;
      }

      if (j == 3)
        user_exits[i] = 1;
    }
  }

  return (0);
}

int
is_arcmail (char *p, int n)
{
  int i, l;
  char c[PATHLEN];
  static char *suffixes[8] =
  {"su", "mo", "tu", "we", "th", "fr", "sa", NULL};

  l = n;
  if (n < 0)
    n = -n;

  if (!isdigit (p[n]))
    return (l >= 0 ? is_user (p) : 0);

  strcpy (c, p);
  strlwr (c);

  for (i = n - 11; i < n - 3; i++)
  {
    if ((!isdigit (c[i])) && ((c[i] > 'f') || (c[i] < 'a')))
      return (l >= 0 ? is_user (p) : 0);
  }

  for (i = 0; i < 7; i++)
  {
    if (strnicmp (&c[n - 2], suffixes[i], 2) == 0)
      break;
  }

  if (i >= 7)
    return (l > 0 ? is_user (p) : 0);

  if (l >= 0)
    got_arcmail = 1;

  return (1);
}

int
get_number (char *target)
{
  ADDR gaddr;
  int k;

  fgets (target, (size_t) 100, stdin);

  k = (int) strlen (target);
  if (k == 1)
    return (0);

  target[--k] = '\0';           /* no '\n' */

  if (!isdigit (target[0]) && target[0] != '\"')
  {
    (*userfunc) (target, &gaddr);

    if ((gaddr.Net != 0xffff)
        && (gaddr.Node != 0xffff)
        && (gaddr.Zone != 0xffff))
    {
      sprintf (target, "%s", Full_Addr_Str (&gaddr));
    }
    else
      return (0);               /* Gotta have addr */
  }

  return (1);
}

void
gong (void)
{
  long t;
  int i;

  if (gong_allowed)
  {
    for (i = 0; i < 15; i++)
    {
      WRITE_ANSI ('\007');      /* Bell code       */
      t = timerset (100);       /* 1 second        */
      while (!timeup (t))
      {
        if (KEYPRESS ())        /* If key pressed, */
        {
          READKB ();            /* Throw it away   */
          return;               /* And get out     */
        }
      }
    }
  }
}

#ifdef DEBUG
void
show_debug_name (char *string)
{
  int x, y;
  static char *filler = "                           ";

  x = wherex ();
  y = wherey ();
  gotoxy (40, 0);
  scr_printf (string);
  scr_printf (&filler[strlen (string)]);
  gotoxy (x, y);
}
#endif

void
change_prompt (void)
{
  static char *newprompt = NULL;

  if (!newprompt)
  {
    char pre[80];
    char *old;

    strcpy (pre, "PROMPT=[");
    strcat (pre, xfer_id);
    strcat (pre, " Shell]$_");
    old = getenv ("PROMPT");
    if (!old)
      old = "$P$G";

    newprompt = (char *) malloc (strlen (pre) + strlen (old) + 1);
    if (newprompt)
    {
      strcpy (newprompt, pre);
      strcat (newprompt, old);
    }
    else
      newprompt = "PROMPT=$P$G";
  }

  putenv (newprompt);
}

static char *last_str[] =
{
  "None",                       /* 0 */
  "WaZOO",                      /* 1 */
  "FTS-0001",                   /* 2 */
  "BBS",                        /* 3 */
  "Ext Mail",                   /* 4 */
  "FAX",                        /* 5 */
  "Inc-Fail"                    /* 6 TJW 960625 incoming call failed, CONNECT - nothing more */
};

void
last_type (int n, ADDRP taddr)
{
  char *p;
  char s1[90], s2[90];

  if ((n == 1) || (n == 2))
  {
    if (((short) taddr->Zone != -1000) && ((short) taddr->Net > 0))
    {
      hist.last_addr = *taddr;
      sprintf (s1, "%s", Full_Addr_Str (taddr));
      if ((p = strchr (s1, '@')) != NULL)
        *p = '\0';
      strcpy (s2, s1);
    }
    else
      strcpy (s2, "FTS-0001");
  }
  else
  {
    if ((n < 0) || (n > 6))
      n = 0;

    strcpy (s2, last_str[n]);
  }

  strcpy (hist.last_str, s2);
  hist.last_caller = (short) n;
}

/*--------------------------------------------------------------------------*/
/* CHECK_NETFILE -- find out if the file we've got is a netfile.            */
/*--------------------------------------------------------------------------*/

char *
check_netfile (char *fname)
{
  register char *p;
  register int n;

  p = fname;
  n = (int) strlen (p) - 1;

  if ((tolower (p[n]) == 't')
      && (tolower (p[n - 1]) == 'k')
      && (tolower (p[n - 2]) == 'p')
      && (tolower (p[n - 3]) == '.'))
  {
    got_packet = 1;             /* *.PKT received */
    got_mail = 1;
    p = MSG_TXT (M_MAIL_PACKET);
  }
  else if ((tolower (p[n]) == 'c')  /* VRP 990918 */
           && (tolower (p[n - 1]) == 'i')
           && (tolower (p[n - 2]) == 't')
           && (tolower (p[n - 3]) == '.'))
  {
    got_tic = 1;                /* *.TIC received */
    got_mail = 1;
    p = MSG_TXT (M_NET_FILE);
  }
  else if (is_arcmail (p, n))   /* packed mail received */
  {
    got_mail = 1;
    p = MSG_TXT (M_COMPRESSED_MAIL);
  }
  else
  {
    short mytask = -1;

    if ((tolower (p[n - 2])) != 'r' || p[n - 3] != '.')
    {
      /* if not a *.R?? request file */
      got_mail = 1;
    }
    else
    {
      /* else it is a R?? file */
      sscanf (&p[n - 1], "%02hx", &mytask);
      if (mytask != TaskNumber)
        got_mail = 1;
    }

    p = MSG_TXT (M_NET_FILE);
  }

  return (p);
}

/*--------------------------------------------------------------------------*/
/* UNIQUE_NAME                                                              */
/* Increments the suffix of a filename as necessary to make the name unique */
/*--------------------------------------------------------------------------*/
void
unique_name (char *fname)
{
  static char suffix[] = ".001";
  register char *p;
  register int n;

  if (dexists (fname))
  {                             /* If file already exists...      */
    p = fname;
    while (*p && *p != '.')
      p++;                      /* ...find the extension, if any  */

    for (n = 0; n < 4; n++)     /* ...fill it out if neccessary   */
      if (!*p)
      {
        *p = suffix[n];
        *(++p) = '\0';
      }
      else
        p++;

    while (dexists (fname))     /* ...If 'file.ext' exists suffix++ */
    {
      p = fname + strlen (fname) - 1;
      for (n = 3; n--;)
      {
        if (!isdigit (*p))
          *p = '0';
        if (++(*p) <= '9')
          break;
        else
          *p-- = '0';
      }                         /* for */
    }                           /* while */
  }                             /* if exist */
}                               /* unique_name */

int
got_ESC (void)
{
  word c;

  while (KEYPRESS ())
  {
    screen_blank = 0;
    sb_show ();
    if ((c = READKB ()) == 27)  /* ESC pressed?        */
    {
      while (KEYPRESS ())
        READKB ();

      return (1);
    }
    else
      CommonProtocolKeyCheck (c);
  }

  return (0);
}


static long maxin_size;
static long maxin_time;
static long maxout_size;
static long maxout_time;

void
calc_cost (int caller, long s, long e,
           long *_dt, long *_totcost, long *_totrcost, word * _cost,
           char **ename, word ci)
{
  BINK_EVENTP costp;
  long t;
  long tx10, totcost;

  *_totcost = 0;                /* r. hoerner (init!!) */
  *_totrcost = 0;
  *_cost = 0;

  // status_line(" calc_cost: caller: %d s: %ld e: %ld", caller, s, e);

  if (!e)                       /* if e is not supplied, calc min call cost for 1s */
    e = 1;

  if (!s)
  {                             /* if s is not supplied, take time NOW and e as dt */
    s = unix_time (NULL);
    e += (s + costtimecorr_sess);  /* also add session overhead (def:5s) */
  }

  s -= costtimecorr_conn;       /* it takes some worthy time until "CONNECT" */

  *_dt = e - s;                 /* default is "5" for costtimecorr_conn     */

  /* ci = cost field of nodelist index is cv index */
  if (ci <= 0)
  {                             /* beware of illegal ci values! */
    *ename = "NoCost";
    return;                     /* ==0: all done */
  }

  if (ci > 32)
  {
    *ename = "Invalid";
    return;                     /* >32: invalid, emergency exit ! */
  }

  *_cost = ci;                  /* TJW 960825 take cv index as cost value   */
  ci--;                         /* make cost index 0-based              */

  costp = find_costevent (s);
  if (costp == NULL)
  {
    *ename = "Invalid";
    return;                     /* NULL pointer trap, emergency exit ! */
  }

  *ename = costp->ename;        /* give back name of cost event */
  if (eurocost)
  {
    t = s;
    tx10 = 0;                   /* 1 s and 1/10 s counter             */
    totcost = 0;                /* cumulated costs of call            */
    while (t < e)
    {                           /* simulate costs until we reach end  */
      totcost += cost_unit;     /* we have spent 1 more cost unit now */
      if (costp->costvector[ci] == 0)  /* MR 970803 */
      {
        *ename = "Invalid";
        return;                 /* emergency exit !                   */
      }
      else
        tx10 += costp->costvector[ci];  /* and can do now for some time... */

      t = s + tx10 / 10;        /* calculate real time t              */
      costp = find_costevent (t);  /* which tariff do we have now?    */
      if (costp == NULL)
      {
        *ename = "Invalid";
        return;                 /* emergency exit !                   */
      }
    }
  }
  else
  {
    t = s;                      /* 1 s counter                        */
    totcost = 0;                /* cumulated costs of call            */
    while (t < e)
    {                           /* simulate costs until we reach end  */
      if (costp)
        totcost += costp->costvector[ci];  /* we spent some cents more now */
      t += 60;                  /* and can do now for 1 minute more.. */
      costp = find_costevent (t);  /* which tariff do we have now?    */
    }
  }

  if (caller)
    *_totcost = totcost;        /* my costs */
  else
    *_totrcost = totcost;       /* remote's costs */
}

long
cost_of_call (int caller, long s, long e, CALLHIST * data)
{
  long mycost, remotecost, dt;
  char *p, *ename;
  char logline[34];
  int session = 0;
  word costidx;

  /* AW 982105 (don't laugh) "volatile" allowes the max
   * optimization with Watcom without compiler crash in this module */
  volatile int infiles, outfiles, totfiles;
  volatile long insize, outsize, totsize;
  volatile long intime, outtime, tottime;

  long incps, outcps, totcps;
  long ineff, outeff, toteff;
  long inerr, outerr, toterr;

  char costfmt[] = "%s Stat %4s %3hd Files (%s)%5ldcps %3hd%%  Err: %ld\n";

  unix_time (&ltime);
  tp = unix_localtime (&ltime);

  sprintf (logline, "%02i %3s %02i:%02i",
           tp->tm_mday, mtext[tp->tm_mon], tp->tm_hour, tp->tm_min);

  calc_cost (caller, s, e, &dt, &mycost, &remotecost, &costidx,
             &ename, newnodedes.RealCost);

  if (caller || hist.last_caller == 1 || hist.last_caller == 2)
  {
    session = 1;
    p = Full_Addr_Str (&remote_addr);
  }
  else if (hist.last_caller >= 3 && hist.last_caller <= 6)
    p = last_str[hist.last_caller];
  else
    p = "Unknown";

  /* AW 980521 (see at the "don't laugh" comment above). Leave the        */
  /* following ugly lines as they are, otherwise Watcom cannot compile it */
  /* at maximum optimization, which is needed for the DOS version         */

  infiles = hist.files_in;
  data->h.m.filesin = infiles -= start_hist.files_in;
  outfiles = hist.files_out;
  data->h.m.filesout = outfiles -= start_hist.files_out;
  totfiles = infiles + outfiles;

  insize = hist.size_in;
  data->h.m.bytesin = insize -= start_hist.size_in;
  outsize = hist.size_out;
  data->h.m.bytesout = outsize -= start_hist.size_out;
  totsize = insize + outsize;

  intime = hist.time_in;
  intime -= start_hist.time_in;
  outtime = hist.time_out;
  outtime -= start_hist.time_out;
  tottime = dt;

  inerr = hist.err_in;
  inerr -= start_hist.err_in;
  outerr = hist.err_out;
  outerr -= start_hist.err_out;
  toterr = inerr + outerr;

  start_hist = hist;

  data->h.m.cpsin = incps = (intime) ? (insize / intime) : 0;
  data->h.m.cpsout = outcps = (outtime) ? (outsize / outtime) : 0;
  data->h.m.cpsall = totcps = (tottime) ? (totsize / tottime) : 0;
  data->h.m.speed = realspeed;

  ineff = incps * 1000L / cur_baud.rate_value;
  outeff = outcps * 1000L / cur_baud.rate_value;
  toteff = totcps * 1000L / cur_baud.rate_value;

  data->h.m.cost = mycost = modem_getcost (mycost);  /* change cost!? */
  data->starttime = (time_t) s;
  data->length = (time_t) e - (time_t) s;
  data->h.m.address = remote_addr;

  if (session && cost_log != NULL)
  {
    if (shortcostlog)
    {
      char *c;
      char pad = '\0';          /*  TS 960620 since maybe used unititialized */
      char buf[40];
      int len;

      for (c = shortcostlog; *c; c++)
      {
        if (*c != '$')
          fputc (*c, cost_log);
        else
        {
          len = 0, c++;
          if (isdigit (*c))
          {
            if (*c == '0')
              pad = '0';
            else
              pad = ' ';
            len = atoi (c);
            while (isdigit (*c))
              c++;
          }

          switch (*c)
          {
          case '#':
            sprintf (buf, "%d", TaskNumber);
            break;

          case '$':
            sprintf (buf, "$");
            break;

          case '<':
            sprintf (buf, "\n");
            break;

          case 'A':
            sprintf (buf, "%s", p);
            break;

          case 'B':
            sprintf (buf, "%lu", connectrate);
            break;

          case 'C':
            sprintf (buf, "%ld", max (maxin_size, maxout_size));
            break;

          case 'D':
            sprintf (buf, "%ld", maxin_size > maxout_size ? maxin_time : maxout_time);
            break;

          case 'E':
            sprintf (buf, "%lu", toterr);
            break;

          case 'H':
            sprintf (buf, "%d", tp->tm_hour);
            break;

          case 'I':
            sprintf (buf, "%d", infiles);
            break;              /*  TS 960620 */

          case 'J':
            sprintf (buf, "%ld", incps);
            break;

          case 'M':
            sprintf (buf, "%d", tp->tm_min);
            break;

          case 'O':
            sprintf (buf, "%d", outfiles);
            break;              /*  TS 960620 */

          case 'P':
            sprintf (buf, "%ld", outcps);
            break;

          case 'S':
            sprintf (buf, "%d", tp->tm_sec);
            break;

          case 'T':
            sprintf (buf, "%d", totfiles);
            break;              /*  TS 960620 */

          case 'U':
            sprintf (buf, "%ld", totcps);
            break;

          case 'V':
            sprintf (buf, "%ld", maxin_size);
            break;

          case 'W':
            sprintf (buf, "%ld", maxin_time);
            break;

          case 'X':
            sprintf (buf, "%ld", maxin_time ? (maxin_size / maxin_time) : 0);
            break;

          case 'Y':
            sprintf (buf, "%ld", maxin_size > maxout_size
                     ? (maxin_time ? (maxin_size / maxin_time) : 0)
                     : (maxout_time ? (maxout_size / maxout_time) : 0));
            break;

          case 'b':
            sprintf (buf, "%s", mtext[tp->tm_mon]);
            break;

          case 'c':
            sprintf (buf, "%ld", mycost);
            break;

          case 'd':
            sprintf (buf, "%d", tp->tm_mday);
            break;

          case 'f':
            sprintf (buf, "%ld", inerr);
            break;

          case 'g':
            sprintf (buf, "%ld", outerr);
            break;

          case 'i':
            sprintf (buf, "%ld", insize);
            break;

          case 'j':
            sprintf (buf, "%ld", ineff);
            break;

          case 'm':
            sprintf (buf, "%d", tp->tm_mon + 1);
            break;

          case 'o':
            sprintf (buf, "%ld", outsize);
            break;

          case 'p':
            sprintf (buf, "%ld", outeff);
            break;

          case 'r':
            sprintf (buf, "%ld", remotecost);
            break;              /* TJW 960812 */

          case 's':
            sprintf (buf, "%ld", dt);
            break;

          case 't':
            sprintf (buf, "%ld", totsize);
            break;

          case 'u':
            sprintf (buf, "%ld", toteff);
            break;

          case 'v':
            sprintf (buf, "%ld", maxout_size);
            break;

          case 'w':
            sprintf (buf, "%ld", maxout_time);
            break;

          case 'x':
            sprintf (buf, "%ld", maxout_time ? (maxout_size / maxout_time) : 0);
            break;

          case 'y':
            sprintf (buf, "%d", tp->tm_year % 100);
            break;

          case 'z':
            sprintf (buf, "%hu", costidx);
            break;

          default:
            sprintf (buf, "$%c", *c);
            break;
          }

          len -= strlen (buf);

          while (len-- > 0)
            fputc (pad, cost_log);

          fputs (buf, cost_log);
        }
      }

      fputc ('\n', cost_log);
    }
    else
    {
      fprintf (cost_log, costfmt, logline, "Sent ",
               (outfiles > 999) ? 999 : outfiles,
               numdisp (outsize, 4), outcps, outeff,
               outerr);
      fprintf (cost_log, costfmt, logline, "Rcvd ",
               (infiles > 999) ? 999 : infiles,
               numdisp (insize, 4), incps, ineff,
               inerr);
      fprintf (cost_log, costfmt, logline, "Total",
               (totfiles > 999) ? 999 : totfiles,
               numdisp (totsize, 4), totcps, toteff,
               toterr);
      fprintf (cost_log, "%s Seconds: %ld  Cost: %ld  System: %s\n\n",
               logline, dt, mycost, p);
    }

    fflush (cost_log);
    real_flush (fileno (cost_log));
  }

  if (session)
  {                             /* TJW 960527 changed to KB display */
    status_line (MSG_TXT (M_BYTE_SUMMARY),
                 infiles, (insize + 512) / 1024,
                 outfiles, (outsize + 512) / 1024,
                 totfiles, (totsize + 512) / 1024,
                 totcps);
  }

  status_line (MSG_TXT (M_COST_SUMMARY), dt, costidx, mycost, remotecost, p);

  maxin_size = 0;
  maxin_time = 0;
  maxout_size = 0;
  maxout_time = 0;

  if (lines_aftercall != 0)     /* SM 961106 */
    timer (3);                  /* AW 980522 we have already a function for pausing */

  return (mycost);
}

/* Create a small character string for mailsize and cost_of_call.
 * len is the string length of the output string and must be 4 or 5 ! */

char *
numdisp (long number, int len)
{
  int i = 0;
  char tempstr[6];
  long quotient, oldq, intq;
  static char e_stuff[6];       /* buffer for result */
  static char *rgchSize = " KMG";  /* TJW 960528 was "bKMG" - if we don't */

  /* say "Kb" then also don't say "b" !  */
  oldq = 0L;

  if (len == 4)
  {
    quotient = number;

    while (quotient >= 1024L)
    {
      oldq = quotient;
      quotient = oldq / 1024L;
      i++;
    }

    intq = quotient;

    /*
     * If more than 999 but less than 1024, it's a big fraction of
     * the next power of 1024. Get top two significant digits
     * (so 1023 would come out .99K, for example)
     */

    if (intq > 999)
    {
      intq = (intq * 25) / 256; /* 100/1024 */
      sprintf (e_stuff, ".%2ld%c", intq, rgchSize[++i]);
    }

    /*
     * If less than 10 and not small units, then get some decimal
     * places (e.g. 1.2M)
     */

    else if ((intq < 10) && (i != 0))
    {
      intq = (oldq * 5L) / 512L;  /* 10/1024 */
      sprintf (tempstr, "%02ld", intq);
      sprintf (e_stuff, "%c.%c%c", tempstr[0], tempstr[1], rgchSize[i]);
    }

    /*
     * Simple case. Just do it.
     */

    else
      sprintf (e_stuff, "%3ld%c", intq, rgchSize[i]);
  }
  else if (len == 5)
  {
    quotient = number;

    while (quotient >= 9999L)   /* AG990203 - use all 4 available */
    {                           /* for a detailed output */
      oldq = quotient;
      quotient = oldq / 1024L;
      i++;
    }

    intq = quotient;

    /*
     * If more than 999 but less than 1024, it's a big fraction of
     * the next power of 1024. Get top two significant digits
     * (so 1023 would come out .99K, for example)
     *
     * We don't need this anymore - AG 990207
     *

     if (intq > 999)
     {
     intq = (intq * 250) / 256;
     sprintf (e_stuff, ".%3ld%c", intq, rgchSize[++i]);
     }
     else
     */

    /*
     * If less than 10 and not small units, then get some decimal
     * places (e.g. 1.23M)
     */

    if ((intq < 10) && (i != 0))
    {
      intq = (oldq * 25L) / 256L;  /* 100/1024 */
      sprintf (tempstr, "%03ld", intq);
      sprintf (e_stuff, "%c.%c%c%c",
               tempstr[0], tempstr[1], tempstr[2], rgchSize[i]);
    }

    /*
     * If less than 100 and not small units, then get some decimal
     * places (e.g. 12.3M)
     */

    else if ((intq < 100) && (i != 0))
    {
      intq = (oldq * 5L) / 512L;  /* 10/1024 */
      sprintf (tempstr, "%03ld", intq);
      sprintf (e_stuff, "%c%c.%c%c",
               tempstr[0], tempstr[1], tempstr[2], rgchSize[i]);
    }

    /*
     * Simple case. Just do it.
     */

    else
      sprintf (e_stuff, "%4ld%c", intq, rgchSize[i]);
  }
  return (e_stuff);
}

void
show_os_memfree (void)
{
#if defined(OS_2)
  unsigned long aulSysInfo[QSV_MAX] =
  {
    0
  };
  char junk1[16], junk2[16];

#elif defined(_WIN32)           /* HJK 970814 */
  MEMORYSTATUS MemS;

#endif
  char junk[16];

#if defined(DOS16)

  sprintf (junk, "%3uK/%s", dos_largest_free_block (), numdisp
           (_stackavail (), 5));

#elif defined(OS_2)

  DosQuerySysInfo (1L, QSV_MAX, (PVOID) aulSysInfo,
                   sizeof (unsigned long) * QSV_MAX);

  sprintf (junk1, "%s", numdisp (aulSysInfo[QSV_MAXPRMEM - 1], 4));
  sprintf (junk2, "%s", numdisp (aulSysInfo[QSV_TOTAVAILMEM - 1], 4));
  sprintf (junk, "%s/%s", junk1, junk2);

#elif defined(_WIN32)           /* HJK 970814 */

  MemS.dwLength = sizeof (MemS);
  GlobalMemoryStatus (&MemS);
  sprintf (junk, "%s/%i%%", numdisp (MemS.dwAvailPhys, 4), (100 - MemS.dwMemoryLoad));

#else

  strcpy (junk, MSG_TXT (M_ENOUGH));  /*  for safety's sake (aka other os) */

#endif

  sb_move_puts (settingswin, SET_MEMAVAIL_ROW, SET_COL, junk);
}

void
update_files (int sender, char *filename, long filesize, long filetime, int *err)
{
  if (un_attended)
  {
    if (sender)
    {
      ++hist.files_out;
      hist.size_out += filesize;
      hist.time_out += filetime;
      hist.err_out += *err;
      TxStats.tot_errors += *err;  /* CEH 980803 */
    }
    else
    {
      ++hist.files_in;
      hist.size_in += filesize;
      hist.time_in += filetime;
      hist.err_in += *err;
      RxStats.tot_errors += *err;  /* CEH 980803 */
    }
  }

  if ((cost_log != NULL) && isOriginator && shortcostlog == NULL)
  {
    char *pszMode = (sender ? "Sent" : "Received");

    ltime = unix_time (NULL);
    tp = unix_localtime (&ltime);

    fprintf (cost_log, "%02i %3s %02i:%02i %s: %s (%ld)\n",
             tp->tm_mday, mtext[tp->tm_mon], tp->tm_hour, tp->tm_min,
             pszMode, filename, filesize);
    fflush (cost_log);
    real_flush (fileno (cost_log));
  }

  if (sender)
  {
    if (filesize > maxout_size)
    {
      maxout_size = filesize;
      maxout_time = filetime;
    }
  }
  else
  {
    if (filesize > maxin_size)
    {
      maxin_size = filesize;
      maxin_time = filetime;
    }
  }

  *err = 0;                     /* CEH 980803 */
}

void
screen_clear (void)
{
  unsigned int r;
  PCH q;

  if (!vfossil_installed)
  {
#ifdef DOS16
    if (noansitrash)            /* TS 961214    */
      clear_screen ();          /* r.hoerner */
    else
#endif
    {
      gotoxy (0, 0);
      scr_printf ("\033[H\033[2J");
    }
  }
  else
  {
    for (r = 0; (int) r < (int) SB_ROWS; r++)
    {
      q = (PCH) blanks;
      VioWrtCellStr ((PCH) q, (USHORT) (SB_COLS * 2), (USHORT) r,
                     (USHORT) 0, (HVIO) 0L);
    }

    gotoxy (0, 0);
  }
}

void
clear_eol (void)
{
  unsigned int x, y;
  PCH q;

  if (!vfossil_installed)
  {
#ifdef DOS16
    if (noansitrash)            /* TS 961214    */
      clear_to_eol ();
    else
#endif
      cputs (local_CEOL);       /*  TS 961214   */
  }
  else
  {
    x = (unsigned int) wherex ();
    y = (unsigned int) wherey ();
    q = (PCH) blanks;
    VioWrtCellStr ((PCH) q, (USHORT) ((SB_COLS - x) * 2), (USHORT) y,
                   (USHORT) x, (HVIO) 0L);
  }
}

void
log_product (int product, int version, int subversion)  /* TJW 960528 modified */
{
  char buf[20], *prodstr, buf2[100];

  switch (product)
  {
  case isOPUS:
    prodstr = PRDCT_NM (product);
    if (subversion == 48)
      subversion = 0;
    break;

  default:
    sprintf (buf, "FTSC product #%d", product);

    if (product >= 0 && product < PrdctHdr.ElemCnt)
      prodstr = PRDCT_NM (product);
    else
      prodstr = buf;
    break;
  }

  sprintf (remote_mailer, "%s %s %d.%02d",  /* AW 980209 */
           prodstr,
           MSG_TXT (M_VERSION), version, subversion);

  sprintf (buf2, "%s %s",
           MSG_TXT (M_REMOTE_USES), remote_mailer);

  status_line (buf2);
  return;
}

void
can_Janus (unsigned long baudrate, char *p)
{
  J_TYPESP j;

  janus_OK = 0;
  for (j = j_top; j != NULL; j = j->next)
  {
    if (strnicmp (p, j->j_match, strlen (j->j_match)) == 0)
    {
      janus_OK = 1;
      break;
    }
  }

  /* r. hoerner
   * allow "janus" even if no "reliable string" is sent
   * by the modem, but either BiDiBaud or JanusBaud is set */

  if (!janus_OK)
    janus_OK = janus_baud <= baudrate ? 1 : 0;
}

int
check_failed (char *fname, char *theirname, char *info, char *ourname)
{
  FILE *abortlog;
  char linebuf[64];
  char *p, *badname;
  int ret;

  ret = 0;
  abortlog = fopen (fname, read_ascii);
  if (abortlog == NULL)
    got_error (MSG_TXT (M_OPEN_MSG), fname);
  else
  {
    while (!feof (abortlog))
    {
      linebuf[0] = '\0';
      if (!fgets ((p = linebuf), 64, abortlog))
        break;
      while (*p >= ' ')
        ++p;
      *p = '\0';
      p = strchr (linebuf, ' ');
      *p = '\0';

      if (!stricmp (linebuf, theirname))
      {
        p = strchr ((badname = ++p), ' ');
        *p = '\0';
        if (!stricmp (++p, info))
        {
          strcpy (ourname, badname);
          ret = 1;
          break;
        }
      }
    }

    fclose (abortlog);
  }

  return (ret);
}

void
add_abort (char *fname, char *rname, char *cname,
           char *cpath, char *info)
{
  FILE *abortlog;
  char namebuf[100];

  strcpy (namebuf, cpath);
  strcat (namebuf, "BadWaZOO.001");
  unique_name (namebuf);
  rename (cname, namebuf);

  abortlog = fopen (fname, append_ascii);
  if (abortlog == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), fname);
    unlink (namebuf);
  }
  else
  {
    fprintf (abortlog, "%s %s %s\n", rname, namebuf + strlen (cpath), info);
    fclose (abortlog);
  }
}

void
remove_abort (char *fname, char *rname)
{
  FILE *abortlog, *newlog;
  char namebuf[100];
  char linebuf[100];
  char *p;
  int c;

  if (!dexists (fname))
    return;

  abortlog = fopen (fname, read_ascii);
  if (abortlog == NULL)
    got_error (MSG_TXT (M_OPEN_MSG), fname);
  else
  {
    strcpy (namebuf, fname);
    strcpy (namebuf + strlen (namebuf) - 1, "TMP");
    c = 0;

    newlog = fopen (namebuf, write_ascii);
    if (newlog == NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), namebuf);
      fclose (abortlog);
    }
    else
    {
      while (!feof (abortlog))
      {
        linebuf[0] = '\0';
        if (!fgets (linebuf, 64, abortlog))
          break;

        p = linebuf;
        while (*p > ' ')
          ++p;
        *p = '\0';

        if (stricmp (linebuf, rname))
        {
          *p = ' ';
          fputs (linebuf, newlog);
          ++c;
        }
      }

      fclose (abortlog);
      fclose (newlog);
      unlink (fname);

      if (c)
        rename (namebuf, fname);
      else
        unlink (namebuf);
    }
  }
}

/*-----------------------------------------------------------------------*/
/* returns the remaining time in seconds to transfer "bytes" byte        */
/* at cur_baud.rate_value                                                */
/*-----------------------------------------------------------------------*/
long
remaining (long bytes, long efficiency)
{
  long total, need;

  /* TJW 970407: changed to calculate with 8bits/byte (sync assumed)    */
  /*             instead of 10bits/byte (8 data + 1 start + 1 stop bit) */
  total = (bytes * 8L) / cur_baud.rate_value;
  if (efficiency == 100L)
    need = total;
  else
  {
    /* need less time if efficiency is higher, isn't it? */
    need = (total * 100L) / efficiency;
  }

  return (need);
}

static int LOCALFUNC
find_addr (char *node, ADDRP addr, char *d)
{
  int ret;

  ret = 1;

  if (alias->Node == 0xffff)
    addr->Zone = 0;
  else
    addr->Zone = alias->Zone;

  addr->Net = 0;
  addr->Node = 0;
  addr->Point = 0;
  addr->Domain = NULL;
  d[0] = '\0';

  if ((ret = sscanf (node, "%hd:%hd/%hd.%hd@%s",
                     &(addr->Zone), &(addr->Net), &(addr->Node),
                     &(addr->Point), d)) < 3)
  {
    if (alias->Node == 0xffff)
      addr->Zone = 0;
    else
      addr->Zone = alias->Zone;

    d[0] = '\0';

    if ((ret = sscanf (node, "%hd/%hd.%hd@%s",
                       &(addr->Net), &(addr->Node), &(addr->Point), d)) < 2)
    {
      addr->Net = alias->Net;
      d[0] = '\0';

      if ((ret = sscanf (node, "%hd.%hd@%s",
                         &(addr->Node), &(addr->Point), d)) < 1)
      {
        if ((ret = sscanf (node, ".%hd@%s", &(addr->Point), d)) > 0)
          addr->Node = alias->Node;
      }
      else if (ret == 1)
      {
        d[0] = '\0';
        sscanf (node, "%hd@%s", &(addr->Node), d);
      }
    }
    else if (ret == 2)
    {
      d[0] = '\0';
      sscanf (node, "%hd/%hd@%s", &(addr->Net), &(addr->Node), d);
    }
  }
  else if (ret == 3)
  {
    d[0] = '\0';
    sscanf (node, "%hd:%hd/%hd@%s",
            &(addr->Zone), &(addr->Net), &(addr->Node), d);
  }

  return (ret);
}

int
parse_address (char *node, ADDRP addr)
{
  int ret;
  char d[100];

  ret = find_addr (node, addr, d);

  if (d[0] != '\0')
    addr->Domain = add_domain (d);
  else
    addr->Domain = NULL;

  return (ret);
}

void
copy_address (ADDRP src, ADDRP dst)  /* r. hoerner */
{
  dst->Zone = src->Zone;
  dst->Net = src->Net;
  dst->Node = src->Node;
  dst->Point = src->Point;
  dst->Domain = src->Domain;
}

int
find_address (char *node, ADDRP addr)
{
  int i;
  int ret;
  char d[255];
  char *p;

  d[0] = '\0';
  p = skip_blanks (node);

  if (!isdigit (*p) && (*p != '.'))
  {
    (*userfunc) (p, addr);
    if ((addr->Net == 0xffff) || (addr->Node == 0xffff) || (addr->Zone == 0xffff))
      ret = 0;
    else
      ret = 1;
  }
  else
    ret = find_addr (p, addr, d);

  if (d[0] != '\0' && domain_name[0] != NULL)
    addr->Domain = add_domain (d);
  else
  {
    addr->Domain = domain_name[0];
    for (i = 0; i < num_domain_kludge; i++)
    {
      if (addr->Zone == domainkludge[i].zone)
      {
        addr->Domain = domain_name[domainkludge[i].domain];
        break;
      }
    }
  }

  return (ret);
}

char *
add_domain (char *d)
{
  char *p, *q;
  int i;

  for (i = 0; (i < DOMAINS) && ((p = domain_name[i]) != NULL); i++)
  {
    if (strnicmp (d, p, strlen (p)) == 0)
      return (p);
    q = domain_abbrev[i];
    if (q == NULL)              /* This can happen if no "Domain" line... */
      continue;
    if (strnicmp (d, q, strlen (q)) == 0)
      return (p);
  }

  if (i >= DOMAINS)
    return (NULL);

  domain_name[i] = strdup (d);
  return (domain_name[i]);
}

char *
find_domain (char *d)
{
  char *p, *q, *s;
  char c;
  int i, j, k;

  j = strlen (d);
  /* First see if we can find the whole domain name at the right */
  for (i = 0; (p = domain_name[i]) != NULL; i++)
  {
    k = strlen (p);
    if (k > j)
      continue;

    q = &(d[j - k]);
    if (strnicmp (q, p, k) == 0)
      return (p);
  }

  /* Ok, now see if we can find the abbreviated name at the right */
  for (i = 0; (p = domain_abbrev[i]) != NULL; i++)
  {
    k = strlen (p);
    if (k > j)
      continue;

    q = &(d[j - k]);
    if (strnicmp (q, p, k) == 0)
      return (domain_name[i]);
  }

  /* If there is a period in it, see if we can match the abbreviated name
     just before the period */
  if ((s = strrchr (d, '.')) != NULL)
  {
    c = *s;
    *s = '\0';
    j = strlen (d);
    for (i = 0; (p = domain_abbrev[i]) != NULL; i++)
    {
      k = strlen (p);
      if (k > j)
        continue;

      q = &(d[j - k]);
      if (strnicmp (q, p, k) == 0)
      {
        *s = c;
        return (domain_name[i]);
      }
    }

    *s = c;
  }

  return (NULL);
}

static char addr_str[100];

char *
Hex_Addr_Str (ADDRP a)
{
  if (a->Point != 0)
    sprintf (addr_str, "%04hx%04hx.pnt" DIR_SEPS "%08hx",
             a->Net, a->Node, a->Point);
  else
    sprintf (addr_str, "%04hx%04hx", a->Net, a->Node);

  return (addr_str);
}

char *
Full_Addr_Str (ADDRP a)
{
  char t1[10];
  char t2[30];
  char t3[10];
  char t4[50];

  if (a == NULL)                /* this _can_ happen! (r.hoerner) */
    return ("");

  if (a->Zone)
    sprintf (t1, "%hu:", a->Zone);
  else
    t1[0] = '\0';

  sprintf (t2, "%hu/%hu", a->Net, a->Node);

  if (a->Point)
    sprintf (t3, ".%hu", a->Point);
  else
    t3[0] = '\0';

  if (a->Domain != NULL)
    sprintf (t4, "@%s", a->Domain);
  else
    t4[0] = '\0';

  strcpy (addr_str, t1);
  strcat (addr_str, t2);
  strcat (addr_str, t3);
  strcat (addr_str, t4);
  return (addr_str);
}

unsigned short
crc_block (unsigned char *ptr, int count)
{
  register word crc;
  int i;

  for (crc = 0, i = 0; i < count; i++, ptr++)
    crc = xcrc (crc, (byte) * ptr);

  return (crc);
}

void
Data_Check (XMDATAP xtmp, int mode)
{
  int i;
  unsigned char cs;
  unsigned char *cp;
  unsigned short cs1;

  if (mode == CHECKSUM)
  {
    /* If we are in checksum mode, just do it */
    cp = xtmp->data_bytes;
    cs = 0;

    for (i = 0; i < 128; i++)
    {
      cs += *cp++;
    }
    xtmp->data_check[0] = cs;
  }
  else
  {
    /* If we are in CRC mode, run the characters through the CRC calculator */
    cs1 = crc_block (xtmp->data_bytes, 128);
    xtmp->data_check[0] = (unsigned char) (cs1 >> 8);
    xtmp->data_check[1] = (unsigned char) (cs1 & 0xff);
  }
}

void
wait_for_key (int ts)
{
  long t_idle;

  sb_show ();
  t_idle = longtimerset (ts);

  while (!longtimeup (t_idle) && (PEEKBYTE () == -1) && !(KEYPRESS ()))
    time_release ();

  if (KEYPRESS ())
    ts = (int) FOSSIL_CHAR ();
}

/* MMP 960419 timesync() should be called at the end of a session,
 * and if diff_trx variable is set, the the called_addr matches the
 * TimeSync address in the configuration file, we adjust the clock */

void
timesync (void)
{
  time_t diff;

  diff = diff_trx;
  diff_trx = 0;

  if (diff)
  {

#ifdef OS_2
    DATETIME dt;

#endif /* OS_2 */

#ifdef _WIN32                   /* alex, 07-02-13, timesync support for Win32 */
    SYSTEMTIME st;

#endif /* _WIN32 */

#if defined(DOS16) && !defined(__EMX__)  /*  TS 970404   */
    struct dostime_t dtime;
    struct dosdate_t ddate;

#endif /* defined(DOS16) && !defined(__EMX__)   */

    time_t newtime;
    struct tm t;

    if (max_diff_trx)
      if (labs ((long) diff) > max_diff_trx)
        return;

    unix_time (&newtime);
    newtime += diff;

    /* Important! Make a copy of the tm struct - status_line() will read
     * the current time into it the static buffer used by localtime() */

    t = *unix_localtime (&newtime);
    status_line (MSG_TXT (M_SYNCMYCLOCK),
                 t.tm_year % 100, t.tm_mon + 1, t.tm_mday,
                 t.tm_hour, t.tm_min, t.tm_sec);
#ifdef OS_2
#define TIMESET
    DosGetDateTime (&dt);       /* Get timezone right */
    dt.hours = (unsigned char) t.tm_hour;
    dt.minutes = (unsigned char) t.tm_min;
    dt.seconds = (unsigned char) t.tm_sec;
    dt.hundredths = 0;
    dt.day = (unsigned char) t.tm_mday;
    dt.month = (unsigned char) (t.tm_mon + 1);
    dt.year = (unsigned short) (t.tm_year + ((t.tm_year < 70) ? 2000 : 1900));  /* AG 990120 Y2K fix */
    dt.weekday = (unsigned char) t.tm_wday;
    DosSetDateTime (&dt);
#endif /* OS_2 */

#ifdef DOS16
#define TIMESET
#ifndef __EMX__                 /*  TS 970404   */
    dtime.hour = (unsigned char) t.tm_hour;
    dtime.minute = (unsigned char) t.tm_min;
    dtime.second = (unsigned char) t.tm_sec;
    dtime.hsecond = 0;
    ddate.day = (unsigned char) t.tm_mday;
    ddate.month = (unsigned char) (t.tm_mon + 1);
    ddate.year = (unsigned short) (80 + t.tm_year);
    ddate.dayofweek = (unsigned char) t.tm_wday;
    _dos_settime (&dtime);
    _dos_setdate (&ddate);
#else /*  TS 970404, #ifndef __EMX__  */
    status_line (":TimeSync not yet available");
#endif /*  TS 970404, #ifndef __EMX__  */
#endif /* DOS16 */

#ifdef _WIN32                   /* alex, 97-02-13 */
#define TIMESET
    GetLocalTime (&st);
    st.wYear = t.tm_year + ((t.tm_year < 70) ? 2000 : 1900);
    st.wMonth = t.tm_mon + 1;
    st.wDay = t.tm_mday;
    st.wHour = t.tm_hour;
    st.wMinute = t.tm_min;
    st.wSecond = t.tm_sec;
    if (!SetLocalTime (&st))
      status_line (":timesync(), SetLocalTime() failed, rc = %d",
                   GetLastError ());
#endif

#ifdef __unix__
#define TIMESET
    status_line ("!TimeSync is still not implemented under Linux");
#endif

#ifndef TIMESET
#   error Need some tweaking here in timesync() to set the date/time
#endif
  }
}

void
cleanup_req (char *fname)       /* cleanup *.REQ file from size information */
{
  FILE *fpi, *fpo;
  char tmpfname[PATHLEN];
  char *p1, *p2;
  char line[1024];

  strcpy (tmpfname, fname);
  strcpy (tmpfname + strlen (tmpfname) - 3, "rqq");  /* use *.RQQ as tempfile */

  unlink (tmpfname);

  if (rename (fname, tmpfname))
  {
    status_line ("!Unable to rename %s to %s.", fname, tmpfname);
    return;
  }

  fpo = fopen (fname, write_ascii);
  if (fpo == NULL)
  {
    status_line ("!Can't open %s.", fname);
    return;
  }

  fpi = fopen (tmpfname, read_ascii);
  if (fpi == NULL)
  {
    status_line ("!Can't open %s.", tmpfname);
    fclose (fpo);
    return;
  }

  while (fgets (line, sizeof (line), fpi))
  {                             /* read line from input file */
    /* status_line("!Read RQQ:'%s'", line); */
    p2 = line;
    p1 = skip_to_blank (p2);    /* skip to position after filename */
    p2 = skip_blanks (p1);      /* skip to begin of next word      */
    while (*p2 && *p2 != '$')
    {
      p1 = skip_to_blank (p2);
      p2 = skip_blanks (p1);
    }

    if (*p2 == '$')
    {                           /* do we have size information ? */
      p2 = skip_to_blank (p2);  /* skip it. */
      strcpy (p1, p2);          /* copy the stuff at the end over it */
    }

    /* status_line("!Write REQ:'%s'", line); */
    fputs (line, fpo);          /* write line to output file         */
  }

  fclose (fpi);
  fclose (fpo);
  unlink (tmpfname);
}

void
killblanks (char *s)
{
  char *p = s;

  while (*p)
  {
    if (isspace (*p))
      strcpy (p, p + 1);
    else
      p++;
  }
}

int
eval (char *e)                  /* evaluate expression -> true = 1, false = 0, invalid=-1 */
{
  int ret;
  char *op, *left, *right;

  // killblanks(e); // MR moved to btconfig.c  /* remove blanks, tabs.. */
  strlwr (e);                   /* make all lowercase */
  if ((op = strstr (e, "==")) != NULL)
  {                             /* x == y */
    left = e;
    *op++ = '\0';
    right = ++op;
    ret = (strcmp (left, right) == 0);
  }
  else if ((op = strstr (e, "!=")) != NULL)
  {                             /* x != y */
    left = e;
    *op++ = '\0';
    right = ++op;
    ret = (strcmp (left, right) != 0);
  }
  else if (!strcmp (e, "true") || !strcmp (e, "common"))
  {                             /* true / common */
    ret = 1;
  }
  else if (!strcmp (e, "false") || !strcmp (e, "ignore"))
  {                             /* false / ignore */
    ret = 0;
  }
  else
    ret = -1;                   /* syntax error */

  return ret;
}

/* TJW 960928: some compilers use ANSI style time_t, which begins at 1.1.1900 */
/* Binkley wants time_t beginning at 1.1.1970 (UNIX style).                   */
/* So Binkley will ever call unix_* functions and they will be remapped via   */
/* #define either directly to the library functions (if they are UNIX style)  */
/* or to the adaption functions below for ANSI style libraries...             */

#if defined(ANSI_TIME_T)

int
ansi_stat (const char *fname, struct stat *buffer)
{
  int ret;

  ret = stat (fname, buffer);
  buffer->st_ctime -= ANSI_TIME_T_DELTA;
  buffer->st_mtime -= ANSI_TIME_T_DELTA;
  buffer->st_atime -= ANSI_TIME_T_DELTA;
  return ret;
}

int
ansi_fstat (int handle, struct stat *buffer)
{
  int ret;

  ret = fstat (handle, buffer);
  buffer->st_ctime -= ANSI_TIME_T_DELTA;
  buffer->st_mtime -= ANSI_TIME_T_DELTA;
  buffer->st_atime -= ANSI_TIME_T_DELTA;
  return ret;
}

time_t
ansi_time (time_t * timeptr)
{
  time_t t;

  t = time (timeptr);
  if (t != (time_t) - 1)
  {
    if (timeptr)
      *timeptr -= ANSI_TIME_T_DELTA;
    t -= ANSI_TIME_T_DELTA;
  }
  return t;
}

time_t
ansi_mktime (struct tm * time)
{
  time_t t;

  t = mktime (time);
  if (t != (time_t) - 1)
    t -= ANSI_TIME_T_DELTA;
  return t;
}

struct tm *
ansi_localtime (const time_t * timeval)
{
  time_t t;

  t = *timeval + ANSI_TIME_T_DELTA;
  return localtime (&t);
}

struct tm *
ansi_gmtime (const time_t * timeval)
{
  time_t t;

  t = *timeval + ANSI_TIME_T_DELTA;
  return gmtime (&t);
}

char *
ansi_ctime (const time_t * timeval)
{
  time_t t;

  t = *timeval + ANSI_TIME_T_DELTA;
  return ctime (&t);
}

int
ansi_utime (char *fname, struct utimbuf *buf)
{
  struct utimbuf abuf, *p;

  p = buf;
  if (p)
  {
    abuf = *p;
    p = &abuf;
    p->modtime += ANSI_TIME_T_DELTA;
    p->UT_ACTIME += ANSI_TIME_T_DELTA;
  }
  return utime (fname, p);
}

#endif

// TJW: some more time functions
// reason why we need that:
// BT-XE shall run with a valid TZ setting - so the unix_* functions will
// use/generate UTC(!) timestamps - not local time. But the file transfer
// protocols transmit/receive filestamps as "seconds (local time) since
// 1.1.1970 (UTC)" - weird but true ...
// The file timestamps for files stored on HD shall also NOT get a time shift
// like the normal functions do it when TZ != UTC0

/* AW 980208 we don't need some of the time routines below */
#if 0
// this is just the inverse function for gmtime()

time_t
XX_inv_unix_gmtime (struct tm * t)
{
  unsigned int s;
  unsigned int isleap, leaps_since70;
  unsigned int days_thisyear;
  static unsigned int days[12] =
  {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
  };

  // This is the correct formula for leap year determination:
  // isleap=((t->tm_year%400==0) || (t->tm_year%4==0 && t->tm_year%100!=0)) ? 1:0;
  //
  // But following will be OK, too, until year 2099 (and since UNIX time and
  // 32bit integers will have some other problems even before this date, this
  // should be sufficient):

  isleap = (t->tm_year % 4 == 0) ? 1 : 0;

  leaps_since70 = (t->tm_year - 68 - 1) / 4;

  days_thisyear = days[t->tm_mon] + t->tm_mday - 1;
  if (t->tm_mon > 1)
    days_thisyear += isleap;

  s = ((((t->tm_year - 70) * 365 + leaps_since70 + days_thisyear) * 24
        + t->tm_hour) * 60 + t->tm_min) * 60U + t->tm_sec;

  return s;
}

// timeshift
time_t
XX_unix_shifttime (time_t t)
{
  return XX_inv_unix_gmtime (unix_localtime (&t));
}

// reverse timeshift
time_t
XX_unix_rshifttime (time_t t)
{
  return unix_mktime (unix_gmtime (&t));
}
#endif

// same as unix_utime, but does not do time shifts due to TZ
// (like if we would be in UTC0 timezone)
int
unix_utime_noshift (char *fname, struct utimbuf *buf)
{
  struct utimbuf abuf;

  if (buf)
  {
    abuf = *buf;
    abuf.modtime += TIMEZONE;
    abuf.UT_ACTIME += TIMEZONE;
    return unix_utime (fname, &abuf);
  }

  return unix_utime (fname, buf);
}

// same as unix_stat, but does not do time shift due to TZ
int
unix_stat_noshift (const char *fname, struct stat *buffer)
{
  int ret;

  ret = unix_stat (fname, buffer);
  buffer->st_ctime -= TIMEZONE;
  buffer->st_mtime -= TIMEZONE;
  buffer->st_atime -= TIMEZONE;

  return ret;
}

// same as unix_mktime, but does not do time shift due to TZ
time_t
unix_mktime_noshift (struct tm * time)
{
  time_t t;

  t = unix_mktime (time);
  return t - TIMEZONE;
}

/* TJW 961028 semaphore file operations */
int
sem_tas (char *dir, char *base_fname, char *ext_fname)
{
  char semfname[PATHLEN], tmpfname[PATHLEN], ext[10];
  int rc;
  FILE *fp;

  if (ext_fname)                /* ext_fname != NULL then use provided */
    strcpy (ext, ext_fname);    /* extension as semfile extension      */
  else                          /* else use TaskNumber as extension    */
    sprintf (ext, "%02x", TaskNumber);

  sprintf (tmpfname, "%s%s.$%02x", dir, base_fname, TaskNumber);
  sprintf (semfname, "%s%s.%s", dir, base_fname, ext);

  fp = fopen (tmpfname, write_binary);  /* create temporary flag file */
  fclose (fp);
  rc = rename (tmpfname, semfname);  /* test and set operation */
  if (rc && maxbsyage)
  {
    struct stat st;

    if (!unix_stat (semfname, &st))
    {
      time_t age = age_seconds (st.st_mtime) / 60;

      if (age > maxbsyage)
      {
        status_line (MSG_TXT (M_OLD_BSY_IGNORED), semfname, age);
        remove (semfname);
        rc = rename (tmpfname, semfname);
      }
    }
  }

  if (rc)
    unlink (tmpfname);

  return rc;
  /* rc==0, test and set was successful */
  /* rc!=0, test and set failed         */
}

int
sem_clr (char *dir, char *base_fname, char *ext_fname)
{
  char semfname[PATHLEN], ext[10];
  int rc, tries;
  struct stat st;

  if (ext_fname)                /* ext_fname != NULL then use provided */
    strcpy (ext, ext_fname);    /* extension as semfile extension      */
  else                          /* else use TaskNumber as extension    */
    sprintf (ext, "%02x", TaskNumber);

  sprintf (semfname, "%s%s.%s", dir, base_fname, ext);
  tries = 0;

  do
  {
    rc = unlink (semfname);
    if (rc)
    {
      if (!stat (semfname, &st))
      {
        status_line (">sem_clr: could not delete %s (rc=%d, errno=%d)!",
                     semfname, rc, errno);
        timer (10);
      }
      else
        rc = 0;
    }
  }
  while (++tries < 6 && rc != 0);

  return rc;
}

void
show_cs_port (long sbaud, bool mode)  /*  TS 961122   */
{
  char junk[32];

  if (settingswin)
  {
    sprintf (junk, "%s%d", port_device, 1 + port_ptr);
    sb_move_puts (settingswin, SET_PORT_ROW, SET_COL, junk);
    sprintf (junk, "%-6lu", sbaud);
    sb_move_puts (settingswin, SET_BAUD_ROW, SET_COL, junk);
    sb_move_puts (settingswin, SET_BAUD_ROW, 0,
            mode ? MSG_TXT (M_STATUS_DCERATE) : MSG_TXT (M_STATUS_DTERATE));
  }
}

/*--------------------------------------------------------------------*/
/* Log EMSI strings for debug purposes                     R. Hoerner */
/*                                                                    */
/* input: which, 'S' or 'R' (send- or receive)                        */
/*           and 'A' or 'C' ("Answering call" resp. "Calling"         */
/*        what ,  the actual EMSI string to log                       */
/*        len  ,  the size of EMSI string to log                      */
/*--------------------------------------------------------------------*/
void
log_emsi (char which, char *what, int len)
{
  FILE *fp;
  char c = 0;                   /* avoid EMX warning */
  char junk[40];

  if (!debug_emsi || emsilog == NULL)
    return;

  fp = share_fopen (emsilog, append_ascii, DENY_WRITE);

  if (fp == NULL)
    return;

  fseek (fp, 0, SEEK_END);

  if (len && what)
  {
    c = what[len];              /* save char from emsi-string  */
    what[len] = 0;
  }

  unix_time (&ltime);

  tp = unix_localtime (&ltime);

  sprintf (junk, "(%02d/%02d/%02d %02d:%02d:%02d)",
           tp->tm_year % 100, tp->tm_mon + 1, tp->tm_mday,
           tp->tm_hour, tp->tm_min, tp->tm_sec);

  switch (which)                /* "how much?" "such much?" */
  {
  case 'S':
    fprintf (fp, "-EmsiSend  %s %s\r\n", junk, what);
    break;

  case 'R':
    fprintf (fp, "-EmsiRecv  %s %s\r\n", junk, what);
    break;

  case 'A':
    fprintf (fp, "-Answering %s\r\n", junk);
    break;

  case 'C':
    fprintf (fp, "-Calling   %s\r\n", junk);
    break;
  }

  fflush (fp);
  real_flush (fileno (fp));
  fclose (fp);

  if (len && what)
    what[len] = c;              /* restore char to emsi-string */
}

/*--------------------------------------------------------------------*/
/* useful helper                                                      */
/*--------------------------------------------------------------------*/
int
SameAddress (ADDRP addr1, ADDRP addr2)
{
  return ((addr1->Zone == addr2->Zone)
          && (addr1->Net == addr2->Net)
          && (addr1->Node == addr2->Node)
          && (addr1->Point == addr2->Point)
          && ((addr1->Domain == addr2->Domain)
              || (addr1->Domain == NULL)
              || (addr2->Domain == NULL)));
}

/*--------------------------------------------------------------------*/
int
invalid_address (ADDRP addr)
{
  if (addr == NULL)
    return 1;

  if (addr->Zone == 0)
    return 1;

  return 0;
}


/* MR 970314  QueryNodeStruct */

NODESTRUC *
QueryNodeStruct (ADDRP addr, int create)
{
  NODESTRUC *pnode = NULL;
  NODESTRUC *snode = NULL;

  for (pnode = knownnode; pnode != NULL; pnode = pnode->next)
  {
    if (SameAddress (&pnode->addr, addr))
    {
      if (pnode->addr.Domain == NULL)
        pnode->addr.Domain = addr->Domain;
      return (pnode);           /* node found: bye */
    }

    snode = pnode;
  }

  /* if we reach this point then we didn't find an according structure */
  pnode = NULL;

  if (create && !invalid_address (addr))
  {
    pnode = (NODESTRUC *) calloc (1, sizeof (NODESTRUC));

    if (pnode == NULL)
      printf (MSG_TXT (M_MEM_ERROR));
    else
    {
      if (knownnode == NULL)    /* if first structure ;) */
        knownnode = pnode;
      else
        snode->next = pnode;

      copy_address (addr, &pnode->addr);

      pnode->CallerIDList = NULL;
      pnode->FPoll = NULL;
      pnode->ExtraDir = NULL;
      pnode->ExtraDirIn = NULL;
      pnode->ExtraDirHold = 0;
      pnode->ExtraDirNotify = 0;
      pnode->SyncHydraSessions = 0;
      pnode->tzutc = -1;
      pnode->next = NULL;
      pnode->Flags = 0;
    }
  }

  return (pnode);
}

/* CFS 970622  QueryRandomGroup */

RANDOM_SYSTEM *
QueryRandomGroup (char *name, int create)
{
  word count;
  RANDOM_SYSTEM *newg = NULL;

  if (!name[0])
    return NULL;

  for (count = 0; count < RandomGroups; count++)
  {
    if (!stricmp (name, RandomSystemInfo[count]->id))
      return (RandomSystemInfo[count]);
  }

  if (create)
  {
    newg = (RANDOM_SYSTEM *) malloc (sizeof (RANDOM_SYSTEM));
    if (newg)
    {
      RandomSystemInfo = realloc (RandomSystemInfo, (RandomGroups + 1) *
                                  sizeof (RANDOM_SYSTEM *));
      if (!RandomSystemInfo)
      {
        RandomGroups = 0;
        free (newg);
      }
      else
      {
        RandomSystemInfo[RandomGroups++] = newg;
        strcpy (newg->id, name);
        newg->sysop = NULL;
        newg->sysopcount = 0;
        newg->system = NULL;
        newg->systemcount = 0;
        newg->location = NULL;
        newg->locationcount = 0;
      }
    }
  }

  return (newg);
}

/*--------------------------------------------------------------------*/
/* Query special flags on selective nodes                  R. Hoerner */
/* If no adress specified, return global variable                     */
/* If no individual flag is specified on the node, return global      */
/* else OR nodes flags with global flag and return                    */
/*--------------------------------------------------------------------*/
int
QueryNodeFlag (int global, unsigned long flag, ADDRP addr)
{
  int local;
  char c[20];
  NODESTRUC *pnode = NULL;

  if (invalid_address (addr) || knownnode == NULL)
    return (global);

  switch (flag)
  {
  case NOEMSI:
    sprintf (c, "NoEMSI");
    break;

  case NOWAZOO:
    sprintf (c, "NoWaZOO");
    break;

  case NOJANUS:
    sprintf (c, "NoJanus");
    break;

  case NOHYDRA:
    sprintf (c, "NoHydra");
    break;

  case NOZED:
    sprintf (c, "NoZedZap");
    break;

  case FREEREQ:
    sprintf (c, "ReqOnUs");
    break;

  case NOREQ:
    sprintf (c, "NoRequests");
    break;

  case NOSEA:
    sprintf (c, "NoSeaLink");
    break;

  case NODIET:
    sprintf (c, "NoDietIFNA");
    break;

  case NOPICK:
    sprintf (c, "NoPickup");
    break;

  case NOHYCHAT:
    sprintf (c, "NoHydraChat");
    break;

  case DO_BELL:
    sprintf (c, "Gong");
    break;

  case NOJOKERS:
    sprintf (c, "NoWildcards");
    break;

  case FREEPOLL:
    sprintf (c, "FreePoll");
    break;

  case CALLBACK:
    sprintf (c, "CallBack");
    break;

  case TIMESYNC:
    sprintf (c, "TimeSync");
    break;

  case NOZIP:
    sprintf (c, "NoZedZip");
    break;

  default:
    sprintf (c, "0x%04x", (int) flag);
    break;
  }

  for (pnode = knownnode; pnode; pnode = pnode->next)
  {
    if (SameAddress (&pnode->addr, addr))
    {
      local = ((pnode->Flags & flag) == 0) ? 0 : 1;
      global |=local;

      status_line (">%s setting for %d:%d/%d.%d@%s is '[%s] %s'",
                   local ? "individual" : "global",
                   addr->Zone,
                   addr->Net,
                   addr->Node,
                   addr->Point,
                   addr->Domain,
                   global ? "*" : " ",  /* EITHER global OR individual */
                   c);          /* flag matches                */

      return (global);
    }
  }

  status_line (">global setting '[%s] %s'", global ? "*" : " ", c);

  return (global);
}

/*--------------------------------------------------------------------------*/

int
comment_or_eol (char c)
{
  return ((c == 0) || (c == ';') || (c == '%'));  /* comment or eol */
}

/*--------------------------------------------------------------------------*/

char *
point_to_next_string (char *p)
{
  if (p != NULL)
    while (!isspace (*p) && !comment_or_eol (*p))
      p++;

  return (p);
}

/*--------------------------------------------------------------------------*/

char *
PasswordFromFile (ADDRP addr)   /* r. hoerner */
{
  static char pw[16];           /* must be static! */
  char line[256];
  FILE *fp;
  char *p, *q;
  ADDR node;

  if (invalid_address (addr) || passwordfile == NULL)  /* no data: done */
    return NULL;

  fp = fopen (passwordfile, read_ascii);  /* can't open it: done */
  if (fp == NULL)
    return NULL;

  memset (pw, 0, sizeof (pw));
  memset (line, 0, sizeof (line));

  while (fgets (line, sizeof (line) - 1, fp))  /* for ervery line read do: */
  {
    /* Password 2:2476/7 password ; any comment */
    if (!line)                  /* empty line */
      continue;

    p = line;
    p = skip_blanks (p);
    if (comment_or_eol (*p))    /* no info in line */
      continue;

    /* RS 10.97  Start: String "Password" is optional, like FastLst   */
    /* If exist just ignore and interpret the next character if exist */
    if (strnicmp (p, "PASSWORD ", 9) == 0)  /* "PASSWORD" line */
      p = point_to_next_string (p);
    /* RS 10.97 end */

    if (comment_or_eol (*p))    /* comment or eol: next line */
      continue;

    p = skip_blanks (p);
    if (comment_or_eol (*p))    /* comment or eol: next line */
      continue;

    q = p;                      /* save pointer to nodenumber */

    q = point_to_next_string (q);  /* go to password  field     */
    if (comment_or_eol (*q))    /* comment or eol: next line */
      continue;

    q = skip_blanks (q);
    if (comment_or_eol (*q))    /* comment or eol: next line */
      continue;

    /* ------- now p points to nodenumber, q points to password --------- */

    if (parse_address (p, (ADDRP) & node) < 3)  /* is not Zone:Net/Node */
      continue;

    if ((node.Zone == addr->Zone)
        && (node.Net == addr->Net)
        && (node.Node == addr->Node)
        && (node.Point == addr->Point)
        && ((node.Domain == addr->Domain) || (node.Domain == NULL)))
      /* || (NULL == addr->Domain)   // this would leak security! */
    {
      fclose (fp);
      memset (pw, 0, sizeof (pw));

      p = q;
      q[8] = 0;                 /* beware of longer passwords */
      while (*p > ' ')
        p++;
      *p = 0;
      strcpy (pw, q);
      return (pw);
    }

    memset (line, 0, sizeof (line));
  }

  fclose (fp);
  return (NULL);
}

/*--------------------------------------------------------------------*/

int
GetAltPhone (char *old, ADDRP addr)  /* get alternative phone number */
{
  PHONE *phone = NULL;

  if (invalid_address (addr) || old == NULL || phone1 == NULL)
    return (0);

  for (phone = phone1; phone; phone = phone->next)
  {
    if (SameAddress ((ADDR *) phone, addr))
    {
      if (phone->num == NULL)
        return (0);

      strncpy (old, phone->num, 40);
      old[39] = '\0';
      return (1);
    }
  }

  return (0);
}

/*--------------------------------------------------------------------*/

void
GetAltFidoFlag (word * old, ADDRP addr)  /* get alternative flags     */
{
  PHONE *phone = NULL;

  if (invalid_address (addr) || old == NULL || phone1 == NULL)
    return;

  for (phone = phone1; phone; phone = phone->next)
  {
    if (SameAddress ((ADDR *) phone, addr))
    {
      *old = phone->fido;
      return;
    }
  }

  return;
}

/*--------------------------------------------------------------------*/

void
GetAltModemflag (word * old, ADDRP addr)  /* get alternative flags    */
{
  PHONE *phone = NULL;

  if (invalid_address (addr) || old == NULL || phone1 == NULL)
    return;

  for (phone = phone1; phone; phone = phone->next)
  {
    if (SameAddress ((ADDR *) phone, addr))
    {
      *old = phone->modem;
      return;
    }
  }

  return;
}

/* VRP 990820 start */

int
GetAltOnLineTime (short *start, short *end, ADDRP addr)
{
  ONLINE_TIME *online_time = NULL;
  int wday, hour, min, sec, hdths;

  if (invalid_address (addr) || start == NULL || end == NULL || online_time1 == NULL)
    return 0;

  dostime (&wday, &hour, &min, &sec, &hdths);

  wday = (short) weekdays[wday % 7];

  for (online_time = online_time1; online_time; online_time = online_time->next)
  {
    if (SameAddress ((ADDR *) online_time, addr) && (online_time->days & wday))
    {
      *start = online_time->start;
      *end = online_time->end;

      status_line (">OnLine time: %d:%d/%d.%d %d - %d Days - %d (wday - %d)",
                   addr->Zone, addr->Net, addr->Node, addr->Point,
                   online_time->start, online_time->end, online_time->days,
                   wday);

      if (online_time->start == 0 && online_time->end == 1440)
        return 1;
      if (IsOnLine (online_time->start, online_time->end))
        return 0;
    }
  }

  return 0;
}

/*
 * Hidden lines support
 *
 */

int
GetHiddenInfo (int hidden_no, int zone, struct _newnode *nodedes)
{
  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;
  ADDR addr;
  int wday, hour, min, sec, hdths;
  int i;
  int ret = 0;

  addr.Zone = zone;
  addr.Net = nodedes->NetNumber;
  addr.Node = nodedes->NodeNumber;
  addr.Point = (nodedes->NodeFlags & B_point) ? nodedes->HubNode : 0;
  addr.Domain = NULL;

  for (hidden = hidden1; hidden; hidden = hidden->next)
  {
    if (SameAddress ((ADDR *) hidden, &addr))
    {
      for (hidden_line = hidden->hidden_line1, i = 1;
           hidden_line;
           hidden_line = hidden_line->next, i++)
      {
        if (i == hidden_no)
        {
          nodedes->NodeFlags = (nodedes->NodeFlags & B_point) ?
            (hidden_line->fido | B_point) : hidden_line->fido;
          nodedes->ModemType = (byte) hidden_line->modem;
          strcpy (nodedes->PhoneNumber, hidden_line->num);

          dostime (&wday, &hour, &min, &sec, &hdths);
          wday = (short) weekdays[wday % 7];

          if (hidden_line->days & wday)
          {
            if (hidden_line->start == 0 && hidden_line->end == 1440)
            {
              nodedes->NodeFlags |= B_CM;
            }
            else
            {
              nodedes->online_start = hidden_line->start;
              nodedes->online_end = hidden_line->end;
            }
          }
          else
          {
            nodedes->online_start = 0;
            nodedes->online_end = 0;
          }

          ret = (int) hidden_line->days;
          break;
        }
      }

      break;
    }
  }

  return (ret);
}

/* VRP 990820 end */

/*--------------------------------------------------------------------
           1         2         3         4         5
 0123456789012345678901234567890123456789012345678901234567890
0
1  a00000b   a00000b   a00000b   a00000b
2  1     2   1     2   1     2   1     2
3  1     2   1     2   1     2   1     2
4  1     2   1     2 X 1     2   1     2
5  c33333d   c33333d   c33333d   c33333d
6  4     5   4     5 X 4     5   4     5
7  4     5   4     5   4     5   4     5
8  4     5   4     5   4     5   4     5
9  e66666f   e66666f   e66666f   e66666f
10
  -------------------------------------------------------------------- */
/* This array says: "if '1' then set line (horiz. or vert.) of display */
/*-------------------------------------------------------------------- */
static char seg7[10][7] =
{
  /*0,1, 2, 3, 4, 5, 6 ............. line number, see picture above   */
  {1, 1, 1, 0, 1, 1, 1},        /* 0 */
  {0, 0, 1, 0, 0, 1, 0},        /* 1 */
  {1, 0, 1, 1, 1, 0, 1},        /* 2 */
  {1, 0, 1, 1, 0, 1, 1},        /* 3 */
  {0, 1, 1, 1, 0, 1, 0},        /* 4 */
  {1, 1, 0, 1, 0, 1, 1},        /* 5 */
  {1, 1, 0, 1, 1, 1, 1},        /* 6 */
  {1, 0, 1, 0, 0, 1, 0},        /* 7 */
  {1, 1, 1, 1, 1, 1, 1},        /* 8 */
  {1, 1, 1, 1, 0, 1, 1}         /* 9 */
};

/*--------------------------------------------------------------------*/
/* This array says: "if 1 then set edge in display"                   */
/*--------------------------------------------------------------------*/
static char edge7[10][6] =
{
  /*a,b, c, d, e, f ............... edge number, see picture above  */
  {0, 0, 1, 1, 0, 0},           /* 0 */
  {0, 1, 0, 1, 0, 1},           /* 1 */
  {0, 0, 0, 0, 0, 0},           /* 2 */
  {1, 0, 0, 1, 1, 0},           /* 3 */
  {1, 1, 0, 1, 0, 1},           /* 4 */
  {0, 0, 0, 0, 0, 0},           /* 5 */
  {0, 0, 1, 0, 0, 0},           /* 6 */
  {1, 0, 0, 1, 0, 1},           /* 7 */
  {0, 0, 0, 0, 0, 0},           /* 8 */
  {0, 0, 0, 1, 0, 0}            /* 9 */
};

char mid = '*';

/*--------------------------------------------------------------------*/
/* This is a digital clock, displayed in screen save mode             */
/* The routine returns 1 if the current line should be displayed,     */
/* otherwise it returns 0                                             */
/*--------------------------------------------------------------------*/
/* The first version displayed the clock 1 time each 1 second.        */
/* This led to a cpu load of 5-6 percent (pentium-200)                */
/* I've changed it so, that a complete clock is "painted" once per    */
/* minute, whilst once per second the lines with the "working" dots   */
/* are painted only                                                   */
/*--------------------------------------------------------------------*/
int
update_clock (short row)
{
  int i;
  short col;
  int std_one, std_ten, min_one, min_ten, sec_odd;
  unsigned char cstd_one, cstd_ten, cmin_one, cmin_ten;
  signed short cur_line;
  signed int edge, line;
  static int old_sec_odd;

  if (!DrawClock)
    return (1);

  /* -------------------------------------------------------------------- */
  /* At the beginning of the page we'll update our time structure         */
  /* -------------------------------------------------------------------- */
  if (!row)
  {
    unix_time (&ltime);
    tp = unix_localtime (&ltime);

    /* if minute has changed then blank the line, because we're going to
     * draw a blank line now */
    if (clock_minute != tp->tm_min)
      for (i = 0; i < SB_COLS; i++)
        blanks[i * 2] = ' ';
  }

  /* -------------------------------------------------------------------- */
  /* At the end of a screen page we'll update "clock_minute"              */
  /* -------------------------------------------------------------------- */
  if (row == SB_ROWS - 1)
  {
    int was_clock_minute = clock_minute;

    clock_minute = tp->tm_min;

    /* if minute has changed then blank the line, because we're going to
     * draw a blank line now */
    if (was_clock_minute != clock_minute)
    {
      for (i = 0; i < SB_COLS; i++)
        blanks[i * 2] = ' ';
      return 1;
    }

    return (0);
  }

  /* -------------------------------------------------------------------- */
  std_one = tp->tm_hour % 10;
  std_ten = tp->tm_hour / 10;
  min_one = tp->tm_min % 10;
  min_ten = tp->tm_min / 10;
  sec_odd = tp->tm_sec & 1;

#if (0)                         /* enable it to force rapid changes */
  min_one = (tp->tm_sec / 5) % 10;
  min_ten = (tp->tm_sec / 5) / 10;
  clock_minute = -1;
#endif

  if (min_one & 1)
    cur_line = row - (SB_ROWS - 11);
  else
    cur_line = row;

  if (min_one & 2)
    col = SB_COLS;
  else
    col = 0;

  /* -------------------------------------------------------------------- */
  /* If we are out of our clock lines, we can leave                       */
  /* -------------------------------------------------------------------- */
  if ((cur_line < 1) || (cur_line > 10))
  {
    if (clock_minute != tp->tm_min)  /* minute changed: draw blanks */
      return 1;
    else
      return 0;                 /* else simply skip these lines */
  }

  /* -------------------------------------------------------------------- */
  /* If we are inside our clock lines, we can leave if we have already    */
  /* drawn our clock. Exception: these two dots between hour and minute   */
  /* -------------------------------------------------------------------- */
  if (clock_minute == tp->tm_min)  /* same minute: do dots only   */
  {
    if ((cur_line == 4) || (cur_line == 6))
    {
      if (sec_odd != old_sec_odd)
        goto draw_anyway;
    }

    return (0);                 /* else skip these lines       */
  }

draw_anyway:

  /* -------------------------------------------------------------------- */
  /* Start with a clean line                                              */
  /* -------------------------------------------------------------------- */
  for (i = 0; i < SB_COLS; i++) /* blank blanks             */
    blanks[i * 2] = ' ';

  if (!clock_char)
  {
    cstd_one = (unsigned char) std_one + '0';
    cstd_ten = (unsigned char) std_ten + '0';
    cmin_one = (unsigned char) min_one + '0';
    cmin_ten = (unsigned char) min_ten + '0';
  }
  else
  {
    cstd_one = (unsigned char) clock_char;
    cstd_ten = (unsigned char) clock_char;
    cmin_one = (unsigned char) clock_char;
    cmin_ten = (unsigned char) clock_char;
  }

  edge = -1;
  line = -1;

  /* -------------------------------------------------------------------- */
  /* See the seg7[] and edge[] arrays to understand this                  */
  /* -------------------------------------------------------------------- */
  switch (cur_line)
  {
  case 1:
    line = 0;                   /* seg7[0]             */
    edge = 0;                   /* edge[0] and edge[1] */
    break;

  case 4:
    if (sec_odd)
      blanks[col + 20 * 2] = mid;
    /* fall through */

  case 2:
  case 3:
    line = 1;                   /* seg7[1] and seg7[2] */
    break;

  case 5:
    line = 3;                   /* seg7[3]             */
    edge = 2;                   /* edge[2] and edge[3] */
    break;

  case 6:
    if (sec_odd)
      blanks[col + 20 * 2] = mid;
    old_sec_odd = sec_odd;
    /* fall through */

  case 7:
  case 8:
    line = 4;                   /* seg7[4] and seg7[5] */
    break;

  case 9:
    line = 6;                   /* seg7[6]             */
    edge = 4;                   /* edge[4] and edge[5] */
    break;

  case 10:
    return 1;

  }

  /* -------------------------------------------------------------------- */
  /* See the seg7[] and edge[] arrays to understand this                  */
  /* -------------------------------------------------------------------- */
  if (edge >= 0)                /* then ALWAYS a horizontal line */
  {
    if (edge7[std_ten][edge])
      blanks[col + 2 * 2] = cstd_ten;
    if (edge7[std_one][edge])
      blanks[col + 12 * 2] = cstd_one;
    if (edge7[min_ten][edge])
      blanks[col + 22 * 2] = cmin_ten;
    if (edge7[min_one][edge])
      blanks[col + 32 * 2] = cmin_one;

    if (edge7[std_ten][edge + 1])
      blanks[col + 8 * 2] = cstd_ten;
    if (edge7[std_one][edge + 1])
      blanks[col + 18 * 2] = cstd_one;
    if (edge7[min_ten][edge + 1])
      blanks[col + 28 * 2] = cmin_ten;
    if (edge7[min_one][edge + 1])
      blanks[col + 38 * 2] = cmin_one;

    if (seg7[std_ten][line])
      for (i = 3; i < 8; i++)
        blanks[col + i * 2] = cstd_ten;
    if (seg7[std_one][line])
      for (i = 13; i < 18; i++)
        blanks[col + i * 2] = cstd_one;
    if (seg7[min_ten][line])
      for (i = 23; i < 28; i++)
        blanks[col + i * 2] = cmin_ten;
    if (seg7[min_one][line])
      for (i = 33; i < 38; i++)
        blanks[col + i * 2] = cmin_one;
  }
  else if (line >= 0)
  {
    if (seg7[std_ten][line])
      blanks[col + 2 * 2] = cstd_ten;
    if (seg7[std_one][line])
      blanks[col + 12 * 2] = cstd_one;
    if (seg7[min_ten][line])
      blanks[col + 22 * 2] = cmin_ten;
    if (seg7[min_one][line])
      blanks[col + 32 * 2] = cmin_one;

    if (seg7[std_ten][line + 1])
      blanks[col + 8 * 2] = cstd_ten;
    if (seg7[std_one][line + 1])
      blanks[col + 18 * 2] = cstd_one;
    if (seg7[min_ten][line + 1])
      blanks[col + 28 * 2] = cmin_ten;
    if (seg7[min_one][line + 1])
      blanks[col + 38 * 2] = cmin_one;
  }
  return 1;
}

/*--------------------------------------------------------------------*/

void
show_our_capabilities (char *when)
{
  if (debugging_log)
  {
    char junk[80];

    sprintf (junk,
          ">%s Remote has:   (%s) Hydra (%s) Janus (%s) ZedZip (%s) ZedZap",
             when,
             remote_capabilities & DOES_HYDRA ? HAS : HAS_NOT,
             remote_capabilities & DOES_IANUS ? HAS : HAS_NOT,
             remote_capabilities & ZED_ZIPPER ? HAS : HAS_NOT,
             remote_capabilities & ZED_ZAPPER ? HAS : HAS_NOT);
    status_line (junk);
    sprintf (junk,
          ">%s My protocols: (%s) Hydra (%s) Janus (%s) ZedZip (%s) ZedZap",
             when,
             my_capabilities & DOES_HYDRA ? HAS : HAS_NOT,
             my_capabilities & DOES_IANUS ? HAS : HAS_NOT,
             my_capabilities & ZED_ZIPPER ? HAS : HAS_NOT,
             my_capabilities & ZED_ZAPPER ? HAS : HAS_NOT);
    status_line (junk);
  }
}

/*--------------------------------------------------------------------*/
/* MR 970215 functions to control modem window */

REGIONP modemwin = NULL;

void
ctrl_modemwin (int show)
{
  static REGIONP callwinsave;
  short mdmwin_col, mdmwin_size;

  if (show && !modemwin)
  {
    char junk[2];
    BOXTYPE *boxp;

    if (SB_COLS > 80)           /* AS 970310 */
      mdmwin_size = 20 + ((SB_COLS - 80) / 5);
    else
      mdmwin_size = 20;

    mdmwin_col = SB_COLS - mdmwin_size;

    callwinsave = callwin;

    modemwin = sb_new_win (MODEMWIN_ROW, mdmwin_col,
                           MODEMWIN_HIGH, mdmwin_size,
                           ' ', colors.call, boxtype, colors.frames,
                           MSG_TXT (M_MODEM_ACTIVITY));

    callwin = sb_new_win (CALLWIN_ROW, CALLWIN_COL,
                          CALLWIN_HIGH, (short) (CALLWIN_SIZE -
                                                 mdmwin_size + 1),
                          ' ', colors.call, boxtype, colors.frames,
                          MSG_TXT (M_RECENT_ACTIVITY));

    boxp = &box[boxtype];

    sprintf (junk, "%c", boxp->tt);
    sb_move_puts_abs (modemwin, 0, 0, junk);

    sprintf (junk, "%c", boxp->tb);
    sb_move_puts_abs (modemwin, MODEMWIN_HIGH - 1, 0, junk);

    callwin->linesize = callwinsave->linesize;
    callwin->buffer = callwinsave->buffer;
    callwin->endbuff = callwinsave->endbuff;
    callwin->lastline = callwinsave->lastline;
    callwin->lastshown = callwinsave->lastshown;
    modemwin->wflags |= SB_SCROLL | SB_WRAP;

    redraw_callwin ();
    sb_move (modemwin, MODEMWIN_HIGH - 3, 0);
    sb_dirty ();
    sb_show ();
  }
  else if (!show && modemwin)
  {
    callwinsave->linesize = callwin->linesize;
    callwinsave->buffer = callwin->buffer;
    callwinsave->endbuff = callwin->endbuff;
    callwinsave->lastline = callwin->lastline;
    callwinsave->lastshown = callwin->lastshown;

    free (callwin);
    free (modemwin);

    callwin = callwinsave;
    modemwin = NULL;

    sb_fill (callwin, ' ', colors.call);
    sb_box (callwin, boxtype, colors.frames);
    sb_caption (callwin, MSG_TXT (M_RECENT_ACTIVITY), colors.frames);
    redraw_callwin ();
    sb_dirty ();
    sb_show ();
  }
  else if (show && modemwin)    /* AS 970310 */
  {
    char junk[2];
    BOXTYPE *boxp;

    boxp = &box[boxtype];
    sprintf (junk, "%c", boxp->tt);
    sb_move_puts_abs (modemwin, 0, 0, junk);

    sprintf (junk, "%c", boxp->tb);
    sb_move_puts_abs (modemwin, MODEMWIN_HIGH - 1, 0, junk);

    sb_move (modemwin, MODEMWIN_HIGH - 3, 0);
    sb_dirty ();
    sb_show ();
  }
}

void
print2modemwin (short type, short cmd)
{
  if (modemwin && cmd)
  {
    sb_putc_att (modemwin, cmd, type);
    sb_show ();
  }
}

/* TJW 970512 strntcpy(char *destination, char *source, size_t count)
 * works as strncpy(), but ensures \0-termination at destination[count-1].
 * So just give the char array size as last parameter for strntcpy.
 * Please use this function as it is far more save than strcpy() and more
 * comfortable as strncpy(). */

char *
strntcpy (char *dst, char *src, size_t count)
{
  count--;
  strncpy (dst, src, count);
  dst[count] = '\0';
  return dst;
}

/* AW 971226 moved show_alive/no_longer_alive from b_sessio.c (less overlay overhead) */
/* RDH *** Changes begin */
/* show_alive(void)                          */
/* function should be called                 */
/* always, after forcerescancheck or so      */
/* It creates a I_ALIVE file in the flag_dir */
/* if this file doesn't exist.               */

int
show_alive (void)
{
  struct stat buf;
  char fname[PATHLEN];          /* TJW 960526 static -> dynamic */
  static long show_time = 0L;
  FILE *fp;

  if ((!flag_dir) || (!should_show_alive))
    return (0);

  if (show_time && !timeup (show_time))
    return (0);

  sprintf (fname, "%si_alive.%02x", flag_dir, TaskNumber);
  if (!unix_stat (fname, &buf))
  {
    show_time = timerset (30 * PER_SECOND);  /* Don't show for at least 30 seconds */
    return (0);
  }

  fp = fopen (fname, write_binary);

  if (fp)
    fclose (fp);

  show_time = timerset (30 * PER_SECOND);  /* Don't show for at least 30 seconds */
  return 0;
}
/* RDH *** Changes end */


/* MR 961123 a new function to remove I-ALIVE upon exits...
 *           code simply "stolen" from b_initva.c */

void
no_longer_alive (void)
{
  /* RDH 9604 */

  char fname[PATHLEN];
  struct stat buf;

  if (flag_dir)
  {
    sprintf (fname, "%si_alive.%02x", flag_dir, TaskNumber);
    if (!unix_stat (fname, &buf))
      unlink (fname);
  }
}


static char *LOCALFUNC          /* AW 980209 */
store_TZ_helper (char *envp)
{
  char *s;

  if ((s = (char *) malloc (4 + strlen (envp))) != NULL)
  {
    strcpy (s, "TZ=");
    strcat (s, envp);
  }
  else
    exit (4);

  return s;
}

char *                          /* AW 980201 store value "SET TZ=xxxx" instead */
store_TZ (void)                 /* of pointer into environment to TZ */
{
  char *envp;

  if ((envp = getenv ("TZ")) == NULL)
  {                             /* no SET TZ= defined */
    envp = "TZ=UTC0";
    putenv (envp);
    envp = envp + 3;            /* skip "TZ=" */
  }

  tzset ();

  return store_TZ_helper (envp);
}

#ifndef __unix__

/* AW 980301 calculate the real timediff between local time and UTC */

long
get_bink_timezone (void)
{
  static long bink_timezone = 0;
  time_t localtime;
  char *tmpTZ;

  if (!tzutc_timer || longtimeup (tzutc_timer))
  {
    tmpTZ = store_TZ ();        /* store current TZ    */
    putenv ("TZ=UTC0");         /* temporary set UTC=0 */
    tzset ();
    localtime = time (NULL);    /* get local time      */
    putenv (tmpTZ);
    tzset ();
    bink_timezone = time (NULL) - localtime;  /* store difference    */
    bink_timezone /= 10;
    bink_timezone *= 10;
    if (TZ)
      free (TZ);
    TZ = tmpTZ;
    /* next calculation at full hour */
    tzutc_timer = longtimerset ((3600 - (localtime % 3600)) * PER_SECOND);
  }

  return bink_timezone;
}

#else

long
get_bink_timezone (void)
{
  time_t utc;
  static long bink_timezone = 0;
  struct tm *tm;

  if (!tzutc_timer || longtimeup (tzutc_timer))
  {
    time (&utc);
    tm = gmtime (&utc);
    bink_timezone = mktime (tm) - utc;
  }
  return bink_timezone;
}

#endif

/* $Id: misc.c,v 1.20 1999/09/27 20:51:34 mr Exp $ */
