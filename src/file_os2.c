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
 * Filename    : $Source: E:/cvs/btxe/src/file_os2.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:28 $
 * State       : $State: Exp $
 * Orig. Author: Bill Andrus
 *
 * Description : File I/O routines used by BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int
dexists (char *filename)
{
  struct stat stbuf;

  return (stat (filename, &stbuf) != -1) ? 1 : 0;
}


long
zfree (char *path)
{
  int drive;
  FSALLOCATE dt;
  long clusters, spcl, bps;

  if (*path != '\0' && path[1] == ':')
    drive = tolower (*path) - 'a' + 1;
  else
    drive = 0;

  DosQueryFSInfo ((USHORT) drive, 1, (unsigned char far *) &dt, sizeof (FSALLOCATE));

  clusters = (long) dt.cUnitAvail;  /* units available  */
  spcl = (long) dt.cSectorUnit; /* sectors per unit */
  bps = (long) dt.cbSector;     /* bytes per sector */

  return (clusters * ((bps / 64) * spcl));
}


static int dir_findfirst (char far *, int, struct FILEINFO *);
static int dir_findnext (struct FILEINFO *);
static int dir_findrelease (struct FILEINFO *);


int
dfind (struct FILEINFO *dta, char *name, int times)
{
  if (times == 0)
    return (dir_findfirst (name, 0x37, dta));
  else if (times == 1)
    return (dir_findnext (dta));
  else
    return (dir_findrelease (dta));
}


/*--------------------------------------------------------------------------*/
/* Local constants                                                          */
/*--------------------------------------------------------------------------*/

#define FILENAMELEN 256

static int
dir_findfirst (char far * filename, int attribute, struct FILEINFO *dta)
{
  dta->fii.cSearch = 1;
  dta->fii.usAttrib = (USHORT) attribute;
  dta->fii.infolevel = 1;

  if ((dta->fii.hDirA != 0xffff) &&
      (dta->fii.hDirA != 0x0000) &&
      (dta->fii.hDirA != 0xffffffff))
    DosFindClose (dta->fii.hDirA);

  dta->fii.hDirA = 0xffffffff;
  dta->fii.findrc = DosFindFirst ((PSZ) filename,
                                  &(dta->fii.hDirA),
                                  dta->fii.usAttrib,
                                  (PVOID) & (dta->fii.ffbuf),
                                (ULONG) (sizeof (struct FILEINFOinternalbuf)
                                         * dta->fii.cSearch),
                                  & (dta->fii.cSearch),
                                  dta->fii.infolevel);

  if ((dta->fii.cSearch != 1) || (dta->fii.findrc && (dta->fii.findrc != ERROR_EAS_DIDNT_FIT)))
  {
    DosFindClose (dta->fii.hDirA);
    dta->fii.hDirA = 0xffff;
    errno = ENOENT;
    return (1);
  }
  else
  {
    dta->time = (dta->fii.ffbuf.wr_date << 16) + dta->fii.ffbuf.wr_time;
    dta->attr = (char) dta->fii.ffbuf.attrib;
    dta->size = dta->fii.ffbuf.size / ((_osmajor == 10) ? 2 : 1);
    strncpy (dta->name, dta->fii.ffbuf.name, FILENAMELEN);
    strlwr (dta->name);
    errno = 0;
    return (0);
  }
}


static int
dir_findnext (struct FILEINFO *dta)
{
  dta->fii.cSearch = 1;
  dta->fii.findrc = DosFindNext (dta->fii.hDirA,
                                 (PVOID) & (dta->fii.ffbuf),
                                 (ULONG) (sizeof (struct FILEINFOinternalbuf)
                                          * dta->fii.cSearch),
                                 &dta->fii.cSearch);

  if ((dta->fii.cSearch != 1) || (dta->fii.findrc && (dta->fii.findrc != ERROR_EAS_DIDNT_FIT)))
  {
    DosFindClose (dta->fii.hDirA);
    dta->fii.hDirA = 0xffff;
    errno = ENOENT;
    return (1);
  }
  else
  {
    dta->time = (dta->fii.ffbuf.wr_date << 16) + dta->fii.ffbuf.wr_time;
    dta->attr = (char) dta->fii.ffbuf.attrib;
    dta->size = dta->fii.ffbuf.size / ((_osmajor == 10) ? 2 : 1);
    strncpy (dta->name, dta->fii.ffbuf.name, FILENAMELEN);
    strlwr (dta->name);
    errno = 0;
    return (0);
  }
}


static int
dir_findrelease (struct FILEINFO *dta)
{
  if ((dta->fii.hDirA != 0xffff) && (dta->fii.hDirA != 0xffffffff))
  {
    DosFindClose (dta->fii.hDirA);
    dta->fii.hDirA = 0xffff;
    errno = ENOENT;
    return (1);
  }
  else
    return (0);
}

/* $Id: file_os2.c,v 1.7 1999/03/22 03:47:28 mr Exp $ */
