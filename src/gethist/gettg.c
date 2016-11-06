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
 * Filename    : $Source: E:/cvs/btxe/src/gethist/gettg.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/04/01 00:52:19 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : imports Telegard's LASTON.DAT into Bink's history file
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef long int unixtime;      /* unix time stamp - seconds */
typedef unsigned char boolean;  /* Turbo Pascal "boolean" */
typedef unsigned char byte;     /* Turbo Pascal "byte" */
typedef signed short int integer;  /* Turbo Pascal "integer" */
typedef signed long int longint;  /* Turbo Pascal "longint" */
typedef unsigned char string;   /* Turbo Pascal "string" */
typedef unsigned short int word;  /* Turbo Pascal "word" */
typedef unsigned int bit;       /* shorthand for C bit fields */

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

typedef struct                  /* LASTON.DAT  - Last callers records */
{
  longint caller;               /* system caller number */
  string realname[37];          /* real name of caller */
  string handle[37];            /* user name of caller */
  string location[31];          /* location of caller */
  longint logonspeed;           /* logon speed */
  unixtime logontime;           /* logon time */
  boolean newuser;              /* was newuser? */
  byte reserved1;               /* RESERVED */
  boolean hidden;               /* hidden from display */
  byte node;                    /* node number */
  unixtime logofftime;          /* logoff time */
  word uploads;                 /* number of uploads */
  word downloads;               /* number of downloads */
  longint uk;                   /* number of upload kB */
  longint dk;                   /* number of download kB */
  byte reserved2[8];            /* RESERVED */
}
lcallers;

int
gethist_telegard_main (int argc, char **argv)
{
  CALLHIST rec;
  FILE *in, *out;
  lcallers last;

  tzset ();
  printf ("Telegard to BT-XE, 0.1b, by C.F.S., 2:341/70\n\n");

  if (argc != 3)
  {
    printf ("Usage  : BTUTIL GETHIST TELEGARD laston.dat btxe_history_file\n");
    printf ("Example: BTUTIL GETHIST TELEGARD P:\\TG\\LASTON.DAT P:\\BTXE\\TASK\\CALLHIST.ALL\n\n");
    return 1;
  }

  in = fopen (argv[1], "rb");
  if (in == NULL)
  {
    printf ("Failed to open input file %s\n", argv[1]);
    return 2;
  }

  fseek (in, -sizeof (lcallers), SEEK_END);
  fread (&last, 1, sizeof (lcallers), in);
  fclose (in);
  pas2c (last.realname);
  pas2c (last.handle);
  pas2c (last.location);
  rec.task = last.node;
  rec.outgoing = 0;
  rec.calltype = CT_BBS;
  rec.subclass = SC_TELEGARD;
  rec.starttime = last.logontime;
  if (last.logofftime > last.logontime)
    rec.length = last.logofftime - last.logontime;
  else
    rec.length = 0;

  memcpy (&rec.h.b3, &last, sizeof (lcallers));
  out = fopen (argv[2], "a+b");
  if (out == NULL)
  {
    printf ("Unable to open output file %s\n", argv[2]);
    return 3;
  }

  fwrite (&rec, 1, sizeof (CALLHIST), out);
  fclose (out);
  printf ("Done.\n");
  return 0;
}

/* $Id: gettg.c,v 1.4 1999/04/01 00:52:19 mr Exp $ */
