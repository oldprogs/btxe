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
 * Filename    : $Source: E:/cvs/btxe/include/includes.h,v $
 * Revision    : $Revision: 1.16 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/30 17:50:06 $
 * State       : $State: Exp $
 *
 * Description : Common Include Files for BinkleyTerm
 *
 * Note        :
 *   This file drives all the defines and includes in BinkleyTerm. In
 *   here we handle cross-compiler portability issues. It was originally
 *   born to make compiled headers (a neat feature of Borland C) work,
 *   but the other role quickly caught on as well.
 *
 *   Some of the portability stuff will no doubt wind up in other Bink
 *   include files -- but the system-level stuff such as utime, etc
 *   is all dealt with here.
 *
 *   All main Bink modules should include this file before any specific
 *   local includes.
 *
 *---------------------------------------------------------------------------*/

/* System include files */

#ifdef _WIN32
# include <windows.h>
#endif

#ifndef __unix__
# include <io.h>
# include <share.h>
# include <conio.h>
# include <process.h>
# define DIR_SEPC '\\'
# define DIR_SEPS "\\"
# define f_fancy_str(a) fancy_str(a)
# define f_strupr(a) strupr(a)
# define f_toupper(a) toupper(a)
# define MATCHALL "*.*"
#else
# include <unistd.h>
# include <sys/ioctl.h>
# include <sys/vfs.h>
# include <termios.h>
# include <fnmatch.h>
# define DIR_SEPC '/'
# define DIR_SEPS "/"
# define f_fancy_str(a)
# define f_strupr(a)
# define f_toupper(a) (a)
# define MATCHALL "*"
#endif

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef __IBMC__                /* CSET2 */
#ifndef __unix__
# include <dos.h>
#endif
#endif

#include <errno.h>
#include <time.h>
#include <fcntl.h>


/*
 * Compiler-specific stuff. The baseline in this sucker is
 * Microsoft C 6.00a. Below what you see are the includes and
 * macros we need to differentiate individual compilers from
 * the baseline.
 *
 * The easiest way to add a new compiler is to replicate the
 * _MSC_VER section using whatever manifest constant your
 * compiler uses to identify itself, then do the fixups in
 * that section.
 *
 * Be sure to put your section before that last define for COMPILER_NAME!
 *
 */

/*
 * Borland.
 *
 * No sharing stream open. (s)open/fdopen works, though.
 * Signal call is non-generic and incompatible
 * Read/Write to far memory in near model not implemented
 * Console output non-generic
 * No utime call
 *
 */

#ifdef __TURBOC__               /* Borland C++ 2.0                */
# ifdef __BORLANDC__
#   if _M_IX86 == 300
#     ifndef OS_2
#       define COMPILER_NAME   "Bcc32"
#       define _CPU            386
#     else
#       define COMPILER_NAME   "Bcc/OS2"
#     endif
#   endif
#   if _M_IX86 == 400
#     define COMPILER_NAME   "Bcc32"
#     define _CPU            486
#   endif
#   if _M_IX86 == 500
#     define COMPILER_NAME   "Bcc32"
#     define _CPU            586
#   endif
# else
#   define COMPILER_NAME   "Turbo"
# endif
# if __BORLANDC__ < 0x520       /* Don't need that in 5.20 anymore or old beheaviour, HJK 970805 */
#   define MUST_FDOPEN          /* Share streams: sopen+fdopen    */
#   define NO_SIGNALS           /* Don't install a signal handler */
# endif
# define NOSHORT_BITFIELDS
# ifdef OS_2
#   define _beginthread(a,b,c,d) _beginthread(a,c,d)  /* TE011097 */
#   define _ungetch(x) ungetch(x)  /* TE011097 */
#   include <utime.h>
typedef struct utimbuf UTIMBUF;

#   define UT_ACTIME actime
typedef long off_t;

#   include <sys\types.h>
#   include <sys\stat.h>
#   include <mem.h>
#   include <dir.h>
#   include <alloc.h>
#   define _fmalloc(n) malloc (n)
#   define _ffree(n) free (n)
#   ifndef __BORLANDC__
#     undef sopen               /* we don't want to use all the args */
#     define sopen(a,b,c) open (a, b|c)
#   endif
typedef void _cdecl SIGTYPE;

#   define SIGARGS void
typedef void _cdecl MAINTYPE;

#   define far
#   define __32BIT__
#   define _Far16 __far16
#   define _Seg16 __far16
#   define _Pascal __pascal
# else                          /* Not OS/2 */
// uncomment the following line for BC++ 2.0
// #define __TURBOC_OLD__
#   if __TURBOC__ < 0x310       /* r. hoerner: BC 3.1 has own routines */
#     define NEED_CPUTS         /* Need our own cputs             */
#     define NEED_PUTCH         /* Need our own putch             */
#     define NEED_CPRINTF       /* Need our own cprintf           */
#   endif
#   ifdef __TURBOC_OLD__
struct utimbuf
{
  time_t actime;                /* access time */
  time_t modtime;               /* modification time */
};
int cdecl utime (char *, struct utimbuf *);

#   endif                       /* __TURBOC_OLD__ */
#   ifndef __TURBOC_OLD__
#     include <utime.h>
#   endif
typedef struct utimbuf UTIMBUF;

#   define UT_ACTIME actime
typedef long off_t;

/* #if __BORLANDC__ >= 0x410 */
#   ifdef __BORLANDC__
#     include <sys\types.h>
#     include <sys\stat.h>
#   else
#     include <types.h>
#     include <stat.h>
#   endif
#   include <mem.h>
#   include <dir.h>
#   include <direct.h>
#   include <alloc.h>
#   ifdef __BORLANDC__
#   ifndef _WIN32               // alex, 97-02-11, windows.h does this already
#    include <malloc.h>         /* jl 960729:  added malloc.h to get
                                   _fmalloc() and _ffree */
#   else
#    define _fmalloc(n) malloc(n)
#    define _ffree(n) free(n)
#   endif
#   else
#     define _fmalloc(n) farmalloc (n)
#     define _ffree(n) farfree (n)
#     undef sopen               /* we don't want to use all the args */
#     define sopen(a,b,c) open (a, b|c)
#   endif
typedef void _cdecl SIGTYPE;

#   define SIGARGS void
typedef void _cdecl MAINTYPE;

# endif                         /* OS_2 */
#endif /* defined __TURBOC __ */



/*
 * Microsoft.
 *
 * By and large, it's Microsoft compatible <grin>
 *
 * MSC 5.10 and below has no way to open a stream in
 * sharing mode. sopen/fdopen doesn't work.
 *
 */

#ifdef _MSC_VER                 /* Microsoft C  */
# ifdef DOS16
#  if _MSC_VER == 510
#    define COMPILER_NAME   "uSoft5.1"
#  endif
#  if _MSC_VER == 600
#    define COMPILER_NAME   "uSoft6.0"
#  endif
#  if _MSC_VER == 700
#    define COMPILER_NAME   "uSoft7.0"
#    define ANSI_TIME_T
#  endif
#  if _MSC_VER == 800
#    define COMPILER_NAME   "uSoft8.0"
#    pragma warning ( disable : 4746 )  /* Unsized arrays treated as __far */
#  endif
#  if _MSC_VER < 600
#    define CANT_FSOPEN
#  endif
# elif _WIN32                   // NS980125
#  if _MSC_VER == 900
#   define COMPILER_NAME   "uSoft9.0"
#  endif
#  if _MSC_VER == 1100
#   define COMPILER_NAME   "uSoft11.0"
#  endif
# define MAXPATH 256
# define _fmalloc(n) malloc(n)
# define _ffree(p)   free(p)
# endif                         // defined _WIN32

#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>
typedef struct utimbuf UTIMBUF;

#define UT_ACTIME actime
#include <memory.h>
#include <direct.h>
#include <malloc.h>

#if _MSC_VER < 1100             /* NS130599 */
typedef void _cdecl SIGTYPE;
typedef void _cdecl MAINTYPE;

#else
typedef void SIGTYPE;
typedef void MAINTYPE;

#endif

#define SIGARGS int sigarg
#define SIGNAL_ARGUMENT

#endif /* defined _MSC_VER */



/*
 * IBM.
 *
 * By and large, it's Microsoft compatible <grin>
 * At the moment we have to use the migration libs.
 *
 */

#ifdef __IBMC__                 /* IBM C Set/2 or VAC++ */
# if __IBMC__  < 300            /*  TS 970124, to destinguish CSet from VAC++ */
#   define COMPILER_NAME   "CSet2"
# else
#   define COMPILER_NAME   "VAC++"
# endif
# include <sys\types.h>
# include <sys\stat.h>
# include <sys\utime.h>
typedef struct utimbuf UTIMBUF;

# define UT_ACTIME actime
# include <memory.h>
# include <direct.h>
# include <malloc.h>
# define MUST_FDOPEN            /* Share streams: sopen+fdopen    */
# define SHARE_SOPEN            /* Share streams: sopen, not open */
# define NEED_MKTEMP            /* Need our own mktemp            */
# define NOSHORT_BITFIELDS
typedef void SIGTYPE;

# define SIGARGS int sigarg
# define SIGNAL_ARGUMENT
# define INTMAIN                /* MMP 960419 return in main() instead of exit() */
typedef int MAINTYPE;

# define FLUSHSTDOUT            /* MMP 960419 Need to flush standard output? */
# pragma pack(1)
# define INLINEFUNC _Inline     /* VAC++ on OS/2 or NT, do function inlining for less CPU requirement (bigger EXE doesn't care) */
#endif /* defined __IBMC__ */



/*
 * WATCOM.
 *
 * v9.5 : No sharing stream open. sopen/fdopen doesn't work.
 * v10.0: sharing stream open. sopen/fdopen does work.
 * No mktemp call
 * utime structure member naming different
 *
 */

#ifdef __WATCOMC__              /* WATCOM C++ 9.5, thanks to SJD  */
# define COMPILER_NAME   "Watcom"
# ifdef __FLAT__
#   define __32BIT__
# endif
# if __WATCOMC__ < 1000         /* MMP 960419 */
#   define MUST_FDOPEN          /* Share streams: sopen+fdopen    */
# endif
# define SHARE_SOPEN            /* Share streams: sopen, not open */
# define NEED_MKTEMP            /* Need our own mktemp            */
# define FLUSHSTDOUT            /* MMP 960419 Need to flush standard output? */
# include <sys\types.h>
# include <sys\stat.h>
# include <sys\utime.h>
typedef struct utimbuf UTIMBUF;

# define UT_ACTIME actime
# include <direct.h>
# include <malloc.h>
# define SIGTYPE void
# define SIGARGS int sigarg
# define SIGNAL_ARGUMENT        /* MMP 960419 - avoids a warning */
typedef int MAINTYPE;           /* MR 961121 was void */

# define INTMAIN                /* MR 961121 return instead of exit in main() */
# pragma pack(1)
# ifdef __FLAT__
#   define _osmode OS2_MODE
#   define _fmalloc(n) malloc(n)
#   define _ffree(p)   free(p)
#   ifdef far
#     undef far
#   endif
#   define far
#   define _Seg16
# endif
# define _ungetch    ungetch
# define _stackavail stackavail
#endif /* defined __WATCOMC__ */



#ifdef __EMX__                  /* EMX 0.9c */
# define COMPILER_NAME   "EMX"
# include <sys\types.h>
# include <sys\stat.h>
# include <sys\utime.h>
typedef unsigned short APIRET16;  /*  TS 970126 */

# define APIENTRY16 _Far16 _Pascal
# define far
// # define _Far16
# define _Fastcall
# define _fastcall
# define _cdecl
# define cdecl                  /*  TS 961004 */
# define pascal
# define _near                  /*  TS 970404   */
typedef int MAINTYPE;
typedef void SIGTYPE;

# define SIGARGS int sigarg
# define SIGNAL_ARGUMENT
typedef struct utimbuf UTIMBUF;

# define UT_ACTIME actime
# define itoa _itoa             /*  TS 970308   */
# define ultoa _ultoa
# define _fmalloc(n) malloc (n)
# define _ffree(n) free (n)
# define NEED_CPUTS
# define NEED_PUTCH
# define NEED_CPRINTF
# define NEED_KBHIT
# define mkdir(a) mkdir(a,0)    /*  TS 960628   */
# define DosSetPrty(a,b,c,d) DosSetPriority(a,b,c,d)  /*  TS 960628   */
# define strcmpi(a,b) stricmp(a,b)  /*  TS 970119   */
# define int86(a,b,c) _int86(a,b,c)  /*  TS 970405   */
# define intdos(a,b) _int86(0x21,a,b)  /*  TS 970405   */
# define INLINEFUNC  __inline__ /*  TS 970207   */
# define DosDevIOCtl2 DosDevIOCtl  /*  TS 970403   */
#endif /* EMX */



/*
 * Zortech.
 *
 * No sharing stream open. sopen/fdopen doesn't work.
 * Signal handler stuff incompatible
 * Read/Write to far memory in near model not implemented
 * No console I/O functions
 * No mktemp call
 * utime doesn't use a structure
 * no ultoa, we use more limited ltoa
 * tzset not there or needed
 *
 */

#ifdef __ZTC__                  /* Zortech C++ 3.0 */
# define COMPILER_NAME   "Zortech"
# define CANT_FSOPEN            /* No way to share streams        */
# define SHARE_SOPEN            /* Share streams: sopen, not open */
# define NO_SIGNALS             /* Don't install a signal handler */
# define NEED_CPUTS             /* Need our own cputs             */
# define NEED_PUTCH             /* Need our own putch             */
# define NEED_MKTEMP            /* Need our own mktemp            */
# include <sys\types.h>
# include <sys\stat.h>
struct utimbuf
{
  time_t actime;                /* access time */
  time_t modtime;               /* modification time */
};
typedef time_t UTIMBUF;         /* ZTC utime uses a time_t array  */

# define UT_ACTIME actime
# ifdef __VCM__
#   define __LARGE__            /* VCM model same as large, really */
# endif
typedef long off_t;

# include <direct.h>
# define _fmalloc(n) farmalloc (n)
# define _ffree(n) farfree (n)
# define ultoa(x,y,z) ltoa((long)x,y,z)
# define stricmp(x,y) strcmpl(x,y)
# define _dos_setdrive(x,y) dos_setdrive(x,y)
# define tzset()
typedef void _cdecl SIGTYPE;

# define SIGARGS void
typedef void _cdecl MAINTYPE;

#endif /* defined __ZTC__ */



#ifdef __unix__
# include <dirent.h>
# define far
# define __32BIT__
# define _Far16 __far16
# define _Seg16 __far16
# define _Pascal
# define pascal
# define _cdecl
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef int bool;
typedef unsigned int BOOL;
typedef unsigned char BYTE;
typedef unsigned char *PSZ;
typedef unsigned char *PCH;
typedef unsigned short HVIO;
typedef struct utimbuf UTIMBUF;
typedef void *PVOID;
typedef int MAINTYPE;
typedef void SIGTYPE;
int linux_rename (char *, char *);

# define SIGARGS int sigarg
# define cdecl
# define _fmalloc(n) malloc(n)
# define _ffree(n) free(n)
# define real_flush(a) (a)
# define O_BINARY 0
# define P_WAIT 0
# define USELNGFILE
# define NOSHORT_BITFIELDS
# define HAVE_HYDRA
# define NEED_CPUTS
# define CANT_FSOPEN
# define SHARE_DEFAULT 0
# define COMFILE_PREFIX ""
# define COMFILE_SUFFIX ""
# define UT_ACTIME actime
# define strnicmp(a,b,c) strncasecmp(a,b,c)
# define stricmp(a,b) strcasecmp(a,b)
# define itoa(a,b,c) ultoa(a,b,c)
# define COMPILER_NAME   "GCC"
# include <sys/types.h>
# include <sys/stat.h>
# include <utime.h>
# include <sys/time.h>
# define mkdir(a) mkdir(a,0770)
# define rename(a,b) linux_rename(a,b)
# define tell(a) lseek(a,SEEK_SET,0)
#endif /* __unix__ */


/*
 * Make sure there's at least a zero-length string for
 * the compiler name.
 *
 * THIS LINE SHOULD ALWAYS BE LAST!
 *
 */

#ifndef COMPILER_NAME
# define COMPILER_NAME   ""
#endif

#define _Optlink _Pascal

#ifdef __EMX__                  /*  TS 970216 */
# undef _Optlink
#endif

#define DOS16

#ifdef __unix__
#undef DOS16
#endif


#ifdef OS_2
# define INCL_PM
# define INCL_WIN
# define INCL_BASE
# define INCL_DOS
# define INCL_DOSDEVICES
# define INCL_DOSDEVIOCTL
# define INCL_DOSERRORS
# define INCL_DOSFILEMGR        /* MR 961225 moved from maxmcp.c */
# define INCL_DOSMISC
# define INCL_DOSNMPIPES
# define INCL_DOSPROCESS
# define INCL_DOSSEMAPHORES
# include <os2.h>
  /* if one has not the complete up-to-date bsedev.h file: */
# ifndef ASYNC_EXTSETBAUDRATE
#   define ASYNC_EXTSETBAUDRATE        0x0043
# endif
# ifndef ASYNC_EXTGETBAUDRATE
#   define ASYNC_EXTGETBAUDRATE        0x0063
# endif
# ifndef TESTCFG_SYS_ISSUEOUTIOINSTR
#   define TESTCFG_SYS_ISSUEOUTIOINSTR 0x0042
# endif
typedef APIRET16 (APIENTRY16 _PFN16) ();
typedef _PFN16 *_Seg16 PFN16;   /* need this to access 16 bit DLLs */

# ifdef __EMX__                 /*  TS 970125   */
#  pragma pack (1)
# endif
# ifndef NEED_OS2COMMS
#   include "maxcomm.h"         /* moved here to avoid hcModem undefined        WRA */
# endif
# define COMFILE_PREFIX ""
# define COMFILE_SUFFIX ".CMD"
# define SHARE_DEFAULT 1
# define BBS_SPAWN
# define HAVE_HYDRA
# define BINKLEY_SOUNDS         /* OS/2 can make noise, too! */
# define LOCALFUNC
# define HSNOOP  HPIPE
# define PHSNOOP PHPIPE
# undef DOS16
#endif /* defined OS_2 */


#ifdef _WIN32                   /* changes to this section: alex, 97-02-11 */
typedef unsigned short UWORD;

# undef ERROR                   /* in windows.h */
# undef IGNORE                  /* same */

/* #define far
 * #define pascal
 * #define WINAPI      __stdcall */

/* Use Unix time direct, amazingly, this just works!  - HK 971127 */
/* # define ANSI_TIME_T */
/* # define ANSI_TIME_T_DELTA (_timezone) */

# define HAVE_HYDRA
# define BINKLEY_SOUNDS         /* NT can make noise! */
# define real_flush(a) (a)
typedef unsigned short HVIO;

# define COMFILE_PREFIX ""
# define COMFILE_SUFFIX ".BAT"
# define SHARE_DEFAULT 1
# define BBS_SPAWN
# define NEED_SET_PRIOR         /* alex, 97-02-09, implemented set_prior() (dosfuncs.c) for Win32 */
# pragma warning (disable: 4201 4244)
# undef DOS16
extern int __stat (char *p, struct stat *stbuf);

# define HSNOOP HANDLE          /* alex(as), 97-02-24 */
#endif /* defined _WIN32 */


#ifdef DOS16
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef long DWORD;
typedef unsigned char BYTE;
typedef unsigned short BOOL;
typedef unsigned short bool;    /*  TS 970310   */
typedef int HFILE;
typedef unsigned char far *PCH;
typedef unsigned short HVIO;

# define HAVE_HYDRA
# ifndef __WATCOMC__
#   define LOCALFUNC _near _fastcall
# else
#   define LOCALFUNC _near
# endif
# define hfComHandle    (port_ptr+1)
# define COMFILE_PREFIX ""
# define COMFILE_SUFFIX ".BAT"
typedef unsigned char far *PSZ;

# define SHARE_DEFAULT 0
# define VOID void              /*  TS 960801   */
typedef VOID *PVOID;            /*  TS 960801   */

#endif /* defined DOS16 */


#define isBITBRAIN 0x1b

#define TIMEZONE get_bink_timezone()

#ifndef LOCALFUNC
# define LOCALFUNC
#endif

#ifndef INLINEFUNC
# define INLINEFUNC static      /* default: don't use inline funcs */
#endif

#ifndef NULLHANDLE              /* if NULLHANDLE is undefined */
# ifdef __32BIT__               /* r.hoerner jan 16 97 */
#   define NULLHANDLE 0L
# else
#   define NULLHANDLE 0U
# endif
#endif

// This is the same as FA_SUBDIR
// CEH 19981108
//#ifndef FILE_DIRECTORY
//#define FILE_DIRECTORY 0x10
//#endif

/* TJW960915
 * This is to handle different time_t implementations in the ansi_* conversion
 * functions (see misc.c). Binkley internally works with UNIX style ONLY.
 * UNIX style is 1.1.1970 0:00 and ANSI_TIME_T_DELTA is 0L
 * ANSI style seems to be 1.1.1900 0:00 and ANSI_TIME_T_DELTA is 70 years.
 */

#ifdef ANSI_TIME_T
#  ifndef ANSI_TIME_T_DELTA
#    define ANSI_TIME_T_DELTA 2209075200L  /* 70 years correction for ANSI */
#  endif
#else
#  define ANSI_TIME_T_DELTA 0L  /* no correction for UNIX time */
#endif

#ifdef ANSI_TIME_T
#  ifdef _WIN32
#    ifdef _MSC_VER             // NS980125
#      define unix_stat(f,b)    ansi_stat(f,b)
#    endif
#    define unix_stat(f,b)  __stat(f,b)
#  else                         // _WIN32
#    define unix_stat(f,b)    ansi_stat(f,b)
#  endif
#  define unix_fstat(h,b)   ansi_fstat(h,b)
#  define unix_time(t)      ansi_time(t)
#  define unix_mktime(t)    ansi_mktime(t)
#  define unix_localtime(t) ansi_localtime(t)
#  define unix_gmtime(t)    ansi_gmtime(t)
#  define unix_ctime(t)     ansi_ctime(t)
#  define unix_utime(f,b)   ansi_utime(f,b)
#else
#  define unix_stat(f,b)    stat(f,b)
#  define unix_fstat(h,b)   fstat(h,b)
#  define unix_time(t)      time(t)
#  define unix_mktime(t)    mktime(t)
#  define unix_localtime(t) localtime(t)
#  define unix_gmtime(t)    gmtime(t)
#  define unix_ctime(t)     ctime(t)
#  define unix_utime(f,b)   utime(f,b)
#endif

// Why is this placed here? HJK - 97/12/31
// TJW980104: because BT-XE needed that ;-) But as it looks like nobody
// knows what this is for, I have removed all relating code. See comment
// in b_initva.c ...
// # define UCT_DIFFERENTIAL _timezone

/* Local include files */

/* MMP 960419 - Respawn keyword */
#define exit(n) binkexit(n)

#include "bink.h"
#include "defines.h"
#include "file_io.h"
#include "com.h"
#include "xfer.h"               /* include it BEFORE any other (as hydra.h etc) */
#include "msgs.h"
#include "zmodem.h"
#include "janus.h"
#include "keybd.h"
#include "keyfncs.h"
#include "sbuf.h"
#include "sched.h"
#include "timer.h"
#include "vfossil.h"
#include "box.h"
#include "buffer.h"             /* TJW 960416 */
#include "cache.h"              /* MMP 960507 */
#include "banner.h"
#include "pktmsgs.h"            /* CFS 971202 */
#include "history.h"            /* CFS 970315 */
#include "prototyp.h"

#ifdef HAVE_HYDRA
# include "hydra.h"
#endif

#include "css.h"
#include "version7.h"           /* TJW 960731 */
#include "debug.h"              /* TJW 960828 */
#include "callwin.h"            /* TS 971006  */

#ifdef OS_2
# include "binkpipe.h"          /* r. hoerner */
# include "maxmcp.h"
#else
# define BINK_INTRO 0           /* dummies for DOS */
# define BINK_START 0
# define BINK_END   0
# define IPC_Open()
# define IPC_Ping()
# define IPC_Close()
#endif


/* last include, needs all *.h definitions to be present */
#include "externs.h"

/* $Id: includes.h,v 1.16 1999/09/30 17:50:06 ceh Exp $ */
