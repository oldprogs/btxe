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
 * Filename    : $Source: E:/cvs/btxe/src/history.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:27 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : call history screen
 *
 *---------------------------------------------------------------------------*/

/*
 * Sample mail call:
 * 01 97/04/12 23:26 Carlos Fernandez Sanz               2:341/70        Prot
 *             00:03 Freq CFR for CFRoute 0.94a!         I/O: 3(3.5M)/4(3.2M)
 *              EMSI Madrid, Spain                       Spd: 115200 Cst: 49484
 *
 * Sample fax call:
 * 01 97/04/12 23:26 Remote ID: 34-1-3780127             Speed: 9600
 *             00:12 Pages: 12                           FAX0097.FAX
 *               FAX Bytes: 1938374                      Finished
 *
 * Sample BBS call:
 * 01 97/04/12 23:26 Carlos Fernandez Sanz               Speed: 64000
 *             00:23 CFS                                 Times on: 1039
 *               BBS Madrid, Spain
 */

#include "includes.h"

#define LA_NONE    0
#define LA_ADDRESS 1
#define LA_SYSOP   2
#define LA_FAX     3
#define LA_BBS_RA  4
#define LA_BBS_MAX 5

static REGIONP histwin;
int HistBarPos = 0;
CALLHIST histrec;
long *validr = NULL;            /* List of qualifying records when using line or address selections */
long vrcount = 0, vrblock = 0;
ADDR histaddr;
int lockline = 0, lockaddress = 0;
char *lockstring;
long histrecords;


static char *sess_status[] =    /* VRP 990916 */
{
  "Completed",                  /* mission_ok      */
  "Error",                      /* mission_error   */
  "Aborted",                    /* mission_aborted */
  "Timeout"                     /* mission_timeout */
};


int
Do_RecordInfo (BINK_SAVEP rp, int x)
{
  REGIONP rwin;
  char line[120];
  struct tm *start;

  /* is there a window */
  if (rp == NULL)
    return (0);

  rwin = rp->region;

  /* draw a box */
  sb_fill (rwin, ' ', colors.hold);
  sb_box (rwin, boxtype, colors.frames);

  /* give it a title */
  sb_caption (rwin, "Detailed record info", colors.frames);

  sb_clear (rwin);
  show_alive ();
  sprintf (line, "Task number: %u", histrec.task);
  sb_move_puts (rwin, 2, 2, line);

  if (histrec.outgoing)
    sb_move_puts (rwin, 2, 20, "Outgoing");
  else
    sb_move_puts (rwin, 2, 20, "Incoming");

  start = localtime (&histrec.starttime);
  sprintf (line, "%02u/%02u/%02u %02u:%02u",
           start->tm_year % 100, start->tm_mon + 1, start->tm_mday,
           start->tm_hour, start->tm_min);
  sb_move_puts (rwin, 3, 2, "Date and time: ");
  sb_puts (rwin, line);
  sprintf (line, "      Length: %d:%02d", (int)
           ((histrec.length) / 60), (int) (histrec.length % 60));
  sb_puts (rwin, line);

  switch (histrec.calltype)
  {
  case CT_MAILER:
    sprintf (line, "Mailer call at %ld bps", histrec.h.m.speed);
    sb_move_puts (rwin, 5, 2, line);

    if (histrec.subclass == SC_FAILURE)
    {
      sb_move_puts (rwin, 6, 2, "Failed call");
      break;
    }

    sb_move_puts (rwin, 6, 2, "Session type: ");

    switch (histrec.subclass)
    {
    case SC_FTS1:
      sb_puts (rwin, "FTS-1");
      break;
    case SC_WAZOO:
      sb_puts (rwin, "Wazoo");
      break;
    case SC_EMSI:
      sb_puts (rwin, "EMSI");
      break;
    }

    sprintf (line, "     Address: %u:%u/%u.%u", histrec.h.m.address.Zone,
             histrec.h.m.address.Net, histrec.h.m.address.Node,
             histrec.h.m.address.Point);
    sb_move_puts (rwin, 7, 2, line);
    sb_move_puts (rwin, 8, 2, "       Sysop: ");
    sb_puts (rwin, histrec.h.m.name);
    sb_move_puts (rwin, 9, 2, "      System: ");
    sb_puts (rwin, histrec.h.m.system);
    sb_move_puts (rwin, 10, 2, "    Location: ");
    sb_puts (rwin, histrec.h.m.location);
    sb_move_puts (rwin, 11, 2, "       Phone: ");
    sb_puts (rwin, histrec.h.m.phone);
    sb_move_puts (rwin, 12, 2, "      Mailer: ");
    sb_puts (rwin, histrec.h.m.mailer);
    sprintf (line, "Files received: %ld", histrec.h.m.filesin);
    sb_move_puts (rwin, 14, 2, line);
    sprintf (line, "Bytes received: %s", numdisp (histrec.h.m.bytesin, 5));
    sb_move_puts (rwin, 14, 30, line);
    sprintf (line, "    Files sent: %ld", histrec.h.m.filesout);
    sb_move_puts (rwin, 15, 2, line);
    sprintf (line, "    Bytes sent: %s", numdisp (histrec.h.m.bytesout, 5));
    sb_move_puts (rwin, 15, 30, line);
    sprintf (line, "        CPS in: %-15ld    CPS out: %ld", histrec.h.m.cpsin, histrec.h.m.cpsout);
    sb_move_puts (rwin, 16, 2, line);
    sprintf (line, "          Cost: %ld", histrec.h.m.cost);
    sb_move_puts (rwin, 17, 2, line);

    /* VRP 990916 start */
    sprintf (line, "        Status: %s", sess_status[histrec.h.m.success]);
    sb_move_puts (rwin, 17, 30, line);
    /* VRP 990916 end */

    if (histrec.h.m.passworded)
      sb_move_puts (rwin, 18, 2, "Session was password protected");
    break;

  case CT_BBS:
    sb_move_puts (rwin, 5, 2, "BBS call");

    switch (histrec.subclass)
    {
    case SC_LASTCALL:
      sb_move_puts (rwin, 6, 2, "LASTCALL.BBS (RemoteAccess)");
      sb_move_puts (rwin, 8, 2, "  Name: ");
      sb_puts (rwin, histrec.h.b1.name);
      sb_move_puts (rwin, 9, 2, "Handle: ");
      sb_puts (rwin, histrec.h.b1.handle);
      sb_move_puts (rwin, 10, 2, "  City: ");
      sb_puts (rwin, histrec.h.b1.city);
      sprintf (line, " Speed: %hu        Times logged in: %lu",
               histrec.h.b1.baud, histrec.h.b1.times);
      sb_move_puts (rwin, 12, 2, line);
      break;

    case SC_IBS:
    case SC_CALLINFO:
      if (histrec.subclass == SC_IBS)
      {
        sb_move_puts (rwin, 6, 2, "Binkley internal BBS");
      }
      else
      {
        sb_move_puts (rwin, 6, 2, "Maximus");
      }                         /* endif */
      sb_move_puts (rwin, 8, 2, "  Name: ");
      sb_puts (rwin, histrec.h.b2.name);
      sb_move_puts (rwin, 9, 2, "Handle: ");
      sb_puts (rwin, histrec.h.b2.handle);
      sb_move_puts (rwin, 10, 2, "  City: ");
      sb_puts (rwin, histrec.h.b2.city);
      sprintf (line, " Speed: %hu        Times logged in: %lu",
               histrec.h.b2.baud, (long) histrec.h.b2.calls);
      sb_move_puts (rwin, 12, 2, line);
      sprintf (line, "Files downloaded: %hu", histrec.h.b2.filesdn);
      sb_move_puts (rwin, 14, 2, line);
      sprintf (line, "Bytes downloaded: %s", numdisp (histrec.h.b2.kbdn * 1024, 5));
      sb_move_puts (rwin, 14, 30, line);
      sprintf (line, "  Files uploaded: %hu", histrec.h.b2.filesup);
      sb_move_puts (rwin, 15, 2, line);
      sprintf (line, "  Bytes uploaded: %s", numdisp (histrec.h.b2.kbup * 1024, 5));
      sb_move_puts (rwin, 15, 30, line);
      sprintf (line, "Messages read: %hu      Messages posted: %hu",
               histrec.h.b2.read, histrec.h.b2.posted);
      sb_move_puts (rwin, 17, 2, line);
      break;

    case SC_TELEGARD:
      sb_move_puts (rwin, 6, 2, "Telegard");
      sb_move_puts (rwin, 8, 2, "  Name: ");
      sb_puts (rwin, histrec.h.b3.realname);
      sb_move_puts (rwin, 9, 2, "Handle: ");
      sb_puts (rwin, histrec.h.b3.handle);
      sb_move_puts (rwin, 10, 2, "  City: ");
      sb_puts (rwin, histrec.h.b3.location);
      sprintf (line, " Speed: %lu",
               histrec.h.b3.logonspeed);
      sb_move_puts (rwin, 12, 2, line);
      sprintf (line, "Files downloaded: %hu", histrec.h.b3.downloads);
      sb_move_puts (rwin, 14, 2, line);
      sprintf (line, "Bytes downloaded: %s", numdisp (histrec.h.b3.dk * 1024, 5));
      sb_move_puts (rwin, 14, 30, line);
      sprintf (line, "  Files uploaded: %hu", histrec.h.b3.uploads);
      sb_move_puts (rwin, 15, 2, line);
      sprintf (line, "  Bytes uploaded: %s", numdisp (histrec.h.b3.uk * 1024, 5));
      sb_move_puts (rwin, 15, 30, line);
      break;
    }
    break;

  case CT_FAX:
    sb_move_puts (rwin, 5, 2, "FAX call");
    sb_move_puts (rwin, 7, 2, "Remote ID: ");
    sb_puts (rwin, histrec.h.f.remoteid);
    sprintf (line, "Pages: %lu", histrec.h.f.pages);
    sb_move_puts (rwin, 8, 2, line);
    sprintf (line, "Bytes: %lu", histrec.h.f.bytes);
    sb_move_puts (rwin, 9, 2, line);
    sprintf (line, "Speed: %lu", histrec.h.f.speed);
    sb_move_puts (rwin, 10, 2, line);
    sprintf (line, "Filename: %s", histrec.h.f.filename);
    sb_move_puts (rwin, 12, 2, line);
    sprintf (line, "  Status: %s", histrec.h.f.status);
    sb_move_puts (rwin, 13, 2, line);
    break;

  case CT_EXTMAIL:             /* VRP 990916 */
    if (histrec.outgoing)
    {
      if (histrec.subclass == SC_FAILURE)
      {
        sb_move_puts (rwin, 6, 2, "Failed call");
        break;
      }

      sb_move_puts (rwin, 6, 2, "Session type: EXTERNAL");

      sprintf (line, "     Address: %u:%u/%u.%u", histrec.h.m.address.Zone,
               histrec.h.m.address.Net, histrec.h.m.address.Node,
               histrec.h.m.address.Point);
      sb_move_puts (rwin, 7, 2, line);
      sb_move_puts (rwin, 9, 2, "      System: ");
      sb_puts (rwin, histrec.h.m.system);
      sb_move_puts (rwin, 10, 2, "    Location: ");
      sb_puts (rwin, histrec.h.m.location);
      sb_move_puts (rwin, 12, 2, "      Mailer: ");
      sb_puts (rwin, histrec.h.m.mailer);
      sprintf (line, "Files received: %ld", histrec.h.m.filesin);
      sb_move_puts (rwin, 14, 2, line);
      sprintf (line, "Bytes received: %s", numdisp (histrec.h.m.bytesin, 5));
      sb_move_puts (rwin, 14, 30, line);
      sprintf (line, "    Files sent: %ld", histrec.h.m.filesout);
      sb_move_puts (rwin, 15, 2, line);
      sprintf (line, "    Bytes sent: %s", numdisp (histrec.h.m.bytesout, 5));
      sb_move_puts (rwin, 15, 30, line);
      sprintf (line, "        CPS in: %-15ld    CPS out: %ld", histrec.h.m.cpsin, histrec.h.m.cpsout);
      sb_move_puts (rwin, 16, 2, line);
      sprintf (line, "          Cost: %ld", histrec.h.m.cost);
      sb_move_puts (rwin, 17, 2, line);
      sprintf (line, "        Status: %s", sess_status[histrec.h.m.success]);
      sb_move_puts (rwin, 17, 30, line);

      if (histrec.h.m.passworded)
        sb_move_puts (rwin, 18, 2, "Session was password protected");
    }
    else
    {
      sb_move_puts (rwin, 5, 2, "External mail call");
    }
    break;

  default:
    sb_move_puts (rwin, 5, 2, "Unknown call type");
    break;
  }

  sb_show ();
  FOSSIL_CHAR ();
  return (x);
}

void
ClearArea (void)
{
  char j[2];
  int i;
  BOXTYPE *boxp;

  histwin->sr0++;
  histwin->sr1--;
  histwin->sc0++;
  histwin->sc1--;
  sb_fillc (histwin, ' ');
  histwin->sr0--;
  histwin->sr1++;
  histwin->sc0--;
  histwin->sc1++;
  boxp = &box[boxtype];
  j[1] = 0;
  j[0] = boxp->t270;
  sb_move_puts_abs (histwin, histwin->sr1 - 4, 0, j);
  j[1] = 0;
  j[0] = boxp->t90;
  sb_move_puts_abs (histwin, histwin->sr1 - 4, histwin->sc1, j);
  sb_move_abs (histwin, histwin->sr1 - 4, 1);

  for (i = 0; i < histwin->sc1 - 1; i++)
    sb_putc_att (histwin, boxp->bbar, colors.frames);

  sb_move_puts (histwin, histwin->sr1 - 3, 2, "`brightgreen`TAB `white`toggle long/short mode. `brightgreen`Arrows `white`move. `brightgreen`Enter `white`get detailed info.");
  sb_move_puts (histwin, histwin->sr1 - 2, 2, "`brightgreen`L `white`toggle all/current line. `brightgreen`Space `white`this caller only/all `brightgreen`A`white` this address only/all");
}

void
BuildRecordList (FILE * in)
{
  long l;

  fseek (in, 0, SEEK_END);
  histrecords = ftell (in) / sizeof (CALLHIST);
  free (validr);
  vrcount = 0;
  vrblock = 0;
  validr = NULL;

  if (!lockline && !lockaddress)
    return;

  ClearArea ();
  sb_move_puts (histwin, 10, 26, "Generating record list, please wait");
  sb_show ();
  fseek (in, 0, SEEK_SET);

  for (l = 0; l < histrecords; l++)
  {
    if (fread (&histrec, 1, sizeof (CALLHIST), in) == 0)
      return;

    if (lockline && histrec.task != TaskNumber)
      continue;

    if (lockaddress == LA_ADDRESS && memcmp (&histrec.h.m.address, &histaddr, 8))
      continue;

    if (lockaddress == LA_SYSOP && stricmp (&histrec.h.m.name[0], lockstring))
      continue;

    if (lockaddress == LA_FAX && stricmp (&histrec.h.f.remoteid[0], lockstring))
      continue;

    if (lockaddress == LA_BBS_RA && stricmp (&histrec.h.b1.name[0], lockstring))
      continue;

    if (lockaddress == LA_BBS_MAX && stricmp (&histrec.h.b2.name[0], lockstring))
      continue;

    if (vrcount == vrblock)     /* We need a block resize */
    {
      vrblock += 32;
      validr = (long *) realloc (validr, sizeof (long) * vrblock);

      if (validr == NULL)       /* A DOS user trying to do "advanced" things... */
      {
        vrblock = 0;
        vrcount = 0;
        lockline = 0;
        lockaddress = 0;
        return;
      }
    }

    validr[vrcount++] = l;
  }

  histrecords = vrcount;
  if (!vrcount)
  {
    lockline = 0;
    lockaddress = 0;
    fseek (in, 0, SEEK_END);
    histrecords = ftell (in) / sizeof (CALLHIST);
    sb_move_puts (histwin, 10, 26, "  No records to show. Press a key  ");
    sb_show ();
    FOSSIL_CHAR ();
  }
}

int
SaveCallInfo (CALLHIST data)
{
  FILE *out;
  long t_wait;

  t_wait = timerset (10 * PER_SECOND);

  do
  {
    out = share_fopen (HistoryFileName, append_binary, DENY_WRITE);
    if (out == NULL)            /* Don't panic */
      time_release ();
  }
  while (out == NULL && !timeup (t_wait));

  if (out == NULL)
  {
    status_line ("!Warning: Failed to update history file with this call.");
    return 0;
  }

  data.starttime -= (time_t) TIMEZONE;
  fwrite (&data, 1, sizeof (CALLHIST), out);
  data.starttime += (time_t) TIMEZONE;
  fclose (out);
  // status_line ("!Updated %s",HistoryFileName);
  return 1;
}

void
HistUpdRecord (CALLHIST rec, int pos, int current)
{
  char line[160], col[50], datel[80], saddr[40];
  struct tm *start;

  start = localtime (&rec.starttime);
  sprintf (datel, "%02u %02u/%02u/%02u %02u:%02u",
           rec.task, start->tm_year % 100, start->tm_mon + 1, start->tm_mday,
           start->tm_hour, start->tm_min);

  switch (rec.calltype)
  {
  case CT_MAILER:
    if (rec.outgoing == 0)
      strcpy (col, "`yellow`");
    else
      strcpy (col, "`brightred`");

    rec.h.m.name[30] = 0;
    sprintf (saddr, "%u:%u/%u", rec.h.m.address.Zone,
             rec.h.m.address.Net, rec.h.m.address.Node);

    if (rec.h.m.address.Point)
      sprintf (saddr + strlen (saddr), ".%u", rec.h.m.address.Point);

    sprintf (line, "%s%s %-30s %-18s %s %s", col, datel,
             rec.h.m.name, saddr, rec.h.m.passworded ? "Prot" : "    ",
             rec.h.m.success ? "Err" : "  ");  /* VRP 990916 */
    sb_move_puts (histwin, pos + 1, 2, line);

    if (HistMode == HIST_SHORT)
      break;

    rec.h.m.system[30] = 0;
    sprintf (line, "%s        %6d:%02d %-30s I/O: %ld(%s)/%ld(", col,
             (int) ((rec.length) / 60), (int) (rec.length % 60),
             rec.h.m.system, rec.h.m.filesin, numdisp (rec.h.m.bytesin, 5),
             rec.h.m.filesout);
    strcat (line, numdisp (rec.h.m.bytesout, 5));
    strcat (line, ")");
    sb_move_puts (histwin, pos + 2, 2, line);
    rec.h.m.location[30] = 0;
    sprintf (line, "%s            ", col);

    switch (rec.subclass)
    {
    case SC_EMSI:
      strcat (line, " EMSI ");
      break;
    case SC_FTS1:
      strcat (line, " FTS1 ");
      break;
    case SC_WAZOO:
      strcat (line, "WaZOO ");
      break;
    default:
      strcat (line, "????? ");
      break;
    }

    sprintf (line + strlen (line), "%-30s Spd: %6lu Cst: %lu",
             rec.h.m.location, rec.h.m.speed, rec.h.m.cost);
    sb_move_puts (histwin, pos + 3, 2, line);
    break;


  case CT_FAX:
    rec.h.f.remoteid[21] = 0;
    sprintf (line, "`cyan`%s Remote ID:%-21sSpeed: %lu", datel,
             rec.h.f.remoteid, rec.h.f.speed);
    sb_move_puts (histwin, pos + 1, 2, line);

    if (HistMode == HIST_SHORT)
      break;

    sprintf (line, "`cyan`        %6d:%02d Pages: %-23ld Filename: %s",
             (int) ((rec.length) / 60), (int) (rec.length % 60),
             rec.h.f.pages, rec.h.f.filename);
    sb_move_puts (histwin, pos + 2, 2, line);
    sprintf (line, "`cyan`              FAX Bytes: %-24lu%s",
             rec.h.f.bytes, rec.h.f.status);
    sb_move_puts (histwin, pos + 3, 2, line);
    break;


  case CT_BBS:
    switch (rec.subclass)
    {
    case SC_LASTCALL:
      rec.h.b1.name[30] = 0;
      sprintf (line, "`brightgreen`%s %-30s Speed: %u",
               datel, rec.h.b1.name, rec.h.b1.baud);
      sb_move_puts (histwin, pos + 1, 2, line);

      if (HistMode == HIST_SHORT)
        break;

      rec.h.b1.handle[30] = 0;
      sprintf (line, "`brightgreen`        %6d:%02d %-30s Times: %ld",
               (int) ((rec.length) / 60), (int) (rec.length % 60),
               rec.h.b1.handle, rec.h.b1.times);
      sb_move_puts (histwin, pos + 2, 2, line);
      sprintf (line, "`brightgreen`              BBS %s",
               rec.h.b1.city);
      sb_move_puts (histwin, pos + 3, 2, line);
      break;

    case SC_IBS:
    case SC_CALLINFO:          /* AW970531 Maximus BBS output */
      strcpy (col, "`brightgreen`");
      rec.h.b2.name[30] = 0;
      sprintf (line, "%s%s %-30s Speed: %5u  Calls: %5u", col, datel,
               rec.h.b2.name, rec.h.b2.baud, rec.h.b2.calls);
      sb_move_puts (histwin, pos + 1, 2, line);

      if (HistMode == HIST_SHORT)
        break;

      rec.h.b2.handle[30] = 0;
      sprintf (line, "%s        %6d:%02d %-30s I/O: %d(%s)/%d(", col,
               (int) ((rec.length) / 60), (int) (rec.length % 60),
               rec.h.b2.handle, rec.h.b2.filesup,
               numdisp (rec.h.b2.kbup * 1024, 5),
               rec.h.b2.filesdn);
      strcat (line, numdisp (rec.h.b2.kbdn * 1024, 5));
      strcat (line, ")");
      sb_move_puts (histwin, pos + 2, 2, line);
      rec.h.b2.city[25] = 0;
      sprintf (line, "%s             BBS  %-30s Msgs posted: %6d", col,
               rec.h.b2.city, rec.h.b2.posted);
      sb_move_puts (histwin, pos + 3, 2, line);
      break;

    case SC_TELEGARD:
      strcpy (col, "`brightgreen`");
      rec.h.b3.realname[30] = 0;
      sprintf (line, "`brightgreen`%s %-30s Speed: %-8lu   ",
               datel, rec.h.b3.realname, rec.h.b3.logonspeed);

      if (rec.h.b3.newuser)
        strcat (line, "(New)");

      sb_move_puts (histwin, pos + 1, 2, line);

      if (HistMode == HIST_SHORT)
        break;

      rec.h.b3.handle[30] = 0;
      sprintf (line, "%s        %6d:%02d %-30s I/O: %d(%s)/%d(", col,
               (int) ((rec.length) / 60), (int) (rec.length % 60),
               rec.h.b3.handle, rec.h.b3.uploads,
               numdisp (rec.h.b3.uk * 1024, 5),
               rec.h.b3.downloads);
      strcat (line, numdisp (rec.h.b3.dk * 1024, 5));
      strcat (line, ")");
      sb_move_puts (histwin, pos + 2, 2, line);
      sprintf (line, "`brightgreen`              BBS %s",
               rec.h.b3.location);
      sb_move_puts (histwin, pos + 3, 2, line);
      break;
    }
    break;


  case CT_EXTMAIL:             /* VRP 990916 */
    if (rec.outgoing == 0)
      strcpy (col, "`yellow`");
    else
      strcpy (col, "`brightred`");

    if (rec.outgoing)
    {
      rec.h.m.name[30] = 0;
      sprintf (saddr, "%u:%u/%u", rec.h.m.address.Zone,
               rec.h.m.address.Net, rec.h.m.address.Node);

      if (rec.h.m.address.Point)
        sprintf (saddr + strlen (saddr), ".%u", rec.h.m.address.Point);

      sprintf (line, "%s%s %-30s %-18s %s %s", col, datel,
             "External Mailer", saddr, rec.h.m.passworded ? "Prot" : "    ",
               rec.h.m.success ? "Err" : "  ");
      sb_move_puts (histwin, pos + 1, 2, line);

      if (HistMode == HIST_SHORT)
        break;

      rec.h.m.system[30] = 0;
      sprintf (line, "%s        %6d:%02d %-30s I/O: %ld(%s)/%ld(", col,
               (int) ((rec.length) / 60), (int) (rec.length % 60),
               rec.h.m.system, rec.h.m.filesin, numdisp (rec.h.m.bytesin, 5),
               rec.h.m.filesout);
      strcat (line, numdisp (rec.h.m.bytesout, 5));
      strcat (line, ")");
      sb_move_puts (histwin, pos + 2, 2, line);
      rec.h.m.location[30] = 0;
      sprintf (line, "%s         EXTERNAL ", col);

      sprintf (line + strlen (line), "%-30s Spd: %6lu Cst: %lu",
               rec.h.m.location, rec.h.m.speed, rec.h.m.cost);
      sb_move_puts (histwin, pos + 3, 2, line);

      break;
    }


  default:
    sprintf (line, "%s    Unsupported call type %d - feel free to add your own support code :-)", col, rec.calltype);

    if (HistMode == HIST_SHORT)
    {
      sb_move_puts (histwin, pos + 1, 2, line);
      break;
    }

    sb_move_puts (histwin, pos + 2, 2, line);
    sprintf (line, "%s---------------------------------------------------------------------------", col);
    sb_move_puts (histwin, pos + 1, 2, line);
    sb_move_puts (histwin, pos + 3, 2, line);
    break;
  }

  if (current)
  {
    sb_move_puts (histwin, pos + 1, 1, ">");
    sb_move_puts (histwin, pos + 1, SB_COLS - 2, "<");

    if (HistMode == HIST_LONG)
    {
      sb_move_puts (histwin, pos + 2, 1, ">");
      sb_move_puts (histwin, pos + 2, SB_COLS - 2, "<");
      sb_move_puts (histwin, pos + 3, 1, ">");
      sb_move_puts (histwin, pos + 3, SB_COLS - 2, "<");
    }
  }
}

void
HistUpdScreen (FILE * in, long first, long many)  /*  TS970519    */
{
  long c;
  CALLHIST rec;

  ClearArea ();
  /* If we are browsing the whole file, we use direct file access */
  if (!lockaddress && !lockline)
    fseek (in, first * sizeof (CALLHIST), SEEK_SET);
  for (c = 0; c < many; c++)
  {
    if (!lockaddress && !lockline)
    {
      if (fread (&rec, 1, sizeof (CALLHIST), in) == 0)
        break;
    }
    else
    {
      if (first + c >= vrcount)
        break;
      fseek (in, (validr[first + c]) * sizeof (CALLHIST), SEEK_SET);
      if (fread (&rec, 1, sizeof (CALLHIST), in) == 0)
        break;
    }

    rec.starttime += (time_t) TIMEZONE;  /* 980208 in history file are local timestamps */
    HistUpdRecord (rec, c * HistMode, c == HistBarPos);
  }
}

int
Do_CallHistory (BINK_SAVEP rp, int x)
{
  long t_idle;
  unsigned int c, done = 0;
  long first, capacity;
  FILE *in;

  // char info[80];

  /* is there a window */
  if (rp == NULL)
    return (0);

  histwin = rp->region;

  /* draw a box */
  sb_fill (histwin, ' ', colors.hold);
  sb_box (histwin, boxtype, colors.frames);

  /* give it a title */
  sb_caption (histwin, "Call history screen", colors.frames);

  ClearArea ();
  show_alive ();

  HistBarPos = 0;
  lockline = 0;
  lockaddress = 0;
  vrcount = 0;
  vrblock = 0;
  validr = NULL;
  lockstring = NULL;
  t_idle = timerset (PER_MINUTE);
  in = share_fopen (HistoryFileName, read_binary, DENY_NONE);

  if (in == NULL)
  {
    sb_move_puts (histwin, 10, 26, "Unable to open history file");
    sb_show ();
    timer (50);
    return x;
  }

  fseek (in, 0, SEEK_END);
  histrecords = ftell (in) / sizeof (CALLHIST);

  if (!histrecords)
  {
    sb_move_puts (histwin, 10, 26, "No records in history file");
    sb_show ();
    fclose (in);
    timer (50);
    return x;
  }

  if (HistMode == HIST_LONG)
    capacity = rp->save_ht / 3 - 1;
  else
    capacity = rp->save_ht / 3 * 3 - 3;

  if (histrecords < capacity)
    first = 0;
  else
    first = histrecords - capacity;

  HistBarPos = capacity - 1;

  if (first + HistBarPos >= histrecords)
    HistBarPos = histrecords - first - 1;

  HistUpdScreen (in, first, capacity);

  while (!timeup (t_idle) && (PEEKBYTE () == (short) 0xffff) && !done)
  {
    /* display_life(); */
    sb_show ();
    if (KEYPRESS ())
    {
      t_idle = timerset (30 * PER_SECOND);
      c = (int) KbRemap (FOSSIL_CHAR ());
      switch ((unsigned short) (c))  /* TS970519: correct conversion supplied */
      {
      case F_PEND_UPAR:        /* Up arrow */
        if (HistBarPos > 0)
          HistBarPos--;
        else if (first > 0)
          first--;
        HistUpdScreen (in, first, capacity);
        sb_show ();
        break;

      case F_PEND_DNAR:        /* Down arrow */
        if (HistBarPos < capacity - 1 && first + HistBarPos < histrecords - 1)
          HistBarPos++;
        else if ((first + capacity) < histrecords)
          first++;
        HistUpdScreen (in, first, capacity);
        sb_show ();
        break;

      case F_PEND_PGUP:        /* Page Up */
        if (HistBarPos > 0)
          HistBarPos = 0;
        else
        {
          first -= capacity;
          if (first < 0)
            first = 0;
        }
        HistUpdScreen (in, first, capacity);
        sb_show ();
        break;

      case F_PEND_PGDN:        /* Page Down */
        if (HistBarPos < capacity - 1)
        {
          HistBarPos = capacity - 1;
          if (first + HistBarPos >= histrecords)
            HistBarPos = histrecords - first - 1;
        }
        else
        {
          first += capacity;
          if (first + capacity >= histrecords)
            first = histrecords - capacity;
        }
        HistUpdScreen (in, first, capacity);
        sb_show ();
        break;

      case F_PEND_HOME:        /* Home */
        first = 0;
        HistBarPos = 0;
        HistUpdScreen (in, first, capacity);
        sb_show ();
        break;

      case F_PEND_END:         /* End */
        first = histrecords - capacity;
        if (first < 0)
          first = 0;
        HistBarPos = capacity - 1;
        if (first + HistBarPos >= histrecords)
          HistBarPos = histrecords - first - 1;
        HistUpdScreen (in, first, capacity);
        sb_show ();
        break;

      case TAB:                /* Tab */
        if (HistMode == HIST_LONG)
          HistMode = HIST_SHORT;
        else
          HistMode = HIST_LONG;
        if (HistMode == HIST_LONG)
          capacity = rp->save_ht / 3 - 1;
        else
          capacity = rp->save_ht / 3 * 3 - 3;
        first = first + HistBarPos;
        HistBarPos = 0;
        HistUpdScreen (in, first, capacity);
        break;

      case 13:                 /* Enter */
        if (!lockaddress && !lockline)
          fseek (in, (first + HistBarPos) * sizeof (CALLHIST), SEEK_SET);
        else
          fseek (in, validr[first + HistBarPos] * sizeof (CALLHIST), SEEK_SET);
        fread (&histrec, 1, sizeof (CALLHIST), in);
        histrec.starttime += (time_t) TIMEZONE;
        sb_popup (3, 10, 20, 60, Do_RecordInfo, 0);
        break;

      case 'L':
      case 'l':
        lockline = !lockline;
        BuildRecordList (in);
        if (histrecords < capacity)
          first = 0;
        else
          first = histrecords - capacity;
        if (first + HistBarPos >= histrecords)
          HistBarPos = histrecords - first - 1;
        HistUpdScreen (in, first, capacity);
        break;

      case ' ':
      case 'A':
      case 'a':
      case 65125U:             /* Strange value returned by KbRemap! */
        if (lockaddress != LA_NONE)
        {
          lockaddress = LA_NONE;
          if (lockstring)
          {
            free (lockstring);
            lockstring = NULL;
          }
        }
        else
        {
          if (!lockaddress && !lockline)
            fseek (in, (first + HistBarPos) * sizeof (CALLHIST), SEEK_SET);
          else
            fseek (in, validr[first + HistBarPos] * sizeof (CALLHIST), SEEK_SET);

          fread (&histrec, 1, sizeof (CALLHIST), in);
          histrec.starttime += (time_t) TIMEZONE;

          if (histrec.calltype == CT_MAILER && (c == 'A' || c == 'a' || c == 65125U))
          {
            lockaddress = LA_ADDRESS;
            memcpy (&histaddr, &histrec.h.m.address, sizeof (ADDR));
          }
          else if (histrec.calltype == CT_MAILER)
          {
            lockaddress = LA_SYSOP;
            lockstring = ctl_string (histrec.h.m.name);
          }
          else if (histrec.calltype == CT_FAX)
          {
            lockaddress = LA_FAX;
            lockstring = ctl_string (histrec.h.f.remoteid);
          }
          else if (histrec.calltype == CT_BBS && histrec.subclass == SC_LASTCALL)
          {
            lockaddress = LA_BBS_RA;
            lockstring = ctl_string (histrec.h.b1.name);
          }
          else if (histrec.calltype == CT_BBS && ((histrec.subclass == SC_CALLINFO) || (histrec.subclass == SC_IBS)))
          {
            lockaddress = LA_BBS_MAX;
            lockstring = ctl_string (histrec.h.b2.name);
          }
        }

        BuildRecordList (in);
        if (histrecords < capacity)
          first = 0;
        else
          first = histrecords - capacity;

        if (first + HistBarPos >= histrecords)
          HistBarPos = histrecords - first - 1;

        HistUpdScreen (in, first, capacity);
        break;

      case 27:
        done = 1;
        break;
      }
    }

    time_release ();
  }

  fclose (in);
  free (validr);
  free (lockstring);
  return (x);
}

/* $Id: history.c,v 1.4 1999/09/27 20:51:27 mr Exp $ */
