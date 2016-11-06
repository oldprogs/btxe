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
 * Filename    : $Source: E:/cvs/btxe/src/btutil.c,v $
 * Revision    : $Revision: 1.11 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:40 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello, Thomas Waldmann
 *
 * Description : BinkleyTerm Utility
 *
 * Note        :
 *   This tool unites the following tools in one source / EXE:
 *    - BinkleyTerm OMMM Control File Generator
 *    - BinkleyTerm Language File Compiler
 *    - BinkleyTerm History Importers
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"
#undef exit

/* AW 980111: history importer main function prototypes */
int gethist_bgfax_main (int, char **);

#ifndef __unix__

int gethist_max3_main (int, char **);

#endif

int gethist_ra_main (int, char **);
int gethist_telegard_main (int, char **);

static int usage (void);

/* ============ begin of stuff taken from BTCTL.C ========================= */

/* ------------------------------------------------------------------------ */
/* The following is excerpted from the control structures used by Opus 1.10 */
/* as it is currently implemented. The only part that really concerns BTCTL */
/* is that part of the _PRM structure that contains the version number and  */
/* the network address. Only those parts are actually handled by this code. */
/* We suspect that no changes will be made in that part of the structure    */
/* between now and the release of Opus 1.10. If I were you, I would make    */
/* no such assumptions about the rest.                                      */
/* ------------------------------------------------------------------------ */

#define  THIS_CTL_VERSION  16   /* PRM structure version number         */

#define  CTLSIZE            1
#define  OFS            char*

#define  MAX_EXTERN         8   /* max. number of external programs     */
#define  MAXCLASS          12   /* number of possible priv levels       */
#define  PRM16_ALIAS_CNT   15   /* number of matrix addresses           */

/* ------------------------------------------------------------------------ */
/* Attributes of a given class of users                                     */
/* ------------------------------------------------------------------------ */
struct class_rec
{
  byte ClassPriv;
  byte class_fill;
  short max_time;               /* max cume time per day         */
  short max_call;               /* max time for one call         */
  short max_dl;                 /* max dl bytes per day          */
  word ratio;                   /* ul:dl ratio                   */
  word min_baud;                /* speed needed for logon        */
  word min_file_baud;           /* speed needed for file xfer    */
};

/* ------------------------------------------------------------------------ */
/* Registers to pass to a FOSSIL appendage                                  */
/* ------------------------------------------------------------------------ */
struct _FOSREGS
{
  word ax;
  word bx;
  word cx;
  word dx;
};

/* ------------------------------------------------------------------------ */
/* The format of the PRM file, VERSION 16                                   */
/*                                                                          */
/* THIS IS AN EXPLOSIVE STRUCTURE.  IT IS SUBJECT TO CHANGE WITH NO NOTICE. */
/*                                                                          */
/* Offsets to the following item(s) are guaranteed:                         */
/*                                                                          */
/*      byte   version;        // OFFSET 0, all versions                    */
/*      byte   task_num;       // OFFSET 1, 16+                             */
/*                                                                          */
/* ------------------------------------------------------------------------ */

struct _PRM
{
  /* --------------------------------------------------------- */
  /* DATA                                                      */
  /* --------------------------------------------------------- */
  byte version;                 /* for safety                          STABLE */
  byte task_num;                /* for multi-tasking systems           STABLE */
  ADDR alias[PRM16_ALIAS_CNT];

  byte video;                   /* 0=Dos, 1=Fossil 2=IBM                     */
  byte testmode;                /* input from keyboard, not modem            */

  word carrier_mask;
  word handshake_mask;
  word max_baud;                /* fastest speed we can use                  */
  word com_port;                /* Com1=0, Com2=1, FF=keyboard               */

  byte multitasker;             /* flag for DoubleDos (see below)            */
  byte mailer_type;             /* 0=Opus, 1=load external, 2=call external  */

  byte ModemFlag;               /* (See MODEM FLAG below)                    */
  byte LogFlag;                 /* (See LOG FLAG below)                      */

  byte StyleFlag;               /* (See STYLE FLAG below)                    */
  byte FWDflag;                 /* Bits to control IN TRANSIT messages       */

  byte Flags;                   /* See "FLAGS" below                         */
  byte Flags2;                  /* See "FLAGS 2" below                       */

  byte edit_exit;               /* ERRORLEVEL to use if Matrix area changed  */
  byte exit_val;                /* ERRORLEVEL to use after caller            */

  byte crashexit;               /* non-zero= ErrorLevel exit                 */
  byte arc_exit;                /* ErrorLevel for after incomming ARCmail    */

  byte echo_exit;               /* ERRORLEVEL for after inbound echomail     */
  byte UDB_Flags;               /* User data base flags                      */

  word min_baud;                /* minimum baud to get on-line               */
  word color_baud;              /* min baud for graphics                     */
  word date_style;              /* Used for FILES.BBS display                */

  byte logon_priv;              /* Access level for new users                */
  byte seenby_priv;             /* Min priv to see SEEN_BY line              */

  byte ctla_priv;               /* Priv to see CONTROL-A lines in messages   */
  byte FromFilePriv;            /* Priv. for doing message from file         */

  byte AskPrivs[16];            /* Array of privs. for message attr ask's    */
  byte AssumePrivs[16];         /* Array of privs. for message attr assume's */

  word logon_time;              /* time to give for logons                   */

  word matrix_mask;

  word MinNetBaud;              /* minimum baud rate for remote netmail      */
  word MaxJanusBaud;            /* fastest baud to use Ianus                 */

  struct class_rec class[MAXCLASS];
  struct _FOSREGS FosRegs[10];

  word F_Reward;                /* File upload time reward percentage        */

  word last_area;               /* Highest msg area presumed to exist        */
  word last_farea;              /* Highest file area presumed to exist       */

  word PRM_FILL3[17];

  /* --------------------------------------------------------- */
  /* OFFSETS                                                   */
  /* --------------------------------------------------------- */

  /* ----------------------------------------- */
  /* MODEM COMMAND STRINGS                     */
  /* ----------------------------------------- */
  OFS MDM_Init;                 /* modem initialization string               */
  OFS MDM_PreDial;              /* modem dial command sent before number     */
  OFS MDM_PostDial;             /* modem command sent after dialed number    */
  OFS MDM_LookBusy;             /* mdm cmd to take modem off hook            */

  /* ----------------------------------------- */
  /* PRIMROSE PATHS                            */
  /* ----------------------------------------- */
  OFS misc_path;                /* path to BBS/GBS files                     */
  OFS sys_path;                 /* path to SYSTEM?.BBS files                 */
  OFS temppath;                 /* place to put temporary files              */
  OFS net_info;                 /* path to NODELIST files                    */
  OFS mailpath;                 /* place to put received netmail bundles     */
  OFS filepath;                 /* place to put received netmail files       */
  OFS hold_area;                /* path to pending outbound matrix traffic   */

  /* ----------------------------------------- */
  /* DATA FILE NAMES                           */
  /* ----------------------------------------- */
  OFS user_file;                /* path/filename of User.Bbs                 */
  OFS sched_name;               /* name of file with _sched array            */
  OFS syl;                      /* default system language file              */
  OFS usl;                      /* default user language file                */

  /* ----------------------------------------- */
  /* MISCELLANEOUS TEXT                        */
  /* ----------------------------------------- */
  OFS system_name;              /* board's name                              */
  OFS sysop;                    /* sysop's name                              */
  OFS timeformat;
  OFS dateformat;
  OFS protocols[MAX_EXTERN];    /* external file protocol programs    */

  /* ----------------------------------------- */
  /* BBS/GBS SUPPORT FILES                     */
  /* ----------------------------------------- */
  OFS logo;                     /* first file shown to a caller              */
  OFS welcome;                  /* shown after logon                         */
  OFS newuser1;
  OFS newuser2;
  OFS rookie;

  OFS HLP_Editor;               /* Intro to msg editor for novices.          */
  OFS HLP_Replace;              /* Explain the Msg.Editor E)dit command      */
  OFS HLP_Inquire;              /* Explain the Msg. I)nquire command         */
  OFS HLP_Locate;               /* Explain the Files L)ocate command         */
  OFS HLP_Contents;             /* Explain the Files C)ontents command       */
  OFS HLP_OPed;                 /* help file for the full-screen editor      */
  OFS OUT_Leaving;              /* Bon Voyage                                */
  OFS OUT_Return;               /* Welcome back from O)utside                */
  OFS ERR_DayLimit;             /* Sorry, you've been on too long...         */
  OFS ERR_TimeWarn;             /* warning about forced hangup               */
  OFS ERR_TooSlow;              /* explains minimum logon baud rate          */
  OFS ERR_XferBaud;             /* explains minimum file transfer baud rate  */
  OFS LIST_MsgAreas;            /* dump file... used instead of Dir.Bbs      */
  OFS LIST_FileAreas;           /* dump file... used instead of Dir.Bbs      */

  OFS FREQ_MyFiles;             /* file to send when FILES is file requested */
  OFS FREQ_OKList;              /* list of files approved for file requests  */
  OFS FREQ_About;               /* File Request: ABOUT file                  */

  OFS OEC_Quotes;
  OFS byebye;                   /* file displayed at logoff                  */
  OFS local_editor;             /* text editor to use in keyboard mode       */
  OFS barricade;
  OFS Files_BBS;                /* FILES.BBS filename override for CD ROM    */
  OFS mailer;                   /* full external mailer command              */
  OFS common;                   /* File with data common to all tasks        */

  OFS OFS_Filler[13];

  /* --------------------------------------------------------- */
  /* Log_Name must always be the last offset in this struct    */
  /* because Bbs_Init uses that symbol to flag the end of      */
  /* the offsets.                                              */
  /* --------------------------------------------------------- */
  OFS log_name;                 /* name of the log file                      */

  /* --------------------------------------------------------- */
  /* Big blob of stuff                                         */
  /* It's a sequence of null-terminated character arrays...    */
  /* pointed-to by the offsets (above).                        */
  /* --------------------------------------------------------- */
  char buf[CTLSIZE];
};

/* Stuff used later on here */

void errxit (char *);
void btu_parse_config (char *);
void b_initvars (void);
void b_defaultvars (void);

char *config_name = "binkley.cfg";
char *eventfile = "binkley.evt";
char *BINKpath = NULL;
int net_params = 0;
short Zone = 1;
int TaskNumber = 0;

/* ------------------------------------------------------------------------ */
/* BTCTL                                                                    */
/* Parse the BINKLEY.CFG file and write out a BINKLEY.PRM file (for use by  */
/* OMMM) and a MAIL.SYS file (for use by FASTTOSS, SCANMAIL, SIRIUS, etc).  */
/* If the environment variable BINKLEY exists use the path specified for    */
/* ALL input and output files.                                              */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* MAIL.SYS file structure                                                  */
/* ------------------------------------------------------------------------ */
struct _mail
{
  short pri_node;               /* Our node number                          */
  long fudge;                   /* Unknown/unused                           */
  short rate;                   /* Maximum baud rate                        */
  char mailpath[80];            /* Path for incomming messages              */
  char filepath[80];            /* Path for incomming files                 */
  short pri_net;                /* Our network number                       */
  short alt_node;               /* Alternate node number (mainly for HOSTS) */
  short alt_net;                /* Alternate net number (mainly for HOSTS)  */
};

struct _mail mailsys;           /* MAIL.SYS used by SIRIUS  */
struct _PRM ctl;                /* where the .CTL stuff is  */

int num_addrs = 0;

int
ctl_main (void)
{
  char *envptr;
  FILE *stream;
  char temp[PATHLEN];
  struct stat statbuf;
  int k;

  b_initvars ();

  envptr = getenv ("BINKLEY");  /* get path from environment */
  if ((envptr != NULL)          /* If there was one, and     */
      && (stat (config_name, &statbuf) != 0))  /* No BINKLEY.CFG locally,   */
  {
    BINKpath = malloc (strlen (envptr) + 2);  /* make room for new */
    strcpy (BINKpath, envptr);  /* use BINKLEY as our path   */
    add_backslash (BINKpath);
  }

  envptr = getenv ("TASK");     /* r. hoerner: added "task" */
  if (envptr)
    TaskNumber = atoi (envptr);

  btu_parse_config (config_name);
  btu_parse_config (eventfile); /* r. hoerner */

  b_defaultvars ();

  /*
   * Print out what we got.
   */

  if (ctl.system_name != NULL)
    printf ("System: %s\n", ctl.system_name);

  if (ctl.sysop != NULL)
    printf ("Sysop:  %s\n", ctl.sysop);

  for (k = 0; k < PRM16_ALIAS_CNT; k++)  /* And the alias list        */
  {
    if (!ctl.alias[k].Zone)
      break;
    printf ("Address %hu:%hu/%hu.%hu\n",
            ctl.alias[k].Zone,
            ctl.alias[k].Net,
            ctl.alias[k].Node,
            ctl.alias[k].Point);
  }

  if (ctl.mailpath != NULL)
    printf ("Net Mailpath %s\n", ctl.mailpath);

  if (ctl.filepath != NULL)
    printf ("Net Filepath %s\n", ctl.filepath);

  printf ("\n");

  if (!net_params)
    errxit ("Not enough information to establish Netmail session");

  /*
   * Okay, we have the nodelist data from the BINKLEY.CFG file.
   * Now write it into a BINKLEY.PRM file.
   */

  if (BINKpath != NULL)         /* If there was a BINKLEY,   */
  {
    strcpy (temp, BINKpath);    /* use it here too           */
  }
  else
    temp[0] = '\0';
  strcat (temp, "binkley.prm"); /* add in the file name      */

  if ((stream = fopen (temp, write_binary)) == NULL)  /* OK, let's open the file   */
    errxit ("Unable to open BINKLEY.PRM");
  if (fwrite (&ctl, sizeof (ctl), 1, stream) != 1)  /* Try to write data out  */
    errxit ("Could not write control file data to BINKLEY.PRM");
  fclose (stream);              /* close output file         */
  printf ("Version 16 Control file successfully written\n");
  printf ("oMMM 1.30 or above is required to use it\n\n");

  /*
   * BINKLEY.PRM now written. Let's write a MAIL.SYS file too.
   */

  mailsys.pri_node = ctl.alias[0].Node;
  mailsys.pri_net = ctl.alias[0].Net;
  mailsys.alt_node = ctl.alias[1].Node;
  mailsys.alt_net = ctl.alias[1].Net;
  strcpy (mailsys.mailpath, ctl.mailpath);
  strcpy (mailsys.filepath, ctl.filepath);

  if (BINKpath != NULL)         /* If there was a BINKLEY,   */
    strcpy (temp, BINKpath);    /* use it here too           */
  else
    temp[0] = '\0';

  strcat (temp, "mail.sys");    /* add in the file name      */

  if ((stream = fopen (temp, write_binary)) == NULL)  /* OK, let's open the file   */
    errxit ("Could not open MAIL.SYS");  /* no file, no work to do    */

  if (fwrite (&mailsys, sizeof (mailsys), 1, stream) != 1)
    errxit ("Unable to write data to MAIL.SYS");  /* Try to write data out  */

  fclose (stream);              /* close output file         */
  printf ("MAIL.SYS file successfully written\n");  /* Notify user of success */
  return (0);
}

/**
 ** b_initvars -- called before btu_parse_config. Sets defaults that we want
 ** to have set FIRST.
 **/

void
b_initvars (void)
{
  int k;

  ctl.version = 16;
  for (k = 0; k < PRM16_ALIAS_CNT; k++)  /* And the alias list        */
  {
    ctl.alias[k].Zone = ctl.alias[k].Net =
      ctl.alias[k].Node = ctl.alias[k].Point = 0;
  }

  ctl.alias[0].Zone = 1;        /* Make sure we have a zone  */
  ctl.alias[0].Net = ctl.alias[0].Node = (word) - 1;  /* Default Fidonet address   */
  ctl.alias[0].Point = 0;

  ctl.system_name = ctl.sysop =
    ctl.hold_area = ctl.mailpath = ctl.filepath = NULL;
}

/**
 ** b_defaultvars -- called after all btu_parse_config passes complete.
 ** sets anything not handled by btu_parse_config to default if we know it.
 **/

void
b_defaultvars (void)
{
  /* Set up "point" address correctly if we can */

  if ((pvtnet > 0) && ctl.alias[0].Point)  /* TJW 960701 bugfix for no pvtnet */
  {
    ctl.alias[0].Net = pvtnet;
    ctl.alias[0].Node = ctl.alias[0].Point;
    ctl.alias[0].Point = 0;
  }

  /* If we have the minimum information to do netmail, set the flag */

  if ((ctl.alias[0].Zone != 0)
      && (ctl.alias[0].Net != 0)
      && (ctl.system_name != NULL)
      && (ctl.sysop != NULL)
      && (ctl.hold_area != NULL)
      && (ctl.filepath != NULL)
      && (ctl.mailpath != NULL))
    net_params = 1;
}

char far *config_lines[] =
{
  "Zone",
  "System",
  "Sysop",
  "Boss",
  "Point",
  "Aka",
  "Address",
  "Hold",
  "NetFile",
  "NetMail",
  "Include",
  "PrivateNet",
  "Inbound",
  "Outbound",
  NULL
};

void
btu_parse_config (char *config_file)
{
  FILE *stream;
  char temp[PATHLEN];
  char *c;
  int i;
  short boss_net = 0;
  short boss_node = 0;

  if (BINKpath != NULL)
    sprintf (temp, "%s%s", BINKpath, config_file);
  else
    strcpy (temp, config_file);

  if ((stream = fopen (temp, read_ascii)) == NULL)  /* OK, let's open the file   */
    return;                     /* no file, no work to do    */

  while ((fgets (temp, PATHLEN - 1, stream)) != NULL)  /* Now we parse the file ... */
  {
    c = temp;                   /* Check out the first char  */
    if ((*c == '%') || (*c == ';'))  /* See if it's a comment
                                        * line */
      continue;

    i = strlen (temp);          /* how long this line is     */

    if (i < 3)
      continue;                 /* If too short, ignore it   */

    c = &temp[--i];             /* point at last character   */
    if (*c == '\n')             /* if it's a newline,        */
      *c = '\0';                /* strip it off              */

    switch (parse (temp, config_lines))
    {
    case 1:                    /* "Zone"         */
      c = skip_blanks (&temp[4]);
      Zone = atoi (c);
      if (!Zone)                /* if we didn't find a zone  */
        printf ("Illegal zone: %s\n", &temp[4]);
      break;

    case 2:                    /* "System"       */
      ctl.system_name = ctl_string (&temp[6]);
      break;

    case 3:                    /* "Sysop"        */
      ctl.sysop = ctl_string (&temp[5]);
      break;

    case 4:                    /* "Boss"         */
      c = skip_blanks (&temp[4]);
      if (sscanf (c, "%hd/%hd", &boss_net, &boss_node) == 2)
      {                         /* TJW 960701 */
        if (boss_net)
          pvtnet = boss_net;
      }
      break;

    case 5:                    /* "Point"        */
      i = 5;
      goto address;

    case 6:                    /* "Aka"          */
      i = 3;
      goto address;

    case 7:                    /* "Address"      */
      i = 7;

    address:
      if (num_addrs < PRM16_ALIAS_CNT)
      {                         /* TJW 960701 */
        ctl.alias[num_addrs].Point = 0;
        c = skip_blanks (&temp[i]);
        i = sscanf (c, "%hd:%hd/%hd.%hd",
                    &ctl.alias[num_addrs].Zone,
                    &ctl.alias[num_addrs].Net,
                    &ctl.alias[num_addrs].Node,
                    &ctl.alias[num_addrs].Point);
        if (i < 3)
        {
          i = sscanf (c, "%hd/%hd.%hd",
                      &ctl.alias[num_addrs].Net,
                      &ctl.alias[num_addrs].Node,
                      &ctl.alias[num_addrs].Point);
          if (i < 2)
            break;
          ctl.alias[num_addrs].Zone = Zone;
        }
        Zone = ctl.alias[0].Zone;  /* First is real default */
        ++num_addrs;
      }
      break;

    case 8:                    /* "Hold"         */
      ctl.hold_area = ctl_slash_string (0, &temp[8]);
      break;

    case 14:                   /* "Outbound"     */
      ctl.hold_area = ctl_slash_string (0, &temp[4]);
      break;

    case 9:                    /* "NetFile"      */
    case 13:                   /* "Inbound"      */
      ctl.filepath = ctl_slash_string (0, &temp[7]);
      break;

    case 10:                   /* "NetMail"      */
      ctl.mailpath = ctl_slash_string (0, &temp[7]);
      break;

    case 11:                   /* "Include"      */
      c = skip_blanks (&temp[7]);
      btu_parse_config (c);
      break;

    case 12:                   /* "PrivateNet"   */
      c = skip_blanks (&temp[10]);
      pvtnet = atoi (c);
      break;

    default:
      break;
    }
  }

  fclose (stream);              /* close input file          */
}

void
errxit (char *error)
{
  printf ("\r\n%s\n", error);
  exit (0);
}

/* ============ end of stuff taken from BTCTL.C =========================== */

/* ============ begin of stuff taken from BTLNG.C ========================= */

#include "language.h"

#include "lngmagic.h"

/*
 * Assume average length of a string at 32 characters
 *
 */

char **pointers;
short pointer_size;

struct _lang_hdr LangHdr =
{
  0, 0
};

char *memory;
short memory_size;

struct _lang_hdr PrdctHdr =
{
  0, 0
};
char *PrdctMem;
char **PrdctTbl;
char *PrdctUnknown;

short *TrmnlAccelAry;
short TrmnlAccelCnt = 0;
struct _key_fnc *TrmnlAccelTbl;

short *UnattendedAccelAry;
short UnattendedAccelCnt = 0;
struct _key_fnc *UnattendedAccelTbl;

struct _lang_hdr AnsiHdr =
{
  0, 0
};
char *AnsiMem;

static void patchexe (char *exe, char *lng);

/*
 * Read in a raw text file and write out a compiled BinkleyTerm
 * language file.
 *
 */

int
lng_main (int argc, char **argv)
{
  char *malloc_target;
  char key;
  int error;
  int Idx;

  /* Make sure we were called with the required number of arguments
   * MMP 960420 Patch keyword */

  if (argc == 4 && !stricmp (argv[1], "patch"))
    patchexe (argv[2], argv[3]);

  /* TJW 960618 multiple language file support */
  key = '\0';
  if (argc == 5 && !stricmp (argv[1], "key"))
  {
    key = *argv[2];
    argv[1] = argv[3];
    argv[2] = argv[4];
    argc -= 2;
  }

  if (argc != 3)
    usage ();

  /* Allocate space for the raw character array and for the
   * pointer array */

  malloc_target = malloc (MAX_MEMORY);
  if (malloc_target == NULL)
  {
    fprintf (stderr, "Unable to allocate string memory\n");
    exit (250);
  }

  memory = malloc_target;
  memory_size = MAX_MEMORY;

  malloc_target = malloc ((MAX_STRINGS + 1) * (sizeof (char *)));

  if (malloc_target == NULL)
  {
    fprintf (stderr, "Unable to allocate pointer array\n");
    exit (250);
  }

  pointers = (char **) malloc_target;
  pointer_size = MAX_STRINGS;

  TrmnlAccelAry = calloc (MAX_KEYFNCS, sizeof (int));
  TrmnlAccelTbl = calloc (MAX_KEYFNCS, sizeof (struct _key_fnc));
  UnattendedAccelAry = calloc (MAX_KEYFNCS, sizeof (int));
  UnattendedAccelTbl = calloc (MAX_KEYFNCS, sizeof (struct _key_fnc));

  /* Allocate space for the raw character array and for the
   * pointer array */

  malloc_target = malloc (MAX_PRDCTS * 32);
  if (malloc_target == NULL)
  {
    fprintf (stderr, "Unable to allocate product string memory\n");
    exit (250);
  }

  PrdctMem = malloc_target;

  malloc_target = malloc ((MAX_PRDCTS + 1) * (sizeof (char *)));

  if (malloc_target == NULL)
  {
    fprintf (stderr, "Unable to allocate product pointer array\n");
    exit (250);
  }

  PrdctTbl = (char **) malloc_target;

  for (Idx = 0; MAX_PRDCTS > Idx; ++Idx)
    PrdctTbl[Idx] = PrdctMem;
  PrdctHdr.ElemCnt = MAX_PRDCTS;

  /* Allocate space for the ANSI array */

  malloc_target = malloc (MAX_ANSI * 10);
  if (malloc_target == NULL)
  {
    fprintf (stderr, "Unable to allocate product string memory\n");
    exit (250);
  }
  AnsiMem = malloc_target;

  /* Now read the stuff into our array. */

  error = get_language (argv[1], key);
  if (error != 0)
    exit (240);

  /* Write our stuff out now. */

  error = put_language (argv[2]);
  if (error != 0)
    exit (230);

  return (0);
}

static void
patchexe (char *exe, char *lng)
{
  FILE *in1, *in2, *out, *tmp;
  int ch;
  unsigned short size;
  long magic;
  int lngthere;
  long i, exelen;
  char *tmpfname = "$$BINK$$.TMP";

  if ((in1 = fopen (exe, read_binary)) == NULL)
  {
    fprintf (stderr, "Error opening %s: %s\n", exe, strerror (errno));
    exit (255);
  }

  if ((tmp = fopen (tmpfname, write_binary)) == NULL)
  {
    fprintf (stderr, "Error opening %s: %s\n", tmpfname, strerror (errno));
    exit (255);
  }

  lngthere = 1;
  if (fseek (in1, -4, SEEK_END) ||
      fread (&magic, 1, 4, in1) != 4 ||
      magic != LNGMAGIC)
  {
    lngthere = 0;
    fseek (in1, 0, SEEK_END);
  }

  if (lngthere)
  {
    if (fseek (in1, -6, SEEK_END) ||
        fread (&size, 1, 2, in1) != 2 ||
        fseek (in1, -(long) size - 6, SEEK_END))
    {
      lngthere = 0;
      fseek (in1, 0, SEEK_END);
    }
  }

  exelen = ftell (in1);
  fseek (in1, 0, SEEK_SET);

  for (i = 0; i < exelen; i++)
  {
    ch = fgetc (in1);
    fputc (ch, tmp);
  }

  fclose (in1);
  fclose (tmp);

  if ((tmp = fopen (tmpfname, read_binary)) == NULL)
  {
    fprintf (stderr, "Error opening %s: %s\n", tmpfname, strerror (errno));
    exit (255);
  }

  if ((in2 = fopen (lng, read_binary)) == NULL)
  {
    fprintf (stderr, "Error opening %s: %s\n", lng, strerror (errno));
    exit (255);
  }

  if ((out = fopen (exe, write_binary)) == NULL)
  {
    fprintf (stderr, "Error opening %s: %s\n", exe, strerror (errno));
    exit (255);
  }

  while ((ch = fgetc (tmp)) != EOF)
    fputc (ch, out);

  size = 0;
  while ((ch = fgetc (in2)) != EOF)
  {
    fputc (ch, out);
    size++;
  }

  fwrite (&size, 1, 2, out);
  magic = LNGMAGIC;
  fwrite (&magic, 1, 4, out);

  fclose (out);
  fclose (in2);
  fclose (tmp);
  unlink (tmpfname);

  exit (0);
}

/* ============ end of stuff taken from BTCTL.C =========================== */

/* ============ begin of stuff taken from GET_LANG.C ====================== */

#define DOS16

#ifdef __unix__
#define OS_IDENT "LNX"
#undef DOS16
#endif

#ifdef OS_2
#define OS_IDENT "OS2"
#undef DOS16
#endif

#ifdef _WIN32
#define OS_IDENT "W32"
#undef DOS16
#endif

#ifdef DOS16
#define OS_IDENT "DOS"
#endif

int
parse_escapes (char *string)
{
  char c;
  char *p, *q;
  int escape_on = 0;

  p = q = string;

  while ((c = *p++) != '\0')
  {
    switch (c)
    {
    case ';':
      if (escape_on)
      {
        *q++ = ';';
        --escape_on;
        break;
      }
      /* Otherwise drop into newline code */

    case '\n':
      *q = *p = '\0';
      break;

    case '\\':
      if (escape_on)
      {
        *q++ = '\\';
        --escape_on;
      }
      else
        ++escape_on;
      break;

    case 'n':
      if (escape_on)
      {
        *q++ = '\n';
        --escape_on;
      }
      else
        *q++ = c;
      break;

    case 'r':
      if (escape_on)
      {
        *q++ = '\r';
        --escape_on;
      }
      else
        *q++ = c;
      break;

    case 'b':
      if (escape_on)
      {
        *q++ = ' ';
        --escape_on;
      }
      else
        *q++ = c;
      break;

    case 'X':
    case 'x':
      if (escape_on)
      {
        *q++ = (char) strtol (p, &p, 16);
        --escape_on;
      }
      else
        *q++ = c;
      break;

    case '0':
      if (escape_on)
      {
        *q++ = (char) strtol (p, &p, 8);
        --escape_on;
      }
      else
        *q++ = c;
      break;

    case 's':
      if (escape_on)
      {
        *q++ = ' ';
        --escape_on;
      }
      else
        *q++ = c;
      break;

    default:
      *q++ = c;
      escape_on = 0;
      break;
    }
  }
  return (int) (q - string);
}

/*
 *  get_language - read in BinkleyTerm language source
 *
 * Read lines into table, ignoring blanks and comments
 * Store into a contiguous block of memory with the individual
 *     members being referenced by an array of pointers
 * Store number of lines read into pointer_size
 * Store amount of memory used into memory_size
 *
 */

int
get_language (char *name_of_file, char key)
{
  int len;                      /* length of current string  */
  int count_from_file;          /* no. of strings in file    */
  int file_version;             /* version of file           */
  char *p, *q;                  /* miscellaneous pointers    */
  char *storage;                /* where we're storing now   */
  char **load_pointers;         /* pointer to pointer array  */
  char linebuf[255];            /* biggest line we'll allow  */
  FILE *fpt;                    /* stream pointer            */
  int internal_count;           /* How many strings we got   */
  int total_size;               /* How big it all is         */
  int error;                    /* Internal error value      */
  int PrdctCode;                /* Product Code              */
  char *PrdctNm;
  char *PrdctPtr = PrdctMem;
  char *n;
  char *LangStart;
  unsigned int ansival;         /* Scanned ANSI keymap value */

  internal_count = 0;           /* zero out internal counter */
  count_from_file = 0;          /* zero out internal counter */
  total_size = 0;               /* Initialize storage size   */
  error = 0;                    /* Initialize error value    */

  load_pointers = pointers;     /* Start at the beginning    */
  storage = memory;             /* A very good place to start */

  /* Open the file now. Then read in the appropriate table. First line of
   * the file contains the number of lines we want Second line through end:
   * ignore if it starts with a ; and store only up to ; */

  fpt = fopen (name_of_file, read_ascii);  /* Open the file             */
  if (fpt == NULL)              /* Were we successful?       */
  {
    (void) fprintf (stderr, "Can not open input file %s\n", name_of_file);
    return (-1);                /* Return failure to caller  */
  }

  while (fgets (linebuf, 254, fpt) != NULL)  /* read a line in            */
  {
    p = q = linebuf;            /* set up for scan           */

    /*
     * This label is only hit when a ? line is seen.
     *
     * The format of a ? line is:
     *
     *        ?xxx ....
     *
     * where xxx is a 3-character platform identifier. For DOS systems,
     * the identifier is DOS and for OS/2 it is OS2. The text following
     * ?xxx is the same format as any other language file line.
     *
     * When we see a ?, we compare the following string to the ID of our
     * current platform. If it matches, we point p and q at the text following
     * the expression, and (I'm sorry) jump back. If it doesn't match, we
     * throw the line away.
     */

    if (*p == ';')              /* TJW 960618 Comment       */
      continue;

    /* TJW 960618 support for btlng key X ... : only read lines matching key */
    if (key)
    {                           /* if key is given, ignore   */
      if ((*p != key) && (*p != '*'))  /* lines not matching key or */
        continue;               /* global key "*"            */
      else
        p++;                    /* skip over key             */
    }

  re_cycle:

    switch (*p)
    {
    case ';':                  /* Comment */
    case 'C':                  /* Comment */
      break;

    case '?':
      if (strncmp (++p, OS_IDENT, 3) == 0)
      {
        q = p += 3;
        goto re_cycle;
      }
      break;

    case '#':                  /* msg number */
      q = p += 4;               /* simply skip it... */
      goto re_cycle;
      break;

    case 'L':                  /* Language Line             */
      LangStart = ++p;
      (void) parse_escapes (p);
      if ((len = strlen (p)) == 0)  /* Is anything there?        */
        continue;               /* No -- ignore.             */

      if (!count_from_file)
      {
        (void) sscanf (LangStart, "%d %d", &count_from_file, &file_version);
        if (count_from_file <= pointer_size)
          continue;

        (void) fprintf (stderr,
                        "Messages in file = %d, Pointer array size = %d\n",
                        count_from_file, pointer_size);
        error = -2;
        break;
      }

#ifdef __unix__

      // if the language-source is CRLF-terminated, we will zero the CR
      // instead of incrementing the length to get the string teminated.
      if (LangStart[len - 1] == 13)
        LangStart[len - 1] = 0;
      else
        ++len;                  /* Allow for the terminator  */

#else
      ++len;                    /* Allow for the terminator  */
#endif

      if (((total_size += len) < memory_size)  /* Make sure it fits */
          && (internal_count < pointer_size))
      {
        (void) memcpy (storage, LangStart, len);  /* Copy it   */
        *load_pointers++ = storage;  /* Point to start of string  */
        storage += len;         /* Move pointer into memory  */
      }

      ++internal_count;         /* bump count */
      break;

    case 'A':                  /* ANSI key output map       */
      (void) sscanf (++p, "%4x", &ansival);
      if (*(p += 4) != ' ')
        break;
      if (*++p == ' ')
        break;
      q = p;
      while (*q != '\0' && *q != ' ')  /* RDH 01.Jun.96 Remove Warnings */
        q++;
      *q = '\0';

      q = AnsiMem + AnsiHdr.PoolSize;
      len = parse_escapes (p);
      *((unsigned int *) q) = ansival;
      q += sizeof (unsigned int);

      *q++ = (char) len;
      strncpy (q, p, len);
      AnsiHdr.ElemCnt++;
      AnsiHdr.PoolSize = (int) ((q += len) - AnsiMem);
      break;

    case 'P':                  /* Product Code              */
      /* Format: nnn ProductName   */
      PrdctCode = (int) strtol (++p, &PrdctNm, 10);
      while (' ' == *PrdctNm)
        ++PrdctNm;
      n = PrdctNm + strlen (PrdctNm) - 1;
      while ((PrdctNm <= n) && ((*n == ' ') || (*n == '\n')))
        *n-- = '\0';

      if (PrdctCode == -1)
      {
        strcpy (PrdctMem, PrdctNm);
        PrdctPtr = PrdctMem + strlen (PrdctMem) + 1;
        PrdctHdr.PoolSize += strlen (PrdctNm) + 1;
      }
      else if ((0 <= PrdctCode) && (MAX_PRDCTS > PrdctCode))
      {
        switch (strlen (PrdctNm))
        {
        case 0:
          PrdctTbl[PrdctCode] = PrdctMem;
          break;

        default:
          PrdctTbl[PrdctCode] = PrdctPtr;
          strcpy (PrdctPtr, PrdctNm);
          PrdctPtr += strlen (PrdctNm) + 1;
          PrdctHdr.PoolSize += strlen (PrdctNm) + 1;
          break;
        }                       /* end of switch (strlen (PrdctNm))   */
      }                         /* end of if (...)                    */
      break;

    case 'T':                  /* Terminal Mode                      */
      /* Format: KeyVal KeyXlate            */
      {
        int ScanCode, FncIdx;

        (void) sscanf (++p, "%4x %4x", &ScanCode, &FncIdx);
        TrmnlAccelTbl[TrmnlAccelCnt].ScanCode = (short) ScanCode;
        TrmnlAccelTbl[TrmnlAccelCnt].FncIdx = (short) FncIdx;
        ++TrmnlAccelCnt;
      }
      break;

    case 'U':                  /* Unattended Mode                    */
      /* Format: KeyVal KeyXlate            */
      {
        int ScanCode, FncIdx;

        (void) sscanf (++p, "%4x %4x", &ScanCode, &FncIdx);
        UnattendedAccelTbl[UnattendedAccelCnt].ScanCode = (short) ScanCode;
        UnattendedAccelTbl[UnattendedAccelCnt].FncIdx = (short) FncIdx;
        ++UnattendedAccelCnt;
      }
      break;

    default:
      break;
    }                           /* end of switch (...)                */
  }                             /* end of while (...)                 */

  /* Close the file. Make sure the counts match and that memory size was
   * not exceeded. If so, we have a winner! If not, snort and puke. */

  (void) fclose (fpt);

  if (internal_count > pointer_size)  /* Enough pointers?          */
  {
    (void) fprintf (stderr,
                    "%d messages read exceeds pointer array size of %d\n",
                    internal_count, pointer_size);
    error = -3;
  }

  if (total_size > memory_size) /* Did we fit?               */
  {
    (void) fprintf (stderr,
                 "Required memory of %d bytes exceeds %d bytes available\n",
                    total_size, memory_size);
    error = -4;
  }

  if (count_from_file != internal_count)
  {
    (void) fprintf (stderr,
                "Count of %d lines does not match %d lines actually read\n",
                    count_from_file, internal_count);
    error = -5;
  }

  if (!error)
  {
    pointer_size = internal_count;  /* Store final usage counts  */
    memory_size = total_size;
    *load_pointers = NULL;      /* Terminate pointer table   */
  }

  return (error);
}

/* ============ end of stuff taken from GET_LANG.C ======================== */

/* ============ begin of stuff taken from PUT_LANG.C ====================== */

/*
 * put_language -- store compiled language file
 *
 * This is a simple four step operation
 *
 * 1. Open file for write
 * 2. Write out the used part of the fixup array
 * 3. Write out the used part of the memory block
 * 4. Close the file
 *
 */

int
put_language (char *name_of_file)
{
  FILE *fpt;                    /* stream pointer            */
  int error;                    /* Internal error value      */
  int wanna_write;              /* How many we wanna write   */
  int written;                  /* How many we really write  */
  long total_length;

  /* Open the file for output now. */

  fpt = fopen (name_of_file, write_binary);  /* Open the file             */
  if (fpt == NULL)              /* Were we successful?       */
  {
    fprintf (stderr, "Can not open output file %s\n", name_of_file);
    return (-1);                /* Return failure to caller  */
  }

  /* OK. Looking good so far. Write out the pointer array.
   * Don't forget that last NULL pointer to terminate it! */

  wanna_write = pointer_size;   /* Number of things to write */

  /* Write the string table count and string memory size */

  LangHdr.ElemCnt = wanna_write;
  LangHdr.PoolSize = memory_size;
  written = fwrite ((char *) &LangHdr, sizeof (struct _lang_hdr), 1, fpt);

  if (written != 1)
  {
    fprintf (stderr, "Unable to language file header\n");
    fclose (fpt);
    return (-2);
  }

  written = fwrite ((char *) pointers, sizeof (char *), wanna_write, fpt);

  if (written != wanna_write)
  {
    fprintf (stderr, "Unable to write fixup array to output file\n");
    fclose (fpt);
    return (-2);
  }

  fprintf (stderr, "Pointer Table Elements: %d\n", wanna_write);

  /* Pointer array is there. Now write out the characters. */

  wanna_write = memory_size;    /* Number of chars to write  */
  written = fwrite (memory, sizeof (char), wanna_write, fpt);

  if (written != wanna_write)
  {
    fprintf (stderr, "Unable to write characters to output file\n");
    fclose (fpt);
    return (-3);
  }

  /* Write the terminal mode remap table, first the count,
   * then the table itself. */

  written = fwrite ((char *) &TrmnlAccelCnt, sizeof (short), 1, fpt);

  if (written != 1)
  {
    fprintf (stderr, "Unable to write Terminal Accel Count\n");
    fclose (fpt);
    return (-2);
  }

  wanna_write = TrmnlAccelCnt;
  written = fwrite ((char *) TrmnlAccelTbl,
                    sizeof (struct _key_fnc),
                    wanna_write,
                    fpt);

  if (written != wanna_write)
  {
    fprintf (stderr, "Unable to write terminal accel array to output file\n");
    fclose (fpt);
    return (-2);
  }

  fprintf (stderr, "Terminal Mode Remap Table Size: %d\n", wanna_write);

  /* Write the unattended mode remap table, first, the count,
   * then the table itself. */

  written = fwrite ((char *) &UnattendedAccelCnt, sizeof (short), 1, fpt);

  if (written != 1)
  {
    fprintf (stderr, "Unable to Write Unattended Accel Count\n");
    fclose (fpt);
    return (-2);
  }

  wanna_write = UnattendedAccelCnt;
  written = fwrite ((char *) UnattendedAccelTbl,
                    sizeof (struct _key_fnc),
                    wanna_write,
                    fpt);

  if (written != wanna_write)
  {
    fprintf (stderr, "Unable to write terminal accel array to output file\n");
    fclose (fpt);
    return (-2);
  }

  fprintf (stderr, "Unattended Mode Remap Table Size: %d\n", wanna_write);

  /* Write the product code table, first the string table count and
   * string memory size, then the pointer array. */

  written = fwrite ((char *) &PrdctHdr, sizeof (struct _lang_hdr), 1, fpt);

  if (written != 1)
  {
    fprintf (stderr, "Unable to write product code header\n");
    fclose (fpt);
    return (-2);
  }

  wanna_write = PrdctHdr.ElemCnt;  /* Number of things to write */
  written = fwrite ((char *) PrdctTbl, sizeof (char *), wanna_write, fpt);

  if (written != wanna_write)
  {
    fprintf (stderr, "Unable to write prdct fixup array to output file\n");
    fclose (fpt);
    return (-2);
  }

  /* Pointer array is there. Now write out the characters. */

  wanna_write = PrdctHdr.PoolSize;  /* Number of chars to write  */
  written = fwrite (PrdctMem, sizeof (char), wanna_write, fpt);

  if (written != wanna_write)
  {
    fprintf (stderr, "Unable to write characters to output file\n");
    fclose (fpt);
    return (-3);
  }

  fprintf (stderr, "Product Code Table Size: %d\n", PrdctHdr.ElemCnt);

  /* Write out the ANSI table. */

  written = fwrite ((char *) &AnsiHdr, sizeof (struct _lang_hdr), 1, fpt);

  if (written != 1)
  {
    fprintf (stderr, "Unable to write ANSI map header\n");
    fclose (fpt);
    return (-2);
  }

  wanna_write = AnsiHdr.PoolSize;
  written = fwrite (AnsiMem, sizeof (char), wanna_write, fpt);

  if (written != wanna_write)
  {
    fprintf (stderr, "Unable to write characters to output file\n");
    fclose (fpt);
    return (-3);
  }

  fprintf (stderr, "ANSI Translate Table Size: %d\n", AnsiHdr.ElemCnt);

  /* Everything's there now. Close the file. */

  total_length = ftell (fpt);
  fprintf (stderr, "Size of complete table: %ld\n", total_length);

  error = fclose (fpt);
  if (error != 0)
  {
    fprintf (stderr, "Unable to properly close output file %s\n", name_of_file);
    return (-4);
  }

  return (0);
}

/* ============ end of stuff taken from PUT_LANG.C ======================== */


static int
usage ()
{
  fprintf (stderr,
           "To compile a language file:\n"
           "\n"
           "    BTUTIL LNG language_file_name output_file_name\n"
           " or\n"
           "    BTUTIL LNG key X language_file_name output_file_name_X\n"
           "\n"
           "To patch an .EXE file with a compiled language file:\n"
           "    BTUTIL LNG patch exe_file_name compiled_language_file_name\n"
           "\n"
           "Examples:\n"
           "    BTUTIL LNG english.txt binkley.lng\n"
           "    BTUTIL LNG patch bt32.exe binkley.lng\n"
           "    BTUTIL LNG key E language.txt english.lng\n"
           "    BTUTIL LNG key S language.txt startrek.lng\n"
           "\n"
           "To compile a MAIL.SYS control file:\n"
           "\n"
           "    BTUTIL CTL\n"
           "\n"
           "To get history data from misc. other programs:\n"
           "\n"
           "    BTUTIL GETHIST {Max3|RA|BGFax|Telegard} ...\n"
           "\n"
    );
  return (255);
}

int
gethist_main (int argc, char **argv)
{
  int ret = 0;

  if (argc < 2)
    ret = usage ();
  else
  {
    if (!stricmp (argv[1], "max3"))
    {
#ifndef __unix__
      ret = gethist_max3_main (--argc, ++argv);
#else
      ret = 1;
#endif
    }
    else if (!stricmp (argv[1], "ra"))
      ret = gethist_ra_main (--argc, ++argv);
    else if (!stricmp (argv[1], "bgfax"))
      ret = gethist_bgfax_main (--argc, ++argv);
    else if (!stricmp (argv[1], "telegard"))
      ret = gethist_telegard_main (--argc, ++argv);
    else
      ret = usage ();
  }

  return ret;
}

MAINTYPE
main (int argc, char **argv)
{
  int ret;

  fprintf (stderr, "BinkleyTerm Utility Version 2.60XE");
  fprintf (stderr,
  "\n(C) Copyright 1987-96, Bit Bucket Software, Co. ALL RIGHTS RESERVED.");
  fprintf (stderr, "\nThis version was modified by Binkley XE Team.\n\n");

  if (argc < 2)
    ret = usage ();
  else
  {
    if (!stricmp (argv[1], "lng"))
      ret = lng_main (--argc, ++argv);
    else if (!stricmp (argv[1], "ctl"))
      ret = ctl_main ();
    else if (!stricmp (argv[1], "gethist"))
      ret = gethist_main (--argc, ++argv);
    else
      ret = usage ();
  }
#ifdef INTMAIN                  /* jl 960726 main() is still declared void for some compilers */
  return ret;
#else
  exit (ret);
#endif
}

/* $Id: btutil.c,v 1.11 1999/03/23 22:28:40 mr Exp $ */
