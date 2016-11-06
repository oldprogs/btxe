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
 * Filename    : $Source: E:/cvs/btxe/src/sendbark.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/20 04:08:26 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm SEAdog Mail Session Routines.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
SEA_sendreq ()
{
  char fname[PATHLEN];
  char reqf[PATHLEN];
  char *reqtime = " 0";
  char *p, *name, *pw;
  char *updtime;
  FILE *fp;
  char *HoldName;
  ADDR tmp;

  HoldName = HoldAreaNameMunge (&called_addr);

  tmp = called_addr;

  /* For a point, massage the address to get the right .REQ filename */
  if (tmp.Point != 0)
  {
    tmp.Node = tmp.Point;
    tmp.Point = 0;
    tmp.Net = (pvtnet > 0) ? (unsigned int) pvtnet : 0;
  }

  sprintf (fname, "%s%s.req", HoldName, Hex_Addr_Str (&tmp));

  /* If we have file requests, then do them */
  if (!dexists (fname))
    status_line (MSG_TXT (M_NO_OUT_REQUESTS));
  else
  {
    status_line (MSG_TXT (M_OUT_REQUESTS));
    /* Open the .REQ file */
    fp = fopen (fname, read_ascii);
    if (fp == NULL)
    {
      Bark_Request (CURRENT.sc_Inbound, NULL, NULL, NULL);
      return;
    }

    /* As long as we do not have EOF, read the request */
    while ((fgets (reqf, 79, fp) != NULL) && (CARRIER))
    {
      /* Properly format the request */

      /* First get rid of the trailing junk */
      p = reqf + strlen (reqf) - 1;
      while ((p >= reqf) && (isspace (*p)))
        *p-- = '\0';

      /* Now get rid of the beginning junk */
      p = reqf;
      p = skip_blanks (p);

      /* This is where the name starts */
      name = p;

      /* If the first char is ; then ignore the line */
      if (*name == ';')
        continue;

      /* Now get to where the name ends */
      p = skip_to_blank (p);

      updtime = reqtime;        /* Default to request        */
      pw = p;                   /* This is or will be a null */

    req_scan:

      if (*p)
      {
        *p++ = '\0';

        while ((*p) && (*p != '!') && (*p != '-') && (*p != '+'))
          p++;

        /* Check for a password */
        if (*p == '!')
        {
          *p = ' ';
          pw = p++;

          /* Now get to where the password ends */
          p = skip_to_blank (p);
          goto req_scan;
        }

        /* Try for an update request */
        if (*p == '+')
        {
          *p = ' ';
          updtime = p++;

          /* Now get to where the update time ends */
          p = skip_to_blank (p);
          *p = '\0';

          /* Request time is last thing we care about on line, so
           * we just drop out of the loop after finding it. */
        }

        /* Note: since SEAdog doesn't do "backdate" requests, all we
         * have to do if we see a '-' is to do a full request. Hence
         *  we only process a '+' here. */
      }

      if (Bark_Request (CURRENT.sc_Inbound, name, pw, updtime))
        continue;
    }

    fclose (fp);
    unlink (fname);
    status_line (MSG_TXT (M_END_OUT_REQUESTS));
  }

  /* Finish the file requests off */
  Bark_Request (CURRENT.sc_Inbound, NULL, NULL, NULL);

}

int SBInit (BARKARGSP);
int SBEnd (BARKARGSP);
int SBSendBark (BARKARGSP);
int SBAskFile (BARKARGSP);
int SBRcvFile (BARKARGSP);
int SBNxtFile (BARKARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (BARKARGSP);
}
SSTATES, *SSTATEP;

SSTATES Bark_Sender[] =
{
  {"SBInit", SBInit},
  {"SBEnd", SBEnd},
  {"SB0", SBSendBark},
  {"SB1", SBAskFile},
  {"SB2", SBRcvFile},
  {"SB3", SBNxtFile},
};

void
Build_Bark_Packet (BARKARGSP args)
{
  char *p, *q;
  word crc;

  p = args->barkpacket;
  *p++ = ACK;
  crc = 0;
  q = args->filename;
  while (*q)
  {
    *p++ = *q;
    crc = xcrc (crc, (byte) (*q));
    ++q;
  }

  q = args->barktime;
  while (*q)
  {
    *p++ = *q;
    crc = xcrc (crc, (byte) (*q));
    ++q;
  }

  q = args->barkpw;
  while (*q)
  {
    *p++ = *q;
    crc = xcrc (crc, (byte) (*q));
    ++q;
  }

  *p++ = ETX;
  *p++ = (char) (crc & 0xff);
  *p++ = (char) (crc >> 8);

  args->barklen = (int) (p - args->barkpacket);
}

int
SBInit (BARKARGSP args)
{
  XON_DISABLE ();
  happy_compiler = args->tries; /* Makes the compiler happy! */
  return ((int) args->control);
}

int
SBEnd (BARKARGSP args)
{
  happy_compiler = args->tries; /* Makes the compiler happy! */
  return ((int) args->control);
}

int
SBSendBark (BARKARGSP args)
{
  if ((args->filename != NULL) && (args->filename[0] != '\0'))
  {
    status_line ("%s '%s' %s%s", MSG_TXT (M_MAKING_FREQ), args->filename, (*(args->barkpw)) ? "with password" : "", args->barkpw);
    args->tries = 0;
    Build_Bark_Packet (args);
    return (SB1);
  }
  else
  {
    SENDBYTE (ETB);
    /* Delay one second */
    big_pause (1);
    return (SUCCESS);
  }
}

int
SBAskFile (BARKARGSP args)
{
  SENDCHARS (args->barkpacket, args->barklen, 1);
  return (SB2);
}

int
SBRcvFile (BARKARGSP args)
{
  int c;
  int ret;

  ret = SB2;
  while (CARRIER && (ret == SB2))
  {
    c = TIMED_READ (10);

    if (c == ACK)
    {
      Batch_Receive (args->inbound);
      ret = SB3;
      break;
    }

    if (args->tries > 5)
    {
      /* Report transfer failed */
      SENDBYTE (ETB);
      ret = SENDBLOCK_ERR;
      break;
    }

    CLEAR_INBOUND ();
    ++(args->tries);
    ret = SB1;
  }

  if (!CARRIER)
    return (CARRIER_ERR);
  else
    return (ret);
}

int
SBNxtFile (BARKARGSP args)
{
  short c;
  long SB3Timer;

  SB3Timer = timerset (45 * PER_SECOND);
  happy_compiler = args->tries; /* Makes the compiler happy! */

  while (CARRIER && !timeup (SB3Timer))
  {
    if ((unsigned short) (c = TIMED_READ (5)) == 0xffff)
    {
      SENDBYTE (SUB);
      continue;
    }

    if (c == ENQ)
      return (SUCCESS);
  }

  if (CARRIER)
    return (CARRIER_ERR);
  else
    return (TIME_ERR);
}

int
Bark_Request (char *where, char *filename, char *pw, char *updtime)
{
  BARKARGS bark;
  int res;

  bark.inbound = where;
  bark.filename = filename;
  bark.barkpw = pw;
  bark.barktime = updtime;
  res = state_machine ((STATEP) Bark_Sender, &bark, SB0);
  status_line (">Bark_Request(%s) returns %d",
               filename == NULL ? "(null)" : filename, res);
  return res;
}

/* $Id: sendbark.c,v 1.5 1999/03/20 04:08:26 mr Exp $ */
