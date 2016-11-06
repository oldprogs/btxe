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
 * Filename    : $Source: E:/cvs/btxe/src/gethist/getmax3.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:50 $
 * State       : $State: Exp $
 * Orig. Author: Alex Woick, 2:244/1351
 *
 * Description :
 *   Imports Maximus CBCS 3.0x CALLINFO.BBS/LASTUSxx.BBS into Bink's history
 *   file.
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <ctype.h>
#ifdef __EMX__                  //  TS971229, at least required for emx/gcc
# include <sys/types.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <share.h>

/* this is max.h from Maximus 3.01 "structs.zip" package */
#include "third/max3/max.h"

typedef struct _ADDRESS
{
  word Zone;
  word Net;
  word Node;
  word Point;
  char *Domain;
}
ADDR, *ADDRP;

#include "history.h"
#include "gethcomm.h"

#define YEAR(t)   (((t & 0xFE00) >> 9) + 1980)
#define MONTH(t)  ((t & 0x01E0) >> 5)
#define DAY(t)    (t & 0x001F)

#define HOUR(t)   ((t & 0xF800) >> 11)
#define MINUTE(t) ((t & 0x07E0) >> 5)
#define SECOND(t) ((t & 0x001F) << 1)

struct m_pointers prm;
char *offsets;

static int
ReadMaxPrm (const char *maxprmname)
{
  int prmfile;
  long heaplen;
  long numread;

  prmfile = shopen (maxprmname, O_RDONLY | O_BINARY);
  if (prmfile != -1)
  {
    numread = read (prmfile, &prm, sizeof (struct m_pointers));
    if (numread != sizeof (struct m_pointers))
    {
      close (prmfile);
      return -1;
    }

    heaplen = filelength (prmfile) - prm.heap_offset;
    offsets = (char *) malloc (heaplen);

    if (offsets)
    {
      lseek (prmfile, prm.heap_offset, SEEK_SET);
      read (prmfile, offsets, heaplen);
      close (prmfile);
      return 0;
    }
  }

  return -1;
}

static char *
nicepath (char *s)              /* writes a backslash to the end of the string */
{
  int i;

  if (s == NULL)
    return (s);

  i = strlen (s);
  if (i > 0)
  {
    if (s[i - 1] != '\\')
    {
      strcat (s, "\\");
    }
  }

  return (s);
}


static int
GetMax3Lastcall (CALLHIST * phist, struct callinfo *pcallinfo, struct _usr *plastuser, word task)
{
  struct tm st;

  memset (phist, 0, sizeof (CALLHIST));

  strncpy (phist->h.b2.name, (char *) plastuser->name, sizeof (phist->h.b2.name));
  strncpy (phist->h.b2.handle, (char *) plastuser->alias, sizeof (phist->h.b2.handle));
  strncpy (phist->h.b2.city, (char *) pcallinfo->city, sizeof (phist->h.b2.city));
  phist->h.b2.filesup = pcallinfo->filesup;
  phist->h.b2.filesdn = pcallinfo->filesdn;
  phist->h.b2.kbup = pcallinfo->kbup;
  phist->h.b2.kbdn = pcallinfo->kbdn;
  phist->h.b2.calls = pcallinfo->calls;
  phist->h.b2.read = pcallinfo->read;
  phist->h.b2.posted = pcallinfo->posted;
  phist->h.b2.baud = plastuser->delflag;

  phist->task = task;
  phist->outgoing = 0;
  phist->calltype = CT_BBS;
  phist->subclass = SC_CALLINFO;

  st.tm_sec = SECOND (pcallinfo->login.dos_st.time);
  st.tm_min = MINUTE (pcallinfo->login.dos_st.time);
  st.tm_hour = HOUR (pcallinfo->login.dos_st.time);
  st.tm_mday = DAY (pcallinfo->login.dos_st.date);
  st.tm_mon = MONTH (pcallinfo->login.dos_st.date) - 1;
  st.tm_year = YEAR (pcallinfo->login.dos_st.date) % 100;  /* AG 990120 Y2K fix */
  st.tm_isdst = -1;
  phist->starttime = mktime (&st);

  printf ("Logon:  %02d.%02d.%02d %02d:%02d:%02d (%ul)\n", st.tm_mday, st.tm_mon, st.tm_year, st.tm_hour, st.tm_min, st.tm_sec, phist->starttime);

  st.tm_sec = SECOND (pcallinfo->logoff.dos_st.time);
  st.tm_min = MINUTE (pcallinfo->logoff.dos_st.time);
  st.tm_hour = HOUR (pcallinfo->logoff.dos_st.time);
  st.tm_mday = DAY (pcallinfo->logoff.dos_st.date);
  st.tm_mon = MONTH (pcallinfo->logoff.dos_st.date) - 1;
  st.tm_year = YEAR (pcallinfo->logoff.dos_st.date) % 100;  /* AG 990120 Y2K fix */
  st.tm_isdst = -1;

  printf ("Logoff: %02d.%02d.%02d %02d:%02d:%02d (%ul)\n", st.tm_mday, st.tm_mon, st.tm_year, st.tm_hour, st.tm_min, st.tm_sec, mktime (&st));

  phist->length = mktime (&st) - phist->starttime;

  printf ("Length: %ld seconds\n", phist->length);

  return 0;
}


static void
help (void)
/* show short usage help */
{
  printf ("Usage   : BTUTIL GETHIST MAX3 [/h:<btxe_hist_file>] [/m:<max.prm>] [/t:<task#>]\n");
  printf ("Example : BTUTIL GETHIST MAX3 /h:P:\\BT\\TASK\\ALLTASKS.HST /m:L:\\MAX\\MAX.PRM /t:2\n\n");
  printf ("Defaults: btxe_hist_file: %%BINKLEY%%\\TASK\\CALLHIST.ALL\n");
  printf ("          max.prm:        %%MAXIMUS%%\n");
  printf ("          task#:          %%TASK%%, if not defined then from max.prm\n");
}

int
gethist_max3_main (int argc, char **argv)
{
  CALLHIST rec;

  char tmp[255], tmp2[255];
  char *s, *opt;
  char shistfname[255];
  char *smaxprm;
  char *stasknr;

  unsigned tasknr = 0;
  FILE *callerbbs;
  FILE *lastuserbbs;
  FILE *callhist;

  struct _usr lastuserrec;
  struct callinfo callinforec;

  int i;

  /* TJW: Read TZ variable and adapt time functions */
  tzset ();

  printf ("Maximus CBCS 3.0x to BT-XE, 1.0, by Alex Woick, 2:244/1351\n\n");

  /* set the defaults */
  strcpy (shistfname, "CALLHIST.ALL");
  s = getenv ("BINKLEY");
  if (s)
  {
    strcpy (shistfname, s);
    nicepath (shistfname);
    strcat (shistfname, "TASK\\CALLHIST.ALL");
  }

  smaxprm = getenv ("MAXIMUS");
  stasknr = getenv ("TASK");

  for (i = 1; i < argc; i++)
  {
    opt = argv[i];
    if (strlen (opt) > 3 && (opt[0] == '/' || opt[0] == '-') && opt[2] == ':')
    {
      switch (toupper (opt[1]))
      {
      case 'H':
        strcpy (shistfname, opt + 3);
        break;

      case 'M':
        smaxprm = opt + 3;
        break;

      case 'T':
        stasknr = opt + 3;
        break;

      default:
        help ();
        return 1;
        break;
      }
    }
    else
    {
      help ();
      return 1;
    }
  }

  if (stasknr)
  {
    tasknr = atoi (stasknr);
  }

  printf ("tasknr=%u\n", tasknr);

  if (tasknr == 0)
  {
    printf ("task number not given or not >=1: '%s'\n", stasknr);
    return -1;
  }

  if (ReadMaxPrm (smaxprm))
  {
    printf ("cannot read Maximus .PRM file '%s'\n", smaxprm);
    return -1;
  }

  strcpy (tmp, PRM (sys_path));
  nicepath (tmp);

  sprintf (tmp2, "%slastus%02x.bbs", tmp, tasknr);
  printf ("lastuser.bbs='%s'\n", tmp2);
  lastuserbbs = fopen (tmp2, "rb");
  if (!lastuserbbs)
  {
    printf ("cannot open '%s'\n", tmp2);
    free (offsets);
    return -1;
  }

  fread (&lastuserrec, 1, sizeof (struct _usr), lastuserbbs);

  fclose (lastuserbbs);
  sprintf (tmp2, "%s.bbs", PRM (caller_log));
  printf ("callers logfile='%s'\n", tmp2);

  callerbbs = fopen (tmp2, "rb");
  if (!callerbbs)
  {
    printf ("cannot open callers logfile '%s'\n", tmp2);
    free (offsets);
    return -1;
  }

  printf ("BT XE history file='%s'\n", shistfname);

  for (i = 1; i <= 255; i++)
  {                             /* search the last 255 entries */
    fseek (callerbbs, -(long) i * (long) sizeof (struct callinfo), SEEK_END);
    fread (&callinforec, 1, sizeof (struct callinfo), callerbbs);

    if (callinforec.task == tasknr)
    {                           /* found the last entry in caller.bbs for this task */
      printf ("Found last user: %s (task=%u)\n", callinforec.name, callinforec.task);
      GetMax3Lastcall (&rec, &callinforec, &lastuserrec, tasknr);

      callhist = fopen (shistfname, "a+b");
      if (!callhist)
      {
        printf ("Unable to open output file '%s'\n", shistfname);
        return -1;
      }

      fwrite (&rec, 1, sizeof (CALLHIST), callhist);
      fclose (callhist);
      printf ("Done.\n");
      break;
    }
  }

  if (i == 256)
    printf ("Cannot find entry for last user in '%s'\n", tmp2);

  fclose (callerbbs);
  free (offsets);

  return 0;
}

/* $Id: getmax3.c,v 1.5 1999/03/22 03:47:50 mr Exp $ */
