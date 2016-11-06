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
 * Filename    : $Source: E:/cvs/btxe/src/sbuf.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:39 $
 * State       : $State: Exp $
 * Orig. Author: Augie Hansen
 *
 * Description : Box Drawing subroutines for BinkleyTerm
 *
 * Note        :
 *   This module is derived from code developed by Augie Hansen in his
 *   book "Proficient C" published by Microsoft Press.  Mr. Hansen was
 *   kind enough to give us verbal permission to use his routines, and
 *   Bob, Vince and Alan (and all our full screen users) are grateful.
 *   If you decide to use this code in some package you are doing, give
 *   some thought to going out and buying the book. He deserves that.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#include "video.h"

BUFFER Sbuf;                    /* control information */
CELLP Scrnbuf;                  /* screen buffer array */


BOXTYPE box[] =
{
{'+', '+', '+', '+', '-', '-', '|', '|', '+', '+', '+', '+', '+', '+', '+'},
  {ULC11, URC11, LLC11, LRC11, HBAR1, HBAR1, VBAR1, VBAR1, TT11, TB11, TT11, TR11, TB11, TL11, X11},
  {ULC22, URC22, LLC22, LRC22, HBAR2, HBAR2, VBAR2, VBAR2, TT22, TB22, TT22, TR22, TB22, TL22, X22},
  {ULC12, URC12, LLC12, LRC12, HBAR1, HBAR1, VBAR2, VBAR2, TT12, TB12, TT12, TR12, TB12, TL12, X12},
  {ULC21, URC21, LLC21, LRC21, HBAR2, HBAR2, VBAR1, VBAR1, TT21, TB21, TT21, TR21, TB21, TL21, X21},
  {BLOCK, BLOCK, BLOCK, BLOCK, HBART, HBARB, BLOCK, BLOCK, HBART, HBARB, BLOCK, BLOCK, BLOCK, BLOCK, BLOCK}
};

void
sb_clear (REGIONP win)
{
  win->sr0++;
  win->sr1--;
  win->sc0++;
  win->sc1--;
  sb_fillc (win, ' ');
  win->sr0--;
  win->sr1++;
  win->sc0--;
  win->sc1++;
}

void
sb_send_pm (REGIONP win, short row, short col, char *s)
{
#ifdef OS_2

  if (s == NULL)                /* sb_fill();  */
  {
    if (win == wholewin)
      SendToDlg (NULL, row, col, CLEAR_WHOLEWIN);
    else if (win == filewin)
      SendToDlg (NULL, row, col, CLEAR_FILEWIN);
    else if (win == callwin)
      SendToDlg (NULL, row, col, CLEAR_CALLWIN);
    else if (win == holdwin)
      SendToDlg (NULL, row, col, CLEAR_HOLDWIN);
    else if (win == settingswin)
      SendToDlg (NULL, row, col, CLEAR_SETTINGS);
    else if (win == historywin)
      SendToDlg (NULL, row, col, CLEAR_HISTORY);
  }
  else if (win == wholewin)
  {
    if (row == 0x00ff)
      SendToDlg ((PSZ) s, row, col, BINK_START);
    else
      SendToDlg ((PSZ) s, row, col, BOTTOM_LINE_R);
  }
  else if (win == callwin)
  {
    if (row == 0x00ff)
      SendToDlg ((PSZ) s, row, col, T_TEXT_ACTIVITY);
    else
      SendToDlg ((PSZ) s, row, col, CALLWIN);
  }
  else if (win == holdwin)
  {
    switch (row)
    {
    case 0:
      SendToDlg ((PSZ) s, row, col, E_HOLDWIN_1);
      break;

    case 1:
      SendToDlg ((PSZ) s, row, col, E_HOLDWIN_2);
      break;

    case 2:
      SendToDlg ((PSZ) s, row, col, E_HOLDWIN_3);
      break;

    case 3:
      SendToDlg ((PSZ) s, row, col, E_HOLDWIN_4);
      break;

    case 4:
      SendToDlg ((PSZ) s, row, col, E_HOLDWIN_5);
      break;

    case 0x00ff:
      SendToDlg ((PSZ) s, row, col, T_TEXT_OUTBOUND);
      break;
    }
  }
  else if (win == filewin)
  {
    switch (row)
    {
    case 0:
      SendToDlg ((PSZ) s, row, col, TRANSFER_SEND);
      break;

    case 1:
      SendToDlg ((PSZ) s, row, col, TRANSFER_RECV);
      break;

    case 0x00ff:
      SendToDlg ((PSZ) s, row, col, T_TEXT_TRANSFER);
      break;
    }
  }
  else if (win == settingswin)
  {
    if (row == 0x00ff)
      SendToDlg ((PSZ) s, row, col, T_TEXT_STATUS);
    else if (col == 0)
    {
      switch (row)
      {
      case SET_TIME_ROW:
        SendToDlg (s, row, col, S_TEXT_DATUM);
        break;

      case SET_EVNT_ROW:
        SendToDlg (s, row, col, S_TEXT_EVENT);
        break;

      case SET_PORT_ROW:
        SendToDlg (s, row, col, S_TEXT_PORT);
        break;

      case SET_BAUD_ROW:
        SendToDlg (s, row, col, S_TEXT_DTE);
        break;

      case SET_STAT_ROW:
        SendToDlg (s, row, col, S_TEXT_STATUS);
        break;

        /* RS 9710 start new binkley style  */
      case SET_SESSION_ROW:
        SendToDlg (s, row, col, S_TEXT_SESSION);
        break;

      case SET_PROTOCOL_ROW:
        SendToDlg (s, row, col, S_TEXT_PROTOCOL);
        break;

      case SET_MAIL_ROW:
        SendToDlg (s, row, col, S_TEXT_NETMAIL);
        break;

      case SET_MEMAVAIL_ROW:
        SendToDlg (s, row, col, S_TEXT_MEMORY);
        break;
        /* RS 9710 end new binkley style  */
      }
    }
    else
    {
      switch (row)
      {
      case SET_EVNT_ROW:
        SendToDlg (s, row, col, E_EVENT);
        break;

      case SET_PORT_ROW:
        SendToDlg (s, row, col, E_PORT);
        break;

      case SET_BAUD_ROW:
        SendToDlg (s, row, col, E_DTE);
        break;

      case SET_STAT_ROW:
        SendToDlg (s, row, col, E_STATUS);
        break;

        /* RS 9710 start new binkley style  */
      case SET_SESSION_ROW:
        SendToDlg (s, row, col, E_SESSION);
        break;

      case SET_PROTOCOL_ROW:
        SendToDlg (s, row, col, E_PROTOCOL);
        break;

      case SET_MAIL_ROW:
        SendToDlg (s, row, col, E_NETMAIL);
        break;

      case SET_MEMAVAIL_ROW:
        SendToDlg (s, row, col, E_MEMORY);
        break;
        /* RS 9710 end new binkley style  */
      }
    }
  }
  else if (win == historywin)
  {
    switch (row)
    {
    case HIST_IN_ROW:
      SendToDlg (s, row, col, E_INOUT);
      break;

    case HIST_OUT_ROW:
      SendToDlg (s, row, col, E_OUTBOUND);
      break;

      /* RS 9710 start new binkley style, reused the old values */
    case HIST_RX_ROW:
      SendToDlg (s, row, col, E_COST);
      break;

    case HIST_TX_ROW:
      SendToDlg (s, row, col, E_FILES);
      break;
      /* RS 9710 end new binkley style  */

    case HIST_LAST_ROW:
      SendToDlg (s, row, col, E_LAST);
      break;

    case 0x00ff:
      SendToDlg (s, row, col, T_TEXT_OVERVIEW);
    }
  }

#else
  win = win;
  row = row;
  col = col;
  ++s;
#endif
}

void
sb_box (REGIONP win, short type, short attr)
{
  register short r;
  short x;
  short maxr, maxc;
  BOXTYPE *boxp;

  boxp = &box[type];
  maxc = win->c1 - win->c0;
  maxr = win->r1 - win->r0;
  x = maxc - 1;

  /* draw top row */
  sb_move_abs (win, 0, 0);
  sb_wca_abs (win, boxp->ul, attr, 1);
  sb_move_abs (win, 0, 1);
  sb_wca_abs (win, boxp->tbar, attr, x);
  sb_move_abs (win, 0, maxc);
  sb_wca_abs (win, boxp->ur, attr, 1);

  /* draw left and right sides */
  for (r = 1; r < maxr; r++)
  {
    sb_move_abs (win, r, 0);
    sb_wca_abs (win, boxp->lbar, attr, 1);
    sb_move_abs (win, r, maxc);
    sb_wca_abs (win, boxp->rbar, attr, 1);
  }

  /* draw bottom row */
  sb_move_abs (win, maxr, 0);
  sb_wca_abs (win, boxp->ll, attr, 1);
  sb_move_abs (win, maxr, 1);
  sb_wca_abs (win, boxp->bbar, attr, x);
  sb_move_abs (win, maxr, maxc);
  sb_wca_abs (win, boxp->lr, attr, 1);
}

void
sb_fill (REGIONP win, short ch, short attr)
{
  register short i, j;
  unsigned short ca;

  ca = ((unsigned short) attr << 8) | (unsigned short) ch;
  win->lastfillattr = attr;     /* CFS 970216 */

  for (i = win->sr0; i <= win->sr1; i++)
  {
    for (j = win->sc0; j <= win->sc1; j++)
      (Scrnbuf + i * SB_COLS + j)->cap = ca;

    if (win->sc0 < Sbuf.lcol[i])
      Sbuf.lcol[i] = win->sc0;

    if (win->sc1 > Sbuf.rcol[i])
      Sbuf.rcol[i] = win->sc1;
  }

  Sbuf.flags |= SB_DELTA;
  sb_send_pm (win, 0, 0, NULL);
  return;
}

void
sb_fillc (REGIONP win, short ch)
{
  register short i, j;

  for (i = win->sr0; i <= win->sr1; i++)
  {
    for (j = win->sc0; j <= win->sc1; j++)
      (Scrnbuf + i * SB_COLS + j)->b.ch = (unsigned char) ch;

    if (win->sc0 < Sbuf.lcol[i])
      Sbuf.lcol[i] = win->sc0;

    if (win->sc1 > Sbuf.rcol[i])
      Sbuf.rcol[i] = win->sc1;
  }

  Sbuf.flags |= SB_DELTA;
  if (ch == ' ')                /* indirect window clear */
    sb_send_pm (win, 0, 0, NULL);
  return;
}

void
sb_filla (REGIONP win, short attr)
{
  register short i, j;

  win->lastfillattr = attr;     /* CFS 970216 */

  for (i = win->sr0; i <= win->sr1; i++)
  {
    for (j = win->sc0; j <= win->sc1; j++)
      (Scrnbuf + i * SB_COLS + j)->b.attr = (unsigned char) attr;

    if (win->sc0 < Sbuf.lcol[i])
      Sbuf.lcol[i] = win->sc0;

    if (win->sc1 > Sbuf.rcol[i])
      Sbuf.rcol[i] = win->sc1;
  }

  Sbuf.flags |= SB_DELTA;
  return;
}

void
sb_init ()
{
  short i, j;
  CELLP c;
  char q[20];

  Scrnbuf = (CELLP) calloc ((unsigned short) SB_ROWS * (unsigned short) SB_COLS, sizeof (CELL));
  Sbuf.bp = (CELLP) Scrnbuf;

  Sbuf.row = Sbuf.col = 0;
  Sbuf.lcol = (short *) calloc ((unsigned short) SB_COLS, sizeof (short));
  Sbuf.rcol = (short *) calloc ((unsigned short) SB_ROWS, sizeof (short));

  if (!Scrnbuf || !Sbuf.lcol || !Sbuf.rcol)
    return;

  sprintf (&stat_str[8], "-%ds", SB_COLS - 13);
  sprintf (&script_line[19], "-%ds", SB_COLS - 22);

  if ((Scrnbuf == NULL) ||
      (Sbuf.lcol == NULL) ||
      (Sbuf.rcol == NULL))
  {
    if (Scrnbuf != NULL)
    {
      free (Scrnbuf);
      Scrnbuf = NULL;
    }

    if (Sbuf.lcol != NULL)
    {
      free (Sbuf.lcol);
      Sbuf.lcol = NULL;
    }

    if (Sbuf.rcol != NULL)
    {
      free (Sbuf.rcol);
      Sbuf.rcol = NULL;
    }

    SB_ROWS = 23;
    SB_COLS = 80;

    Scrnbuf = (CELLP) calloc ((unsigned short) SB_ROWS * (unsigned short) SB_COLS, sizeof (CELL));
    Sbuf.bp = (CELLP) Scrnbuf;

    Sbuf.row = Sbuf.col = 0;
    Sbuf.lcol = (short *) calloc ((unsigned short) SB_COLS, sizeof (short));
    Sbuf.rcol = (short *) calloc ((unsigned short) SB_ROWS, sizeof (short));

    if (!Scrnbuf || !Sbuf.lcol || !Sbuf.rcol)
      return;

    sprintf (q, "-%d.%ds", SB_COLS - 16, SB_COLS - 16);
    strcpy (&stat_str[19], q);
    sprintf (q, "-%d.%ds", SB_COLS - 22, SB_COLS - 22);
    strcpy (&script_line[19], q);
  }

  for (i = 0; i < (short) SB_ROWS; i++)
  {
    Sbuf.lcol[i] = SB_COLS;
    Sbuf.rcol[i] = 0;
  }

  Sbuf.flags = 0;

  c = Scrnbuf;
  for (i = 0; i < (short) SB_ROWS; i++)
    for (j = 0; j < (short) SB_COLS; j++)
    {
      (*c).cap = WHITEBLANK;
      ++c;
    }

  return;
}

static void
_sb_move (REGIONP win, register short r, register short c, int abs)
{
  /* don't change anything if request is out of range */
  if (abs)
  {
    if ((r < 0) || (r > (win->r1 - win->r0)) ||
        (c < 0) || (c > (win->c1 - win->c0)))
      return;                   /* (SB_ERR); */
  }
  else
  {
    if ((r < 0) || (r > (win->sr1 - win->sr0)) ||
        (c < 0) || (c > (win->sc1 - win->sc0)))
      return;                   /* (SB_ERR); */
  }

  win->row = r;
  win->col = c;

  if (abs)
  {
    Sbuf.row = r + win->r0;
    Sbuf.col = c + win->c0;
  }
  else
  {
    Sbuf.row = r + win->sr0;
    Sbuf.col = c + win->sc0;
  }

  return;                       /* (SB_OK); */
}

void
sb_move (REGIONP win, register short r, register short c)
{
  _sb_move (win, r, c, 0);
}

void
sb_move_abs (REGIONP win, register short r, register short c)
{
  _sb_move (win, r, c, 1);
}

void
sb_caption (REGIONP Rgn, char *Caption, unsigned short delim_att)
{
  /*  TS 970412;  introduced a new caption/frame style                    */
  BOXTYPE *boxp;

  boxp = &box[boxtype];
  sb_move_abs (Rgn, 0, 1);
  sb_wca_abs (Rgn, boxp->t90, delim_att, 1);
  sb_move_abs (Rgn, 0, 2);
  sb_wa_abs (Rgn, colors.headers, strlen (Caption));  /* TJW 960428 window header colors! */
  sb_puts_abs (Rgn, Caption);
  sb_move_abs (Rgn, 0, strlen (Caption) + 2);
  sb_wca_abs (Rgn, boxp->t270, delim_att, 1);

#ifdef OS_2
  sb_send_pm (Rgn, 0x00ff, 1, Caption);
#endif
}

REGIONP
sb_new (short top, short left, short height, short width)
{
  REGIONP new;

  new = calloc (1, sizeof (REGION));
  if (new != NULL)
  {
    new->r0 = new->sr0 = top;
    new->r1 = new->sr1 = top + height - 1;
    new->c0 = new->sc0 = left;
    new->c1 = new->sc1 = left + width - 1;
    new->row = new->col = 0;
    new->wflags = SB_ADDLF | SB_IGNLF;
    /* cr -> cr/lf is standard! */
    /* ignore lf is standard!   */
  }
  return (new);
}

REGIONP
sb_new_win (short top, short left, short height, short width,    /* sb_new */
            short fch, short fattr,  /* sb_fill    */
            short btype, short battr,  /* sb_box   */
            char *Caption)      /* sb_caption */
{
  REGIONP win;

  win = sb_new (top, left, height, width);
  if (win)
  {
    sb_fill (win, fch, fattr);
    win->lastfillattr = fattr;  /* CFS 970216 */
    sb_box (win, btype, battr);
    sb_caption (win, Caption, battr);
    win->sr0++;
    win->sc0++;
    win->sr1--;
    win->sc1--;
    win->lines = win->sr1 - win->sr0 + 1;
    win->linesize = win->sc1 - win->sc0 + 1;
    sb_move (win, 0, 0);
  }
  return win;
}

int
sb_popup (short top, short left, short height, short width,
          int (*func) (BINK_SAVEP, int), int arg)
{
  int saved, x;
  BINK_SAVEP tmp = (BINK_SAVEP) NULL;

  saved = 0;
  tmp = sb_save (top, left, height, width);
  if (tmp == NULL)
    return (-1);

  if (popped_up == NULL)        /* store ptr to 1st popup window */
  {
    popped_up = tmp;
    saved = 1;
  }

  x = (*func) (tmp, arg);
  sb_restore (tmp);
  if (saved)
    popped_up = NULL;

  sb_show ();
  return (x);
}

void
sb_show_popup (BINK_SAVEP old, BINK_SAVEP new)
{
  CELLP cellp;
  short top, left, height, width;
  short i, j;

  if (old == NULL)
  {
    if (new != NULL)
      sb_restore (new);
    return;
  }

  cellp = old->save_cells;
  top = old->save_row;
  left = old->save_col;
  height = old->save_ht;
  width = old->save_wid;

  j = top * SB_COLS + left;

  for (i = 0; i < height; i++)
  {
    memcpy (&cellp[i * width], &Scrnbuf[j], width * sizeof (CELL));
    j += SB_COLS;
  }

  if (new != NULL)
    sb_restore (new);
}

BINK_SAVEP
sb_hide_popup (BINK_SAVEP save)
{
  BINK_SAVEP tmp;
  CELLP cellp;
  short top, left, height, width;
  short i, j;

  if (save == NULL)
    return (NULL);

  cellp = save->save_cells;
  top = save->save_row;
  left = save->save_col;
  height = save->save_ht;
  width = save->save_wid;
  tmp = sb_save (top, left, height, width);

  j = top * SB_COLS + left;

  for (i = 0; i < height; i++)
  {
    memcpy (&Scrnbuf[j], &cellp[i * width], width * sizeof (CELL));
    j += SB_COLS;
  }

  return (tmp);
}

INLINEFUNC int
_sb_putc (REGIONP win, short ch, int abs, short attr)
{
  short cmax, rmax;
  int noscroll = 0, puterr = 0;

  if (ch == '\007' && gong_allowed)
  {
    WRITE_ANSI ('\007');        /* TJW 960706 chat request gong */
    return SB_OK;
  }

  /* calculate the screen buffer position and limits */
  if (abs)
  {
    cmax = win->c1 - win->c0;
    rmax = win->r1 - win->r0;
    Sbuf.row = win->r0 + win->row;
    Sbuf.col = win->c0 + win->col;
  }
  else
  {
    cmax = win->sc1 - win->sc0;
    rmax = win->sr1 - win->sr0;
    Sbuf.row = win->sr0 + win->row;
    Sbuf.col = win->sc0 + win->col;
  }

  /* process the character */
  switch (ch)
  {
  case '\b':
    /* TJW 960706 change from nond. to destructive backspace */
    if (win->col > 0)
    {
      --(win->col);
      --(Sbuf.col);
      _sb_putc (win, ' ', abs, attr);
      --(win->col);
      --(Sbuf.col);
      return (SB_OK);
    }
    else
      return (SB_ERR);

  case '\r':
  case '\n':
    if ((ch == '\r' && !(win->wflags & SB_IGNCR))
        || (ch == '\n' && win->wflags & SB_ADDCR))
    {
      // why do we need this:
      //  /* clear trailing line segment */
      //  while (win->col < cmax)
      //  {
      //    if (_sb_putc (win, ' ',abs,attr) == SB_ERR)
      //    {
      //      ++puterr;
      //    }
      //  }
      win->col = 0;
    }

    if ((ch == '\n' && !(win->wflags & SB_IGNLF))
        || (ch == '\r' && win->wflags & SB_ADDLF))
    {
      ++(win->row);
    }

    --(win->col);
    break;

#ifdef TABEXP
  case '\t':
    /* convert tabs to spaces */
    lim = win->col + 8 - (win->col & 7);
    while (win->col < lim)
    {
      if (_sb_putc (win, ' ', abs, -1) == SB_ERR)
      {
        ++puterr;
      }
    }
    break;
#endif /* TABEXP */

  default:
    if (win->col <= cmax)       /* MR 970307 */
    {
      (Scrnbuf + Sbuf.row * SB_COLS + Sbuf.col)->b.ch = (unsigned char) ch;

      if (attr != -1)
        (Scrnbuf + Sbuf.row * SB_COLS + Sbuf.col)->b.attr =
          (((Scrnbuf + Sbuf.row * SB_COLS + Sbuf.col)->b.attr) & 0xf0) |
          (((unsigned char) attr) & 0x0f);

      if (Sbuf.col < Sbuf.lcol[Sbuf.row])
      {
        Sbuf.lcol[Sbuf.row] = Sbuf.col;
      }
      if (Sbuf.col > Sbuf.rcol[Sbuf.row])
      {
        Sbuf.rcol[Sbuf.row] = Sbuf.col;
      }
    }
    break;
  }

  /* update the cursor position */
  if (win->col < cmax)
  {
    ++(win->col);
  }
  else if (win->wflags & SB_WRAP)  /* MR 970307 */
  {
    win->col = 0;
    ++(win->row);
  }
  else
  {                             /* MR 970307 */
    win->col = cmax + 1;
  }

  if (win->row > rmax)          /* MR 970310 */
  {
    if (win->wflags & SB_SCROLL)
    {
      sb_scrl (win, 1);
      win->row = rmax;
    }
    else
    {
      --(win->row);
      ++noscroll;
    }
  }

  /* update screen buffer position */
  if (abs)
  {
    Sbuf.row = win->r0 + win->row;
    Sbuf.col = win->c0 + win->col;
  }
  else
  {
    Sbuf.row = win->sr0 + win->row;
    Sbuf.col = win->sc0 + win->col;
  }

  Sbuf.flags |= SB_DELTA;

  return ((noscroll || puterr) ? SB_ERR : SB_OK);
}

int
sb_putc (REGIONP win, short ch)
{
  return _sb_putc (win, ch, 0, -1);
}

int
sb_putc_att (REGIONP win, short ch, short att)
{
  return _sb_putc (win, ch, 0, att);
}

int
sb_putc_abs (REGIONP win, short ch)
{
  return _sb_putc (win, ch, 1, -1);
}

char *availcolors[] =
{
  "black", "blue", "green", "cyan", "red", "magenta", "brown", "white",
  "gray", "brightblue", "brightgreen", "brightcyan", "brightred",
  "brightmagenta", "yellow", "brightwhite", NULL
};

short
getnewcolor (char **string)
{
  int i;

  for (i = 0; availcolors[i] != NULL; i++)
  {
    if (strstr (*string, availcolors[i]) == *string)
    {
      (*string) += strlen (availcolors[i]);
      if (**string == '`')
        (*string)++;
      return i;
    }
  }

  return -1;
}

char *
stripcolors (char *s)
{
  char *c, *l;

  c = s;
  while (*c)
  {
    if (*c != '`')
      c++;
    else
    {
      if (*(c + 1) == '`')
      {
        strcpy (c, c + 1);      /* Eliminate one of the ` */
        c++;                    /* And skip the other     */
      }
      else
      {
        l = c + 1;
        getnewcolor (&l);
        strcpy (c, l);
      }
    }
  }

  return s;
}

static void
_sb_puts (REGIONP win, char *s, int abs, short defattrib)
{
  while (*s)
  {
    if (*s != '`')              /* CFS 970216 - test for color escape code */
    {
      if (_sb_putc (win, *s++, abs, defattrib) == SB_ERR)
        return;                 /* (SB_ERR); */
    }
    else
    {
      s++;
      if (*s == '`')
      {
        if (_sb_putc (win, *s++, abs, defattrib) == SB_ERR)
          return;               /* (SB_ERR); */
      }
      else
        defattrib = getnewcolor (&s);
    }
  }

  return;                       /* (SB_OK); */
}

void
sb_puts (REGIONP win, char *s)
{
  _sb_puts (win, s, 0, -1);
}

void
sb_puts_abs (REGIONP win, char *s)
{
  _sb_puts (win, s, 1, -1);
}

void
_sb_move_puts (REGIONP win, short row, short col, char *s, int abs)
{
  _sb_move (win, row, col, abs);
  _sb_puts (win, s, abs, -1);
}

void
sb_move_puts (REGIONP win, short row, short col, char *s)
{
  _sb_move_puts (win, row, col, s, 0);

#ifdef OS_2
  sb_send_pm (win, row, col, s);
#endif
}

void
sb_move_puts_abs (REGIONP win, short row, short col, char *s)
{
  _sb_move_puts (win, row, col, s, 1);
}

#if 0                           /* UNUSED ! */
unsigned char
sb_ra (REGIONP win, short r, short c)
{
  return ((unsigned char) (Scrnbuf + (win->r0 + r) * SB_COLS + win->c0 + c)->b.attr);
}

unsigned char
sb_rc (REGIONP win, short r, short c)
{
  return ((unsigned char) (Scrnbuf + (win->r0 + r) * SB_COLS + win->c0 + c)->b.ch);
}

unsigned int
sb_rca (REGIONP win, short r, short c)
{
  return ((unsigned int) (Scrnbuf + (win->r0 + r) * SB_COLS + win->c0 + c)->cap);
}
#endif /* 0 */


/* MR 970517 modified to accept default string                             */
/*           you must set str[0]='\0' if you do not want a default string! */

int
sb_input_chars (REGIONP win, short row, short col, char *str, short maxlen)
{
  short i;
  short fchr;
  long maxwait;
  short dflt;
  short len;
  int done = 0;

#ifdef DOS16

  short rfchr;

#endif

  len = dflt = strlen (str);

  sb_move (win, row, col);

  for (i = 0; i < maxlen; i++)
  {
    if (i < dflt)
      sb_putc (win, str[i]);
    else
      sb_putc (win, '_');
  }

  maxwait = longtimerset (15L * (long) PER_MINUTE);
  sb_move (win, row, col + len);

  do
  {
    sb_show ();

    while (!KEYPRESS () && !longtimeup (maxwait))
    {
      time_release ();
      show_alive ();
    }

    if (longtimeup (maxwait))
    {
      len = 0;                  /* MR 970919 make input invalid after timeout */
      done = 1;                 /* exit */
    }

    maxwait = longtimerset (15L * (long) PER_MINUTE);

    fchr = FOSSIL_CHAR ();
#ifdef DOS16
    rfchr = KbRemap (fchr);
#endif

    if (dflt)                   /* check 1.key if user wants to modify or remove dflt */
    {
#ifdef DOS16
      if ((rfchr == BKS) || (rfchr == LFAR) ||
          (rfchr == CR) || (rfchr == LF))
#else
      if ((fchr == BKS) || (fchr == LFAR) ||
          (fchr == CR) || (fchr == LF))
#endif
        dflt = 0;
      else
      {
        sb_move (win, row, col);
        for (i = 0; i < len; i++)
          sb_putc (win, '_');
        sb_move (win, row, col);
        len = dflt = 0;
      }
    }

    if ((fchr & 0xff) != 0)
    {
      fchr &= 0xff;
      fchr = toupper (fchr);

      if (isprint (fchr))
      {
        if (len < maxlen)
        {
          sb_putc (win, (short) (fchr & 0xff));
          str[len++] = (char) (fchr & 0xff);
        }
        continue;
      }
    }

    switch (fchr)
    {
    case 0x0c00:
      if (len < maxlen)
      {
        sb_putc (win, (short) '\\');
        str[len++] = (char) '\\';
      }
      break;

    case ESC:
      len = 0;
      done = 1;
      break;

    case BKS:
    case LFAR:
      if (len > 0)
      {
        --len;
        sb_move (win, row, col + len);
        sb_putc (win, '_');
        sb_move (win, row, col + len);
      }
      break;

    case CR:
    case LF:
      done = 1;
      break;
    }
  }
  while (!done && !(!dflt && maxlen == 1 && len == 1));

  str[len] = '\0';

  for (i = len; i < maxlen; i++)  /* MR 970919 exit on one char questions */
    sb_putc (win, ' ');         /* if one char was pressed */

  sb_show ();

  if (len)
    return (0);
  else
    return (1);
}

BINK_SAVEP
sb_save (short top, short left, short height, short width)
{
  BINK_SAVEP new;
  CELLP c;
  short i;
  short j;

#ifdef CFSPROBLEM

  short k;
  CELL tc;

#endif

  new = calloc (1, sizeof (BINK_SAVE));
  if (new == NULL)
    return (NULL);

  c = new->save_cells = (CELLP) malloc (sizeof (CELL) * height * width);
  if (c == NULL)
  {
    free (new);
    return (NULL);
  }

  new->region = sb_new (top, left, height, width);
  new->save_row = (short) top;
  new->save_col = (short) left;
  new->save_ht = (short) height;
  new->save_wid = (short) width;

  j = top * SB_COLS + left;
  for (i = 0; i < height; i++)
  {
#ifdef CFSPROBLEM
    for (k = 0; k < width; k++)
    {
      tc.b = Scrnbuf[j + k].b;
      tc.cap = Scrnbuf[j + k].cap;
      c[i * width + k].b = tc.b;
      c[i * width + k].cap = tc.cap;
    }
#else
    memcpy (&c[i * width], &Scrnbuf[j], width * sizeof (CELL));
#endif
    j += SB_COLS;
  }

  return (new);
}

void
sb_restore (BINK_SAVEP save)
{
  short i;
  short j;
  short r;

#ifdef CFSPROBLEM

  short k;
  CELL ct;

#endif

  if (save == NULL)
    return;

  j = save->save_row * SB_COLS + save->save_col;
  for (r = save->save_row, i = 0; i < save->save_ht; r++, i++)
  {
#ifdef CFSPROBLEM
    for (k = 0; k < save->save_wid; k++)
    {
      ct.b = save->save_cells[i * save->save_wid + k].b;
      ct.cap = save->save_cells[i * save->save_wid + k].cap;
      Scrnbuf[j + k].b = ct.b;
      Scrnbuf[j + k].cap = ct.cap;
    }
#else
    memcpy (&Scrnbuf[j], &(save->save_cells[i * save->save_wid]), save->save_wid * sizeof (CELL));
#endif
    j += SB_COLS;

    if (save->save_col < Sbuf.lcol[r])
      Sbuf.lcol[r] = save->save_col;

    if (save->save_col + save->save_wid > Sbuf.rcol[r])
      Sbuf.rcol[r] = save->save_col + save->save_wid;
  }

  Sbuf.flags |= SB_DELTA;

  free ((char *) (save->save_cells));
  free (save->region);
  free (save);
}

void
sb_scrl (REGIONP win, short n)
{
  register short r, c;

  c = win->sc0;
  if (n == 0)
  {
    /* clear the entire region to spaces */
    sb_fill (win, ' ', win->lastfillattr);  /* CFS 970216 - was sb_fillc() */
  }
  else if (n > 0)
  {
    /* scroll n rows up */
    for (r = win->sr0; r <= win->sr1 - n; r++)
    {
      memcpy (Scrnbuf + r * SB_COLS + c, Scrnbuf + (r + n) * SB_COLS + c,
              (unsigned short) (win->sc1 - win->sc0 + 1) * 2);

      if (win->sc0 < Sbuf.lcol[r])
        Sbuf.lcol[r] = win->sc0;

      if (win->sc1 > Sbuf.rcol[r])
        Sbuf.rcol[r] = win->sc1;
    }
    for (; r <= win->sr1; r++)
    {
      for (c = win->sc0; c <= win->sc1; c++)
      {
        (Scrnbuf + r * SB_COLS + c)->b.ch = ' ';
        /* CFS 970216 - attributes for the empty line can't be left
           untouched! */
        (Scrnbuf + r * SB_COLS + c)->b.attr = (char) win->lastfillattr;
      }

      if (win->sc0 < Sbuf.lcol[r])
        Sbuf.lcol[r] = win->sc0;

      if (win->sc1 > Sbuf.rcol[r])
        Sbuf.rcol[r] = win->sc1;
    }
  }
  else
  {
    /* scroll n rows down */
    n = -n;
    for (r = win->sr1; r >= win->sr0 + n; r--)
    {
      memcpy (Scrnbuf + r * SB_COLS + c, Scrnbuf + (r - n) * SB_COLS + c,
              (unsigned short) (win->sc1 - win->sc0 + 1) * 2);
      if (win->sc0 < Sbuf.lcol[r])
        Sbuf.lcol[r] = win->sc0;

      if (win->sc1 > Sbuf.rcol[r])
        Sbuf.rcol[r] = win->sc1;
    }
    for (; r >= win->sr0; r--)
    {
      for (c = win->sc0; c <= win->sc1; c++)
      {
        (Scrnbuf + r * SB_COLS + c)->b.ch = ' ';
        /* CFS 970216 - attributes for the empty line can't be left
           untouched! */
        (Scrnbuf + r * SB_COLS + c)->b.attr = (char) win->lastfillattr;
      }

      if (win->sc0 < Sbuf.lcol[r])
        Sbuf.lcol[r] = win->sc0;

      if (win->sc1 > Sbuf.rcol[r])
        Sbuf.rcol[r] = win->sc1;
    }
  }

  Sbuf.flags |= SB_DELTA;

  return;
}

void
sb_show ()
{
  static long wait = 0L;
  register short r;
  unsigned short src_os;
  char far *q;

  if (screen_blank && do_screen_blank)
  {
    if (!timeup (wait))         /* do it ONCE per second! (r.hoerner) */
      return;

    if (clock_minute == -1)
      for (r = 0; r < SB_COLS; r++)
        blanks[r * 2 + 1] = clock_color;

    wait = timerset (PER_SECOND);

    for (r = 0; r < SB_ROWS; r++)
    {
      q = (char far *) blanks;
      if (!DrawClock || update_clock (r))
        VioWrtCellStr ((PCH) q, (USHORT) (SB_COLS * 2), (USHORT) r,
                       (USHORT) 0, (HVIO) 0L);
    }

    sb_dirty ();
    return;
  }
  else if (clock_minute != -1)
  {
    clock_minute = -1;
    for (r = 0; r < SB_COLS; r++)
    {
      blanks[r * 2] = ' ';
      blanks[r * 2 + 1] = 7;
    }
  }

  /* Anything to do? */
  if (!(Sbuf.flags & SB_DELTA))
  {
    return;
  }

  src_os = 0;
  for (r = 0; r < SB_ROWS; r++)
  {
    /* Copy only changed portions of lines */
    if ((Sbuf.lcol[r] < SB_COLS) && (Sbuf.rcol[r] > 0))
    {
      q = (char far *) (Scrnbuf + src_os + Sbuf.lcol[r]);
      VioWrtCellStr ((PCH) q,
                     (USHORT) ((Sbuf.rcol[r] - Sbuf.lcol[r] + 1) * 2),
                     (USHORT) r, (USHORT) Sbuf.lcol[r], (HVIO) 0L);
      Sbuf.lcol[r] = SB_COLS;
      Sbuf.rcol[r] = 0;
    }

    src_os += (unsigned short) SB_COLS;
  }

  /* the display now matches the buffer -- clear flag bit */
  Sbuf.flags &= ~SB_DELTA;

  /* Put sanity check on cursor_row and cursor_col here */
  if (cursor_row < 0 || cursor_row > (short) (SB_ROWS - 1))
    cursor_row = SB_ROWS - 1;

  if (cursor_col < 0 || cursor_col > (short) (SB_COLS - 1))
    cursor_col = SB_COLS - 1;

  gotoxy (cursor_col, cursor_row);
  return;
}

/*
 * Just cleans up the structure to say it is reality - I can use this when
 * I write directly to the screen for single char writes.
 */

void
sb_clean ()
{
  short r;

  for (r = 0; r < (short) SB_ROWS; r++)
  {
    Sbuf.lcol[r] = SB_COLS;
    Sbuf.rcol[r] = 0;
  }

  Sbuf.flags &= ~SB_DELTA;
}

/*
 * Make the entire buffer "dirty" so it will be updated.
 */

void
sb_dirty ()
{
  short r;

  for (r = 0; r < (short) SB_ROWS; r++)
  {
    Sbuf.lcol[r] = 0;
    Sbuf.rcol[r] = SB_COLS_M_1;
  }

  Sbuf.flags |= SB_DELTA;
}

static void
_sb_wa (REGIONP win, short attr, short n, int abs)
{
  short i;
  short row;
  short col;

  if (abs)
  {
    row = win->r0 + win->row;
    col = win->c0 + win->col;
  }
  else
  {
    row = win->sr0 + win->row;
    col = win->sc0 + win->col;
  }

  i = n;
  while (i--)
    (Scrnbuf + row * SB_COLS + col + i)->b.attr = (unsigned char) attr;

  /* marked the changed region */
  if (col < Sbuf.lcol[row])
    Sbuf.lcol[row] = col;

  if (col + n > Sbuf.rcol[row])
    Sbuf.rcol[row] = col + n;

  Sbuf.flags |= SB_DELTA;

  return;                       /* ((i == 0) ? SB_OK : SB_ERR); */
}

void
sb_wa (REGIONP win, short attr, short n)
{
  _sb_wa (win, attr, n, 0);
}

void
sb_wa_abs (REGIONP win, short attr, short n)
{
  _sb_wa (win, attr, n, 1);
}

static void
_sb_wc (REGIONP win, short ch, short n, int abs)
{
  short i;
  short row;
  short col;

  if (abs)
  {
    row = win->r0 + win->row;
    col = win->c0 + win->col;
  }
  else
  {
    row = win->sr0 + win->row;
    col = win->sc0 + win->col;
  }

  i = n;
  while (i--)
    (Scrnbuf + row * SB_COLS + col + i)->b.ch = (unsigned char) ch;

  /* marked the changed region */
  if (col < Sbuf.lcol[row])
    Sbuf.lcol[row] = col;

  if (col + n > Sbuf.rcol[row])
    Sbuf.rcol[row] = col + n;

  Sbuf.flags |= SB_DELTA;

  return;                       /* ((i == 0) ? SB_OK : SB_ERR); */
}

void
sb_wc (REGIONP win, short ch, short n)
{
  _sb_wc (win, ch, n, 0);
}

void
sb_wc_abs (REGIONP win, short ch, short n)
{
  _sb_wc (win, ch, n, 1);
}

static void
_sb_wca (REGIONP win, short ch, short attr, short n, int abs)
{
  short i;
  short row;
  short col;
  unsigned short ca;

  if (abs)
  {
    row = win->r0 + win->row;
    col = win->c0 + win->col;
  }
  else
  {
    row = win->sr0 + win->row;
    col = win->sc0 + win->col;
  }

  ca = (((unsigned short) attr) << 8) | (unsigned short) ch;
  i = n;

  while (i--)
    (Scrnbuf + row * SB_COLS + col + i)->cap = ca;

  /* marked the changed region */
  if (col < Sbuf.lcol[row])
    Sbuf.lcol[row] = col;

  if (col + n > Sbuf.rcol[row])
    Sbuf.rcol[row] = col + n;

  Sbuf.flags |= SB_DELTA;

  return;                       /* ((i == 0) ? SB_OK : SB_ERR); */
}

void
sb_wca (REGIONP win, short ch, short attr, short n)
{
  _sb_wca (win, ch, attr, n, 0);
}

void
sb_wca_abs (REGIONP win, short ch, short attr, short n)
{
  _sb_wca (win, ch, attr, n, 1);
}

/* RDH *** 26-Apr-96 begin */
/* funktion to get a filename (with Drive and Path) */
/* used in Do_Send  insted of sb_input_chars();     */
/* MR 980923 added useddfind to close dfind handle  */
int
sb_input_fname (REGIONP win, short row, short col, char *str, short len)
{
  char *string;                 /* Pointer to input/output string */
  char inpstring[128];          /* userbuffer hold the input      */
  char okstring[128];           /* string without expansion       */
  short inexpand;               /* last key was expand-key (TAB)  */
  int useddfind;                /* we used dfind                  */
  short i;                      /* user pos in string             */
  short j;                      /* input char                     */
  short slen;                   /* len of string                  */
  struct FILEINFO fileinfo;     /* to use dfind                   */
  int rc;                       /* returnvalue of dfind           */
  short oldcol;                 /* to hold old col                */

  /* first do draw a line */
  sb_move (win, row, col);
  for (i = 0; i < len; i++)
    sb_putc (win, '_');

  /* set cursor to begin of line */
  sb_move (win, row, col);

  /* original pointer to in/out string */
  string = str;
  oldcol = col;
  inexpand = FALSE;             /* we are not in expand mode      */
  useddfind = FALSE;            /* and we didn't use dfind        */

  i = 0;
  while (i < len)
  {
    sb_show ();

    while (!KEYPRESS ())
    {
      time_release ();
      show_alive ();
    }

    j = FOSSIL_CHAR ();
    if ((j & 0xff) != 0)
    {
      j &= 0xff;
      j = f_toupper (j);
      if (isprint (j))
      {
        inexpand = FALSE;
        sb_putc (win, (short) (j & 0xff));
        *str = (char) (j & 0xff);
        ++str;
        ++i;
        ++col;
        continue;
      }
    }

    switch (j)
    {
    case TAB:
      if (inexpand)
      {
        slen = strlen (okstring);
        while ((slen >= 0) && (okstring[slen] != DIR_SEPC) && (okstring[slen] != ':'))
        {
          okstring[slen] = '\0';
          slen--;
        }

        rc = dfind (&fileinfo, inpstring, 1);
        while (!rc && ((0 == strcmp (fileinfo.name, ".")) || (0 == strcmp (fileinfo.name, ".."))))
        {
          rc = dfind (&fileinfo, inpstring, 1);
        }

        if (!rc)
        {
          strcat (okstring, fileinfo.name);
          sb_move (win, row, oldcol);

          for (i = 0; i < len; i++)
            sb_putc (win, '_');

          sb_move (win, row, oldcol);
          sb_puts (win, okstring);
          i = strlen (okstring);
          sb_move (win, row, oldcol + i);
          col = oldcol + i;
          strcpy (string, okstring);
          str = string + i;
        }
      }
      else
      {
        if (useddfind)          /* CEH990113: close before using again, too */
          dfind (&fileinfo, NULL, 2);
        inexpand = TRUE;
        useddfind = TRUE;
        string[i] = '\0';
        strcpy (inpstring, string);

        if ((NULL == strchr (inpstring, '*')))
        {
#ifdef DOS16
          strcat (inpstring, "*.*");
#else
          strcat (inpstring, "*");
#endif
        }

        strcpy (okstring, string);
        slen = strlen (okstring);
        while ((slen >= 0) && (okstring[slen] != DIR_SEPC) && (okstring[slen] != ':'))
        {
          okstring[slen] = '\0';
          slen--;
        }

        rc = dfind (&fileinfo, inpstring, 0);
        while (!rc && ((0 == strcmp (fileinfo.name, ".")) || (0 == strcmp (fileinfo.name, ".."))))
        {
          rc = dfind (&fileinfo, inpstring, 1);
        }

        if (!rc)
        {
          strcat (okstring, fileinfo.name);
          sb_move (win, row, oldcol);
          for (i = 0; i < len; i++)
            sb_putc (win, '_');
          sb_move (win, row, oldcol);
          sb_puts (win, okstring);
          i = strlen (okstring);
          sb_move (win, row, oldcol + i);
          col = oldcol + i;
          strcpy (string, okstring);
          str = string + i;
        }
      }
      break;

    case 0x0c00:               /* German NT BACKSLASH */
      inexpand = FALSE;
      sb_putc (win, (short) DIR_SEPC);
      *str = (char) DIR_SEPC;
      ++str;
      ++i;
      ++col;
      break;

    case ESC:
      inexpand = FALSE;
      *str = '\0';              /* MR 971127 added */
      if (useddfind)            /* MR 980923 if we did use dfind, close it now */
        dfind (&fileinfo, NULL, 2);
      return (0);

    case BKS:
    case LFAR:
      inexpand = FALSE;
      if (i > 0)
      {
        --col;
        sb_move (win, row, col);
        sb_putc (win, '_');
        sb_move (win, row, col);
        --str;
        --i;
      }
      break;

    case CR:
    case LF:
      inexpand = FALSE;
      *str = '\0';

      for (j = i; j < len; j++)
        sb_putc (win, ' ');

      if (useddfind)            /* MR 980923 if we did use dfind, close it now */
        dfind (&fileinfo, NULL, 2);

      if (i)
        return (0);
      else
        return (1);

    }
  }

  if (useddfind)                /* CEH990113: close it. */
    dfind (&fileinfo, NULL, 2);

  *str = '\0';
  sb_show ();
  return (0);
}
/* RDH *** 26-Apr-96 end */

/* $Id: sbuf.c,v 1.7 1999/03/22 03:47:39 mr Exp $ */
