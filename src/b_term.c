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
 * Filename    : $Source: E:/cvs/btxe/src/b_term.c,v $
 * Revision    : $Revision: 1.8 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:16 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Terminal Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"
#include "vt100.h"

void term_Poll (ADDRP);

void
send_modem (char *s, int len)
{
  int i;

  for (i = 0; i < len; i++)
    SENDBYTE (*s++);
}

static REGIONP callwinsave, termwin;

void
termwin_init (void)
{
  short rows, rows_call, termwin_size;

  callwinsave = callwin;
  rows_call = CALLWIN_HIGH;

  if (rows_call >= 29)
    rows = 25;
  else
    rows = 8;
  rows_call -= rows;

  termwin_size = SB_COLS;

  termwin = sb_new (TOP_PARTS_ROWS, 0, rows, termwin_size);
  termwin->wflags |= SB_SCROLL | SB_WRAP;
  termwin->wflags &= ~(SB_ADDLF | SB_IGNLF);

  sb_fill (termwin, ' ', colors.background);

  callwin = sb_new_win ((short) (TOP_PARTS_ROWS + rows), 0,
                        rows_call, termwin_size,
                        ' ', colors.call,
                        boxtype, colors.frames,
                        MSG_TXT (M_RECENT_ACTIVITY));
  callwin->linesize = callwinsave->linesize;
  callwin->buffer = callwinsave->buffer;
  callwin->endbuff = callwinsave->endbuff;
  callwin->lastline = callwinsave->lastline;
  callwin->lastshown = callwinsave->lastshown;

  vt_init (ANSI, WHITE, BLACK, 1, 0);
  sb_box (callwin, boxtype, colors.frames);
  sb_caption (callwin, MSG_TXT (M_RECENT_ACTIVITY), colors.frames);
  redraw_callwin ();
  sb_show ();
}

void
termwin_exit (void)
{
  callwinsave->linesize = callwin->linesize;
  callwinsave->buffer = callwin->buffer;
  callwinsave->endbuff = callwin->endbuff;
  callwinsave->lastline = callwin->lastline;
  callwinsave->lastshown = callwin->lastshown;

  /* MR 970215 added free; shouldn't we give back allocated memory!? */
  free (callwin);
  free (termwin);

  callwin = callwinsave;
  termwin = NULL;

  sb_fill (callwin, ' ', colors.call);
  sb_box (callwin, boxtype, colors.frames);
  sb_caption (callwin, MSG_TXT (M_RECENT_ACTIVITY), colors.frames);
  redraw_callwin ();
}


int
b_term ()
{
#ifdef BUFFERED_CONSOLE_WRITE
  char tbuf[200];               /*PLF Sun  12-01-1991  06:28:59 */

  /* TJW 960526 static -> dynamic */
  int tcnt = 0;                 /*PLF Sun  12-01-1991  06:29:06 */

#endif

  char junk[256];
  short j, k;
  short c;
  unsigned int mask = 0xff;
  int gateway_mode = 0;
  int done;
  struct _key_fnc_hdr *OldKeyFncHdr;
  char *autodl = "**\030B00";   /* Zmodem autodl str.  */
  char *search_autodl = autodl;

#ifdef HAVE_HYDRA
  char *hauto = "\030cA\\f5\\a3\030a";
  char *search_hauto = hauto;

#endif /* HAVE_HYDRA */

  FILE *logfile = NULL;         /* log file stream */
  char prot;
  char *p;

  int retval = 1;               /* What to tell the mainline */
  ADDR baddr;

  ctrl_modemwin (0);            /* MR 970216 switch off modemwin in terminal */
  termwin_init ();
  vt_install (send_modem, NULL, termwin);

  OldKeyFncHdr = KbMapSet (&TrmnlKeyFncHdr);

  sb_dirty ();                  // ???
  // opening_banner ();         /* MR 970307 */

  strcpy (GenStats.remote, "*** TERMINAL MODE ***");

  IPC_SetStatus (MSG_TXT (M_MCP_TERM));
  IPC_Ping ();

  // vfossil_cursor (1);
  done = 0;
  first_block = 0;
  modem_init ();

  do_status (M_READY_TERMINAL, NULL);

  while (!done)
  {
    show_alive ();
    c = -1;
    if (CHAR_AVAIL ())
    {
      c = MODEM_IN ();

      if (comm_bits == BITS_7)
        c &= 0x7f;

      /* Check for Zmodem AutoDownload sequence */
      if ((c & 0x7f) == (int) *search_autodl)
      {
        if (!*++search_autodl)
        {
          sb_puts (termwin, MSG_TXT (M_ZMODEM_STARTED));
          CLEAR_INBOUND ();

          Download (NULL, (int) 'Z', NULL);

          sb_puts (termwin, MSG_TXT (M_ZMODEM_FINISHED));
          search_autodl = autodl;
        }
      }
      else
        search_autodl = autodl;

#ifdef HAVE_HYDRA
      /* Check for HYDRA AutoStart sequence */
      if ((c & 0x7f) == (int) *search_hauto)
      {
        if (!*++search_hauto)
        {
          sb_puts (termwin, MSG_TXT (M_HYDRA_AUTO_STARTED));

          Download (NULL, (int) 'H', NULL);

          sb_puts (termwin, MSG_TXT (M_HYDRA_AUTO_FINISHED));
          search_hauto = hauto;
        }
      }
      else
        search_hauto = hauto;
#endif /* HAVE_HYDRA */

      /* I really don't like looking for this piece of crap, but people
       * seem to really want it, so I guess I lose.  Now if only
       * QuickBBS would do things that were ANSI compatible - ugh! */

      if (c == '')
      {
        screen_clear ();
#ifdef BUFFERED_CONSOLE_WRITE
        tcnt = 0;
#endif
      }
      else if ((c == ENQ) && (answerback != NULL))
      {
        mdm_cmd_string (answerback, 1);
      }
      else
      {
#ifndef BUFFERED_CONSOLE_WRITE
        vt_out (c);             // TJW experimental, was: WRITE_ANSI ((char)c);
#else
        tbuf[tcnt++] = (char) c;  /*PLF Sun  12-01-1991  06:31:52 */
        if (tcnt == sizeof (tbuf) - 1)
        {
          tbuf[tcnt] = 0;
          puts (tbuf);
          tcnt = 0;
        }
#endif
      }

      if (logfile != NULL)
        fwrite (&c, 1, 1, logfile);
    }
#ifdef BUFFERED_CONSOLE_WRITE
    else if (tcnt)              /* No characters received */
    {
      tbuf[tcnt] = 0;
      puts (tbuf);
      tcnt = 0;
      c = 0;
    }
#endif

    if (KEYPRESS () || ctrlc_ctr)
    {
      if (ctrlc_ctr)
      {
        /* We got a ^C, so output it properly */
        while (KEYPRESS ())     /* Flush keyboard buffer */
          FOSSIL_CHAR ();
        ctrlc_ctr = 0;
        SENDBYTE (3);
        continue;
      }

      /* Get the keystroke. Map it, saving the original keystroke. */
      k = (short) KbRemap (j = FOSSIL_CHAR ());

      /* See if it mapped into 'toggle gateway'. If so, do it.     */
      if ((unsigned short) k == F_TERM_GATEWAYMODE)
      {
        gateway_mode = ~gateway_mode;
        continue;
      }

      /* It wasn't 'toggle gateway'. See if it's a command at all.
       * Or if we are currently in gateway mode.
       *
       * If we are in gateway mode, we will send the raw keystroke
       * out the comm port (zero plus scan code in the case of a
       * function key).
       *
       * Otherwise, we'll send normal keystrokes out verbatim, and
       * apply "ANSI mapping" to function keys. */

      if (gateway_mode || ((unsigned short) k & F_TRM_BS) != F_TRM_BS)
      {
        if (comm_bits == BITS_7)
          mask = 0x7f;
        else
          mask = 0xff;

        if (gateway_mode)
        {
          SENDBYTE ((unsigned char) (c = (j & mask)));
          if (c == 0)
            SENDBYTE ((unsigned char) (((unsigned short) j >> 8) & mask));
        }
        else
        {
          c = k & mask;
          if (c != 0)
            SENDBYTE ((unsigned char) c);
          else
            ansi_map ((unsigned short) k);
        }

        continue;
      }

      /* It's a command key, which has been remapped to one of our internal
       * codes. Dispatch on it. */

      if (!term_overlay (k))
        continue;

      switch ((unsigned short) k)
      {
      case F_TERM_CAPTUREFILE:
        if (logfile != NULL)
        {
          fclose (logfile);
          logfile = NULL;
          status_line (MSG_TXT (M_LOG_CLOSED), capturename);
          break;
        }

        if ((logptr == NULL) || (*logptr == '\0'))
        {
          scr_printf (MSG_TXT (M_INPUT_LOGFILE));
          fgets (capturename, 100, stdin);
          if ((k = (short) strlen (capturename)) != 0)
            capturename[--k] = '\0';  /* no '\n' */
        }
        else
        {
          strcpy (capturename, logptr);
          k = 1;
        }

        if (k)
        {
          logfile = share_fopen (capturename, append_binary, DENY_WRITE);
          if (logfile == NULL)
            status_line (MSG_TXT (M_LOG_NOT_OPEN), capturename);
          else
            status_line (MSG_TXT (M_LOG_OPEN), capturename);
        }
        break;

      case F_TERM_POLLBOSS:
        term_Poll (&boss_addr);
        break;

      case F_TERM_POLLNODE:
        scr_printf ("\n");
        scr_printf (MSG_TXT (M_ENTER_NET_NODE));
        if (!get_number (junk))
          break;
        if ((find_address (junk, &baddr) == 0)
            || ((short) (baddr.Net) < 0)
            || ((short) (baddr.Node) < 0)
            || (nodefind (&baddr, 1) == 0))
        {
          printf ("\n%s '%s'\n", &(MSG_TXT (M_NO_ADDRESS)[1]), junk);
          break;
        }

        if (!baddr.Zone)
          baddr.Zone = found_zone;

        term_Poll (&baddr);
        break;

      case F_TERM_DIALGROUP:
        if (!list_search ())
          screen_clear ();
        break;

      case F_TERM_MAILERMODE:
        if (port_ptr != original_port)
        {
          status_line (" %s %s%d:",
                       MSG_TXT (M_SWITCHING_BACK),
                       port_device,
                       1 + original_port);
          MDM_DISABLE ();
          port_ptr = original_port;
          if (Cominit (port_ptr, buftmo) != 0x1954)
          {
            port_ptr = c;
            Cominit (port_ptr, buftmo);
          }
          program_baud ();
          RAISE_DTR ();
        }
        retval = 0;
        /* Fall into exit code */

      case F_TERM_EXITBINK:
        if (logfile != NULL)
          fclose (logfile);
        done = 1;
        break;

      case F_TERM_UPLOAD:
        scr_printf (MSG_TXT (M_READY_TO_SEND));
        scr_printf (MSG_TXT (M_WHICH_PROTOCOL));
#ifdef HAVE_HYDRA
        scr_printf ("H)ydra    Z)modem     S)EAlink     T)elink     X)modem\r\n");
#else
        scr_printf ("Z)modem   S)EAlink    T)elink      X)modem\r\n");
#endif /* HAVE_HYDRA */
        if (extern_protocols != NULL)
        {
          scr_printf (extern_protocols);
          scr_printf ("\r\n");
        }

        scr_printf (MSG_TXT (M_CHOOSE));
        junk[0] = '\0';
        fgets (junk, 100, stdin);

        if ((junk[0] == '\0') || (junk[0] == '\n'))
          break;

        if (!(prot = (char) toupper (junk[0])))
          break;

        p = NULL;
        if ((strchr (native_protocols, prot) == NULL)
            && ((extern_protocols == NULL) ? (1) : (p = strchr (extern_protocols, prot)) == NULL))
        {
          scr_printf (MSG_TXT (M_DONT_KNOW_PROTO));
          break;
        }

        scr_printf (MSG_TXT (M_FILE_TO_SEND));
        junk[0] = '\0';
        fgets (junk, 100, stdin);

        if ((k = strlen (junk)) != 0)
          junk[--k] = '\0';

        if ((junk[0] == '\0') || (junk[0] == '\n'))
          break;

        if ((!k) || (!dexists (junk)))
          break;

        Upload (junk, (int) prot, p);
        break;

      case F_TERM_DOWNLOAD:
        scr_printf (MSG_TXT (M_READY_TO_RECEIVE));
        scr_printf (MSG_TXT (M_WHICH_PROTOCOL));

#ifdef HAVE_HYDRA
        scr_printf ("H)ydra    Z)modem     S)EAlink     T)elink     X)modem\r\n");
#else
        scr_printf ("Z)modem   S)EAlink    T)elink      X)modem\r\n");
#endif /* HAVE_HYDRA */

        if (extern_protocols != NULL)
        {
          scr_printf (extern_protocols);
          scr_printf ("\r\n");
        }

        scr_printf (MSG_TXT (M_CHOOSE));
        junk[0] = '\0';
        fgets (junk, 100, stdin);

        if ((junk[0] == '\0') || (junk[0] == '\n'))
          break;

        if (!(prot = (char) toupper (junk[0])))
          break;

        p = NULL;
        if ((strchr (native_protocols, prot) == NULL)
            && ((extern_protocols == NULL) ? (1) : (p = strchr (extern_protocols, prot)) == NULL))
        {
          scr_printf (MSG_TXT (M_DONT_KNOW_PROTO));
          break;
        }

        Download (NULL, (int) prot, p);
        break;

      case F_TERM_HELPSCREEN:
        main_help ();
        break;

      default:
        break;
      }
    }

    if (c == -1)
      time_release ();
  }

  KbMapSet (OldKeyFncHdr);

  set_prior (PRIO_REGULAR);

  termwin_exit ();
  return (retval);
}

/* Handle polls from terminal mode. */

void
term_Poll (ADDRP address)
{
  unsigned save1, save2, save3;
  char junk[256];

  save1 = comm_bits;
  save2 = parity;
  save3 = stop_bits;

  un_attended = 1;
  sb_dirty ();
  opening_banner ();
  mailer_banner ();

  sprintf (junk, "%s", Full_Addr_Str (address));
  junk[28] = '\0';

  sb_fill (holdwin, ' ', colors.hold);
  sb_move_puts (holdwin, 1, 11, MSG_TXT (M_POLLING_COLON));
  sb_move_puts (holdwin, 3, (short) (15 - strlen (junk) / 2), junk);
  sb_show ();

  if (!CARRIER)
    modem_hangup ();

  do_mail (address, 1);
  un_attended = 0;
  screen_clear ();
  scr_printf (MSG_TXT (M_MANUAL_MODE));

  comm_bits = save1;
  parity = save2;
  stop_bits = save3;
  program_baud ();

  gong ();
  // vfossil_cursor (1);
}

/* Handle uploads from terminal mode.
 * Note: filepath parameter below will be trashed if it's a wildcard! */

int
Upload (char *filepath, int prot, char *extern_ptr)
{
  unsigned save1, save2, save3;
  char *p;
  struct FILEINFO fileinfo;

  int err = FILE_SENT_OK;       /* Xmodem, Ymodem, Telink flag */

  if (dfind (&fileinfo, filepath, 0))
    return (FAILURE);

  save1 = comm_bits;
  save2 = parity;
  save3 = stop_bits;
  comm_bits = BITS_8;
  parity = NO_PARITY;
  stop_bits = STOP_1;
  program_baud ();
  XON_DISABLE ();

  /* If external protocol requested, call it */
  if (extern_ptr != NULL)
    do_extern ("Send", prot, filepath);
  else
  {
    css_init ();
    switch (prot)
    {
    case 'X':
    case 'Y':
      err = Xmodem_Send_File (filepath, NULL);
      break;

    default:
      switch (prot)             /* init the proper protocol if need be */
      {
#ifdef HAVE_HYDRA
      case 'H':
        hydra_init (hydra_options);
        break;
#endif /* HAVE_HYDRA */
      case 'Z':
        Send_Zmodem (NULL, NULL, ZMODEM_INIT, !DO_WAZOO);
        break;
      default:
        break;
      }
      /* Find the delimiter on the pathspec for use by the various
       * batch protocols. */

      p = strrchr (filepath, '\\');
      if (p == NULL)
        p = strrchr (filepath, '/');
      if (p == NULL)
        p = strchr (filepath, ':');
      if (p == NULL)
        p = filepath;
      else
        p++;

      /*  At this point *p points to the location in the input
       *  string where the prepended path information ends. All
       *  we need to do, then, is to keep plugging in the stuff
       *  we get from _dos_find(first|next) and transfer files.
       *  We already have the first matching filename from the
       *  _dos_findfirst we did above, so we use a "do" loop. */

      do
      {
        /* Append the current filename */
        strcpy (p, fileinfo.name);

        switch (prot)
        {
#ifdef HAVE_HYDRA
        case 'H':
          err = hydra (filepath, NULL);
          break;
#endif /* HAVE_HYDRA */
        case 'Z':
          err = Send_Zmodem (filepath, NULL, 1, !DO_WAZOO);
          break;
        default:
          err = Batch_Send (filepath);
          break;
        }
      }
      while ((err != CANNOT_SEND) && (!dfind (&fileinfo, NULL, 1)));

      switch (prot)             /* Finish the proper protocol if need be */
      {
#ifdef HAVE_HYDRA
      case 'H':
        hydra (NULL, NULL);
        hydra_deinit ();
        break;
#endif /* HAVE_HYDRA */
      case 'Z':
        Send_Zmodem (NULL, NULL, ZMODEM_ENDBATCH, !DO_WAZOO);
        break;
      default:
        Batch_Send (NULL);
        break;
      }
    }

    css_done ();
  }

  dfind (&fileinfo, NULL, 2);

  comm_bits = save1;
  parity = save2;
  stop_bits = save3;
  program_baud ();
  XON_ENABLE ();
  gong ();

  err = (err == CANNOT_SEND ? FAILURE : IGNORE);

  return (err);
}

/* Handle downloads from terminal mode.

 * If filespec is NULL and we need a filename, ask for it.
 * Otherwise, assume that the spec we got is OK and use it. */

int
Download (char *filepath, int prot, char *extern_ptr)
{
  int k;
  int err = 1;
  unsigned save1, save2, save3;
  char junk[256];

  if ((prot == 'X') || (prot == 'Y') || extern_ptr != NULL)
  {
    if (filepath != NULL)
      strcpy (junk, filepath);
    else
    {
      scr_printf (MSG_TXT (M_FILE_TO_RECEIVE));
      junk[0] = '\0';
      fgets (junk, 100, stdin);
      if ((junk[0] == '\0') || (junk[0] == '\n'))
        return (0);
      k = strlen (junk);
      if (k <= 1)
        return (0);
      junk[--k] = '\0';
    }
  }

  save1 = comm_bits;
  save2 = parity;
  save3 = stop_bits;
  comm_bits = BITS_8;
  parity = NO_PARITY;
  stop_bits = STOP_1;
  program_baud ();
  XON_DISABLE ();

  /* If external protocol requested, call it */

  if (extern_ptr != NULL)
    do_extern ("Get", prot, junk);
  else
  {
    css_init ();
    switch (prot)
    {

#ifdef HAVE_HYDRA
    case 'H':
      hydra_init (hydra_options);
      err = hydra (NULL, NULL);
      hydra_deinit ();
      break;
#endif /* HAVE_HYDRA */

    case 'X':
    case 'Y':
      err = Xmodem_Receive_File (download_path, junk);
      break;

    case 'Z':
      err = get_Zmodem (download_path, NULL);
      break;

    default:
      err = Batch_Receive (download_path);
      break;
    }

    css_done ();
  }

  comm_bits = save1;
  parity = save2;
  stop_bits = save3;
  program_baud ();
  XON_ENABLE ();
  gong ();

  err = err == CANNOT_RECV ? FAILURE : IGNORE;
  return (err);
}

/* $Id: b_term.c,v 1.8 1999/03/22 03:47:16 mr Exp $ */
