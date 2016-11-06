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
 * Filename    : $Source: E:/cvs/btxe/include/cache.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:02 $
 * State       : $State: Exp $
 *
 * Description : Cache definitions
 *
 *---------------------------------------------------------------------------*/

#ifdef CACHE

#ifndef CACHE_H
#define CACHE_H

#define BLOCKSIZE   50          /* Allocation unit size */
#define HASHSIZE    41          /* Size of hash table   */
#define KEEPORDER               /* Keep directory order */

#ifdef __BORLANDC__             /* jl 960729:  DIRECTORY is already defined in dir.h */
#  undef DIRECTORY
#endif

typedef struct FILEINFO DTA;
typedef struct _DIRECTORY DIRECTORY;
typedef struct _DIRENTRY DIRENTRY;
typedef struct _DIRBUF DIRBUF;
typedef struct _FLOWFILE FLOWFILE;
typedef struct _FLOWENT FLOWENT;

struct _DIRECTORY
{
  DIRECTORY *next;              /* -> Next in directory chain   */
  FLOWFILE *flow;               /* -> Cached flow files         */
#ifdef KEEPORDER
  DIRENTRY *list;               /* List of entries              */
#endif
  DIRENTRY *ent[HASHSIZE];      /* Hash table with entries      */
  time_t loaded;                /* If non-zero, time of loading */
  int len;                      /* Length of name               */
  char drive;                   /* Upper cased drive letter     */
  char name[1];                 /* Upper cased name, no drive   */
};

struct _DIRENTRY
{
  DIRENTRY *next;               /* -> Next in hash chain        */
#ifdef KEEPORDER
  DIRENTRY *dirnext;            /* -> Next in dir listing       */
#endif
  long time;                    /* Timestamp                    */
  long size;                    /* Size of file in bytes        */
  char attr;                    /* Attribute                    */
  char name[13];                /* Name of file                 */
};

struct _FLOWENT
{
  FLOWENT *next;                /* Next in list of files        */
  DIRECTORY *dir;               /* -> Directory                 */
  char name[13];                /* Name of file in directory    */
  int hash;                     /* Hash key of name             */
};

struct _FLOWFILE
{
  FLOWFILE *next;               /* -> Next in chain             */
  long time;                    /* Timestamp                    */
  long size;                    /* Size of file in bytes        */
  char attr;                    /* Attribute                    */
  char name[13];                /* Name of file                 */
  int hash;                     /* Hash key of name[]           */
  FLOWENT *files;               /* List of files                */
};

struct _DIRBUF
{
  int cache;                    /* Flag. Using cached method?   */
  long time;                    /* New transfer area            */
  long size;
  char attr;
  char name[13];
  union
  {
    struct
    {                           /* If not using cached method   */
      DTA dta;                  /*      Next entry to be read   */
      int more;                 /*      Flag. Is dta valid?     */
      int cleanup;              /*      Flag. Call dfind(,,2)?  */
    }
    slow;
    struct
    {                           /* Cached method                */
      DIRECTORY *dir;           /*      Directory with hash tab */
      DIRENTRY *next;           /*      Next in hash chain      */
      int hash;                 /*      Current hash key        */
      char mask[12];
    }
    fast;
  }
  u;
};

#endif /* CACHE_H */
#endif

/* $Id: cache.h,v 1.2 1999/02/27 01:15:02 mr Exp $ */
