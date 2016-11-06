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
 * Filename    : $Source: E:/cvs/btxe/src/common.c,v $
 * Revision    : $Revision: 1.6 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:24 $
 * State       : $State: Exp $
 *
 * Description : These are some routines and data used by BT and BTCTL
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static int LOCALFUNC mkdirs (char *);  /* MR 970715 */

short pvtnet = -1;

void
fancy_str (char *string)
{
  register int flag = 0;
  char *s;

  s = string;

  while (*string)
  {
    if (isalpha (*string))      /* If alphabetic,     */
    {
      if (flag)                 /* already saw one?   */
        *string = (char) tolower (*string);  /* Yes, lowercase it  */
      else
      {
        flag = 1;               /* first one, flag it */
        *string = (char) toupper (*string);  /* Uppercase it       */
      }
    }
    else                        /* if not alphabetic  */
      flag = 0;                 /* reset alpha flag   */
    string++;
  }
}

char *
skip_blanks (char *string)
{
  while (*string && isspace (*string))
    ++string;
  return (string);
}

char *
skip_to_blank (char *string)
{
  while (*string && (!isspace (*string)))
    ++string;
  return (string);
}

char *
skip_to_word (char *string)
{
  string = skip_to_blank (string);
  return skip_blanks (string);
}

int
parse (char *input, char far * list[])
{
  int i, j, len;
  char *p;
  char far *q;

  /* MMP 960420 Before we only required a listed string to match the
   * prefix of input. Therefore "Task" could also be specified as
   * "TaskNumber", and "Log" as "LogFile" etc. This may sound nice,
   * but it also meant that we could not have two keywords where the
   * first was a prefix of the other. And we could not set environment
   * variables on the command line where the name of the variable also
   * was the name of a keyword (Task)
   */

  for (i = 0; list[i] != NULL; i++)
  {
    p = input;
    q = list[i];
    len = strlen (list[i]);

    for (j = len; j > 0; j--, p++, q++)
    {
      if (tolower (*p) != tolower (*q))
        break;
    }

    if (j == 0 &&
        (*p == '\0' ||          /* MMP 960420 / TJW 960504 */
         strchr (" \n\r\t!\"$%&/()=?'+*~#-_.:,;<>|", *p) != NULL ||
         *--p == '['            /* TJW 960830 special treatment for [expression] */
        )
      )
      return (++i);
  }

  return (-1);
}


char *
ctl_string (char *source)       /* malloc & copy to ctl      */
{
  char *dest, *c;
  int i;

  c = skip_blanks (source);     /* get over the blanks       */
  i = (int) strlen (c);         /* get length of remainder   */
  if (i < 1)                    /* must have at least 1      */
    return (NULL);              /* if not, return NULL       */
  dest = calloc (1, (unsigned int) (i + 2));  /* allocate space for string */
  /* allocate 1 byte more for \ */
  if (dest == NULL)             /* If we failed to get it,   */
    return (NULL);              /* Just return NULL          */
  strcpy (dest, c);             /* copy the stuff over       */
  return (dest);                /* and return the address    */
}

char *
ctl_slash_string (int make, char *source)  /* malloc & copy to ctl      */
{
  char *dest;
  int i;
  struct stat buffer;

  dest = ctl_string (source);   /* TJW 960414 replaced redundant code by */
  if (dest == NULL)             /*            function call to above fn  */
    return NULL;

  delete_backslash (dest);      /* get rid of trailing stuff */
  /* Check to see if the directory exists */
  i = stat (dest, &buffer);
  if (i || (!(buffer.st_mode & S_IFDIR)))  /* r. hoerner: "MakeDir" */
  {
    if (make)
    {
      mkdirs (dest);
      /* Check to see if the NOW directory exists */
      i = stat (dest, &buffer);
    }
  }

  if (i || (!(buffer.st_mode & S_IFDIR)))
  {
    printf ("Directory '%s' does not exist!\n", dest);
    printf ("This program may fail to execute properly because of this!\n");
    free (dest);
    return (NULL);
  }

  add_backslash (dest);         /* add the backslash         */
  return (dest);                /* return the directory name */
}

char *
add_backslash (char *str)
{
  char *p;

  p = str + strlen (str) - 1;

  if (p >= str)
  {
    /* Strip off the trailing blanks */
    while ((p >= str) && (isspace (*p)))
    {
      *p = '\0';
      --p;
    }

    /* Put a backslash if there isn't one */
    if ((p >= str) && (*p != '\\') && (*p != '/'))
    {
      *(++p) = DIR_SEPC;
      *(++p) = '\0';
    }
  }

  f_fancy_str (str);
  return (str);
}

char *
delete_backslash (char *str)
{
  char *p;

  p = str + strlen (str) - 1;

  if (p >= str)
  {
    /* Strip off the trailing blanks */
    while ((p >= str) && (isspace (*p)))
    {
      *p = '\0';
      --p;
    }

    /* Get rid of backslash if there is one */
    if ((p >= str) && ((*p == '\\') || (*p == '/')))
    {
      if ((p > str) && (*(p - 1) != ':'))  /* Don't delete on root */
        *p = '\0';
    }
  }

  f_fancy_str (str);
  return (str);
}


static int LOCALFUNC
mkdirs (char *path)
{
  int retval;

  while (0 != (retval = mkdir (path)))
  {
    char subpath[PATHLEN + 1] = "";
    char *delim;

    if (EACCES == errno)
      return retval;

    if (NULL == (delim = strrchr (path, DIR_SEPC)))
      return retval;

    strncat (subpath, path, delim - path);  /* Appends NUL    */
    mkdirs (subpath);
  }

  return retval;
}

/* $Id: common.c,v 1.6 1999/03/22 03:47:24 mr Exp $ */
