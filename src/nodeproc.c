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
 * Filename    : $Source: E:/cvs/btxe/src/nodeproc.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:36 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Nodelist processing module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static off_t index_filesize = (off_t) 0L;
static time_t index_filetime = (time_t) 0L;
static char index_filename[PATHLEN];

static char *curr_domain = NULL;

int get_nodelist_name (ADDR *);

int
get_nodelist_name (ADDRP opus_addr)
{
  int i;

  if (opus_addr->Domain == NULL)
  {
    opus_addr->Domain = domain_name[0];
    for (i = 0; i < num_domain_kludge; i++)
    {
      if (opus_addr->Zone == domainkludge[i].zone)
      {
        opus_addr->Domain = domain_name[domainkludge[i].domain];
        break;
      }
    }
  }

  curr_domain = opus_addr->Domain;

  nodelist_base = nodelist_name;

  for (i = 0; domain_name[i] != NULL; i++)
  {
    if (domain_name[i] == opus_addr->Domain)
    {
      nodelist_base = domain_nodelist[i];
      if (nodelist_base == NULL)
        nodelist_base = nodelist_name;
      return 1;
    }
  }

  return (i && domain_name[i] == NULL) ? 0 : 1;
}

/*---------------------------------------------------------------------------*/
/* CHECKLIST                                                                 */
/* See if nodelist has changed since we first tried to use it and if so,     */
/* dismiss old copy and get a new one                                        */
/*---------------------------------------------------------------------------*/

int
checklist ()
{
  struct stat idxstat;

  if (index_filesize == (off_t) 0L)
    return (0);

  unix_stat (index_filename, &idxstat);

  if ((index_filesize == idxstat.st_size) && (index_filetime == idxstat.st_mtime))
    return (0);

  status_line (MSG_TXT (M_REFRESH_NODELIST));
  free (node_index);
  node_index = (char *) NULL;
  index_filesize = (off_t) 0L;
  return (1);
}

/*---------------------------------------------------------------------------*/
/* NODEPROC                                                                  */
/* Find nodelist entry and set baud to nodelist baud for dialing out         */
/*---------------------------------------------------------------------------*/

int
nodeproc (char *nodeaddr)
{
  ADDR opus_addr;
  char *c;

  c = skip_blanks (nodeaddr);   /* get rid of the blanks      */

  if (!find_address (c, &opus_addr))
    return (0);

  if (!nodefind (&opus_addr, 1))  /* if we can't find the node  */
    return (0);                 /* go away now                */

  if (!CARRIER)                 /* if no carrier yet,         */
  {
    if (autobaud)
    {
      /* Set to highest rate for autobaud */
      set_baud (max_baud.rate_value, 1);
    }
    else
    {
      unsigned long baudrate;

      /* Otherwise use the nodelisted rate */
      /* if nothing "assumed"              */

      baudrate = (unsigned long) 300 *newnodedes.BaudRate;

      if ((baudrate == 300) && (assumebaud != 0))
        baudrate = assumebaud;
      set_baud (baudrate, 1);
    }
  }

  return (1);                   /* return success to caller   */
}

/*---------------------------------------------------------------------------*/
/* NODEFIND                                                                  */
/* Find nodelist entry for use by other routines (password, nodeproc)        */
/* If found, result will be in "newnodedes".                                 */
// 0  = failure
// 1  = success
// -1 = curmudgeon ???
/*---------------------------------------------------------------------------*/

int
nodefind (ADDRP bink_addr, int prtflag)
{
  int i, j, k;
  int have_boss_data = 0;
  int need_boss_data = 0;
  AKA *aka;

  checklist ();

  memset (&newnodedes, 0, sizeof (newnodedes));  /* r. hoerner */
  found_zone = found_net = 0;
  strcpy (lastfound_SysopName, "(unknown)");

  CURRENT = DEFAULT;            /* Set default paths, quotas  */

  if ((bink_addr->Net == boss_addr.Net)
      && (bink_addr->Node == boss_addr.Node)
      && (bink_addr->Zone == alias->Zone)
      && ((bink_addr->Domain == boss_addr.Domain)
          || (bink_addr->Domain == NULL)))
  {
    ++need_boss_data;
    if (BOSSphone && strlen (BOSSphone) > 2)
      ++have_boss_data;
    if (BOSSpwd && strlen (BOSSpwd) > 2)
      ++have_boss_data;
  }

  if (!bink_addr->Zone)
    bink_addr->Zone = alias->Zone;

  i = (*nodefunc) (bink_addr, have_boss_data);

  /* if i==0: maybe we have included the node in our "override" statement?
   * if the adress is invalid or no override statement exists for this node,
   * then leave "i" unchanged, else if phonenumber exists, then the node
   * exists. We'll check for a password directly in the password file (if
   * any)
   */
  i += GetAltPhone ((char *) &newnodedes.PhoneNumber, bink_addr);
  GetAltModemflag ((word *) & newnodedes.ModemType, bink_addr);
  GetAltFidoFlag ((word *) & newnodedes.NodeFlags, bink_addr);
  if (GetAltOnLineTime (&newnodedes.online_start, &newnodedes.online_end, bink_addr))
  {
    newnodedes.NodeFlags |= B_CM;  /* VRP 990820 */
  }

  assumed = k = 0;              /* Default to zone of first   */

  /* if (!i)
   *   goto lookup_done;
   * lookup_done:
   */

  j = 0;
  aka = alias;
  while (aka != NULL)
  {
    if ((aka->Point != 0)
        && ((aka->Node != newnodedes.NodeNumber)
            || (aka->Net != found_net)
            || (aka->Zone != found_zone)))
    {
      /* Don't use our point address except to the boss! */
      goto continue_label;
    }

    if ((aka->Domain == curr_domain)
        || ((curr_domain == NULL) && (aka->Domain == my_addr.Domain))
        || ((aka->Domain == NULL) && (curr_domain == my_addr.Domain)))
    {
      if (k == 0)
      {
        assumed = j;
        ++k;
      }

      if (aka->Zone == found_zone)
      {
        if (k == 1)
        {
          assumed = j;
          ++k;
        }

        if (aka->Net == found_net)
        {
          assumed = j;
          break;
        }
      }
    }

  continue_label:
    j++;
    aka = aka->next;
  }

  /* Another point of your boss? */
  if (!i && need_boss_data && bink_addr->Point)
    return (i);                 /* Don't use your bosspw ! */

  if (!i && (have_boss_data != 2))
  {
    if (prtflag)
      status_line (MSG_TXT (M_NO_ADDRESS), Full_Addr_Str (bink_addr));

    if (curmudgeon && CARRIER &&
        (bink_addr->Net != 0xffff)
        && (bink_addr->Node != 0xffff)
        && (bink_addr->Node != 9999)
        && ((bink_addr->Zone != alias->Zone)
            || (bink_addr->Net != alias->Net)))
    {
      return (-1);
    }

  }

  if (bink_addr->Zone == (unsigned short) -1)
    return i;

  /* If we found the entry, then we promote the file request
   * to the "KNOWN" class. If the password field is non-zero,
   * then promote to "PROT". It's OK to do that since the higher
   * level code will hang up before f.req's if the password does
   * not match.
   *
   */

  if (i)
    CURRENT = (newnodedes.Password[0] ? PROT : KNOWN);

  if (!need_boss_data)
    return (i);

  /*
   *    We can get here one of two ways:
   *
   *    1) No nodelist data was found and this is the BOSS.
   *
   *    2) Nodelist lookup occurred, but this is the BOSS.
   *
   *    For case (1), have_boss_data MUST be 2 (meaning we have
   *    both a phone number and a password entry). If that is the
   *    case, fill in newnodedes with mostly zeroes, plugging in
   *    the BOSS net, node, phone number and password.
   *
   *    For case (2), just see if there is any substitution for
   *    BOSSphone and/or BOSSpwd, then exit.
   *
   */

  if (BOSSphone && strlen (BOSSphone) > 2)
    strntcpy (newnodedes.PhoneNumber, BOSSphone, 40);

  if (BOSSpwd && strlen (BOSSpwd) > 2)
  {
    memset (newnodedes.Password, 0, sizeof (newnodedes.Password));
    strncpy (newnodedes.Password, BOSSpwd, 8);
  }

  if (i)
    return (1);

  /* No BOSS in the nodelist */
  if (have_boss_data != 2)
  {
    status_line (MSG_TXT (M_NO_BOSS));
    return (0);
  }

  newnodedes.NodeNumber = bink_addr->Node;  /* Node Number */
  newnodedes.NetNumber = bink_addr->Net;  /* Net Number  */
  newnodedes.Cost = newnodedes.RealCost = 0;  /* Assume boss is free  */

  /* Default System Name and City */
  strcpy (newnodedes.SystemName, PRDCT_PRTY "'s Boss");
  strcpy (newnodedes.MiscInfo, "Somewhere out There");

  strcpy (lastfound_SysopName, newnodedes.SystemName);
  if (!nofancystrings)
    fancy_str (lastfound_SysopName);
  newnodedes.HubNode = 0;       /* Don't know who's HUB */
  newnodedes.BaudRate = (char) (max_baud.rate_value / 300);

  /* Assume boss speed = ours */
  newnodedes.ModemType = 0;     /* Or modem type        */
  newnodedes.NodeFlags = B_CM;  /* Assume boss is CM    */
  newnodedes.online_start = 0;
  newnodedes.online_end = 0;

  return (1);
}

/* $Id: nodeproc.c,v 1.7 1999/09/27 20:51:36 mr Exp $ */
