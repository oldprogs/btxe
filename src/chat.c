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
 * Filename    : $Source: E:/cvs/btxe/src/chat.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:23 $
 * State       : $State: Exp $
 *
 * Description : Hydra Chat
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */
#include "includes.h"

/****************************************************************************/
/*  local static data                                                       */
/****************************************************************************/
static REGIONP callwinsave;
static REGIONP chatwinl, chatwinr;

static char *chatstart = "\007\007 * Chat mode start\r\n";
static char *chatend = "\007\007\r\n * Chat mode end\r\n";
static char *chattime = "\007\007\r\n * Chat mode timeout\r\n";

char HaveChat_Bell[] = " * Remote has chat facility (bell enabled)\n";
char HaveChat_NoBell[] = " * Remote has chat facility (bell disabled)\n";
char RemoteHasChat[] = " has chat ";

// MR 970329 not used
// char RemoteHasNoChat[] = " has no chat ";

static word chatfill;
static long lasttimer;
static BOOL msgsent = FALSE;
static BOOL chatting = FALSE;

extern REGIONP modemwin;        /* AS 970315 */

char RemInitials[40], LocInitials[40];

/****************************************************************************/
/*  local prototypes                                                        */
/****************************************************************************/

void chatautostart (void);

/****************************************************************************/
/*  functions                                                               */
/****************************************************************************/

#if 0

#ifdef DOS16                    /* MR 961122 changed DOS to DOS16 */
#define CHATBUFSIZE 256
#else
#define CHATBUFSIZE HYDRATXBUFSIZE
#endif

static char buffer1[CHATBUFSIZE], buffer2[CHATBUFSIZE];  /* chat buffers */
static int bufpos = 0;

void
chat_prepare (void)
{
  bufpos = 0;
}

#define CHATDELAY    30         /* how many 10ms units min. delay between packets */

int
chat_putc (int c)
{
  static long flushtimer;
  int ret = 0;

  if (!nohydrachat)
  {
    if (c >= 0)
    {
      if (bufpos > 0 && (c == '\n' || c == ' '))  /* if this is not the 1st char and we */
        flushtimer = timerset (1);  /* have a blank/newline, don't wait long */
      else                      /* do not immediately send char, but    */
        flushtimer = timerset (CHATDELAY);  /* wait for some others maybe following */
      if (bufpos < CHATBUFSIZE)
      {
        buffer1[bufpos++] = (char) c;
        ret = 0;
      }
      else
        ret = -1;
    }

    if (bufpos && hydra_devfree () && (timeup (flushtimer) || (bufpos > CHATBUFSIZE / 2)))
    {
      strncpy (buffer2, buffer1, bufpos);
      hydra_devsend ("CON", buffer2, bufpos);
      bufpos = 0;
    }
  }
  return ret;
}

void
chat_puts (char *s)
{
  if (!nohydrachat)
    while (*s)
      chat_putc (*s++);
}

#endif /* 0 */

void
chatsendstring (char *s)
{
  hydra_devsend ("CON", (byte *) s, (word) strlen (s));
  sb_puts (chatwinl, s);
}

void
chatautostart (void)
{
  if (autochatopen && chattimer == 0L)
  {
    chat_init ();
    sb_puts (chatwinl, &chatstart[2]);
    css_upd ();
    chattimer = lasttimer = unix_time (NULL) + CHAT_TIMEOUT;
  }
}

int
chatprocess (char *sorig, int islocal)
{
  int isfor = 2;
  char *c, *s, fn[80], line[80], *back;

  back = s = (char *) malloc (strlen (sorig) + 1);

  if (s == NULL)
    return 0;

  strcpy (s, sorig);
  while ((c = strchr (s, '\n')) != NULL)
    strcpy (c, c + 1);
  while ((c = strchr (s, '\r')) != NULL)
    strcpy (c, c + 1);
  while ((c = strchr (s, 7)) != NULL)
    strcpy (c, c + 1);

  if (ChatLogHandle)
    fprintf (ChatLogHandle, "%s>%s\n", islocal ? LocInitials : RemInitials, s);

  if (strstrci (s, "/btloc") == s)
    isfor = 1;
  else if (strstrci (s, "/btrem") == s)
    isfor = 0;

  if (islocal != isfor)         /* Ignore btloc from remote and btrem from local */
  {
    free (s);
    return 0;
  }
  s += 6;

  while (*s == ' ')
    strcpy (s, s + 1);

  if (strstrci (s, "log") == s)
  {
    if (islocal)
    {
      if (!ChatLogDir)
        status_line ("Directory for chat logs not defined, can't log.");
      else if (ChatLogHandle == NULL)
      {
        c = skip_to_word (s + 3);
        if (!*c)
#if defined(OS_2) | defined(_WIN32)
          sprintf (fn, "%s%s.CHT", ChatLogDir, remote_sysop);
#else
          sprintf (fn, "%s%s.CHT", ChatLogDir, RemInitials);
#endif
        else
          sprintf (fn, "%s%s", ChatLogDir, c);

        ChatLogHandle = share_fopen (fn, append_ascii, DENY_WRITE);

        if (ChatLogHandle == NULL)
          sprintf (line, "Warning: Failed to open chat log file %s", fn);
        else
          sprintf (line, "\nNow logging to %s\n", fn);

        chatsendstring (line);
      }
      else
      {
        fclose (ChatLogHandle);
        status_line ("Chat log file closed.");
      }
    }
    else
      chatsendstring ("\nLog is only available for the local user.\n");
  }

  status_line (":Received command: %s", s);
  free (back);
  return 0;
}


void
chat_prepare (void)
{
  chatfill = 0;
  chattimer = -1L;
  lasttimer = 0L;
  msgsent = FALSE;
  chatting = FALSE;
  RemCommandBuf[0] = LocCommandBuf[0] = 0;  /* CFS 970426 */
}

void
chat_init (void)
{
  short rows, rows_call, rows_chl, rows_chr;
  short chatwinl_size, chatwinr_size;  /* AS 960315 */
  char junk[256], *c;

  /* ---------------------------------------------------------------------------- */
  /* Init. initials for chat logging (for example CFS for Carlos Fernandez Sanz) */
  /* ---------------------------------------------------------------------------- */
  LocInitials[0] = RemInitials[0] = 0;

  c = remote_sysop;
  while (c)
  {
    if (!*c)
      break;
    if (*c != ' ')
      sprintf (RemInitials + strlen (RemInitials), "%c", *c);
    c = skip_to_word (c);
  }

  c = sysop;
  while (c)
  {
    if (!*c)
      break;
    if (*c != ' ')
      sprintf (LocInitials + strlen (LocInitials), "%c", *c);
    c = skip_to_word (c);
  }

  if (!nohydrachat && un_attended && !do_chat)
  {
    chatting = TRUE;
    RemCommandBuf[0] = LocCommandBuf[0] = 0;  /* CFS 970426 */

    hydra_devsend ("CON", (byte *) chatstart, (word) strlen (chatstart));

    callwinsave = callwin;

    rows = CALLWIN_HIGH;
    if (rows < 21)
      rows_call = 4;
    else
      rows_call = 10;
    rows -= rows_call;
    rows_chl = rows / 2;
    rows_chr = rows - rows_chl;

    sprintf (junk, MSG_TXT (M_CHAT_WINLOCAL), sysop);

    /*
     * AS 970315, check for existing modemwin and calculate the proper size
     * for the two chat windows and the new (reduced in size) callwin.
     */

    if (modemwin)
    {
      chatwinl_size = SB_COLS - (modemwin->sc1 - modemwin->sc0) - 2;
      chatwinr_size = chatwinl_size;
    }
    else
      chatwinl_size = chatwinr_size = SB_COLS;

    chatwinl = sb_new_win (TOP_PARTS_ROWS, 0,
                           rows_chl, (short) chatwinl_size,
                           ' ', colors.call,
                           boxtype, colors.frames,
                           junk);

    chatwinl->wflags = 0;
    chatwinl->wflags |= SB_SCROLL | SB_WRAP | SB_ADDCR | SB_ADDLF;
    /* MR 970512 we ONLY get LFs and must add CRs */

    sprintf (junk, MSG_TXT (M_CHAT_WINREMOTE), remote_sysop);

    chatwinr = sb_new_win (TOP_PARTS_ROWS + rows_chl, 0,
                           rows_chr, (short) chatwinr_size,
                           ' ', colors.call,
                           boxtype, colors.frames,
                           junk);

    chatwinr->wflags = 0;
    chatwinr->wflags |= SB_SCROLL | SB_WRAP | SB_ADDCR | SB_ADDLF;
    /* MR 970512 we ONLY get LFs and must add CRs */

    callwin = sb_new_win ((short) (TOP_PARTS_ROWS + rows_chl + rows_chr), 0,
                          rows_call, (short) chatwinl_size,
                          ' ', colors.call,
                          boxtype, colors.frames,
                          MSG_TXT (M_RECENT_ACTIVITY));
    callwin->linesize = callwinsave->linesize;
    callwin->buffer = callwinsave->buffer;
    callwin->endbuff = callwinsave->endbuff;
    callwin->lastline = callwinsave->lastline;
    callwin->lastshown = callwinsave->lastshown;

    sb_box (callwin, boxtype, colors.frames);
    sb_caption (callwin, MSG_TXT (M_RECENT_ACTIVITY), colors.frames);
    redraw_callwin ();

    if (modemwin)               /* MR 970327 */
      ctrl_modemwin (1);

    do_chat = 1;
  }
}

void
chat_put_remote (char *s)
{
  char buf[150];
  char *d;
  int i;

  if (!nohydrachat)
  {
    d = s;
    buf[0] = 0;
    // status_line (":Entered with [%s]",s);
    while (*d)
    {
      // sprintf (buf+strlen (buf),"%02X ",*d);
      switch (*d)
      {
      case '\n':
        /* status_line (":Remote sent %s",RemCommandBuf); */
        chatprocess (RemCommandBuf, 0);
        RemCommandBuf[0] = 0;
        break;

      case '\b':
        if (strlen (RemCommandBuf))
          RemCommandBuf[strlen (LocCommandBuf) - 1] = 0;
        break;

      default:
        RemCommandBuf[strlen (RemCommandBuf) + 1] = 0;
        RemCommandBuf[strlen (RemCommandBuf)] = *d;
        break;
      }

      d++;
    }
    // status_line (":Hex: %s",buf);

    if (chatwinr)
    {
      sb_puts (chatwinr, s);
      css_upd ();
    }
    else
    {
      if (!chatting)
      {
        if (strstr (s, RemoteHasChat))  /* update remote sysop info */
          show_nodes_name (" (Chat) ");  /* in "recent activity"     */
      }

      while (*s)
      {
        strcpy (buf, "*HCON: ");
        i = 0;
        d = buf + 7;
        while (*s && (*s != '\n') && (i < 132))
        {                       /* break up into lines */
          *d++ = *s++;
          i++;
        }
        if (*s == '\n')
          s++;
        *d = '\0';
        status_line (buf);
      }
    }
  }
}

void
chat_exit (void)
{
  chatting = FALSE;
  if (!nohydrachat && un_attended && do_chat)
  {
    hydra_devsend ("CON", (byte *) chatend, (word) strlen (chatend));
    sb_puts (chatwinl, &chatend[2]);
    do_chat = 0;
    callwinsave->linesize = callwin->linesize;
    callwinsave->buffer = callwin->buffer;
    callwinsave->endbuff = callwin->endbuff;
    callwinsave->lastline = callwin->lastline;
    callwinsave->lastshown = callwin->lastshown;

    /* MR 970215 added free; shouldn't we give back allocated memory!? */
    free (callwin);
    free (chatwinr);
    free (chatwinl);

    callwin = callwinsave;
    chatwinr = NULL;
    chatwinl = NULL;

    sb_fill (callwin, ' ', colors.call);  /* MR 970307 sb_filla -> sb_fill */
    sb_box (callwin, boxtype, colors.frames);
    sb_caption (callwin, MSG_TXT (M_RECENT_ACTIVITY), colors.frames);
    redraw_callwin ();

    if (modemwin)               /* MR 970327 */
      ctrl_modemwin (1);
  }
}

#if 0
int
chat_get_key (void)
{
  int ret = 0;
  char c;

  while (KEYPRESS ())
  {
    c = READKB ();
    ret = 0;
    if (!do_chat)
    {
      switch (c)
      {
      case 27:
        while (KEYPRESS ())
          READKB ();
        ret = 27;
        break;

      case '!':
        chat_init ();
        break;
      }
    }
    else
    {                           /* do_chat */
      switch (c)
      {
      case 27:
        chat_exit ();
        break;

      case '\r':
        c = '\n';
        /* fall through into default */

      default:
        sb_putc (chatwinl, c);
        chat_putc (c);
        break;
      }
    }
  }

  chat_putc (-1);               /* if there's something old to send, send it! */
  return ret;
}
#endif

BOOL
keyabort (void)
{
#define CHATLEN 256
  static byte chatbuf1[CHATLEN + 5], chatbuf2[CHATLEN + 5], *curbuf = chatbuf1;
  static BOOL warned = FALSE;
  BOOL esc = FALSE;
  char *p;
  word c;

  if (chattimer > 0L)
  {
    if (unix_time (NULL) > (unsigned long) chattimer)
    {
      chattimer = lasttimer = 0L;
      hydra_devsend ("CON", (byte *) chattime, (word) strlen (chattime));
      sb_puts (chatwinl, &chattime[2]);
      sb_show ();
      chat_exit ();
    }
    else if ((unix_time (NULL) + 10L) > (unsigned long) chattimer && !warned)
    {
      if (!autochatopen)
        sb_puts (chatwinl, "\007\r\n * Warning: chat mode timeout in 10 seconds\r\n");

      css_upd ();
      warned = TRUE;
    }
  }
  else if (chattimer != lasttimer)
  {
    if (chattimer == 0L && !msgsent)
    {                           /* other side has DEV capacity */
      if (!nohydrachat)
      {
        if (gong_allowed)
          p = HaveChat_Bell;
        else
          p = HaveChat_NoBell;
        hydra_devsend ("CON", (byte *) p, (word) strlen (p));
      }
      msgsent = TRUE;
      /* commented out ...
         status_line(" * Hydra session in progress, chat facility now available");
       */
    }
    /* commented out ...
       else if (chattimer == -1L)
       status_line(" * Hydra session in init state, can't chat yet\r\n");
       else if (chattimer == -2L)
       status_line(" * Remote has no chat facility available\r\n");
       else if (chattimer == -3L)
       status_line(" * Hydra session in exit state, can't chat anymore\r\n");
     */
    lasttimer = chattimer;
  }

  if (nohydrachat || !un_attended)  /* fixes nohydrachat */
    return (FALSE);

  while (KEYPRESS ())
  {
#ifndef _WIN32
    c = (word) get_key ();
#else
    c = GetKBKey ();
#endif

    if (!CommonProtocolKeyCheck (c))
      switch (c)
      {
      case ESC:
        if (chattimer == 0L)
        {                       /* ESC: if in chat mode: exit chat mode */
          esc = TRUE;           /*  if not in chat mode: exit hydra     */
        }
        else if (chattimer > 0L)
        {
          chattimer = lasttimer = 0L;
          chat_exit ();
        }
        else
          esc = TRUE;
        break;

      case 0x2e00:
        if (chattimer == 0L)
        {
          chat_init ();
          sb_puts (chatwinl, &chatstart[2]);
          css_upd ();
          chattimer = lasttimer = unix_time (NULL) + CHAT_TIMEOUT;
        }
        else if (chattimer > 0L)
        {
          chattimer = lasttimer = 0L;
          chat_exit ();
        }
        else if (!autochatopen)
          sb_putc (chatwinl, '\007');
        break;

      default:
        if (c < ' ' || c == 127 || c == 255)  /*AGL:23apr93 */
          break;

      case '\r':
      case '\a':
      case '\b':
        if (chattimer <= 0L)
          break;
        if (c == '\r')
        {
          /* status_line ("#Got this line: %s",LocCommandBuf); */
          chatprocess (LocCommandBuf, 1);
          LocCommandBuf[0] = 0;
        }

        chattimer = unix_time (NULL) + CHAT_TIMEOUT;
        warned = FALSE;

        if (chatfill >= CHATLEN)
          sb_putc (chatwinl, '\007');
        else
        {
          switch (c)
          {
          case '\r':
            curbuf[chatfill++] = '\n';
            sb_putc (chatwinl, '\n');
            break;

          case '\b':
            if (strlen (LocCommandBuf))
              LocCommandBuf[strlen (LocCommandBuf) - 1] = 0;
            if (chatfill > 0 && curbuf[chatfill - 1] != '\n')
              chatfill--;
            else
            {
              curbuf[chatfill++] = '\b';
              curbuf[chatfill++] = ' ';
              curbuf[chatfill++] = '\b';
            }
            sb_puts (chatwinl, "\b \b");
            break;

          default:
            LocCommandBuf[strlen (LocCommandBuf) + 1] = 0;
            LocCommandBuf[strlen (LocCommandBuf)] = (byte) c;
            curbuf[chatfill++] = (byte) c;
            if (c != 7)
              sb_putc (chatwinl, c);
            break;
          }
        }
        break;
      }
  }

  if (chatfill > 0)
  {
    if (hydra_devsend ("CON", curbuf, chatfill))
    {
      curbuf = (curbuf == chatbuf1) ? chatbuf2 : chatbuf1;
      chatfill = 0;
    }
    css_upd ();
  }

  return (esc);
}                               /*keyabort() */

/* $Id: chat.c,v 1.5 1999/03/22 03:47:23 mr Exp $ */
