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
 * Filename    : $Source: E:/cvs/btxe/src/callback.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 01:49:22 $
 * State       : $State: Exp $
 * Orig. Author: Carlos Fernandez Sanz
 *
 * Description : Callback module 
 *
 * Note        :
 *   this module doesn't try to get clever and reject absurd strings - one
 *   never knows what will be defined as valid numbers in the future. So if
 *   you want to define 'CALL_ME_KNOW' as a callback number, you are welcome
 *   to do so. If that comes after the defined CID prefix, it will work.
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

#if 0
void
parse_callback (char *number)
{
  char *node;
  struct SCallback *newitem;
  char GotPhone[80], GotNode[80];

  while (*number == ' ')
    number++;
  if (number[0] == '\0')
    return;
  node = strchr (number, ' ');
  if (node == NULL)
  {
    strcpy (GotPhone, number);
    GotNode[0] = '\0';
  }
  else
  {
    *node = '\0';
    strcpy (GotPhone, number);
    strcpy (GotNode, node + 1);
    while (GotNode[0] == ' ')
      strcpy (GotNode, GotNode + 1);
  }
  newitem = (struct SCallback *) malloc (sizeof (struct SCallback));

  if (newitem == NULL)
    return;
  CallbackNumbers = (struct SCallback **) realloc (CallbackNumbers, (size_t) ((DefinedCallback + 1) * sizeof (struct SCallback *)));

  if (CallbackNumbers != NULL)
  {
    newitem->phone = ctl_string (GotPhone);
    parse_address (GotNode, &newitem->node);
    CallbackNumbers[(int) DefinedCallback] = newitem;
    parse_address (GotNode, (ADDRP) (&CallbackNumbers[(int) DefinedCallback]->node));
    DefinedCallback++;
  }
  else
    DefinedCallback = 0;
}
#endif


/* MR 970311 moved docrash from mailroot.c to callback.c */

void
docrash (ADDR * address)        /* CFS 961127 */
{
  FILE *fitemp;
  char *HoldName;
  char ve[80];

  HoldName = HoldAreaNameMunge (address);
  sprintf (ve, "%s%s.clo", HoldName, Hex_Addr_Str (address));
  status_line ("#Set: %s", ve);
  fitemp = share_fopen (ve, append_ascii, DENY_WRITE);
  fclose (fitemp);
}

void
callback (ADDRP CBAddress)
{
  int cftemp;

  status_line ("#Number is on callback list, rejecting call.");
  modem_reject ();

  status_line ("#Changing all mail for %u:%u/%u.%u to crash.",
               CBAddress->Zone, CBAddress->Net, CBAddress->Node,
               CBAddress->Point);

  cftemp = chg_mail;
  chg_mail = 'C';               /* Set all mail to crash */
  change_call_to_normal (CBAddress);
  chg_mail = cftemp;
  docrash (CBAddress);
  time_release ();
#if defined(OS_2) && !defined(__EMX__)
  UpdateCID = 1;
#endif
}

/* $Id: callback.c,v 1.5 1999/03/22 01:49:22 mr Exp $ */
