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
 * Filename    : $Source: E:/cvs/btxe/src/b_passwo.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/06/10 16:55:38 $
 * State       : $State: Exp $
 * Orig. Author: Wynn Wagner III
 *
 * Description : BinkleyTerm Password Processor
 *
 * Note        :
 *   This module is based largely on a similar module in OPUS-CBCS V1.03b.
 *   The original work is (C) Copyright 1987, Wynn Wagner III. The original
 *   author has graciously allowed us to use his code in this work.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static char *LOCALFUNC strnpbk (char *, int);

/*--------------------------------------------------------------------------*/
/* N PASSWORD                                                               */
/*--------------------------------------------------------------------------*/

int
n_password (ADDR * rem_addrp, char *theirs, char *ours, int *logit)
{
  int got_one;
  char s_ours[9], s_theirs[9];

  if ((ours != NULL) && (ours[0]))
  {
    got_one = 2;

    if ((theirs != NULL) && (theirs[0]))
    {
      got_one = 1;

      strnpbk (theirs, 8);      /* Get rid of trailing blanks */
      strnpbk (ours, 8);

      if (!strnicmp (theirs, ours, 8))
      {
        /* If this is the first time, we'll log it.
         * Otherwise, forget it.
         */

        if (*logit)
        {
          status_line (MSG_TXT (M_PROTECTED_SESSION));
          *logit = FALSE;
          is_protected = 1;     /* mtt 96-04-28 */
        }
        return 0;
      }
    }

    strntcpy (s_ours, ours, 9);
    strntcpy (s_theirs, theirs, 9);

    status_line (MSG_TXT (M_PWD_ERROR),
                 Full_Addr_Str (rem_addrp),
                 s_theirs,      /* TJW 960427 removed PW trash bug if len(pw)=8 */
                 s_ours);       /* TJW 960427 removed PW trash bug if len(pw)=8 */

    return got_one;
  }
  return 0;
}

/*--------------------------------------------------------------------------*/
/* N GET PASSWORD                                                           */
/* Find the nodelist entry for this system and point remote_password at     */
/* its password if any                                                      */
/*--------------------------------------------------------------------------*/

int
n_getpassword (ADDRP pw_addr)
{
  int i;

  remote_password = NULL;       /* Default to no password   */
  newnodedes.Password[0] = '\0';

  remote_password = PasswordFromFile (pw_addr);  /* r. hoerner  */

#ifdef USE_FIDOCONFIG
  if (remote_password == NULL)
    remote_password = PasswordFromFidoconfig (pw_addr);
#endif

  i = nodefind (pw_addr, 0);    /* find the node in the list */
  if (i <= 0)                   /* and fills newnodedes-struc */
  {
    /* remote_password = NULL; */

    /* if a node or point exists in my password file, then he is at least "KNOWN" */
    CURRENT = (newnodedes.Password[0] ? PROT : KNOWN);
    return (i);                 /* return failure if can't  */
  }

  is_listed = i;                /* mtt 96-04-28 */

  if (remote_password)          /* as taken from password file, r. hoerner */
    memcpy (newnodedes.Password, remote_password, 8);  /* MR 970717 memcpy! */

  if (newnodedes.Password[0] != '\0')  /* If anything there,       */
  {
    remote_password = (char *) (&newnodedes.Password[0]);  /* Point at it */
    CURRENT = PROT;
    return (1);                 /* Successful attempt       */
  }
  else
  {
    /* No password involved */
    return (0);
  }
}

/*
 * Strip all trailing blanks from a record.
 */

static char *LOCALFUNC
strnpbk (register char *string, register int n)
{
  string += n;                  /* point past end of string */
  while (n--)                   /* now keep count */
  {
    if (*--string)              /* if there's a character   */
    {
      if (*string != ' ')       /* if not a blank, we exit  */
        break;
      *string = '\0';           /* if blank, terminate here */
    }
  }
  return string;
}

/* $Id: b_passwo.c,v 1.5 1999/06/10 16:55:38 ceh Exp $ */
