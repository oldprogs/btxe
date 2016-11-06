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
 * Filename    : $Source: E:/cvs/btxe/include/defines.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:06 $
 * State       : $State: Exp $
 *
 * Description : Major definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#ifndef TRUE
#define TRUE  1                 /* Watcom DOS needs it */
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define SUCCESS          0
#define SUCCESS_EOT     -1
#define OPEN_ERR        -1
#define GETBLOCK_ERR    -2
#define SENDBLOCK_ERR   -3
#define SEND_TIMEOUT    -4
#define END_OF_FILE     -5
#define SEND_RETRY_ERR  -6
#define SYSTEM_ERR      -7
#define DATA_STREAM_ERR -8
#define CARRIER_ERR     -9
#define FNAME_ERR       -10

#define BUSY_ERR        -11
#define BAD_LINE_ERR    -12
#define VOICE_ERR       -13
#define DIAL_TONE_ERR   -14
#define MODEM_ERR       -15

#define TIME_ERR        -16
#define WHACK_CR_ERR    -17
#define XMODEM_ERR      -18
#define SENDFILE_ERR    -19
#define STREAM_ERR      -20
#define XMODEM_REC_ERR  -21
#define NO_CR_ERR       -22
#define EOT_RECEIVED    -23
#define STAT_ERR        -24
#define START_TIMEOUT   -25
#define NO_CARRIER_ERR  -26
#define CONN_COUNT_ERR  -27
#define NO_CONN_COUNT_ERR -28
#define NO_CALL         -29
#define READ_ERR        -30
#define OUR_MSG_ERR     -31
#define MSG_SENT_ERR    -32
#define MLO_ERR         -33
#define PICKUP_DONE     -34
#define DEV_ERR         -35
#define KBD_ERR         -36
#define CRC_ERR         -37
#define RESYNC_ERR      -38
#define TOO_MANY_ERRORS -39

#define STREAM_CHARS  12000
#define CRC           1
#define CHECKSUM      0
#define UNKNOWN      -1
#define WANTCRC      'C'

#define EOT_BLOCK     1
#define TELINK_BLOCK  2
#define DATA_BLOCK    3
#define XMODEM_BLOCK  3
#define BAD_BLOCK     4
#define SHORT_BLOCK   5
#define DUP_BLOCK     6
#define SEALINK_BLOCK 7
#define NO_BLOCK      10

#define XS0 2
#define XS0T 3
#define XS1 4
#define XS2 5
#define XS3 6

#define MS0 2
#define MS1 3
#define MS2 4

#define S0 1
#define S1 2
#define S2 3
#define S3 4
#define S4 5
#define S5 6
#define S6 7
#define S7 8
#define S8 9

#define XR0 2
#define XR0B 3
#define XR1 4
#define XR2 5
#define XR3 6
#define XR4 7
#define XR5 8

#define MR0 2
#define MR1 3
#define MR2 4
#define MR3 5

#define R0 1
#define R1 2
#define R2 3
#define R3 4
#define R4 5
#define R5 6
#define R6 7
#define R7 8

#define BSND0 2
#define BSND1 3
#define BSND2 4
#define BSND3 5
#define BSND4 6

#define BR0 2
#define BR1 3
#define BR2 4
#define BR3 5
#define BR4 6

#define W0 1
#define W1 2
#define W2 3
#define W3 4
#define W4 5
#define W5 6

#define SR0 1
#define SR1 2
#define SR2 3

#define AC0 2
#define AC1 3
#define AC2 4
#define AC3 5
#define AC4 6
#define AC5 7
#define AC6 8
#define AC7 9
#define AC8 10
#define AC9 11
#define AC10 12

#define SB0 2
#define SB1 3
#define SB2 4
#define SB3 5

#define RB0 2
#define RB1 3
#define RB2 4
#define RB3 5
#define RB4 6

#define SA0 2
#define SA1 3
#define SA2 4
#define SA3 5

#define SN0 2
#define SN1 3
#define SN2 4
#define SN3 5

#define SEND_ONLY          1
#define RECEIVE_ONLY       2

#define INITIALIZE         0
#define TEST_AND_SET       1
#define CLEAR_FLAG         2
#define SET_SESSION_FLAG   3
#define CLEAR_SESSION_FLAG 4

/* MR 961122 Options for BBS and Extern */

#define OPT_NONE  0
#define OPT_EXIT  1
#define OPT_BATCH 2
#define OPT_SPAWN 3
#define OPT_INTERNAL 4

#define UNDEFINED        0

#define CALLED_FTSC      1      /* sendsync/recvsync */
#define CALLED_YOOHOO    2
#define CALLED_EMSI      4

#define RECVED_BBS       1
#define RECVED_FTSC      2
#define RECVED_WAZOO     3
#define RECVED_EMSI      4
#define RECVED_UUCP      5

#define HYDRA_SESSION    0x0001 /* use it as bit mask! */
#define JANUS_SESSION    0x0002
#define ZMODEM_SESSION   0x0004
#define FTSC_SESSION     0x0008

#define COL_BLACK        0
#define COL_BLUE         1
#define COL_GREEN        2
#define COL_CYAN         3
#define COL_RED          4
#define COL_MAGENTA      5
#define COL_BROWN        6
#define COL_WHITE        7
#define COL_GRAY         8
#define COL_BRIGHTBLUE   9
#define COL_BRIGHTGREEN 10
#define COL_BRIGHTCYAN  11
#define COL_BRIGHTRED   12
#define COL_BRIGHTMAGENTA 13
#define COL_YELLOW      14
#define COL_BRIGHTWHITE 15

#define SBCOM_BUFLENGTH 512

#define HIST_LONG  3
#define HIST_SHORT 1

#define DAYS_ALL        0
#define DAYS_TODAY      1
#define DAYS_LAST24     2
#define DAYS_THISWEEK   3
#define DAYS_LASTWEEK   4
#define DAYS_THISMONTH  5
#define DAYS_LASTMONTH  6
#define DAYS_THISYEAR   7
#define DAYS_LASTYEAR   8
#define DAYS_FIXED      9

/* $Id: defines.h,v 1.3 1999/02/27 01:15:06 mr Exp $ */
