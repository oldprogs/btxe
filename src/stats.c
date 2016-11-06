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
 * Filename    : $Source: E:/cvs/btxe/src/stats.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/05/15 23:53:38 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : Bink stats screen
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

static REGIONP stwin;

#define BSIZE 32
#define MAILERSIZE 50

typedef struct
{
  ADDR address;
  unsigned long callsin, callsout;
  unsigned long cost;
  unsigned long bytesin, bytesout;
  unsigned long filesin, filesout;
  unsigned long cpsall;
  unsigned long timeconnected;
  unsigned long speed;
  byte protected;
  char mailer[MAILERSIZE];
}
NODESTAT;

typedef struct
{
  char remoteid[40];
  long faxes;
  long pages;
  long bytes;
}
FAXSTAT;

typedef struct
{
  char name[36];
  unsigned long calls;
}
BBSSTAT;

typedef struct
{
  char mailer[MAILERSIZE];
  unsigned long used;
}
MAILERSTAT;

typedef struct
{
  unsigned long bps, times;
}
SPEEDSTAT;

typedef struct
{
  NODESTAT *ndata;
  FAXSTAT *faxdata;
  BBSSTAT *bbsdata;
  MAILERSTAT *mfulldata, *mworddata;
  SPEEDSTAT *speeddata;
  unsigned long ndatacount, ndatacapacity;
  unsigned long faxcount, faxcapacity;
  unsigned long bbscount, bbscapacity;
  unsigned long mfullcount, mfullcapacity;
  unsigned long mwordcount, mwordcapacity;
  unsigned long speedcount, speedcapacity;
  unsigned long statprot, statunprot;
  int topwhat, stfirst;
  unsigned long mailercalls, bbscalls, faxcalls, othercalls;
  unsigned long ftscalls, wazoocalls, emsicalls;
  unsigned int statsdays;
  unsigned int statshours[24][3];  /* One for each hour, in case you haven't guessed it :-) */
  unsigned long statsbusy[24];
  unsigned long historyrecords, historylength, linerecords, validrecords;
  unsigned long cbmypoints, cbmynets, cbmyregions, cbmyzones, cbother;
  byte alllines;
}
STATSTRUC;

#define TZ_OTHERZONE 0
#define TZ_MYZONE    1
#define TZ_MYREGION  2
#define TZ_MYNET     3
#define TZ_MYPOINTS  4


static void LOCALFUNC memorypanic (FILE *, STATSTRUC *);

static STATSTRUC *LOCALFUNC
initstatsmemory (FILE * in)
{
  STATSTRUC *pstats = NULL;

  pstats = calloc (1, sizeof (STATSTRUC));
  if (!pstats)
  {
    memorypanic (NULL, NULL);
  }
  else
  {
    fseek (in, 0, SEEK_END);
    pstats->historylength = ftell (in);
    pstats->historyrecords = pstats->historylength / sizeof (CALLHIST);
    pstats->alllines = 1;
    if (!pstats->historyrecords)
    {
      sb_move_puts (stwin, 10, 26, "No records in history file");
      sb_show ();
      fclose (in);
      timer (50);
    }
  }
  return pstats;
}

static void LOCALFUNC
freestatsmemory (STATSTRUC * pstats)
{
  if (pstats)
  {
    if (pstats->ndata)
      free (pstats->ndata);
    if (pstats->faxdata)
      free (pstats->faxdata);
    if (pstats->bbsdata)
      free (pstats->bbsdata);
    if (pstats->mfulldata)
      free (pstats->mfulldata);
    if (pstats->mworddata)
      free (pstats->mworddata);
    if (pstats->speeddata)
      free (pstats->speeddata);
    free (pstats);
  }
}



static void LOCALFUNC
memorypanic (FILE * in, STATSTRUC * pstats)
{
  if (in)
    fclose (in);
  freestatsmemory (pstats);
  sb_move_puts (stwin, 10, 3, " Not enough memory for stats - we'll try to return to the mailer w/o crashing");
  sb_show ();
  timer (50);
  return;
}

static void LOCALFUNC
cleanmailer (char *s)
{
  char *c;

  c = s;
  while (*c == ' ')             /* I don't think there is any idiot starting the id with spaces, but... */
    strcpy (c, c + 1);
  while (*c && *c != ' ')       /* Maybe / should end the spec too? (as BT-XE/2?) */
    c++;
  *c = 0;
}

static char *LOCALFUNC
getaddress (ADDR * d)
{
  static char result[40];
  char point[7];

  if (d->Point)
    sprintf (point, ".%u", d->Point);
  else
    point[0] = 0;
  sprintf (result, "%u:%u/%u%s", d->Zone, d->Net, d->Node, point);
  return result;
}

static int
SortAddress (long result, const void *p1, const void *p2)
{
  if (result)
    return result < 0 ? -1 : 1;
  return (memcmp (&((NODESTAT *) p1)->address, &((NODESTAT *) p2)->address, sizeof (ADDR)));
}

static int
SortFaxID (long result, const void *p1, const void *p2)
{
  if (result)
    return result < 0 ? -1 : 1;
  return (strcmp (((FAXSTAT *) p1)->remoteid, ((FAXSTAT *) p2)->remoteid));
}

static int
SortSpeedTimes (long result, const void *p1, const void *p2)
{
  if (result)
    return result < 0 ? -1 : 1;
  return (((SPEEDSTAT *) p2)->bps - ((SPEEDSTAT *) p1)->bps);
}

static int
SortUserName (long result, const void *p1, const void *p2)
{
  if (result)
    return result < 0 ? -1 : 1;
  return (strcmp (((BBSSTAT *) p1)->name, ((BBSSTAT *) p2)->name));
}

static int
SortMailerName (long result, const void *p1, const void *p2)
{
  if (result)
    return result < 0 ? -1 : 1;
  return (strcmp (((MAILERSTAT *) p1)->mailer, ((MAILERSTAT *) p2)->mailer));
}

static int
SortCallers (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->callsin -
                               ((NODESTAT *) p1)->callsin), p1, p2));
}

static int
SortCallersPlusCalled (const void *p1, const void *p2)
{
  return (SortAddress ((long) (
                (((NODESTAT *) p2)->callsin + ((NODESTAT *) p2)->callsout) -
                 (((NODESTAT *) p1)->callsin + ((NODESTAT *) p1)->callsout))
                       ,p1, p2));
}

static int
SortCalled (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->callsout -
                               ((NODESTAT *) p1)->callsout), p1, p2));
}

static int
SortCost (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->cost - ((NODESTAT *) p1)->cost), p1, p2));
}

static int
SortRecvFiles (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->filesout - ((NODESTAT *) p1)->filesout), p1, p2));
}

static int
SortSentFiles (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->filesin - ((NODESTAT *) p1)->filesin), p1, p2));
}

static int
SortRecvBytes (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->bytesout - ((NODESTAT *) p1)->bytesout), p1, p2));
}

static int
SortSentBytes (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->bytesin - ((NODESTAT *) p1)->bytesin), p1, p2));
}

static int
SortRecvFilesSession (const void *p1, const void *p2)
{
  unsigned long x, y;
  long z;

  x = ((NODESTAT *) p2)->callsin + ((NODESTAT *) p2)->callsout;
  y = ((NODESTAT *) p1)->callsin + ((NODESTAT *) p1)->callsout;
  z = (long) (((NODESTAT *) p2)->filesout / x -
              ((NODESTAT *) p1)->filesout / y);
  if (!z)
    return SortRecvFiles (p1, p2);

  return z < 0 ? -1 : 1;
}

static int
SortSentFilesSession (const void *p1, const void *p2)
{
  unsigned long x, y;
  long z;

  y = ((NODESTAT *) p1)->callsin + ((NODESTAT *) p1)->callsout;
  x = ((NODESTAT *) p2)->callsin + ((NODESTAT *) p2)->callsout;
  z = (long) (((NODESTAT *) p2)->filesin / x -
              ((NODESTAT *) p1)->filesin / y);
  if (!z)
    return SortSentFiles (p1, p2);

  return z < 0 ? -1 : 1;
}

static int
SortRecvBytesSession (const void *p1, const void *p2)
{
  unsigned long x, y;
  long z;

  x = ((NODESTAT *) p2)->callsin + ((NODESTAT *) p2)->callsout;
  y = ((NODESTAT *) p1)->callsin + ((NODESTAT *) p1)->callsout;
  z = (long) (((NODESTAT *) p2)->bytesout / x -
              ((NODESTAT *) p1)->bytesout / y);
  if (!z)
    return SortRecvBytes (p1, p2);

  return z < 0 ? -1 : 1;
}

static int
SortSentBytesSession (const void *p1, const void *p2)
{
  unsigned long x, y;
  long z;

  x = ((NODESTAT *) p2)->callsin + ((NODESTAT *) p2)->callsout;
  y = ((NODESTAT *) p1)->callsin + ((NODESTAT *) p1)->callsout;
  z = (long) (((NODESTAT *) p2)->bytesin / x -
              ((NODESTAT *) p1)->bytesin / y);
  if (!z)
    return SortSentBytes (p1, p2);

  return z < 0 ? -1 : 1;
}

static int
SortCPS (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->cpsall - ((NODESTAT *) p1)->cpsall), p1, p2));
}

static int
SortTimeOnline (const void *p1, const void *p2)
{
  return (SortAddress ((long) (((NODESTAT *) p2)->timeconnected - ((NODESTAT *) p1)->timeconnected), p1, p2));
}

static int
SortFaxesNumber (const void *p1, const void *p2)
{
  return (SortFaxID ((long) (((FAXSTAT *) p2)->faxes - ((FAXSTAT *) p1)->faxes), p1, p2));
}

static int
SortFaxesPages (const void *p1, const void *p2)
{
  return (SortFaxID ((long) (((FAXSTAT *) p2)->pages - ((FAXSTAT *) p1)->pages), p1, p2));
}

static int
SortFaxesBytes (const void *p1, const void *p2)
{
  return (SortFaxID ((long) (((FAXSTAT *) p2)->bytes - ((FAXSTAT *) p1)->bytes), p1, p2));
}

static int
SortSpeed (const void *p1, const void *p2)
{
  return (SortSpeedTimes ((long) (((SPEEDSTAT *) p2)->times - ((SPEEDSTAT *) p1)->times),
                          p1, p2));
}

static int
SortUserCalls (const void *p1, const void *p2)
{
  return (SortUserName ((long) (((BBSSTAT *) p2)->calls - ((BBSSTAT *) p1)->calls), p1, p2));
}

static int
SortMailer (const void *p1, const void *p2)
{
  return (SortMailerName ((long) (((MAILERSTAT *) p2)->used - ((MAILERSTAT *) p1)->used), p1, p2));
}

static char *LOCALFUNC
getbar (char filler, int maxlength, unsigned long many, unsigned long top)
{
  int x;
  static char bar[133];

  if (maxlength > 132)
    return NULL;

  memset (bar, 0, maxlength + 1);
  if (top == 0)
    top = 1;

  x = (int) ((maxlength * many) / top);
  memset (bar, filler, x);
  return bar;
}

static char *LOCALFUNC
HorizontalLine (int len, int mid, char c, char midchar)
{
  static char dummy[180];
  int i;

  for (i = 0; i < len; i++)
    if (i == mid)
      dummy[i] = midchar;
    else
      dummy[i] = c;
  dummy[i] = 0;
  return (dummy);
}

static void LOCALFUNC
StatsScreen8 (STATSTRUC * pstats)
{
  int c, c2, c3, rows;
  char line[180], *x;
  long totalcalls = 0, l;
  unsigned maxcalls = 0;
  BOXTYPE *boxp = &box[boxtype];

  for (c = 0; c < 24; c++)
    for (c2 = 0; c2 < 3; c2++)
      if ((pstats->statshours[c][c2] / pstats->statsdays) > maxcalls)
        maxcalls = (pstats->statshours[c][c2] / pstats->statsdays);

  sprintf (line, "`yellow`%c%s%cPage 8/8%c%c", boxp->ul, HorizontalLine (66, -1, boxp->tbar, 0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);
  sprintf (line, "%c                    `brightgreen`Topological distribution of calls    `yellow`                   %c", boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 2, 1, line);

  for (c = 1; c <= 6; c++)
  {
    sprintf (line, "%c                                                                            %c", boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, 2 + c, 1, line);
  }

  totalcalls = pstats->cbmypoints + pstats->cbmynets + pstats->cbmyregions + pstats->cbmyzones + pstats->cbother;
  if (totalcalls)
  {
    l = (pstats->cbmypoints * 100) / totalcalls;
    sprintf (line, " `brightred`From points of this system...: %-36s %3d%%", getbar (boxp->tbar, 36, pstats->cbmypoints, totalcalls), (int) l);
    sb_move_puts (stwin, 4, 2, line);
    l = (pstats->cbmynets * 100) / totalcalls;
    sprintf (line, " `brightcyan`From addresses of our nets...: %-36s %3d%%", getbar (boxp->tbar, 36, pstats->cbmynets, totalcalls), (int) l);
    sb_move_puts (stwin, 5, 2, line);
    l = (pstats->cbmyregions * 100) / totalcalls;
    sprintf (line, " `brightred`From addresses of our regions: %-36s %3d%%", getbar (boxp->tbar, 36, pstats->cbmyregions, totalcalls), (int) l);
    sb_move_puts (stwin, 6, 2, line);
    l = (pstats->cbmyzones * 100) / totalcalls;
    sprintf (line, " `brightcyan`From addresses of our zones..: %-36s %3d%%", getbar (boxp->tbar, 36, pstats->cbmyzones, totalcalls), (int) l);
    sb_move_puts (stwin, 7, 2, line);
    l = (pstats->cbother * 100) / totalcalls;
    sprintf (line, " `brightred`From addresses of other zones: %-36s %3d%%", getbar (boxp->tbar, 36, pstats->cbother, totalcalls), (int) l);
    sb_move_puts (stwin, 8, 2, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, -1, boxp->tbar, 0), boxp->t90);
  sb_move_puts (stwin, 9, 1, line);

  sprintf (line, "%c`brightgreen`         Average number of minutes busy per hour (this line only)           `yellow`%c", boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 10, 1, line);

  for (c = 11; c <= pstats->topwhat * 2 + 4; c++)
  {
    sprintf (line, "%c                                                                            %c", boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, -1, boxp->tbar, 0), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);
  rows = pstats->topwhat * 2 - 7;
  c3 = -1;

  for (c = rows - 1; c >= 0; c--)
  {
    c2 = (60 * (rows - c)) / rows;
    sprintf (line, "%3d", c2);
    if (c3 != c2)
      sb_move_puts (stwin, c + 12, 2, line);
    c3 = c2;
  }

  sb_move_puts (stwin, pstats->topwhat * 2 + 4, 3, "`yellow`   00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 ");

  for (c = 0; c < 24; c++)
  {
    x = getbar (boxp->lbar, rows, pstats->statsbusy[c] / pstats->statsdays, 3600);
    for (c3 = 0; (c3 < rows) && (x[rows - c3 - 1]); c3++)
    {
      sb_move (stwin, c3 + 11, c * 3 + 6);
      sb_putc_att (stwin, x[rows - c3 - 1], COL_WHITE);
      sb_putc_att (stwin, x[rows - c3 - 1], COL_WHITE);
    }
  }

  sb_show ();
}

static void LOCALFUNC
StatsScreen7 (STATSTRUC * pstats)
{
  int c;
  char line[180];
  BOXTYPE *boxp = &box[boxtype];

  sprintf (line, "`yellow`%c%s%cPage 7/8%c%c", boxp->ul, HorizontalLine (66, 37, boxp->tbar, boxp->t0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);
  sprintf (line, "%c    `brightgreen`Top receivers (files/session)`yellow`    %c`brightgreen`     Top receivers (bytes/session) `yellow`   %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 2, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, 2 + c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, 37, boxp->tbar, boxp->plus), boxp->t90);
  sb_move_puts (stwin, pstats->topwhat + 3, 1, line);
  sprintf (line, "%c   `brightgreen` Top senders (files/session)`yellow`      %c  `brightgreen`   Top senders (bytes/session)`yellow`      %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 4, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, pstats->topwhat + c + 4, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, 37, boxp->bbar, boxp->t180), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortRecvFilesSession);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].filesout)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1, getaddress (&pstats->ndata[c].address), boxp->lbar,
                 pstats->ndata[c].filesout / (pstats->ndata[c].callsin + pstats->ndata[c].callsout));
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortRecvBytesSession);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].bytesout)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7s", c + 1, getaddress (&pstats->ndata[c].address), boxp->rbar,
                 numdisp (pstats->ndata[c].bytesout / (pstats->ndata[c].callsin + pstats->ndata[c].callsout), 5));
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 40, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortSentFilesSession);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].filesin)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1, getaddress (&pstats->ndata[c].address), boxp->lbar,
                 pstats->ndata[c].filesin / (pstats->ndata[c].callsin + pstats->ndata[c].callsout));
        sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortSentBytesSession);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].bytesin)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7s", c + 1, getaddress (&pstats->ndata[c].address), boxp->rbar,
                 numdisp (pstats->ndata[c].bytesin / (pstats->ndata[c].callsin + pstats->ndata[c].callsout), 5));
        sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 40, line);
      }
    }
  }

  sb_show ();
}

static void LOCALFUNC
StatsScreen6 (STATSTRUC * pstats)
{
  int c;
  unsigned long l, sc;
  char line[180];
  BOXTYPE *boxp = &box[boxtype];

  sprintf (line, "`yellow`%c%s%cPage 6/8%c%c", boxp->ul, HorizontalLine (66, -1, boxp->tbar, 0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);

  for (c = 0; c <= pstats->topwhat + 4; c++)
  {
    sprintf (line, "`yellow`%c                                                                            %c", boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, c + 2, 1, line);
  }

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                                     %c                                      %c", boxp->lbar, boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, pstats->topwhat + c + 4, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, 37, boxp->bbar, boxp->t180), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);
  sb_move_puts (stwin, 2, 3, "Some extra numbers you may want to know...");
  sprintf (line, "`yellow`Your history file is `white`%lu `yellow`bytes long, and contains `white`%lu `yellow`entries,",
           pstats->historylength, pstats->historyrecords);
  sb_move_puts (stwin, 3, 3, line);
  sprintf (line, "`white`%lu`yellow` of them for calls from/to this line.", pstats->linerecords);

  if (!pstats->alllines)
    strcat (line, " Numbers for `white`this line only:`yellow`");

  sb_move_puts (stwin, 4, 3, line);
  sprintf (line, "`yellow`You have connected (by calling or by being called) `white`%lu `yellow`different systems,",
           pstats->ndatacount);
  sb_move_puts (stwin, 5, 3, line);
  sprintf (line, "`yellow`have received faxes from `white`%lu`yellow` different numbers, and have been visited",
           pstats->faxcount);
  sb_move_puts (stwin, 6, 3, line);
  sprintf (line, "`yellow`by `white`%lu `yellow`different BBS users.", pstats->bbscount);
  sb_move_puts (stwin, 7, 3, line);
  sprintf (line, "`yellow`Your FTN callers use `white`%lu `yellow`mailers with different IDs and `white`%lu `yellow`mailers",
           pstats->mfullcount, pstats->mwordcount);
  sb_move_puts (stwin, 8, 3, line);
  sb_move_puts (stwin, 9, 3, "whose first word is different.");
  sprintf (line, "`yellow`These stats are based on the `white`%u`yellow` days covered by your history file.",
           pstats->statsdays);
  sb_move_puts (stwin, 10, 3, line);
  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, 37, boxp->tbar, boxp->t0), boxp->t90);
  sb_move_puts (stwin, pstats->topwhat + 3, 1, line);
  sprintf (line, "`yellow`%c          `brightgreen`Most common speeds`yellow`         %c      `brightgreen`Protected/Unprotected ratio`yellow`", boxp->lbar, boxp->lbar);
  sb_move_puts (stwin, pstats->topwhat + 4, 1, line);

  qsort (pstats->speeddata, pstats->speedcount, sizeof (SPEEDSTAT), SortSpeed);
  sc = 0;
  for (c = 0; c < pstats->speedcount; c++)
    sc += pstats->speeddata[c].times;
  if (sc)
  {
    for (c = pstats->stfirst; c < pstats->topwhat - 2 + pstats->stfirst; c++)
    {
      if ((int) pstats->speedcount > c)
      {
        l = (pstats->speeddata[c].times * 100) / sc;
        sprintf (line, "`yellow`%2d. %7lu `cyan`%-20s`brightgreen`%3lu%% `yellow`%c", c + 1, pstats->speeddata[c].bps,
                 getbar (boxp->tbar, 20, pstats->speeddata[c].times, sc), l, boxp->lbar);
        sb_move_puts (stwin, pstats->topwhat + 6 + c - pstats->stfirst, 2, line);
      }
    }
  }

  if (pstats->ndatacount)
  {
    sprintf (line, "`yellow`  Protected `brightgreen`%-20s %2lu%%",
             getbar (boxp->tbar, 20, pstats->statprot, pstats->ndatacount), (pstats->statprot * 100L) / pstats->ndatacount);
    sb_move_puts (stwin, pstats->topwhat + 6, 41, line);
    sprintf (line, "`yellow`Unprotected `brightred`%-20s %2lu%%",
             getbar (boxp->tbar, 20, pstats->statunprot, pstats->ndatacount), (pstats->statunprot * 100L) / pstats->ndatacount);
    sb_move_puts (stwin, pstats->topwhat + 7, 41, line);
  }

  sprintf (line, "%c%s%c", boxp->t270, HorizontalLine (38, -1, boxp->tbar, 0), boxp->t90);
  sb_move_puts (stwin, pstats->topwhat + 9, 39, line);
  sprintf (line, "%cCommercial%c", boxp->t90, boxp->t270);
  sb_move_puts (stwin, pstats->topwhat + 9, 39 + 15, line);
  sprintf (line, "%c Can't find the netmail router of your%c", boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 10, 39, line);
  sprintf (line, "%c dreams? Try  CFRoute - The best  Bink%c", boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 11, 39, line);
  sprintf (line, "%c style netmail router available!      %c", boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 12, 39, line);
  sb_show ();
}

static void LOCALFUNC
StatsScreen5 (STATSTRUC * pstats)
{
  int c, c2, c3, rows;
  short color = COL_WHITE;      /* Inited to avoid compiler warnings */
  char line[180], *x;
  unsigned long l, totalcalls;
  unsigned maxcalls = 0;
  BOXTYPE *boxp = &box[boxtype];

  totalcalls = pstats->bbscalls + pstats->mailercalls + pstats->faxcalls + pstats->othercalls;
  for (c = 0; c < 24; c++)
    for (c2 = 0; c2 < 3; c2++)
      if ((pstats->statshours[c][c2] / pstats->statsdays) > maxcalls)
        maxcalls = (pstats->statshours[c][c2] / pstats->statsdays);

  sprintf (line, "`yellow`%c%s%cPage 5/8%c%c", boxp->ul, HorizontalLine (66, 37, boxp->tbar, boxp->t0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);
  sprintf (line, "%c   `brightgreen`Distribution of calls by type  `yellow`   %c `brightgreen`Distribution of calls by session type`yellow`%c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 2, 1, line);

  for (c = 1; c <= 6; c++)
  {
    sprintf (line, "%c                                     %c                                      %c", boxp->lbar, boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, 2 + c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, 37, boxp->tbar, boxp->t180), boxp->t90);
  sb_move_puts (stwin, 9, 1, line);
  sprintf (line, "%c`brightgreen`                       Average number of calls per hour                     `yellow`%c", boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 10, 1, line);

  for (c = 11; c <= pstats->topwhat * 2 + 4; c++)
  {
    sprintf (line, "%c                                                                            %c", boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, -1, boxp->tbar, 0), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);
  rows = pstats->topwhat * 2 - 7;
  c3 = -1;

  for (c = rows - 1; c >= 0; c--)
  {
    c2 = (maxcalls * (rows - c)) / rows;
    sprintf (line, "%3d", c2);
    if (c3 != c2)
      sb_move_puts (stwin, c + 12, 2, line);
    c3 = c2;
  }

  sb_move_puts (stwin, pstats->topwhat * 2 + 4, 3, "`yellow`   00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 ");
  if (totalcalls)
  {
    l = pstats->mailercalls;
    l = (l * 100) / totalcalls;
    sprintf (line, "`yellow`Mailer: %-23s %3d%%", getbar (boxp->tbar, 23, pstats->mailercalls, totalcalls), (int) l);
    sb_move_puts (stwin, 4, 2, line);
    l = pstats->bbscalls;
    l = (l * 100) / totalcalls;
    sprintf (line, "`brightred`   BBS: %-23s %3d%%", getbar (boxp->tbar, 23, pstats->bbscalls, totalcalls), (int) l);
    sb_move_puts (stwin, 5, 2, line);
    l = pstats->faxcalls;
    l = (l * 100) / totalcalls;
    sprintf (line, "`brightcyan`   Fax: %-23s %3d%%", getbar (boxp->tbar, 23, pstats->faxcalls, totalcalls), (int) l);
    sb_move_puts (stwin, 6, 2, line);
    l = pstats->othercalls;
    l = (l * 100) / totalcalls;
    sprintf (line, "`green` Other: %-23s %3d%%", getbar (boxp->tbar, 23, pstats->othercalls, totalcalls), (int) l);
    sb_move_puts (stwin, 7, 2, line);
  }

  totalcalls = pstats->ftscalls + pstats->wazoocalls + pstats->emsicalls;
  if (totalcalls)
  {
    l = pstats->ftscalls;
    l = (l * 100) / totalcalls;
    sprintf (line, "`yellow`FTS-1: %-23s %3d%%", getbar (boxp->tbar, 23, pstats->ftscalls, totalcalls), (int) l);
    sb_move_puts (stwin, 4, 41, line);
    l = pstats->wazoocalls;
    l = (l * 100) / totalcalls;
    sprintf (line, "`brightred`Wazoo: %-23s %3d%%", getbar (boxp->tbar, 23, pstats->wazoocalls, totalcalls), (int) l);
    sb_move_puts (stwin, 5, 41, line);
    l = pstats->emsicalls;
    l = (l * 100) / totalcalls;
    sprintf (line, "`brightcyan` EMSI: %-23s %3d%%", getbar (boxp->tbar, 23, pstats->emsicalls, totalcalls), (int) l);
    sb_move_puts (stwin, 6, 41, line);
  }

  for (c = 0; c < 24; c++)
    for (c2 = 0; c2 < 3; c2++)
    {
      switch (c2)
      {
      case 0:                  /* MAILER CALLS */
        color = COL_YELLOW;
        break;

      case 1:                  /* BBS CALLS */
        color = COL_BRIGHTRED;
        break;

      case 2:                  /* FAX CALLS */
        color = COL_BRIGHTCYAN;
        break;
      }

      x = getbar (boxp->lbar, rows, pstats->statshours[c][c2] / pstats->statsdays, maxcalls);
      for (c3 = 0; (c3 < rows) && (x[rows - c3 - 1]); c3++)
      {
        sb_move (stwin, c3 + 11, c * 3 + c2 + 6);
        sb_putc_att (stwin, x[rows - c3 - 1], color);
      }
    }

  sb_show ();
}

static void LOCALFUNC
StatsScreen4 (STATSTRUC * pstats)
{
  int c;
  char line[180];
  BOXTYPE *boxp = &box[boxtype];

  sprintf (line, "`yellow`%c%s%cPage 4/8%c%c", boxp->ul, HorizontalLine (66, 37, boxp->tbar, boxp->t0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);
  sprintf (line, "%c      `brightgreen`Top fax senders (pages)   `yellow`     %c       `brightgreen`Top fax senders (bytes)`yellow`        %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 2, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, 2 + c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, 37, boxp->tbar, boxp->plus), boxp->t90);
  sb_move_puts (stwin, pstats->topwhat + 3, 1, line);
  sprintf (line, "%c      `brightgreen`  Top mailers (full ID)`yellow`        %c   `brightgreen`  Top mailers (first word only)`yellow`    %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 4, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, pstats->topwhat + c + 4, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, 37, boxp->bbar, boxp->t180), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);

  qsort (pstats->faxdata, (size_t) pstats->faxcount, sizeof (FAXSTAT), SortFaxesPages);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->faxcount > c)
    {
      if (pstats->faxdata[c].pages)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1, pstats->faxdata[c].remoteid, boxp->lbar,
                 pstats->faxdata[c].pages);
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->faxdata, (size_t) pstats->faxcount, sizeof (FAXSTAT), SortFaxesBytes);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->faxcount > c)
    {
      if (pstats->faxdata[c].bytes)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7s", c + 1, pstats->faxdata[c].remoteid, boxp->rbar,
                 numdisp (pstats->faxdata[c].bytes, 5));
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 40, line);
      }
    }
  }

  qsort (pstats->mfulldata, (size_t) pstats->mfullcount, sizeof (MAILERSTAT), SortMailer);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->mfullcount > c)
    {
      pstats->mfulldata[c].mailer[23] = 0;
      sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1,
        pstats->mfulldata[c].mailer, boxp->lbar, pstats->mfulldata[c].used);
      sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 2, line);
    }
  }

  qsort (pstats->mworddata, (size_t) pstats->mwordcount, sizeof (MAILERSTAT), SortMailer);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->mwordcount > c)
    {
      pstats->mworddata[c].mailer[24] = 0;
      sprintf (line, "`yellow`%4d. %-24s%c%7lu", c + 1,
        pstats->mworddata[c].mailer, boxp->rbar, pstats->mworddata[c].used);
      sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 40, line);
    }
  }

  sb_show ();
}


static void LOCALFUNC
StatsScreen3 (STATSTRUC * pstats)
{
  int c, seconds, minutes, hours, days;
  unsigned long w;
  char line[180];
  BOXTYPE *boxp = &box[boxtype];

  sprintf (line, "`yellow`%c%s%cPage 3/8%c%c", boxp->ul, HorizontalLine (66, 37, boxp->tbar, boxp->t0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);
  sprintf (line, "%c    `brightgreen`  Top bandwidth users (CPS)`yellow`      %c     `brightgreen`    Top most time online`yellow`         %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 2, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                           %c          %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, 2 + c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, 37, boxp->bbar, boxp->plus), boxp->t90);
  sb_move_puts (stwin, pstats->topwhat + 3, 1, line);
  sprintf (line, "%c          `brightgreen`  Top BBS callers     `yellow`     %c     `brightgreen`  Top fax senders (faxes)`yellow`        %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 4, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, pstats->topwhat + c + 4, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, 37, boxp->bbar, boxp->t180), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortCPS);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].cpsall)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1, getaddress (&pstats->ndata[c].address), boxp->lbar,
                 pstats->ndata[c].cpsall);
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortTimeOnline);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].timeconnected)
      {
        w = pstats->ndata[c].timeconnected;
        seconds = (int) (w % 60);
        w = (w - seconds) / 60;
        minutes = (int) (w % 60);
        w = (w - minutes) / 60;
        hours = (int) (w % 24);
        days = (int) ((w - hours) / 24);
        sprintf (line, "`yellow`%4d. %-21s%c%3dd %02d:%02d", c + 1, getaddress (&pstats->ndata[c].address), boxp->rbar,
                 days, hours, minutes);
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 40, line);
      }
    }
  }

  qsort (pstats->bbsdata, (size_t) pstats->bbscount, sizeof (BBSSTAT), SortUserCalls);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->bbscount > c)
    {
      pstats->bbsdata[c].name[23] = 0;
      sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1,
             pstats->bbsdata[c].name, boxp->lbar, pstats->bbsdata[c].calls);
      sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 2, line);
    }
  }

  qsort (pstats->faxdata, (size_t) pstats->faxcount, sizeof (FAXSTAT), SortFaxesNumber);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->faxcount > c)
    {
      if (pstats->faxdata[c].faxes)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7lu", c + 1, pstats->faxdata[c].remoteid, boxp->rbar,
                 pstats->faxdata[c].faxes);
        sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 40, line);
      }
    }
  }

  sb_show ();
}

static void LOCALFUNC
StatsScreen2 (STATSTRUC * pstats)
{
  int c;
  char line[180];
  BOXTYPE *boxp = &box[boxtype];

  sprintf (line, "`yellow`%c%s%cPage 2/8%c%c", boxp->ul, HorizontalLine (66, 37, boxp->tbar, boxp->t0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);
  sprintf (line, "%c          `brightgreen`Top receivers (files)   `yellow`   %c       `brightgreen` Top receivers (bytes)`yellow`         %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 2, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, 2 + c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, 37, boxp->tbar, boxp->plus), boxp->t90);
  sb_move_puts (stwin, pstats->topwhat + 3, 1, line);
  sprintf (line, "%c          `brightgreen`Top senders (files)   `yellow`     %c         `brightgreen`Top senders (bytes)`yellow`          %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 4, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar);
    sb_move_puts (stwin, pstats->topwhat + c + 4, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, 37, boxp->bbar, boxp->t180), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortRecvFiles);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].filesout)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1, getaddress (&pstats->ndata[c].address), boxp->lbar,
                 pstats->ndata[c].filesout);
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortRecvBytes);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].bytesout)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7s", c + 1, getaddress (&pstats->ndata[c].address), boxp->rbar,
                 numdisp (pstats->ndata[c].bytesout, 5));
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 40, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortSentFiles);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].callsin + pstats->ndata[c].filesin)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1,
                 getaddress (&pstats->ndata[c].address), boxp->lbar, pstats->ndata[c].filesin);
        sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortSentBytes);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].bytesin)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7s", c + 1, getaddress (&pstats->ndata[c].address), boxp->rbar,
                 numdisp (pstats->ndata[c].bytesin, 5));
        sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 40, line);
      }
    }
  }

  sb_show ();
}

static void LOCALFUNC
StatsScreen1 (STATSTRUC * pstats)
{
  int c;
  char line[180];
  BOXTYPE *boxp = &box[boxtype];

  sprintf (line, "`yellow`%c%s%cPage 1/8%c%c", boxp->ul, HorizontalLine (66, 37, boxp->tbar, boxp->t0), boxp->t90, boxp->t270, boxp->ur);
  sb_move_puts (stwin, 1, 1, line);
  sprintf (line, "`yellow`%c             `brightgreen`Top callers`yellow`             %c               `brightgreen`Top called`yellow`             %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, 2, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, 2 + c, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->t270, HorizontalLine (76, 37, boxp->tbar, boxp->plus), boxp->t90);
  sb_move_puts (stwin, pstats->topwhat + 3, 1, line);
  sprintf (line, "`yellow`%c          `brightgreen`Top callers+called `yellow`        %c         `brightgreen`Top most expensive`yellow`           %c", boxp->lbar, boxp->lbar, boxp->rbar);
  sb_move_puts (stwin, pstats->topwhat + 4, 1, line);

  for (c = 1; c <= pstats->topwhat; c++)
  {
    sprintf (line, "`yellow`%c                             %c       %c                              %c       %c", boxp->lbar, boxp->lbar, boxp->lbar, boxp->rbar, boxp->rbar);
    sb_move_puts (stwin, pstats->topwhat + c + 4, 1, line);
  }

  sprintf (line, "`yellow`%c%s%c", boxp->ll, HorizontalLine (76, 37, boxp->bbar, boxp->t180), boxp->lr);
  sb_move_puts (stwin, pstats->topwhat * 2 + 5, 1, line);

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortCallers);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].callsin)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1, getaddress (&pstats->ndata[c].address), boxp->lbar, pstats->ndata[c].callsin);
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortCalled);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].callsout)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7lu", c + 1, getaddress (&pstats->ndata[c].address), boxp->rbar,
                 pstats->ndata[c].callsout);
        sb_move_puts (stwin, 3 + c - pstats->stfirst, 40, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortCallersPlusCalled);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].callsin + pstats->ndata[c].callsout)
      {
        sprintf (line, "`yellow`%4d. %-23s%c%7lu", c + 1,
                 getaddress (&pstats->ndata[c].address), boxp->lbar, pstats->ndata[c].callsin + pstats->ndata[c].callsout);
        sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 2, line);
      }
    }
  }

  qsort (pstats->ndata, (size_t) pstats->ndatacount, sizeof (NODESTAT), SortCost);
  for (c = pstats->stfirst; c < pstats->topwhat + pstats->stfirst; c++)
  {
    if ((int) pstats->ndatacount > c)
    {
      if (pstats->ndata[c].cost)
      {
        sprintf (line, "`yellow`%4d. %-24s%c%7lu", c + 1, getaddress (&pstats->ndata[c].address), boxp->rbar,
                 pstats->ndata[c].cost);
        sb_move_puts (stwin, pstats->topwhat + 5 + c - pstats->stfirst, 40, line);
      }
    }
  }

  sb_show ();
}

int
Do_Stats (BINK_SAVEP rp, int x)
{
  long t_idle, secremain;
  unsigned long count, count2;
  time_t timelimit, timerecord = 0;
  int c = 0, done = 0, OnScreen = 1, bestmatch;
  time_t t1, t2;
  FILE *in = NULL;
  CALLHIST cur;
  struct tm *start, *tl;
  char line[81], user[60];
  AKA *pnode;
  STATSTRUC *pstats;

  /* is there a window */
  if (rp == NULL)
    return (0);

  stwin = rp->region;

  /* draw a box */
  // sb_fillc (stwin, ' ');
  sb_fill (stwin, ' ', colors.hold);
  sb_box (stwin, boxtype, colors.frames);

  /* give it a title */
  sb_caption (stwin, "Stats screen - press 1-8 or left/right arrows to select page", colors.frames);

  show_alive ();

  in = share_fopen (HistoryFileName, read_binary, DENY_NONE);
  if (in == NULL)
  {
    sb_move_puts (stwin, 10, 26, "Unable to open history file");
    sb_show ();
    timer (50);
    return x;
  }

  if ((pstats = initstatsmemory (in)) == NULL)
    return x;

  if (!pstats->historyrecords)
  {
    sb_move_puts (stwin, 10, 26, "No records in history file");
    sb_show ();
    fclose (in);
    freestatsmemory (pstats);
    timer (50);
    return x;
  }

  sb_move_puts (stwin, 8, 24, "(A)ll lines or (T)his line only?");
  t_idle = timerset (PER_SECOND * 10);
  sb_show ();

  while (!timeup (t_idle))
  {
    display_life ();
    if (KEYPRESS ())
    {
#ifdef DOS16                    /*  TS 970604   */
      c = FOSSIL_CHAR ();
#else
      c = READKB ();            /* AG 990311 emx+gcc doesn't like toupper of 16bit values */
#endif
      if ((c & 0xff) != 0)
      {
        c &= 0xff;
        c = toupper (c);
      }
      if (c == 27)              /* ESC */
      {
        fclose (in);
        freestatsmemory (pstats);
        return x;
      }
      else if (c == 'T')
        pstats->alllines = 0;
      break;
    }
  }

  sb_move_puts (stwin, 10, 24, "1. Today            2. Last day (24 hours)");
  sb_move_puts (stwin, 11, 24, "3. This week        4. Last week");
  sb_move_puts (stwin, 12, 24, "5. This month       6. Last month");
  sb_move_puts (stwin, 13, 24, "7. This year        8. Last year");
  sb_move_puts (stwin, 14, 24, "0. All records");
  sb_show ();

  t_idle = timerset (PER_SECOND * 10);
  while (!timeup (t_idle))
  {
    display_life ();
    if (KEYPRESS ())
    {
#ifdef DOS16                    /*  TS 970604   */
      c = FOSSIL_CHAR ();
#else
      c = READKB ();            /* AG 990311 emx+gcc doesn't like toupper of 16bit values */
#endif
      if ((c & 0xff) != 0)
      {
        c &= 0xff;
        c = toupper (c);
      }
      break;
    }
  }

  timelimit = unix_time (NULL);
  tl = localtime (&timelimit);

  switch (c)
  {
  case '1':                    /* Today */
    tl->tm_hour = 0;
    tl->tm_min = 0;
    tl->tm_sec = 0;
    timelimit = unix_mktime (tl);
    break;

  case '2':                    /* Last 24 hours */
    timelimit -= 3600L * 24L;
    break;

  case '3':                    /* This week */
    tl->tm_hour = 0;
    tl->tm_min = 0;
    tl->tm_sec = 0;
    timelimit = unix_mktime (tl);
    timelimit -= ((tl->tm_wday + 6) % 7) * 3600L * 24L;
    break;

  case '4':                    /* Last week */
    timelimit -= 3600L * 24L * 7L;
    break;

  case '5':                    /* This month */
    tl->tm_hour = 0;
    tl->tm_min = 0;
    tl->tm_sec = 0;
    tl->tm_mday = 1;
    timelimit = unix_mktime (tl);
    break;

  case '6':                    /* Last month */
    tl->tm_hour = 0;
    tl->tm_min = 0;
    tl->tm_sec = 0;
    if (tl->tm_mon)
      tl->tm_mon--;
    else
    {
      tl->tm_mon = 11;
      tl->tm_year--;
    }
    timelimit = unix_mktime (tl);
    break;

  case '7':                    /* This year */
    tl->tm_hour = 0;
    tl->tm_min = 0;
    tl->tm_sec = 0;
    tl->tm_mday = 1;
    tl->tm_mon = 0;
    timelimit = unix_mktime (tl);
    break;

  case '8':                    /* Last year */
    tl->tm_year--;
    timelimit = unix_mktime (tl);
    break;

  case 27:                     /*ESC */
    fclose (in);
    freestatsmemory (pstats);
    return x;
    /* break; *//* HJK 98/07/21 - Unreachable code */

  default:
    timelimit = 0;
    break;
  }

  sb_clear (stwin);
  sb_move_puts (stwin, 10, 23, "   Gathering data, please wait  ");
  sb_show ();
  fseek (in, 0, SEEK_SET);

  for (count = 0; count < pstats->historyrecords; count++)
  {
    fread (&cur, 1, sizeof (CALLHIST), in);
    cur.starttime += TIMEZONE;  /* AW 980208 local timestamps in historyfile */
    start = localtime (&cur.starttime);

    if (!count)
      timerecord = cur.starttime;

    if (cur.starttime > timelimit)
      if (timerecord < timelimit)
        timerecord = timelimit;

    if (!count)
    {
      if (cur.starttime > timelimit)
      {
        time (&t1);
        t1 -= (t1 % (24L * 3600L));
        t2 = cur.starttime;
        t2 -= (t2 % (24L * 3600L));
        /*  So t1 & t2 counts seconds to start of the day */
        t1 = ((t1 - t2) / (3600L * 24L)) + 1;
        pstats->statsdays = (unsigned int) t1;
      }
    }

    if (timelimit > cur.starttime)  /* Check time limits */
      continue;

    if ((long) cur.length < 0)  /* Who has seen a session lasting some years? */
      continue;

    /* Prepare data for minutes-busy graph */
    if (cur.task == TaskNumber)
    {
      secremain = cur.length;
      t2 = start->tm_hour;
      /* Number of seconds remaining on starting hour */
      t1 = 3600L - start->tm_min * 60L - start->tm_sec;
      while (secremain)
      {
        if (t1 > secremain)
        {
          pstats->statsbusy[t2] += secremain;
          secremain = 0;
        }
        else
        {
          pstats->statsbusy[t2] += t1;
          secremain -= t1;
        }

        t2 = (t2 + 1) % 24;
        t1 = 3600;
      }
    }

    if (cur.task == TaskNumber)
      pstats->linerecords++;
    else if (!pstats->alllines)
      continue;

    pstats->validrecords++;

    switch (cur.calltype)
    {
    case CT_MAILER:
      pstats->statshours[start->tm_hour][0]++;
      pstats->mailercalls++;
      switch (cur.subclass)
      {
      case SC_FTS1:
        pstats->ftscalls++;
        break;

      case SC_WAZOO:
        pstats->wazoocalls++;
        break;

      case SC_EMSI:
        pstats->emsicalls++;
        break;
      }

      /* Process node data */
      if (cur.h.m.address.Zone == 0 && cur.h.m.address.Net == 0 &&
          cur.h.m.address.Node == 0 && cur.h.m.address.Point == 0)
        break;

      /* -------------------------------------------- */
      /* Determine topological location of the system */
      /* -------------------------------------------- */

      if (islocalregion (cur.h.m.address.Zone, cur.h.m.address.Net))
        bestmatch = TZ_MYREGION;
      else
        bestmatch = TZ_OTHERZONE;

      for (pnode = alias; pnode != NULL; pnode = pnode->next)
      {
        if (pnode->Zone == cur.h.m.address.Zone &&
            pnode->Net == cur.h.m.address.Net &&
            pnode->Node == cur.h.m.address.Node)
        {
          bestmatch = TZ_MYPOINTS;
          break;                /* Best possible match; no need to compare more */
        }

        if (pnode->Zone == cur.h.m.address.Zone &&
            pnode->Net == cur.h.m.address.Net)
          bestmatch = (TZ_MYNET > bestmatch) ? TZ_MYNET : bestmatch;
        else if (pnode->Zone == cur.h.m.address.Zone)
          bestmatch = (TZ_MYZONE > bestmatch) ? TZ_MYZONE : bestmatch;
      }

      switch (bestmatch)
      {
      case TZ_MYPOINTS:
        pstats->cbmypoints++;
        break;

      case TZ_MYNET:
        pstats->cbmynets++;
        break;

      case TZ_MYREGION:
        pstats->cbmyregions++;
        break;

      case TZ_MYZONE:
        pstats->cbmyzones++;
        break;

      case TZ_OTHERZONE:
        pstats->cbother++;
        break;
      }

      for (count2 = 0; count2 < pstats->ndatacount; count2++)
      {
        /* Note that we don't compare the whole address structure
           but only the 8 bytes containing the 4D address */
        if (!memcmp (&cur.h.m.address, &pstats->ndata[count2].address, 8))
        {
          if (cur.outgoing)
            pstats->ndata[count2].callsout++;
          else
            pstats->ndata[count2].callsin++;
          pstats->ndata[count2].bytesin += cur.h.m.bytesin;
          pstats->ndata[count2].bytesout += cur.h.m.bytesout;
          pstats->ndata[count2].filesin += cur.h.m.filesin;
          pstats->ndata[count2].filesout += cur.h.m.filesout;
          pstats->ndata[count2].cost += cur.h.m.cost;
          pstats->ndata[count2].timeconnected += cur.length;

          if (pstats->ndata[count2].timeconnected != 0)
            pstats->ndata[count2].cpsall = (pstats->ndata[count2].bytesin +
                                            pstats->ndata[count2].bytesout) /
              pstats->ndata[count2].timeconnected;
          else
            pstats->ndata[count2].cpsall = (pstats->ndata[count2].bytesin +
                                            pstats->ndata[count2].bytesout);

          // avoid div by zero !
          pstats->ndata[count2].speed = cur.h.m.speed;
          pstats->ndata[count2].protected = cur.h.m.passworded;
          strntcpy (pstats->ndata[count2].mailer, cur.h.m.mailer, MAILERSIZE);
          break;
        }
      }

      if (count2 == pstats->ndatacount)  /* No match then */
      {
        if (pstats->ndatacount >= pstats->ndatacapacity)
        {
          pstats->ndatacapacity += BSIZE;
          pstats->ndata = (NODESTAT *) realloc (pstats->ndata, (size_t) (pstats->ndatacapacity * sizeof (NODESTAT)));
          if (!pstats->ndata)
          {
            memorypanic (in, pstats);
            return (x);
          }
        }

        memcpy (&pstats->ndata[pstats->ndatacount].address, &cur.h.m.address, sizeof (ADDR));

        if (cur.outgoing)
        {
          pstats->ndata[pstats->ndatacount].callsout = 1;
          pstats->ndata[pstats->ndatacount].callsin = 0;
        }
        else
        {
          pstats->ndata[pstats->ndatacount].callsout = 0;
          pstats->ndata[pstats->ndatacount].callsin = 1;
        }

        pstats->ndata[pstats->ndatacount].bytesin = cur.h.m.bytesin;
        pstats->ndata[pstats->ndatacount].bytesout = cur.h.m.bytesout;
        pstats->ndata[pstats->ndatacount].filesin = cur.h.m.filesin;
        pstats->ndata[pstats->ndatacount].filesout = cur.h.m.filesout;
        pstats->ndata[pstats->ndatacount].cost = cur.h.m.cost;
        pstats->ndata[pstats->ndatacount].timeconnected = cur.length;

        // avoid div by zero!
        if (pstats->ndata[pstats->ndatacount].timeconnected != 0)
          pstats->ndata[pstats->ndatacount].cpsall =
            (pstats->ndata[pstats->ndatacount].bytesin +
             pstats->ndata[pstats->ndatacount].bytesout) /
            pstats->ndata[pstats->ndatacount].timeconnected;
        else
          pstats->ndata[pstats->ndatacount].cpsall =
            (pstats->ndata[pstats->ndatacount].bytesin +
             pstats->ndata[pstats->ndatacount].bytesout);

        pstats->ndata[pstats->ndatacount].speed = cur.h.m.speed;
        pstats->ndata[pstats->ndatacount].protected = cur.h.m.passworded;
        strntcpy (pstats->ndata[count2].mailer, cur.h.m.mailer, MAILERSIZE);
        pstats->ndatacount++;
      }
      break;

    case CT_FAX:
      pstats->statshours[start->tm_hour][2]++;
      pstats->faxcalls++;

      for (count2 = 0; count2 < pstats->faxcount; count2++)
      {
        if (!strcmp (cur.h.f.remoteid, pstats->faxdata[count2].remoteid))
        {
          pstats->faxdata[count2].faxes++;
          pstats->faxdata[count2].bytes += cur.h.f.bytes;
          pstats->faxdata[count2].pages += cur.h.f.pages;
          break;
        }
      }

      if (count2 == pstats->faxcount)  /* No match then */
      {
        if (pstats->faxcount >= pstats->faxcapacity)
        {
          pstats->faxcapacity += BSIZE;
          pstats->faxdata = (FAXSTAT *) realloc (pstats->faxdata, (size_t) (pstats->faxcapacity * sizeof (FAXSTAT)));
          if (!pstats->faxdata)
          {
            memorypanic (in, pstats);
            return (x);
          }
        }

        strcpy (pstats->faxdata[pstats->faxcount].remoteid, cur.h.f.remoteid);
        pstats->faxdata[count2].faxes = 1;
        pstats->faxdata[count2].bytes = cur.h.f.bytes;
        pstats->faxdata[count2].pages = cur.h.f.pages;
        pstats->faxcount++;
      }
      break;

    case CT_BBS:
      pstats->statshours[start->tm_hour][1]++;
      pstats->bbscalls++;
      switch (cur.subclass)
      {
      case SC_LASTCALL:
        strcpy (user, cur.h.b1.name);
        break;

      case SC_CALLINFO:
      case SC_IBS:             /* internal BBS System */
        strcpy (user, cur.h.b2.name);
        break;

      case SC_TELEGARD:
        strcpy (user, cur.h.b3.realname);
        break;

      default:
        strcpy (user, "-- Unknown BBS --");
        break;
      }

      for (count2 = 0; count2 < pstats->bbscount; count2++)
      {
        if (!strcmp (user, pstats->bbsdata[count2].name))
        {
          pstats->bbsdata[count2].calls++;
          break;
        }
      }

      if (count2 == pstats->bbscount)  /* No match then */
      {
        if (pstats->bbscount >= pstats->bbscapacity)
        {
          pstats->bbscapacity += BSIZE;
          pstats->bbsdata = (BBSSTAT *) realloc (pstats->bbsdata, (size_t) (pstats->bbscapacity * sizeof (BBSSTAT)));
          if (!pstats->bbsdata)
          {
            memorypanic (in, pstats);
            return (x);
          }
        }

        strcpy (pstats->bbsdata[pstats->bbscount].name, user);
        pstats->bbsdata[count2].calls = 1;
        pstats->bbscount++;
      }
      break;

    default:
      pstats->othercalls++;
    }
  }

  fclose (in);

  if (!pstats->validrecords)
  {
    sb_move_puts (stwin, 10, 26, "     No entries to show!     ");
    sb_show ();
    timer (50);
    freestatsmemory (pstats);
    return x;
  }

  for (count = 0; count < pstats->ndatacount; count++)
  {
    if (!strstrci (pstats->ndata[count].mailer, "Unknown mailer"))
    {
      /* Add mailer to mailer list (full-id) */
      for (count2 = 0; count2 < pstats->mfullcount; count2++)
      {
        if (!strcmp (pstats->ndata[count].mailer,
                     pstats->mfulldata[count2].mailer))
        {
          pstats->mfulldata[count2].used++;
          break;
        }
      }

      if (count2 == pstats->mfullcount)  /* No match then */
      {
        if (pstats->mfullcount >= pstats->mfullcapacity)
        {
          pstats->mfullcapacity += BSIZE;
          pstats->mfulldata = (MAILERSTAT *) realloc (pstats->mfulldata, (size_t) (pstats->mfullcapacity * sizeof (MAILERSTAT)));
          if (!pstats->mfulldata)
          {
            memorypanic (in, pstats);
            return (x);
          }
        }

        strntcpy (pstats->mfulldata[pstats->mfullcount].mailer,
                  pstats->ndata[count].mailer, MAILERSIZE);
        pstats->mfulldata[count2].used = 1;
        pstats->mfullcount++;
      }

      /* Add mailer to mailer list (first word only) */
      strcpy (line, pstats->ndata[count].mailer);
      cleanmailer (line);

      for (count2 = 0; count2 < pstats->mwordcount; count2++)
      {
        if (!strcmp (line, pstats->mworddata[count2].mailer))
        {
          pstats->mworddata[count2].used++;
          break;
        }
      }

      if (count2 == pstats->mwordcount)  /* No match then */
      {
        if (pstats->mwordcount >= pstats->mwordcapacity)
        {
          pstats->mwordcapacity += BSIZE;
          pstats->mworddata = (MAILERSTAT *) realloc (pstats->mworddata, (size_t) (pstats->mwordcapacity * sizeof (MAILERSTAT)));
          if (!pstats->mworddata)
          {
            memorypanic (in, pstats);
            return (x);
          }
        }

        strntcpy (pstats->mworddata[pstats->mwordcount].mailer,
                  line, MAILERSIZE);
        pstats->mworddata[count2].used = 1;
        pstats->mwordcount++;
      }
    }

    /* Add speed to list - also count protected and unprotected nodes */
    if (pstats->ndata[count].protected)
      pstats->statprot++;
    else
      pstats->statunprot++;

    for (count2 = 0; count2 < pstats->speedcount; count2++)
    {
      if (pstats->ndata[count].speed == pstats->speeddata[count2].bps)
      {
        pstats->speeddata[count2].times++;
        break;
      }
    }

    if (count2 == pstats->speedcount)  /* No match then */
    {
      if (pstats->speedcount >= pstats->speedcapacity)
      {
        pstats->speedcapacity += BSIZE;
        pstats->speeddata = (SPEEDSTAT *) realloc (pstats->speeddata, (size_t) (pstats->speedcapacity * sizeof (SPEEDSTAT)));
        if (!pstats->speeddata)
        {
          memorypanic (in, pstats);
          return (x);
        }
      }

      pstats->speeddata[count2].bps = pstats->ndata[count].speed;
      pstats->speeddata[count2].times = 1;
      pstats->speedcount++;
    }
  }

  time (&t1);
  t1 -= (t1 % (24L * 3600L));
  t2 = timerecord;
  t2 -= (t2 % (24L * 3600L));

  /* So t1 & t2 counts seconds to start of the day */
  t1 = ((t1 - t2) / (3600L * 24L)) + 1;
  pstats->statsdays = (unsigned int) t1;

  pstats->topwhat = (rp->save_ht - 7) / 2;
  StatsScreen1 (pstats);
  t_idle = timerset (PER_MINUTE);

#ifdef DEBUGSTATS
  for (count = 0; count < pstats->ndatacount; count++)
    status_line (":Address: %u:%u/%u.%u",
                 pstats->ndata[count].address.Zone,
                 pstats->ndata[count].address.Net,
                 pstats->ndata[count].address.Node,
                 pstats->ndata[count].address.Point);
  done = 1;
#endif

  while (!timeup (t_idle) && (PEEKBYTE () == (short) 0xffff) && !done)
  {
    if (KEYPRESS ())
    {
      t_idle = timerset (30 * PER_SECOND);
#ifdef DOS16                    /*  TS 970604   */
      c = FOSSIL_CHAR ();
      if ((c & 0xff) != 0)
      {
        c &= 0xff;
        c = toupper (c);
      }
#else
      c = (int) KbRemap (FOSSIL_CHAR ());
#endif

      switch (c)
      {
      case 27:
        done = 1;
        break;

      case LFAR:
        if (OnScreen > 1)
          OnScreen--;
        break;

      case RTAR:
        if (OnScreen < 8)
          OnScreen++;
        break;

      case F_PEND_UPAR:
        if (pstats->stfirst > 0)
          pstats->stfirst--;
        break;

      case F_PEND_DNAR:
        pstats->stfirst++;
        break;

      default:
        if (c >= '0' && c <= '9')
        {
          if ((c - '0') == OnScreen)
            continue;
          else
            OnScreen = c - '0';
        }
      }

      switch (OnScreen)
      {
      case 1:
        StatsScreen1 (pstats);
        break;

      case 2:
        StatsScreen2 (pstats);
        break;

      case 3:
        StatsScreen3 (pstats);
        break;

      case 4:
        StatsScreen4 (pstats);
        break;

      case 5:
        StatsScreen5 (pstats);
        break;

      case 6:
        StatsScreen6 (pstats);
        break;

      case 7:
        StatsScreen7 (pstats);
        break;

      case 8:
        StatsScreen8 (pstats);
        break;
      }
    }

    time_release ();
  }

  freestatsmemory (pstats);
  return (x);
}

/* $Id: stats.c,v 1.7 1999/05/15 23:53:38 ag Exp $ */
