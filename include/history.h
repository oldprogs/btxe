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
 * Filename    : $Source: E:/cvs/btxe/include/history.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:09 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : history file definition
 *
 *---------------------------------------------------------------------------*/

/* Calltypes */

#define CT_MAILER   0
#define CT_BBS      1
#define CT_FAX      2
#define CT_EXTMAIL  9           /* MR 970508 external mailers */

/* Subclasses defined for mailer calls */
#define SC_FTS1     0           /*  TS 970403 defined name SC_FTS-1 was illegal */
#define SC_WAZOO    1
#define SC_EMSI     2
#define SC_FAILURE  9           /* MR 970504 */

/* Subclasses defined for BBS */
/* Note that each BBS type may have its own structure in the union - I haven't
   tried to define a fixed structure common for all BBS since some (such as
   RemoteAccess) provides too few information about the last caller */

#define SC_LASTCALL 0
#define SC_CALLINFO 2           /* AW 970531 */
#define SC_TELEGARD 3
#define SC_IBS      4           /* CEH 970810 */

/*** VRP 990920 start ***/

#define SAVE_EXT_OUT    1
#define SAVE_EXT_IN     2
#define SAVE_EXT_BOTH   3

/*** VRP 990920 end ***/

typedef struct
{
  /* These fields are common to all calls */
  word task;
  byte outgoing;                /* 1=outgoing call, 0=incoming call */
  byte calltype;                /* Mailer, BBS, fax */
  byte subclass;                /* Session protocol for mail connections, for example */
  time_t starttime;             /* Date/time in UNIX format */
  time_t length;                /* Length of call, in seconds */
  union
  {
    char raw[300];
    struct                      /* Mailer call */
    {
      char name[40];            /* Sysop name for mailer connections, user name for BBS */
      char system[40];          /* System name for mailer connections */
      char location[40];        /* Location of remote */
      char phone[25];           /* Phone # */
      char mailer[40];          /* Remote's mailer */
      long filesin, filesout;
      long bytesin, bytesout;
      long cpsin, cpsout, cpsall;
      long cost;
      long speed;
      ADDR address;             /* Remote's main address */
      byte passworded;          /* Session was password protected */
      /* These fields are yet unused */
      byte success;             /* Call was completed or user-aborted at our side */
    }
    m;
    struct                      /* Fax call */
    {
      char remoteid[40];
      long pages;
      long bytes;
      long speed;
      char filename[20];
      char status[20];
    }
    f;
    struct                      /* BBS call - generic type */
    {
      /* Note - all char fields expect a C-style string, it is the
         responsability of the importing program to convert them from
         Pascal-style */
      byte line;                /* This one is actually ignored */
      char name[36];
      char handle[36];
      char city[26];
      word baud;
      long times;
      char logon[6];
      char logoff[6];
      byte attribute;
    }
    b1;
    struct                      /* BBS call - Maximus 3.01  AW 970531 */
    {
      byte line;                /* This one is actually ignored */
      char name[36];            /* User's name/alias               */
      char handle[36];          /* User's alias                    */
      char city[26];            /* User's city                     */
      word baud;
      word calls;               /* Number of previous calls + 1    */
      word filesup;             /* Number of files uploaded        */
      word filesdn;             /* Number of files dnloaded        */
      word kbup;                /* kb uploaded                     */
      word kbdn;                /* kb dnloaded                     */
      word read;                /* Number of messages read         */
      word posted;              /* Number of messages posted       */
    }
    b2;
    struct                      /* LASTON.DAT  - Telegard's Last callers records */
    {
      long caller;              /* system caller number */
      char realname[37];        /* real name of caller */
      char handle[37];          /* user name of caller */
      char location[31];        /* location of caller */
      long logonspeed;          /* logon speed */
      long logontime;           /* logon time */
      unsigned char newuser;    /* was newuser? */
      byte reserved1;           /* RESERVED */
      unsigned char hidden;     /* hidden from display */
      byte node;                /* node number */
      long logofftime;          /* logoff time */
      word uploads;             /* number of uploads */
      word downloads;           /* number of downloads */
      long uk;                  /* number of upload kB */
      long dk;                  /* number of download kB */
      byte reserved2[8];        /* RESERVED */
    }
    b3;
  }
  h;
}
CALLHIST;

/* $Id: history.h,v 1.3 1999/09/27 20:51:09 mr Exp $ */
