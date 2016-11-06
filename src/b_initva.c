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
 * Filename    : $Source: E:/cvs/btxe/src/b_initva.c,v $
 * Revision    : $Revision: 1.12 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:19 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Variable Initialization
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static void LOCALFUNC compile_externs (void);
static void LOCALFUNC fillin_defaults (struct secure *, struct secure *);

/**
 ** b_initvars -- called before parse_config. Sets defaults that we want
 ** to have set FIRST.
 **/

void
b_initvars ()
{
  int k;

  /* VRP 990830 start */
  ONLINE_TIME *online_time;
  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;

  /* VRP 990830 end */

  TZ = store_TZ ();             /* AW 980209 get timezone (cfg_get_nodetimezone needs it */

  e_input = (char *) calloc (1, E_INPUT_LEN);  /* status_line only */
  if (!e_input)
  {
    printf (MSG_TXT (M_MEM_ERROR));
    exit (252);
  }

  e_ptrs = (BINK_EVENT *) malloc (256 * sizeof (BINK_EVENT));
  if (e_ptrs == (BINK_EVENT *) NULL)
  {
    printf (MSG_TXT (M_BAD_EVENT_ALLOC));
    exit (252);
  }

  SendHoldIfCaller = 0;         /* VRP 990824 */
  RestorePoll = 0;              /* VRP 990826 */

  /* VRP 990820 start */

  while (online_time1 != NULL)
  {
    online_time = online_time1;
    online_time1 = online_time1->next;

    if (online_time->Domain != NULL)
      free (online_time->Domain);
    free (online_time);
  };

  while (hidden1 != NULL)
  {
    hidden = hidden1;
    hidden1 = hidden1->next;

    while (hidden->hidden_line1 != NULL)
    {
      hidden_line = hidden->hidden_line1;
      hidden->hidden_line1 = hidden->hidden_line1->next;

      if (hidden_line->num != NULL)
        free (hidden_line->num);
      free (hidden_line);
    };

    if (hidden->Domain != NULL)
      free (hidden->Domain);
    free (hidden);
  };

  /* VRP 990820 end */

  memset ((char *) e_ptrs, 0, 256 * sizeof (BINK_EVENT));
  memset ((char *) &DEFAULT, 0, sizeof (DEFAULT));

  KNOWN = DEFAULT;              /* Does some of the work      */
  KNOWN.rq_Limit = -1;
  KNOWN.byte_Limit = -1L;
  KNOWN.time_Limit = -1;
  KNOWN.rq_Cum_Limit = -1;
  KNOWN.byte_Cum_Limit = -1L;
  KNOWN.time_Cum_Limit = -1L;

  PROT = KNOWN;                 /* Initially the same default */
  SPECIAL = KNOWN;              /* Initially the same default */

  memset (&errlvlshell, 0, 255);
  memset (&modem, 0, sizeof (struct modem_commands));

  for (k = 0; k < 10; k++)      /* Zero the phone scan list   */
    scan_list[k] = NULL;

  for (k = 0; k < MAX_EXTERN; k++)  /* And the external protocols */
    protocols[k] = NULL;

  alias = (AKA *) calloc (1, sizeof (AKA));
  if (alias == NULL)
  {
    printf (MSG_TXT (M_MEM_ERROR));
    exit (2);
  }

  alias->Zone = 1;              /* Make sure we have a adress */
  alias->Net = alias->Node = (unsigned short) -1;  /* Default Fido address */
  alias->Point = 0;
  alias->Domain = NULL;
  alias->next = NULL;

#if 0
  for (k = 0; k < FPADDR_CNT + 1; k++)  /* And the fpaddrs list         */
  {
    fpaddr[k].addr.Zone = fpaddr[k].addr.Net =
      fpaddr[k].addr.Node = fpaddr[k].addr.Point = 0;  /* TJW 960505 */
    fpaddr[k].addr.Domain = NULL;
    fpaddr[k].Size = 0L;
    fpaddr[k].Delta_T = (time_t) 300;  /* Default is 5 minutes */
    fpaddr[k].and_op = 0;       /* Default is OR operation for accepting calls */
  }
#endif

#ifdef  DOS16                   /*  TS 961215   */
  cshape = get_cshape ();
#endif

  b_init ();

  baud = 2;
  cur_baud = pbtypes[baud];

  memset (&dta_str, 0, sizeof (dta_str));

#ifdef SET_CONMODE
  setmode (fileno (stdin), O_BINARY);
  setmode (fileno (stdout), O_BINARY);
#endif

  noBBS = MSG_TXT (M_NO_BBS);   /* TJW 961028 */
  BBSesc = MSG_TXT (M_PRESS_ESCAPE);  /* TJW 961028 */

#ifdef SMP_API
  if (!smp)
    smp = smp_init ();
#endif
}


/**
 ** b_defaultvars -- called after all parse_config passes complete.
 ** sets anything not handled by parse_config to default if we know it.
 **/

void
b_defaultvars ()
{
  int i;
  char *p;

  /* VRP 990820 start */

  ONLINE_TIME *online_time;
  short deltaTZ;
  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;

  /* VRP 990820 end */

  TZ = store_TZ ();             /* AW 980209 Timezone settings after config */
  tzutc_timer = 0;

  unix_time (&etm);             /* r.hoerner: set etm NOW */

  if (alias == NULL)
    alias = (AKA *) calloc (1, sizeof (AKA));
  if (alias == NULL)
    exit (4);

  if (location == NULL)
    location = "Somewhere";
  if (myphone == NULL)
    myphone = "-Unpublished-";
  if (myflags == NULL)
    myflags = "";
  if (mymaxbaud == NULL)
    mymaxbaud = "";
  if (download_path == NULL)
    download_path = "";

  if (version7)
    nodelist_name = "nodex";
  else
    nodelist_name = "nodelist";

#ifdef BBS_SPAWN
  if (BBSopt == OPT_NONE)
    BBSopt = OPT_SPAWN;
#endif

  if (modem.init_cnt == 0)
    modem.init[modem.init_cnt++] = ctl_string ("|AT|");  /* MR 970310 */
  if (modem.busy == NULL)
    modem.busy = ctl_string ("v");
  if (modem.hangup == NULL)     /* MR 970325 */
    modem.hangup = ctl_string ("|`v~~^````` |");
  if (modem.init_setup == NULL) /* MR 970310 moved from mdm_proc.c */
    modem.init_setup = ctl_string ("~^````` |");
  if (modem.dial_setup == NULL) /* MR 970310 moved from mdm_proc.c */
    modem.dial_setup = ctl_string ("~^`````");

  if (net_info == NULL)
    net_info = ctl_string ("." DIR_SEPS);

  /* Initialize the modem response strings if user didn't */

  if (mdm_resps == NULL)
  {
    mdm_resps = def_mdm_resps;
    resp_count = max_resp_count = def_resp_count;
  }

  /* Initialize session-level no-WaZOO and no-EMSI flags */

  no_WaZOO_Session = no_WaZOO;
  no_EMSI_Session = no_EMSI;

  /* Set up "boss" and "point" addresses correctly if we can */

  if (boss_addr.Zone == 0)
    boss_addr.Zone = alias->Zone;

  if (boss_addr.Net == 0)
  {
    memcpy (&boss_addr, alias, sizeof (ADDR));
    boss_addr.Point = 0;
  }

  if (task_dir)
    strcpy (HistoryFileName, task_dir);
  else
    strcpy (HistoryFileName, BINKpath);
  if (HistoryFileName[strlen (HistoryFileName) - 1] != DIR_SEPC)
    strcat (HistoryFileName, DIR_SEPS);
  strcat (HistoryFileName, "callhist.all");

  if ((pvtnet >= 0) && (alias->Point != 0))
  {
    my_addr = boss_addr;
    my_addr.Point = alias->Point;
    alias->Net = (word) pvtnet;
    alias->Node = alias->Point;
    alias->Point = 0;
  }
  else
    my_addr = *(ADDR *) alias;

  /* If we have the minimum information to do netmail, set the flag */

  if ((alias->Zone != 0)
      && (alias->Net != 0)
      && (system_name != NULL)
      && (sysop != NULL)
      && (hold_area != NULL)
      && (DEFAULT.sc_Inbound != NULL))
  {
    net_params = 1;
    flag_file (INITIALIZE, (ADDR *) alias, 0);
  }

  if (system_name == NULL)
    system_name = "";

  if (hold_area == NULL)
    hold_area = ctl_string ("." DIR_SEPS);

  if (flag_dir == NULL)
    flag_dir = ctl_string ("." DIR_SEPS);

  if (DEFAULT.sc_Inbound == NULL)
    DEFAULT.sc_Inbound = ctl_string ("." DIR_SEPS);

  if (DEFAULT.class == NULL)
    DEFAULT.class = "0";

  /* Make the "higher class" requests at least as well off as the
   * "lowest class"... */

  fillin_defaults (&KNOWN, &DEFAULT);
  fillin_defaults (&PROT, &KNOWN);
  fillin_defaults (&SPECIAL, &KNOWN);

  /* AW 980220 get Maximus priviledge levels into .security and .xkeys */
  get_PrivKey ((word *) & DEFAULT.security, &DEFAULT.xkeys, DEFAULT.class);
  get_PrivKey ((word *) & KNOWN.security, &KNOWN.xkeys, KNOWN.class);
  get_PrivKey ((word *) & PROT.security, &PROT.xkeys, PROT.class);
  get_PrivKey ((word *) & SPECIAL.security, &SPECIAL.xkeys, SPECIAL.class);

  if (extern_index)
    compile_externs ();         /* generate extern_protocols  */

  if (!colors.calling && colors.hold)
    colors.calling = (unsigned char) (((colors.hold & 0x70) >> 4) |
                                      ((colors.hold & 0x7) << 4) |
                                      (colors.hold & 0x8));

  if ((!colors.popup) && colors.call)
    colors.popup = colors.call;

  first_block = 0;

  /* Make our domain first in the list */

  if (my_addr.Domain != NULL)
  {
    for (i = 0; i < DOMAINS; i++)
    {
      if (domain_name[i] == my_addr.Domain)
        break;
    }

    if ((i > 0) && (i < DOMAINS) && (domain_name[i] == my_addr.Domain))
    {
      p = domain_name[0];
      domain_name[0] = domain_name[i];
      domain_name[i] = p;
      p = domain_nodelist[0];
      domain_nodelist[0] = domain_nodelist[i];
      domain_nodelist[i] = p;
      p = domain_abbrev[0];
      domain_abbrev[0] = domain_abbrev[i];
      domain_abbrev[i] = p;
    }
  }

  /* VRP 990820 start */

  /*
   * Convert overrided online times to UTC
   */

  deltaTZ = (short) (TIMEZONE / 60L);
  for (online_time = online_time1; online_time; online_time = online_time->next)
  {
    if ((online_time->end - online_time->start) == 1440 ||
        (online_time->start - online_time->end) == 1440)
    {
      online_time->start = 0;
      online_time->end = 1440;
    }
    else
    {
      online_time->start += deltaTZ;
      online_time->end += deltaTZ;

      if (online_time->start < 0)
        online_time->start += 1440;
      else if (online_time->start > 1440)
        online_time->start -= 1440;

      if (online_time->end < 0)
        online_time->end += 1440;
      else if (online_time->end > 1440)
        online_time->end -= 1440;
    }
  }

  for (hidden = hidden1; hidden; hidden = hidden->next)
  {
    for (hidden_line = hidden->hidden_line1;
         hidden_line;
         hidden_line = hidden_line->next)
    {
      if ((hidden_line->end - hidden_line->start) == 1440 ||
          (hidden_line->start - hidden_line->end) == 1440)
      {
        hidden_line->start = 0;
        hidden_line->end = 1440;
      }
      else
      {
        hidden_line->start += deltaTZ;
        hidden_line->end += deltaTZ;

        if (hidden_line->start < 0)
          hidden_line->start += 1440;
        else if (hidden_line->start > 1440)
          hidden_line->start -= 1440;

        if (hidden_line->end < 0)
          hidden_line->end += 1440;
        else if (hidden_line->end > 1440)
          hidden_line->end -= 1440;
      }
    }
  }

  /* VRP 990820 end */

  set_prior (PRIO_MODEM);

  if (Cominit (port_ptr, buftmo) != 0x1954)
  {
#ifdef DOS16
    printf (MSG_TXT (M_DRIVER_DEAD_1), "COM", 1 + port_ptr);
#else
    printf (MSG_TXT (M_DRIVER_DEAD_1), port_device, 1 + port_ptr);
#endif
    printf (MSG_TXT (M_DRIVER_DEAD_2));
    printf (MSG_TXT (M_DRIVER_DEAD_3));

    set_prior (PRIO_NORMAL);
    exit (1);
  }

  i = un_attended;
  un_attended = 0;

  set_baud (max_baud.rate_value, 0);
  un_attended = i;

  MDM_ENABLE (lock_baud && (pbtypes[baud].rate_value >= lock_baud) ?
              max_baud.rate_mask : pbtypes[baud].rate_mask);
  RAISE_DTR ();
  XON_ENABLE ();

  set_prior (PRIO_REGULAR);

  InUseBBSBanner = BBSbanner;
  bufp = (BUF *) calloc (1, sizeof (BUF));  /* TJW 960416 */
  if (!bufp)
  {
    status_line (MSG_TXT (M_MEM_ERROR));
    set_prior (PRIO_NORMAL);
    exit (2);
  }

  Txbuf = bufp->_Txbuf;

  /* TJW960522 Init Hydra CRC tables once here */
#ifdef HAVE_HYDRA
  hydracrcinit ();
#endif

  /* Do a couple of system-related things.
   * Maybe they shouldn't be here -- but better here than in BT.C.
   *
   * Because of some config verbs that affect its behavior, mtask_find
   * MUST follow parse_config! */

#ifdef DOS16
  fossil_ver ();
#endif
  mtask_find ();

#ifdef OS_2
  LoadOS2DLLs ();               /* load OS/2 related DLLs now       */
  atexit (UnLoadOS2DLLs);

  snoop_open ((PSZ) pipename);  /* open the snoop pipe after readcfg */
  atexit (snoop_close);         /* 'cause we need the final name    */
  {
    char junk[32];

    SendToDlg ((PSZ) ANNOUNCE, 0, 0, BINK_INTRO);  /* need the TaskNumber for this! */
    sprintf (junk, "Task = %d", TaskNumber);
    SendToDlg ((PSZ) junk, 0, 0, BINK_START);
  }
#endif

#ifdef _WIN32
  // HJK 970205 - Now Windows also uses snoop
  snoop_open ((PSZ) pipename);  /* open the snoop pipe after readcfg */
  atexit (snoop_close);         /* 'cause we need the final name    */
#endif

  IPC_Open ();
  IPC_SetStatus (MSG_TXT (M_MCP_INIT));
  IPC_Ping ();

  LoadScanList (0, 0);          /* Get default scan list */

  if (local_inbound == NULL)
    local_inbound = PROT.sc_Inbound;
}

static void LOCALFUNC
compile_externs ()
{
  register char *c;
  register int i;
  char rgchT[100];
  int j, k, l;
  char *p;
  char x;

  i = l = 0;                    /* start at beginning     */
  rgchT[0] = '\0';

  for (k = 0; protocols[k] != NULL; k++)  /* Total no. of protos    */
  {
    c = protocols[k];           /* Point at filename      */
    if (!dexists (c))           /* Is it there?           */
    {
      printf ("%s %s\n", MSG_TXT (M_NO_PROTOCOL), c);
      continue;
    }

    p = NULL;
    while (*c)                  /* Until end of string    */
    {
      if ((*c == DIR_SEPC) || (*c == ':'))  /* Look for last path */
        p = c;                  /* Delimiter              */
      c++;
    }

    if (strlen (p) < 3)         /* If no name,            */
      continue;                 /* No protocol...         */

    p++;                        /* Point to the           */
    x = (char) toupper (*p);    /* First character        */
    if (strchr (rgchT, x) != NULL)
    {
      printf ("%s %s\n", MSG_TXT (M_DUP_PROTOCOL), c);
      continue;
    }

    protos[l].first_char = x;   /* Makes lookup fast      */
    protos[l++].entry = (short) k;  /* Now we know where      */

    rgchT[i++] = x;             /* Store first char       */
    rgchT[i++] = ')';           /* Then a ')'             */
    c = ++p;                    /* Point to 2nd char      */

    for (j = 0; j < 9; j++)     /* Up to 9 chars more     */
    {
      if (*c != '.')            /* If no comma yet,       */
      {
        rgchT[i++] = (char) tolower (*c);  /* store the char and    */
        ++c;                    /* bump the pointer       */
      }
      else
        rgchT[i++] = ' ';       /* otherwise pad it       */
    }

    rgchT[i++] = ' ';           /* And one more space     */
    rgchT[i] = '\0';            /* Need for testing       */
  }

  if (!i)                       /* If we got none,        */
    return;                     /* Return now.            */

  i += 2;                       /* Total for malloc       */
  if ((extern_protocols = calloc (1, (unsigned) i)) == NULL)
    return;                     /* Return on failure      */

  strcpy (extern_protocols, rgchT);  /* Copy the string        */
  return;                       /* Back to caller         */
}

static void LOCALFUNC
fillin_defaults (struct secure *higher, struct secure *lower)
{
  if (higher->time_Limit == -1)
    higher->time_Limit = lower->time_Limit;
  if (higher->byte_Limit == -1L)
    higher->byte_Limit = lower->byte_Limit;
  if (higher->rq_Limit == -1)
    higher->rq_Limit = lower->rq_Limit;

  if (higher->time_Cum_Limit == -1)
    higher->time_Cum_Limit = lower->time_Cum_Limit;
  if (higher->byte_Cum_Limit == -1L)
    higher->byte_Cum_Limit = lower->byte_Cum_Limit;
  if (higher->rq_Cum_Limit == -1)
    higher->rq_Cum_Limit = lower->rq_Cum_Limit;
  if (higher->class == NULL)
    higher->class = lower->class;

  if (higher->rq_OKFile == NULL)
    higher->rq_OKFile = lower->rq_OKFile;
  if (higher->rq_Template == NULL)
    higher->rq_Template = lower->rq_Template;
  if (higher->sc_Inbound == NULL)
    higher->sc_Inbound = lower->sc_Inbound;
#ifdef SPECIALMAGICS
  if (higher->rq_FILES == NULL)
    higher->rq_FILES = lower->rq_FILES;
  if (higher->rq_About == NULL)
    higher->rq_About = lower->rq_About;
#endif
}

/**
 ** b_exitproc -- called by mainline to do exit processing.
 **/
void
b_exitproc ()
{
  /* VRP 990820 start */

  ONLINE_TIME *online_time;
  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;

  while (online_time1 != NULL)
  {
    online_time = online_time1;
    online_time1 = online_time1->next;

    if (online_time->Domain != NULL)
      free (online_time->Domain);
    free (online_time);
  };

  while (hidden1 != NULL)
  {
    hidden = hidden1;
    hidden1 = hidden1->next;

    while (hidden->hidden_line1 != NULL)
    {
      hidden_line = hidden->hidden_line1;
      hidden->hidden_line1 = hidden->hidden_line1->next;

      if (hidden_line->num != NULL)
        free (hidden_line->num);
      free (hidden_line);
    };

    if (hidden->Domain != NULL)
      free (hidden->Domain);
    free (hidden);
  };

  /* VRP 990820 end */

  if (command_line_un)
    modem_busy ();

  no_longer_alive ();
  vfossil_cursor (1);

  while (KEYPRESS ())
    FOSSIL_CHAR ();

  gotoxy (0, SB_ROWS - 1);      /* TJW 960517 */
  cputs ("\n\n");               /* TJW 960517 */
  clear_eol ();
  printf (MSG_TXT (M_THANKS), ANNOUNCE);
  clear_eol ();
  printf (MSG_TXT (M_ANOTHER_FINE_PRODUCT));

  if (vfossil_installed)
    vfossil_close ();

#ifdef SET_CONMODE
  setmode (fileno (stdin), O_TEXT);
  setmode (fileno (stdout), O_TEXT);
#endif

  if (!share)
  {
    set_prior (PRIO_MODEM);
    MDM_DISABLE ();
    set_prior (PRIO_REGULAR);
  }

  IPC_SetStatus (MSG_TXT (M_MCP_EXITING));
  IPC_Close ();
}

#ifdef _WIN32
// HJK 98/06/13 - set application title
void
SetWindowTitle (char *status)
{
  if (title_format)
    sprintf (ntwin, title_format, TaskNumber, status);
  else
    sprintf (ntwin, "%s - TASK %i", PRDCT_PRTY " " PRDCT_VRSN, TaskNumber);

  SetConsoleTitle (ntwin);
  // Bring the active window to top, useful when other program
  // has been started - HJK 98/06/13
  BringWindowToTop (GetForegroundWindow ());
}
#endif

/* $Id: b_initva.c,v 1.12 1999/09/27 20:51:19 mr Exp $ */
