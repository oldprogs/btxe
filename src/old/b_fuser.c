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
 * Filename    : $Source: E:/cvs/btxe/src/old/b_fuser.c,v $
 * Revision    : $Revision: 1.1 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/28 13:48:00 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Fidolist processing module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static int reclength = -1;
static int nrecs = -1;

void
fidouser (char *name, ADDRP faddr)
{
  int low, high, mid, f, cond, namelen;
  char midname[80];
  char last_name_first[80];
  char *c, *p, *m;
  struct stat buffer;

  faddr->Zone = faddr->Net = faddr->Node = faddr->Point = (unsigned short) -1;
  faddr->Domain = NULL;

  c = midname;                  /* Start of temp name buff  */
  p = name;                     /* Point to start of name   */
  m = NULL;                     /* Init pointer to space    */

  *c = *p++;
  while (*c)                    /* Go entire length of name */
  {
    if (*c == ' ')              /* Look for space           */
      m = c;                    /* Save location            */
    c++;
    *c = *p++;
  }

  if (m != NULL)                /* If we have a pointer,    */
  {
    *m++ = '\0';                /* Terminate the first half */
    (void) strcpy (last_name_first, m);  /* Now copy the last name   */
    (void) strcat (last_name_first, ", ");  /* Insert a comma and space */
    (void) strcat (last_name_first, midname);  /* Finally copy first half */
  }
  else
    (void) strcpy (last_name_first, midname);  /* Use whole name otherwise */

  (void) fancy_str (last_name_first);  /* Get caps in where needed */
  namelen = (int) strlen (last_name_first);  /* Calc length now          */

  midname[0] = '\0';            /* "null-terminated string" */
  (void) strcpy (midname, net_info);  /* take nodelist path       */
  (void) strcat (midname, "FIDOUSER.LST");  /* add in the file name     */

  if ((f = share_open (midname, O_RDONLY | O_BINARY, DENY_WRITE)) == -1)
  {
    reclength = -1;             /* Reset all on open failure */
    return;
  }

  /* Find out if we have done this before */

  if (reclength == -1)
  {
    /* If not, then determine file size and record length */
    (void) unix_stat (midname, &buffer);  /* Get file stats in buffer */
    (void) read (f, midname, 80);  /* Read 1 record            */
    reclength = (int) (strchr (midname, '\n') - midname) + 1;  /* FindEnd */
    nrecs = (int) (buffer.st_size / reclength);  /* Get num of records   */
  }

  /* Binary search algorithm */

  low = 0;
  high = nrecs - 1;
  while (low <= high)
  {
    mid = low + (high - low) / 2;
    (void) lseek (f, (long) ((long) mid * (long) reclength), SEEK_SET);
    (void) read (f, midname, (unsigned int) reclength);
    if ((cond = strnicmp (last_name_first, midname, (unsigned int) namelen)) < 0)
      high = mid - 1;
    else
    {
      if (cond > 0)
        low = mid + 1;
      else
      {
        /* Return the address information */

        (void) close (f);

        /* The offset of 40 is just a number that should work properly */

        faddr->Point = 0;
        if (!find_address (&midname[40], faddr))
        {
          faddr->Zone = faddr->Net = faddr->Node = faddr->Point = (unsigned short) -1;
        }
        return;
      }
    }
  }

  faddr->Zone = faddr->Net = faddr->Node = faddr->Point = (unsigned short) -1;
  faddr->Domain = NULL;
  (void) close (f);
}

/* $Id: b_fuser.c,v 1.1 1999/02/28 13:48:00 hjk Exp $ */
