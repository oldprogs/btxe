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
 * Filename    : $Source: E:/cvs/btxe/src/file_lnx.c,v $
 * Revision    : $Revision: 1.6 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/06/08 20:20:35 $
 * State       : $State: Exp $
 * Orig. Author: HJK, CEH
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
zfree (char *path)              /* HJK - 98/12/30 */
{
  struct statfs sfs;
  char fname[512];
  char *npath;
  char *p;

  npath = strcpy (fname, path);

  p = strrchr (npath, '/');
  if (p != NULL)
    *(p + 1) = '\0';

  if (statfs (npath, &sfs))
  {
    status_line (">Free space on %s unknown, enough?", npath);
    return (200000);
  }

  return (sfs.f_bavail * (sfs.f_bsize / 64));
  // We can not check if there is no free inode, because that won't work
  // on non-Unix filesystems
  // and we've got to divide by 64 before multiplying (f_bsize because
  // f_bsize % 64 usually is zero) or we'll get an overflow on partitions
  // bigger than 2 GB.
}


int
dfind (struct FILEINFO *dta, char *name, int times)  /* CEH 19981112 */
{
  int i, j, k;
  struct dirent *d;
  struct stat buf;
  char fname[512];

  switch (times)
  {
  case 0:
    dta->fii.isopen = 0;
    for (i = strlen (name); (i + 1) && (name[i] != DIR_SEPC); i--) ;
    for (k = 0, j = i + 1; name[j]; j++, k++)
      dta->fii.fpat[k] = name[j];
    dta->fii.fpat[k] = 0;
    for (k = 0; k <= i; k++)
      dta->fii.dname[k] = name[k];
    dta->fii.dname[k] = 0;
    if (!dta->fii.dname[0])
      strcpy (dta->fii.dname, "." DIR_SEPS);
    if (!dexists (dta->fii.dname))
      return (1);
    dta->fii.dir = opendir (dta->fii.dname);
    /* status_line (">opendir [0][%i][%s:%s,%s]", (int) dta->fii.dir, name, dta->fii.dname, dta->fii.fpat); */
    dta->fii.isopen = 1;
    /* just continue with 1: return a filename */

  case 1:
    if (dta->fii.isopen)
    {
      while ((d = readdir (dta->fii.dir)) != NULL)
      {
        strcpy (fname, d->d_name);
        if (!fnmatch (dta->fii.fpat, fname, 0))
        {
          strcpy (fname, dta->fii.dname);
          strcat (fname, d->d_name);
          stat (fname, &buf);
          strcpy (dta->name, d->d_name);
          dta->time = buf.st_mtime;
          dta->size = buf.st_size;
          dta->attr = 0;
          if (S_ISDIR (buf.st_mode))
            dta->attr |= FA_SUBDIR;
          /* status_line (">opendir [1][%i][%s]", (int) dta->fii.dir, dta->name); */
          return (0);
        }
      }
    }
    else
      return (1);
    /* if not found, close now and return error */

  case 2:
    if (dta->fii.isopen)
      closedir (dta->fii.dir);
    dta->fii.isopen = 0;
    return (1);

  default:
    status_line ("!dfind-error %i", times);
    return (1);
  }
}

size_t
filelength (int handle)
{
  struct stat buf;

  if (fstat (handle, &buf) == 0)
    return (buf.st_size);
  else
    return (-1);
}

/* CEH 19990504:
 * Linux "rename" deletes newpath if it exists. So rename should not be
 * invoked if it does, and an error returned.
 *
 * rename will bt changed to linux_rename in includes.h in the
 * __unix__-section: #define rename(a,b) linux_rename(a,b)
 */

#undef rename

int
linux_rename (char *oldpath, char *newpath)
{
  struct stat buf;

  if (!stat (newpath, &buf))
    return (-1);
  else
    return (rename (oldpath, newpath));
}

/* $Id: file_lnx.c,v 1.6 1999/06/08 20:20:35 ceh Exp $ */
