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
 * Filename    : $Source: E:/cvs/btxe/src/callerid.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 01:49:22 $
 * State       : $State: Exp $
 * Orig. Author: MR
 *
 * Description : BinkleyTerm CallerID Functions
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"


char *
Get_CallerID (char *number)
{
  char *cid, *cfwork;           /* Used for string handling */

  cid = strstrci (number, CIDline);  /* search caller-id prefix  */

  if (cid != NULL)              /* got caller-id prefix     */
  {
    cfwork = (char *) malloc (strlen (cid) + 1);

    if (cfwork)
    {
      strcpy (cfwork, cid + strlen (CIDline));

      while (cfwork[0] == ' ')
        strcpy (cfwork, cfwork + 1);

      cid = cfwork;

      while (isdigit (*cid) || (*cid == '-') || (*cid == '.'))  /* MR 970723 */
        cid++;

      *cid = '\0';

      cid = (char *) malloc (strlen (cfwork) + 1);
      strcpy (cid, cfwork);
      free (cfwork);
      status_line ("#Got CID: %s", cid);
    }
    else
      cid = NULL;
  }
  return (cid);
}


int
rejected_call (char *cid)
{
  int accept = -1;
  NODESTRUC *pnode = NULL;
  CIDLSTP cidlst = NULL;
  FPP tmpfp = NULL;
  time_t t;

  if (cid != NULL)
  {
    for (pnode = knownnode; pnode != NULL; pnode = pnode->next)
    {
      t = unix_time (NULL);     /* MR 970714 enable TRX checking  */

      for (cidlst = pnode->CallerIDList; cidlst != NULL; cidlst = cidlst->next)
      {
        if (strstr (cid, cidlst->CallerID) != NULL)
        {
          if (pnode->Flags & FREEPOLL)
          {
            for (tmpfp = pnode->FPoll; tmpfp != NULL; tmpfp = tmpfp->next)
            {
              if (accept < 0)   /* no boolean for first accept    */
                accept = freepoll (&pnode->addr, tmpfp, t);
              else
              {
                if (tmpfp->and_op)
                  accept = freepoll (&pnode->addr, tmpfp, t) && accept;
                else
                  accept = freepoll (&pnode->addr, tmpfp, t) || accept;
              }
            }

            t = 0;              /* MR 970714 disable TRX checking */
          }

          if (pnode->Flags & CALLBACK)
          {
            callback (&pnode->addr);
            return 1;
          }
        }
      }
    }
  }

  if (accept != -1)             /* MR 970329 checked at least one freepoll condition */
  {
    if (accept)
      status_line (MSG_TXT (M_ACCEPT));
    else
    {
      status_line (MSG_TXT (M_REJECT));
      modem_reject ();
      time_release ();
      global_rescan ();         /* bugfix for multiline CE 971020 */
#if defined(OS_2) && !defined(__EMX__)  /*  TS 970403   */
      UpdateCID = 1;
#endif
    }

    return (!accept);
  }

  return 0;
}

/* $Id: callerid.c,v 1.4 1999/03/22 01:49:22 mr Exp $ */
