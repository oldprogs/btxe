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
 * Filename    : $Source: E:/cvs/btxe/src/evtparse.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:26 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Scheduler Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

char *start_time (BINK_EVENT *, char *);
char *end_time (BINK_EVENT *, char *);
char *eventname (BINK_EVENT *, char *);

char *
start_time (BINK_EVENTP e, char *p)
{
  int n, hr, mi, mo, dy;

  /* Skip blanks to get to the start-time field */
  p = skip_blanks (p);

  if ((n = sscanf (p, "%d:%d,%d,%d", &hr, &mi, &mo, &dy)) < 2)
  {
    return NULL;
  }

  e->minute = hr * 60 + mi;
  if ((e->minute < 0) || (e->minute > (24 * 60)))
  {
    return (NULL);
  }

  if (n >= 3)
    e->month = (char) mo;
  if (n >= 4)
    e->day = (char) dy;

  p = skip_to_blank (p);

  return (p);
}

char *
end_time (BINK_EVENTP e, char *p)
{
  int hh, mm, k;

  if (sscanf (p, "%d:%d", &hh, &mm) != 2)
  {
    return NULL;
  }

  k = hh * 60 + mm;
  if ((k > (24 * 60)) || (k < 0))
  {
    return (NULL);
  }

  if (k < e->minute)
  {
    printf (MSG_TXT (M_NO_END_MIDNIGHT));
    return (NULL);
  }

  e->length = k - e->minute;

  p = skip_to_blank (p);

  return (p);
}

char *
days (BINK_EVENTP e, char *p)
{
  e->days = 0;

  /* Skip blanks to get to the days field */
  p = skip_blanks (p);

  while ((*p) && (!isspace (*p)))
  {
    if (!strnicmp (p, "all", 3))
    {
      e->days |= (DAY_WEEK | DAY_WKEND);
      p += 3;
    }
    else if (!strnicmp (p, "mon", 3))
    {
      e->days |= DAY_MONDAY;
      p += 3;
    }
    else if (!strnicmp (p, "tue", 3))
    {
      e->days |= DAY_TUESDAY;
      p += 3;
    }
    else if (!strnicmp (p, "wed", 3))
    {
      e->days |= DAY_WEDNESDAY;
      p += 3;
    }
    else if (!strnicmp (p, "thu", 3))
    {
      e->days |= DAY_THURSDAY;
      p += 3;
    }
    else if (!strnicmp (p, "fri", 3))
    {
      e->days |= DAY_FRIDAY;
      p += 3;
    }
    else if (!strnicmp (p, "sat", 3))
    {
      e->days |= DAY_SATURDAY;
      p += 3;
    }
    else if (!strnicmp (p, "sun", 3))
    {
      e->days |= DAY_SUNDAY;
      p += 3;
    }
    else if (!strnicmp (p, "week", 4))
    {
      e->days |= DAY_WEEK;
      p += 4;
    }
    else if (!strnicmp (p, "wkend", 5))
    {
      e->days |= DAY_WKEND;
      p += 5;
    }
    else                        /* error condition */
      return NULL;

    if (*p == '|')
      ++p;
  }
  return p;
}

char *
eventflags (BINK_EVENTP e, char *p, char **eptr)
{
  int i, j, j1, j2;
  char *p1;

  /* Give each event a default of 60 minutes */
  e->length = 60;

  /* Give each event a default of T=3,10000 */
  e->with_connect = 3;
  e->no_connect = 10000;

  e->behavior = 0;
  e->extramask = 0;

  /* Give each event a local cost of 0 */
  e->node_cost = 0;

  /* Each event defaults to Q=0 */
  e->mailqsize = 0l;

  e->wait_time = 120;

  /* While there are still things on the line */
  while (*p)
  {
    /* Skip blanks to get to the next field */
    p = skip_blanks (p);

    /* switch to find what thing is being parsed */

    switch (tolower (*p))
    {
    case '\0':                 /* No more stuff */
      break;

    case '0':                  /* Digits must be an ending time */
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      /* Parse ending time */
      if ((p = end_time (e, p)) == NULL)
      {
        *eptr = MSG_TXT (M_INVALID_END);
        goto bad_line;
      }
      break;

    case ';':                  /* Comment */
    case '%':
      *p = '\0';
      break;

    case '"':                  /* Extra chars to append to packer strings */
      ++p;
      p1 = e->cmd;
      *p1++ = ' ';
      while (*p != '"')
        *p1++ = *p++;
      *p1 = '\0';
      ++p;
      break;

    case 'a':                  /* Average wait */
      ++p;
      if (*p == '=')
      {
        ++p;
        if (isdigit (*p))
        {
          i = atoi (p);

          /*
           * Original code: if ((i > 1800) || (i < 0))
           * r.hoerner:
           * 'cause timerset() allows only up to 65535 1/100 secs a value of
           * 1800 seconds will result in an overflow. The maximum possible
           * value with timerset() is 655*PER_SECOND. Search mailer.c and
           * mailroot.c for "random_time()".
           */

          if ((i > 655) || (i < 0))
          {
            *eptr = MSG_TXT (M_INVALID_AVGWAIT);
            goto bad_line;
          }

          e->wait_time = i;
          p = skip_to_blank (p);
          break;
        }
      }

      *eptr = MSG_TXT (M_INVALID_AVGWAIT);
      goto bad_line;

    case 'b':                  /* BBS type event */
      p = skip_to_blank (p);
      e->behavior |= MAT_BBS;
      break;

    case 'c':                  /* #CM event */
      p = skip_to_blank (p);
      e->behavior |= MAT_CM;
      break;

    case 'd':                  /* Dynamic event */
      p = skip_to_blank (p);
      e->behavior |= MAT_DYNAM;
      break;

    case 'e':                  /* An errorlevel exit */
      ++p;
      if (isdigit (*p))         /* E0 through E9 */
      {
        i = *p - '0';
        ++p;
        if (*p == '=')
        {
          if ((i <= 3) && (i > 0))
          {
            ++p;
            if (isdigit (*p))
            {
              j = atoi (p);
              e->errlevel[i - 1] = (unsigned char) j;
              p = skip_to_blank (p);
              break;
            }
          }
          else if ((i > 3) && (i <= 9))
          {
            ++p;
            if (isdigit (*p))
            {
              j = atoi (p);
              e->errlevel[i - 1] = (unsigned char) j;
              while (*p && (*p != ','))
                ++p;
              ++p;
              strncpy (&(e->err_extent[i - 4][0]), p, 3);
              p = skip_to_blank (p);
              break;
            }
          }
        }
      }
      else if (tolower (*p) == 'f')  /* Errorlevel for fax */
      {
        ++p;
        if (*p == '=')
        {
          ++p;
          if (isdigit (*p))
          {
            j = atoi (p);
            e->faxerrlevel = (unsigned char) j;
            p = skip_to_blank (p);
            break;
          }
        }
      }

      *eptr = MSG_TXT (M_BAD_ERRORLEVEL);
      goto bad_line;

    case 'f':                  /* Forced event */
      p = skip_to_blank (p);
      e->behavior |= MAT_FORCED;
      break;

    case 'h':                  /* hi-priority crash */
      p = skip_to_blank (p);
      e->behavior |= MAT_HIPRICM;
      break;

    case 'k':                  /* no #CM event */
      p = skip_to_blank (p);
      e->behavior |= MAT_NOCM;
      break;

    case 'l':                  /* Local only mail */
      ++p;
      e->node_cost = 0;
      switch (tolower (*p))
      {
      case 'i':
        ++p;
        e->extramask |= EXTR_LINDEXCOST;
        break;
      case 'c':
        ++p;
        e->extramask |= EXTR_LMINUTECOST;
        break;
      case 't':
        ++p;
        e->extramask |= EXTR_LTOTALCOST;
        break;
      default:
        *eptr = MSG_TXT (M_BAD_LEVENT);
        goto bad_line;
      }

      switch (*p)
      {
      case '=':
        ++p;
        e->extramask |= EXTR_LEQUALS;
        if (isdigit (*p))
          e->node_cost = atoi (p);
        break;
      case '>':
        ++p;
        e->extramask |= EXTR_LGREATER;
        if (isdigit (*p))
          e->node_cost = atoi (p);
        break;
      case '<':
        ++p;
        e->extramask |= EXTR_LLESS;
        if (isdigit (*p))
          e->node_cost = atoi (p);
        break;
      default:
        *eptr = MSG_TXT (M_BAD_LEVENT);
        goto bad_line;
      }
      p = skip_to_blank (p);
      e->behavior |= MAT_LOCAL;
      break;

    case 'm':                  /* send to CM and !CM */
      p = skip_to_blank (p);
      e->behavior |= MAT_NOMAIL24;
      break;

    case 'n':                  /* No requests */
      p = skip_to_blank (p);
      e->behavior |= MAT_NOREQ;
      break;

    case 'p':
      p = skip_to_blank (p);
      e->behavior |= MAT_NOPICKUP;
      break;

    case 'q':                  /* Mail Queued size */
      ++p;
      if (*p == '=')
      {
        ++p;
        if (isdigit (*p))
        {
          e->mailqsize = (unsigned long) atol (p);
          p = skip_to_blank (p);
          break;
        }
      }
      *eptr = MSG_TXT (M_BAD_QSIZE);
      goto bad_line;

    case 'r':                  /* Receive only */
      p = skip_to_blank (p);
      e->behavior |= MAT_NOOUT;
      break;

    case 's':                  /* Send only */
      p = skip_to_blank (p);
      e->behavior |= MAT_OUTONLY;
      break;

    case 't':                  /* Tries */
      ++p;
      if (sscanf (p, "=%d,%d", &j1, &j2) != 2)
      {
        *eptr = MSG_TXT (M_BAD_TRIES);
        goto bad_line;
      }
      else
      {
        if ((j1 > 8) || (j1 < 1))
        {
          *eptr = MSG_TXT (M_BAD_TRIES);
          goto bad_line;
        }
        e->with_connect = j1;
        e->no_connect = j2;
      }
      p = skip_to_blank (p);
      break;

    case 'x':                  /* No outbound requests here */
      p = skip_to_blank (p);
      e->behavior |= MAT_NOOUTREQ;
      break;

    case 'y':                  /* no sounds in this event */
      p = skip_to_blank (p);
      e->extramask |= EXTR_NOSOUND;
      break;

    case '$':
      p = skip_to_blank (p);
      e->behavior |= MAT_KILLBAD;
      break;

    case '!':                  /* TJW 960721 Cost event definition */
      ++p;
      if (*p == '=')
      {
        e->behavior = MAT_COSTEVENT;  /* can have ONLY costs in this! */
        /* CostVector[i] 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16 */
        if (sscanf (++p, "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,"
          "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
        /* 17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32 */
                    &e->costvector[0], &e->costvector[1],
                    &e->costvector[2], &e->costvector[3],
                    &e->costvector[4], &e->costvector[5],
                    &e->costvector[6], &e->costvector[7],
                    &e->costvector[8], &e->costvector[9],
                    &e->costvector[10], &e->costvector[11],
                    &e->costvector[12], &e->costvector[13],
                    &e->costvector[14], &e->costvector[15],
                    &e->costvector[16], &e->costvector[17],
                    &e->costvector[18], &e->costvector[19],
                    &e->costvector[20], &e->costvector[21],
                    &e->costvector[22], &e->costvector[23],
                    &e->costvector[24], &e->costvector[25],
                    &e->costvector[26], &e->costvector[27],
                    &e->costvector[28], &e->costvector[29],
                    &e->costvector[30], &e->costvector[31]) < 1)
        {
          *eptr = MSG_TXT (M_INVALID_COSTEVENT);
          goto bad_line;
        }
        p = skip_to_blank (p);
        break;
      }
      *eptr = MSG_TXT (M_INVALID_COSTEVENT);
      goto bad_line;

    default:                   /* Error condition */
      *eptr = MSG_TXT (M_INDECIPHERABLE);
      printf ("Error: '%c'\n", tolower (*p));
      goto bad_line;
    }
  }
  return p;

bad_line:
  return NULL;
}

char *
eventname (BINK_EVENTP e, char *p)
{
  p = skip_blanks (p);
  if (*p == '"')                /* is optional event name field "name" present ? */
  {
    char *start = ++p;

    p = strchr (start, '"');
    if (p != NULL)              /* end of event name found ? */
    {
      *p = '\0';
      strncpy (e->ename, start, 32);  /* copy first 32 chars  */
      e->ename[31] = '\0';      /* ensure 0 termination */
      *p++ = '"';
    }
  }
  return p;
}

KWD_ERR
parse_event (char *e_line)
{
  char *p, *eptr;
  BINK_EVENT e;

  /* If we already have a schedule, then forget it */
  if (got_sched)
    return kwd_ok;

  if (num_events > 254)         /* r. hoerner. See reason in b_initva.c */
    return kwd_ok;

  /* Zero out the event structure */

  memset ((char *) &e, 0, sizeof (e));

  p = e_line;

  /* TJW 961104 parse optional event name field */
  if ((p = eventname (&e, p)) == NULL)
  {
    eptr = MSG_TXT (M_BAD_EVENTNAME);
    goto bad_line;
  }

  /* Parse the days field */
  if (((p = days (&e, p)) == NULL) || (e.days == 0))
  {
    eptr = MSG_TXT (M_BAD_DAY);
    goto bad_line;
  }

  /* Parse the start-time field */
  if ((p = start_time (&e, p)) == NULL)
  {
    eptr = MSG_TXT (M_INVALID_START);
    goto bad_line;
  }

  /* Parse the end-time and flags field */
  if ((p = eventflags (&e, p, &eptr)) == NULL)
  {
    goto bad_line;
  }

  /* Save it in the array  of pointers */
  e_ptrs[num_events++] = e;

  /* Return that everything is cool */
  return kwd_ok;

bad_line:

  printf (eptr, e_line);
  return kwd_bad;
}

/* $Id: evtparse.c,v 1.3 1999/03/22 03:47:26 mr Exp $ */
