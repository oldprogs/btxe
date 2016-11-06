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
 * Filename    : $Source: E:/cvs/btxe/src/pktmsgs.c,v $
 * Revision    : $Revision: 1.10 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/08/23 00:27:09 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : PKT interface for Binkley
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

#ifdef PORTABLEBINFILES
#include "foffsets.h"
#endif

/* A quick explanation of the MSGID generation. I don't like playing
   russian roulette, so CRCs, random numbers and other methods that can
   produce dupe MSGIDs are out. FTS-0009 says that all MSGIDs generated
   by a system for three years must be unique.
   I use 5 bits for the task number, 9 for day of the year, 2 for the
   year (year%3 is enough to meet FTS-0009), 12 for a count of seconds of
   the day with a 1:30 resolution (seconds/30) and 4 for an internal count.
   It is obvious that the only possibility of getting dupes MSGID is to
   generate more than 16 messages (by the same task) in the same
   30-seconds frame. */

union uMSGID
{
  struct
  {
    unsigned TaskNumber:5;
    unsigned DayOfYear:9;
    unsigned Year:2;            /* Enough to meet the three-year rule */
    unsigned Secs30:12;
    unsigned Count:4;
  }
  x;
  unsigned long number;
};

int MSGIDcount = 0;

void
FindUniquePKT (char *storage, char *dir)
{
  char rname[9], temp[80];
  int c, r;
  FILE *o;

  for (;;)                      /* Keep trying until we succeed */
  {
    for (c = 0; c < 6; c++)
    {
      r = rand () % 16;
      if (r >= 10)
        rname[c] = r + 'A' - 10;
      else
        rname[c] = r + '0';
    }

    sprintf (rname + 6, "%02d", TaskNumber % 100);
    rname[8] = 0;
    strcpy (temp, dir);

    if (temp[strlen (temp) - 1] != DIR_SEPC)
      strcat (temp, DIR_SEPS);

    sprintf (storage, "%s%s.pkt", temp, rname);
    o = fopen (storage, read_binary);

    if (o != NULL)
      fclose (o);
    else                        /* Failed to open it - finally... */
      return;
  }
}

struct S_PKTh *
CreateMSGinPKT (char *dir, ADDR fromsystem, ADDR tosystem,
                char *fromname, ADDR fromaddr,
                char *toname, ADDR toaddr, char *subject,
                word attribs, char *area, char *password,
                struct tm *dattim)
{
  struct S_PKTh *newpkt;
  char PKTName[80];
  time_t now;
  struct _pkthdr39 PKTheader;
  struct S_Packed Msgheader;
  struct tm *nowsp;
  union uMSGID MSGID;
  newpkt = (struct S_PKTh *) malloc (sizeof (struct S_PKTh));

  if (!newpkt)
    return NULL;

  FindUniquePKT (PKTName, dir);
  newpkt->pktname = ctl_string (PKTName);
  newpkt->handle = fopen (PKTName, write_binary_plus);

  if (newpkt->handle == NULL)
  {
    status_line (":Failed to create %s", PKTName);
    free (newpkt);
    return NULL;
  }

  memcpy (&newpkt->originaddress, &fromaddr, sizeof (ADDR));
  memcpy (&newpkt->targetaddress, &toaddr, sizeof (ADDR));

  if (dattim == NULL)
  {
    time (&now);
    nowsp = localtime (&now);
  }
  else
    nowsp = dattim;

  MSGID.x.TaskNumber = TaskNumber % 32;
  MSGID.x.DayOfYear = nowsp->tm_yday;
  MSGID.x.Year = nowsp->tm_year % 4;
  MSGID.x.Secs30 = (nowsp->tm_hour) * 120 + (nowsp->tm_min) * 2 + nowsp->tm_sec / 30;
  MSGID.x.Count = (MSGIDcount++) % 16;
  PKTheader.orig_node = fromsystem.Node;
  PKTheader.dest_node = tosystem.Node;
  PKTheader.year = nowsp->tm_year + ((nowsp->tm_year < 70) ? 2000 : 1900);
  PKTheader.month = nowsp->tm_mon;
  PKTheader.day = nowsp->tm_mday;
  PKTheader.hour = nowsp->tm_hour;
  PKTheader.minute = nowsp->tm_min;
  PKTheader.second = nowsp->tm_sec;
  PKTheader.rate = 0;
  PKTheader.ver = 2;
  PKTheader.orig_net = fromsystem.Net;
  PKTheader.dest_net = tosystem.Net;
  PKTheader.product_low = 27;
  PKTheader.prod_rev_low = 2;
  memcpy (&PKTheader.password, password, 8);
  PKTheader.CapValid = 256;
  PKTheader.product_hi = 0;
  PKTheader.prod_rev_hi = 60;
  PKTheader.CapWord = 1;
  PKTheader.orig_zone = PKTheader.zone_ignore[0] = fromsystem.Zone;
  PKTheader.dest_zone = PKTheader.zone_ignore[1] = tosystem.Zone;
  PKTheader.orig_point = fromsystem.Point;
  PKTheader.dest_point = tosystem.Point;
  PKTheader.ProdData = 0;

#ifndef PORTABLEBINFILES

  fwrite (&PKTheader, sizeof (PKTheader), 1, newpkt->handle);

#else
  {
    unsigned char buf[58], *bufp = buf;

    put_short (bufp, PKTheader.orig_node);
    put_short (bufp, PKTheader.dest_node);
    put_short (bufp, PKTheader.year);
    put_short (bufp, PKTheader.month);
    put_short (bufp, PKTheader.day);
    put_short (bufp, PKTheader.hour);
    put_short (bufp, PKTheader.minute);
    put_short (bufp, PKTheader.second);
    put_short (bufp, PKTheader.rate);
    put_short (bufp, PKTheader.ver);
    put_short (bufp, PKTheader.orig_net);
    put_short (bufp, PKTheader.dest_net);
    *(bufp++) = PKTheader.product_low;
    *(bufp++) = PKTheader.prod_rev_low;
    memcpy (bufp, PKTheader.password, 8);
    bufp += 8;
    put_short (bufp, PKTheader.zone_ignore[0]);
    put_short (bufp, PKTheader.zone_ignore[1]);
    memcpy (bufp, PKTheader.B_fill1, 2);
    bufp += 2;
    put_short (bufp, PKTheader.CapValid);
    *(bufp++) = PKTheader.product_hi;
    *(bufp++) = PKTheader.prod_rev_hi;
    put_short (bufp, PKTheader.CapWord);
    put_short (bufp, PKTheader.orig_zone);
    put_short (bufp, PKTheader.dest_zone);
    put_short (bufp, PKTheader.orig_point);
    put_short (bufp, PKTheader.dest_point);
    put_long (bufp, PKTheader.ProdData);
    fwrite (&buf, sizeof (buf), 1, newpkt->handle);
  }
#endif

  Msgheader.Signature = 2;
  Msgheader.OrigNode = fromaddr.Node;
  Msgheader.OrigNet = fromaddr.Net;
  Msgheader.DestNode = toaddr.Node;
  Msgheader.DestNet = toaddr.Net;
  Msgheader.Attribs = attribs;
  Msgheader.Cost = 0;
  sprintf (Msgheader.DateTime, "%02d %s %02d  %02d:%02d:%02d",
           nowsp->tm_mday, mtext[nowsp->tm_mon], nowsp->tm_year % 100,
           nowsp->tm_hour, nowsp->tm_min, nowsp->tm_sec);

#ifndef PORTABLEBINFILES

  fwrite (&Msgheader, 1, sizeof (struct S_Packed), newpkt->handle);

#else
  {
    unsigned char buf[34], *bufp = buf;

    put_short (bufp, Msgheader.Signature);
    put_short (bufp, Msgheader.OrigNode);
    put_short (bufp, Msgheader.DestNode);
    put_short (bufp, Msgheader.OrigNet);
    put_short (bufp, Msgheader.DestNet);
    put_short (bufp, Msgheader.Attribs);
    put_short (bufp, Msgheader.Cost);
    memcpy (bufp, Msgheader.DateTime, 20);
    fwrite (&buf, 1, sizeof (buf), newpkt->handle);
  }
#endif

  fwrite (toname, 1, strlen (toname) + 1, newpkt->handle);
  fwrite (fromname, 1, strlen (fromname) + 1, newpkt->handle);
  fwrite (subject, 1, strlen (subject) + 1, newpkt->handle);

  /* Write body kludges and control lines */
  if (area != NULL && *area != '\0')
  {
    /* Echomail */
    newpkt->isecho = 1;
    /* AREA is not a kludge! It is a normal line AREA: echoname that
       goes *before* MSGID and the other kludges */
    fprintf (newpkt->handle, "AREA:%s\r", area);
    strcpy (newpkt->echotag, area);
  }
  else
  {
    /* Netmail */
    newpkt->isecho = 0;
    fprintf (newpkt->handle, "\1INTL %u:%u/%u %u:%u/%u\r",
             toaddr.Zone, toaddr.Net, toaddr.Node,
             fromaddr.Zone, fromaddr.Net, fromaddr.Node);
    if (toaddr.Point != 0)
      fprintf (newpkt->handle, "\1TOPT %u\r", toaddr.Point);
    if (fromaddr.Point != 0)
      fprintf (newpkt->handle, "\1FMPT %u\r", toaddr.Point);
    newpkt->echotag[0] = '\0';
  }

  /* Common */
  fprintf (newpkt->handle, "\1MSGID: %u:%u/%u.%u %08lx\r",
           fromaddr.Zone, fromaddr.Net, fromaddr.Node, fromaddr.Point,
           MSGID.number);
  fprintf (newpkt->handle, "\1FLAGS PGM\r");  /* Program-generated message */
  fprintf (newpkt->handle, "\1PID: Binkley-XE\r");
  return newpkt;
}

int
WriteToPKT (struct S_PKTh *h, char *fmt,...)
{
  va_list vap;
  char *expand;

  if (h->handle == NULL)
    return 0;

  expand = (char *) malloc (640);  /* 640 bytes should be enough for everyone :-) */

  if (expand == NULL)
    return 0;

  va_start (vap, fmt);

  if (vsprintf (expand, fmt, vap) > 640)
  {
    /* They weren't. Now we have a serious problem */
    return 0;
  }

  fwrite (expand, 1, strlen (expand), h->handle);
  free (expand);
  va_end (vap);
  return 1;
}

int
ClosePKT (struct S_PKTh *h)
{
  char PInfo[7];

  if (h->handle == NULL)
    return 0;
  free (h->pktname);

  if (h->isecho)
  {
    /* Add tear & origin lines */
    if (h->originaddress.Point == 0)
      PInfo[0] = 0;
    else
      sprintf (PInfo, ".%u", h->originaddress.Point);
    WriteToPKT (h, "\r--- \r");
    // WriteToPKT (h," * Origin: CFS's PKT report system - on test (%u:%u/%u%s)\r",
    WriteToPKT (h, " * Origin: %s (%u:%u/%u%s)\r", def_system_name,
                h->originaddress.Zone, h->originaddress.Net,
                h->originaddress.Node, PInfo);
    WriteToPKT (h, "SEEN-BY: %hu/%hu\r", h->targetaddress.Net, h->targetaddress.Node);
  }

  fprintf (h->handle, "%c%c%c", 0, 0, 0);
  fclose (h->handle);
  free (h);
  return 1;
}

/* $Id: pktmsgs.c,v 1.10 1999/08/23 00:27:09 ceh Exp $ */
