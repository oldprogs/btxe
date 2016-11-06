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
 * Filename    : $Source: E:/cvs/btxe/src/b_frproc.c,v $
 * Revision    : $Revision: 1.15 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/07/30 17:37:18 $
 * State       : $State: Exp $
 * Orig. Author: Wynn Wagner III
 *
 * Description : BinkleyTerm File Request Processor
 *
 * Note        :
 *   This module is based largely on a similar module in OPUS-CBCS V1.03b.
 *   The original work is (C) Copyright 1987, Wynn Wagner III. The original
 *   author has graciously allowed us to use his code in this work.
 *
 *   Much rewritten by Alex Woick from the original BT 2.60 source.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"
#include "squish.h"

/* for the Maximus 3.0 structs */
typedef word zstr;
typedef signed long sdword;

#include "third/max3/option.h"
#include "third/max3/newarea.h"
#include "third/max3/uclass.h"


/* TE 960518: Proboard structs */

struct _pbfile
{
  byte name[13];
  word area;
};


struct _pbarea
{
  char name[80];                /* Name of file area                           */
  char filesbbs[80];            /* Path for file-list                          */
  byte filepath[80];            /* Path for files w/o+ trailing \             */
  short filepriv;               /* Level needed to access file area            */
  word flags1;                  /* Flags needed to access file area            */
  word flags2;                  /* Flags needed to access file area            */
  byte dummy[20];               /* not needed for filerequest purposes         */
};

/* Maximus 2.0 structs */

#define AREA_ID 0x54414441L     /* "ADAT" */

#define MAX_ALEN    10          /* Max length of usr.msg[] and usr.files[]  */
#define MAX_OVR     16          /* Maximum # of override privs per area     */
#define NUM_MENU    256         /* Max. # of options in a menu file         */
#define MAXCLASS    12          /* number of possible priv levels           */


/* New Max 2.xx format for MAXFILES.IDX.  The file is simply an array of    *
 * these structures.                                                        */

struct _maxfile
{
  byte name[12];
  word areaofs;
  word fileofs;
};

struct _override
{
  signed short priv;            /* Override priv level */
  unsigned long lock;           /* Override lock setting */

  byte ch;                      /* First letter of menu option to override  */
  byte fill;                    /* Reserved by Maximus */
};

struct _maxarea                 /* TE 960518: renamed from _area to _maxarea */
{
  long id;                      /* Unique identifier for AREA.DAT structure.*
                                 * Should be AREA_ID, above.                */

  word struct_len;              /* Length of _maxarea structure -- this needs  *
                                 * only to be read from the first record in *
                                 * an area data file, since it can be       *
                                 * assumed to remain the same throughout the*
                                 * entire file.  This is GUARANTEED to be at*
                                 * offset four for this and all future      *
                                 * versions of this structure.              */

  word areano;                  /* OBSOLETE. Two-byte integer representation*
                                 * of the area name. Use area.name instead. */

  byte name[40];                /* String format of area's name.  USE THIS! */

  /* ********************************************************************** */
  /*                        Message Area Information                        */
  /* ********************************************************************** */

  word type;                    /* Message base type.  MSGTYPE_SDM = *.MSG. *
                                 * MSGTYPE_SQUISH = SquishMail.  (Constants *
                                 * are in MSGAPI.H)                         */

  byte msgpath[80];             /* Path to messages                         */
  byte msgname[40];             /* The 'tag' of area, used in ECHOTOSS.LOG  */
  byte msginfo[80];             /* The DIR.BBS-like desc for msg section    */
  byte msgbar[80];              /* Barricade file for message area          */
  byte origin[62];              /* The ORIGIN line for this area            */

  short msgpriv;                /* This is the priv required to access the  *
                                 * msg section of this area.                */
  byte fill0;                   /* The lock for the message area (obsolete) */

  byte fill1;

  short origin_aka;             /* The AKA number to use on the origin line.*
                                 * See the normal SysOp documentation on    *
                                 * the "Origin" statement, for info on how  *
                                 * this number is used.                     */

  /* ********************************************************************** */
  /*                        File Area Information                           */
  /* ********************************************************************** */

  byte filepath[80];            /* Path for downloads                       */
  byte uppath[80];              /* Path for uploads                         */
  byte filebar[80];             /* Barricade file for file areas            */
  byte filesbbs[80];            /* Path to FILES.BBS-like catalog for area  */
  byte fileinfo[80];            /* The DIR.BBS-like desc for file section   */

  short filepriv;               /* The priv required to access the file     *
                                 * section of this area.                    */
  byte fill15;                  /* The locks for the file area (obsolete)   */
  byte fill2;

  /* ********************************************************************** */
  /*                      Miscellaneous Information                         */
  /* ********************************************************************** */

  byte msgmenuname[13];         /* Alternate *.MNU name to use for msg.area     */
  byte filemenuname[13];        /* Alternate *.MNU name to use for file area    */

  word attrib[MAXCLASS];        /* This is an array of attributes for the       *
                                 * msg/file areas.  These are dependant on PRIV *
                                 * level.  Once you have the CLASS number for a *
                                 * particular user (via Find_Class_Number()),you*
                                 * can find the attributes for that particular  *
                                 * priv level like this: "area.attrib[class]"   *
                                 * ...which will get you the attribute for that *
                                 * priv level.                                  */

  /* ********************************************************************** */
  /*                      Stuff hacked on later                             */
  /* ********************************************************************** */

  struct _override movr[MAX_OVR];  /* Override privs for msg/file areas    */
  struct _override fovr[MAX_OVR];

  unsigned long msglock;        /* 32-bit locks for message areas          */
  unsigned long filelock;       /* 32-bit locks for file areas             */

  /* MAXREN: max # of days to keep msgs in area (use 0 for no deletion by age) */
  word killbyage;

  /* MAXREN: max # of msgs to keep in area (use 0 for no deletion by #msgs.)   */
  word killbynum;

};

/* AW971206 structure of files.dat */
/* Structure for FILES.DAT in each file directory */

typedef struct _fdat
{
  byte name[13];                /* Name of the file                           */
  byte struct_len;              /* Length of this structure.  USE THIS!       */

  word flag;                    /* See FF_XXXX definitions, above             */

  dword acs;                    /* Offset to ACS in FILES.DMP                 */
  word rsvd1;

  dword times_dl;               /* # of times this file has been DLed         */

  union stamp_combo fdate;      /* Date of actual file (creation)             */
  union stamp_combo udate;      /* Date of upload (modification)              */

  dword fsize;                  /* File's size, in bytes                      */

  /* Offsets in FILES.DMP */

  dword uploader;               /* Uploader's name                            */
  dword desc;                   /* File description (or comment if no file)   */
  dword path;                   /* Path of file - 0 if in normal file dir     */
  dword pwd;                    /* Password for file                          */

  char rsvd[10];                /* Reserved by Maximus for future use         */
}
FDAT;

/* request response codes */

#define REQ_NOTFOUND   1        /* file not found                             */
#define REQ_NOUPDATE   2        /* no update necessary                        */
#define REQ_WRONGPWD   3        /* password missing or wrong                  */
#define REQ_FILELIMIT  4        /* file request limit exceeded                */
#define REQ_EVTOVERRUN 5        /* start of no-requests-honored event         */
#define REQ_BYTELIMIT  6        /* file would exceed request byte limit       */
#define REQ_TIMELIMIT  7        /* file would exceed request time limit       */
#define REQ_BAUDLIMIT  8        /* baud rate too low (minbaud)                */
#define REQ_SUCCESS    9        /* successful file request                    */


/* AW 980216 structure to hold info about the current request */
struct _reqinfo
{                               /* info from the *.req file                    */
  char *REQline;                /* original line from *.REQ file               */
  char request[PATHLEN];        /* the requested filespec                      */
  char their_pwd[16];           /* the supplied password                       */
  char updtype;                 /* 0=normal request, '-' or '+'=update request */
  long updreq;                  /* time from update request                    */
  char their_wildcard[15];      /* requested file expanded with ??? wildcards  */

  /* info from the current okfile line           */
  char *okfilepath;             /* current okfile line                         */
  char required_pwd[16];        /* required password                           */
  char *after_pwd;              /* stuff after the password in okfile          */
  char magicfunc;               /* 0=normal filespec, otherwise magic func     */

  long alivetimer;              /* timer for request aborts                    */
  int failure_reason;           /* constants from REQ_*                        */
  int (*callback) (char *);     /* callback for sending files                  */
  int (*calltime) (long);       /* callback for transfer time calculation      */

};


static int LOCALFUNC prep_match (char *, char *);
static int LOCALFUNC match (char *, char *);
static int LOCALFUNC process_okfile_line (char *, struct _reqinfo *);
static int LOCALFUNC process_magic (struct _reqinfo *);
static void LOCALFUNC run_prog (char *);
static int LOCALFUNC check_password (char *, char *);
static int LOCALFUNC freq_abort (long, int (*)(long));
static int LOCALFUNC what_event (int);
static int LOCALFUNC what_event_sub (int, int, int, struct tm *);
static void LOCALFUNC check_HLO (int (*)(char *));
static int LOCALFUNC make_RSP (char *, int, int (*)(char *));
static int LOCALFUNC get_req_info (char *, struct _reqinfo *,
                                   int (*)(char *),
                                   int (*)(long));
static int LOCALFUNC get_okfile_info (char *, struct _reqinfo *);
static int LOCALFUNC send_filespec (char *, struct _reqinfo *, int);
static int LOCALFUNC check_timed_abort (struct _reqinfo *);
static int LOCALFUNC send_file (char *, struct _reqinfo *);

static int LOCALFUNC search_via_simple_index (struct _reqinfo *,
                                              int,  /* index is sorted */
                                              long (LOCALFUNC *) (FILE *),  /* get index structlen    */
                                              long (LOCALFUNC *) (FILE *),  /* get areafile structlen */
                                              char *(LOCALFUNC *) (void *),  /* get filename from index */
                                  long (LOCALFUNC *) (void *, long, FILE *),  /* get areanr from index  */
                               char *(LOCALFUNC *) (FILE *, char *, void *),  /* get area heap   */
                       char *(LOCALFUNC *) (char *, void *, void *, char *),  /* get filepath    */
                                        int (LOCALFUNC *) (void *, char *));  /* security check  */

/* Maximus 2.0 indexed request functions */
static int LOCALFUNC process_maximus_index (struct _reqinfo *);
static long LOCALFUNC max2_get_idx_structlen (FILE *);
static long LOCALFUNC max2_get_dat_structlen (FILE *);
static char *LOCALFUNC max2_get_idx_fname (void *);
static long LOCALFUNC max2_get_idx_areaofs (void *, long, FILE *);
static char *LOCALFUNC max2_get_filepath (char *, void *, void *, char *);
static int LOCALFUNC max2_privok (void *, char *);
static void LOCALFUNC GetPathFromFilesDat (char *, char *, word, char *);

/* Proboard indexed request functions */
static int LOCALFUNC process_proboard_index (struct _reqinfo *);
static long LOCALFUNC pb_get_idx_structlen (FILE *);
static long LOCALFUNC pb_get_dat_structlen (FILE *);
static char *LOCALFUNC pb_get_idx_fname (void *);
static long LOCALFUNC pb_get_idx_areaofs (void *, long, FILE *);
static char *LOCALFUNC pb_get_filepath (char *, void *, void *, char *);
static int LOCALFUNC pb_privok (void *, char *);

/* Maximus 3.0 indexed request functions */
static long LOCALFUNC max3_get_dat_structlen (FILE *);
static long LOCALFUNC max3_get_idx_areaofs (void *, long, FILE *);
static char *LOCALFUNC max3_get_dat_heap (FILE *, char *, void *);
static char *LOCALFUNC max3_get_filepath (char *, void *, void *, char *);
static int LOCALFUNC max3_privok (void *, char *);

bool ACS (word, dword, char *, char *, char *);

/* local prototypes for ACS handling */
static bool LOCALFUNC get_max_classname (void);
static struct _max_classname *LOCALFUNC free_max_classname (struct _max_classname *);

static word LOCALFUNC class_to_priv (char *);
static dword LOCALFUNC parse_flags (char *);
static dword LOCALFUNC key2bit (char);
static bool LOCALFUNC Key (dword, char);
static bool LOCALFUNC NotKey (dword, char);
static bool LOCALFUNC NameAlias (char *, char *);
static bool LOCALFUNC Keylist (dword, char *);
static bool LOCALFUNC PrivKeyOp (word, dword, enum _privcmp, char *);
static bool LOCALFUNC simpleACS (word, dword, char *, char *, char *);

/* now local variables */
static int xfer_seconds;

/* default Maximus 2.0 classnames list */
struct _max_classname fri_max2_classname[] =
{
  {65534, "Twit"},
  {0, "Disgrace"},
  {1, "Limited"},
  {2, "Normal"},
  {3, "Worthy"},
  {4, "Privil"},
  {5, "Favored"},
  {6, "Extra"},
  {7, "Clerk"},
  {8, "Asstsysop"},
  {10, "Sysop"},
  {11, "Hidden"},
  {65535, NULL}
};


static int LOCALFUNC
prep_match (char *template, char *buffer)
{
  register int i, delim;
  register char *sptr;
  int start;

  memset (buffer, 0, 11);

  i = (int) strlen (template);
  sptr = template;

  for (start = i = 0; sptr[i]; i++)
    if ((sptr[i] == DIR_SEPC) || (sptr[i] == ':'))
      start = i + 1;

  if (start)
    sptr += start;
  delim = 8;                    /* last column for '?' */

  /* strupr (sptr); 
     CEH990331: Don't uppercase template, but just the result (buffer) */

  for (i = 0; *sptr && i < 12; sptr++)
    switch (*sptr)
    {
    case '.':
      if (i > 8)
        return (-1);
      while (i < 8)
        buffer[i++] = ' ';
      buffer[i++] = toupper (*sptr);
      delim = 12;
      break;

    case '*':
      while (i < delim)
        buffer[i++] = '?';
      break;

    default:
      buffer[i++] = toupper (*sptr);
      break;
    }                           /* switch */

  while (i < 12)
  {
    if (i == 8)
      buffer[i++] = '.';
    else
      buffer[i++] = ' ';
  }

  buffer[i] = '\0';

  return 0;
}

static int LOCALFUNC
match (char *s1, char *s2)
{
  register char *i, *j;

  i = s1;
  j = s2;

  while (*i)
  {
    if ((*j != '?') && (*i != *j))
      return ((int) (*i - *j));
    i++;
    j++;
  }

  return 0;
}

/*--------------------------------------------------------------------------*/
/* Check wild against string. return 1 if it matches, otherwise 0           */
/*--------------------------------------------------------------------------*/

int LOCALFUNC
wildcard_match (char *wild, char *str)
{
  for (; *wild; wild++, str++)
  {
    switch (*wild)
    {
    case '*':
      do
        wild++;
      while (*wild == '*');
      if (*wild)
      {
        for (; *str; str++)
          if ((*wild == *str) || (*wild == '?'))
            if (wildcard_match (wild, str))
              return (1);
        return (0);
      }
      else
        return (1);
    case '?':
      if (!*str)
        return (0);
      break;
    default:
      if (tolower (*wild) != tolower (*str))
        return (0);
      break;
    }
  }
  if (*str)
    return (0);
  else
    return (1);
}

/*--------------------------------------------------------------------------*/
/* Process file requests from the remote system. The filespec requested is  */
/* turned into a local filespec if possible, then transferred via the       */
/* caller-supplied routine.                                                 */
/*--------------------------------------------------------------------------*/

int
n_frproc (char *REQline,
          int (*callback) (char *),
          int (*calltime) (long))
{
  struct _reqinfo reqinfo;

  char s[PATHLEN];
  FILE *approved;
  int sentresult;
  int nfiles = 0;

  if (freq_accum.time == 0L)    /* begin recording of request time */
    freq_accum.time = (long) unix_time (NULL);

  if (!get_req_info (REQline, &reqinfo, callback, calltime))
    return 0;                   /* no filename?                    */

  if (freq_abort (0L, calltime))  /* Any reason to abort? */
    return -2;                  /* If so, return error. */

  /* ---------------------------------------------------------------------- */
  /* Initialization(s)                                                      */
  /* ---------------------------------------------------------------------- */

  status_line ("*%s %s (%s)", (reqinfo.updreq != 0L)
               ? MSG_TXT (M_UPDATE) : MSG_TXT (M_FILE), MSG_TXT (M_REQUEST),
               reqinfo.request);

  /* ---------------------------------------------------------------------- */
  /* See if the file is approved for transmission                           */
  /* ---------------------------------------------------------------------- */

  if (QueryNodeFlag (noWildcards, NOJOKERS, &remote_addr))  /*r. hoerner */
    if (strchr (reqinfo.their_wildcard, '?'))
    {
      /* send .RSP file, frequest limit exceeded */
      status_line ("*the node has no wildcards allowed!");
      return make_RSP (reqinfo.request, REQ_FILELIMIT, callback);
    }

  approved = share_fopen (CURRENT.rq_OKFile, read_ascii, DENY_WRITE);
  if (approved == NULL)
  {
    /* send .RSP file, no files found */
    got_error (MSG_TXT (M_OPEN_MSG), CURRENT.rq_OKFile);
    return make_RSP (reqinfo.request, REQ_NOTFOUND, callback);
  }

  while (fgets (s, PATHLEN - 1, approved) != NULL &&
         (reqinfo.failure_reason < REQ_FILELIMIT ||
          reqinfo.failure_reason == REQ_SUCCESS))
  {
    sentresult = process_okfile_line (s, &reqinfo);
    if (sentresult < 0)
    {
      fclose (approved);        /* return error, abort requests */
      return -1;
    }

    nfiles += sentresult;       /* count sent files */
  }

  fclose (approved);

  sentresult = make_RSP (reqinfo.request, reqinfo.failure_reason, callback);
  if (sentresult < 0)
    return -1;

  return nfiles + sentresult;
}


static int LOCALFUNC
process_okfile_line (char *s, struct _reqinfo *reqinfo)
{
  if (!get_okfile_info (s, reqinfo))
    return 0;                   /* no info in this line, cont. search */

  if (reqinfo->magicfunc)
    return process_magic (reqinfo);  /* magic request */

  return send_filespec (s, reqinfo, 0);
}


static int LOCALFUNC
process_magic (struct _reqinfo *reqinfo)
{
  char s[PATHLEN];

  if (reqinfo->magicfunc == '*')  /* search in request index */
  {
    switch (fri_areatype)
    {
    case FRI_MAXIMUS:
      return process_maximus_index (reqinfo);

    case FRI_PROBOARD:
      return process_proboard_index (reqinfo);

    default:
      break;
    }

    /* error in this okfile line, but continue search */
    status_line (MSG_TXT (M_OKFILE_ERR), reqinfo->okfilepath);
    return 0;
  }

  if (stricmp (reqinfo->okfilepath, reqinfo->request))
    return 0;                   /* magic name does not match, cont. search */

  /* Name matches, check password */
  if (!(check_password (reqinfo->required_pwd, reqinfo->their_pwd)))
  {
    reqinfo->failure_reason = REQ_WRONGPWD;  /* password doesn't match */
    return 0;
  }

  /* now handle real magics */

  switch (reqinfo->magicfunc)
  {
  case '$':
    sprintf (s, reqinfo->after_pwd,
             (int) remote_addr.Net,
             (int) remote_addr.Node,
             (int) remote_addr.Point);
    break;

  case '+':
    if (strlen (reqinfo->REQline) > PATHLEN - 26)
      reqinfo->REQline[PATHLEN - 26] = '\0';
    sprintf (s, "%s %u %u %u %u", reqinfo->REQline,
             (int) remote_addr.Zone, (int) remote_addr.Net,
             (int) remote_addr.Node, (int) remote_addr.Point);
    break;

  case '@':
    return send_filespec (reqinfo->after_pwd, reqinfo, 1);

  default:
    status_line (MSG_TXT (M_OKFILE_ERR), reqinfo->okfilepath);
    return 0;
  }

  run_prog (s);
  check_HLO (reqinfo->callback);
  reqinfo->failure_reason = REQ_SUCCESS;  /* file "sent" ok */
  return 0;
}


static void LOCALFUNC
run_prog (char *s)
{
  struct baud_str saved_baud;

  status_line ("%s '%s'", MSG_TXT (M_EXECUTING), s);

  if (un_attended)
    screen_clear ();

  scr_printf (&(MSG_TXT (M_EXECUTING)[1]));
  scr_printf (s);
  scr_printf ("\r\n");
  vfossil_cursor (1);
  saved_baud = cur_baud;
  b_spawn (s);
  vfossil_cursor (0);

  if (un_attended)
  {
    screen_clear ();
    sb_dirty ();
    opening_banner ();
    mailer_banner ();
    show_session (MSG_TXT (M_MCP_MAIL_XFER), &remote_addr);
  }

  set_baud (saved_baud.rate_value, 0);  /* Restore baud rate */
}

static int LOCALFUNC
check_password (char *required_pwd, char *their_pwd)
{
  /* RDH *** Changes begin */
  if (required_pwd != NULL && required_pwd[0])
  {
    if (their_pwd == NULL || !their_pwd[0])
    {
      status_line (MSG_TXT (M_FREQ_PW_ERR), required_pwd, "(NULL)", "(NULL)");
      return (0);
    }

    fancy_str (required_pwd);
    if (stricmp (required_pwd, their_pwd))
    {
      status_line (MSG_TXT (M_FREQ_PW_ERR), required_pwd,
                   their_pwd ? their_pwd : "(NULL)");
      return (0);
    }
  }

  return (1);
  /* RDH *** Changes end */
}

static int LOCALFUNC
freq_abort (long file_size, int (*calltime) (long))
{
  int w_event;
  int xfer_time;

  if (!CARRIER)
  {
    status_line (MSG_TXT (M_CARRIER_REQUEST_ERR));
    return (1);
  }

  if ((CURRENT.rq_Limit != 0) && ((freq_accum.files + 1) >= CURRENT.rq_Limit))
  {
    status_line (MSG_TXT (M_FREQ_LIMIT));
    return (4);
  }

  if ((CURRENT.byte_Limit != 0L) && ((freq_accum.bytes + file_size)
                                     > CURRENT.byte_Limit))
  {
    status_line (MSG_TXT (M_BYTE_LIMIT));
    return (6);
  }

  /* Get file time in seconds for comparison with quota */

  if (file_size != 0L)
    xfer_time = calltime (file_size);
  else
    xfer_time = 0;

  xfer_seconds = xfer_time;

  if ((CURRENT.time_Limit != 0)
      && (((long) unix_time (NULL) + xfer_time - freq_accum.time)
          > CURRENT.time_Limit))
  {
    status_line (MSG_TXT (M_TIME_LIMIT));
    return (7);
  }

  xfer_time /= 60;              /* We want minutes for event calculations */

  w_event = what_event (xfer_time);
  if ((w_event >= 0) && (w_event != cur_event))
  {
    if (e_ptrs[w_event].behavior & MAT_NOREQ)
    {
      status_line (MSG_TXT (M_EVENT_OVERRUN));
      return (5);
    }
  }

#if 0
  else if (w_event == -2)       /* TJW 960927 !!! this is NEVER the case !!! */
  {
    status_line (MSG_TXT (M_EVENT_OVERRUN));
    return (5);
  }
#endif

  return (0);
}

static int LOCALFUNC
what_event (int delta_time)
{
  time_t long_time;
  struct tm *tm;

  int now;
  int tomorrow = 0;
  int our_time;
  int w_event;
  int save_time;

  /* Get the current time into a structure */
  unix_time (&long_time);
  tm = unix_localtime (&long_time);

  /* Calculate minutes since midnight      */
  now = tm->tm_hour * 60 + tm->tm_min;

  /* Then number of minutes past midnight at end of delta_time */
  save_time = our_time = delta_time + now;

  /* Is the end in Tomorrow? (Sounds like a soap opera, eh?) */
  if (1440 < our_time)
  {
    tomorrow = 1;               /* Remember that we've wrapped        */
    our_time = 23 * 60 + 59;    /* Set event end = 23:59 for now      */
  }

  /* Test "today," and if we find an event, get the hell out */
  w_event = what_event_sub (now, our_time, 0, tm);

  if (w_event != -1)
    return w_event;

  /* If we need to test tomorrow, figure out all the salient info
   * on what tomorrow is, and test that. */

  if (tomorrow)
  {
    long_time += 86400L;        /* Any time tomorrow will do.         */
    tm = unix_localtime (&long_time);
    w_event = what_event_sub (0, (save_time - 1440), 1, tm);
  }

  return w_event;
}

static int LOCALFUNC
what_event_sub (int now, int our_time, int tomorrow, struct tm *tm)
{
  int cur_mon = tm->tm_mon;
  int cur_day = tm->tm_wday;
  int cur_mday = tm->tm_mday;
  int i;

  cur_day = 1 << cur_day;       /* Make cur_day a bit mask             */

  for (i = 0; i < num_events; i++)
  {
    if (e_ptrs[i].behavior & MAT_COSTEVENT)  /* we aren't interested in */
      continue;                 /* costevents, skip them   */

    /* If this is an event we need to think about (i.e., it started
     * before the end of the transfer, and ends after "now") ... */

    if ((our_time >= e_ptrs[i].minute)
        && (now <= (e_ptrs[i].minute + e_ptrs[i].length)))
    {
      /* And if it's on a day we care about ... */
      if ((cur_day & e_ptrs[i].days)
          && ((!e_ptrs[i].day) || (e_ptrs[i].day == (char) cur_mday))
          && ((!e_ptrs[i].month) || (e_ptrs[i].month == (char) cur_mon)))
      {
        /* Don't do events that have been exited already unless
         * the tomorrow flag is set (in which case, no events have
         * occurred, have they?) */

        if (!tomorrow
            && (e_ptrs[i].last_ran_date == custom_date (tm))
            && (e_ptrs[i].behavior & MAT_SKIP))
          continue;

        /* If the event we are considering does not allow requests,
         * then this is a bugger we are looking for. Return its index. */

        if ((e_ptrs[i].behavior & MAT_NOREQ))
          return i;
      }                         /* end of if ((cur_day & ... ))        */
    }                           /* end of if ( our_time >= ... )       */
  }                             /* end of for ( ... )                  */

  return -1;
}

/* -------------------------------------------------------------------- */
/* See if we generated a .QLO file somehow, if so send listed files     */
/* -------------------------------------------------------------------- */

static void LOCALFUNC
check_HLO (int (*callback) (char *))
{
  CLEAR_INBOUND ();
  *ext_flags = 'q';
  do_FLOfile (ext_flags, callback);

  /* -------------------------------------------------------------------- */
  /* Maybe the magic request made a conventional .FLO file, try that too  */
  /* -------------------------------------------------------------------- */

  *ext_flags = 'f';
  do_FLOfile (ext_flags, callback);
  return;
}



static int LOCALFUNC
make_RSP (char *requested_name, int failure_reason, int (*callback) (char *))
{
  char s[PATHLEN];

  strntcpy (s, requested_name, PATHLEN);
  if ((CURRENT.rq_Template != NULL) && (dexists (CURRENT.rq_Template)))
    Make_Response (s, failure_reason);  /* Build a response      */
  else
    return 0;

  if (!s[0])                    /* if no .RSP file,      */
    return 0;                   /* give it up            */

  CLEAR_INBOUND ();
  if ((*callback) (s) == FILE_SENT_OK)
    return 1;

  return 0;
}


/*----------------------------------------------------------------------*/
/* prepare the file name and other request info                         */
/*----------------------------------------------------------------------*/

static int LOCALFUNC
get_req_info (char *REQline, struct _reqinfo *reqinfo,
              int (*callback) (char *), int (*calltime) (long))
{
  int i, j;
  char *p;

  /* FD sends "alive" every 5 seconds */
  reqinfo->alivetimer = timerset (5 * PER_SECOND);
  reqinfo->callback = callback;
  reqinfo->calltime = calltime;
  reqinfo->REQline = REQline;
  reqinfo->updreq = 0L;
  reqinfo->updtype = 0;
  reqinfo->failure_reason = REQ_NOTFOUND;  /* file not (yet) found */

  /* get filespec */
  for (i = 0, p = reqinfo->request;
       REQline[i] && REQline[i] > ' ' && i < PATHLEN;
       i++)
  {
    if ((REQline[i] == ':') || (REQline[i] == DIR_SEPC))
    {
      reqinfo->request[0] = '\0';
      return 0;
    }

    p[i] = REQline[i];
  }

  p[i++] = '\0';

  prep_match (p, reqinfo->their_wildcard);

  /* get password */
  p = reqinfo->their_pwd;
  p[0] = '\0';

  if (REQline[i] == '!')
  {
    j = 0;
    i++;
    while (REQline[i] && REQline[i] > ' ' && j < 15)
      p[j++] = REQline[i++];

    p[j] = '\0';
  }

  /* Test for update/backdate request */

  i++;
  if (reqinfo->request[i] == '+' || reqinfo->request[i] == '-')
  {
    reqinfo->updtype = reqinfo->request[i++];
    reqinfo->updreq = atol (&reqinfo->request[i]);
  }

  return 1;
}


/*----------------------------------------------------------------------*/
/* prepare the okfile information                                       */
/*----------------------------------------------------------------------*/

static int LOCALFUNC
get_okfile_info (char *s, struct _reqinfo *reqinfo)
{
  int i;
  char *p;

  reqinfo->okfilepath = s;
  reqinfo->required_pwd[0] = '\0';
  reqinfo->after_pwd = NULL;
  reqinfo->magicfunc = '\0';

  if (s[0] == ';')
    return 0;

  for (i = 0; s[i]; i++)
    if (s[i] == 0x09)
      s[i] = ' ';
    else if (s[i] < ' ')
      s[i] = '\0';

  if (!s[0])
    return 0;

  /* -------------------------------------------------------------- */
  /* Check for transaction-level password                           */
  /* -------------------------------------------------------------- */

  /* TJW 961013 modifications begin */

  /* this part of binkley was not quite "error tolerant" - you had to */
  /* have EXACTLY 1 blank between filespec and password in your OKFile */
  /* or anybody could freq files from the directories you thought to  */
  /* be PW-protected. Also if password had 6 characters or more the   */
  /* \0-termination was not ensured.                                  */

  p = skip_to_blank (s);        /* skip over "filespec"                  */
  reqinfo->after_pwd = p;       /* init after_pwd                        */
  if (*p)                       /* is there anything left ?              */
  {
    *p++ = '\0';                /* yes:       terminate filespec with \0 */
    p = skip_blanks (p);        /*            skip blanks following      */
    reqinfo->after_pwd = p;     /* init after_pwd (for cases w/o pwd)    */
    if (*p == '!')              /* is there a password definition ?      */
    {
      strntcpy (reqinfo->required_pwd, ++p, 16);  /* copy <=16 chars  */
      reqinfo->after_pwd = skip_to_blank (p);  /* set after_pwd to next  */
      reqinfo->after_pwd = skip_blanks (reqinfo->after_pwd);  /* word after passwd    */
      for (i = 0; reqinfo->required_pwd[i]; i++)
      {
        if (reqinfo->required_pwd[i] <= ' ')  /* chars as pwd then seek */
          reqinfo->required_pwd[i] = '\0';  /* pwd end (== blank) and */
        /* terminate it with \0 ! */
      }
    }
  }
  /* TJW 961013 modifications end */

  if (!s[0])
    return 0;

  /* now check for special entry in okfile (magic, request index) */
  if (strchr ("*@+$", s[0]))
  {
    reqinfo->magicfunc = s[0];
    reqinfo->okfilepath++;
  }

  return 1;
}


/*----------------------------------------------------------------------*/
/* send_filespec: send all files in the given filespec with limit check */
/*     send_all=1 send all files in filespec                            */
/*     send_all=0 check filespec with reqinfo->their_wildcard           */
/*                                                                      */
/*     result:  -1 carrier lost                                         */
/*             >=0 number of files sent                                 */
/*----------------------------------------------------------------------*/

static int LOCALFUNC
send_filespec (char *filespec, struct _reqinfo *reqinfo, int send_all)
{
  struct FILEINFO dta;
  char s2[PATHLEN];
  char *p;
  int filesent = 0;
  int i, status = 1;

  if (!send_all)                /* check match only if not sending all files (magic) */
  {
    // CEH; 980723; modification starts
    // If the filename is in the okfile explicitly (without wildcards), we
    // have a much faster method to check if the filename matches the
    // requested one, than first getting the filename with dfind (quite slow
    // on old cdromdrives for example ;)
    // first check, if filespec does not contain any wildcard

    for (i = 0; filespec[i]; i++)
    {
      if ((filespec[i] == '?') || (filespec[i] == '*'))
      {
        status = 0;
        break;
      }
    }

    // if there are no wildcards, and the filenames do not match, return.
    // otherwise just continue.

    if (status)
    {
      for (i = strlen (filespec) - 1; i >= 0; i--)
        if (filespec[i] == DIR_SEPC)
          break;

      i++;

      if (!wildcard_match (reqinfo->request, &(filespec[i])))
        return (0);
    }

    // CEH; modification ends
  }

  if (dfind (&dta, filespec, 0))
  {
    status_line (MSG_TXT (M_OKFILE_ERR), filespec);
    return 0;
  }
  else
    do
    {
      if ((status = check_timed_abort (reqinfo)) != 1)  /*    1 = ok */
        break;

      /* reset status, no files sent this iteration (yet) */
      status = 0;

      if (!send_all)
      {

        if (!wildcard_match (reqinfo->request, dta.name))
          continue;

        /* File names match, check password now */

        if (!check_password (reqinfo->required_pwd, reqinfo->their_pwd))
        {
          reqinfo->failure_reason = REQ_WRONGPWD;  /* Password doesn't match */
          break;                /* Go on with next path   */
        }
      }

      /* Good password, get path with wildcard from OKFILE  */
      strntcpy (s2, filespec, PATHLEN);

      /* Subtract the wild card file name, keep path */
      if ((p = strrchr (s2, DIR_SEPC)) != NULL)
        *++p = '\0';
      else
        s2[0] = '\0';

      /* Then add in the exact name found */
      strcat (s2, dta.name);

      /* Got full filename, now send file */
      if ((status = send_file (s2, reqinfo)) > 0)
        ++filesent;

    }
    while (!dfind (&dta, filespec, 1) && status != -1);

  dfind (&dta, filespec, 2);

  return status == -1 ? -1 : filesent;
}


/*----------------------------------------------------------------------*/
/* check for an abort cause every 5 Seconds                             */
/*   result:                                                            */
/*   -1 abort, carrier lost                                             */
/*    0 abort, send .RSP file and handle no more requests               */
/*    1 ok                                                              */
/*----------------------------------------------------------------------*/
static int LOCALFUNC
check_timed_abort (struct _reqinfo *reqinfo)
{
  int testabort;

  if (timeup (reqinfo->alivetimer))
  {
    /* See if we want to quit */
    if ((testabort = freq_abort (0L, reqinfo->calltime)) != 0)
    {
      if (testabort == 1)       /* Yes, why? Carrier?     */
        return -1;              /* uh-huh -- get out fast */
      else
      {
        reqinfo->failure_reason = testabort;
        return 0;               /* make a .RSP file and abort */
      }
    }

    reqinfo->alivetimer = timerset (5 * PER_SECOND);
    SENDBYTE (NUL);             /* buy more time */
  }

  return 1;
}


/*----------------------------------------------------------------------*/
/* send one file with limit check                                       */
/*   result: -1 carrier lost                                            */
/*            0 no file sent (skipped or something)                     */
/*            1 file or error response successfully sent                */
/*----------------------------------------------------------------------*/

static int LOCALFUNC
send_file (char *file, struct _reqinfo *reqinfo)
{
  int status = 0;
  struct stat st;

  if (!unix_stat_noshift (file, &st))
  {
    /* Got full filename, now do file update validation */

    if (reqinfo->updtype)
    {                           /* TJW 960421 mtime not atime */
      if ((reqinfo->updtype == '+' && (st.st_mtime <= reqinfo->updreq)) ||
          (reqinfo->updtype == '-' && (st.st_mtime >= reqinfo->updreq)))
      {
        reqinfo->failure_reason = REQ_NOUPDATE;  /* No update available    */
        return 0;               /* Go on */
      }
    }

    /* Check file size */
    if ((status = freq_abort (st.st_size, reqinfo->calltime)) != 0)
    {
      if (status == 1)          /* Yes, why? Carrier?     */
        return -1;              /* uh-huh -- get out fast */
      else                      /* make .RSP file but continue with next match */
        return make_RSP (reqinfo->request, status, reqinfo->callback);
    }

    /* Everything is OK, send the file if we can */

    CLEAR_INBOUND ();
    switch ((*reqinfo->callback) (file))
    {
    case FILE_SENT_OK:
      status = 1;
      reqinfo->failure_reason = REQ_SUCCESS;
      freq_accum.bytes += st.st_size;
      freq_accum.CumTime += xfer_seconds;
      ++freq_accum.files;
      break;

    case FILE_SKIPPED:
      status = 0;
      reqinfo->failure_reason = REQ_SUCCESS;
      break;

    default:
      status = -1;
    }
  }
  else
    status_line ("!about to send '%s', but it's not there!", file);

  return status;
}


/*--------------------------------------------------------------*/
/* generic index search routine                                 */
/*--------------------------------------------------------------*/
/* search via a simple index.                                   */
/* simple means: index for files, dat-file with areainfo,       */
/*                 optional: index file for areainfo (Max 3.0)  */

static int LOCALFUNC
search_via_simple_index (struct _reqinfo *reqinfo,
                         int sorted,  /* index is sorted       */
                         long (LOCALFUNC * get_idx_structlen) (FILE *),  /* get index recordlen   */
                         long (LOCALFUNC * get_dat_structlen) (FILE *),  /* get areafile recordln */
                         char *(LOCALFUNC * get_idx_fname) (void *),  /* get filename from idx */
                  long (LOCALFUNC * get_idx_areaofs) (void *, long, FILE *),  /* get areaofs from index */
                  char *(LOCALFUNC * get_dat_heap) (FILE *, char *, void *),  /* get area heap         */
          char *(LOCALFUNC * get_filepath) (char *, void *, void *, char *),  /* get filepath    */
                         int (LOCALFUNC * privok) (void *, char *))  /* security check        */
{
  struct stat st;
  unsigned int fmaxstep = 30;   /* step width searching sorted index        */
  char fname[13];
  char our_wildcard[15];
  char *idxbuf, *datbuf;        /* buffer for index record and area record  */
  char *heap = NULL;            /* additional info for area records         */
  FILE *fidx;                   /* file handle for index file               */
  FILE *adat;                   /* file handle for area file                */
  FILE *aidx;                   /* file handle for optional area index file */
  char s2[PATHLEN];
  int status = 0;
  int filesent = 0;             /* # of files sent for the current request  */
  long idx_slen;                /* length of index record                   */
  long dat_slen;                /* length of dat record                     */
  long fmaxpos = 0L;            /* current index into index                 */
  long fmaxmax = 0L;            /* number of entries in index               */
  int i;

  if (!fri_areadat)
  {
    status_line ("!request index specified, but no areafile");
    return 0;
  }

  /* try to open file indexfile */
  fidx = share_fopen (reqinfo->okfilepath, read_binary, DENY_NONE);
  if (fidx == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), reqinfo->okfilepath);
    return 0;
  }

  /* try to open areafile */
  adat = share_fopen (fri_areadat, read_binary, DENY_NONE);
  if (adat == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), fri_areadat);
    fclose (fidx);
    return 0;
  }

  /* try to open area indexfile, only if specified */
  if (fri_areaidx)
  {
    aidx = share_fopen (fri_areaidx, read_binary, DENY_NONE);
    if (aidx == NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), fri_areaidx);
      fclose (fidx);
      fclose (adat);
      return 0;
    }
  }
  else
    aidx = NULL;

  idx_slen = get_idx_structlen (fidx);
  dat_slen = get_dat_structlen (adat);

  idxbuf = (char *) malloc (idx_slen);
  datbuf = (char *) malloc (dat_slen);
  if (idxbuf && datbuf)
  {
    unix_stat (reqinfo->okfilepath, &st);
    fmaxmax = st.st_size / idx_slen;

    if (!sorted)
    {
      fmaxstep = 0;
      fseek (fidx, 0, SEEK_SET);
    }
    else if (fmaxmax > 5000L)
      fmaxstep = 60;

    if (abs (loglevel) >= 5)
      status_line (MSG_TXT (M_MAXFILE_SCANNING), fmaxmax, fmaxstep,
                   reqinfo->okfilepath);

    do
    {
      if (fmaxstep)
      {
        fmaxpos += fmaxstep;
        if (fmaxpos >= fmaxmax)
        {
          fmaxpos -= (long) fmaxstep;
          fmaxstep = 0;
        }

        fseek (fidx, fmaxpos * idx_slen, SEEK_SET);
      }

      if ((status = check_timed_abort (reqinfo)) != 1)  /* 1 = ok */
        break;

      /* reset status, no files sent this iteration (yet) */
      status = 0;

      /* get index entry from index file */
      fread (idxbuf, sizeof (char), (unsigned) idx_slen, fidx);

      if (ferror (fidx))
        break;

      /* get filename from index entry */
      strntcpy (fname, get_idx_fname (idxbuf), 13);

      /* check if it matches */
      if (prep_match (fname, our_wildcard) < 0)
        continue;

      i = match (our_wildcard, reqinfo->their_wildcard);
      if (i < 0)                /* position not yet reached */
        continue;
      else
      {
        if (fmaxstep)           /* go back and search each file */
        {
          fmaxpos -= (long) fmaxstep;
          fmaxstep = 0;
          fseek (fidx, fmaxpos * idx_slen, SEEK_SET);
          continue;
        }
        else if (i > 0)
        {
          if (!sorted)
            continue;
          else
            break;
        }
      }

      /* position in index reached, send found files */
      fmaxstep = 0;

      /* get area info */
      fseek (adat, (long) get_idx_areaofs (idxbuf, dat_slen, aidx), SEEK_SET);
      fread (datbuf, sizeof (char), dat_slen, adat);

      if (get_dat_heap)
      {
        heap = get_dat_heap (adat, heap, datbuf);
        if (heap == NULL)
        {
          status_line ("!unable to allocate memory for processing request index (heap)");
          continue;
        }
      }

      /* check security */
      i = privok (datbuf, heap);
      if (i != 0)
      {
        reqinfo->failure_reason = i;
        continue;
      }

      /* get file path to file */
      get_filepath (s2, idxbuf, datbuf, heap);

      /* Append the actual file name */
      strcat (s2, fname);

      /* send the file */
      if ((status = send_file (s2, reqinfo)) > 0)
        filesent++;
    }
    while (status != -1);
  }
  else
    status_line ("!unable to allocate memory for processing request index ");

  fclose (fidx);
  fclose (adat);
  if (aidx)
    fclose (aidx);

  if (heap)
    free (heap);
  if (idxbuf)
    free (idxbuf);
  if (datbuf)
    free (datbuf);

  return status == -1 ? -1 : filesent;
}


/*--------------------------------------------------------------*/
/* Check MAXFILES.IDX                                           */
/*--------------------------------------------------------------*/
static int LOCALFUNC
process_maximus_index (struct _reqinfo *reqinfo)
{
  if (fri_areaidx)              /* Maximus 3.0 */
    return search_via_simple_index (reqinfo,
                                    1,  /* index is sorted */
                                    max2_get_idx_structlen,  /* get index structlen   */
                                    max3_get_dat_structlen,  /* get areafile structlen */
                                    max2_get_idx_fname,  /* get filename from index   */
                                    max3_get_idx_areaofs,  /* get area-ofs from index */
                                    max3_get_dat_heap,  /* get area heap */
                                    max3_get_filepath,  /* get filepath  */
                                    max3_privok);  /* security check     */

  else                          /* Maximus 2.0 */
    return search_via_simple_index (reqinfo,
                                    1,  /* index is sorted */
                                    max2_get_idx_structlen,  /* get index structlen   */
                                    max2_get_dat_structlen,  /* get areafile structlen */
                                    max2_get_idx_fname,  /* get filename from index   */
                                    max2_get_idx_areaofs,  /* get area-ofs from index */
                                    0,  /* no heap */
                                    max2_get_filepath,  /* get filepath */
                                    max2_privok);  /* security check    */
}

/*--------------------------------------------------------------*/
/* helper functions for Maximus 2.0 */
/*--------------------------------------------------------------*/

static long LOCALFUNC
max2_get_idx_structlen (FILE * fidx)
{
  fidx = fidx;
  return (long) sizeof (struct _maxfile);
}

static long LOCALFUNC
max2_get_dat_structlen (FILE * adat)
{
  struct _maxarea maxarea;

  /* Read the first record of the file, to grab the structure-length byte. */
  fread (&maxarea, sizeof (struct _maxarea), 1, adat);

  return (long) maxarea.struct_len;
}

static char *LOCALFUNC
max2_get_idx_fname (void *idxbuf)
{
  struct _maxfile *maxfile = (struct _maxfile *) idxbuf;

  return (char *) maxfile->name;
}

static long LOCALFUNC
max2_get_idx_areaofs (void *idxbuf, long dat_slen, FILE * aidx)
{
  struct _maxfile *maxfile = (struct _maxfile *) idxbuf;

  aidx = aidx;
  return (long) maxfile->areaofs * dat_slen;
}

static char *LOCALFUNC
max2_get_filepath (char *s, void *idxbuf, void *datbuf, char *heap)
{
  struct _maxfile *maxfile = (struct _maxfile *) idxbuf;
  struct _maxarea *maxarea = (struct _maxarea *) datbuf;

  heap = heap;

  strntcpy (s, (char *) maxarea->filepath, 80);
  GetPathFromFilesDat (s, (char *) maxarea->filesbbs, maxfile->fileofs, (char *) maxfile->name);
  return s;
}

static int LOCALFUNC
max2_privok (void *datbuf, char *heap)
{
  struct _maxarea *maxarea = (struct _maxarea *) datbuf;

  heap = heap;

  if (CURRENT.security < maxarea->filepriv)
  {
    if (abs (loglevel) >= 5)
      status_line (MSG_TXT (M_MAX_SECURITY_MISMATCH), (int) CURRENT.security,
                   (int) maxarea->filepriv);
    return REQ_WRONGPWD;
  }

  return 0;
}


/* AW 971206 find path to file via Maximus 2.0/3.0 files.dat/.dmp */
static void LOCALFUNC
GetPathFromFilesDat (char *filepath, char *filesbbs, word fileofs,
                     char *filename)
{
  char s1[PATHLEN];
  char *p;
  FILE *dat;
  FILE *dmp;
  long datlen;
  FDAT fdat;

  /* get path to <files>.dat into s1 */

  if (filesbbs[0])              /* special path to files.bbs? */
  {                             /* yes */
    strntcpy (s1, filesbbs, PATHLEN);
    p = strrchr (s1, DIR_SEPC);
    if (!p)
      p = s1;
    p = strrchr (p, '.');
    if (p)
      (*p) = '\0';
  }
  else
  {                             /* no, use path of area */
    strntcpy (s1, filepath, PATHLEN);
    strcat (s1, "files");
    p = &(s1[strlen (s1)]);
  }

  /* open files.dat */
  strcpy (p, ".dat");
  // status_line(">GetPathFromFilesDat: files.dat = \"%s\"", s1);
  dat = share_fopen (s1, read_binary, DENY_NONE);
  if (dat == NULL)
    return;

  /* get first record for length */
  if (fread (&fdat, sizeof (FDAT), 1, dat) != 1)
  {
    fclose (dat);
    return;
  }

  datlen = (long) fdat.struct_len;

  /* get record of requested file */
  fseek (dat, (long) fileofs * datlen, SEEK_SET);
  if (fread (&fdat, sizeof (char), datlen, dat) != datlen)
  {
    fclose (dat);
    return;
  }

  fclose (dat);

  // status_line(">GetPathFromFilesDat: comparing \"%s\" with \"%s\"", fdat.name, filename);
  if (strnicmp ((char *) fdat.name, filename, 12))
    return;

  if (fdat.path == 0)           /* no extra path in files.dmp */
    return;

  /* get offset in files.dmp */

  /* open files.dmp */
  strcpy (p, ".dmp");
  // status_line(">GetPathFromFilesDat: files.dmp = \"%s\"", s1);
  dmp = share_fopen (s1, read_binary, DENY_NONE);
  if (dmp == NULL)
    return;

  /* get filepath from files.dmp */
  fseek (dmp, fdat.path + 2, SEEK_SET);
  fread (filepath, sizeof (char), PATHLEN, dmp);

  filepath[PATHLEN - 1] = '\0';
  fclose (dmp);
  // status_line(">GetPathFromFilesDat: new path = \"%s\"", filepath);

  return;
}




/*--------------------------------------------------------------*/
/* Check FILESIDX.PRO                                           */
/*--------------------------------------------------------------*/
static int LOCALFUNC
process_proboard_index (struct _reqinfo *reqinfo)
{
  return search_via_simple_index (reqinfo,
                                  0,  /* index is not sorted */
                                  pb_get_idx_structlen,  /* get index structlen   */
                                  pb_get_dat_structlen,  /* get areafile structlen */
                                  pb_get_idx_fname,  /* get filename from index   */
                                  pb_get_idx_areaofs,  /* get area-ofs from index */
                                  0,  /* no heap */
                                  pb_get_filepath,  /* get filepath */
                                  pb_privok);  /* security check    */
}

/*--------------------------------------------------------------*/
/* helper functions for Proboard                                */
/*--------------------------------------------------------------*/

static long LOCALFUNC
pb_get_idx_structlen (FILE * fidx)
{
  fidx = fidx;
  return sizeof (struct _pbfile);
}

static long LOCALFUNC
pb_get_dat_structlen (FILE * adat)
{
  adat = adat;
  return sizeof (struct _pbarea);
}

static char *LOCALFUNC
pb_get_idx_fname (void *idxbuf)
{
  struct _pbfile *pbfile = (struct _pbfile *) idxbuf;

  return (char *) pbfile->name;
}

static long LOCALFUNC
pb_get_idx_areaofs (void *idxbuf, long dat_slen, FILE * aidx)
{
  struct _pbfile *pbfile = (struct _pbfile *) idxbuf;

  aidx = aidx;
  return (long) (pbfile->area - 1) * dat_slen;
}

static char *LOCALFUNC
pb_get_filepath (char *s, void *idxbuf, void *datbuf, char *heap)
{
  struct _pbarea *pbarea = (struct _pbarea *) datbuf;

  idxbuf = idxbuf;
  heap = heap;

  strntcpy (s, (char *) pbarea->filepath, 80);

  if (s[strlen (s) - 1] != DIR_SEPC && s[strlen (s) - 1] != ':')
    strcat (s, DIR_SEPS);

  return s;
}

static int LOCALFUNC
pb_privok (void *datbuf, char *heap)
{
  struct _pbarea *pbarea = (struct _pbarea *) datbuf;

  heap = heap;

  if (CURRENT.security < pbarea->filepriv)
  {
    if (abs (loglevel) >= 5)
      status_line (MSG_TXT (M_MAX_SECURITY_MISMATCH), (int) CURRENT.security,
                   (int) pbarea->filepriv);
    return REQ_WRONGPWD;
  }

  return 0;
}


/*--------------------------------------------------------------*/
/* Maximus 3.0 support                                          */
/*--------------------------------------------------------------*/
/* interpreting a Maximus 3.0 ACS

   ACS        :== simpleACS | simpleACS ACSop ACS
   simpleACS  :== PrivKeyOp | UserSpec
   PrivKeyOp  :== PrivKey | Op PrivKey
   PrivKey    :== PrivSpec | PrivSpec "/" Keylist
   Keylist    :== Key | NotKey | Key Keylist | NotKey Keylist
   UserSpec   :== Name | Alias
   Name       :== "name=" (string)
   Alias      :== "alias=" (string)
   PrivSpec   :== Priv | Class
   Class      :== (string)
   Priv       :== (number)
   Op         :== "=" | ">" | "<" | ">=" | "<=" | "<>" | "!="
   NotKey     :== "!" Key
   Key        :== "0".."9" | "A".."Z"
   ACSOp      :== "|" | "&"
 */


/* destroy old classname list */
static struct _max_classname *LOCALFUNC
free_max_classname (struct _max_classname *list)
{
  int i;

  if (list && list != fri_max2_classname)
  {
    for (i = 0; list[i].classname; i++)
      free (list[i].classname);
    free (list);
  }

  return NULL;
}


static bool LOCALFUNC
get_max_classname ()
{
  struct _clsrec clsrec;
  struct _clshdr clshdr;
  FILE *uclass;
  int i, j;
  char *heap;

  fri_max_classname = free_max_classname (fri_max_classname);

  if (!fri_uclass)              /* we have Maximus 2.x */
  {
    fri_max_classname = fri_max2_classname;
    return 1;
  }

  /* open access.dat */
  uclass = share_fopen (fri_uclass, read_binary, DENY_NONE);
  if (uclass == NULL)
  {
    status_line ("!error opening %s", fri_uclass);
    return 0;
  }

  /* read header of access.dat */
  if (fread (&clshdr, sizeof (struct _clshdr), 1, uclass) != 1)
  {
    status_line ("!error reading header of %s", fri_uclass);
    fclose (uclass);
    return 0;
  }

  /* check, if it is really an access.dat file */
  if (clshdr.ulclhid != CLS_ID)
  {
    status_line ("!%s does not seem to be an access.dat style file", fri_uclass);
    fclose (uclass);
    return 0;
  }

  /* read heap temporarily */
  heap = (char *) malloc (clshdr.usstr);
  if (!heap)
  {
    status_line ("!out of memory reading %s", fri_uclass);
    fclose (uclass);
    return 0;
  }

  /* seek to begin of the heap and read it */
  fseek (uclass, (long) clshdr.usclfirst + (long) clshdr.ussize * (long) clshdr.usn, SEEK_SET);
  fread (heap, sizeof (char), clshdr.usstr, uclass);

  /* preallocate entries in our classname list      */
  /* 2*clshdr.usn, for zAbbrev and potential zAlias */
  fri_max_classname = (struct _max_classname *) malloc (sizeof (struct _max_classname) * ((clshdr.usn * 2) + 1));

  for (i = 0, j = 0; i < clshdr.usn; i++, j++)
  {
    fseek (uclass, (long) clshdr.usclfirst + (long) clshdr.ussize * (long) i, SEEK_SET);
    fread (&clsrec, sizeof (struct _clsrec), 1, uclass);

    fri_max_classname[j].classname = strdup (heap + clsrec.zAbbrev);
    if (!fri_max_classname[j].classname)
    {
      status_line ("!out of memory reading %s", fri_uclass);
      fri_max_classname = free_max_classname (fri_max_classname);
      free (heap);
      fclose (uclass);
      return 0;
    }
    fri_max_classname[j].priv = clsrec.usLevel;

    if (clsrec.zAlias)          /* AW992030 add alias definition, if present */
    {
      ++j;
      fri_max_classname[j].classname = strdup (heap + clsrec.zAlias);
      if (!fri_max_classname[j].classname)
      {
        status_line ("!out of memory reading %s", fri_uclass);
        fri_max_classname = free_max_classname (fri_max_classname);
        free (heap);
        fclose (uclass);
        return 0;
      }
      fri_max_classname[j].priv = clsrec.usLevel;
    }
  }

  fri_max_classname[j].classname = NULL;
  free (heap);
  fclose (uclass);
  return 1;
}

/* NOT static LOCALFUNC */
void
get_PrivKey (word * ppriv, dword * pkeys, char *PrivKey)
{
  char *tmp, *p;

  if (!PrivKey)
  {
    *ppriv = 65535;
    *pkeys = 0;
    return;
  }

  tmp = strdup (PrivKey);

  p = strchr (tmp, '/');
  if (p)                        /* flags present */
  {
    *p = '\0';
    *pkeys = parse_flags (++p);
  }

  *ppriv = class_to_priv (tmp);
  free (tmp);
}


static dword LOCALFUNC
parse_flags (char *flags)
{
  dword xkeys = 0;

  if (!flags)
    return 0;

  while (*flags)
    xkeys |= key2bit (*(flags++));

  return xkeys;
}


static word LOCALFUNC
class_to_priv (char *class)
{
  word priv;
  int i;

  if (class && *class)
  {
    if (sscanf (class, "%hu", &priv))  /* already a number */
      return priv;

    /* now in class we have a string class name. Let's search the
       Maximus 3.0 access.dat for this class name */

    /* have we already read the list? For Max 2.x the internal list is used */
    if (!fri_max_classname)
    {
      if (!get_max_classname ())
        return 65535;
    }

    for (i = 0; fri_max_classname[i].classname != NULL; i++)
    {
      if (!stricmp (class, fri_max_classname[i].classname))
        return fri_max_classname[i].priv;
    }
  }

  status_line ("!Class '%s' not in classnames list, check FileSec/KnownSec/ProtSec", class ? class : "(unspecified)");
  return 65535;
}

static bool LOCALFUNC
Keylist (dword xkeys, char *testKeylist)
{
  char *p = testKeylist;
  bool result = 1;

  while (*p)
  {
    result = result && ((*p == '!' && *(++p)) ?
                        NotKey (xkeys, *p) :
                        Key (xkeys, *p));
    p++;
  }

  return result;
}


static bool LOCALFUNC
PrivKeyOp (word priv, dword xkeys, enum _privcmp op, char *testPrivKey)
{
  char *p;
  word acspriv;

  if (!(*testPrivKey))          /* empty ACS: false */
    return 0;

  if ((p = strchr (testPrivKey, '/')) != NULL)  /* flags present? */
  {
    *p = '\0';
    if (!Keylist (xkeys, ++p))  /* but did not match */
      return 0;
  }

  acspriv = class_to_priv (testPrivKey);  /* get priv from class */

  if (acspriv == 65535)         /* priv not found?     */
    return 0;

  switch (op)
  {
  case privGE:
    return priv >= acspriv ? 1 : 0;

  case privLE:
    return priv <= acspriv ? 1 : 0;

  case privGT:
    return priv > acspriv ? 1 : 0;

  case privLT:
    return priv < acspriv ? 1 : 0;

  case privEQ:
    return priv == acspriv ? 1 : 0;

  case privNE:
    return priv != acspriv ? 1 : 0;

  default:
    return 0;
  }
}


static bool LOCALFUNC
NameAlias (char *name, char *testname)
{
  char *p;

  if (!(*testname))
    return 0;

  if (!name)
    return 0;

  while ((p = strchr (testname, '_')) != NULL)
    *p = ' ';

  return stricmp (name, testname) ? 0 : 1;
}


static bool LOCALFUNC
simpleACS (word priv, dword xkeys, char *name, char *alias, char *testsimpleACS)
{
  if (!(*testsimpleACS))        /* empty ACS: false */
    return 0;

  if (!strnicmp (testsimpleACS, "name=", 5))  /* name= style ACS */
    return NameAlias (name, testsimpleACS + 5);

  if (!strnicmp (testsimpleACS, "alias=", 6))  /* alias= style ACS */
    return NameAlias (alias, testsimpleACS + 6);

  if (!strncmp (testsimpleACS, "==", 2))
    return PrivKeyOp (priv, xkeys, privEQ, testsimpleACS + 2);

  if (!strncmp (testsimpleACS, "<=", 2))
    return PrivKeyOp (priv, xkeys, privLE, testsimpleACS + 2);

  if (!strncmp (testsimpleACS, "<>", 2))
    return PrivKeyOp (priv, xkeys, privNE, testsimpleACS + 2);

  if (!strncmp (testsimpleACS, "!=", 2))
    return PrivKeyOp (priv, xkeys, privNE, testsimpleACS + 2);

  if (!strncmp (testsimpleACS, ">=", 2))
    return PrivKeyOp (priv, xkeys, privGE, testsimpleACS + 2);

  if (!strncmp (testsimpleACS, "=", 1))
    return PrivKeyOp (priv, xkeys, privEQ, testsimpleACS + 1);

  if (!strncmp (testsimpleACS, "<", 1))
    return PrivKeyOp (priv, xkeys, privLT, testsimpleACS + 1);

  if (!strncmp (testsimpleACS, ">", 1))
    return PrivKeyOp (priv, xkeys, privGT, testsimpleACS + 1);

  return PrivKeyOp (priv, xkeys, privGE, testsimpleACS);
}


/* NOT static LOCALFUNC */
bool
ACS (word priv, dword xkeys, char *name, char *alias, char *acs)
{
  char *p;
  char ACSOp;
  bool privok1, privok2;

  if (!acs)                     /* no ACS -> give access */
    return 1;

  while (isspace (acs[0]))      /* skip leading spaces */
    ++acs;

  p = acs + strlen (acs) - 1;
  while ((p >= acs) && isspace (*p))  /* chop off trailing spaces */
  {
    *p = '\0';
    --p;
  }

  if (!(*acs))                  /* empty ACS -> give access */
    return 1;

  /* find concatenation chars of simpleACS */
  if ((p = strpbrk (acs, "|&")) != NULL)
  {
    ACSOp = *p;
    *(p++) = '\0';
    privok1 = simpleACS (priv, xkeys, name, alias, acs);
    privok2 = ACS (priv, xkeys, name, alias, p);
    return ACSOp == '|' ? privok1 || privok2 : privok1 && privok2;
  }

  return simpleACS (priv, xkeys, name, alias, acs);
}


static dword LOCALFUNC
key2bit (char key)
{
  key = toupper (key);
  if (key >= '0' && key <= '8')
    return 1 << (key - '0');
  else if (key >= 'A' && key <= 'X')
    return 1 << (key - 'A');
  else
    return 0;
}

static bool LOCALFUNC
Key (dword xkeys, char key)
{
  return (xkeys & key2bit (key)) ? 1 : 0;
}

static bool LOCALFUNC
NotKey (dword xkeys, char key)
{
  return Key (~xkeys, key);
}



/*--------------------------------------------------------------*/
/* helper functions for Maximus 3.0                             */
/*--------------------------------------------------------------*/

static long LOCALFUNC
max3_get_dat_structlen (FILE * adat)
{
  struct _filearea farea;

  /* Read the first record of the file, to grab the structure-length byte. */
  fseek (adat, ADATA_START, SEEK_SET);
  fread (&farea, sizeof (struct _filearea), 1, adat);

  return (long) farea.cbArea;
}

static long LOCALFUNC
max3_get_idx_areaofs (void *idxbuf, long dat_slen, FILE * aidx)
{
  struct _maxfile *maxfile = (struct _maxfile *) idxbuf;
  struct _mfidx aidxbuf;

  dat_slen = dat_slen;

  /* seek in area index file */
  fseek (aidx, (long) maxfile->areaofs * (long) sizeof (struct _mfidx), SEEK_SET);
  fread (&aidxbuf, sizeof (struct _mfidx), 1, aidx);

  return (long) aidxbuf.ofs;
}

static char *LOCALFUNC
max3_get_dat_heap (FILE * adat, char *heap, void *datbuf)
{
  struct _filearea *farea = (struct _filearea *) datbuf;

  /* free previously allocated heap */
  if (heap != NULL)
    free (heap);

  /* get new memory for heap */
  heap = (char *) malloc (farea->cbHeap);
  if (heap == NULL)
    return NULL;

  /* read over overrides */
  fseek (adat, (long) farea->num_override * (long) sizeof (struct _ovride), SEEK_CUR);
  fread (heap, sizeof (char), farea->cbHeap, adat);

  return heap;
}


static char *LOCALFUNC
max3_get_filepath (char *s, void *idxbuf, void *datbuf, char *heap)
{
  struct _maxfile *maxfile = (struct _maxfile *) idxbuf;
  struct _filearea *farea = (struct _filearea *) datbuf;

  strntcpy (s, heap + farea->downpath, PATHLEN);
  GetPathFromFilesDat (s, (char *) heap + farea->filesbbs, maxfile->fileofs, (char *) maxfile->name);
  return s;
}

static int LOCALFUNC
max3_privok (void *datbuf, char *heap)
{
  struct _filearea *farea = (struct _filearea *) datbuf;

  if (CURRENT.security == -1 ||
      !ACS ((word) CURRENT.security, CURRENT.xkeys, remote_sysop, NULL,
            heap + farea->acs))
  {
    if (abs (loglevel) >= 5)
      status_line ("!Security mismatch: his='%s', required acs='%s'",
                   CURRENT.class ? CURRENT.class : "(unspecified)",
                   heap + farea->acs);
    return REQ_WRONGPWD;
  }

  return 0;
}

/* $Id: b_frproc.c,v 1.15 1999/07/30 17:37:18 ceh Exp $ */
