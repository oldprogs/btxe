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
 * Filename    : $Source: E:/cvs/btxe/src/bt.c,v $
 * Revision    : $Revision: 1.16 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/10/03 13:46:47 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm "Main" Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void print_exeinfo (void);

#ifdef OS_2
#ifdef EXCEPTIONS

ULONG APIENTRY exHandler (PEXCEPTIONREPORTRECORD,
                          PEXCEPTIONREGISTRATIONRECORD,
                          PCONTEXTRECORD,
                          PVOID);

#endif
#endif

#ifndef NO_SIGNALS

SIGTYPE handler (SIGARGS);

#ifdef SIGTERM

SIGTYPE handler_sigterm (SIGARGS);

#endif

#endif

extern char compiled[];

static char far *DOS_commands[] =
{
  "noforce",
  "mail",
  "share",
  "dynam",
  "unattended",
  "config",
  "poll",
  "debug",
  "term",                       /* MR 970307 */
  "notreleasedyet",             /* TJW 970531 */
  "info",                       /* MR 980503 - display additional infos */
  NULL
};


/* MMP 960419 main() is now another function implementing the Respawn command
 * line keyword
 **/

static int
binkmain (int argc, char *argv[])
{
  int argc2 = argc;
  char **argv2 = argv;
  int done;
  char *envptr;
  ADDR baddr;
  long init_timer;
  char *poll_addr = NULL;
  char junk[256];
  int additional_info = 0;


  // CEH991001: read Config from command-line before reading the
  // language-file, to be able to read it from the same path. 

#ifdef USE_FIDOCONFIG
  // but first: Create the path for config_name (and eventfile)
  // from fidoconfig
  setpathsfromfidoconfig ();
#endif

  while (--argc2)
  {
    ++argv2;
    switch (parse (argv2[0], DOS_commands))
    {
    case 6:                    /* Config     */
      config_name = argv2[1];
      --argc2;
      ++argv2;
      break;

    case 7:                    /* Poll       */
      --argc2;                  // just skip

      ++argv2;
      break;
    }
  }

  load_language (*argv);

  /* MMP 960419 signal handling was here */

  init_timer = timerset (2 * PER_SECOND);

  b_initvars ();

  while (--argc)
  {
    ++argv;

    switch (parse (argv[0], DOS_commands))
    {
    case 1:                    /* Noforce    */
      noforce = 1;
      break;

    case 2:                    /* Mail       */
      mail_only = 1;
      break;

    case 3:                    /* Share      */
      share = 1;
      break;

    case 4:                    /* Dynam      */
      redo_dynam = 1;
      break;

    case 5:                    /* Unattended */
      un_attended = 1;
      command_line_un = 1;
      break;

    case 6:                    /* Config     */
      // CEH: moved before load_language
      // config_name = argv[1];
      --argc;
      ++argv;
      break;

    case 7:                    /* Poll       */
      poll_addr = argv[1];
      --argc;
      ++argv;
      break;

    case 8:                    /* Debug      */
      debugging_log = 1;
      break;

    case 9:                    /* MR 970307 Term */
      command_line_term = 1;
      break;

    case 10:                   /* MR 990302 removed, no longer needed */
      break;

    case 11:                   /* MR 980503 Info */
      additional_info = 1;
      break;

    default:                   /* Not there  */
      if (strchr (*argv, '='))
      {
        /* MMP 960420 Set environment variable */
        char *c;

        for (c = *argv; *c != '='; c++)
          *c = (char) toupper (*c);
        putenv (*argv);
      }
      else
      {
        printf (MSG_TXT (M_UNRECOGNIZED_OPTION), argv[0]);
        exit (1);
      }
    }
  }

  /* MR 970714 set OS environment variable */

#if defined OS_2
  putenv ("OS=OS2");
#elif defined _WIN32
  putenv ("OS=W32");
#elif defined __unix__
  putenv ("OS=UNIX");
#else
  putenv ("OS=DOS");
#endif

  /* MR 970715 set BTXEVER environment variable */

  sprintf (junk, "BTXEVER=%s", version_base);
  putenv (strdup (junk));

  /* TJW 960421 changed version string generation vvv */

  sprintf (junk, "%s %s/%s-%s%s%s %s",
           PRD_SH_PRTY, PRDCT_VRSN,
           version_type, version_base, version_modified, version_opts,
           compilerinfo);
  ANNOUNCE = strdup (junk);

  /* TJW 960421 changed version string generation ^^^ */

  /* MR 991003 removed developer names as there are too many... */
  printf ("\n /----------------------------------------------------------------------------\\\n");
  printf (" | %-74s |\n", ANNOUNCE);
  printf (" | (C) Copyright 1987-96, Bit Bucket Software, Co. ALL RIGHTS RESERVED.       |\n");
  printf (" +----------------------------------------------------------------------------+\n");
  printf (" | Note: This version contains modifications done by the BinkleyTerm XE Team  |\n");
  printf (" |       and contributers (see included team.lst).                            |\n");
  printf (" \\----------------------------------------------------------------------------/\n");
  printf ("  \\--       feel free to visit our homepage at http://btxe.uue.org/        --/\n\n");

  if (additional_info)
    print_exeinfo ();

  printf ("%s", MSG_TXT (M_SYSTEM_INITIALIZING));

  /* MR 961121 if the environment variable BINKLEY is set then this is
   *           binkley's home directory (BINKpath).
   */

  envptr = getenv (PRDCT_PRFX); /* get path from environment */

  if (envptr != NULL)
  {
    BINKpath = calloc (1, (strlen (envptr) + 2));  /* make room for new   */
    strcpy (BINKpath, envptr);  /* use BINKLEY as our path */
    add_backslash (BINKpath);
  }
  else
  {
    BINKpath = calloc (1, PATHLEN + 1);  /* make room for new   */

    getcwd (BINKpath, PATHLEN - 1);  /* get current directory   */
    add_backslash (BINKpath);
  }

  task_dir = BINKpath;          /* TJW 960830 */

  memset (&hist, 0x00, sizeof (HISTORY));

  envptr = getenv ("TASK");     /* TJW 960422 get TaskNumber from environment */

  if (envptr)
    TaskNumber = atoi (envptr);
  else
  {
    printf (MSG_TXT (M_NOENVVARTASK));
    exit (1);
  }

#ifdef USE_FIDOCONFIG
  if (read_fidoconfig ())
  {
    printf ("\nNot enough memory or error in fidoconfig\n\n");
    exit (1);
  }
  else
  {
    printf (". done\n\n");
  }
#endif

  parse_config (config_name);   /* TJW 960830 changed parse order */

  if (!read_sched ())
    parse_config (eventfile);   /* r. hoerner */

  if (num_events == 0)
  {
    printf ("ERR: No Events defined. Please define at least one event 00:00 - 24:00.\n");
    exit (1);
  }

  scdcompiletime = unix_time (NULL);  // TJW 971031 remember compile time

  openlogs (1);                 /* TJW 960527 open the log files, print \n into main log */
  b_defaultvars ();

  /* We have everything we need to handle addresses now.
   * Parse the 'poll' address if there was one. */

  if (poll_addr != NULL && find_address (poll_addr, &baddr))
  {
    doing_poll = 1;
  }

  /* We expect this to fail right away, but in order to keep the copyright
   * notice displayed long enough, we check anyway */

  while (!timeup (init_timer))
    time_release ();

  done = 0;

  b_sbinit ();

  if (mail_only)
  {
    doing_poll = 1;
    baddr = boss_addr;
  }

  start_hist = hist;

  if (doing_poll)
  {
    if (baddr.Zone == 0)
      baddr.Zone = alias->Zone;
    un_attended = 1;
    opening_banner ();
    mailer_banner ();
    sprintf (junk, "%s", Full_Addr_Str (&baddr));
    junk[28] = '\0';

    sb_fill (holdwin, ' ', colors.hold);
    sb_move_puts (holdwin, 1, 11, MSG_TXT (M_POLLING_COLON));
    sb_move_puts (holdwin, 3, (short) (15 - strlen (junk) / 2), junk);
    sb_show ();

    find_event ();
    set_prior (PRIO_MODEM);
    if (!CARRIER)
      modem_hangup ();
    do_mail (&baddr, 1);
    set_prior (PRIO_REGULAR);
    done = 1;
  }

  while (!done)
  {
    if (un_attended)
    {
      done = unattended ();
      un_attended = 0;
    }
    else
    {
      done = b_term ();
      un_attended = done ? 0 : 1;
    }
  }

  b_sbexit ();
  b_exitproc ();

#if defined(OS_2) && !defined(__EMX__)
  if (UseCAPICID)               /* CFS 970310 - Unlink from CAPI on exit */
    CAPIStop ();
#endif

  return (1);
}


/* MMP 960423 signal(SIGBREAK,SIG_IGN) seems to be broken using Watcom */

#ifndef NO_SIGNALS

#ifdef __WATCOMC__

void
handler (int signo)
{
  switch (signo)
  {
  case SIGINT:
    signal (SIGINT, handler);
    ctrlc_ctr++;
    break;
  case SIGBREAK:
    signal (SIGBREAK, handler);
    break;
  }
}

#else /* __WATCOMC__ */

SIGTYPE
handler (SIGARGS)               /* Function called at system interrupt */
{
#ifdef SIGNAL_ARGUMENT
  happy_compiler = (int) sigarg;  /* For the compiler */
#endif

  signal (SIGINT, SIG_IGN);     /* Disallow CTRL + C during handler    */
  ctrlc_ctr++;
  signal (SIGINT, handler);

  /* This is necessary so that the next CTRL+C interrupt will call
   * "handler", since the DOS 3.x operating system resets the
   * interrupt handler to the system default after the
   * user-defined handler is called */
}

#ifdef SIGTERM
SIGTYPE
handler_sigterm (SIGARGS)       /* Function called at system interrupt */
{
#ifdef SIGNAL_ARGUMENT
  happy_compiler = (int) sigarg;  /* For the compiler */
#endif
  ctrlc_ctr++;
  signal (SIGTERM, handler);
}
#endif

#endif /* __WATCOMC__ */

#endif /* NO_SIGNALS */


/* MMP 960419 This new main contains the implementation of the Respawn
 * command line keyword. On OS/2, if this keyword is used, Binkley is
 * respawned if it exits irregullarly. I hate to put strings in here
 * that really belongs to the language file, but it would be overkill
 * to load the language file twice just for those few string.. Also, I
 * would like to have a respawn keyword in the configuration file, but
 * this requires the first Binkley to do a lot of initialization (and
 * memory allocation).
 */

#ifndef exit
#error exit undefined!?
#endif
#undef exit

#define BINKEXIT "_BINKLEY_EXIT_"

MAINTYPE
main (int argc, char *argv[])
{
#ifdef OS_2
#ifdef EXCEPTIONS

  EXCEPTIONREGISTRATIONRECORD xcpthand;

#endif
#endif

  char flagbuf[40], *flagname;
  char **av;
  int errlev, rc;
  int i, j;

  /* Disable Ctrl-C/Break */

#ifndef NO_SIGNALS

#ifdef __WATCOMC__
  signal (SIGINT, handler);
  signal (SIGBREAK, handler);
#else

  if (signal (SIGINT, handler) == SIG_ERR)
    fprintf (stderr, MSG_TXT (M_NO_SIGINT));
#ifdef SIGTERM
  if (signal (SIGTERM, handler_sigterm) == SIG_ERR)
    fprintf (stderr, MSG_TXT (M_NO_SIGINT));
#endif

#ifdef SIGBREAK
  signal (SIGBREAK, SIG_IGN);
#endif

#endif /* __WATCOMC__ */

#endif /* NO_SIGNALS */

// ===================================================================

#ifdef OS_2
#ifdef EXCEPTIONS
  xcpthand.prev_structure = 0;
  xcpthand.ExceptionHandler = &exHandler;
  DosError (FERR_DISABLEEXCEPTION | FERR_DISABLEHARDERR);
  DosSetExceptionHandler (&xcpthand);
#endif
#endif

  // ===================================================================

#ifdef OS_2
  DosSetProcessCp (437);        // MR 970113 set codepage to 437 (US-English)
#endif

#ifdef _WIN32
  SetConsoleCP (437);           //HJK 980802 set codepage to 437 (US-English)

  //NS971214  switch off mouse cursor
  SetConsoleMode (GetStdHandle (STD_INPUT_HANDLE), ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
  FlushConsoleInputBuffer (GetStdHandle (STD_INPUT_HANDLE));
#endif

  /* Check if "Respawn" is specified. If not, act normally */

  for (i = 1; i < argc; i++)
    if (!stricmp (argv[i], "respawn"))
      break;

  if (i == argc)
    binkexit (binkmain (argc, argv));

  /* Set BINKEXIT filename */

  strcpy (flagbuf, BINKEXIT);
  flagname = flagbuf + strlen (flagbuf);
  *flagname++ = '=';
  sprintf (flagname, "bink%04x.xit", getpid () & 0xFFFF);
  putenv (strdup (flagbuf));

  /* Set up argument vector (argv[] may not be NULL terminated) */

  if ((av = malloc ((argc) * sizeof (char *))) == NULL)
      exit (3);

  for (i = j = 0; i < argc; i++)
    if (stricmp (argv[i], "respawn"))
      av[j++] = argv[i];
  av[j] = NULL;

  /* Keep spawning Binkley until we succesfully can unlink the flag file */

  do
  {
    errlev = spawnv (P_WAIT, av[0], av);
    if (errlev == -1)
    {
      printf ("Error spawning %s: %s\n", av[0], strerror (errno));
      exit (1);
    }
    rc = unlink (flagname);
    if (rc)
      printf ("\n\nBinkley died (errorlevel %d) - respawning!\a\n", errlev);
  }
  while (rc);

  /* Now, do final exit */

#ifdef  OS_2                    /*  TS 970403   */
  SendToDlg ((PSZ) "---*---", 0, 0, BINK_END);
#endif

// ===================================================================
#ifdef OS_2
#ifdef EXCEPTIONS
  DosUnsetExceptionHandler (&xcpthand);
#endif
#endif
// ===================================================================

#ifdef INTMAIN                  /* MMP 960419 main() is still declared void for some compilers */
  return (errlev);              /* CE/TJW 960418 makes compiler happy, was: exit (1); */
#else
  exit (errlev);
#endif
}

void
binkexit (int n)
{
  char *c;

  movelogs ();
  if ((c = getenv (BINKEXIT)) != NULL)
    if (fopen (c, write_binary) == NULL)
      printf ("\nError opening %s. This may cause Binkley to be respawned!\n", c);

#ifdef  OS_2                    /*  TS 970403   */
  SendToDlg ((PSZ) "---*---", 0, 0, BINK_END);
#endif
  exit (n);
}

/* $Id: bt.c,v 1.16 1999/10/03 13:46:47 mr Exp $ */
