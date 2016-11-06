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
 * Filename    : $Source: E:/cvs/btxe/src/cache.c,v $
 * Revision    : $Revision: 1.14 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:42 $
 * State       : $State: Exp $
 * Orig. Author: MMP
 *
 * Description : Outbound Cache
 *
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * CACHE DATA STRUCTURE
 *
 * The ent[] array in DIRECTORY is a hash table with single chained lists
 * of directory entries with the same hash value. If KEEPORDER is defined,
 * an additional chain is maintained using DIRECTORY.list and DIRENTRY.dirnext.
 * This chain is used to chain the directory entries in the order they were
 * originally read. HPFS users will expect the outbound to be sorted like
 * they are used to.
 *
 *   dirlist
 *      |
 *      V                +-----------+  +-----------+  +-----------+
 *  +-----------------++>|DIRENT next|->|DIRENT next|->|DIRENT next|->NULL
 *  |DIRECTORY  ent[0]|' +-----------+  +-----------+  +-----------+
 *  |           ent[1]|->...
 *  |              :  |  +-----------+  +-----------+  +-----------+
 *  |  ent[HASHSIZE-1]|->|DIRENT next|->|DIRENT next|->|DIRENT next|->NULL
 *  | next        flow|, +-----------+  +-----------+  +-----------+
 *  +-----------------+| +-------------+  +-------------+
 *      |              +>|FLOWFILE next|->|FLOWFILE next|->NULL
 *      |                |    files    |  |    files    |
 *      |                +-------------+  +-------------+
 *      |                       |                | +------------+
 *      |                       |                }>|FLOWENT next|->...
 *      |                       |                  |     dir    |
 *      |                       |                  +------------+
 *      |                       |                         :
 *      |                       |
 *      |                       | +------------+  +------------+
 *      |                       +>|FLOWENT next|->|FLOWENT next|->NULL
 *      |                         |     dir    |  |     dir    |
 *      |                         +------------+  +------------+
 *      V                                |               |
 *  +-----------------+<-----------------|---------------+
 *  |DIRECTORY  ent[0]|->...             |
 *  |           ent[1]|->...             |
 *  |              :  |                  |
 *  |  ent[HASHSIZE-1]|->...             |
 *  | next        flow|->...             |
 *  +-----------------+                  |
 *      |                                |
 *      V                                |
 *  +-----------------+<-----------------+
 *  |DIRECTORY  ent[0]|->...
 *  |           ent[1]|->...
 *  |              :  |
 *  |  ent[HASHSIZE-1]|->...
 *  | next        flow|->...
 *  +-----------------+
 *      |
 *      V
 *     NULL
 *
 * The DIRECTORY structures are considered static; once a directory makes it
 * into the list, it is never removed again. The other structures are dynamic
 * in the sense, that they are deallocated and reused. They are not actually
 * freed, but inserted into free lists. In this way, we avoid heap manager
 * overhead, and reduce fragmentation. Structures are allocated in blocks of
 * BLOCKSIZE (50), and when the system is up and running, the memory usage is
 * likely to have stabilized, and the heap manager is no longer used. However,
 * if memory is exhausted, allocations can fail. Therefore, all allocations
 * are made by getmem() which turns the cache off, if an allocation error
 * occurs.
 *
 * The structures are dimensioned to DOS' 8.3 char filenames, and the cache
 * simply can't contain those names. Long names are handled specially in
 * cachestat() so the statistics will be correct, but dirread() will not
 * return them. This is no big problem, as Binkley is not looking for long
 * names, when scanning directories.
 *-----------------------------------------------------------------------------
 */

#include "includes.h"

#ifdef CACHE

static DIRECTORY *dirlist = NULL;  /* List of known directories        */
static DIRENTRY *defl = NULL;   /* Directory Entry Free List        */
static FLOWFILE *fffl = NULL;   /* Flow File Free List              */
static FLOWENT *fefl = NULL;    /* Flow Entry Free List             */

#define _C_DEBUG

/*-----------------------------------------------------------------------------
 * void *getmem(size_t size)
 *
 * size         Number of bytes to allocate
 *
 * Returns      Pointer to new memory on success; NULL on error
 *
 * This is a simple wrapper around to malloc() used for two purposes:
 *   1. Turn the cache off, if a memory allocation occurs
 *   2. Keep track of the memory usage for statistics
 *-----------------------------------------------------------------------------
 */

static void *
getmem (size_t size)
{
  void *p;

  if ((p = malloc (size)) == NULL)
  {
    cachehold = 0;
    status_line (MSG_TXT (M_NOMEM_FOR_OUTB_CACHE));
  }
  else
  {
    cachemem += size;
    memset (p, 0, size);        /* For dumping freelists in cachedump() */
  }

  return (p);
}


/*-----------------------------------------------------------------------------
 * void cacheflush(void)
 *
 * Flushes the cache for all directories. This will force dirload() to actually
 * reload the directory. cacheflush() does NOT remove the cached flow files,
 * because they can be reused after the directory has been loaded.
 *-----------------------------------------------------------------------------
 */

static void
dirflush (DIRECTORY * dir)
{
  DIRENTRY *ent;
  int i;

  if (dir->loaded)
  {
    for (i = 0; i < HASHSIZE; i++)
      while ((ent = dir->ent[i]) != NULL)
        dir->ent[i] = ent->next, ent->next = defl, defl = ent;
#ifdef KEEPORDER
    dir->list = NULL;
#endif
    dir->loaded = 0;
  }
}

void
cacheflush (void)
{
  DIRECTORY *dir;

#ifdef C_DEBUG
  cachedump ("cache.0");
#endif
  for (dir = dirlist; dir; dir = dir->next)
    dirflush (dir);
#ifdef C_DEBUG
  cachedump ("cache.1");
  status_line ("Mcacheflush() done");
#endif
}


/*-----------------------------------------------------------------------------
 * char *basename(char *path)
 *
 * path         Pathname
 *
 * Returns      Basename within path
 *
 * Returns a pointer to the basename within a pathname; that is a pointer to
 * the first character after the last drive or directory separator. If the
 * path does not contain any drive or directory seprators, path itself is
 * returned.
 *-----------------------------------------------------------------------------
 */

static char *
basename (char *path)
{
  char *a, *b;

  for (a = "\\:/"; *a; a++)
    if ((b = strrchr (path, *a)) != NULL)
      path = b + 1;

  return (path);
}


/*-----------------------------------------------------------------------------
 * DIRECTORY *getdir(char *name)
 *
 * name         Name of file
 *
 * Returns      Diretory structure on success; NULL on error
 *
 * This function is used for looking up known directories and add new ones. If
 * getdir() is able to find <name> in the known directory list, it simply
 * returns a pointer to the directory. If getdir() cannot find the directory,
 * it is added to the list, and a new (empty) directory structure is returned.
 * This addition may fail because of memory problems (getmem), in which case
 * NULL is returned.
 *-----------------------------------------------------------------------------
 */

static DIRECTORY *
getdir (char *name)
{
  char drive;                   /* Drive letter */
  long len;                     /* Lenght of path without drive */
  DIRECTORY *dir;               /* The one to return */
  DIRECTORY **dpp;              /* Used for rearanging list */
  int hit;                      /* Counter for debugging and statistics */

  /* Extract drive letter. The drive letter might no be present, in which
   * we use the current drive. The logic about the current drive, ensures
   * the the current drive is only obtained once; the operating system is
   * only bothered with a single call. */

  if (isalpha (name[0]) && name[1] == ':')
  {
    drive = (char) toupper (*name);
    name += 2;
  }
  else
  {
    static char curdrive = '\0';

    if (!curdrive)
    {
      char cwd[PATHLEN];

      getcwd (cwd, sizeof (cwd));
      curdrive = (char) toupper (*cwd);
    }
    drive = curdrive;
  }

  /* Get length of path. We use the length and memicmp() instead of
   * stricmp(). This has two advantanges: 1) it allows us to skip a
   * lot of string comparisons 2) we don't need to strip off the
   * filename in the name.
   */
  len = (long) (basename (name) - name);

  /* Check if directory is listed. If we can't find, we continue below
   * and creates a new entry for it. However, if we can find it, the
   * directory is move in front of the list. This speeds up the next
   * call to getdir() if we are searching for a file in the same
   * directory, which is most likely. Then we return the directory.
   */
  hit = 0;
  dpp = &dirlist;
  for (dir = *dpp; dir; dpp = &dir->next, dir = *dpp, hit++)
    if (drive == dir->drive && len == dir->len && !memicmp (dir->name, name,
                                                            (size_t) len))
    {
#ifdef C_DEBUG
      status_line ("MDIRECTORY %c:%.*s hit at %d", drive, len, name, hit);
#endif
      *dpp = dir->next;
      dir->next = dirlist;
      dirlist = dir;
      return (dir);
    }

  /* Allocate new directory node */
  if ((dir = (DIRECTORY *) getmem ((size_t) (sizeof (DIRECTORY) + len))) == NULL)
    return (NULL);

  /* Initalize new node, and insert in list */
#ifdef C_DEBUG
  status_line ("MDIRECTORY %c:%.*s not hit", drive, len, name);
#endif
  dir->flow = NULL;
  dir->loaded = 0;
  memset (dir->ent, 0, sizeof (dir->ent));  /* Reset hash table */
#ifdef KEEPORDER
  dir->list = NULL;
#endif
  dir->drive = drive;
  dir->len = (int) len;
  memcpy (dir->name, name, (size_t) len);
  dir->name[(int) len] = '\0';

  /* Insert in directory list */
  dir->next = dirlist;
  dirlist = dir;

  /* Return new directory node */
  return (dir);
}


/*-----------------------------------------------------------------------------
 * int strhash(char *str)
 *
 * str          String
 *
 * Returns      Hash value
 *
 * Computes a hash value in the range 0..HASHSIZE-1 for the string str. The
 * same hash key is generated for both lower and upper case strings.
 *-----------------------------------------------------------------------------
 */
#define NBITS   (sizeof(unsigned)<<3)  /* bits in an int (2*8 or 4*8) */
#define N75     ((unsigned)(((NBITS)*3)>>2))
#define N12     ((unsigned)((NBITS)>>3))
#define HB      (~((unsigned)(~0)>>N12))

/*
   ----------------------------------- 16 bits --------------------------
   nbits =  16 ==   2*8
   n75   =  12 == ((2*8)*3)/4 == 12
   N12   =  2  ==  (2*8)/8
   HB    =  ~(1111 1111 1111 1111 >> n12)
   =  ~(0011 1111 1111 1111)
   =  0xC000
   ----------------------------------- 32 bits --------------------------
   nbits =  32 == 4*8
   n75   =  24 == ((4*8)*3)/4 == 24
   n12   =   4 == (4*8)/8
   HB    =  ~(1111 1111 1111 1111 1111 1111 1111 1111 >> N12)
   =  ~(0000 1111 1111 1111 1111 1111 1111 1111)
   =  0xF0000000
   ----------------------------------------------------------------------
 */

static int
strhash (char *s)
{
  unsigned g, h = 0;

  while (*s)
  {
    h = (h << N12) + toupper (*s);
    s++;
    if ((g = (h & HB)) != 0)
      h = (h ^ (g >> N75)) & ~HB;
  }

  return ((int) (h % HASHSIZE));
}


/*-----------------------------------------------------------------------------
 * int isdosname(char *name)
 *
 * name         Basename
 *
 * Returns      Non-zero if 8.3, zero if not
 *
 * Checks if a filename, without path, is a valid DOS 8.3 name.
 *-----------------------------------------------------------------------------
 */

static int
isdosname (char *name)
{
  char *e;

  e = strchr (name, '.');
  if (e)
    return ((e - name) < 9 && strlen (e) < 5 && (e > name) && *name);
  return (strlen (name) < 9 && *name);
}


/*-----------------------------------------------------------------------------
 * int dirload(DIRECTORY *dir)
 *
 * dir          Directory
 * lazy         Flag. Use cache if available?
 *
 * Returns      0 on success; non-zero on memory problems.
 *
 * Loads a directory from the disk into the cache. If the directory is already
 * cached, this is a no-operation. Use cacheflush() first to force a reload of
 * directories. dirload() may fail because of memory problems, in which case a
 * non-zero value if returned.
 *-----------------------------------------------------------------------------
 */

static int
dirload (DIRECTORY * dir)
{
  DTA dta;
  DIRENTRY *ent;
  FLOWFILE *flow;
  FLOWFILE **flowp;
  FLOWENT *fent;

#ifdef KEEPORDER
  DIRENTRY *last;

#endif
  char path[PATHLEN];
  int j, cnt;
  int hash;                     /* Hash key for filename */

  /* Check if directory is already loaded */
  if (dir->loaded)
  {
#ifdef C_DEBUG
    status_line ("M%c:%s already loaded", dir->drive, dir->name);
#endif
    return (0);
  }

  /* Setup search path */
  sprintf (path, "%c:%s*.*", dir->drive, dir->name);

  /* Traverse directory. */
  cnt = j = 0;
#ifdef KEEPORDER
  dir->list = last = NULL;
#endif
  while (!dfind (&dta, path, j))
  {
    j = 1;
    if (isdosname (dta.name) && !(dta.attr & FA_VOLID))
    {

      /* Allocate DIRENTRY structure from the free list. If the free list
       * is empty, we expand it first. */
      if (defl == NULL)
      {
        int i;

#ifdef C_DEBUG
        status_line ("MExpanding freelist with DIRENTRY");
#endif
        if ((defl = getmem (BLOCKSIZE * sizeof (DIRENTRY))) == NULL)
          return (1);
        for (i = 0; i < BLOCKSIZE - 1; i++)
          defl[i].next = defl + i + 1;
        defl[i].next = NULL;
      }
      ent = defl;
      defl = defl->next;

      /* Initialize directory entry. The name is lower cased, because this
       * will simplify comparisons later when traversing the directory. */
      ent->time = dta.time;
      ent->attr = dta.attr;
      ent->size = dta.size;
      strcpy (ent->name, dta.name);
      strlwr (ent->name);       /* Better now, than when comparing */

      /* Insert in hash table of directory */
      hash = strhash (ent->name);
      ent->next = dir->ent[hash];
      dir->ent[hash] = ent;

      /* Insert in single chained list */
#ifdef KEEPORDER
      ent->dirnext = NULL;
      if (last)
        last->dirnext = ent;
      else
        dir->list = ent;
      last = ent;
#endif
      cnt++;
    }
  }

#ifdef C_DEBUG
  status_line ("Mdirload(%s) found %d files", path, cnt);

  /* Dump hash table */
  for (hash = 0; hash < HASHSIZE; hash++)
  {
    for (ent = dir->ent[hash]; ent; ent = ent->next)
      status_line ("M    dir[\"%s\"].ent[%d], %s", dir->name, hash, ent->name);
  }

  /* Dump single chained list */
  for (ent = dir->list; ent; ent = ent->next)
  {
    status_line ("M    %s%s", dir->name, ent->name);
  }
#endif

  /* Clean up */
  if (j)
    dfind (&dta, NULL, 2);

  /* Set time of loading and return */
  dir->loaded = unix_time (NULL);

  /* Invalidate cached flow files */
  flowp = &dir->flow, j = 0;
  while ((flow = *flowp) != NULL)
  {
#ifdef C_DEBUG
    status_line ("Mdirload(): check %s", flow->name);
#endif

    ent = dir->ent[flow->hash];
    while (ent && strcmp (ent->name, flow->name))
      ent = ent->next;

    if (ent == NULL ||
        ent->time != flow->time ||
        ent->size != flow->size ||
        ent->attr != flow->attr)
    {
#ifdef C_DEBUG
      status_line ("M%s%s has %s", dir->name, flow->name, ent ? "changed" :
                   "disappeared");
#endif

      while ((fent = flow->files) != NULL)
      {
        flow->files = fent->next;
        fent->next = fefl;
        fefl = fent;
      }

      *flowp = flow->next;
      flow->next = fffl;
      fffl = flow;
      j++;
    }
    else
    {
      flowp = &flow->next;
    }
  }

#ifdef C_DEBUG
  if (j)
  {
    status_line ("M%d flow files invalidated. Dumping cache", j);
    cachedump ("cache.inv");
  }
#endif

  return (0);
}


/*-----------------------------------------------------------------------------
 * void dirinit(DIRBUF *buf, char *fspec)
 *
 * buf          Directory buffer
 * fspec        File specification
 *
 * Initializes reading of a directory. After the setup, the directory entries
 * can actually be read using dirread().
 *
 * dirinit() and dirread() implements the optional caching of directories. If
 * caching is turned off, they are simple wrappers around dfind(). If caching
 * is turned on, dirinit() sets up a list of directory entries by loading the
 * directory, or simple initializes the pointer to an already loaded directory
 * list.
 *-----------------------------------------------------------------------------
 */

void
dirinit (DIRBUF * buf, char *fspec)
{
  DIRECTORY *dir;
  int len;
  char *src, *srcext, *dest;

  if (!cachehold ||
      (dir = getdir (fspec)) == NULL ||
      (!dir->loaded && dirload (dir)))
  {
    /* Better be safe and clear the DTA structure. Originally, it was
     * initialized to {0} everywhere, eventhough i can se no reason for
     * that */
    buf->cache = 0;
    memset (&buf->u.slow.dta, 0, sizeof (DTA));
    buf->u.slow.more = buf->u.slow.cleanup = !dfind (&buf->u.slow.dta,
                                                     fspec, 0);
    return;
  }

  /* Initialize cached traversal */
  buf->cache = 1;
  buf->u.fast.dir = dir;
#ifdef KEEPORDER
  buf->u.fast.next = dir->list;
#else
  buf->u.fast.hash = 0;
  buf->u.fast.next = dir->ent[0];
#endif

  /* Format mask. dirread() depends on 11 character with space padding,
   * '*' expanded to '?'s, and upper case letters.
   */
  src = basename (fspec), srcext = strchr (src, '.'), len = strlen (src);
  strcpy (buf->u.fast.mask, "           ");  /* 11 spaces */
  if (srcext == NULL)
    srcext = "???";
  else
    srcext++;
  len = 0, dest = buf->u.fast.mask;
  while (*src && *src != '.' && len < 8)
  {
    if (*src == '*')
      *dest++ = '?';
    else
      *dest++ = *src++;
    len++;
  }
  len = 0, dest = buf->u.fast.mask + 8;
  while (*srcext && len < 3)
  {
    if (*srcext == '*')
      *dest++ = '?';
    else
      *dest++ = *srcext++;
    len++;
  }
  strupr (buf->u.fast.mask);
}


/*-----------------------------------------------------------------------------
 * int fmatch(char *name, char *mask)
 *
 * name         Filename without path
 * mask         Filemask without path
 *
 * Returns      Non-zero if match; zero if mismatch
 *
 * Compares the name with the mask, and returns the result. This routine
 * assumes that the name is no longer than 12 characters.
 *-----------------------------------------------------------------------------
 */

static int
fmatch (char *name, char *mask)
{
  char nbuf[12];
  char *e;
  int len;

  len = strlen (name);
  e = strchr (name, '.');
  strcpy (nbuf, "           "); /* 11 spaces */
  if (e)
  {
    if (e > name + 8)
      return (0);
    memcpy (nbuf, name, (size_t) (e - name));
    e++, len = strlen (e);
    memcpy (nbuf + 8, e, len);
  }
  else
  {
    memcpy (nbuf, name, len);
  }
  strupr (nbuf);

  /* Match filename */
  name = nbuf;
  while (*name && *mask)
  {
    if (*mask != '?' && *mask != *name)
      return (0);
    name++, mask++;
  }

  return (*name == '\0' && *mask == '\0');
}


/*-----------------------------------------------------------------------------
 * int dirread(DIRBUF *buf)
 *
 * buf          Directory buffer
 *
 * Returns      0 on success; non-zero on error
 *
 * Reads the next entry from the directory buffer. If there are no more
 * available entries, a non-zero value is returned. If zero is returned,
 * the entry is returned in the DIRBUF fields time, attr, size, and name
 * fields.
 *-----------------------------------------------------------------------------
 */

int
dirread (DIRBUF * buf)
{
  int res;

  if (!buf->cache)
  {
    res = !buf->u.slow.more;
    if (!res)
    {
      buf->attr = buf->u.slow.dta.attr;
      buf->size = buf->u.slow.dta.size;
      buf->time = buf->u.slow.dta.time;
      strcpy (buf->name, buf->u.slow.dta.name);
      buf->u.slow.more = !dfind (&buf->u.slow.dta, NULL, 1);
    }
  }
  else
  {
#ifdef KEEPORDER
    DIRENTRY *ent;

    res = 1;
    while ((ent = buf->u.fast.next) != NULL)
    {
      buf->u.fast.next = ent->dirnext;
      if (fmatch (ent->name, buf->u.fast.mask))
      {
        buf->attr = ent->attr;
        buf->size = ent->size;
        buf->time = ent->time;
        strcpy (buf->name, ent->name);
        res = 0;
        break;
      }
    }
#else
    DIRENTRY *ent;
    DIRECTORY *dir;
    int hash;

    hash = buf->u.fast.hash;
    dir = buf->u.fast.dir;
    res = 1;
    while (res && hash < HASHSIZE)
    {
      while ((ent = buf->u.fast.next) != NULL)
      {
        buf->u.fast.next = ent->next;
        if (fmatch (ent->name, buf->u.fast.mask))
        {
          buf->attr = ent->attr;
          buf->size = ent->size;
          buf->time = ent->time;
          strcpy (buf->name, ent->name);
          res = 0;
          break;
        }
      }
      if (res)
      {
        if (hash++ < HASHSIZE)
          buf->u.fast.next = dir->ent[hash];
        else
          buf->u.fast.next = NULL;
      }
    }
    buf->u.fast.hash = hash;
#endif
  }

  return (res);
}


/*-----------------------------------------------------------------------------
 * void dirquit(DIRBUF *buf, int flush)
 *
 * buf          Directory buffer
 * flush        Flag. Flush directory?
 *
 * Returns      -
 *
 * Deallocate any memory neccesary when stopping reading of a directory.
 *-----------------------------------------------------------------------------
 */

void
dirquit (DIRBUF * buf, int flush)
{
  if (!buf->cache)
  {
    if (buf->u.slow.cleanup)
      dfind (&buf->u.slow.dta, NULL, 2);
  }
  else
  {
    if (flush)
      dirflush (buf->u.fast.dir);
  }
}


/*-----------------------------------------------------------------------------
 * time_t dos2unix(unsigned long dostime)
 *
 * time         DOS file time stamp
 *
 * Returns      Unix style time stamp
 *
 * Converts the timestamp to a unix style julian
 *-----------------------------------------------------------------------------
 */

static time_t
dos2unix (unsigned long dostime)
{
  struct tm t;

  t.tm_sec = (int) (dostime & 0x1FUL) * 2;
  dostime >>= 5UL;
  t.tm_min = (int) (dostime & 0x3FUL);
  dostime >>= 6UL;
  t.tm_hour = (int) (dostime & 0x1FUL);
  dostime >>= 5UL;
  t.tm_mday = (int) (dostime & 0x1FUL);
  dostime >>= 5UL;
  t.tm_mon = (int) (dostime & 0x0FUL) - 1;
  dostime >>= 4UL;
  t.tm_year = (int) (dostime + 80);  /* TJW 971020 bugfix */
  t.tm_wday = t.tm_yday = t.tm_isdst = -1;
  return (unix_mktime (&t));
}

#endif /* CACHE */

/*-----------------------------------------------------------------------------
 * int cachestat(char *path, struct stat *buf, DIRECTORY *dirp, DIRENTRY **entp,
 *               int freqflag, long freqfilesize);
 *
 * path         Pathname of file
 * buf          stat buffer
 * dirp         Where to store directory, or NULL
 * dp           Where to store pointer to directory entry, or NULL
 * freqfilesize if != 0 : size of file to be frequested
 *
 * Returns      0 on success; non-zero on error
 *
 * stat() replacement that takes advantage of the information stored in the
 * cached directories. If cachestat() can find the entry in the cache, only
 * the st_size, st_mtime, and the directory attribute of the st_mode field
 * are guaranteed to be be valid.
 *
 * If (dp!=NULL), a pointer to the found cached entry is returned in *dp, if
 * found, otherwise *dp is set to NULL. cachestat() may run out of memory, in
 * which case cachehold will be reset.
 *-----------------------------------------------------------------------------
 */

int
cachestat (char *path, struct stat *buf,
#ifdef CACHE
           DIRECTORY ** dirp, DIRENTRY ** entp,
#endif
           int freqflag, long freqfilesize)
{
#ifdef CACHE
  DIRECTORY *dir;
  DIRENTRY *ent;
  char *base;
  int hit;

  /* Return NULL's */
  if (dirp)
    *dirp = NULL;
  if (entp)
    *entp = NULL;
#endif /* CACHE */

  if (freqflag)
  {                             /* keep this short, this is a freq */
    buf->st_size = freqfilesize;
#ifdef __WATCOMC__              /*  TS 960901 seems only Watcom does it like this  */
    buf->st_attr = 0;
#else /*  anybody else except Watcom  */
    buf->st_mode = 0;
#endif
    buf->st_mtime = unix_time (NULL);
    buf->st_ctime = buf->st_mtime;
    buf->st_atime = buf->st_mtime;
    return (0);
  }

#ifdef CACHE
  /* Get directory structure */
  if (!cachehold || (dir = getdir (path)) == NULL)
    goto fallback;
  if (dirp)
    *dirp = dir;

  /* Load directory if neccesary */
  if (!dir->loaded && dirload (dir))
    goto fallback;

  /* Get basename, and check if it is not an 8.3 name (can't cache them) */
  base = basename (path);
  if (!isdosname (base))
    goto fallback;

  /* Search directory for a match */
  for (hit = 0, ent = dir->ent[strhash (base)]; ent; ent = ent->next, hit++)
  {
    if (!stricmp (base, ent->name))
    {
      if (entp)
        *entp = ent;
      buf->st_size = ent->size;
#ifdef __WATCOMC__              /*  TS 960901 seems only Watcom does it like this  */
      buf->st_attr = 0;
#else /*  anybody else except Watcom  */
      buf->st_mode = 0;
#endif
      buf->st_mtime = dos2unix (ent->time);
      buf->st_ctime = buf->st_mtime;
      buf->st_atime = buf->st_mtime;
      if (ent->attr & FA_SUBDIR)
#ifdef __WATCOMC__              /*  TS 960901 seems only Watcom does it like this  */
        buf->st_attr |= S_IFDIR;
#else /*  anybody else except Watcom  */
        buf->st_mode |= S_IFDIR;
#endif
#ifdef C_DEBUG
      status_line ("M%s%s hit at %d", dir->name, ent->name, hit);
#endif
      return (0);
    }
  }

  /* It wasn't there - report that */
#ifdef C_DEBUG
  status_line ("M%s not found in cache", path);
#endif
  errno = ENOENT;
  return (-1);

fallback:
#endif /* CACHE */

  return (unix_stat (path, buf));
}

#ifdef CACHE

/*-----------------------------------------------------------------------------
 * int isarc(char *name)
 *
 * name         Filename, upper case
 *
 * Returns      1 if ArcMail bundle; 0 if not
 *
 * Checks if the filename designates an ArcMail bundle or not
 *-----------------------------------------------------------------------------
 */

static int
isarc (char *name)
{
  static char week[] = "MOTUWETHFRSASU";
  int i;

  if (strlen (name) != 12)
    return (0);

  for (i = 0; i < 8; i++)
    if (!isxdigit (name[i]))
      return (0);

  if (name[i++] != '.')
    return (0);

  if (!isdigit (name[11]))
    return (0);

  for (i = 0; i < 14; i += 2)
    if (name[9] == week[i] && name[10] == week[i + 1])
      break;

  return (i < 14);
}

#endif /* CACHE */

/*-----------------------------------------------------------------------------
 * long netsize(MAILP p,char *extension)
 *
 * p            Mail node
 *
 * Returns      Bytes waiting
 *
 * This is a replacement or reimplementation of netsize found in mailovly.c,
 * that takes advantage of the cache.
 *-----------------------------------------------------------------------------
 */

long
netsize (MAILP p, char *ext, unsigned short AttachType)
{
  struct stat stbuf;
  char net_path[PATHLEN];
  char *ptr;
  FILE *fp;
  char *q;

#ifdef CACHE
  char *base = NULL;
  int cachethis;
  DIRECTORY *flowdir;
  DIRECTORY *dir = NULL;
  DIRENTRY *ent;
  FLOWFILE *flow = NULL;
  FLOWENT *fent;

#endif /* CACHE */
  unsigned long holdtime;
  unsigned short holdcnt;
  long holdsize;
  int freqflag;
  long freqfilesize;

  if (!p)
    return (0);

  /* Append the ARCmail file name to the path line  */

  sprintf (net_path, "%s%s.%s", HoldAreaNameMunge (&(p->mail_addr)),
           Hex_Addr_Str (&(p->mail_addr)), ext);

#ifdef CACHE
  /* Set cache flag */
  cachethis = (cachehold > 1);
#endif /* CACHE */

  /* TJW 960809 Is this a REQ file ? */
  freqflag = !strnicmp (ext, "req", 3);

  /* Reset counters */
  holdtime = 4294967295UL;
  holdcnt = 0;
  holdsize = 0;

#ifdef CACHE
  /* Try to locate flow file in cache */
  if (!freqflag && cachethis)
  {                             /* !freqflag for debugging */
    base = basename (net_path);
    if ((dir = getdir (net_path)) != NULL)
      for (flow = dir->flow; flow; flow = flow->next)
        if (!stricmp (base, flow->name))
        {
#ifdef C_DEBUG
          status_line ("Mfound %s in cache", flow->name);
#endif
          for (fent = flow->files; fent; fent = fent->next)
          {
            flowdir = fent->dir;
            if (!flowdir)
            {
              cachethis = 0;
              holdsize = 0;
              holdcnt = 0;
              goto fallback;
            }
            if (!flowdir->loaded && dirload (flowdir))
            {
              cachethis = 0;
              holdsize = 0;
              holdcnt = 0;
              goto fallback;
            }
            ent = flowdir->ent[fent->hash];
            while (ent && strcmp (ent->name, fent->name))
              ent = ent->next;
            if (ent)
            {
              /* CFS 970504 */
              if (!AttachType || (AttachType & get_filetype (ent->name)))
              {
                holdcnt++;
                holdsize += ent->size;
              }
              if (isarc (ent->name))
              {
                if ((unsigned long) ent->time < holdtime)
                  holdtime = ent->time;
              }
            }
          }
          if (holdtime != 4294967295UL)
          {
            holdtime = dos2unix (holdtime);
            if (holdtime < (unsigned long) p->oldest)
              p->oldest = holdtime;
          }
          p->numfiles += holdcnt;
          return (holdsize);
        }
  }

fallback:
#endif /* CACHE */

  /* Open file */
  if ((fp = share_fopen (net_path, read_binary, DENY_NONE)) == NULL)
    return (0);

#ifdef CACHE
  /* Initialize FLOWFILE structure */
  if (!freqflag && cachethis)
  {                             /* !freqflag for debugging */
    /* Get information on the flow file */
    if (cachestat (net_path, &stbuf, NULL, &ent, 0, 0))
      return (0);
    /* Allocate FLOWFILE structure from the free list. If the free list
     * is empty, we expand it first.
     */
    if (fffl == NULL)
    {
      if ((fffl = getmem (BLOCKSIZE * sizeof (FLOWFILE))) != NULL)
      {
        int i;

        for (i = 0; i < BLOCKSIZE - 1; i++)
          fffl[i].next = fffl + i + 1;
        fffl[i].next = NULL;
      }
    }
    if (fffl)
    {
      flow = fffl;
      fffl = flow->next;
      strntcpy (flow->name, base, 13);
      strlwr (flow->name);
      flow->hash = strhash (flow->name);
      flow->time = ent->time;
      flow->size = ent->size;
      flow->attr = ent->attr;
      flow->files = NULL;
      /* Insert into directory structure */
      flow->next = dir->flow;
      dir->flow = flow;
#ifdef C_DEBUG
      status_line ("MRegistered %s%s", dir->name, dir->flow->name);
#endif
    }
    else
    {
      cachethis = 0;
    }
  }
#endif /* CACHE */

  while (!feof (fp))
  {
    /*
     *      Make sure of a nice zero there if we're at an undetected EOF.
     *      Then try to read a line from the file. MMP - also make sure to
     *      NUL-terminate the line if it is too long.
     */
    net_path[0] = net_path[sizeof (net_path) - 1] = '\0';
    fgets (net_path, sizeof (net_path) - 1, fp);

    freqfilesize = 0;           /* init size */

    if (freqflag)
    {

      /*
       *      Get file sizes out of request file:
       *      NNNNnnnn.REQ:
       *      FILENAME.123 !PASSWORD $FILESIZEBYTES
       *      ...
       */
      q = ptr = net_path;
      if (!*ptr)
        continue;               /* nothing there */
      else if (*ptr == '+')     /* function request ? */
        ptr++;
      q = skip_to_blank (ptr);
      if (*q)                   /* if there is anything left */
        *q++ = '\0';            /* 0-terminate filename */
      while (*q && *q != '$')
        q = skip_to_word (q);
      if (*q == '$')            /* do we have size information ? */
        freqfilesize = atol (++q);  /* get size */
#ifdef C_DEBUG
      status_line ("MFreq %s size %ld", ptr, freqfilesize);
#endif
    }
    else
    {
      /*
       * Clean up anything we don't want to see (blanks, tabs, CR, etc)
       */
      for (q = ptr = net_path; *q; q++)
        if (*q <= ' ')
          *q = '\0';
      /*
       * File disposition commands should be skipped over to get to
       * actual filenames.
       */
      if ((*ptr == TRUNC_AFTER) ||
          (*ptr == DELETE_AFTER) ||
          (*ptr == SHOW_DELETE_AFTER) ||
          (*ptr == NOTHING_AFTER))
        ptr++;
      /*
       *  Now -- if what's left starts with a semicolon, it's a comment.
       *  If it starts with a tilde, the file has already been sent. If
       *  what we see is a zero, there's nothing on the line. In any one
       *  of these cases, we should skip this line.
       */
      if ((*ptr == '\0') ||
          (*ptr == ';') ||
          (*ptr == FILE_ALREADY_SENT))
        continue;
    }

    /*
     *  Get the file size by doing a stat call. If it's not there
     *  then we obviously need not add any size in. If the file is
     *  arcmail, figure it into the "oldest" calculation.
     */

    if (!AttachType || (AttachType & get_filetype (ptr)))
    {

#ifdef CACHE
      if (cachestat (ptr, &stbuf, &flowdir, NULL, freqflag, freqfilesize))
#else
      if (cachestat (ptr, &stbuf, freqflag, freqfilesize))
#endif
        /* file exist? */
        continue;
      else
      {
        if (!freqflag)          /* REQs don't count as hold file, but */
          holdcnt++;
        holdsize += stbuf.st_size;  /* this also includes FREQ traffic now */

#ifdef CACHE
        /* Register file */
        if (!freqflag && cachethis)
        {                       /* !freqflag for debugging */
          if (!isdosname (base = basename (ptr)))
          {
            status_line ("MNot DOS 8.3 name: %s", ptr);
            /* The file is not a DOS 8.3 name. Our structures are not
             * dimensioned for anything else, so we cant cache this
             * particular file. Clear the dos flag, and deallocate
             */
            while ((fent = flow->files) != NULL)
            {
              flow->files = fent->next;
              fent->next = fefl;
              fefl = fent;
            }

            dir->flow = flow->next;
            flow->next = fffl;
            fffl = flow;
            cachethis = 0;
          }
          else
          {
            /* Expand free list */
            if (fefl == NULL && (fefl = getmem (BLOCKSIZE * sizeof
                                                (FLOWENT))) != NULL)
            {
              int i;

              for (i = 0; i < BLOCKSIZE - 1; i++)
                fefl[i].next = fefl + i + 1;
              fefl[i].next = NULL;
            }

            if (fefl)
            {
              /* Allocate node */
              fent = fefl;
              fefl = fent->next;
              /* Register file */
              fent->dir = flowdir;
              strcpy (fent->name, base);
              strlwr (fent->name);
              fent->hash = strhash (fent->name);
              /* Insert in chain */

              fent->next = flow->files;
              flow->files = fent;
            }
          }
        }
#endif /* CACHE */

        if (!freqflag && is_arcmail (ptr, 1 - strlen (ptr)))
        {
          if ((unsigned long) stbuf.st_mtime < (unsigned long) holdtime)
            holdtime = stbuf.st_mtime;
        }
      }
    }
  }

  fclose (fp);

  /* Summarize */
  p->numfiles += holdcnt;
  p->oldest = min ((unsigned long) holdtime, p->oldest);

  return (holdsize);
}

#ifdef CACHE

/*-----------------------------------------------------------------------------
 * int cachedump(char *path)
 *
 * path         Pathname of output file
 *
 * Returns      0 on success; non-zero on i/o error
 *
 * Dumps the contents of the cache to the file specified
 *-----------------------------------------------------------------------------
 */

#ifdef C_DEBUG                  /* DEBUGGING */
static char *
timestr (time_t t)
{
  static char buf[20];

  strftime (buf, 20, "%y-%m-%d %H:%M:%S", unix_localtime (&t));
  return (buf);
}

static char *
ptrstr (void *p)
{
  static char buf[20];

  if (p)
    sprintf (buf, "*%p", p);
  else
    strcpy (buf, "NULL");
  return (buf);
}

int
cachedump (char *path)
{
  FILE *fp;
  DIRECTORY *dir;
  DIRENTRY *ent;
  FLOWFILE *flow;
  FLOWENT *fent;
  int hash;

  if ((fp = fopen (path, write_ascii)) == NULL)
    return (errno);

  for (dir = dirlist; dir; dir = dir->next)
  {

    fprintf (fp, "DIRECTORY at %s\n", ptrstr (dir));
    fprintf (fp, "    next    %s\n", ptrstr (dir->next));
    fprintf (fp, "    flow    %s\n", ptrstr (dir->flow));
#ifdef KEEPORDER
    fprintf (fp, "    list    %s\n", ptrstr (dir->list));
#endif
    for (hash = 0; hash < HASHSIZE; hash++)
      if (dir->ent[hash])
        fprintf (fp, "    ent[%02d] %s\n", hash, ptrstr (dir->ent[hash]));
    fprintf (fp, "    loaded  0x%08X (%s)\n", dir->loaded, timestr (dir->loaded));
    fprintf (fp, "    len     %d\n", dir->len);
    fprintf (fp, "    drive   '%c'\n", dir->drive);
    fprintf (fp, "    name    \"%s\"\n", dir->name);
    fprintf (fp, "\n");

    for (flow = dir->flow; flow; flow = flow->next)
    {
      fprintf (fp, "    FLOWFILE at %s\n", ptrstr (flow));
      fprintf (fp, "        next    %s\n", ptrstr (flow->next));
      fprintf (fp, "        time    0x%08X (%s)\n", flow->time, timestr
               (dos2unix (flow->time)));
      fprintf (fp, "        size    %ld\n", flow->size);
      fprintf (fp, "        attr    0x%02X\n", flow->attr);
      fprintf (fp, "        name    \"%s\"\n", flow->name);
      fprintf (fp, "        hash    %d\n", flow->hash);
      fprintf (fp, "        files   %s\n", ptrstr (flow->files));
      for (fent = flow->files; fent; fent = fent->next)
      {
        fprintf (fp, "        FLOWENT at %s\n", ptrstr (fent));
        fprintf (fp, "            next    %s\n", ptrstr (fent->next));
        fprintf (fp, "            dir     %s (\"%c:%s\")\n", ptrstr
                 (fent->dir), fent->dir->drive, fent->dir->name);
        fprintf (fp, "            name    \"%s\"\n", fent->name);
        fprintf (fp, "            hash    %d\n", fent->hash);
      }
      fprintf (fp, "\n");
    }

    for (hash = 0; hash < HASHSIZE; hash++)
      if (dir->ent[hash])
      {
        fprintf (fp, "    Hash chain ent[%d] starting at %s\n", hash, ptrstr
                 (dir->ent[hash]));
        for (ent = dir->ent[hash]; ent; ent = ent->next)
        {
          fprintf (fp, "        DIRENTRY at %s\n", ptrstr (ent));
          fprintf (fp, "            next    %s\n", ptrstr (ent->next));
#ifdef KEEPORDER
          fprintf (fp, "            dirnext %s\n", ptrstr (ent->dirnext));
#endif
          fprintf (fp, "            time    0x%08X (%s)\n", ent->time,
                   timestr (dos2unix (ent->time)));
          fprintf (fp, "            size    %ld\n", ent->size);
          fprintf (fp, "            attr    0x%02X\n", ent->attr);
          fprintf (fp, "            name    \"%s\"\n", ent->name);
        }
      }
    fprintf (fp, "\n");
  }
  fprintf (fp, "\n");

  fprintf (fp, "DIRENTRY freelist starting at %s\n", ptrstr (defl));
  for (ent = defl; ent; ent = ent->next)
  {
    fprintf (fp, "    DIRENTRY at %s\n", ptrstr (ent));
    fprintf (fp, "        next    %s\n", ptrstr (ent->next));
#ifdef KEEPORDER
    fprintf (fp, "        dirnext %s\n", ptrstr (ent->dirnext));
#endif
    fprintf (fp, "        time    0x%08X (%s)\n", ent->time, timestr
             (dos2unix (ent->time)));
    fprintf (fp, "        size    %ld\n", ent->size);
    fprintf (fp, "        attr    0x%02X\n", ent->attr);
    fprintf (fp, "        name    \"%s\"\n", ent->name);
  }
  fprintf (fp, "\n");

  fprintf (fp, "FLOWFILE freelist starting at %s\n", ptrstr (fffl));
  for (flow = fffl; flow; flow = flow->next)
  {
    fprintf (fp, "    FLOWFILE at %s\n", ptrstr (flow));
    fprintf (fp, "        next    %s\n", ptrstr (flow->next));
    fprintf (fp, "        time    0x%08X (%s)\n", flow->time, timestr
             (dos2unix (flow->time)));
    fprintf (fp, "        size    %ld\n", flow->size);
    fprintf (fp, "        attr    0x%02X\n", flow->attr);
    fprintf (fp, "        name    \"%s\"\n", flow->name);
    fprintf (fp, "        hash    %d\n", flow->hash);
    fprintf (fp, "        files   %s\n", ptrstr (flow->files));
  }
  fprintf (fp, "\n");

  fprintf (fp, "FLOWENT freelist starting at %s\n", ptrstr (fefl));
  for (fent = fefl; fent; fent = fent->next)
  {
    fprintf (fp, "    FLOWENT at %s\n", ptrstr (fent));
    fprintf (fp, "        next    %s\n", ptrstr (fent->next));
    if (fent->dir)
      fprintf (fp, "        dir     %s (\"%c:%s\")\n", ptrstr (fent->dir),
               fent->dir->drive, fent->dir->name);
    else
      fprintf (fp, "        dir     %s\n", ptrstr (fent->dir));
    fprintf (fp, "        name    \"%s\"\n", fent->name);
    fprintf (fp, "        hash    %d\n", fent->hash);
  }

  if (fclose (fp))
    return (errno);

  return (0);
}

#endif /* DEBUGGING */
#endif /* CACHE */

/* $Id: cache.c,v 1.14 1999/03/23 22:28:42 mr Exp $ */
