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
 * Filename    : $Source: E:/cvs/btxe/src/bbs_io.c,v $
 * Revision    : $Revision: 1.9 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/06/10 16:55:39 $
 * State       : $State: Exp $
 * Orig. Author: C. Eike Hofmeister, 2:2426/2280
 *
 * Description : bbs io-functions
 *
 *---------------------------------------------------------------------------*/

#ifdef BBS_INTERNAL

#include "includes.h"
#include "bbs_incl.h"

#define SCREEN_WIDTH 80

/****************************************************************************/

int
strcmpcase (char *a, char *b)
{
  int i;
  char da[defaultstrlen + 1], db[defaultstrlen + 1];

  for (i = 0; i < strlen (a); i++)
  {
    da[i] = toupper (a[i]);
  }

  da[i] = '\0';

  for (i = 0; i < strlen (b); i++)
  {
    db[i] = toupper (b[i]);
  }

  db[i] = '\0';
  return strcmp (da, db);
}

/****************************************************************************/

void
TimeCheck (char init)
{
  static time_t lasttimerdec;
  static USHORT wascarrier;

  time_t timenow;
  int i;

  unix_time (&timenow);
  if (init)
  {
    wascarrier = CARRIER;
    lasttimerdec = timenow;
    return;
  }

  while (timenow != lasttimerdec)
  {
    for (i = 0; i < timers_used; i++)
    {                           /* decrement timer-variables. if one of them */
      if (numval[i + 64])
        if (!(--numval[i + 64]))
          eventdetected = i + ev_timer_expired;  /* becomes zero: event! */
    }

    lasttimerdec++;
  }

  if (CARRIER ^ wascarrier)
  {                             /* set event on new carrier or loss of carrier */
    if (wascarrier)
      eventdetected = ev_carrier_lost;
    else
      eventdetected = ev_carrier_found;
    wascarrier = CARRIER;
  }

  show_alive ();
}

/****************************************************************************/

char *
str_address (char number)
{
  static char nothing[2] = "\0";
  static char retstr[35];

  if (!(number & 64))
  {                             /* special-strings */
    switch (number)
    {
    case 0:
      return (sysop);           /* 0: Sysop name (config: Sysop)       */

    case 1:
      strcpy (retstr, "0:0/0.0");
      return (retstr);          /* 1: main address (config: address)   */

    case 2:
      return (system_name);     /* 2: system name (config: System)     */

    case 3:
      return (location);        /* 3: location (config: MyLocation)    */

    case 4:
      return (myphone);         /* 4: phone (config: MyPhone)          */

    case 6:
      return (ANNOUNCE);        /* 6: "BT/2 2.60XE/..."                */

    case 7:
      sprintf (retstr, "%3d", TaskNumber);
      if (retstr[0] == ' ')
        retstr[0] = '0';
      if (retstr[1] == ' ')
        retstr[1] = '0';
      return (retstr);          /* 7: tasknumber ("001" etc.)          */

    default:
      eventdetected = ev_internalerror;
      return (nothing);
    }
  }
  else
    return (strval[number & (1 + 2 + 4 + 8 + 16 + 32)]);
}

/****************************************************************************/

void
createrealstring (char *now, char *old, int maxlen)
{
  int i, j, t = 0;
  char dummy[200];
  char *str2add;

  if (maxlen)
    maxlen--;

  for (i = 1; (i <= old[0]) && (t < maxlen);)
  {
    switch (old[i++])
    {
    case 0:                    /* just a char */
      now[t++] = old[i++];
      break;

    case 1:                    /* add a variable */
      if (old[i] & 128)
      {
        str2add = str_address (old[i++] & (1 + 2 + 4 + 8 + 16 + 32 + 64));
        for (j = 0; (str2add[j]) && (t < maxlen); now[t++] = str2add[j++]) ;
        for (j = j; (j < old[i]) && (t < maxlen); j++)
          now[t++] = old[i + 1];
      }
      else
      {
        j = numval[old[i++] & (1 + 2 + 4 + 8 + 16 + 32 + 64)];
        sprintf (dummy, "%i", j);
        for (j = strlen (dummy); (j < old[i]) && (t < maxlen); j++)
          now[t++] = old[i + 1];
        for (j = 0; (dummy[j]) && (t < maxlen); now[t++] = dummy[j++]) ;
      }

      i += 2;
      break;

    case 'c':
      i++;
      break;

    case 'x':
      break;

    case 'n':
      break;

    case 's':
      break;

    case 'r':
      break;

    case 'm':
      i += 2;
      break;

    case '>':
      i++;
      break;

    case '<':
      i++;
      break;

    case '!':
      i++;
      break;

    case '^':
      i++;
      break;
    }
  }

  now[t] = '\0';
}

/****************************************************************************/

char curcolor;                  /* last used color for writing              */
int curposx, curposy;           /* current cursor position                  */
char last_col;                  /* space for saveing cursor-position        */
int last_posx, last_posy;       /*                                   /color */

void
bbs_initoutput ()
{
  curcolor = last_col = 7;
  curposx = curposy = last_posx = last_posy = 0;
}

int
chrpos (char *s, char c)
{
  int i;

  for (i = 0; i < strlen (s); i++)
  {
    if (s[i] == c)
      return i;
  }

  return -1;
}

void
bbsclearscreen ()
{
  if (eventdetected != ev_none)
    return;

  curcolor = 7;

  switch (emulation)
  {
  case emul_ansi:
    if (!bbslocalmode)
      SENDCHARS ("\x1b[0;37;40m\x1b[2J\0", 14, 0);
    if (script_fullscreen)
      screen_clear ();
    break;

  default:                     /* ascii */
    if (!bbslocalmode)
      SENDCHARS ("\14", 1, 0);
    if (script_fullscreen)
      screen_clear ();
    break;
  }

  curposx = curposy = 0;
  curcolor = 0;
}

void
inccursorposition ()
{
  switch (emulation)
  {
  case emul_ansi:
    if (curposx < (SCREEN_WIDTH - 1))
      curposx++;
    break;

  default:
    curposx++;
    if (curposx >= SCREEN_WIDTH)
    {
      curposx = 0;
      curposy++;
    }

    break;
  }
}

void
bbssendchar (char this)
{
  if (eventdetected != ev_none)
    return;

  if (!bbslocalmode)
    SENDBYTE (transout (this));

  if (script_fullscreen)
    WRITE_ANSI (this);

  inccursorposition ();
}

void
bbssendstring (char *this)
{
  int i;

  if (eventdetected != ev_none)
    return;

  for (i = 0; this[i]; i++)
  {
    if (!bbslocalmode)
      SENDBYTE (transout (this[i]));
    if (script_fullscreen)
      WRITE_ANSI (this[i]);
    inccursorposition ();
  }
}

void
bbsgotoxy (char x, char y)
{
  char dummys[10];

  switch (emulation)
  {
  case emul_ansi:
    sprintf (dummys, "\33[%d;%dH", y + 1, x + 1);
    if (!bbslocalmode)
      SENDCHARS (dummys, strlen (dummys), 0);
    if (script_fullscreen)
      fossil_gotoxy (x, y);
    curposx = x;
    curposy = y;
    break;

  case emul_not_set:
  case emul_ascii:
    break;
  }
}

void
bbschangecolor (char newcolor)
{
  short doall = 0;
  char dummy[20];
  int i;

  if (curcolor == newcolor)
    return;

  if ((emulation == emul_ansi))
  {
    dummy[0] = '\x1b';
    dummy[1] = '[';
    dummy[2] = '\0';

    if (((!(newcolor & 0x08)) && (curcolor & 0x08)) || ((!(newcolor & 0x80))
                                                      && (curcolor & 0x80)))
    {
      strcat (dummy, "0;");
      doall = 1;
    }

    if (((newcolor & 0x07) != (curcolor & 0x07)) || (doall))
    {
      switch (newcolor & 0x07)
      {
      case 0:
        strcat (dummy, "30;\0");
        break;

      case 1:
        strcat (dummy, "34;\0");
        break;

      case 2:
        strcat (dummy, "32;\0");
        break;

      case 3:
        strcat (dummy, "36;\0");
        break;

      case 4:
        strcat (dummy, "31;\0");
        break;

      case 5:
        strcat (dummy, "35;\0");
        break;

      case 6:
        strcat (dummy, "33;\0");
        break;

      case 7:
        strcat (dummy, "37;\0");
        break;
      }
    }

    if (((newcolor & 0x70) != (curcolor & 0x70)) || (doall))
    {
      switch ((newcolor & 0x70) >> 4)
      {
      case 0:
        strcat (dummy, "40;\0");
        break;

      case 1:
        strcat (dummy, "44;\0");
        break;

      case 2:
        strcat (dummy, "42;\0");
        break;

      case 3:
        strcat (dummy, "46;\0");
        break;

      case 4:
        strcat (dummy, "41;\0");
        break;

      case 5:
        strcat (dummy, "45;\0");
        break;

      case 6:
        strcat (dummy, "43;\0");
        break;

      case 7:
        strcat (dummy, "47;\0");
        break;
      }
    }

    if ((newcolor & 0x08) && ((!(curcolor & 0x08)) || (doall)))
      strcat (dummy, "1;");     /* intense */

    if ((newcolor & 0x80) && ((!(curcolor & 0x80)) || (doall)))
      strcat (dummy, "5;");     /* blink */

    dummy[strlen (dummy)] = '\0';
    dummy[strlen (dummy) - 1] = 'm';
    curcolor = newcolor;

    if (script_fullscreen)
    {
      for (i = 0; i < strlen (dummy); i++)
        WRITE_ANSI (dummy[i]);
    }
  }

  switch (emulation)
  {
  case emul_ansi:
    if (!bbslocalmode)
      SENDCHARS (dummy, strlen (dummy), 0);
    break;                      /* ansi */

  case emul_not_set:
  case emul_ascii:
    break;
  }
}

void
bbssend (char *old)
{
  int i, j;
  char dummy[200];
  char *str2add;

  for (i = 1; i <= old[0];)
  {
    switch (old[i++])
    {
    case 0:                    /* just a char */
      if (!bbslocalmode)
        SENDBYTE (transout (old[i]));
      if (script_fullscreen)
        WRITE_ANSI (old[i]);
      inccursorposition ();
      i++;
      break;

    case 1:                    /* add a variable */
      if (old[i] & 128)
      {
        str2add = str_address (old[i++] & (1 + 2 + 4 + 8 + 16 + 32 + 64));
        bbssendstring (str2add);
        for (j = strlen (str2add); j < old[i]; j++)
          bbssendchar (old[i + 1]);
      }
      else
      {
        sprintf (dummy, "%d", j);
        numval[old[i++] & (1 + 2 + 4 + 8 + 16 + 32 + 64)] = j;
        for (j = strlen (dummy); j < old[i]; j++)
          bbssendchar (old[i + 1]);
        bbssendstring (dummy);
      }
      i += 2;
      break;

    case 'c':
      bbschangecolor (old[i++]);
      break;

    case 'x':
      bbsclearscreen ();
      break;

    case 'n':
      switch (emulation)
      {
      case emul_ansi:
        bbsgotoxy (0, curposy + 1);
        break;

      default:                 /* ascii: */
        bbssendstring ("\12\15");
        curposx = 0;
        curposy++;
        break;
      }
      break;

    case 's':
      last_posx = curposx;
      last_posy = curposy;
      last_col = curcolor;
      break;

    case 'r':
      bbsgotoxy (last_posx, last_posy);
      bbschangecolor (last_col);
      break;

    case 'm':
      bbsgotoxy (old[i], old[i + 1]);
      i += 2;
      break;

    case '>':
      bbsgotoxy (curposx + old[i++], curposy);
      break;

    case '<':
      bbsgotoxy (curposx - old[i++], curposy);
      break;

    case '!':
      bbsgotoxy (curposx, curposy + old[i++]);
      break;

    case '^':
      bbsgotoxy (curposx, curposy - old[i++]);
      break;

    default:
      eventdetected = ev_internalerror;
      return;
    }
  }
}

/****************************************************************************/

void
bbsbackchar ()
{
  switch (emulation)
  {
  case emul_ansi:
    if (!bbslocalmode)
      SENDCHARS ("\33[D", 3, 0);
    if (script_fullscreen)
      fossil_gotoxy (curposx - 1, curposy);
    break;

  default:                     /* ascii */
    if (!bbslocalmode)
      SENDBYTE ('\10');
    if (script_fullscreen)
      fossil_gotoxy (curposx - 1, curposy);
    break;
  }

  if (curposx > 0)
    curposx--;
}

void
bbsgetinputline (char *this, char spacechar, char *special, int maxlength)
{
  int i, j, dummy, outchar = 0;
  char allowed[256], upall = 0, dnall = 0;

  status_line (">bbsgetinputline: <%c><%d><%s>", spacechar, maxlength, special);

  if (eventdetected != ev_none)
    return;

  if (strlen (this) > maxlength)
    this[maxlength] = '\0';

  bbssendstring (this);

  for (i = strlen (this); i < maxlength; i++)
    bbssendchar (spacechar);

  for (i = strlen (this); i < maxlength; i++)
    bbsbackchar ();

  for (j = 0; j < 256; j++)
    allowed[j] = 0;

  for (i = 0; i < strlen (special); i++)
  {
    switch (toupper (special[i]))
    {
    case 'D':
      for (j = 0; j < 256; j++)
        if (isdigit (j))
          allowed[j] = 1;
      break;

    case 'A':
      for (j = 0; j < 256; j++)
        if (isalpha (j))
          allowed[j] = 1;
      break;

    case 'P':
      for (j = 0; j < 256; j++)
        if (isprint (j))
          allowed[j] = 1;
      break;

    case '+':
      j = special[++i];
      allowed[j] = 1;
      break;

    case 'H':
      outchar = special[++i];
      break;

    case 'U':
      upall = 1;
      dnall = 0;
      break;

    case 'L':
      upall = 0;
      dnall = 1;
      break;
    }
  }

  do
  {
    dummy = 0;

    do
    {
      if ((CHAR_AVAIL ()))
        dummy = transin (MODEM_IN ());

      if (KEYPRESS ())
        dummy = READKB ();

      if (!dummy)
      {
        TimeCheck (0);
        /* time_release(); - this cant be used here, as it moves the cursor to the life-indicator */
      }

      else if (timer_setonkey)
        numval[timer_setonkey] = time_setonkey;
    }
    while ((eventdetected == ev_none) && (dummy == 0));

    if (eventdetected != ev_none)
    {
      for (i = 0; i < strlen (this); i++)
        bbsbackchar ();
      return;
    }

    switch (dummy)
    {
    case '\10':
      if (strlen (this) >= 1)
      {
        this[strlen (this) - 1] = '\0';
        bbsbackchar ();
        bbssendchar (spacechar);
        bbsbackchar ();
      }
      break;

    default:
      if (dnall)
        dummy = tolower (dummy);

      if (upall)
        dummy = toupper (dummy);

      if (strlen (this) < maxlength)
      {
        if (allowed[dummy])
        {
          this[strlen (this) + 1] = '\0';
          this[strlen (this)] = dummy;
          if (outchar)
            bbssendchar (outchar);
          else
            bbssendchar (dummy);
        }
      }

      break;
    }
  }
  while ((eventdetected == ev_none) && (dummy != '\15'));

  for (i = 0; i < strlen (this); i++)
    bbsbackchar ();
}

/*************************************************************************************************************/

void
removespace (FILE * file, long howlong)
{
  int dummy;
  long src, tgt, was;

  was = tgt = ftell (file);
  src = tgt + howlong;
  fseek (file, src, SEEK_SET);

  while ((dummy = getc (file)) != EOF)
  {
    fseek (file, tgt++, SEEK_SET);
    fputc (dummy, file);
    fseek (file, ++src, SEEK_SET);
  }                             /* endwhile */

  fseek (file, tgt, SEEK_SET);
  fputs ("", file);             /* writing zero bytes only writes Ctrl-Z, but does not truncate the file :-( */
  fseek (file, was, SEEK_SET);
}

void
preparespace (FILE * file, long howlong)
{
  long was, i, tgt, src;
  int dummy;

  was = ftell (file);
  fseek (file, 0, SEEK_END);
  src = ftell (file);

  for (i = 0; i < howlong; i++)
    fputc ('\0', file);

  tgt = ftell (file);

  while (was < src)
  {
    fseek (file, --src, SEEK_SET);
    dummy = fgetc (file);
    fseek (file, --tgt, SEEK_SET);
    fputc ((unsigned char) dummy, file);
  }                             /* endwhile */

  fseek (file, was, SEEK_SET);
}

void
file_removeline (FILE * file, long howmany)
{
  long was, end;
  char dummystr[defaultstrlen];

  was = ftell (file);

  while (howmany--)
  {
    if (!(feof (file)))
      fgets (dummystr, defaultstrlen, file);
  }

  end = ftell (file);
  fseek (file, was, SEEK_SET);
  removespace (file, end - was);
}

void
file_replaceline (FILE * file, char *this)
{
  long was;
  char dummystr[300];

  was = ftell (file);
  dummystr[0] = '\0';

  if (!(feof (file)))
    fgets (dummystr, 300, file);

  fseek (file, was, SEEK_SET);

  if (strlen (dummystr) < strlen (this))
    preparespace (file, strlen (this) - strlen (dummystr));
  else if (strlen (dummystr) > strlen (this))
    removespace (file, strlen (this) - strlen (dummystr));

  fputs (this, file);
  fputc ('\r', file);
  fputc ('\n', file);
}

void
file_insertline (FILE * file, char *this)
{
  preparespace (file, strlen (this) + 2);
  fputs (this, file);
  fputc ('\r', file);
  fputc ('\n', file);
}

/*************************************************************************************************************/

void
bbs_calculate (char *instr, long *outnum)
{
  char dummy[20];
  char op;
  int src, tgt, dummyint;;

  for (src = 0, tgt = 0; src <= strlen (instr); src++)
    if (instr[src] != ' ')
      instr[tgt++] = instr[src];

  src = 0;
  *outnum = 0;

  while (src < strlen (instr))
  {
    if (!src)
      op = '+';
    else
      op = instr[src++];
    tgt = 0;
    while ((src <= strlen (instr)) && ('0' <= instr[src]) && (instr[src] <= '9'))
      dummy[tgt++] = instr[src++];
    dummy[tgt] = '\0';
    sscanf (dummy, "%i", &dummyint);
    switch (op)
    {
    case '+':
      (*outnum) += dummyint;
      break;

    case '-':
      (*outnum) -= dummyint;
      break;

    case '*':
      (*outnum) *= dummyint;
      break;

    case '/':
      (*outnum) /= dummyint;
      break;
    }
  }
}

/****************************************************************************/

void
filegetnamefrompathname (char *inpathname, char *name)
{
  int i, j, t;

  j = strlen (inpathname) - 1;

  while ((inpathname[j] != DIR_SEPC) && (j > 0))
    j--;

  if ((inpathname[j] != DIR_SEPC))
    j--;

  t = 0;

  for (i = j + 1; i < strlen (inpathname); i++)
    name[t++] = inpathname[i];

  name[t] = '\0';
}

void
filegetpathfrompathname (char *inpathname, char *path)
{
  int i, j;

  j = strlen (inpathname) - 1;

  while ((inpathname[j] != DIR_SEPC) && (j > 0))
    j--;

  if ((inpathname[j] != DIR_SEPC))
    j--;

  for (i = 0; i <= j; i++)
    path[i] = inpathname[i];

  path[i] = '\0';
}

#define MAX_LINE_LENGTH 80
#define MAX_INFO_LINES  20

typedef char infodatalinetype[MAX_LINE_LENGTH];
infodatalinetype *nextinfodata = NULL;
int nextinfodatapos = 0, nextinfodataposlast;

void
filegetdescriptiondone ()
{
  if (nextinfodata != NULL)
  {
    free (nextinfodata);
    nextinfodata = NULL;
  }
}

void
filegetdescriptioninit (char *infile, char *filesbbs, long *lines, long
                        *size, long *date, long *counter)
{
  FILE *descfile;
  short done, lastlinewasempty;
  char read[MAX_LINE_LENGTH], foundfile[13], countstr[12], infilefile[20];
  int i, j, t;
  struct stat buf;

  if ((stat (infile, &buf) != -1))
  {
    (*size) = buf.st_size;
    (*date) = buf.st_mtime;
  }
  else
  {
    size = 0;
    date = 0;
  }

  filegetdescriptiondone ();
  filegetnamefrompathname (infile, infilefile);

  if (filesbbs[0] == '\0')
  {
    filegetpathfrompathname (infile, read);
    strcat (read, "FILES.BBS");
  }
  else
    strcpy (read, filesbbs);

  strcpy (infile, infilefile);

  if ((descfile = fopen (read, read_ascii)) != NULL)
  {
    done = 0;
    while ((!done))
    {
      if ((fgets (read, MAX_LINE_LENGTH, descfile)) == NULL)
      {
        done = 1;
      }
      else
      {
        if ((read[strlen (read) - 1] == '\n') || (read[strlen (read) - 1] == '\r'))
          read[strlen (read) - 1] = '\0';

        if ((read[strlen (read) - 1] == '\n') || (read[strlen (read) - 1] == '\r'))
          read[strlen (read) - 1] = '\0';

        switch (read[0])
        {
        case '\0':
          continue;

        case ' ':
          continue;
        }

        for (i = 0; i < strlen (infilefile); i++)
          infilefile[i] = toupper (infilefile[i]);

        for (i = 0; ((i < strlen (read)) && (read[i] != ' ') && (i < 12));
             i++)
          foundfile[i] = toupper (read[i]);

        foundfile[i] = '\0';

        if (!(strcmp (infilefile, foundfile)))
        {                       /* file found in files.bbs */
          j = i + 1;

          while ((i < strlen (read)) && (read[i] == ' '))
            i++;

          counter = 0;

          if (read[i] == '[')
          {                     /* check for download-counters */
            i++;
            t = 0;

            while ((read[i] == ' ') || (('0' <= read[i]) && (read[i] <= '9')))
              countstr[t++] = read[i++];

            countstr[t] = '\0';

            if (read[i] == ']')
            {
              sscanf (countstr, "%d", &t);
              *counter = t;
              j = i + 2;
            }
          }

          t = 0;

          nextinfodatapos = 0;
          nextinfodataposlast = 0;
          nextinfodata = calloc (MAX_INFO_LINES, sizeof (infodatalinetype));

          while (!done)
          {
            t = 0;

            while (read[j])
              nextinfodata[nextinfodataposlast][t++] = read[j++];

            read[j] = '\0';

            if (nextinfodata[nextinfodataposlast][0])
            {
              nextinfodataposlast++;
              if (nextinfodataposlast == MAX_INFO_LINES)
                done = 1;
              lastlinewasempty = 0;
            }
            else
            {
              if (!lastlinewasempty)
              {
                nextinfodataposlast++;
                if (nextinfodataposlast == MAX_INFO_LINES)
                  done = 1;
              }

              lastlinewasempty = 1;
            }

            if ((fgets (read, MAX_LINE_LENGTH, descfile)) == NULL)
            {
              done = 1;
            }
            else
            {
              if (read[0])
                if ((read[strlen (read) - 1] == '\33') || (read[strlen
                                                       (read) - 1] == '\n'))
                  read[strlen (read) - 1] = '\0';

              if (read[0])
                if ((read[strlen (read) - 1] == '\33') || (read[strlen
                                                       (read) - 1] == '\r'))
                  read[strlen (read) - 1] = '\0';

              j = 0;

              while ((read[j] == ' ') && (j < strlen (read)))
                j++;

              if (!((read[j] == '+') || (read[j] == '>')))
                done = 1;
              else
                j++;
            }
          }

          if (nextinfodataposlast == 0)
          {
            free (nextinfodata);
            nextinfodata = NULL;
          }

          fclose (descfile);
          (*lines) = nextinfodataposlast;
          return;
        }
      }
    }

    fclose (descfile);
  }
  else
    counter = 0;

  (*lines) = 0;
}

void
filegetdescriptiontext (char *info)
{
  if (nextinfodata != NULL)
  {
    strcpy (info, nextinfodata[nextinfodatapos]);
    nextinfodatapos++;

    if (nextinfodataposlast == nextinfodatapos)
    {
      free (nextinfodata);
      nextinfodata = NULL;
    }
  }
  else
    info[0] = '\0';
}

void
bbs_printdate (time_t * longtime, char *mode, char *info)
{
  struct tm *time;
  int i, j;
  short addzero;
  char dummy[5];

  info[0] = 0;
  time = localtime (longtime);

  for (i = 0; i < strlen (mode); i++)
  {
    addzero = 1;
    switch (mode[i])
    {
    case 'D':
    case 'd':                  /* add day: */
      sprintf (dummy, "%2d", time->tm_mday);
      break;

    case 'M':
    case 'm':
      sprintf (dummy, "%2d", (time->tm_mon) + 1);
      break;

    case 'Y':                  /* year (1997) */
      sprintf (dummy, "%4d", time->tm_year + ((time->tm_year < 70) ? 2000 : 1900));
      break;                    /* AG 990120 Y2K fix */

    case 'y':                  /* year (97) */
      sprintf (dummy, "%2d", time->tm_year);
      break;

    case 'H':
    case 'h':                  /* add hour */
      sprintf (dummy, "%2d", time->tm_hour);
      break;

    case 'I':
    case 'i':                  /* add mInutes */
      sprintf (dummy, "%2d", time->tm_min);
      break;

    case 'S':
    case 's':                  /* seconds */
      sprintf (dummy, "%2d", time->tm_sec);
      break;

    default:
      strncat (info, &(mode[i]), 1);
      addzero = 0;
      break;
    }

    if (addzero)
    {
      for (j = 0; j < strlen (dummy); j++)
        if (dummy[j] == ' ')
          dummy[j] = '0';
      strcat (info, dummy);
    }
  }
}

void
getfileinfosearch (char *find)
{
}

/****************************************************************************/

signed int
bbs_sendfile (char *filename, char prot)
{
  char *p;

  if ((strchr (native_protocols, prot) == NULL) && ((extern_protocols ==
                                                     NULL) ? (1) : (p =
                                                                    strchr
                                                          (extern_protocols,
                                                           prot)) == NULL))
  {
    status_line ("!Unknown protocol <%c>", prot);
    return (1);
  }

  if ((!filename[0]) || (!dexists (filename)))
  {
    status_line ("!Can't find <%s>", filename);
    return (1);
  }

  if ((Upload (filename, (int) prot, p)) == FILE_SENT_OK)
    return (0);
  else
    return (1);
}

signed int
bbs_receivefile (char *filename, char prot)
{
  char *p;

  if ((strchr (native_protocols, prot) == NULL) && ((extern_protocols ==
                                                     NULL) ? (1) : (p =
                                                                    strchr
                                                          (extern_protocols,
                                                           prot)) == NULL))
  {
    status_line ("!Unknown protocol <%c>", prot);
    return (1);
  }

  if ((Download (filename, (int) prot, p)) == 1)
    return (0);
  else
    return (1);
}

/****************************************************************************/

#undef SCREEN_WIDTH

#endif

/* $Id: bbs_io.c,v 1.9 1999/06/10 16:55:39 ceh Exp $ */
