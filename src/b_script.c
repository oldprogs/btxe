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
 * Filename    : $Source: E:/cvs/btxe/src/b_script.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:14 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Script Handler Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static int LOCALFUNC nextline (char *);
static int LOCALFUNC get_line (void);

/*--------------------------------------------------------------------------*/
/*   Define our current script functions for use in our dispatch table.     */
/*--------------------------------------------------------------------------*/

static int LOCALFUNC script_port (void);  /* Change the current port  */
static int LOCALFUNC script_upload (void);  /* Upload files             */
static int LOCALFUNC script_download (void);  /* Download files           */
static int LOCALFUNC script_baud (void);  /* Set our baud rate to     *

                                           * that of remote           */
static int LOCALFUNC script_xmit (void);  /* transmit characters out  *

                                           * the port                 */
static int LOCALFUNC script_rawxmit (void);  /* transmit characters out  *

                                              * the port (no translation)*/
static int LOCALFUNC script_pattern (void);  /* define pattern to wait on */
static int LOCALFUNC script_wait (void);  /* wait for pattern         */
static int LOCALFUNC script_dial (void);  /* dial the whole number    */
static int LOCALFUNC script_areacode (void);  /* transmit the areacode    */
static int LOCALFUNC script_phone (void);  /* transmit the phone number */
static int LOCALFUNC script_carrier (void);  /* test for carrier         */
static int LOCALFUNC script_session (void);  /* Exit script w/"success"  */
static int LOCALFUNC script_if (void);  /* Branch on pattern match  */
static int LOCALFUNC script_goto (void);  /* Absolute branch          */
static int LOCALFUNC script_timer (void);  /* Set master script timeout */
static int LOCALFUNC script_bps100 (void);  /* Send BPS/100 to remote   */
static int LOCALFUNC script_break (void);  /* Send break to remote     */
static int LOCALFUNC script_params (void);  /* Set comm parameters      */
static int LOCALFUNC script_DOS (void);  /* Execute a DOS command    */
static int LOCALFUNC script_abort (void);  /* Abort in certain hours   */
static int LOCALFUNC script_noWaZOO (void);  /* No WaZOO this session    */
static int LOCALFUNC script_noEMSI (void);  /* No EMSI this session only */

struct dispatch
{
  char *string;
  int (LOCALFUNC * fun) (void);
};

static struct dispatch disp_table[] =
{
  {"download", script_download},
  {"upload", script_upload},
  {"baud", script_baud},
  {"xmit", script_xmit},
  {"rawxmit", script_rawxmit},
  {"pattern", script_pattern},
  {"wait", script_wait},
  {"dial", script_dial},
  {"areacode", script_areacode},
  {"phone", script_phone},
  {"carrier", script_carrier},
  {"session", script_session},
  {"if", script_if},
  {"goto", script_goto},
  {"timer", script_timer},
  {"speed", script_bps100},
  {"break", script_break},
  {"comm", script_params},
  {"dos", script_DOS},
  {"abort", script_abort},
  {"port", script_port},
  {"nowazoo", script_noWaZOO},
  {"noemsi", script_noEMSI},
  {(char *) NULL, NULL}
};

static char *script_dial_string = NULL;  /* string for 'dial'    */
static char *script_phone_string = NULL;  /* string for 'phone'   */
static char *script_areacode_string = "          ";  /* string for 'areacode' */

#define PATTERNS 9
#define PATSIZE 22

static char pattern[PATTERNS][PATSIZE];  /* 'wait' patterns      */
static int scr_session_flag = 0;  /* set by "session".    */
static int pat_matched = -1;
static char *script_function_argument;  /* function argument ptr */

#define MAX_LABELS 50
#define MAX_LAB_LEN 20
typedef struct                  /* TJW 960526 don't do this STATIC, 1.6KB mem! */
{
  char name[MAX_LAB_LEN + 1];
  long foffset;
  int line;
}
LABELENTRY;
typedef LABELENTRY LABELTAB[MAX_LABELS];

static LABELENTRY *labels = NULL;  /* TJW 960526 static -> heap */

static long offset;
static long script_alarm;       /* for master timeout   */
static int num_labels = 0;
static int curline;
static FILE *stream;

static char *temp = NULL;

int
do_script (char *phone_number)
{
  register int i, j, retval;
  register char *c, *f;
  char s[64], *t;

  /* ---------------------------------------------------------------------- */
  /* Reset everything from possible previous use of this function.          */
  /* ---------------------------------------------------------------------- */

  /* Get rid of cr/lf stuff if it is there */

  if ((c = strchr (phone_number, '\r')) != NULL)
    *c = '\0';
  if ((c = strchr (phone_number, '\n')) != NULL)
    *c = '\0';

  if (labels == NULL)           /* TJW 960526 */
    if ((labels = (LABELENTRY *) calloc (MAX_LABELS, sizeof (LABELENTRY))) == NULL)
      return (FAILURE);

  if (temp == NULL)
    if ((temp = calloc (1, 256)) == NULL)
      return (FAILURE);

  curline = 0;
  pat_matched = -1;
  num_labels = 0;
  *script_areacode_string = '\0';  /* reset the special strings */
  script_dial_string = script_phone_string = NULL;
  script_alarm = 0L;            /* reset timeout            */

  for (i = 0; i < PATTERNS; i++)
  {
    pattern[i][0] = 1;
    pattern[i][1] = '\0';       /* and the 'wait' patterns  */
  }

  scr_session_flag = 0;

  /* ---------------------------------------------------------------------- */
  /* Now start doing things with phone number:                              */
  /*     1) construct the name of the script file into temp                 */
  /*     2) build script_dial_string, script_areacode_string and            */
  /*        script_phone_string                                             */
  /* ---------------------------------------------------------------------- */

  if (script_path == NULL)
    strcpy (temp, BINKpath);    /* get our current path     */
  else
    strcpy (temp, script_path); /* Otherwise use given path */

  t = c = &temp[strlen (temp)]; /* point past paths         */
  f = phone_number;             /* then get input side      */

  while (*++f != '\"')          /* look for end of string   */
  {
    if ((*c++ = *f) == '\0')    /* if premature ending,     */
      return (FAILURE);
  }

  *c = '\0';                    /* Now we have file name    */
  strcpy (s, t);

  c = script_areacode_string;   /* point to area code       */
  if (*++f)                     /* if there's anything left */
  {
    script_dial_string = f;     /* dial string is remainder */

    for (i = 0; (i < 10) && (*f != '\0') && (*f != '-'); i++)
      *c++ = *f++;              /* copy it for 'areacode'   */
  }

  *c = '\0';                    /* terminate areacode       */
  if (*f && *f++ == '-')        /* If more, and we got '-', */
    script_phone_string = f;    /* point to phone string    */

  if (script_dial_string == NULL)  /* To make the log happy,   */
    script_dial_string = "";    /* NULL => 0-length string  */

  /* ---------------------------------------------------------------------- */
  /* Finally open the script file and start doing some WORK.                */
  /* ---------------------------------------------------------------------- */

  status_line (MSG_TXT (M_DIALING_SCRIPT), script_dial_string, s);

  /* OK, let's open the file */

  if ((stream = share_fopen (temp, read_binary, DENY_WRITE)) == NULL)
  {
    status_line (MSG_TXT (M_NOOPEN_SCRIPT), temp);
    return (FAILURE);           /* no file, no work to do   */
  }

  retval = FAILURE;             /* default return is "fail" */
  while (nextline (NULL))       /* Now we parse the file ... */
  {
    retval = FAILURE;           /* default return is "fail" */
    for (j = 0; (c = disp_table[j].string) != NULL; j++)
    {
      i = (int) strlen (c);
      if (strnicmp (temp, c, (unsigned int) i) == 0)
      {
        script_function_argument = temp + i + 1;
        retval = (*disp_table[j].fun) ();
        break;
      }
    }

    if (script_alarm && longtimeup (script_alarm))  /* Check master timer    */
    {
      status_line (MSG_TXT (M_MASTER_SCRIPT_TIMER));
      retval = FAILURE;
    }

    if (retval == FAILURE || scr_session_flag)  /* get out for failure    */
      break;                    /* or 'session'.           */

  }

  fclose (stream);              /* close input file         */

  if (retval == FAILURE)
  {
    status_line (MSG_TXT (M_SCRIPT_FAILED), s, curline);
    modem_hangup ();
  }

  free (temp);
  temp = NULL;

  free (labels);                /* TJW 960526 */
  labels = NULL;

  return (retval);              /* return success/fail      */
}

static int LOCALFUNC
script_download ()
{
  int c = toupper (*script_function_argument);

  switch (c)
  {
  case 'S':
  case 'X':
  case 'Z':
  case 'H':
    return (Download (NULL, c, NULL));

  default:
    break;
  }
  return (FAILURE);
}

static int LOCALFUNC
script_upload ()
{
  char *p = script_function_argument;
  int c = toupper (*p);

  switch (c)
  {
  case 'S':
  case 'X':
  case 'Z':
  case 'H':
    break;

  default:
    return (FAILURE);
  }

  p = skip_to_blank (p);
  p = skip_blanks (p);

  if (p == NULL || strlen (p) == 0)
    return (FAILURE);

  return (Upload (p, c, NULL));
}

static int LOCALFUNC
script_xmit ()
{
  mdm_cmd_string (script_function_argument, 1);
  return (IGNORE);
}

static int LOCALFUNC
script_rawxmit ()
{
  char *p;

  p = script_function_argument;

  while (*p)
  {
    SENDBYTE ((unsigned char) *p);
    ++p;
  }

  return (IGNORE);
}

static int LOCALFUNC
script_DOS ()
{
  vfossil_cursor (1);
  close_up (share);
  b_spawn (script_function_argument);
  come_back ();
  return (IGNORE);
}

static int LOCALFUNC
script_abort ()
{
  time_t long_time;
  struct tm *tm;

  int s1, s2, e1, e2;
  int starttime, endtime, us;

  /* If we don't get everything we need, it is a true abort */

  if (sscanf (script_function_argument, "%d:%d %d:%d", &s1, &s2, &e1, &e2) != 4)
    return (FAILURE);

  /* Get the current time into a structure */

  unix_time (&long_time);
  tm = unix_localtime (&long_time);

  starttime = s1 * 60 + s2;
  endtime = e1 * 60 + e2;
  us = tm->tm_hour * 60 + tm->tm_min;

  if (endtime < starttime)
    endtime += 60 * 60;

  if (us < starttime)
    us += 24 * 60;

  if ((us >= starttime) && (us <= endtime))
    return (FAILURE);           /* "Abort in certain hours!"  (r.hoerner) */

  return (IGNORE);
}

static int LOCALFUNC
script_break ()
{
  int t;

  t = atoi (script_function_argument);
  if (t == 0)
    t = 100;

  send_break (t);
  return (IGNORE);
}

static int LOCALFUNC
script_params ()
{
  char c;
  int i, j;

  sscanf (script_function_argument, "%d%c%d", &i, &c, &j);
  comm_bits = (i == 7) ? BITS_7 : BITS_8;
  switch (toupper (c))
  {
  case 'E':
    parity = EVEN_PARITY;
    break;

  case 'O':
    parity = ODD_PARITY;
    break;

  case 'N':
    parity = NO_PARITY;
    break;
  }

  stop_bits = (j == 1) ? STOP_1 : STOP_2;
  program_baud ();
  return (IGNORE);
}

static int LOCALFUNC
script_bps100 ()
{
  char rgchT[10];

  sprintf (rgchT, "%ld", cur_baud.rate_value / 100L);
  mdm_cmd_string (rgchT, 0);
  return (IGNORE);
}

static int LOCALFUNC
script_areacode ()
{
  mdm_cmd_string (script_areacode_string, 0);
  return (IGNORE);
}

static int LOCALFUNC
script_phone ()
{
  mdm_cmd_string (script_phone_string, 0);
  return (IGNORE);
}

static int LOCALFUNC
script_dial ()
{
  mdm_cmd_string (script_dial_string, 0);
  mdm_cmd_char (CR);            /* terminate the string     */

  if (modem_response (7500) == CONNECTED)  /* we got a good response,  */
  {
    timer (20);                 /* wait for other side      */
    return (IGNORE);            /* Carrier should be on now */
  }

  return (FAILURE);             /* no good */
}

static int LOCALFUNC
script_carrier ()
{
  return ((int) CARRIER);
}

static int LOCALFUNC
script_session ()
{
  ++scr_session_flag;           /* signal end of script     */
  return (IGNORE);
}

static int LOCALFUNC
script_pattern ()
{
  register int i, j;
  register char *c;

  c = script_function_argument; /* copy the pointer   */
  i = atoi (c);                 /* get pattern number */

  if (i < 0 || i >= PATTERNS)   /* check bounds       */
    return (FAILURE);

  c += 2;                       /* skip digit, space  */

  for (j = 1; (j <= PATSIZE) && (*c != '\0'); j++)
    pattern[i][j] = *c++;       /* store the pattern  */

  pattern[i][j] = '\0';         /* terminate it here  */
  return (IGNORE);
}

static int LOCALFUNC
script_wait ()
{
  long t1;
  long iWaitT;
  register int i, j;
  register char c;
  int cnt;
  int got_it = 0;

  pat_matched = -1;

  /* try to get wait length */

  iWaitT = atol (script_function_argument);
  if (!iWaitT)
    iWaitT = 40;                /* default is 40 seconds    */

  t1 = longtimerset (iWaitT * PER_SECOND);
  printf ("\n");
  clear_eol ();
  cnt = 0;
  while (!longtimeup (t1) && !KEYPRESS ())
  {
    if (script_alarm && longtimeup (script_alarm))  /* Check master timer */
      break;                    /* Oops, out of time...     */

    if (!CHAR_AVAIL ())         /* if nothing ready yet,    */
    {
      time_release ();          /* give others a shot       */
      continue;                 /* just process timeouts    */
    }

    t1 = longtimerset (iWaitT * PER_SECOND);  /* reset the timeout        */
    c = (char) MODEM_IN ();     /* get a character          */

    if (!c)
      continue;                 /* ignore null characters   */

    if (c >= ' ')
    {
      WRITE_ANSI ((byte) (c & 0x7f));
      if (++cnt >= (int) SB_COLS - 10)
      {
        cnt = 0;
        printf ("\r");
        clear_eol ();
        printf ("(cont): ");
      }
    }

    for (i = 0; i < PATTERNS; i++)
    {
      j = pattern[i][0];        /* points to next match char */
      if (c == pattern[i][j])   /* if it matches,           */
      {
        ++j;                    /* bump the pointer         */
        pattern[i][0] = (char) j;  /* store it                 */
        if (!pattern[i][j])     /* if at the end of pattern, */
        {
          ++got_it;
          pat_matched = i;
          goto done;
        }
      }
      else
      {
        pattern[i][0] = 1;      /* back to start of string  */
      }
    }
  }

done:

  for (i = 0; i < PATTERNS; i++)
    pattern[i][0] = 1;          /* reset these for next time */

  if (!got_it)                  /* timed out,look for label */
  {
    /* First skip over the numeric argument for "wait" */

    while (isdigit (*script_function_argument))
      script_function_argument++;

    /* Then skip over any spaces that follow it */

    script_function_argument = skip_blanks (script_function_argument);

    /* Now, if there's anything more, treat it as a goto */

    if (*script_function_argument)
      return (script_goto ());
  }

  return (got_it);
}

static int LOCALFUNC
script_baud ()
{
  unsigned long b;

  if ((b = (unsigned long) atol (script_function_argument)) != 0)
    return set_baud (b, 0);

  return (IGNORE);
}

static int LOCALFUNC
script_goto ()
{
  int i;

  /* First see if we already found this guy */

  for (i = 0; i < num_labels; i++)
  {
    if (stricmp (script_function_argument, labels[i].name) == 0)
    {                           /* We found it */
      fseek (stream, labels[i].foffset, SEEK_SET);
      curline = labels[i].line;
      return (IGNORE);
    }
  }

  return (nextline (script_function_argument));
}

static int LOCALFUNC
script_if ()
{

  /* First, move past any spaces that might be between IF and value.  */

  script_function_argument = skip_blanks (script_function_argument);

  /* Then check for digit. Only current legal non-digit is 'B' but     *
   * that might change with time...                                    *
   *                                                                   *
   * If it's a non-digit,                                              *
   *                                                                   *
   *    a) look for "BPS". If not, return error.                       *
   *                                                                   *
   *    b) compare current baud with number that should follow         *
   *       "BPS". If no match, return error.                           *
   *                                                                   *
   * If it's a digit, compare the number of the last pattern we matched*
   * with the argument value. If no match, return error.               *
   *                                                                   */

  if (!isdigit (*script_function_argument))
  {
    if (strnicmp (script_function_argument, "BPS", 3) != 0)
      return (IGNORE);

    script_function_argument += 3;
    if (atol (script_function_argument) != (long) cur_baud.rate_value)
      return (IGNORE);
  }
  else if (atoi (script_function_argument) != pat_matched)
    return (IGNORE);

  /* We matched, skip the pattern number and the space                */

  script_function_argument = skip_to_blank (script_function_argument);
  script_function_argument = skip_blanks (script_function_argument);

  return (script_goto ());
}

static int LOCALFUNC
script_timer ()                 /* Set a master timer */
{
  int i;

  /* If we got a number, set the master timer. Note: this could be
   * done many times in the script, allowing you to program timeouts
   * on individual parts of the script. */

  i = atoi (script_function_argument);
  if (i)
    script_alarm = longtimerset ((long) (i * PER_SECOND));

  return (IGNORE);
}

static int LOCALFUNC
script_port ()
{
  int c;

  c = port_ptr;
  MDM_DISABLE ();
  port_ptr = atoi (script_function_argument) - 1;

  if (Cominit (port_ptr, buftmo) != 0x1954)
  {
    port_ptr = c;
    Cominit (port_ptr, buftmo);
    return (FAILURE);
  }

  program_baud ();
  RAISE_DTR ();
  return (IGNORE);
}

static int LOCALFUNC
script_noWaZOO ()
{
  no_WaZOO_Session = 1;
  return (IGNORE);
}

static int LOCALFUNC
script_noEMSI ()
{
  no_EMSI_Session = 1;
  return (IGNORE);
}

static int LOCALFUNC
nextline (char *str)
{
  char save[256];

  if (str != NULL)
    strcpy (save, str);
  else
    save[0] = '\0';

  while (get_line ())           /* Now we parse the file ... */
  {
    if (!isalpha (temp[0]))
    {
      if (temp[0] != ':')
      {                         /* This line is a comment line */
        continue;
      }
      else
      {                         /* It is a label */
        if (num_labels >= MAX_LABELS)
        {
          status_line (MSG_TXT (M_TOO_MANY_LABELS));
          return (FAILURE);
        }

        strcpy (labels[num_labels].name, &(temp[1]));
        labels[num_labels].foffset = offset;
        labels[num_labels].line = curline;
        ++num_labels;

        if (stricmp (&temp[1], save))
          continue;
        else
          return (IGNORE);
      }
    }

    if (!save[0])
      return (IGNORE);
  }

  return (FAILURE);
}

static int LOCALFUNC
get_line ()
{
  char *c;
  char j[100];

  if (fgets (temp, 255, stream) == NULL)
    return (FAILURE);

  ++curline;

  /* Deal with side effects of opening the script file in binary mode  */

  c = &temp[strlen (temp) - 1];
  while ((*c == '\r') || (*c == '\n'))
    c--;

  *++c = '\0';                  /* Don't want newlines, terminate after text */

  sprintf (j, script_line, curline, temp);
  if (un_attended || doing_poll)
  {
    sb_move_puts (filewin, 1, 0, j);
    sb_show ();
  }
  else
    printf ("\n%s", j);

  offset = ftell (stream);
  return (IGNORE);
}

/* $Id: b_script.c,v 1.4 1999/03/22 03:47:14 mr Exp $ */
