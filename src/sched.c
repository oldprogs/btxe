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
 * Filename    : $Source: E:/cvs/btxe/src/sched.c,v $
 * Revision    : $Revision: 1.9 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:38 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Scheduler Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void writeactivitydump (void);  /* MR 971223 from b_sbinit.c */

/**************************************************************************/
/*** This MUST be exactly 16 total bytes including the terminating null ***/
/*** or the routines read_sched() and write_sched() must be changed!!!! ***/
/**************************************************************************/
static char *BinkSched = PRDCT_SHRT "Schedule 15";  /* Version of scheduler */

long
custom_date (struct tm *tm)
{
  return 400L * (tm->tm_year - 80) + (tm->tm_mon * 32) + tm->tm_mday;
}

int
stats_to_msg (void)
{
  struct S_PKTh *h;
  long day, month, year;        /* long -> To avoid type warnings */
  time_t long_time;
  char subject[80];
  ADDR target;
  struct tm *tm;
  char junk1[150];              /* VRP 990827 */

  month = hist.which_date % 400L;
  year = (hist.which_date - month) / 400L + 1980;
  day = month % 32;
  month = (month - day) / 32;
  sprintf (subject, "Stats for %s %02ld, %ld on task %d",
           mtext[month], day, year, TaskNumber);
  unix_time (&long_time);
  long_time -= (24L * 60L * 60L);  /* One day before */
  tm = unix_localtime (&long_time);
  tm->tm_hour = 23;
  tm->tm_min = 59;
  tm->tm_sec = 59;
  target.Zone = alias[0].Zone;
  target.Net = alias[0].Net;
  target.Node = alias[0].Node;
  target.Point = alias[0].Point;

  if (PKTStats.toecho == 0)
    h = CreateMSGinPKT (local_inbound, PKTStats.from,
                        target, "Binkley-XE", PKTStats.from,
                        "Stats coordinator", PKTStats.to,
                        subject, MSG_PRIVATE | MSG_LOCAL, "",
                        PKTStats.password, tm);
  else
    h = CreateMSGinPKT (local_inbound, PKTStats.from,
                        target, "Binkley-XE", PKTStats.from,
                        "Stats coordinator", PKTStats.from,
                        subject, MSG_PRIVATE | MSG_LOCAL, PKTStats.echo,
                        PKTStats.password, tm);

  if (h == NULL)
    return 0;

  WriteToPKT (h, "Dear stats coordinator, we are about to reset the stats counter for today.\r");
  WriteToPKT (h, "Below you will find the current figures for this task.\r\r");
  WriteToPKT (h, "                   Incoming BBS calls: %hd\r", hist.bbs_calls);
  WriteToPKT (h, "                  Incoming mail calls: %hd\r", hist.mail_calls);
  WriteToPKT (h, "                   Incoming FAX calls: %hd\r", hist.fax_calls);
  WriteToPKT (h, "                 Total incoming calls: %hd\r\r", hist.bbs_calls + hist.mail_calls + hist.fax_calls);
  WriteToPKT (h, "    Outgoing calls (tried/successful): %hd/%d\r", hist.calls_made, hist.connects);
  WriteToPKT (h, "               Cost of outgoing calls: %ld\r\r", hist.callcost);
  WriteToPKT (h, "    Files transmitted (sent/received): %hd/%hd\r", hist.files_out, hist.files_in);
  WriteToPKT (h, "    Bytes transmitted (sent/received): %ld/%ld\r", hist.size_out, hist.size_in);
  WriteToPKT (h, "Time transmitting (sending/receiving): %ld/%ld\r", hist.time_out, hist.time_in);
  WriteToPKT (h, "           Errors (sending/receiving): %ld/%ld\r", hist.err_out, hist.err_in);

  ClosePKT (h);

  /* VRP 990827 start */

  if (MailFlag)
  {
    FILE *fp;

    if ((MailFlag[0] == '%') && (MailFlag[1] == 's'))
      sprintf (junk1, MailFlag, flag_dir ? flag_dir : "");
    else
      strcpy (junk1, MailFlag);

    fp = fopen (junk1, write_binary);
    if (fp != NULL)
      fclose (fp);
  }

  if (NetMailFlag && PKTStats.toecho == 0)
  {
    FILE *fp;

    if ((NetMailFlag[0] == '%') && (NetMailFlag[1] == 's'))
      sprintf (junk1, NetMailFlag, flag_dir ? flag_dir : "");
    else
      strcpy (junk1, NetMailFlag);

    fp = fopen (junk1, write_binary);
    if (fp != NULL)
      fclose (fp);
  }
  else if (ArcMailFlag && PKTStats.toecho != 0)
  {
    FILE *fp;

    if ((ArcMailFlag[0] == '%') && (ArcMailFlag[1] == 's'))
      sprintf (junk1, ArcMailFlag, flag_dir ? flag_dir : "");
    else
      strcpy (junk1, ArcMailFlag);

    fp = fopen (junk1, write_binary);
    if (fp != NULL)
      fclose (fp);
  }

  /* VRP 990827 end */

  return 1;
}

void
find_event (void)
{
  time_t long_time;
  struct tm *tm;
  int cur_day;
  int our_time;
  int i;
  char cmds[PATHLEN];
  long customdate;
  BINK_EVENT evt;

  /* Get the current time into a structure */
  unix_time (&long_time);
  tm = unix_localtime (&long_time);
  customdate = custom_date (tm);
  cur_day = 1 << tm->tm_wday;
  our_time = (tm->tm_hour % 24) * 60 + (tm->tm_min % 60);
  cur_event = -1;

  if (customdate != hist.which_date)
  {
    write_stats ();

    if (hist.which_date != 0)   /* mtt971210 when evt-file ist recompiled which_date == 0 */
    {
      /* start of new day msg mtt971115 */
      /* MR 971202 moved msg to language.txt;                        */
      /*           changed date format to match general one (YYMMDD) */
      /* TE 980414 Y2K fix: tm_year will be 100 in year 2000         */
      status_line (MSG_TXT (M_NEW_DAY), tm->tm_year % 100, (tm->tm_mon + 1), tm->tm_mday);
      writeactivitydump ();     /* MR 971223 write activity dump to disk */
      movelogs ();              /* MR 971223 move local logs to logs     */
    }

    if (DoStats && hist.which_date)
    {
      if (stats_to_msg () == 0)
        status_line (":Warning: Couldn't msg stats");
      else
        status_line (":Stats written to .PKT");
    }

    // CEH; 980727
    // dont clear the "last" field in history. The last caller is even the
    // last caller when a new day has started.
    memset (&hist, 0, sizeof (HISTORY) - sizeof (hist.last_str));
    // memset (&hist, 0, sizeof (HISTORY));
    start_hist = hist;
    hist.which_date = customdate;

    do_today (1);
  }

  /* Go through the events from top to bottom */
  for (i = 0; i < num_events; i++)
  {
    if (our_time >= e_ptrs[i].minute)
    {
      evt = e_ptrs[i];

      if ((cur_day & evt.days) && !(evt.behavior & MAT_COSTEVENT) &&
          ((!evt.day) || (evt.day == (char) tm->tm_mday)) &&
          ((!evt.month) || (evt.month == (char) (tm->tm_mon + 1))))
      {
        if (((our_time - evt.minute) < evt.length) ||
            ((our_time == evt.minute) && (evt.length == 0)) ||
            ((evt.behavior & MAT_FORCED) &&
             (evt.last_ran_date != customdate)))
        {
          /* Are we not supposed to force old events */
          if (((our_time - evt.minute) > evt.length) && (noforce))
          {
            e_ptrs[i].last_ran_date = customdate;
            continue;
          }

          if (evt.last_ran_date != customdate)
          {
            cur_event = i;
            do_status (M_READY_WAITING, NULL);
            status_line (MSG_TXT (M_STARTING_EVENT),
                         i + 1, e_ptrs[i].ename);

            if (!blank_on_key)
              screen_blank = 0;

            more_mail = 1;

            /* Mark that this one is running */
            e_ptrs[i].last_ran_date = customdate;

            /* Mark that we have not yet skipped it.
             * After all, it just started! */
            e_ptrs[i].behavior &= ~MAT_SKIP;

            /* Write out the schedule */
            write_sched ();

            /* Do this first ... if any $.. to kill, do it */
            if (e_ptrs[i].behavior & MAT_KILLBAD)
              kill_bad ();

            /* If we are supposed to exit, then do it */
            if (evt.errlevel[0])
            {
              screen_blank = 0;
              errl_exit_shell (evt.errlevel[0],  /* MR 961123 */
                               MSG_TXT (M_EVENT_EXIT),
                               MSG_TXT (M_EVENT_SHELL));
            }
            else if (packer != NULL || cleanup != NULL)
            {
              if (!blank_on_key)
                screen_blank = 0;
              status_line (MSG_TXT (M_CLEAN_PACK));
              modem_busy ();
              screen_clear ();
              vfossil_cursor (1);
              close_up (0);

              if (cleanup != NULL)
              {
                strcpy (cmds, cleanup);
                if (i >= 0)
                  strcat (cmds, evt.cmd);
                b_spawn (cmds);
              }

              if (packer != NULL)
              {
                strcpy (cmds, packer);
                if (i >= 0)
                  strcat (cmds, evt.cmd);
                b_spawn (cmds);
              }

              come_back ();
              status_line (MSG_TXT (M_AFTER_CLEAN_PACK));
              modem_init ();
              waitfor_line = timerset (PER_MINUTE);
            }

            cur_event = i;
            max_connects = evt.with_connect;
            max_noconnects = evt.no_connect;
            set_up_outbound (0);
          }
          else
          {
            /* Don't do events that have been exited already */
            if (evt.behavior & MAT_SKIP)
              continue;
          }

          cur_event = i;

          /* CFS 970309 - if you add more variables that depend
           * on the event behaviour, remember to update evtsetup.c
           * too! */

          if (evt.behavior & MAT_NOREQ)
          {
            matrix_mask &= ~TAKE_REQ;
            no_requests = 1;
          }
          else
          {
            matrix_mask |= TAKE_REQ;
            no_requests = 0;
          }

          if (evt.behavior & MAT_NOOUTREQ)
            requests_ok = 0;
          else
            requests_ok = 1;

          if (evt.behavior & MAT_NOPICKUP)
          {
            pickup_ok = 0;      /* no pickup implies that i */
            requests_ok = 0;    /* cannot request something */
          }                     /* - r.hoerner -            */
          else
            pickup_ok = 1;

          max_connects = evt.with_connect;
          max_noconnects = evt.no_connect;

          break;
        }
      }
    }
  }

  if (cur_event >= 0)           /* otherwise we will see a LOT of traps... */
  {
    keeperr1 = e_ptrs[cur_event].errlevel[1];  /* CFS 970309 */
    keeperr2 = e_ptrs[cur_event].errlevel[2];
  }
  else
  {
    keeperr1 = 99;
    keeperr2 = 99;
  }
}

BINK_EVENTP
find_costevent (long t)
{
  struct tm *tm;
  time_t long_time = t;
  int cur_day;
  int our_time;
  int i;
  BINK_EVENTP evt, ret;

  tm = unix_localtime (&long_time);
  cur_day = 1 << tm->tm_wday;
  our_time = (tm->tm_hour % 24) * 60 + (tm->tm_min % 60);
  ret = NULL;

  /* Go through the events from top to bottom */
  for (i = 0; i < num_events; i++)
  {
    if (our_time >= e_ptrs[i].minute)
    {
      evt = &e_ptrs[i];

      if ((cur_day & evt->days) && (evt->behavior & MAT_COSTEVENT) &&
          ((!evt->day) || (evt->day == (char) tm->tm_mday)) &&
          ((!evt->month) || (evt->month == (char) (tm->tm_mon + 1))))
      {
        if (((our_time - evt->minute) < evt->length))
        {
          ret = evt;
          break;
        }
      }
    }
  }

  return ret;
}

int
read_sched (void)
{
  char temp1[PATHLEN], temp2[PATHLEN];
  struct stat buffer1, buffer2;
  BINK_EVENT far *tmp = NULL;
  FILE *stream;
  unsigned long was, got;
  size_t needed;

  sprintf (temp1, "%s%s.s%02x", task_dir, PRDCT_PRFX, TaskNumber);  /* TJW 960414 */

  if (strstr (eventfile, DIR_SEPS) == NULL)
    sprintf (temp2, "%s%s", BINKpath, eventfile);  /* r. hoerner */
  else
    sprintf (temp2, "%s", eventfile);  /* r. hoerner */

  if (unix_stat (temp1, &buffer1) == -1)
    return 0;

  if (unix_stat (temp2, &buffer2) == -1)
  {
    status_line ("!Cannot get stats of %s", temp2);

    if (strstr (config_name, DIR_SEPS) == NULL)
      sprintf (temp2, "%s%s", BINKpath, config_name);  /* r. hoerner */
    else
      sprintf (temp2, "%s", config_name);  /* r. hoerner */

    if (unix_stat (temp2, &buffer2) == -1)
    {
      status_line ("!Cannot get stats of %s", temp2);
      return 0;
    }
  }

  if (buffer1.st_size < sizeof (BINK_EVENT))
  {
    printf ("Re-compiling events, %s is invalid!\n", temp1);
    return 0;
  }

  if ((stream = share_fopen (temp1, read_binary, DENY_WRITE)) == NULL)
  {
    printf ("Can't open compiled events %s!\n", temp1);
    return 0;
  }

  needed = (size_t) (buffer1.st_size - 16 - sizeof (time_t) - sizeof (HISTORY));
  was = ftell (stream);
  fseek (stream, 0L, SEEK_END);
  got = ftell (stream) - 16 - sizeof (time_t) - sizeof (HISTORY);
  fseek (stream, was, SEEK_SET);

  temp1[0] = '\0';

  fread (temp1, 16, 1, stream);
  if (strcmp (temp1, BinkSched) != 0)
  {
    fclose (stream);
    printf ("Invalid scheduler version in %s!\n", temp1);
    return 0;
  }

  fread ((char *) &scdcompiletime, (size_t) sizeof (scdcompiletime), 1, stream);
  if (buffer2.st_mtime > scdcompiletime)  /* TJW 971031 evt is newer than Sxx */
  {
    fclose (stream);
    printf ("Event file is newer than compiled events - recompiling %s\n", temp1);
    return 0;
  }

  fread ((char *) &hist, (size_t) sizeof (HISTORY), 1, stream);

  start_hist = hist;

  /* r. hoerner changes begin
   * in orginal code binkley requested 32KB for 256 events, wether we really
   * had 256 events or not.
   * I change it conservativly to:
   * - request your 32 KB.
   * - if you have a binkley.s?? file:
   *   - read binkley.s?? and check, how many events you really have
   *   - release these 32 KB and request the amount really needed
   * - else continue with 32KB. */

  if ((unsigned long) needed != got)
  {
    printf ("Inconsistent data in file '%s'!\n", temp1);
    fclose (stream);
    return 0;
  }

  if (needed == 0)
  {
    printf ("ERR: No Events defined. Trying to recompile ...\n");
    fclose (stream);
    return 0;
  }

  tmp = (BINK_EVENT *) malloc (needed);
  if (tmp == (BINK_EVENT *) NULL)
  {
    printf ("not enough memory for events!\n");
    fclose (stream);
    return 0;
  }

  if (e_ptrs != NULL)
    free ((BINK_EVENT *) e_ptrs);

  e_ptrs = tmp;
  fread ((char far *) e_ptrs, (size_t) needed, 1, stream);

  /* r. hoerner: my changes ending */

  got_sched = 1;
  num_events = (int) ((buffer1.st_size - 16 - sizeof (HISTORY)) / sizeof (BINK_EVENT));

  /* MR 961117 orig: if(close(stream)!=0) */
  if (fclose (stream) != 0)     /* TJW 960420 */
    printf ("Error closing event file *.s%02x!\n", TaskNumber);

  return 1;
}

void
write_sched (void)
{
  char temp1[PATHLEN], temp2[PATHLEN];
  int i;
  struct stat buffer1;
  struct utimbuf times;
  long t;

  FILE *stream;

  /* Get the current time */
  t = unix_time (NULL);
  sprintf (temp1, "%s%s.s%02x", task_dir, PRDCT_PRFX, TaskNumber);  /* TJW 960414 */

  if (strstr (eventfile, DIR_SEPS) == NULL)
    sprintf (temp2, "%s%s", BINKpath, eventfile);  /* r. hoerner */
  else
    sprintf (temp2, "%s", eventfile);  /* r. hoerner */

  /* Get the current stat for .Evt file */
  if (unix_stat (temp2, &buffer1) == -1)
  {
    status_line ("!Cannot get stats of %s", temp2);

    if (strstr (config_name, DIR_SEPS) == NULL)
      sprintf (temp2, "%s%s", BINKpath, config_name);  /* r. hoerner */
    else
      sprintf (temp2, "%s", config_name);  /* r. hoerner */

    if (unix_stat (temp2, &buffer1) == -1)
    {
      status_line ("!Cannot get stats of %s", temp2);
      return;
    }
  }

  /* If it is newer than current time, we have a problem and we must
   * reset the file date - yucky, but it will probably work */

  if ((time_t) t < buffer1.st_mtime)
  {
    times.UT_ACTIME = times.modtime = buffer1.st_mtime;
    status_line (MSG_TXT (M_DATE_PROBLEM));
  }
  else
    times.UT_ACTIME = times.modtime = t;

  stream = share_fopen (temp1, write_binary, DENY_WRITE);
  if (stream == NULL)
  {
    status_line ("!Cannot open %s", temp1);
    return;
  }

  fwrite (BinkSched, 16, 1, stream);
  fwrite (&scdcompiletime, sizeof (scdcompiletime), 1, stream);  /* TJW 971031 put time into file */
  fwrite ((char *) &hist, (int) sizeof (HISTORY), 1, stream);

  for (i = 0; i < num_events; i++)
  {
    /* If it is skipped, but not dynamic, reset it */
    if ((!(e_ptrs[i].behavior & MAT_COSTEVENT)) &&
        (e_ptrs[i].behavior & MAT_SKIP) &&
        (!(e_ptrs[i].behavior & MAT_DYNAM)))
    {
      e_ptrs[i].behavior &= ~MAT_SKIP;
    }

  }

  fwrite ((char far *) e_ptrs,
          (size_t) (num_events * sizeof (BINK_EVENT)), 1, stream);

  fclose (stream);

  return;
}

void
write_stats (void)
{
  char temp1[PATHLEN];
  FILE *fp;

  sprintf (temp1, "%s%s.d%02x", task_dir, PRDCT_PRFX, TaskNumber);  /* TJW 960414 */

  fp = fopen (temp1, write_binary);
  if (fp != NULL)
  {
    fwrite (&hist, (int) sizeof (HISTORY), 1, fp);
    fclose (fp);
  }

  return;
}

int
time_to_next (int skip_bbs)
{
  time_t long_time;
  struct tm *tm;
  int cur_day;
  int our_time;
  int i;
  int time_to;
  int guess;
  int nmin;

  /* Get the current time into a structure */

  unix_time (&long_time);
  tm = unix_localtime (&long_time);
  our_time = tm->tm_hour * 60 + tm->tm_min;
  next_event = -1;
  cur_day = 1 << tm->tm_wday;

  /* A ridiculous number */
  time_to = 3000;

  /* Go through the events from top to bottom */
  for (i = 0; i < num_events; i++)
  {
    /* If it is a costevent, skip it */
    if (e_ptrs[i].behavior & MAT_COSTEVENT)
      continue;

    /* If it is the current event, skip it */
    if (cur_event == i)
      continue;

    /* If it is a BBS event, skip it */
    if (skip_bbs && e_ptrs[i].behavior & MAT_BBS)
      continue;

    /* If it was already run today, skip it */
    if (e_ptrs[i].last_ran_date == custom_date (tm))
      continue;

    /* If it doesn't happen today, skip it */
    if (!(e_ptrs[i].days & cur_day))
      continue;

    /* If not this day of the month, skip it */
    if (e_ptrs[i].day && (e_ptrs[i].day != (char) tm->tm_mday))
      continue;

    /* If not this month of the year, skip it */
    if (e_ptrs[i].month && (e_ptrs[i].month != (char) (tm->tm_mon + 1)))
      continue;

    /* If it is earlier than now, skip it unless it is forced */
    if (e_ptrs[i].minute <= our_time)
    {
      if (!(e_ptrs[i].behavior & MAT_FORCED))
        continue;

      /* Hmm, found a forced event that has not executed yet */
      /* Give the guy 2 minutes and call it quits */
      guess = 2;
    }
    else
    {
      /* Calculate how far it is from now */
      guess = e_ptrs[i].minute - our_time;
    }

    /* If less than closest so far, keep it */
    if (time_to > guess)
    {
      time_to = guess;
      next_event = i;
    }
  }

  /* If we still have nothing, then do it again, starting at midnight */
  if (time_to > 1440)
  {
    /* Calculate here to midnight */
    nmin = 1440 - our_time;

    /* Go to midnight */
    our_time = 0;

    /* Go to the next possible day */
    cur_day = (int) (((unsigned) cur_day) << 1);
    if (cur_day > DAY_SATURDAY)
      cur_day = DAY_SUNDAY;

    /* Go through the events from top to bottom */
    for (i = 0; i < num_events; i++)
    {
      /* If it is a costevent, skip it */
      if (e_ptrs[i].behavior & MAT_COSTEVENT)
        continue;

      /* If it is a BBS event, skip it */
      if (skip_bbs && e_ptrs[i].behavior & MAT_BBS)
        continue;

      /* If it doesn't happen today, skip it */
      if (!(e_ptrs[i].days & cur_day))
        continue;

      /* If not this day of the month, skip it */
      if (e_ptrs[i].day && (e_ptrs[i].day != (char) tm->tm_mday))
        continue;

      /* If not this month of the year, skip it */
      if (e_ptrs[i].month && (e_ptrs[i].month != (char) (tm->tm_mon + 1)))
        continue;

      /* Calculate how far it is from now */
      guess = e_ptrs[i].minute + nmin;

      /* If less than closest so far, keep it */
      if (time_to > guess)
      {
        time_to = guess;
        next_event = i;
      }
    }
  }

  if (time_to > 1440)
    time_to = 1440;

  if (skip_bbs && (time_to < 1))
    time_to = 1;

  return (time_to);
}

/* $Id: sched.c,v 1.9 1999/09/27 20:51:38 mr Exp $ */
