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
 * Filename    : $Source: E:/cvs/btxe/src/gethist/getra.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:36 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description :
 *   imports RemoteAccess' (and others) LASTCALL.BBS into Bink's history file
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

struct                          /* Lastcall */
{
  byte line;
  char pasname[36];
  char pashandle[36];
  char pascity[26];
  word baud;
  long times;
  char paslogon[6];
  char paslogoff[6];
  byte attribute;
}
lastcall;

int
gethist_ra_main (int argc, char **argv)
{
  CALLHIST rec;
  FILE *in, *out;
  char *x1, *x2, l1[7], l2[7];
  time_t curtime;
  long calc;
  unsigned task;

  tzset ();
  time (&curtime);
  printf ("RemoteAccess to BT-XE, 0.1b, by C.F.S., 2:341/70\n\n");

  if (argc != 4)
  {
    printf ("Usage  : BTUTIL GETHIST RA task_number ra_lastcall.bbs_file btxe_history_file\n");
    printf ("Example: BTUTIL GETHIST RA 2 P:\\RA\\LASTCALL.BBS P:\\BTXE\\TASK\\CALLHIST.ALL\n\n");
    return 1;
  }

  task = atoi (argv[1]);
  in = fopen (argv[2], "rb");

  if (in == NULL)
  {
    printf ("Failed to open input file %s\n", argv[1]);
    return 2;
  }

  fseek (in, -sizeof (lastcall), SEEK_END);
  fread (&lastcall, 1, sizeof (lastcall), in);
  fclose (in);
  pas2c (lastcall.pasname);
  pas2c (lastcall.pashandle);
  pas2c (lastcall.pascity);
  pas2c (lastcall.paslogon);
  pas2c (lastcall.paslogoff);
  rec.task = task;
  rec.outgoing = 0;
  rec.calltype = CT_BBS;
  rec.subclass = SC_LASTCALL;
  strcpy (l1, lastcall.paslogon);
  strcpy (l2, lastcall.paslogoff);
  *(x1 = strchr (l1, ':')) = 0;
  *(x2 = strchr (l2, ':')) = 0;
  calc = atoi (l2) * 3600 + atoi (x2 + 1) * 60;
  calc -= atoi (l1) * 3600 + atoi (x1 + 1) * 60;
  rec.length = calc;
  rec.starttime = curtime - rec.length;
  memcpy (&rec.h.b1, &lastcall, sizeof (lastcall));
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

/* $Id: getra.c,v 1.3 1999/02/27 01:16:36 mr Exp $ */
