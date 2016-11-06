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
 * Filename    : $Source: E:/cvs/btxe/src/b_sbinit.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:36 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Initial Fullscreen Setup
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"
#include "video.h"

extern VIOMODEINFO vfos_mode;

static void readactivitydump (void);
void writeactivitydump (void);

#define MAKE_WIN(a,b,c,d,e,s) sb_new_win (a,b,c,d,' ',e,boxtype,colors.frames,MSG_TXT(s))

void
top_line (void)
{
  char *junk;

  junk = (char *) malloc (SB_COLS + 1);
  memset (junk, ' ', SB_COLS);
  junk[SB_COLS] = 0;
  sb_move_puts (wholewin, 0, 0, junk);
  sprintf (junk, MSG_TXT (M_NODE_COLON), TaskNumber, Full_Addr_Str (&my_addr));
  sb_move_puts (wholewin, 0, 0, junk);
  sb_move_puts (wholewin, 0, (short) (SB_COLS - (short) strlen (system_name)), system_name);
  free (junk);
}

void
b_sbinit (void)
{
  register int r;
  char junk[64];
  BOXTYPE *boxp;

  vfossil_init ();
  if (vfos_mode.col < 80)
    vfos_mode.col = 80;
#ifdef DOS16                    /*  TS 961214   */
  if (!vfossil_installed)
    vfos_mode.row = getbiosscreenrows ();
#endif
  if (vfos_mode.row < 24)       /*  TS 961214   */
    vfos_mode.row = 24;         /*  TS 961214   */

  SB_ROWS = vfos_mode.row;      /* TJW 960506 no "- 1" */
  SB_COLS = vfos_mode.col;

  SB_COLS_M_1 = SB_COLS - 1;

  /* TJW 960723 removed because unused ...
     SB_ROWS_M_1 = SB_ROWS - 1;
     SB_ROWS_M_2 = SB_ROWS - 2;
   */

  if (blanks == NULL)
    blanks = (char far *) _fmalloc (2 * SB_COLS);  /* now dynamic */

  if (blanks == NULL)           /* r.hoerner 15 jan 97 */
  {
    printf (MSG_TXT (M_MEM_ERROR));
    exit (252);
  }

  for (r = 0; r < SB_COLS; r++)
  {
    blanks[r * 2] = ' ';
    blanks[r * 2 + 1] = 7;
  }

  screen_clear ();
  sb_init ();

  wholewin = sb_new (0, 0, SB_ROWS, SB_COLS);
  if (wholewin == NULL)
  {
    printf (MSG_TXT (M_MEM_ERROR));
    exit (252);
  }

  sb_fill (wholewin, ' ', colors.background);
  sprintf (junk, MSG_TXT (M_NODE_COLON), TaskNumber, Full_Addr_Str (&my_addr));
  top_line ();
  bottom_line (1);

  settingswin = MAKE_WIN (SETTINGS_ROW, SETTINGS_COL, SETTINGS_HIGH, SETTINGS_SIZE, colors.settings, M_CURRENT_SETTINGS);
  historywin = MAKE_WIN (HISTORY_ROW, HISTORY_COL, HISTORY_HIGH, HISTORY_SIZE, colors.history, M_TODAY_GLANCE);
  holdwin = MAKE_WIN (HOLDWIN_ROW, HOLDWIN_COL, HOLDWIN_HIGH, HOLDWIN_SIZE, colors.hold, M_PENDING_OUTBOUND);
  callwin = MAKE_WIN (CALLWIN_ROW, CALLWIN_COL, CALLWIN_HIGH, CALLWIN_SIZE, colors.call, M_RECENT_ACTIVITY);
  filewin = MAKE_WIN (FILEWIN_ROW, FILEWIN_COL, FILEWIN_HIGH, FILEWIN_SIZE, colors.file, M_TRANSFER_STATUS);

  if (!settingswin || !historywin || !holdwin || !callwin || !filewin)
  {
    printf (MSG_TXT (M_MEM_ERROR));
    exit (2);
  }

  sb_move_puts (settingswin, SET_EVNT_ROW, 0, MSG_TXT (M_EVENT_COLON));
  sb_move_puts (settingswin, SET_PORT_ROW, 0, MSG_TXT (M_PORT_COLON));
  sb_move_puts (settingswin, SET_BAUD_ROW, 0, MSG_TXT (M_STATUS_DTERATE));
  sb_move_puts (settingswin, SET_STAT_ROW, 0, MSG_TXT (M_STATUS_INIT));
  sb_move_puts (settingswin, SET_SESSION_ROW, 0, MSG_TXT (M_SESSION));
  sb_move_puts (settingswin, SET_PROTOCOL_ROW, 0, MSG_TXT (M_PROTOCOL));
  sb_move_puts (settingswin, SET_MAIL_ROW, 0, MSG_TXT (M_UNREAD_NETMAIL));
  sb_move_puts (settingswin, SET_MEMAVAIL_ROW, 0, MSG_TXT (M_MEM_AVAIL));
  show_os_memfree ();

  do_today (0);

  sb_move_puts (holdwin, 2, 6, MSG_TXT (M_INITIALIZING_SYSTEM));

  if (scrllines < callwin->lines * 2)  // TJW 970604 have at least 2 times as  

    scrllines = callwin->lines * 2;  // much scrollback buffer as screenlines

  callwin->linesize += 2;       /* add one for \0 and 1 for padding */
  callwin->buffer = calloc (scrllines * callwin->linesize, 1);

  if (callwin->buffer == NULL)  /* TJW 960606 are we running under DOS ? ;-) */
  {
    printf (MSG_TXT (M_MEM_ERROR));
    exit (2);
  }

  callwin->endbuff = callwin->buffer + ((scrllines - 1) * callwin->linesize);
  callwin->lastline = callwin->buffer;
  callwin->lastshown = callwin->lastline;

  boxp = &box[boxtype];

  sprintf (junk, "%c", boxp->tt);
  sb_move_puts_abs (historywin, 0, 0, junk);  /* MR 970216 a little bit */
  sb_move_puts_abs (holdwin, 0, 0, junk);  /*           more easy... */
  sprintf (junk, "%c", boxp->t270);
  sb_move_puts_abs (historywin, HISTORY_HIGH - 1, 0, junk);
  sb_move_puts_abs (filewin, 0, 0, junk);
  sprintf (junk, "%c", boxp->t90);
  sb_move_puts_abs (holdwin, HOLDWIN_HIGH - 1, HOLDWIN_SIZE - 1, junk);
  sprintf (junk, "%c", boxp->tb);
  sb_move_puts_abs (holdwin, HOLDWIN_HIGH - 1, 0, junk);
  sb_move_puts_abs (filewin, FILEWIN_HIGH - 1, 0, junk);

  readactivitydump ();
  initialized = 1;
}

void
b_sbexit ()
{
  writeactivitydump ();
}

#define DUMPMAGIC 0x19661220L

static void
readactivitydump (void)
{
  char fname[PATHLEN];
  FILE *fp;
  int len;
  int saved_scrllines = 0;
  short saved_linesize = 0;
  long savedmagic;
  unsigned int savedlastline, savedlastshown;

  sb_fillc (callwin, ' ');

  sprintf (fname, "%s%s.a%02x", task_dir, PRDCT_PRFX, TaskNumber);

  if ((fp = fopen (fname, read_binary)) != NULL)
  {
    savedmagic = 0L;
    fread (&savedmagic, sizeof (savedmagic), 1, fp);
    if (savedmagic != DUMPMAGIC)
      goto end;

    fread (&saved_scrllines, sizeof (saved_scrllines), 1, fp);
    if (saved_scrllines != scrllines)
      goto end;

    fread (&saved_linesize, sizeof (saved_linesize), 1, fp);
    if (saved_linesize != callwin->linesize)
      goto end;

    fread (&savedlastline, sizeof (savedlastline), 1, fp);
    callwin->lastline = callwin->buffer + savedlastline;

    fread (&savedlastshown, sizeof (savedlastshown), 1, fp);
    callwin->lastshown = callwin->buffer + savedlastshown;

    len = scrllines * callwin->linesize;
    fread (callwin->buffer, len, 1, fp);

    savedmagic = 0L;
    fread (&savedmagic, sizeof (savedmagic), 1, fp);
    if (savedmagic != DUMPMAGIC)
      goto end;

    redraw_callwin ();

  end:
    fclose (fp);
  }
}

void
redraw_callwin (void)
{
  char *p;
  short j;

  if (scrllines > callwin->lines)
  {
    sb_scrl (callwin, 0);

    /* TJW970209 please check if this is ok with 16bit version ! */
    p = callwin->lastshown;
    p -= (callwin->lines - 1) * callwin->linesize;
    for (j = 0; j < callwin->lines; j++)
    {
      if (p >= callwin->buffer)
        sb_move_puts (callwin, j, 0, p);
      p += callwin->linesize;
    }
  }
}

void
writeactivitydump (void)
{
  char fname[PATHLEN];
  FILE *fp;
  int len;
  unsigned int offset;
  long magic = DUMPMAGIC;

  sprintf (fname, "%s%s.a%02x", task_dir, PRDCT_PRFX, TaskNumber);

  if ((fp = fopen (fname, write_binary)) != NULL)
  {
    fwrite (&magic, sizeof (magic), 1, fp);
    fwrite (&scrllines, sizeof (scrllines), 1, fp);
    fwrite (&callwin->linesize, sizeof (callwin->linesize), 1, fp);
    offset = (unsigned int) (callwin->lastline - callwin->buffer);
    fwrite (&offset, sizeof (offset), 1, fp);
    offset = (unsigned int) (callwin->lastshown - callwin->buffer);
    fwrite (&offset, sizeof (offset), 1, fp);
    len = scrllines * callwin->linesize;
    fwrite (callwin->buffer, len, 1, fp);
    fwrite (&magic, sizeof (magic), 1, fp);
    fclose (fp);
  }
}

/* $Id: b_sbinit.c,v 1.7 1999/03/23 22:28:36 mr Exp $ */
