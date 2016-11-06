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
 * Filename    : $Source: E:/cvs/btxe/src/maxprm.c,v $
 * Revision    : $Revision: 1.6 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:35 $
 * State       : $State: Exp $
 * Orig. Author: Alex Woick
 *
 * Description : get filerequest-relevant info from max.prm
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/* for the Maximus 3.0 structs */
typedef signed short sword;
typedef signed char sbyte;

#undef ALIAS_CNT

typedef struct _netaddr NETADDR;

struct _netaddr
{
  word zone;
  word net;
  word node;
  word point;
};

#include "third/max3/prm.h"

/* AW 980219 configure request module with Maximus 2.x or 3.x info */

KWD_ERR
cfg_MaxPrm (int keyword, char *c)
{
  KWD_ERR error = kwd_other_err;
  FILE *maxprm;
  struct m_pointers prm;
  char *offsets;
  long len;
  char junk[PATHLEN];

  memset (&prm, 0, sizeof (struct m_pointers));

  /* open max.prm */
  maxprm = share_fopen (c, read_binary, DENY_NONE);
  if (maxprm != NULL)
  {
    /* get max.prm */
    fread (&prm, sizeof (struct m_pointers), 1, maxprm);

    if (prm.id == 'M' && prm.version == CTL_VER)
    {
      /* read heap */
      len = filelength (fileno (maxprm));
      offsets = (char *) malloc (len - prm.heap_offset);
      if (offsets)
      {
        fseek (maxprm, (long) prm.heap_offset, SEEK_SET);
        fread (offsets, sizeof (char), (unsigned) (len - (long) prm.heap_offset), maxprm);

        /* decide if max 2.0 or 3.0 */
        if (prm.heap_offset < sizeof (struct m_pointers))
        {
          /* Max 2.0: store filename of area.dat */
          error = cfg_CopyString (keyword, &fri_areadat, PRM (adat_name));
        }
        else
        {
          /* Max 3.0: store filenames of farea.dat/farea.idx/access.dat */
          strntcpy (junk, PRM (farea_name), PATHLEN - 4);
          strcat (junk, ".idx");
          error = cfg_CopyString (keyword, &fri_areaidx, junk);
          strcpy (junk + strlen (junk) - 3, "dat");
          error = cfg_CopyString (keyword, &fri_areadat, junk);
          strntcpy (junk, PRM (access), PATHLEN - 4);
          strcat (junk, ".dat");
          error = cfg_CopyString (keyword, &fri_uclass, junk);
        }

        free (offsets);
        fri_areatype = FRI_MAXIMUS;
      }
      else
        printf ("ERR: unable to allocate memory reading '%s'", c);
    }
    else
      printf ("ERR: file '%s' does not seem to be a max.prm style file", c);

    fclose (maxprm);
  }
  else
    printf ("ERR: cannot open max.prm file '%s'", c);

  return error;
}

/* $Id: maxprm.c,v 1.6 1999/03/22 03:47:35 mr Exp $ */
