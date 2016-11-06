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
 * Filename    : $Source: E:/cvs/btxe/src/version.c,v $
 * Revision    : $Revision: 1.104 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/10/07 20:09:46 $
 * State       : $State: Exp $
 *
 * Description : This file just holds the date of this version of Bink
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

# define VERSION_TYPE        "Beta"

/* DO NOT MODIFY */
#define VERSION_BASE         "XH6"
#define VERSION_MODIFIED     ""
/* DO NOT MODIFY */

// let's see what options we've got...

#ifndef CO_NOPT
# define COMPILER_OPT        "Enabled"
#else
# define COMPILER_OPT        "Disabled"
#endif

#if defined DEBUG || defined BTPE_DEBUG
# define MODULE_DEBUG        "Enabled"
#else
# define MODULE_DEBUG        "Disabled"
#endif

#ifdef CACHE
# define MODULE_OUTBCACHE    "Enabled"
#else
# define MODULE_OUTBCACHE    "Disabled"
#endif

#ifdef NEED_SET_PRIOR
# define MODULE_PRIOHAND     "Enabled"
#else
# define MODULE_PRIOHAND     "Disabled"
#endif

#ifdef OS_2
# ifdef NEED_OS2COMMS
#  define MODULE_COMM        "Internal Async Module"
#  define VERSION_OPT        "(async)"
# else
#  define MODULE_COMM        "maxcomm.dll"
#  define VERSION_OPT        "(mxcom)"
# endif
#elif _WIN32
# ifndef USE_WINFOSSIL
#  define MODULE_COMM        "ntcomm.dll"
#  define VERSION_OPT        "(ntcomm)"
# else
#  define MODULE_COMM        "WinFossil"
#  define VERSION_OPT        "(winfos)"
# endif
#elif __unix__
# define MODULE_COMM         "Internal Async Module"
# define VERSION_OPT         ""
#else
# define MODULE_COMM         "DOS Fossil"
# define VERSION_OPT         ""
#endif

#ifdef EXCEPTIONS
# define MODULE_EXCEPTHAND   "Enabled"
#else
# define MODULE_EXCEPTHAND   "Disabled"
#endif

#ifdef HEAPCHECK
# define MODULE_HEAPCHK      "Enabled"
#else
# define MODULE_HEAPCHK      "Disabled"
#endif

#ifdef BBS_INTERNAL
# define MODULE_INTBBS       "Enabled"
#else
# define MODULE_INTBBS       "Disabled"
#endif

char *version_type = VERSION_TYPE;
char *version_base = VERSION_BASE;
char *version_modified = VERSION_MODIFIED;
char *version_opts = VERSION_OPT;

#ifdef OVERLAYS                 /* MR 970217 */
# define OVLY_STRING "-Overlay"
#else
# define OVLY_STRING ""
#endif

#ifdef _CPU                     /*  TS 961013   */
# if _CPU==86
#   define CPU_STRING "i86"
# elif _CPU==186
#   define CPU_STRING "i186"
# elif _CPU==286
#   define CPU_STRING "i286"
# elif _CPU==386
#   define CPU_STRING "i386"
# elif _CPU==486
#   define CPU_STRING "i486"
# elif _CPU==586
#   define CPU_STRING "i586"
# elif _CPU==686                /* AG 990120 added for PPro/PII */
#   define CPU_STRING "i686"
# endif
#else /*  #ifdef _CPU     */
# ifdef __i486__
#   define CPU_STRING "i486"
# elif __i386__
#   define CPU_STRING "i386"
# else
#   define CPU_STRING "unknown"
# endif
#endif /*  #ifdef _CPU     */

char compilerinfo[] = COMPILER_NAME "/" CPU_STRING OVLY_STRING;
char compiledatetime[] = "on " __DATE__ " at " __TIME__;


void
print_exeinfo (void)
{
  printf ("Version: %s-%s%s%s %s\r\n", version_type, version_base,
          version_modified, version_opts, compilerinfo);
  printf ("Compiled %s with\r\n", compiledatetime);
  printf (" - Compiler              : " COMPILER_NAME "\r\n");
  printf (" - Compiler Optimizations: " COMPILER_OPT "\r\n");
  printf (" - Debug Code            : " MODULE_DEBUG "\r\n");
  printf (" - Exception Handling    : " MODULE_EXCEPTHAND "\r\n");
  printf (" - Heap Checking         : " MODULE_HEAPCHK "\r\n");
  printf (" - Communication Module  : " MODULE_COMM "\r\n");
  printf (" - Outbound Cache Module : " MODULE_OUTBCACHE "\r\n");
  printf (" - Priority Handling     : " MODULE_PRIOHAND "\r\n");
  printf (" - Internal BBS          : " MODULE_INTBBS "\r\n\r\n");
}

/* $Id: version.c,v 1.104 1999/10/07 20:09:46 daemon Exp $ */
