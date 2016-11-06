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
 * Filename    : $Source: E:/cvs/btxe/src/gethist/getbgfax.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:35 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : imports BGFAX's log file into Bink's history file
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned short word;
typedef unsigned char byte;

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

/*        1         2         3         4         5         6         7
   01234567890123456789012345678901234567890123456789012345678901234567890123456789
   04-04 13:56 00:48   42351  9600 FAX0101.FAX  5938404                1  Finished
 */

char length[30], bytes[30], speed[30], filename[30], remoteid[60], pages[20],
  status[30];

int
gethist_bgfax_main (int argc, char **argv)
{
  CALLHIST rec;
  FILE *in, *out;
  char *x;
  time_t curtime;
  unsigned task;
  char line[160];

  tzset ();
  time (&curtime);
  printf ("BGFAX to BT-XE, 0.0beta, by C.F.S., 2:341/70\n\n");
  if (argc != 4)
  {
    printf ("Usage  : BTUTIL GETHIST BGFAX task_number bgfax_log_file btxe_history_file\n");
    printf ("Example: BTUTIL GETHIST BGFAX 2 P:\\BGFAX\\FAXES\\FAXIN.LOG P:\\BTXE\\TASK\\CALLHIST.ALL\n\n");
    printf ("Make sure you do NOT run this program after every BGFAX run, but only after\n");
    printf ("successfully receiving a fax, otherwise you will get duplicate entries in\n");
    printf ("BT-XE's history. BGFAX exits with errorlevel 4 if a fax has been received.\n");
    printf ("The task number is used for displaying purpuses only - but of course it should\n");
    printf ("be the number of the mailer that has received the fax.\n");
    return 1;
  }

  task = atoi (argv[1]);
  in = fopen (argv[2], "rb");
  if (in == NULL)
  {
    printf ("Failed to open input file %s\n", argv[1]);
    return 2;
  }

  fseek (in, -120, SEEK_END);
  line[fread (line, 1, 120, in)] = 0;
  fclose (in);

  if (strchr (line, '\n'))
    strcpy (line, strchr (line, '\n') + 1);

  printf ("Read: \n%s\n", line);
  line[5] = line[11] = line[17] = line[25] = line[31] = line[43] = line[65] = line[69] = 0;
  strcpy (length, line + 12);
  strcpy (bytes, line + 18);
  strcpy (speed, line + 26);
  strcpy (filename, line + 32);
  strcpy (remoteid, line + 45);
  strcpy (pages, line + 66);
  strcpy (status, line + 71);
  pack (length);
  pack (bytes);
  pack (speed);
  pack (filename);
  pack (remoteid);
  pack (pages);
  pack (status);
  printf ("Length: %s\n", length);
  printf ("Bytes: %s\n", bytes);
  printf ("Speed: %s\n", speed);
  printf ("Filename: %s\n", filename);
  printf ("Remote ID: %s\n", remoteid);
  printf ("Pages: %s\n", pages);
  printf ("Status: %s\n", status);
  rec.task = task;
  rec.outgoing = 0;
  rec.calltype = CT_FAX;
  rec.subclass = 0;             /* Subclass is currently unused in fax calls */
  *(x = strchr (length, ':')) = 0;
  rec.length = atoi (length) * 60 + atoi (x + 1);
  rec.starttime = curtime - rec.length;
  strcpy (rec.h.f.remoteid, remoteid);
  rec.h.f.pages = atoi (pages);
  rec.h.f.bytes = atoi (bytes);
  rec.h.f.speed = atoi (speed);
  strcpy (rec.h.f.filename, filename);
  strcpy (rec.h.f.status, status);
  out = fopen (argv[3], "a+b");

  if (out == NULL)
  {
    printf ("Unable to open output file %s\n", argv[3]);
    return 3;
  }

  fwrite (&rec, 1, sizeof (CALLHIST), out);
  fclose (out);
  printf ("Done.\n");
  return 0;
}

/* $Id: getbgfax.c,v 1.3 1999/02/27 01:16:35 mr Exp $ */
