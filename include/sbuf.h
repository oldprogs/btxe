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
 * Filename    : $Source: E:/cvs/btxe/include/sbuf.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:17 $
 * State       : $State: Exp $
 *
 * Description : Screen Buffer Definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#define WHITEBLANK      (7 << 8) | ' '

#define SB_OK           0
#define SB_ERR        (-1)

#define SB_MODE_ANSI    1
#define SB_MODE_AVATAR  2
#define SB_MODE_SIMPLE  0

/* screen buffer constants */

extern short SB_ROWS;
extern short SB_COLS;

#define SB_SIZ  (SB_ROWS * SB_COLS)

#define SB_NXTLINE(w,p) (((p) + (w)->linesize >= (w)->endbuff) ? (w)->buffer : (p) + (w)->linesize)

typedef struct
{
  unsigned short Attr;          /* Note, this is NOT the same as a *
                                 * Windows attr word               */
  short Bgd;
  unsigned char Buf[81];
  short CtrlSeqIntro;
  short Esc;
  short Fgd;
  short NewLineKludge;
  short Mode;                   /* Simple, ANSI, or Avatar */
  unsigned short SaveCol;
  unsigned short SaveRow;
  short Scroll;
  short State;                  /* Maybe run it like an FSM */
}
_ANSIStuff, *_ANSIStuffP;

/* screen character/attribute buffer element definition */

typedef struct
{
  unsigned char ch;             /* character */
  unsigned char attr;           /* attribute */
}
BYTEBUF, *BYTEBUFP;

typedef union
{
  BYTEBUF b;
  unsigned short cap;           /* character/attribute pair */
}
CELL, *CELLP;

/* screen buffer control structure */

typedef struct
{
  short row, col;               /* current position */
  CELLP bp;                     /* pointer to screen buffer array */

  /* changed region per screen buffer row */

  short *lcol;                  /* left end of changed region */
  short *rcol;                  /* right end of changed region */

  unsigned int flags;           /* buffer status */
}
BUFFER, *BUFFERP;

/* buffer flags values */

#define SB_DELTA    0x0001
//#define SB_RAW    0x0002        TJW: not used !?
//#define SB_DIRECT 0x0004        TJW: not used !?
#define SB_SCROLL   0x0008
//#define SB_SYNC   0x0010        TJW: not used !?
#define SB_WRAP     0x0020      /* MR 970307 auto-wrap at end of line */
#define SB_ADDLF    0x0040      /* TJW 970503 auto cr -> cr/lf conv.  */
#define SB_ADDCR    0x0080      /* TJW 970503 auto lf -> cr/lf conv.  */
#define SB_IGNLF    0x0100      /* TJW 970503 ignore cr               */
#define SB_IGNCR    0x0200      /* TJW 970503 ignore lf               */

/* coordinates of a window (rectangular region) on the screen buffer */

/* At one point, we tried substituting REGION with HWND.  There were some
 * inherent problems with this, most notably, that a window doesn't have
 * a current cursor position concept.
 *
 * We probably can and should create a special window class that would
 * deal with this, but not today...
 */

typedef struct
{
  short row, col;               /* current position */

  /* window boundaries */
  short r0, c0;                 /* upper left corner */
  short r1, c1;                 /* lower right corner */

  /* scrolling region boundaries */
  short sr0, sc0;               /* upper left corner */
  short sr1, sc1;               /* lower right corner */

  unsigned short wflags;        /* window buffer flags */

  short linesize;               /* line size */
  short lines;                  /* number of scrollable lines */
  char *buffer;                 /* scroll buffer */
  char *endbuff;                /* end of buffer */
  char *lastline;               /* -> last line */
  char *lastshown;              /* -> last line displayed */

  short lastfillattr;           /* CFS 970216 - attribute of the last sb_fill() */

  _ANSIStuff ANSI;
}
REGION, *REGIONP;

/* coordinates of a window (rectangular region) on the screen buffer */
typedef struct
{
  short save_row, save_col;     /* top left corner */
  short save_ht, save_wid;      /* height and width */

  REGIONP region;               /* Window to use */

  CELLP save_cells;             /* Saved cells from the window */

}
BINK_SAVE, *BINK_SAVEP;

#define HIST_IN_ROW     0
#define HIST_OUT_ROW    1
#define HIST_RX_ROW     2
#define HIST_TX_ROW     3
#define HIST_ERR_ROW    4
#define HIST_LAST_ROW   5

#define SET_COL         8
#define SET_TIME_COL    0

#define SET_TIME_ROW     0
#define SET_EVNT_ROW     1
#define SET_PORT_ROW     2
#define SET_BAUD_ROW     3
#define SET_STAT_ROW     4
#define SET_SESSION_ROW  5
#define SET_PROTOCOL_ROW 6
#define SET_MAIL_ROW     7
#define SET_MEMAVAIL_ROW 8

/* ------------------------------- definitions of main windows ------------ */

#define TOP_PARTS_ROWS    12
#define BOTTOM_PARTS_ROWS 1

#define TOPWIN_ROW        1
#define TOPWIN_HIGH       (TOP_PARTS_ROWS - TOPWIN_ROW)

#define SETTINGS_COL      0
#define SETTINGS_ROW      TOPWIN_ROW
#define SETTINGS_SIZE     20
#define SETTINGS_HIGH     TOPWIN_HIGH

#define HISTORY_COL       (SETTINGS_COL + SETTINGS_SIZE - 1)
#define HISTORY_ROW       TOPWIN_ROW
#define HISTORY_SIZE      21
#define HISTORY_HIGH      (TOPWIN_HIGH - FILEWIN_HIGH + 1)

#define HOLDWIN_COL       (HISTORY_COL + HISTORY_SIZE - 1)
#define HOLDWIN_ROW       TOPWIN_ROW
#define HOLDWIN_SIZE      (SB_COLS - SETTINGS_SIZE - HISTORY_SIZE + 2)
#define HOLDWIN_HIGH      (TOPWIN_HIGH - FILEWIN_HIGH + 1)

#define SB_ROW_HOLD       (HOLDWIN_HIGH - 2)

#define FILEWIN_COL       HISTORY_COL
#define FILEWIN_ROW       (TOP_PARTS_ROWS - FILEWIN_HIGH)
#define FILEWIN_HIGH      4
#define FILEWIN_SIZE      (SB_COLS-FILEWIN_COL)

#define CALLWIN_COL       0
#define CALLWIN_ROW       (TOP_PARTS_ROWS)
#define CALLWIN_HIGH      (SB_ROWS - TOP_PARTS_ROWS - BOTTOM_PARTS_ROWS)
#define CALLWIN_SIZE      SB_COLS

/* MR 970215 */
#define MODEMWIN_ROW      CALLWIN_ROW
#define MODEMWIN_HIGH     CALLWIN_HIGH
#define MW_SENT           colors.mw_sent
#define MW_RCVD           colors.mw_rcvd

/* ------------------------------- definitions of transfer window -------------
   This sheme is implemented to all protocols (jan 14 1997, r. hoerner)
   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
   ³ Z-Send filename.ext  12345678/12345678   1234   xx:xx:xx/xx:xx:xx  xxxxx cps ³
   ³ Z-Recv filename.ext  12345678/12345678   1234   xx:xx:xx/xx:xx:xx  xxxxx cps ³
   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
   0         1         2         3         4         5         6         7
   01234567890123456789012345678901234567890123456789012345678901234567890123456789
 */

#define POS_INFO       50
#define POS_NAME       9
#define POS_SIZE_TRANS 23
#define POS_SIZE_ABS   32
#define POS_BLOCKSIZE  42
#define POS_ESTIMATE   50
#define POS_ELAPSED    59
#define POS_CPS        69

/* $Id: sbuf.h,v 1.3 1999/02/27 01:15:17 mr Exp $ */
