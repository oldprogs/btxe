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
 * Filename    : $Source: E:/cvs/btxe/src/debug.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/05 02:51:46 $
 * State       : $State: Exp $
 *
 * Description : debug code
 *
 *---------------------------------------------------------------------------*/

#ifdef DEBUG

/* #include <stdio.h>
 * #include <stdlib.h>
 */

#include "includes.h"

/* FILE *share_fopen (char *fname, char *mode, int shflag); */
void dostime (int *, int *, int *, int *, int *);

int allocs = 0, filesopen = 0;
static FILE *debug_files_log, *debug_mem_log;

void
debug_files_log_open (int TaskNumber)
{
  static char fname[20] = "";

  sprintf (fname, "dbgfiles.l%02x", TaskNumber);
  debug_files_log = fopen (fname, append_ascii);
}

void
debug_files_log_close (void)
{
  fclose (debug_files_log);
}

void
debug_mem_log_open (int TaskNumber)
{
  static char fname[20] = "";

  sprintf (fname, "dbgalloc.l%02x", TaskNumber);
  debug_mem_log = fopen (fname, append_ascii);
}

void
debug_mem_log_close (void)
{
  fclose (debug_mem_log);
}

/* MR 970308 added time to debug log */

static int wday, hours, mins, secs, hths;

FILE *
debug_share_fopen (char *fname, char *mode, int shflag, int TaskNumber)
{
  FILE *fp;

  fp = share_fopen (fname, mode, shflag);
  if (fp)
    filesopen++;

  debug_files_log_open (TaskNumber);
  dostime (&wday, &hours, &mins, &secs, &hths);

  fprintf (debug_files_log,
           "%02i:%02i:%02i.%02i %08p %8d %-8s %-40s %-4s %4x\n",
           hours, mins, secs, hths,
           fp, filesopen, "sh_fopen", fname, mode, shflag);

  debug_files_log_close ();
  return fp;
}

FILE *
debug_fopen (char *fname, char *mode, int TaskNumber)
{
  FILE *fp;

  fp = fopen (fname, mode);
  if (fp)
    filesopen++;

  debug_files_log_open (TaskNumber);
  dostime (&wday, &hours, &mins, &secs, &hths);

  fprintf (debug_files_log,
           "%02i:%02i:%02i.%02i %08p %8d %-8s %-40s %-4s %4s\n",
           hours, mins, secs, hths,
           fp, filesopen, "fopen", fname, mode, "");

  debug_files_log_close ();
  return fp;
}

int
debug_fclose (FILE * fp, int TaskNumber)
{
  int rc;

  rc = fclose (fp);
  if (!rc && fp)
    filesopen--;

  debug_files_log_open (TaskNumber);
  dostime (&wday, &hours, &mins, &secs, &hths);

  fprintf (debug_files_log,
           "%02i:%02i:%02i.%02i %08p %8d %-8s %-40s %-4s %4s %d\n",
           hours, mins, secs, hths,
           fp, filesopen, "fclose", "", "", "", rc);

  debug_files_log_close ();
  return rc;
}

void *
debug_calloc (size_t x, size_t y, int TaskNumber, char *cfname, int cline)
{
  void *p;

  p = calloc (x, y);
  if (p)
    allocs++;

  debug_mem_log_open (TaskNumber);
  dostime (&wday, &hours, &mins, &secs, &hths);

  fprintf (debug_mem_log,
           "%02i:%02i:%02i.%02i %08p %8d %-8s %8d %8d %d %s\n",
           hours, mins, secs, hths,
           p, allocs, "calloc", x, y, cline, cfname);

  debug_mem_log_close ();
  return p;
}

void *
debug_malloc (size_t amount, int TaskNumber, char *cfname, int cline)
{
  void *p;

  p = malloc (amount);
  if (p)
    allocs++;

  debug_mem_log_open (TaskNumber);
  dostime (&wday, &hours, &mins, &secs, &hths);

  fprintf (debug_mem_log,
           "%02i:%02i:%02i.%02i %08p %8d %-8s %8d %8s %d %s\n",
           hours, mins, secs, hths,
           p, allocs, "malloc", amount, "", cline, cfname);

  debug_mem_log_close ();
  return p;
}

void
debug_free (void *p, int TaskNumber, char *cfname, int cline)
{
  free (p);
  if (p)
    allocs--;

  debug_mem_log_open (TaskNumber);
  dostime (&wday, &hours, &mins, &secs, &hths);

  fprintf (debug_mem_log,
           "%02i:%02i:%02i.%02i %08p %8d %-8s %8s %8s %d %s\n",
           hours, mins, secs, hths,
           p, allocs, "free", "", "", cline, cfname);

  debug_mem_log_close ();
}

#endif

/* $Id: debug.c,v 1.4 1999/03/05 02:51:46 mr Exp $ */
