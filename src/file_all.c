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
 * Filename    : $Source: E:/cvs/btxe/src/file_all.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:27 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello, OS/2 code: Bill Andrus
 *
 * Description : File I/O routines used by BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
print_error (char *filename)
{
  if (loglevel < 5)
    return;

  if (!initialized)             /* r. hoerner */
  {                             /* say WHY it failed! */
    fprintf (stderr, "%s: %s\n", filename == NULL ? "(null)" : filename,
             strerror (errno));
  }
  else
    status_line (">%s: %s", filename == NULL ? "(null)" : filename, strerror (errno));

  errno = 0;
}

#ifdef NEED_MKTEMP

char *
mktemp (char *template)
{
  static char save[8] = "Z";
  char *p;
  int i;

  p = save;

  if (*p == 'Z')
    sprintf (p, "%06d", TaskNumber);

  while (*p)
  {
    if (isdigit (*p))
    {
      *p = 'a';
      break;
    }

    if (*p++ < 'z')
      break;
  }

  if ((*p == '\0') || ((i = strlen (template)) < 6))
    return (NULL);

  strcpy (&template[i - 6], save);
  return (template);
}

#endif

/* For Borland C++ 2.0, change __TURBOC_OLD__ to __TURBOC__ */
#ifdef __TURBOC_OLD__

/*
 * utime function for Turbo / Borland C.
 *
 * We should make this more generic in case some other DOS
 * compiler comes up lacking, but for now the only one we
 * have that needs it happens to be Borland.
 */

int cdecl
utime (char *name, struct utimbuf *times)
{
  int handle;
  struct date d;
  struct time t;
  struct ftime ft;

  unixtodos (times->modtime, &d, &t);
  ft.ft_tsec = t.ti_sec / 2;
  ft.ft_min = t.ti_min;
  ft.ft_hour = t.ti_hour;
  ft.ft_day = d.da_day;
  ft.ft_month = d.da_mon;
  ft.ft_year = d.da_year - 1980;

  if ((handle = open (name, O_RDONLY)) == -1)
  {
    print_error (name);
    return -1;
  }

  setftime (handle, &ft);
  close (handle);
  return 0;
}

#endif

int
got_error (char *string1, char *string2)
{
  if (errno == 0x18)
    errno = 0;

  if (errno != 0)
  {
    status_line ("%s %d, %s %s %s", MSG_TXT (M_ERROR), errno, MSG_TXT
                 (M_CANT), string1, string2);
    errno = 0;
    return (1);
  }

  return (0);
}


#ifndef __unix__

static int share_flags[] =
{
#ifdef SH_COMPAT
  SH_COMPAT,
#else
  0,
#endif
  SH_DENYNO,
  SH_DENYRD,
  SH_DENYRW,
  SH_DENYWR
};

#else

static int share_flags[] =
{
  0, 0, 0, 0, 0
};

#endif


int
share_open (char *filename, int open_mode, int shflag)
{
#ifdef DOS16
  if ((no_sharing == 0) && (_osmajor >= 3))
#else
  if (no_sharing == 0)
#endif /* DOS16 */
#ifdef  SHARE_SOPEN
    return sopen (filename, open_mode, share_flags[shflag]);
#else
    return open (filename, open_mode | share_flags[shflag]);
#endif
  else
    return (open (filename, open_mode));
}


#ifdef DEBUG
#undef share_fopen
#endif

#ifdef DOS16
# define DOSMAJOR3 (_osmajor >= 3)
#else
# define DOSMAJOR3 (1)
#endif


FILE *
share_fopen (char *filename, char *mode, int shflag)
{
  FILE *stream;

#ifdef MUST_FDOPEN
  int fd;
  int open_mode;
  char c, *p;
  int sflag = S_IREAD | S_IWRITE;

  /* Microsoft made this easy for us. They gave us a stream-open
   *  function that supports file sharing. Borland was not so kind.
   *  So -- what we do here is open the file using the only sharing
   *  API they provide -- that gives us a file handle -- then we
   *  use fdopen to get a file stream out of it. Sheesh!
   *
   *  Of course, Microsoft 5.1 provides about the same level of
   *  support as Borland -- almost. They explicitly warn you not
   *  to do this neat thing I did for Borland. So I get no sharing
   *  support for fopen() under 5.1. Double sheesh! */

  errno = 0;                    /* reset error (c-lib) */

  if ((no_sharing == 0) && DOSMAJOR3)
  {
    /* Figure out the translation from fopen-mode to
     * open-mode... */

    p = mode;
    c = *p++;
    if (c == 'w')
      open_mode = O_CREAT | O_RDWR | O_TEXT;
    else if (c == 'r')
      open_mode = O_RDONLY | O_TEXT;
    else if (c == 'a')
      open_mode = O_CREAT | O_RDWR | O_APPEND | O_TEXT;
    else
      return (FILE *) NULL;
    while (*p)
    {
      c = *p++;
      if (c == 'b')
        open_mode = (open_mode & ~O_TEXT) | O_BINARY;
      if (c == '+')
      {
        if ((open_mode & (O_RDONLY | O_WRONLY | O_RDWR)) == O_RDONLY)
          open_mode = (open_mode & ~O_RDONLY) | O_RDWR;
        else if (!(open_mode & O_APPEND))
          open_mode |= O_TRUNC;
      }
    }

    /* Open the file handle */

#ifdef SHARE_SOPEN
    fd = sopen (filename, open_mode, share_flags[shflag], sflag);
#else
    open_mode |= share_flags[shflag];

#ifdef __TURBOC__               /* Borland C 3.10 */
    if (open_mode & O_CREAT)
      fd = open (filename, open_mode, sflag);
    else
      fd = open (filename, open_mode);
#else
    fd = open (filename, open_mode, sflag);
#endif

#endif

    if (fd == -1)
    {
      stream = NULL;
      goto end_of_sharefopen;
    }

    /* Got the handle, make the stream */

    if ((stream = fdopen (fd, mode)) == (FILE *) NULL)
      close (fd);

    goto end_of_sharefopen;
  }
  else
#else /* MUST_FDOPEN */

#ifndef CANT_FSOPEN
  if ((no_sharing == 0) && DOSMAJOR3)
  {
    stream = _fsopen (filename, mode, share_flags[shflag]);
    goto end_of_sharefopen;
  }
  else
#endif /* CANT_FSOPEN */

#endif /* MUST_FDOPEN */

    stream = fopen (filename, mode);

end_of_sharefopen:

  if (stream == NULL)
    print_error (filename);

  return (stream);
}

/* $Id: file_all.c,v 1.3 1999/03/22 03:47:27 mr Exp $ */
