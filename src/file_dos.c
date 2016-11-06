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
 * Filename    : $Source: E:/cvs/btxe/src/file_dos.c,v $
 * Revision    : $Revision: 1.12 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/06/10 22:01:15 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello, OS/2 code: Bill Andrus
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


/* Z F R E E -- Return total number of free 64Byte units on drive specified */

long
zfree (char *drive)
{
  union REGS r;

  unsigned char driveno;
  long clusters, spcl, bps;

  if (drive[0] != '\0' && drive[1] == ':')
  {
    driveno = (unsigned char) (islower (*drive) ? toupper (*drive) : *drive);
    driveno = (unsigned char) (driveno - 'A' + 1);
  }
  else
    driveno = 0;                /* Default drive    */

  r.x.ax = 0x3600;              /* get free space   */
  r.h.dl = driveno;             /* on this drive    */
  int86 (0x21, &r, &r);         /* go do it         */

  if (r.x.ax == 0xffff)         /* error return??   */
    return (0);

  clusters = (long) r.x.bx;     /* bx = clusters avail  */
  spcl = (long) r.x.ax;         /* ax = sectors/clust   */
  bps = (long) r.x.cx;          /* cx = bytes/sector    */

  return (clusters * ((bps / 64) * spcl));
}


int
dfind (struct FILEINFO *dta, char *name, int times)
{
  union REGS r;
  struct SREGS s;
  char far *dtaptr = (char *) &(dta->fii);
  char far *nameptr = (char *) name;
  unsigned olddtaseg, olddtaofs;
  int retval = 0;               /* TE 960615 */

  /* TE 960615: Backup old DTA address. You never know ... */
  r.h.ah = 0x2F;
  intdosx (&r, &r, &s);
  olddtaseg = s.es;
  olddtaofs = r.x.bx;

  s.ds = FP_SEG (dtaptr);
  r.x.dx = FP_OFF (dtaptr);
  r.h.ah = 0x1a;
  intdosx (&r, &r, &s);
  r.x.bx = 0;
  r.x.cx = (unsigned int) ~0x08;
  s.ds = FP_SEG (nameptr);
  r.x.dx = FP_OFF (nameptr);
  r.x.si = 0;
  r.x.di = 0;
  if (times == 0)
  {
    r.h.ah = 0x4e;
    intdosx (&r, &r, &s);
    /* If not found or a character device (e.g. "COM1") */
    if ((r.x.cflag != 0)
        || ((dta->fii.attr & 0x40) != 0))
    {
      dta->name[0] = '\0';
      retval = 1;
    }
  }
  else
    do
    {
      r.h.ah = 0x4f;
      intdosx (&r, &r, &s);
      /* If not found or a character device (e.g. "COM1") */
      if ((r.x.cflag != 0) || ((dta->attr & 0x40) != 0))
      {                         /* no more files found */
        dta->name[0] = '\0';
        retval = 1;
        times = 0;
      }
    }
    while (times == 2);

  dta->attr = dta->fii.attr;
  dta->time = dta->fii.time;
  dta->size = dta->fii.size;
  strncpy (dta->name, dta->fii.name, MAXFILENAME);
  dta->name[MAXFILENAME] = '\0';
  strlwr (dta->name);

  /* TE 960615: Now restore old DTA address */
  r.h.ah = 0x1A;
  s.ds = olddtaseg;
  r.x.dx = olddtaofs;
  intdosx (&r, &r, &s);

  return retval;
}

/* $Id: file_dos.c,v 1.12 1999/06/10 22:01:15 mr Exp $ */
