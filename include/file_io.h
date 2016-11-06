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
 * Filename    : $Source: E:/cvs/btxe/include/file_io.h,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:09 $
 * State       : $State: Exp $
 *
 * Description : File I/O definitions
 *
 *---------------------------------------------------------------------------*/

/* some defines for fopen */

#define read_binary        "rb"
#define read_binary_plus   "rb+"
#define write_binary       "wb"
#define write_binary_plus  "wb+"
#define append_binary      "ab"
#define append_binary_plus "ab+"

#ifdef __IBMC__
#define read_ascii         "r"
#define read_ascii_plus    "r+"
#define write_ascii        "w"
#define write_ascii_plus   "w+"
#define append_ascii       "a"
#define append_ascii_plus  "a+"
#else
#define read_ascii         "rt"
#define read_ascii_plus    "rt+"
#define write_ascii        "wt"
#define write_ascii_plus   "wt+"
#define append_ascii       "at"
#define append_ascii_plus  "at+"
#endif


// Most of them are not used, so I disabled them.
// If you need to use them, please also add
// appropriate code to dfind/linux in file_dos.c
// CEH; 19981108

// #define FA_RDONLY   0x01        /* File is readonly */
// #define FA_HIDDEN   0x02        /* File is hidden   */
// #define FA_SYSTEM   0x04        /* System file      */
#define FA_VOLID    0x08        /* Volume ID        */
#define FA_SUBDIR   0x10        /* Subdirectory     */
// #define FA_ARCH     0x20        /* Archive file     */

#if defined(OS_2)
# define MAXFILENAME CCHMAXPATH
#elif defined(_WIN32)
# define MAXFILENAME MAXPATH
#else
# define MAXFILENAME 12
#endif

#if defined (OS_2)

struct FILEINFOinternalbuf
{
  ULONG nDir;                   /* pointer to next structure */
  unsigned short create_date;   /* date of file creation */
  unsigned short create_time;   /* time of file creation */
  unsigned short access_date;   /* date of last access */
  unsigned short access_time;   /* time of last access */
  unsigned short wr_date;       /* date of last write */
  unsigned short wr_time;       /* time of last write */
  ULONG size;                   /* file size (end of data) */
  ULONG falloc_size;            /* file allocated size */
  ULONG attrib;                 /* attributes of the file */
  UCHAR string_len;             /* returned length of ascii name str. (length does not include null byte) */
  char name[CCHMAXPATH];        /* name string */
};

#elif defined (_WIN32)

#if !defined(_MSC_VER) && !defined(__WATCOMC__)  /* NS130599 */
/* Stas Mekhanoshin 160999: also WATCOM */


struct FILEINFOinternalbuf      /* originally : wfd */
{
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD dwReserved0;
  DWORD dwReserved1;
  TCHAR cFileName[MAXPATH];
  TCHAR cAlternateFileName[14];
};

#endif

#endif


#if defined (__unix__)

struct FILEINFOinternal
{
  char dname[512];              /* used to store name of scanned directory */
  char fpat[256];               /* pattern to match */
  int isopen;                   /* already dirclose()ed? */
  DIR *dir;
};

#elif defined (OS_2)

struct FILEINFOinternal
{
  struct FILEINFOinternalbuf ffbuf;
  HDIR hDirA;
  ULONG cSearch;
  ULONG usAttrib;
  ULONG infolevel;
  APIRET findrc;
};

#elif defined (_WIN32)

struct FILEINFOinternal
{
#if defined(_MSC_VER) || defined(__WATCOMC__)  /* NS130599 */
  /* Stas Mekhanoshin 160999: also watcom */
  struct _WIN32_FIND_DATAA ffbuf;
#else
  struct FILEINFOinternalbuf ffbuf;
#endif
  HANDLE hDirA;
  ULONG cSearch;
  ULONG usAttrib;
  ULONG infolevel;
  BOOL findrc;
};

#elif defined (DOS16)

struct FILEINFOinternal
{
  char rsvd[21];
  char attr;
  long time;
  long size;
  char name[MAXFILENAME];
  char nill;
};

#else
# error Unknown System?
#endif

struct FILEINFO
{
  struct FILEINFOinternal fii;  /* system-specific data (see above) */
  char attr;                    /* only used are FA_VOLID and VA_SUBDIR */
  long time;
  long size;
  char name[256];
};

#define DENY_ALL   0
#define DENY_NONE  1
#define DENY_READ  2
#define DENY_RDWR  3
#define DENY_WRITE 4

/* $Id: file_io.h,v 1.5 1999/09/27 20:51:09 mr Exp $ */
