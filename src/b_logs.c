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
 * Filename    : $Source: E:/cvs/btxe/src/b_logs.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:34 $
 * State       : $State: Exp $
 *
 * Description : log handling
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

static char *LOCALFUNC status_log_path (void);
static char *LOCALFUNC cost_log_path (void);

static char temp[PATHLEN];      /* MMP 960419 - Ugly, i know :-( */

static char *LOCALFUNC
status_log_path (void)
{
  if (locallogs || log_name == NULL)
    sprintf (temp, "%s%s.l%02x", task_dir, PRDCT_PRFX, TaskNumber);
  else
    strntcpy (temp, log_name, PATHLEN);
  return (temp);
}

static char *LOCALFUNC
cost_log_path (void)
{
  if (locallogs || cost_log_name == NULL)
    sprintf (temp, "%s%s.c%02x", task_dir, PRDCT_PRFX, TaskNumber);
  else
    strntcpy (temp, cost_log_name, PATHLEN);
  return (temp);
}

static FILE *
movelog (FILE * in, char *localpath, char *globalpath)
{
  FILE *out;
  char *mode;
  int ch;
  int ok = 0;

  if (in == NULL)
    return (NULL);

  fclose (in);

  if ((in = share_fopen (localpath, read_binary, DENY_WRITE)) != NULL)
  {
    if ((out = share_fopen (globalpath, append_binary, DENY_WRITE)) != NULL)
    {
      while ((ch = fgetc (in)) != EOF)
        fputc (ch, out);
      ok = 1;
      if (ferror (in) || ferror (out))
        ok = 0;
      if (fclose (out))
        ok = 0;
    }
    if (fclose (in))
      ok = 0;
  }

  mode = ok ? write_ascii_plus : append_ascii;
  return (share_fopen (localpath, mode, DENY_WRITE));
}


void
openlogs (int newlineflag)
{
  if (log_name && status_log == NULL)
  {
    status_log = share_fopen (status_log_path (), append_ascii, DENY_WRITE);
    if (status_log == NULL)
    {
      printf ("\n%s\n", MSG_TXT (M_NO_LOGFILE));
      free (log_name);
      log_name = NULL;
    }
    else if (newlineflag)
    {                           /* TJW 960527 */
      fprintf (status_log, "\n");
      fflush (status_log);
      real_flush (fileno (status_log));
    }
  }
  if (cost_log_name && cost_log == NULL)
  {
    cost_log = share_fopen (cost_log_path (), append_ascii, DENY_WRITE);
    if (cost_log == NULL)
    {
      printf ("\n%s\n", MSG_TXT (M_NO_COSTFILE));
      free (cost_log_name);
      cost_log_name = NULL;
    }
  }
}

void
read_in_log (void)
{
  char in_line[256], *p, *sp;
  char logchar, logdate[20], logtime[20], logproduct[10], logtext[256];
  int hr, mi, sc, hs, count;
  char logline[256];
  char hundredths[10];
  FILE *fp;

  if (!readlog_name)
    return;

  if ((fp = share_fopen (readlog_name, read_ascii, DENY_WRITE)) != NULL)
  {
    while (!feof (fp))
    {
      if (fgets (in_line, sizeof (in_line), fp) && strlen (in_line) > 1)
      {

        p = in_line;
        p[strlen (p) - 1] = '\0';  /* kill \n at end */

        logchar = *p++;

        sp = p = skip_blanks (p);  /* skip to day start */
        p = skip_to_word (p);   /* skip to month start */
        p = skip_to_blank (p);  /* skip to blank after month */
        *p++ = '\0';            /* terminate here */
        strntcpy (logdate, sp, 20);  /* copy this to logdate */

        sp = p = skip_blanks (p);  /* skip to time start */
        p = skip_to_blank (p);  /* skip to blank after time */
        *p++ = '\0';            /* terminate here */
        hr = mi = sc = -1;
        hs = 0;
        count = sscanf (sp, "%d:%d:%d.%d", &hr, &mi, &sc, &hs);
        if (count < 3
            || hr < 0 || hr > 24
            || mi < 0 || mi > 60
            || sc < 0 || sc > 60
            || hs < 0 || hs > 100)
          continue;             /* parsing time failed - continue */

        sp = p = skip_blanks (p);  /* skip to product */
        p = skip_to_blank (p);  /* skip to blank after product */
        *p++ = '\0';            /* terminate here */
        strntcpy (logproduct, sp, 10);  /* copy this to product */

        sp = p = skip_blanks (p);  /* skip to text */
        strntcpy (logtext, sp, 256);  /* copy this to text */

        if (status_log)
        {
          if (!nologhundredths)
            sprintf (hundredths, ".%02d", hs);
          else
            hundredths[0] = '\0';
          sprintf (logtime, "%02d:%02d:%02d%s", hr, mi, sc, hundredths);
          sprintf (logline, "%c %s %s %-4s %s\n",
                   logchar, logdate, logtime, logproduct, logtext);
          fputs (logline, status_log);
        }

        sprintf (logtime, "%02d:%02d:%02d", hr, mi, sc);
        sprintf (logline, "%c%s", logchar, logtext);
        log_screen (logtime, logline);
      }
    }

    fclose (fp);
    unlink (readlog_name);
  }
}

void
closelogs (void)                /* TJW 960527 */
{
  movelogs ();                  /* MR 970503 */

  if (status_log != NULL)
  {
    fflush (status_log);
    real_flush (fileno (status_log));
    fclose (status_log);        /*mtt 96-05-17 */
    status_log = NULL;
  }

  if (cost_log != NULL)
  {
    fflush (cost_log);
    real_flush (fileno (cost_log));
    fclose (cost_log);          /*mtt 96-05-17 */
    cost_log = NULL;
  }
}

void
movelogs (void)
{
  if (locallogs)
  {
    status_log = movelog (status_log, status_log_path (), log_name);
    cost_log = movelog (cost_log, cost_log_path (), cost_log_name);
  }
}

/* $Id: b_logs.c,v 1.5 1999/03/23 22:28:34 mr Exp $ */
