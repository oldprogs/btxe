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
 * Filename    : $Source: E:/cvs/btxe/xprmntl/bbs/compile.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 12:10:49 $
 * State       : $State: Exp $
 * Orig. Author: C. Eike Hofmeister, 2:2426/2280
 *
 * Description : Binkley BBS Script Compiler
 *
 *---------------------------------------------------------------------------*/

// includes

//----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#ifdef __unix__
#include "../typesize/typesize.h"
#else
#include "..\typesize\typesize.h"
#endif

#define DONE  2
#define ERROR 1
#define OK    0

#define STRLEN 255

#define MAXERRORS 20

//============================================================================

// Error-Messages

//----------------------------------------------------------------------------

UCHAR errormessage[STRLEN];

//============================================================================

// Statistics

//----------------------------------------------------------------------------

struct statisticsstruct
{
  INT totalfiles;
  INT totallines;
  INT totallinescompiled;
}
statistics, statisticstotal;

//============================================================================

// Stuff for reading the source and include-files

//----------------------------------------------------------------------------

// Variables

FILE *readmainfile = NULL;
FILE *readinclfile = NULL;

UCHAR readmainname[STRLEN];
UCHAR readinclname[STRLEN];
UCHAR name[STRLEN];

INT readmainline;
INT readinclline;
INT line;

UCHAR readingin;                // m:mainfile, i:includefile, other:nowhere

UCHAR readline[STRLEN];         // line read in

UCHAR readpart[STRLEN];         // part of it

//----------------------------------------------------------------------------

// strip leading blanks in readline

void 
stripleadingblanks (UCHAR * this)
{
  INT i, j;

  for (i = 0; this[i] == ' '; i++) ;
  for (j = 0; this[i]; i++, j++)
    this[j] = this[i];
  this[j] = 0;
}

//----------------------------------------------------------------------------

// copy first part of readline to readpart, fill with blanks in readline

void 
readgetpart (UCHAR upcase)
{
  INT i, j;

  stripleadingblanks (readline);
  j = 0;
  if (readline[0] == '"')
  {
    readline[0] = ' ';
    for (i = 1; (readline[i] != '"') && (readline[i]); i++, j++)
    {
      readpart[j] = readline[i];
      readline[i] = ' ';
    }                           /* endif */
    if (readline[i] == '"')
      readline[i] = ' ';
  }
  else
  {
    for (i = 0; (readline[i]) && (readline[i] != ' '); i++, j++)
    {
      readpart[j] = readline[i];
      readline[i] = ' ';
    }                           /* endif */
  }                             /* endif */
  readpart[j] = 0;
  switch (upcase)
  {
  case 'u':
    for (i = 0; readpart[i]; i++)
      readpart[i] = toupper (readpart[i]);
    break;
  case 'l':
    for (i = 0; readpart[i]; i++)
      readpart[i] = tolower (readpart[i]);
    break;
  }                             /* endswitch */
// printf("[%s][%s]",readpart,readline);
}

//----------------------------------------------------------------------------

// start reading "readmainname"

UCHAR 
readstart ()
{
  statistics.totalfiles = 1;
  statistics.totallines = 0;
  statistics.totallinescompiled = 0;
  if ((readmainfile = fopen (readmainname, "r")) == NULL)
  {
    sprintf (errormessage, "Could not open %s", readmainname);
    return (ERROR);
  }
  else
  {
    readmainline = 0;
    readingin = 'm';
    return (OK);
  }                             /* endif */
}

//----------------------------------------------------------------------------

// read next line to compile

UCHAR 
readnextline ()
{
  UCHAR foundline = 0;
  INT i;

  while (!foundline)
  {
    switch (readingin)
    {
    case 'm':
      strncpy (name, readmainname, STRLEN);
      if ((fgets (readline, STRLEN, readmainfile)) == NULL)
      {
        fclose (readmainfile);
        readmainfile = NULL;
        readingin = 0;
        readline[0] = 0;
        return (DONE);
      }                         /* endif */
      line = ++readmainline;
      break;
    case 'i':
      strncpy (name, readinclname, STRLEN);
      if ((fgets (readline, STRLEN, readinclfile)) == NULL)
      {
        fclose (readinclfile);
        readinclfile = NULL;
        readingin = 'm';
        readline[0] = 0;
      }                         /* endif */
      line = ++readinclline;
      break;
    default:
      strcpy (errormessage, "Internal error");
      return (ERROR);
    }                           /* endswitch */
    statistics.totallines++;
    stripleadingblanks (readline);
    for (i = 0; i < strlen (readline); i++)
      if ((readline[i] == '\r') || (readline[i] == '\n'))
        readline[i] = 0;
    switch (readline[0])
    {
    case ';':
    case 0:
      break;
    case '#':
      if (readingin == 'i')
      {
        sprintf (errormessage,
                 "Cannot include file [%s] from included file [%s]",
                 &(readline[1]), readinclname);
        return (ERROR);
      }                         /* endif */
      readinclline = 0;
      statistics.totalfiles++;
      readline[0] = ' ';
      stripleadingblanks (readpart);
      strncpy (readinclname, &(readline[1]), STRLEN);
      if ((readinclfile = fopen (readinclname, "r")) == NULL)
      {
        sprintf (errormessage, "File [%s] not found", readinclname);
        return (ERROR);
      }
      else
      {
        readingin = 'i';
      }                         /* endif */
      break;
    default:
      foundline = 1;
      break;
    }                           /* endswitch */
  }                             /* endwhile */
  statistics.totallinescompiled++;
  return (OK);
}

//----------------------------------------------------------------------------

// end reading

void 
readend ()
{
  if (readmainfile != NULL)
  {
    fclose (readmainfile);
    readmainfile = NULL;
  }                             /* endif */
  if (readinclfile != NULL)
  {
    fclose (readinclfile);
    readinclfile = NULL;
  }                             /* endif */
  readingin = 0;
}

//============================================================================

// List of script-commands, emulation-types, etc.

//----------------------------------------------------------------------------

static UCHAR *funcs[] =
{

  "goto",                       /* 00 */
  "ifstr",                      /* 01 */
  "ifnum",                      /* 02 */
  "event",                      /* 03 */

  "loadtranslation",            /* 04 */
  "logfile",                    /* 05 */

  "setvar",                     /* 06 */
  "addtohistory",               /* 07 */

  "print",                      /* 08 */

  "getkey",                     /* 09 */
  "getkeysysop",                /* 0a */
  "getkeymodem",                /* 0b */

  "getstr",                     /* 0c */
  "parse",                      /* 0d */

  "fileopen",                   /* 0e */
  "fileclose",                  /* 0f */
  "fileposition",               /* 10 */
  "filegoto",                   /* 11 */
  "filetruncate",               /* 12 */
  "filelineread",               /* 13 */
  "filelineinsert",             /* 14 */
  "filelinereplace",            /* 15 */
  "filelineremove",             /* 16 */

  "fullscreen",                 /* 17 */
  "unattended",                 /* 18 */
  "timesetonkey",               /* 19 */

  "abort",                      /* 1a */

  "emulation",                  /* 1b */

  "dropcarrier",                /* 1c */
  "ifcarrier",                  /* 1d */

  "call",                       /* 1e */
  "return",                     /* 1f */

  "releasetime",                /* 20 */
  "waitforallsent",             /* 21 */

  "getfileinfo",                /* 22 */
  "getfileinfotext",            /* 23 */
  "getfileinfosearch",          /* 24 */

  "setvardate",                 /* 25 */

  "sendfile",                   /* 26 */
  "sendfilelist",               /* 27 */
  "receivefile",                /* 28 */
  "deletefile",                 /* 29 */

  NULL};

//----------------------------------------------------------------------------

static UCHAR *var_str_ro[] =
{                               /* read-only string variables */
  "!sysop",                     /* 0: Sysop name (config: Sysop)       */
  "!address",                   /* 1: main address (config: address)   */
  "!system",                    /* 2: system name (config: System)     */
  "!location",                  /* 3: location (config: MyLocation)    */
  "!phone",                     /* 4: phone (config: MyPhone)          */
  "!binkversion",               /* 6: "BT/2 2.60XE/..."                */
  "!task",                      /* 7: tasknumber ("001" etc.)          */
  NULL};

//----------------------------------------------------------------------------

static UCHAR *event_names[] =
{
  "NOCARRIER",
  "CARRIER",
  "FILEERROR",
  "INTERNALERROR",
  "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
  NULL};

//----------------------------------------------------------------------------

static UCHAR *emulation_types[] =
{
  "none", "ascii", "ansi",
  NULL};

//============================================================================

// Script-Data

//----------------------------------------------------------------------------

UCHAR var_str[64][20];
UCHAR var_tim[64][20];
UCHAR var_num[64][20];

UCHAR var_str_num = 0;
UCHAR var_tim_num = 0;
UCHAR var_num_num = 0;

INT labels, labelsmax;

struct labelstr
{
  INT32 address;
  UCHAR name[20];
}
 *label;

INT jumps, jumpsmax;

struct jumpstr
{
  LONG address;
  INT num;
}
 *jump;

UCHAR compiledcommand[512];

FILE *fout;

//============================================================================

// find dummy in datalist. return position (-1=not found)

INT 
findinlist (UCHAR * dummy, UCHAR ** datalist)
{
  INT i;

  for (i = 0; datalist[i]; i++)
    if (!strcmp (datalist[i], dummy))
      return (i);
  return (-1);
}

//----------------------------------------------------------------------------

// calculates variable-id (number in list + type (str/ro-str/num/timer)

INT 
variable_number (UCHAR * this)
{
  INT i;

  switch (this[0])
  {
  case '!':
    if ((i = findinlist (this, var_str_ro)) != -1)
      return (i | 128);
    break;
  case '$':
    for (i = 0; i < var_str_num; i++)
      if (!strcmp (this, var_str[i]))
        return (i | 128 | 64);
    break;
  case '%':
    for (i = 0; i < var_num_num; i++)
      if (!strcmp (this, var_num[i]))
        return (i);
    break;
  case '&':
    for (i = 0; i < var_tim_num; i++)
      if (!strcmp (this, var_tim[i]))
        return (i | 64);
    break;
  }                             /* endswitch */
  return (-1);
}

//----------------------------------------------------------------------------

// adds variable listed in readpart to variable-list

UCHAR 
add_variable ()
{
  INT i;

  if (variable_number (readpart) != -1)
    return (2);
  switch (readpart[0])
  {
  case '$':                    // string variables

    for (i = 0; i < 64; i++)
    {
      if (var_str[i][0] == '\0')
      {
        strncpy (var_str[i], readpart, STRLEN);
        if (!(var_str_num > i))
          var_str_num = i + 1;
        return (0);
      }                         /* endif */
    }                           /* endfor */
    break;
  case '&':                    // timer variables

    for (i = 0; i < 64; i++)
    {
      if (var_tim[i][0] == '\0')
      {
        strncpy (var_tim[i], readpart, STRLEN);
        if (!(var_tim_num > i))
          var_tim_num = i + 1;
        return (0);
      }                         /* endif */
    }                           /* endfor */
    break;
  case '%':                    // numerical variables

    for (i = 0; i < 64; i++)
    {
      if (var_num[i][0] == '\0')
      {
        strncpy (var_num[i], readpart, STRLEN);
        if (!(var_num_num > i))
          var_num_num = i + 1;
        return (0);
      }                         /* endif */
    }                           /* endfor */
    break;
  }                             /* endswitch */
  return (1);
}

//----------------------------------------------------------------------------

UCHAR *
varname_addr (UCHAR i)
{
  switch (i & (128 + 64))
  {
  case 0:
    return (&(var_num[i & (1 + 2 + 4 + 8 + 16 + 32)][0]));
    break;
  case 64:
    return (&(var_tim[i & (1 + 2 + 4 + 8 + 16 + 32)][0]));
    break;
  case 64 + 128:
    return (&(var_str[i & (1 + 2 + 4 + 8 + 16 + 32)][0]));
    break;
  case 128:
    return (&(var_str_ro[i & (1 + 2 + 4 + 8 + 16 + 32)][0]));
    break;
  }                             /* endswitch */
  return (NULL);
}                               /* end varname_addr */

//============================================================================

INT 
compile_label ()
{
  INT i;

  readgetpart ('u');
  if (readpart[0] == ':')
    for (i = 0; i < strlen (readpart); i++)
      readpart[i] = readpart[i + 1];
  i = 0;
  while ((i < labelsmax) && (strcmp (readpart, label[i].name)))
    i++;
  if (i < labelsmax)
  {
    jump[jumps].address = ftell (fout) + compiledcommand[1];
    jump[jumps].num = i;
    jumps++;
    for (i = 0; i < 4; i++)
      compiledcommand[compiledcommand[1]++] = 0xFF;  // reserve space for address

    return (OK);
  }
  else
  {
    strcpy (errormessage, "Label not found");
    return (ERROR);
  }                             /* endif */
}

//----------------------------------------------------------------------------

INT 
compile_char (UCHAR * chars, UCHAR lower)
{
  INT i;

  readgetpart (0);
  if (lower)
    readpart[0] = tolower (readpart[0]);
  if ((readpart[0] == 0) || (readpart[1] != 0))
  {
    sprintf (errormessage, "Single character [%s] required", chars);
    return (ERROR);
  }                             /* endif */
  if (chars != NULL)
  {
    for (i = 0; i < strlen (chars); i++)
      if (chars[i] == readpart[0])
      {
        compiledcommand[compiledcommand[1]++] = chars[i];
        return (OK);
      }                         /* endif */
  }
  else
  {
    compiledcommand[compiledcommand[1]++] = readpart[0];
    return (OK);
  }                             /* endif */
  strcpy (errormessage, "Invalid character");
  return (ERROR);
}

//----------------------------------------------------------------------------

INT 
compile_number ()
{
  LONG n;

  readgetpart (0);
  if (!sscanf (readpart, "%li", &n))
  {
    strcpy (errormessage, "Numerical value required");
    return (ERROR);
  }                             /* endif */
  memcpy (&(compiledcommand[compiledcommand[1]]), &n, sizeof (LONG));
  compiledcommand[1] += sizeof (LONG);
  return (OK);
}

//----------------------------------------------------------------------------

INT 
compile_variable (UCHAR allowed)
{
  INT i;

  readgetpart ('u');
  if ((i = variable_number (readpart)) == -1)
  {
    strcpy (errormessage, "Unknown variable");
    return (ERROR);
  }                             /* endif */
  if ((((i & (64 + 128)) == 0) && (allowed & 1)) ||  // num
       (((i & (64 + 128)) == 64) && (allowed & 2)) ||  // timer
       (((i & (64 + 128)) == 128) && (allowed & 4)) ||  // predef str
       (((i & (64 + 128)) == 64 + 128) && (allowed & 8)))
  {                             // user str

    compiledcommand[compiledcommand[1]++] = (UCHAR) i;
    return (OK);
  }
  else
  {
    strcpy (errormessage, "Variable of this type not allowed here");
    return (ERROR);
  }                             /* endif */
}

//----------------------------------------------------------------------------

INT 
compile_string ()
{
  INT lenpos, i, j, varnum;
  UCHAR dummy[STRLEN], varname[STRLEN];
  INT dummy1, dummy2;

  readgetpart (0);
  lenpos = compiledcommand[1]++;
  compiledcommand[lenpos] = 0;

  for (i = 0; i < strlen (readpart); i++)
  {

    if (readpart[i] != '{')
    {
      compiledcommand[compiledcommand[1]++] = 0;
      compiledcommand[compiledcommand[1]++] = readpart[i];
      compiledcommand[lenpos] += 2;
    }
    else
    {

      for (j = 0, i++; (i < strlen (readpart)) && (readpart[i] != '}'); i++, j++)
        dummy[j] = toupper (readpart[i]);
      dummy[j] = 0;
      if (readpart[i] != '}')
      {
        strcpy (errormessage, "Missing \"}\"");
        return (ERROR);
      }                         /* endif */

      for (j = 0; ((dummy[j]) && (dummy[j] != ':')); j++)
        varname[j] = dummy[j];
      varname[j] = 0;

      if ((varnum = variable_number (varname)) != -1)
      {

        if (dummy[strlen (varname_addr (varnum))] == ':')
        {                       // fill-length

          if (sscanf (&dummy[strlen (varname_addr (varnum)) + 1], "%d", &dummy1) != 1)
          {
            strcpy (errormessage, "Wrong parameter for variable fill-length");
            return (ERROR);
          }                     /* endif */
          for (j = 0; j <= strlen (varname_addr (varnum)); j++)
            dummy[j] = ' ';
          for (j = j; (j < strlen (dummy)) && (dummy[j] != ':'); j++)
            dummy[j] = ' ';
          stripleadingblanks (dummy);
          if (dummy[0] != 0)
          {
            if (dummy[0] == ':')
              if (sscanf (&dummy[1], "%d", &dummy2) != 1)
              {
                strcpy (errormessage, "Wrong parameter for variable fill-char");
                return (ERROR);
              }                 /* endif */
          }
          else
            dummy2 = ' ';       /* endif */
        }
        else
        {
          dummy1 = 0;
          dummy2 = ' ';
        }                       /* endif */
        compiledcommand[compiledcommand[1]++] = 1;
        compiledcommand[compiledcommand[1]++] = varnum;
        compiledcommand[compiledcommand[1]++] = dummy1;
        compiledcommand[compiledcommand[1]++] = dummy2;
        compiledcommand[lenpos] += 4;
      }
      else
      {
        switch (tolower (dummy[0]))
        {
        case 'x':
        case 'n':
        case 's':
        case 'r':
          compiledcommand[compiledcommand[1]++] = tolower (dummy[0]);
          compiledcommand[lenpos]++;
          break;
        case 'm':
          if (sscanf (&dummy[1], "%d,%d", &dummy1, &dummy2) != 2)
          {
            strcpy (errormessage, "Wrong m-command in string");
            return (ERROR);
          }                     /* endif */
          compiledcommand[compiledcommand[1]++] = tolower (dummy[0]);
          compiledcommand[compiledcommand[1]++] = dummy1 - 1;
          compiledcommand[compiledcommand[1]++] = dummy2 - 1;
          compiledcommand[lenpos] += 3;
          break;
        case 'c':              /* change color */
          if (dummy[1] == ':')
            dummy[1] = '0';
          if (sscanf (&dummy[1], "%x", &dummy1) != 1)
          {
            strcpy (errormessage, "Wrong parameter for color-change in string");
            return (ERROR);
          }                     /* endif */
          compiledcommand[compiledcommand[1]++] = tolower (dummy[0]);
          compiledcommand[compiledcommand[1]++] = dummy1;
          compiledcommand[lenpos] += 2;
          break;
        case '!':              /* move down */
        case '^':              /* move up */
        case '<':              /* move left */
        case '>':              /* move right */
          if (dummy[1] == ':')
            if (sscanf (&dummy[2], "%d", &dummy1) != 1)
            {
              strcpy (errormessage, "Wrong parameter for cursor-move in string");
              return (ERROR);
            }                   /* endif */
          compiledcommand[compiledcommand[1]++] = tolower (dummy[0]);
          compiledcommand[compiledcommand[1]++] = dummy1;
          compiledcommand[lenpos] += 2;
          break;
        default:
          if (sscanf (dummy, "%d", &dummy1) == 1)
          {
            compiledcommand[compiledcommand[1]++] = 0;
            compiledcommand[compiledcommand[1]++] = dummy1;
            compiledcommand[lenpos] += 2;
          }
          else
          {
            sprintf (errormessage, "Unknown string-command [%s]", dummy);
            return (ERROR);
          }                     /* endif */
          break;
        }                       /* endswitch */
      }                         /* endif */
    }                           /* endif */

  }                             /* endfor */

  return (OK);

}

//============================================================================

UINT errors = 0;                // count errors

void 
mainerror ()
{
  if (!errors)
    printf ("\n");
  printf ("\n%s/%i: %s!", name, line, errormessage);
  errors++;
}

void 
errornow (UCHAR * x)
{
  if (!errors)
    printf ("\n");
  printf ("\n%s/%i: %s!", name, line, x);
  errors++;
}

//----------------------------------------------------------------------------

void 
main (INT argc, UCHAR * argv[], UCHAR * envp[])
{
  UCHAR r;                      // return-code for reading files

  INT i;
  INT shortifmode = 0;          // ifstr/ifnum with command (not label)

  ULONG shortiflabelpos, dummy;
  UCHAR outfilename[STRLEN];

  printf ("\nBinkley BBS Script Compiler\n\n");

  // check parameters

  if ((argc != 3) && (argc != 2))
  {
    printf ("Required parameters: <file to compile> [output-file]\n\n");
    return;
  }                             /* endif */
  strncpy (readmainname, argv[1], STRLEN);
  if (argc == 3)
  {
    strncpy (outfilename, argv[2], STRLEN);
  }
  else
  {
    strcpy (outfilename, readmainname);
  }                             /* endif */

  // add ".bcs" and ".bs"

  for (i = 0; readmainname[i]; i++)
    if (readmainname[i] == '.')
      break;
  if (readmainname[i] != '.')
    strcat (readmainname, ".bs");
  for (i = 0; outfilename[i]; i++)
    if (outfilename[i] == '.')
      break;
  if (outfilename[i] != '.')
  {
    strcat (outfilename, ".bcs");
  }
  else
  {
    if (strcmp (readmainname, outfilename))
    {
      outfilename[i] = 0;
      strcat (outfilename, ".bcs");
    }                           /* endif */
  }                             /* endif */

  // write output-file

  if ((fout = fopen (outfilename, "wb")) == NULL)
  {
    printf ("Could not write %s!\n", outfilename);
    return;
  }                             /* endif */

  // count labels

  if ((r = readstart ()) == ERROR)
  {
    mainerror ();
    fclose (fout);
    remove (outfilename);
    printf ("\n");
    return;
  }                             /* endif */
  labels = 0;
  jumps = 0;
  printf ("reading lines... %5i\b\b\b\b\b", statistics.totallines);
  while (((r = readnextline ()) == OK) && (errors < MAXERRORS))
  {
    if (readline[0] == ':')
      labels++;
    readgetpart ('l');
    switch (findinlist (readpart, funcs))
    {
    case 0x00:                 // goto

    case 0x01:                 // ifstr

    case 0x02:                 // ifnum

    case 0x03:                 // event

    case 0x1d:                 // ifcarrier

    case 0x1e:                 // call

      jumps++;
    }                           /* endswitch */
    printf ("%5i\b\b\b\b\b", statistics.totallines);
  }                             /* endwhile */
  if (r == ERROR)
  {
    mainerror ();
  }
  readend ();
  if (errors)
  {
    fclose (fout);
    remove (outfilename);
    printf ("\n");
    return;
  }
  labelsmax = labels;
  jumpsmax = jumps;

  // print statistics

  printf ("\rtotal lines........: %5i\n", statistics.totallines);
  printf ("lines to compile...: %5i\n", statistics.totallinescompiled);
  printf ("files to compile...: %5i\n", statistics.totalfiles);
  printf ("labels.............: %5i\n", labelsmax);
  printf ("jumps..............: %5i\n\n", jumpsmax);
  statisticstotal = statistics;

  // allocate memory for labels and jumps

  if ((label = calloc (sizeof (struct labelstr), labelsmax + 1)) == NULL)
  {
    printf ("\nNot enough memory for labels!\n");
    return;
  }                             /* endif */
  if ((jump = calloc (sizeof (struct jumpstr), jumpsmax + 1)) == NULL)
  {
    printf ("\nNot enough memory for jumps!\n");
    return;
  }                             /* endif */

  // save labels

  printf ("reading labels.....:     0%%");
  if ((r = readstart ()) == ERROR)
  {
    mainerror ();
  }
  labels = 0;
  jumps = 0;
  while ((r = readnextline ()) == OK)
  {
    if (readline[0] == ':')
    {
      readline[0] = ' ';
      readgetpart ('u');
      strcpy (label[labels].name, readpart);
      readgetpart (0);
      if (readpart[0] != 0)
      {
        errornow ("Labels do not have parameters");
      }                         /* endif */
      labels++;
      printf ("\b\b\b\b\b\b%5i%%", (labels * 100) / labelsmax);
    }                           /* endif */
  }                             /* endwhile */
  if (r == ERROR)
  {
    mainerror ();
  }
  readend ();
  if (errors)
  {
    fclose (fout);
    remove (outfilename);
    printf ("\n");
    return;
  }

  // compile

  printf ("\ncompiling..........:     0%%");
  if ((r = readstart ()) == ERROR)
  {
    mainerror ();
  }
  labels = 0;
  jumps = 0;
  while (((r = readnextline ()) == OK) && (errors < MAXERRORS))
  {

    switch (readline[0])
    {
    case '$':
    case '%':
    case '&':
      readgetpart ('u');
      switch (add_variable ())
      {
      case 1:
        errornow ("Too many variables");
      case 2:
        errornow ("Variable already defined");
      }                         /* endswitch */
      readgetpart (0);
      if (readpart[0])
      {
        errornow ("Variable definitions have no parameters");
      }
      break;
    case '!':
      errornow ("Can not define pre-defined variable");
      break;
    case '-':
      readline[0] = ' ';
      readgetpart ('u');
      if ((i = (variable_number (readpart))) != -1)
      {
        switch (i & (128 + 64))
        {
        case 0:
          var_num[i & (1 + 2 + 4 + 8 + 16 + 32)][0] = '\0';
          break;
        case 64:
          var_tim[i & (1 + 2 + 4 + 8 + 16 + 32)][0] = '\0';
          break;
        case 128 + 64:
          var_str[i & (1 + 2 + 4 + 8 + 16 + 32)][0] = '\0';
          break;
        }                       /* endswitch */
      }
      else
      {
        errornow ("Can not undefine not defined variable");
      }                         /* endif */
      break;
    case ':':
      readline[0] = ' ';
      readgetpart (0);
      label[labels].address = ftell (fout);
      labels++;
      readgetpart (0);
      if (readpart[0])
      {
        errornow ("Labels do not have parameters");
      }                         /* endif */
      break;
    default:
    again:                     // for commands which may have another command as parameter (if*)
      readgetpart ('l');
      compiledcommand[1] = 2;
      switch (compiledcommand[0] = findinlist (readpart, funcs))
      {
      case 0x00:               // goto <label>

        if (compile_label () == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x01:               // ifstr <str*> <char> <str*> <label>

      case 0x02:               // ifnum <str*> <char> <str*> <label>

        if (shortifmode)
        {
          errornow ("Only one ifstr/ifnum/ifcarrier per line allowed");
        }
        if (compile_string () == ERROR)
        {
          mainerror ();
        }
        if (compiledcommand[0] == 0x01)
        {
          if (compile_char ("=~:!iI", 0) == ERROR)
          {
            mainerror ();
          }
        }
        else
        {
          if (compile_char ("=!<>[]", 0) == ERROR)
          {
            mainerror ();
          }
        }                       /* endif */
        if (compile_string () == ERROR)
        {
          mainerror ();
        }
        stripleadingblanks (readline);
        if (readline[0] == 0)
        {
          errornow ("Label or command required");
        }
        if (readline[0] == ':')
        {
          if (compile_label () == ERROR)
          {
            mainerror ();
          }
        }
        else
        {
          shortifmode = 1;
          shortiflabelpos = ftell (fout) + compiledcommand[1];
          compiledcommand[1] += 4;  // reserve space for address

          fwrite (&compiledcommand, compiledcommand[1], 1, fout);
          goto again;
        }                       /* endif */
        break;
      case 0x1D:               // ifcarrier

        if (shortifmode)
        {
          errornow ("Only one ifstr/ifnum/ifcarrier per line allowed");
        }
        stripleadingblanks (readline);
        if (readline[0] == ':')
        {
          if (compile_label () == ERROR)
          {
            mainerror ();
          }
        }
        else
        {
          shortifmode = 1;
          shortiflabelpos = ftell (fout) + compiledcommand[1];
          compiledcommand[1] += 4;  // reserve space for address

          fwrite (&compiledcommand, compiledcommand[1], 1, fout);
          goto again;
        }                       /* endif */
        break;
      case 0x03:               // event <eventname/timer> [<g/c> <label>]

        readgetpart ('u');
        compiledcommand[compiledcommand[1]] = findinlist (readpart, event_names);
        if (compiledcommand[compiledcommand[1]] == 0xff)
        {
          compiledcommand[compiledcommand[1]] = variable_number (readpart);
          if ((compiledcommand[compiledcommand[1]] & (128 + 64)) != 64)
          {
            errornow ("Unknown event/timer variable");
          }                     /* endif */
        }                       /* endif */
        compiledcommand[1]++;
        stripleadingblanks (readline);
        if (readline[0])
        {
          if (compile_char ("gc", 1) == ERROR)
          {
            mainerror ();
          }
          if (compile_label () == ERROR)
          {
            mainerror ();
          }
        }
        else
        {
          compiledcommand[compiledcommand[1]++] = 'g';
          compiledcommand[compiledcommand[1]++] = 0xff;  // -1: go nowhere

          compiledcommand[compiledcommand[1]++] = 0xff;
          compiledcommand[compiledcommand[1]++] = 0xff;
          compiledcommand[compiledcommand[1]++] = 0xff;
        }                       /* endif */
        break;
      case 0x04:               // loadtranslation <num>

        if (compile_number ())
        {
          mainerror ();
        }
        break;
      case 0x05:               // logfile <str*>

        if (compile_string () == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x06:               // setvar <var> <str*>

        if (compile_variable (1 + 2 + 8) == ERROR)
        {
          mainerror ();
        }
        if (compile_string () == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x07:               // addtohistory <sssnnnnnnn>

        for (i = 0; i < 10; i++)
          if (compile_string () == ERROR)
          {
            mainerror ();
          }
        break;
      case 0x08:               // print <str>

        if (compile_string () == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x09:               // getkey <var>

      case 0x0A:               // getkeysysop <var>

      case 0x0B:               // getkeymodem <var>

        if (compile_variable (1 + 2 + 8) == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x0C:               // getstr <var> <char> [length] [ULSDAPH]

        if (compile_variable (1 + 2 + 8) == ERROR)
        {
          mainerror ();
        }
        if (compile_char (NULL, 0))
        {
          mainerror ();
        }
        if (compile_number ())
        {
          mainerror ();
        }
        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x0D:               // parse <numvar> <string>

        if (compile_variable (1 + 2) == ERROR)
        {
          mainerror ();
        }
        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x0E:               // fileopen <str>

        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x0F:               // fileclose

        break;
      case 0x10:               // fileposition <numvar>

      case 0x11:               // filegoto <numvar>

        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x12:               // filetruncate

        break;
      case 0x13:               // filelineread <strvar>

        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x14:               // filelineinsert <str>

      case 0x15:               // filelinereplace <str>

        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x016:              // filelineremove <num>

        if (compile_number ())
        {
          mainerror ();
        }
        break;
      case 0x17:               // fullscreen

      case 0x18:               // unattended

        break;
      case 0x19:               // timesetonkey <timer> <num>

        if (compile_variable (2))
        {
          mainerror ();
        }
        if (compile_number ())
        {
          mainerror ();
        }
        break;
      case 0x1A:               // abort

        break;
      case 0x1B:               // emulation <type: none/ascii/ansi>

        readgetpart ('l');
        compiledcommand[compiledcommand[1]] = findinlist (readpart, emulation_types);
        if (compiledcommand[1] == 0xff)
        {
          errornow ("Unknown emulation-type");
        }
        compiledcommand[1]++;
        break;
      case 0x1C:               // dropcarrier

        break;
        // 0x1D: ifcarrier <label>: see above (below ifstr/ifnum)
      case 0x1E:               // call <label>

        if (compile_label () == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x1F:               // return

      case 0x20:               // releasetime

      case 0x21:               // waitforallsent

        break;
      case 0x22:               // getfileinfo <nv><sv><nv><nv><nv><sv><str>

        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x23:               // getfileinfotext <strvar>

      case 0x24:               // getfileinfosearch <strvar>

        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        break;
      case 0x25:               // setvardate <numvar> <strvar> <str>

        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x26:               // sendfile     <numvar><numvar><strvar><str>

      case 0x27:               // sendfilelist <numvar><numvar><strvar><str>

        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x28:               // receivefile <numvar><numvar><strvar><strvar><str>

        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (1) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        if (compile_variable (8) == ERROR)
        {
          mainerror ();
        }
        if (compile_string ())
        {
          mainerror ();
        }
        break;
      case 0x29:               // deletefile <str>

        if (compile_string ())
        {
          mainerror ();
        }
        break;
      default:
        errornow ("Unknown command");
        break;
      }                         /* endswitch */
      readgetpart (0);
      if (readpart[0])
      {
        errornow ("Too many parameters");
      }
      // write command
      if (!errors)
        fwrite (&compiledcommand, compiledcommand[1], 1, fout);
      // if we had an if with command as parameter
      if (shortifmode)
      {
        shortifmode = 0;
        dummy = ftell (fout);
        fseek (fout, shortiflabelpos, SEEK_SET);
        fwrite (&dummy, 4, 1, fout);
        fseek (fout, dummy, SEEK_SET);
      }                         /* endif */
      break;
    }                           /* endswitch */
    if (!errors)
    {
      printf ("\b\b\b\b\b\b%5i%%", (statistics.totallinescompiled * 100) / statisticstotal.totallinescompiled);
    }                           /* endif */
  }                             /* endwhile */

  readend ();

  if (errors)
  {

    fclose (fout);
    remove (outfilename);
    printf ("\n\nErrors detected.\n\n");

  }
  else
  {

    // set jump-addresses

    printf ("\nsetting jumps......:     0%%");
    jumpsmax = jumps;
    for (jumps = 0; jumps < jumpsmax; jumps++)
    {
      fseek (fout, jump[jumps].address, SEEK_SET);
      fwrite (&(label[jump[jumps].num].address), 4, 1, fout);
      printf ("\b\b\b\b\b\b%5i%%", (jumps * 100 + 100) / (jumpsmax));
    }                           /* endfor */

    fclose (fout);
    printf ("\n\nCompleted.\n\n");

  }                             /* endif */

  // exit programm

}

//============================================================================
