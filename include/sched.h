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
 * Filename    : $Source: E:/cvs/btxe/include/sched.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:11 $
 * State       : $State: Exp $
 *
 * Description : Scheduler definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Definitions for day of the week */

#define DAY_SUNDAY      0x01
#define DAY_MONDAY      0x02
#define DAY_TUESDAY     0x04
#define DAY_WEDNESDAY   0x08
#define DAY_THURSDAY    0x10
#define DAY_FRIDAY      0x20
#define DAY_SATURDAY    0x40
#define DAY_UNUSED      0x80

#define DAY_WEEK    (DAY_MONDAY|DAY_TUESDAY|DAY_WEDNESDAY|DAY_THURSDAY|DAY_FRIDAY)
#define DAY_WKEND   (DAY_SUNDAY|DAY_SATURDAY)
#define DAY_ALL     (DAY_WEEK|DAY_WKEND)  /* VRP 990829 */

/* Definitions for matrix behavior */

#define MAT_CM          0x0001  /* 'C': send C?? mail only                */
#define MAT_DYNAM       0x0002  /* 'D': dynamic event                     */
#define MAT_BBS         0x0004  /* 'B': BBS allowed, human users welcome  */
#define MAT_NOREQ       0x0008  /* 'N': dont satisfy incoming requests    */
#define MAT_OUTONLY     0x0010  /* 'S': outbound mail only                */
#define MAT_NOOUT       0x0020  /* 'R': no outgoing calls allowed         */
#define MAT_FORCED      0x0040  /* 'F': event is forced                   */
#define MAT_LOCAL       0x0080  /* 'L': local calls only                  */
#define MAT_SKIP        0x0100  /* internal: end of dynamic event reached */
#define MAT_NOMAIL24    0x0200  /* 'M': mailable 24 hours, call CM & !CM  */
#define MAT_NOOUTREQ    0x0400  /* 'X': No outgoing requests              */
#define MAT_NOCM        0x0800  /* 'K': do not send to CM nodes           */
#define MAT_HIPRICM     0x1000  /* 'H': send high priority crashmail only */
#define MAT_KILLBAD     0x2000  /* '$': Kill *.$$? packets                */
#define MAT_COSTEVENT   0x4000  /* '!': 0 = normal event, 1 = CostEvent   */
#define MAT_NOPICKUP    0x8000  /* 'P': outgoing calls: no pickup, send only */

#define EXTR_NOSOUND     0x0001 /* 'Y': no sounds allowed      r. hoerner */

#define EXTR_LRELMASK    0x0006 /* Bits 1 and 2 are for different l relations */
#define EXTR_LEQUALS     0x0002 /* L=xxx                                      */
#define EXTR_LLESS       0x0004 /* L<xxx                                      */
#define EXTR_LGREATER    0x0006 /* L>xxx                                      */

#define EXTR_LBHVMASK    0x0018 /* Bits 3 and 4 are for different l behaviour */
#define EXTR_LINDEXCOST  0x0008 /* li flag: use cost index                    */
#define EXTR_LMINUTECOST 0x0010 /* lc flag: use cost[cost index] (1 minute)   */
#define EXTR_LTOTALCOST  0x0018 /* lt flag: use cost[cost index] * estim.time */


/**********************************************************************
 * If either of these structures are changed, don't forget to change  *
 * the BinkSched string in sched.c, as well as the routines that read *
 * and write the schedule file (read_sched, write_sched)!!!           *
 **********************************************************************/

typedef struct _bink_event
{
  short minute;                 /* Start of Event, minutes past 12AM */
  short length;                 /* Number of minutes event runs      */
  short behavior;               /* Behavior mask                     */
  short wait_time;              /* Avg seconds to wait between dials */
  short node_cost;              /* Max cost node to call, this event */
  short with_connect;           /* Number of calls to make w/carrier */
  short no_connect;             /* Number of calls to make w/o DCD   */
  unsigned char days;           /* Bit field for days to execute     */
  long last_ran_date;           /* TJW 960804 bugfix: custom date
                                 * event last ran (was: day of month)*/
  unsigned long mailqsize;      /* Mail amount needed to make call   */
  unsigned char errlevel[9];    /* Errorlevel exits                  */

  char cmd[32];                 /* Chars to append to packer,        *
                                 * aftermail and cleanup             */
  char month;                   /* Month when to do it               */
  char day;                     /* Day of month to do it             */
  char err_extent[6][4];        /* 3 byte ext's for errlvls 4-9      */
  short costvector[32];         /* CostEvent: 32 costvector entries  */
  unsigned char faxerrlevel;    /* Exit for fax reception            */
  short extramask;              /* Extra Behavior mask               */
  unsigned char extra;          /* Extra space for later             */
  char ename[32];               /* name string of event              */
}
BINK_EVENT, *BINK_EVENTP;

typedef struct _history
{
  short total_errors;           /* CEH/971005                          *
                                 * was: replacement for old which_date *
                                 * now: number of !-lines in logfile   */
  short bbs_calls;              /* Number of BBS callers             */
  short mail_calls;             /* Number of mail calls              */
  short calls_made;             /* Number of outgoing calls made     */
  short connects;               /* Number of outbound call successes */
  short files_in;               /* Number of files received          */
  short files_out;              /* Number of files sent              */
  short last_caller;            /* Type of last call                 */
  ADDR last_addr;               /* Address of last, excl. Domain     */
  char last_Domain[32];         /* Domain of last                    */
  long last_Elapsed;            /* Time of last outbound session     */
  ADDR next_addr;               /* Address of next, excl. Domain     */
  char next_Domain[32];         /* Domain of next                    */
  long callcost;                /* Cumulative of call costs          */
  long size_in;                 /* Size of files received            */
  long time_in;                 /* Time of files received            */
  long err_in;                  /* Errors while receiving files      */
  long size_out;                /* Size of files sent                */
  long time_out;                /* Time of files sent                */
  long err_out;                 /* Errors while sending files        */
  long which_date;              /* Custom date for this record       */
  short fax_calls;              /* Number of FAX calls               */
  char last_str[32];            /* string for "Last: ..." display    */// CEH 980803:
  /* if you change this, so that last_str is not the last item, please
   * also change sched.c (line 166) */
}
HISTORY, *HISTORYP;

/* $Id: sched.h,v 1.3 1999/09/27 20:51:11 mr Exp $ */
