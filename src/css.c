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
 * Filename    : $Source: E:/cvs/btxe/src/css.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:24 $
 * State       : $State: Exp $
 * Orig. Author: Thomas Waldmann and tom schlangen
 *
 * Description : Current Session Statistics
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/*

   + Current Session Statistics -------------------------------+
   |<ftn address, sysop name>                          hh:mm:ss| <- Elapsed
   |                                                           |
   |Tx File    Filename                                   #0001| <- # files so far 
   |Tx current xxxxc/yyyyc zzz% hh:mm:ss [....................]|
   |Tx total   xxxxc/yyyyc zzz% hh:mm:ss [....................]|
   |                                                           |
   |Rx File    Filename                                   #0001| <- # files so far 
   |Rx current xxxxc/yyyyc zzz% hh:mm:ss [....................]|
   |Rx total   xxxxc/yyyyc zzz% hh:mm:ss [....................]|
   +-----------------------------------------------------------+
   ^           ^    ^        ^
   | pos./len. |%   |est.time|
   |           |done|        |
   |           |so  |        |
   |           |far |        |

 */

/****************************************************************************/
/*  local constants                                                         */
/****************************************************************************/
#define CSS_ROW_HEADLINE 0      /*  row for headline            */
#define CSS_ROW_TX_FNAME 2      /*  row for tx current filename */
#define CSS_ROW_TX_CUR   3      /*  row for tx current file     */
#define CSS_ROW_TX_TOT   4      /*  row for tx total            */
#define CSS_ROW_RX_FNAME 6      /*  row for rx current filename */
#define CSS_ROW_RX_CUR   7      /*  row for tx current file     */
#define CSS_ROW_RX_TOT   8      /*  row for rx total            */
#define CSS_MSG_X        0      /*  column of explanatory msgs  */

#define CSSP_TX_CUR      0
#define CSSP_TX_TOT      1
#define CSSP_RX_CUR      2
#define CSSP_RX_TOT      3

#define CSSP_WIDTH       20     /*  bar length w/o delimiters   */
#define CSSP_STRLEN      CSSP_WIDTH + 3  /*  length of complete bar str. */
#define CSSP_COL         37     /*  column for bars             */
#define CSSP_INVALID     -1L

/****************************************************************************/
/*  local static data                                                       */
/****************************************************************************/
static REGIONP csswin, filewinold;
static BINK_SAVEP holdwinsave, historywinsave, filewinsave;
static char acbarchars[6];      /*  holds array of used chars   */
static char acprometer[CSSP_STRLEN];  /*  holds bar string to display */
static int otxc, orxc, otxm, orxm;

/****************************************************************************/
/*  local function prototypes                                               */
/****************************************************************************/
static void LOCALFUNC cssp_upd (long total, long done, int what);
static char *LOCALFUNC cssp_str (int i);
static int LOCALFUNC cssp_comp (long total, long done);

/****************************************************************************/
/*  functions                                                               */
/****************************************************************************/

/* return string current/total percentage time_left */
static char *LOCALFUNC
css_helper1 (long cur, long tot)
{
  char buf1[8], buf2[8], buf3[8], buf4[12];
  static char buf5[64];
  long p, need, efficiency, hrs, mins, secs;

  if (cur >= 0)
    sprintf (buf1, "%5s", numdisp (cur, 5));
  else
    sprintf (buf1, "???? ");

  if (tot >= 0 && tot >= cur)
    sprintf (buf2, "%5s", numdisp (tot, 5));
  else
    sprintf (buf2, "???? ");


  switch (session_method)
  {
  case HYDRA_SESSION:
    efficiency = 95L;
    break;

  case JANUS_SESSION:
    efficiency = 95L;
    break;

  case ZMODEM_SESSION:
    efficiency = 95L;
    break;

  case FTSC_SESSION:
    efficiency = 70L;
    break;                      /* 70 ??? */

  default:
    efficiency = 90L;
    break;                      /* !? */
  }

  if (tot < 0)
  {
    sprintf (buf3, "???%%");
    sprintf (buf4, "??:??:??");
  }
  else
  {
    if (tot == 0)
    {
      sprintf (buf3, "100%%");
      sprintf (buf4, "00:00:00");
    }
    else
    {
      if (cur < 20000000L)      /* if cur<20M,100L*cur will not cause long-overflow */
        p = 100L * cur / tot;
      else
      {
        long cur2 = cur / 1024; /* this is OK from 20MB up to 20GB */
        long tot2 = tot / 1024;

        p = 100L * cur2 / tot2;
      }

      // if(p>98)
      //   status_line("!CSS debug: done=%ld, total=%ld, percent=%ld",
      //               cur,tot,p);

      if (p >= 0 && p <= 100)
      {
        sprintf (buf3, "%3ld%%", p);
        if (tot >= cur)
        {
          need = remaining (tot - cur, efficiency);
          hrs = need / 3600;
          mins = (need % 3600) / 60;
          secs = need % 60;
          sprintf (buf4, "%02ld:%02ld:%02ld", hrs, mins, secs);
        }
        else
        {
          sprintf (buf4, "??:??:??");
        }
      }
      else
      {
        sprintf (buf3, "???%%");
        sprintf (buf4, "??:??:??");
      }
    }
  }

  sprintf (buf5, "%s/%s %s %s", buf1, buf2, buf3, buf4);
  return buf5;
}

/* return string elapsed (or NULL if unchanged) */
static char *LOCALFUNC
css_helper2 (void)
{
  time_t tnow, dt;
  static time_t told;
  static char buf[12];

  unix_time (&tnow);
  if (tnow != told)
  {
    long eh, em, es;

    if (tnow < etm)
      tnow += 86400L;
    dt = tnow - etm;
    eh = dt / 3600L;
    em = (dt % 3600) / 60;
    es = dt % 60;
    sprintf (buf, "%02ld:%02ld:%02ld", eh, em, es);
    return buf;
  }
  return NULL;
}

void
css_upd (void)
{
  static XFERSTATS TxOld, RxOld;
  static GENSTATS GenOld;

  if (debugflags & DEBUGFLAGS_NOCSS)
    return;

  if (csswin != NULL)
  {
    int changed = 0;
    char buf[256];
    char *elapsedp;

    if (memcmp (&TxStats, &TxOld, sizeof (XFERSTATS)))
    {

      if (strcmp (TxStats.fname, TxOld.fname)
          || TxStats.cur_fxfrd != TxOld.cur_fxfrd)
      {
        sprintf (buf, "%-42s #%04ld", TxStats.fname, TxStats.cur_fxfrd);
        sb_move_puts (csswin, CSS_ROW_TX_FNAME, 11, buf);
      }

      if (TxStats.FilePos != TxOld.FilePos
          || TxStats.FileLen != TxOld.FileLen)
      {
        sb_move_puts (csswin, CSS_ROW_TX_CUR, 11,
                      css_helper1 (TxStats.FilePos, TxStats.FileLen));

        cssp_upd (TxStats.FileLen, TxStats.FilePos, CSSP_TX_CUR);
      }

      if (TxStats.cur_mxfrd != TxOld.cur_mxfrd
          || TxStats.FilePos != TxOld.FilePos
          || TxStats.tot_moh != TxOld.tot_moh)
      {
        if (TxStats.FilePos >= 0)
        {
          sb_move_puts (csswin, CSS_ROW_TX_TOT, 11,
                        css_helper1 (TxStats.cur_mxfrd + TxStats.FilePos,
                                     TxStats.tot_moh));
          cssp_upd (TxStats.tot_moh, TxStats.cur_mxfrd + TxStats.FilePos,
                    CSSP_TX_TOT);
        }
        else
        {
          sb_move_puts (csswin, CSS_ROW_TX_TOT, 11,
                        css_helper1 (TxStats.cur_mxfrd, TxStats.tot_moh));
          cssp_upd (TxStats.tot_moh, TxStats.cur_mxfrd, CSSP_TX_TOT);
        }

      }

      TxOld = TxStats;          /* struct assignment */
      changed = 1;
    }

    if (memcmp (&RxStats, &RxOld, sizeof (XFERSTATS)))
    {

      if (strcmp (RxStats.fname, RxOld.fname)
          || RxStats.cur_fxfrd != RxOld.cur_fxfrd)
      {
        sprintf (buf, "%-42s #%04ld", RxStats.fname, RxStats.cur_fxfrd);
        sb_move_puts (csswin, CSS_ROW_RX_FNAME, 11, buf);
      }

      if (RxStats.FilePos != RxOld.FilePos
          || RxStats.FileLen != RxOld.FileLen)
      {
        sb_move_puts (csswin, CSS_ROW_RX_CUR, 11,
                      css_helper1 (RxStats.FilePos, RxStats.FileLen));

        cssp_upd (RxStats.FileLen, RxStats.FilePos, CSSP_RX_CUR);
      }

      if (RxStats.cur_mxfrd != RxOld.cur_mxfrd
          || RxStats.FilePos != RxOld.FilePos
          || RxStats.tot_moh != RxOld.tot_moh)
      {
        if (RxStats.FilePos >= 0)
        {
          sb_move_puts (csswin, CSS_ROW_RX_TOT, 11,
                        css_helper1 (RxStats.cur_mxfrd + RxStats.FilePos,
                                     RxStats.tot_moh));
          cssp_upd (RxStats.tot_moh, RxStats.cur_mxfrd + RxStats.FilePos,
                    CSSP_RX_TOT);
        }
        else
        {
          sb_move_puts (csswin, CSS_ROW_RX_TOT, 11,
                        css_helper1 (RxStats.cur_mxfrd, RxStats.tot_moh));
          cssp_upd (RxStats.tot_moh, RxStats.cur_mxfrd, CSSP_RX_TOT);
        }
      }

      RxOld = RxStats;          /* struct assignment */
      changed = 1;
    }

    if (memcmp (&GenStats, &GenOld, sizeof (GENSTATS)))
    {

      sprintf (buf, "%s", GenStats.remote);
      sb_move_puts (csswin, CSS_ROW_HEADLINE, CSS_MSG_X, buf);
      sprintf (buf, "%12s", GenStats.protocol);
      sb_move_puts (csswin, CSS_ROW_HEADLINE, 47, buf);

      GenOld = GenStats;        /* struct assignment */
      changed = 1;
    }

    elapsedp = css_helper2 ();

    if (elapsedp)
    {
      sb_move_puts (csswin, CSS_ROW_HEADLINE, 51, elapsedp);
      show_alive ();
      IPC_Ping ();
      changed = 1;
    }

    if (changed)
      display_life ();          /* does also sb_show(); */
  }
}

static void
StatsInit (XFERSTATS * p)
{
  memset (p, '\0', sizeof (*p));
  *p->fname = '\0';
}

void
css_init (void)
{
  char junk[128];
  int i;
  char *p;
  char c;
  BOXTYPE *boxp;

  static char cssp_notavail[CSSP_STRLEN] = "[                    ]";

  /*  save hold/history/file window contents                          */
  holdwinsave = sb_save (HOLDWIN_ROW, HOLDWIN_COL,
                         HOLDWIN_HIGH, HOLDWIN_SIZE);

  historywinsave = sb_save (HOLDWIN_ROW, HISTORY_COL,
                            HOLDWIN_HIGH, HISTORY_SIZE);

  filewinsave = sb_save (FILEWIN_ROW, FILEWIN_COL,
                         FILEWIN_HIGH, FILEWIN_SIZE);

  filewinold = filewin;
  filewin = NULL;

  memset (&GenStats, '\0', sizeof (GENSTATS));
  StatsInit (&RxStats);
  StatsInit (&TxStats);

  csswin = sb_new_win (TOPWIN_ROW, HISTORY_COL,
                       TOPWIN_HIGH, SETTINGS_SIZE + HOLDWIN_SIZE,
                       ' ', colors.hold,
                       boxtype, colors.frames,
                       MSG_TXT (M_CSS_CAPTION));

  boxp = &box[boxtype];

  junk[0] = boxp->tt;
  junk[1] = '\0';
  sb_move_puts_abs (csswin, 0, 0, junk);

  junk[0] = boxp->t180;
  sb_move_puts_abs (csswin, TOPWIN_HIGH - 1, 0, junk);

  /* now let's make two additional separator lines                        */
  c = boxtype == 0 ? '-' : 0xc4;
  p = junk;
  i = SETTINGS_SIZE + HOLDWIN_SIZE - 2;

  for (i = SETTINGS_SIZE + HOLDWIN_SIZE - 2; i > 0; i--)
    *p++ = c;

  *p = '\0';
  sb_move_puts (csswin, CSS_ROW_HEADLINE + 1, 0, junk);
  sb_move_puts (csswin, CSS_ROW_HEADLINE + 5, 0, junk);

  /*  initialize some local data                                          */
  orxc = otxc = orxm = otxm = (int) CSSP_INVALID;

  /*  set up bar characters according to frametype in use                 */
  strcpy (acbarchars, boxtype == 0 ? MSG_TXT (M_CSS_CHARS_ASC) : MSG_TXT (M_CSS_CHARS_GRA));

  /*  write `not avail' bars                                              */
  sb_move_puts (csswin, CSS_ROW_TX_CUR, CSSP_COL, cssp_notavail);
  sb_move_puts (csswin, CSS_ROW_TX_TOT, CSSP_COL, cssp_notavail);
  sb_move_puts (csswin, CSS_ROW_RX_CUR, CSSP_COL, cssp_notavail);
  sb_move_puts (csswin, CSS_ROW_RX_TOT, CSSP_COL, cssp_notavail);

  sb_move_puts (csswin, CSS_ROW_TX_FNAME, CSS_MSG_X, MSG_TXT (M_CSS_TX_FNAME));
  sb_move_puts (csswin, CSS_ROW_TX_CUR, CSS_MSG_X, MSG_TXT (M_CSS_TX_CUR));
  sb_move_puts (csswin, CSS_ROW_TX_TOT, CSS_MSG_X, MSG_TXT (M_CSS_TX_TOT));
  sb_move_puts (csswin, CSS_ROW_RX_FNAME, CSS_MSG_X, MSG_TXT (M_CSS_RX_FNAME));
  sb_move_puts (csswin, CSS_ROW_RX_CUR, CSS_MSG_X, MSG_TXT (M_CSS_RX_CUR));
  sb_move_puts (csswin, CSS_ROW_RX_TOT, CSS_MSG_X, MSG_TXT (M_CSS_RX_TOT));

}

void
css_done (void)
{
  if (csswin != NULL)
  {
    /* restore old window contents */
    sb_restore (holdwinsave);
    sb_restore (historywinsave);
    filewin = filewinold;
    sb_restore (filewinsave);
    csswin = NULL;
  }
}

static int LOCALFUNC
cssp_comp (long total, long done)
{
  int i;

  if (done > total)             /* TJW 960707 catch prob with FREQs */
    total = done;               /* until there is a better solution */

  if (total <= 0)
    i = 0;                      /* catch division by zero           */
  else
  {
    if (done < (2000000000L / 2 / CSSP_WIDTH))  /* no long overflow will occur    */
      i = abs ((int) ((done * CSSP_WIDTH * 2) / total));  /* compute `done' chars */
    else
    {                           /*  scaled up by 2                  */
      long done2 = done / 1024;
      long total2 = total / 1024;

      i = abs ((int) ((done2 * CSSP_WIDTH * 2) / total2));  /* compute `done' chars */
    }
  }

  return i;
}

static char *LOCALFUNC
cssp_str (int i)                /*  construct string to display */
{
  register char *p = acprometer;
  int j, k;

  k = (i >> 1);                 /*  scale down by 2             */
  j = CSSP_WIDTH - k;           /*  remaining `todo' chars      */

  *p++ = acbarchars[0];         /*  write left bar delimiter    */

  for (; k > 0; k--)            /*  write chars for `done'      */
    *p++ = acbarchars[1];

  if (i % 2)                    /*  check if between two blocks */
  {
    *p++ = acbarchars[2];       /*  yes, so write `half' block  */
    j--;                        /*  and dec `todo' chars by one */
  }

  for (; j > 0; j--)            /*  write chars for `todo'      */
    *p++ = acbarchars[3];

  *p++ = acbarchars[4];         /*  write right bar delimiter   */
  *p = '\0';                    /*  terminate string            */
  return (acprometer);          /*  return string adress        */
}

static void LOCALFUNC
cssp_upd (long total, long done, int what)
{
  int i;

  i = cssp_comp (total, done);
  switch (what)
  {
  case CSSP_TX_CUR:
    if (otxc != i)
    {
      otxc = i;
      sb_move_puts (csswin, CSS_ROW_TX_CUR, CSSP_COL, cssp_str (i));
    }
    break;

  case CSSP_RX_CUR:
    if (orxc != i)
    {
      orxc = i;
      sb_move_puts (csswin, CSS_ROW_RX_CUR, CSSP_COL, cssp_str (i));
    }
    break;

  case CSSP_TX_TOT:
    if (otxm != i)
    {
      otxm = i;
      sb_move_puts (csswin, CSS_ROW_TX_TOT, CSSP_COL, cssp_str (i));
    }
    break;

  case CSSP_RX_TOT:
    if (orxm != i)
    {
      orxm = i;
      sb_move_puts (csswin, CSS_ROW_RX_TOT, CSSP_COL, cssp_str (i));
    }
    break;
  }
}

/* $Id: css.c,v 1.5 1999/03/22 03:47:24 mr Exp $ */
