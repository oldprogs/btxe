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
 * Filename    : $Source: E:/cvs/btxe/src/script2.c,v $
 * Revision    : $Revision: 1.11 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/06/10 16:55:39 $
 * State       : $State: Exp $
 * Orig. Author: C. Eike Hofmeister, 2:2426/2280
 *
 * Description : scriptfile processor for precompiled (bbs-)scripts
 *
 *---------------------------------------------------------------------------*/

#ifdef BBS_INTERNAL

#include "includes.h"
#include "bbs_incl.h"

/*************************************************************************************************************/

#define GOSUB_MAXDEPTH 5

void
switch_to_unattended_screen ()
{
  termwin_exit ();
  screen_clear ();
  sb_dirty ();
  sb_show ();

  if (show_modemwin)
  {
    ctrl_modemwin (0);
    ctrl_modemwin (1);
  }
}

void
run_script (char *this, int locmode)
{
  long gosubstack[GOSUB_MAXDEPTH];
  int gosubstackpos = -1;
  long cmndpos, datapos;
  char *menudata;
  char *dummypointer;
  long menusize;
  long timeout, dummylong;
  char dummychar1[300], dummychar2[300], dummy, c1, c2, c3;
  enum ScriptEvents dummyevent;
  short dontmove;
  int i, j;
  short quit_script = 0;
  CALLHIST CallInfo;
  FILE *userfile;

  memset (&CallInfo, 0, sizeof (CallInfo));
  CallInfo.task = TaskNumber;
  CallInfo.calltype = CT_BBS;
  CallInfo.subclass = SC_IBS;
  time (&CallInfo.starttime);
  strcpy (CallInfo.h.b2.name, "[unknown]");

  userfile = share_fopen (this, read_binary, DENY_WRITE);
  if (!userfile)
  {
    status_line ("!Script not found");
    return;
  }

  fseek (userfile, 0, SEEK_END);
  menusize = ftell (userfile) - 1;
  if (menusize <= 0)
  {
    status_line ("!Script contains zero bytes");
    fclose (userfile);
    return;
  }

  rewind (userfile);
  if ((menudata = malloc (menusize)) == NULL)
  {
    status_line ("!Not enough memory for script");
    return;
  }

  if (!fread (menudata, menusize, 1, userfile))
  {
    status_line ("!Error while reading script");
    free (menudata);
    fclose (userfile);
    return;
  }

  fclose (userfile);
  userfile = NULL;
  status_line (" Script loaded");

  if ((strval = calloc (64, sizeof (defaultstr))) == NULL)
  {
    free (menudata);
    status_line ("!Not enough memory for script-variables");
    return;
  }

  if ((numval = calloc (128, sizeof (long))) == NULL)
  {
    free (strval);
    free (menudata);
    status_line ("!Not enough memory for script-variables");
    return;
  }

  if ((event_addresses = calloc ((int) ev_number_of_scriptevents, sizeof (long))) == NULL)
  {
    free (numval);
    free (strval);
    free (menudata);
    status_line ("!Not enough memory for script-variables");
    return;
  }
  else
  {
    /* clear all event jump-destinations */
    for (i = 0; i < ((int) ev_number_of_scriptevents); i++)
      event_addresses[i] = -1;
  }

  if ((event_modes = calloc ((int) ev_number_of_scriptevents, sizeof (short))) == NULL)
  {
    free (event_addresses);
    free (numval);
    free (strval);
    free (menudata);
    status_line ("!Not enough memory for script-variables");
    return;
  }

  bbsLoadTranslation (0);

  script_fullscreen = 0;
  emulation = emul_not_set;
  bbslocalmode = locmode;
  timers_used = 0;
  timer_setonkey = 0;

  TimeCheck (1);                /* initialize */
  bbs_initoutput ();

  eventdetected = ev_none;
  cmndpos = 0L;

  while (!quit_script)
  {
    dontmove = 0;
    datapos = cmndpos + 2;
    status_line (">doing %2x at %3x", menudata[cmndpos], cmndpos);

    switch (menudata[cmndpos])
    {
    case 0x1E:                 /* call <label> */
      if (++gosubstackpos < GOSUB_MAXDEPTH)
      {
        gosubstack[gosubstackpos] = cmndpos + menudata[cmndpos + 1];
      }
      else
      {
        status_line ("!Too many CALLs");
        eventdetected = ev_internalerror;
        break;
      }
      /* no "break" here, we continue with "goto <label>" */

    case 0x00:                 /* goto <label> */
      memcpy (&cmndpos, &menudata[datapos], 4);
      dontmove = 1;
      break;

    case 0x01:                 /* ifstr <str> <char> <str> <label> ... */
    case 0x02:                 /* ifnum <str> <char> <str> <label> ... */
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      datapos += menudata[datapos] + 1;
      c1 = menudata[datapos++];
      createrealstring (dummychar2, &(menudata[datapos]), sizeof (dummychar2));
      datapos += menudata[datapos] + 1;
      status_line (">comparing <%s> and <%s>", dummychar1, dummychar2);

      dummy = 1;
      if (menudata[cmndpos] == 0x01)
      {                         /* ifstr */
        if (toupper (c1) == 'I')
        {
          if (c1 == 'I')
          {
            for (i = 0; i < strlen (dummychar1); i++)
              dummychar1[i] = toupper (dummychar1[i]);
            for (i = 0; i < strlen (dummychar2); i++)
              dummychar2[i] = toupper (dummychar2[i]);
          }

          if (strstr (dummychar2, dummychar1) == NULL)
            dummy = 0;
        }
        else
        {
          if (strlen (dummychar1) != strlen (dummychar2))
            dummy = 0;
          else
          {
            for (i = 0; (i < strlen (dummychar1)) && (i < strlen (dummychar2)); i++)
            {
              switch (c1)
              {
              case '!':
              case '=':
                if (dummychar1[i] != dummychar2[i])
                  dummy = 0;
                break;

              case ':':
              case '~':
                if (toupper (dummychar1[i]) != toupper (dummychar2[i]))
                  dummy = 0;
                break;

              default:
                eventdetected = ev_internalerror;
                dummy = 0;
                break;
              }
            }
          }

          if ((c1 == '!') || (c1 == ':'))
            dummy = !dummy;
        }
      }
      else
      {                         /* ifnum */
        if (sscanf (dummychar1, "%d", &i) != 1)
          dummy = 0;

        if (sscanf (dummychar2, "%d", &j) != 1)
          dummy = 0;

        if (dummy)
        {
          switch (c1)
          {
          case '<':
            if (!(i < j))
              dummy = 0;
            break;

          case '>':
            if (!(i > j))
              dummy = 0;
            break;

          case '[':
            if (!(i <= j))
              dummy = 0;
            break;

          case ']':
            if (!(i >= j))
              dummy = 0;
            break;

          case '=':
            if (!(i == j))
              dummy = 0;
            break;

          case '!':
            if (!(i != j))
              dummy = 0;
            break;

          default:
            eventdetected = ev_internalerror;
            dummy = 0;
            break;
          }
        }
      }

      if (!dummy)
      {                         /* false: goto false-label */
        memcpy (&cmndpos, &menudata[datapos], 4);
        dontmove = 1;
      }

      /* if true: just continue with next command */
      break;

    case 0x03:                 /* event <event> [<g|c><label>] */
      c1 = menudata[datapos++];
      if (c1 & 64)
        c1 = (c1 & (1 + 2 + 4 + 8 + 16 + 32)) + ev_timer_expired;
      event_modes[(int) c1] = menudata[datapos++];
      memcpy (&(event_addresses[(int) c1]), &menudata[datapos], 4);
      break;

    case 0x04:                 /* loadtranslation <translation> */
      memcpy (&dummylong, &(menudata[datapos]), 4);
      bbsLoadTranslation (dummylong);
      break;

    case 0x05:                 /* logfile <str> */
      createrealstring (dummychar1, &menudata[datapos], sizeof (dummychar2));
      status_line (" %s", dummychar1);
      break;

    case 0x06:                 /* setvar <var> <expression> */
      createrealstring (dummychar1, &menudata[datapos + 1], sizeof (dummychar1));
      if (menudata[datapos] & 128)
      {
        if ((menudata[datapos] & 64))
          strcpy (str_address (menudata[datapos]), dummychar1);
        else
          eventdetected = ev_internalerror;  /* writing to pre-defined variable */
      }
      else
      {
        if ((menudata[datapos] & 64) && ((menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32)) >= timers_used))
          timers_used = menudata[datapos] - 64 + 1;
        bbs_calculate (dummychar1, &(numval[(int) menudata[datapos]]));
      }
      break;

    case 0x07:                 /* addtohistory <name> <handle> <city> <calls> <filesup> <filesdn> <kbup> <kbdn> <read> <posted> */
      createrealstring (CallInfo.h.b2.name, &(menudata[datapos]), sizeof (CallInfo.h.b2.name));
      datapos += menudata[datapos] + 1;
      createrealstring (CallInfo.h.b2.handle, &(menudata[datapos]), sizeof (CallInfo.h.b2.handle));
      datapos += menudata[datapos] + 1;
      createrealstring (CallInfo.h.b2.city, &(menudata[datapos]), sizeof (CallInfo.h.b2.city));
      datapos += menudata[datapos] + 1;
      CallInfo.h.b2.baud = cur_baud.rate_value;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      datapos += menudata[datapos] + 1;
      bbs_calculate (dummychar1, &dummylong);
      CallInfo.h.b2.calls = dummylong;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      datapos += menudata[datapos] + 1;
      bbs_calculate (dummychar1, &dummylong);
      CallInfo.h.b2.filesup = dummylong;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      datapos += menudata[datapos] + 1;
      bbs_calculate (dummychar1, &dummylong);
      CallInfo.h.b2.filesdn = dummylong;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      datapos += menudata[datapos] + 1;
      bbs_calculate (dummychar1, &dummylong);
      CallInfo.h.b2.kbup = dummylong;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      datapos += menudata[datapos] + 1;
      bbs_calculate (dummychar1, &dummylong);
      CallInfo.h.b2.kbdn = dummylong;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      datapos += menudata[datapos] + 1;
      bbs_calculate (dummychar1, &dummylong);
      CallInfo.h.b2.read = dummylong;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      bbs_calculate (dummychar1, &dummylong);
      CallInfo.h.b2.posted = dummylong;
      break;

    case 0x08:                 /* print <str> */
      bbssend (&menudata[datapos]);
      break;

    case 0x09:                 /* getkey <var> */
    case 0x0A:                 /* getkeysysop <var> */
    case 0x0B:                 /* getkeymodem <var> */
      if ((menudata[datapos] & (128 + 64)) != 128 + 64)
        eventdetected = ev_internalerror;
      else
      {
        c1 = menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32);
        if ((menudata[cmndpos] == 0x09) || (menudata[cmndpos] == 0x0B))
          if ((CHAR_AVAIL ()))
            c2 = transin (MODEM_IN ());

        if ((menudata[cmndpos] == 0x09) || (menudata[cmndpos] == 0x0A))
          if (KEYPRESS ())
            c2 = READKB ();

        if (c2)
        {
          strval[(int) c1][strlen (strval[(int) c1]) + 1] = 0;
          strval[(int) c1][strlen (strval[(int) c1])] = c2;
          if (timer_setonkey)
            numval[timer_setonkey] = time_setonkey;
        }
      }
      break;

    case 0x0C:                 /* getstr <var> <char> <length> <str> */
      c1 = menudata[datapos++];
      c2 = menudata[datapos++];
      memcpy (&dummylong, &(menudata[datapos]), 4);
      datapos += 4;
      createrealstring (dummychar1, &(menudata[datapos]), sizeof (dummychar1));
      bbsgetinputline (str_address (c1), c2, dummychar1, dummylong);
      if (eventdetected != ev_none)
      {
        /* if an event with "call" occured, we have to make sure that the
         * return-point is not the next but this command, so that the user
         * can continue with his input */
        if (((event_addresses[eventdetected]) >= 0) &&
            (event_modes[eventdetected] == 'c'))
          dontmove = 1;
      }
      break;

    case 0x0D:                 /* parse <numvar> <string> */
      i = 0;
      j = 0;
      c3 = menudata[datapos + 1] & (1 + 2 + 4 + 8 + 16 + 32);

      dummy = strlen (strval[(int) c3]);
      while ((!isdigit (strval[(int) c3][i])) && (i < dummy))
        i++;

      while ((isdigit (strval[(int) c3][i])) && (i < dummy))
        dummychar2[j++] = strval[(int) c3][i++];
      dummychar2[j] = '\0';

      if (sscanf (dummychar2, "%d", &i) != 1)
        numval[menudata[datapos]] = 0;
      else
        numval[menudata[datapos]] = i;

      /* while ((!isdigit(strval[c3][c]))&&(c<dummy)) c++; */
      j = 0;

      while (i <= dummy)
        strval[(int) c3][j++] = strval[(int) c3][i++];
      break;

    case 0x0E:                 /* fileopen <str> */
      if (userfile != NULL)
      {
        fseek (userfile, 0, SEEK_END);
        fclose (userfile);
      }

      createrealstring (dummychar1, &menudata[datapos], sizeof (dummychar1));
      userfile = share_fopen (dummychar1, read_binary_plus, DENY_ALL);
      if (userfile == NULL)
      {
        userfile = share_fopen (dummychar1, write_binary_plus, DENY_ALL);
        if (userfile == NULL)
          eventdetected = ev_fileerror;
        else
          status_line (":created %s", dummychar1);
      }
      break;

    case 0x0F:                 /* fileclose */
      if (userfile == NULL)
      {
        eventdetected = ev_fileerror;
        break;
      }

      fseek (userfile, 0, SEEK_END);
      fclose (userfile);
      userfile = NULL;
      break;

    case 0x10:                 /* fileposition <numvar> */
      if (userfile == NULL)
      {
        eventdetected = ev_fileerror;
        break;
      }

      c1 = menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32);
      numval[(int) c1] = ftell (userfile);
      break;

    case 0x11:                 /* filegoto <numvar> */
      if (userfile == NULL)
      {
        eventdetected = ev_fileerror;
        break;
      }

      c1 = menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32);
      fseek (userfile, numval[(int) c1], SEEK_SET);
      break;

    case 0x12:                 /* filetruncate */
      status_line ("!ERROR: filetruncate does not work.");
      if (userfile == NULL)
      {
        eventdetected = ev_fileerror;
        break;
      }
      break;

    case 0x13:                 /* filelineread <stringvar> */
      c1 = menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32);
      strval[c1][0] = '\0';
      if (userfile == NULL)
      {
        status_line ("!file not open");
        eventdetected = ev_fileerror;
        break;
      }

      if (feof (userfile))
      {
        status_line ("!eof reached");
        eventdetected = ev_fileerror;
        break;
      }

      if ((fgets (strval[(int) c1], defaultstrlen, userfile)) == NULL)
        eventdetected = ev_fileerror;

      if (strlen (strval[(int) c1]))
        if ((strval[(int) c1][strlen (strval[(int) c1]) - 1]) == '\n')
          strval[(int) c1][strlen (strval[(int) c1]) - 1] = '\0';

      if (strlen (strval[(int) c1]))
        if ((strval[(int) c1][strlen (strval[(int) c1]) - 1]) == '\r')
          strval[(int) c1][strlen (strval[(int) c1]) - 1] = '\0';

      if (strlen (strval[(int) c1]))
        if ((strval[(int) c1][strlen (strval[(int) c1]) - 1]) == '\32')
          strval[(int) c1][strlen (strval[(int) c1]) - 1] = '\0';

      break;

    case 0x14:                 /* filelineinsert <str> */
      if (userfile == NULL)
      {
        eventdetected = ev_fileerror;
        break;
      }

      createrealstring (dummychar1, &menudata[datapos], sizeof (dummychar1));
      file_insertline (userfile, dummychar1);
      break;

    case 0x15:                 /* filelinereplace <str> */
      if (userfile == NULL)
      {
        eventdetected = ev_fileerror;
        break;
      }

      createrealstring (dummychar1, &menudata[datapos], sizeof (dummychar1));
      file_replaceline (userfile, dummychar1);
      break;

    case 0x16:                 /* filelineremove <num> */
      if (userfile == NULL)
      {
        eventdetected = ev_fileerror;
        break;
      }

      file_removeline (userfile, menudata[datapos]);
      break;

    case 0x17:                 /* fullscreen */
      if (!script_fullscreen)
        termwin_init ();
      script_fullscreen = 1;
      break;

    case 0x18:                 /* unattended */
      if (script_fullscreen)
        switch_to_unattended_screen ();
      script_fullscreen = 0;
      break;

    case 0x19:                 /* timesetonkey <num> */
      timer_setonkey = (menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32)) | 64;
      memcpy (&time_setonkey, &menudata[datapos + 1], 4);
      if (timers_used <= (timer_setonkey - 64))
        timers_used = timer_setonkey - 64 + 1;
      break;

    case 0x1A:                 /* abort */
      quit_script = 1;
      break;

    case 0x1B:                 /* emulation <type> */
      emulation = menudata[datapos];
      switch (emulation)
      {
      case 0:
        status_line (">emulation set to <none>");
        break;

      case 1:
        status_line (">emulation set to <ascii>");
        break;

      case 2:
        status_line (">emulation set to <ansi>");
        break;
      }
      break;

    case 0x1C:                 /* dropcarrier */
      modem_hangup ();
      break;

    case 0x1D:                 /* ifcarrier <label> */
      if (!CARRIER)
      {
        memcpy (&cmndpos, &menudata[datapos], 4);
        dontmove = 1;
      }
      /* if carrier: just continue */
      break;

      /* case 0x1E: gosub: see above goto (0x00) */

    case 0x1F:                 /* return */
      if (gosubstackpos >= 0)
      {
        cmndpos = gosubstack[gosubstackpos--];
        dontmove = 1;
      }
      else
      {
        eventdetected = ev_internalerror;
        status_line ("!RETURN without CALL");
      }
      break;

    case 0x20:                 /* releasetime */
      time_release ();
      break;

    case 0x21:                 /* waitforallsent */
      timeout = timerset (PER_SECOND * 10);
      while ((CARRIER) && (!OUT_EMPTY ()) && (!timeup (timeout)))
        time_release ();
      break;

    case 0x22:                 /* getfileinfo <nv><sv><nv><nv><nv><sv><str> */
      filegetdescriptioninit ((strval[menudata[datapos + 5] & (1 + 2 + 4 + 8 + 16 + 32)]),
                (strval[menudata[datapos + 1] & (1 + 2 + 4 + 8 + 16 + 32)]),
              &(numval[menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32 + 64)]),
          &(numval[menudata[datapos + 2] & (1 + 2 + 4 + 8 + 16 + 32 + 64)]),
          &(numval[menudata[datapos + 3] & (1 + 2 + 4 + 8 + 16 + 32 + 64)]),
         &(numval[menudata[datapos + 4] & (1 + 2 + 4 + 8 + 16 + 32 + 64)]));
      break;

    case 0x23:                 /* getfileinfotext <sv> */
      filegetdescriptiontext (strval[(1 + 2 + 4 + 8 + 16 + 32)
                                     & menudata[datapos]]);
      break;

    case 0x24:                 /* getfileinfosearch <sv> */
      getfileinfosearch (strval[63 & menudata[datapos]]);
      break;

    case 0x25:                 /* setvardate <nv><sv><str> */
      createrealstring (dummychar1, &menudata[datapos + 2], sizeof (dummychar1));
      bbs_printdate ((time_t *) (numval[menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32 + 64)]),
                     dummychar1,
               (strval[menudata[datapos + 1] & (1 + 2 + 4 + 8 + 16 + 32)]));
      break;

    case 0x26:                 /* sendfile <nv><nv><sv><str> */
      if (locmode)
        break;
      createrealstring (dummychar1, &menudata[datapos + 3], sizeof (dummychar1));
      if (!(bbs_sendfile (dummychar1, strval[menudata[datapos + 2]
                                          & (1 + 2 + 4 + 8 + 16 + 32)][0])))
      {
        numval[menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 1;
        numval[menudata[datapos + 1] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 100;  /* size of file */
      }
      else
      {
        numval[menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 0;
        numval[menudata[datapos + 1] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 0;
      }
      break;

    case 0x27:                 /* receivefile <nv><nv><sv><sv><str> */
      if (locmode)
        break;
      createrealstring (dummychar1, &menudata[datapos + 4], sizeof (dummychar1));
      dummypointer = download_path;
      download_path = dummychar1;
      if (bbs_receivefile (strval[menudata[datapos + 3] & 63],
                           strval[menudata[datapos + 2]
                                  & (1 + 2 + 4 + 8 + 16 + 32)][0]))
      {
        numval[menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 1;
        numval[menudata[datapos + 1] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 100;  /* size of file */
      }
      else
      {
        numval[menudata[datapos] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 0;
        numval[menudata[datapos + 1] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = 0;
      }
      download_path = dummypointer;
      break;

    case 0x28:                 /* sendfilelist <nv><nv><sv><sv><str> */
      status_line ("!sendfilelist is not implemented yet");
      break;

    case 0x29:                 /* deletefile <str> */
      createrealstring (dummychar1, &menudata[datapos], sizeof (dummychar1));
      if (remove (dummychar1))
        eventdetected = ev_fileerror;
      break;

    default:                   /* error! */
      status_line ("!Error in script at %xh", cmndpos);
      eventdetected = ev_internalerror;
      quit_script = 1;
      break;
    }

    if (eventdetected == ev_none)
      TimeCheck (0);

    if (!dontmove)
      cmndpos += menudata[cmndpos + 1];

    while (eventdetected != ev_none)
    {
      if ((event_addresses[eventdetected]) >= 0)
      {
        dummyevent = eventdetected;
        if (event_modes[eventdetected] == 'c')
        {
          if (++gosubstackpos < GOSUB_MAXDEPTH)
          {
            gosubstack[gosubstackpos] = cmndpos;
            eventdetected = ev_none;
          }
          else
          {
            status_line ("!Too many CALLs");
            if (eventdetected == ev_internalerror)
            {
              quit_script = 1;
              eventdetected = ev_none;
            }
            else
              eventdetected = ev_internalerror;
          }
        }
        else
          eventdetected = ev_none;

        cmndpos = event_addresses[dummyevent];
        dontmove = 1;
      }
      else
        eventdetected = ev_none;
    }

    if (cmndpos > menusize)
      quit_script = 1;          /* We reached the end. Just quit. */
  }

  if (userfile != NULL)
    fclose (userfile);

  filegetdescriptiondone ();
  free (event_modes);
  free (event_addresses);
  free (numval);
  free (strval);
  free (menudata);

  if (script_fullscreen)
    switch_to_unattended_screen ();

  opening_banner ();
  mailer_banner ();
  time (&CallInfo.length);
  CallInfo.length -= CallInfo.starttime;
  localtime (&CallInfo.starttime);
  SaveCallInfo (CallInfo);
}

#undef GOSUB_MAXDEPTH

/*************************************************************************************************************/

void
bbsinternal (int locmode)
{
  char dummy[512], *p;

  if (server_mode)
    locmode = 0;                /* if connected directly to another system, send the stuff */

  strcpy (dummy, config_name);
  p = strrchr (dummy, DIR_SEPC);
  if (p != NULL)
  {
    p++;
    *p = 0;
  }

  strcat (dummy, "binkley.bbs");
  run_script (dummy, locmode);
  do_today (1);
}

/*************************************************************************************************************/

#endif

/* $Id: script2.c,v 1.11 1999/06/10 16:55:39 ceh Exp $ */
