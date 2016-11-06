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
 * Filename    : $Source: E:/cvs/btxe/src/file_w32.c,v $
 * Revision    : $Revision: 1.9 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/10/05 14:44:54 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello, OS/2 code: Bill Andrus
 *
 * Description : File I/O routines used by BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#include <limits.h>

int
dexists (char *filename)
{
  return (GetFileAttributes (filename) == 0xffffffff ? 0 : 1);
}

long
zfree (char *drive)
{
  /* We don't really need this library stuff, because
   * WIN32 has its own API GetDiskFreeSpace()
   * This should equally work with all compilers and 
   *
   * rewritten by AS 98-01-20
   * Modified by SVM at 17/09/99
   * modifications by HJK 10/01/99 */

  DWORD spcl, bps, clusters, totalclusters;
  char szDrive[MAXPATH + 1];
  BOOL rc, oldfunc;
  OSVERSIONINFO win_vi;
  HANDLE kernel = NULL;
  FARPROC df;
  char *fnm;

  GetFullPathName (drive, sizeof (szDrive), szDrive, &fnm);

  win_vi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&win_vi);
  oldfunc = TRUE;

  /* SVM: Check the win version and use extened call if high anough */
  if (win_vi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
      win_vi.dwMajorVersion >= 4 ||  /* if WINNT >= 4.0 */
      win_vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
      LOWORD (win_vi.dwBuildNumber) > 1000  /* if Win95 >= OSR2 */
    )
  {
    kernel = GetModuleHandle ("KERNEL32");

    if (kernel)
    {
      df = GetProcAddress (kernel, "GetDiskFreeSpaceExA");

      if (df)
      {
        ULARGE_INTEGER bytes, total, totalfree =
        {0, 0};

        oldfunc = FALSE;        /* Everything went ok, we can try new function */
        rc = (BOOL) df (szDrive, &bytes, &total, &totalfree);

        while (!rc && !oldfunc)
        {
          if (szDrive[3])       /* ...if we have path and this path does not exist */
          {
            status_line (">GetDiskFreeSpaceEx(%s): path not found", szDrive);

            if (fnm)            /* we got a file -- not a path */
            {
              *fnm = '\0';
              fnm = NULL;
            }
            else
              szDrive[3] = '\0';  /* path incorrect, ask for the space on the volume */

            rc = (BOOL) df (szDrive, &bytes, &total, &totalfree);
          }
          else
            oldfunc = TRUE;

        }

        if (!oldfunc)
        {
          CloseHandle (kernel);
          /* must return long, not int64, check if fit or return maximum long */
          return (bytes.QuadPart / 64 > LONG_MAX) ? LONG_MAX :
            (long) (bytes.QuadPart / 64);
        }
      }
    }
  }

  if (oldfunc)                  /* use just a standard old style call (Win95 1.0? ;) */
  {
    szDrive[3] = '\0';

    rc = GetDiskFreeSpace (szDrive, &spcl, &bps, &clusters, &totalclusters);
  }

  if (!rc)
  {
    status_line ("!SYS%04hu: GetDiskFreeSpace(%s)", GetLastError (), szDrive);
    if (kernel)
      CloseHandle (kernel);
    return (0);
  }

  if (kernel)
    CloseHandle (kernel);

  return (clusters * ((bps / 64) * spcl));
}


/* changed by alex, 97-02-11
 * I use API calls for findfirst, findnext et.al. This should not hurt
 * anything else because it is already #ifdef'ed. */

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
dir_findfirst (char *filename, int attribute, struct FILEINFO *dta)
{
  WORD dosDate, dosTime;

  dta->fii.cSearch = 1;
  dta->fii.usAttrib = (USHORT) attribute;
  dta->fii.infolevel = 1;
  dta->fii.hDirA = (HANDLE) 0xffffffff;
  dta->fii.hDirA = (HANDLE) FindFirstFile (filename, &dta->fii.ffbuf);

  if ((dta->fii.cSearch != 1) || (dta->fii.hDirA == INVALID_HANDLE_VALUE))
  {
    FindClose (dta->fii.hDirA);
    dta->fii.hDirA = (HANDLE) 0xffffffff;
    errno = ENOENT;
    return (1);
  }
  else
  {
    FileTimeToDosDateTime (&dta->fii.ffbuf.ftLastWriteTime, &dosDate, &dosTime);
    dta->time = (dosDate << 16) + dosTime;
    dta->attr = (char) dta->fii.ffbuf.dwFileAttributes;
    dta->size = dta->fii.ffbuf.nFileSizeLow;
    strncpy (dta->name, dta->fii.ffbuf.cFileName, FILENAMELEN);
    strlwr (dta->name);
    errno = 0;
    return (0);
  }
}

static int
dir_findnext (struct FILEINFO *dta)
{
  WORD dosDate, dosTime;

  dta->fii.cSearch = 1;
  dta->fii.findrc = (BOOL) FindNextFile (dta->fii.hDirA, &dta->fii.ffbuf);

  if ((dta->fii.cSearch != 1) || (!dta->fii.findrc))
  {
    FindClose (dta->fii.hDirA);
    dta->fii.hDirA = (HANDLE) 0xffffffff;
    errno = ENOENT;
    return (1);
  }
  else
  {
    FileTimeToDosDateTime (&dta->fii.ffbuf.ftLastWriteTime, &dosDate, &dosTime);
    dta->time = (dosDate << 16) + dosTime;
    dta->attr = (char) dta->fii.ffbuf.dwFileAttributes;
    dta->size = dta->fii.ffbuf.nFileSizeLow;
    strncpy (dta->name, dta->fii.ffbuf.cFileName, FILENAMELEN);
    strlwr (dta->name);
    errno = 0;
    return (0);
  }
}

static int
dir_findrelease (struct FILEINFO *dta)
{
  if (dta->fii.hDirA != INVALID_HANDLE_VALUE &&
      dta->fii.hDirA != (HANDLE) 0xffffffff)
  {
    FindClose (dta->fii.hDirA);
    dta->fii.hDirA = (HANDLE) 0xffffffff;
    errno = ENOENT;
    return (1);
  }
  else
    return (1);
}

/* alex, 97-02-11
 * this is a replacement for the stdandard stat() function. It _seems_ that
 * this function is somewhat buggy in Borland C, because calling stat() with
 * a non-existing filename (e.g. "NEWFILES") may sometimes result in a crash
 * (access violation). (No, I don't think this is funny :-) ). */

int
__stat (char *p, struct stat *stbuf)
{
  if (GetFileAttributes (p) == 0xffffffff)
  {                             /* check if the file exists */
    errno = EBADF;
    return (-1);                /* not there, return stat() - like. */
  }
  else
    return __stat (p, stbuf);
}

/* $Id: file_w32.c,v 1.9 1999/10/05 14:44:54 mr Exp $ */
