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
 * Filename    : $Source: E:/cvs/btxe/src/os2_pm.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:36 $
 * State       : $State: Exp $
 * Orig. Author: robert hoerner, 2:2476/7
 *
 * Description :
 *
 *   This file contains all OS/2 functions, where binkley needs external DLLs.
 *   These DLLs are (at this moment) MCP32 and the OS/2 PM DLLs, the
 *   last ones are accessed via another DLL (BINK_PM.DLL), 'cause loading
 *   and executing of these DLLS "by hand" seem to not work at all.
 *
 *   There is a comfortable structure provided that allows you to easily add
 *   new DLLs and their functions. You can make the loading of your DLL
 *   dependend of the prior loading of another DLL (loaded by another program
 *   then binkley).
 *
 *   You can define whether the DLL should be searched in the LIBPATH or in
 *   binkleys start directory.
 *
 *     BINK_PM.DLL  is searched in binkley startdirectory,
 *     MCP32.DLL    is searched in LIBPATH.
 *
 *   Take care about case sensitiveness of DLL functions. You should verify
 *   these names in case your code doesn't work ("error loading blabla from
 *   blabla.DLL", you'll get these messages only when binkley is started with
 *   the command line parameter "debug"!).
 *
 *   After you have defined the structures fields you can directly code your
 *   functions. These functions must provide an entry for the "rest of binkley"
 *   to call the DLL functions. One way could be (see McpSendMsg()) to code
 *   the function "as if" it would be the DLL function itself. This will help
 *   you to port the other code, 'cause for the other (calling) code it will
 *   make no difference whether "McpSendMsg" is resolved by the linker
 *   looking into a library or by the linker finding this function already
 *   provided. The only difference is that you must give the library name to
 *   the linker in the first case, whilst you may not give it in the second.
 *
 *
 *   Very important:
 *   ==============
 *
 *   Please make sure that you define your function prototypes correctly
 *   and accurate.
 *
 *   There is NO chance for the compiler to verify any argument types!
 *   The one and only thing that can prevent crashes and burns are
 *   correct and accurate function prototypes and type casts!
 *
 *   I failed to make that code below work for loading 16 bit functions.
 *   So SNSERVER is not included, but the internal binkley pipe is used.
 *
 *---------------------------------------------------------------------------*/

#ifdef OS_2

#include "includes.h"           /* this includes os2.h */

int APIENTRY McpGetMsg (HPIPE, PVOID, USHORT *, USHORT);
int APIENTRY McpPeekMsg (HPIPE, PVOID, USHORT *, USHORT);
int APIENTRY McpSendMsg (HPIPE, USHORT, BYTE *, USHORT);
int APIENTRY McpDispatchMsg (PVOID, USHORT, VOID *, PVOID);
int APIENTRY McpOpenPipe (char *, HPIPE *);
void APIENTRY McpClosePipe (HPIPE);

// ===========================================================================
#define DLL_VERSION  100000L    /* to be returned version of bink_pm.dll */
// ===========================================================================

typedef struct
{
  PSZ needname;                 /* load DLL only of "needname.DLL" is already loaded */
  PSZ dllname;                  /* name of DLL to load                               */
  PSZ fn_name;                  /* name of function to load                          */
  BOOL use_libpath;             /* if TRUE: load DLL from LIBPATH                    */
  ULONG use_version;            /* load DLL only of GetVersion() returns this number */
  HMODULE *hmod;                /* handle of loaded DLL                              */
  PFN *pfn;                     /* 32 bit pointer to loaded function                 */
}
DLL_LOAD, *PDLL_LOAD;

// ===========================================================================

HMODULE hBINK_PM = NULLHANDLE;  /* handle for BINK_PM.DLL         */
PFN pfnSetWindowTitle = NULL;   /* ptr to function SetWindowTitle */
PFN pfnIsPipeRunning = NULL;    /* ptr to function IsPipeRunning  */

// ===========================================================================

HMODULE hMAX_DLL = NULLHANDLE;  /* handle for MCP32.DLL           */
PFN pfnMcpSendMsg = NULL;       /* ptr to function McpSendMsg     */
PFN pfnMcpGetMsg = NULL;
PFN pfnMcpPeekMsg = NULL;
PFN pfnMcpDispatchMsg = NULL;
PFN pfnMcpOpenPipe = NULL;
PFN pfnMcpClosePipe = NULL;

// ===========================================================================

static DLL_LOAD dll_struc[] =   /* we like it easy, do we? */
{
  {"PMMERGE", "BINK_PM", "SetWindowTitle", FALSE, DLL_VERSION, &hBINK_PM, &pfnSetWindowTitle},
  {"PMMERGE", "BINK_PM", "IsPipeRunning", FALSE, DLL_VERSION, &hBINK_PM, &pfnIsPipeRunning},
  {NULL, "MCP32", "McpSendMsg", TRUE, 0L, &hMAX_DLL, &pfnMcpSendMsg},
  {NULL, "MCP32", "McpGetMsg", TRUE, 0L, &hMAX_DLL, &pfnMcpGetMsg},
  {NULL, "MCP32", "McpPeekMsg", TRUE, 0L, &hMAX_DLL, &pfnMcpPeekMsg},
  {NULL, "MCP32", "McpDispatchMsg", TRUE, 0L, &hMAX_DLL, &pfnMcpDispatchMsg},
  {NULL, "MCP32", "McpOpenPipe", TRUE, 0L, &hMAX_DLL, &pfnMcpOpenPipe},
  {NULL, "MCP32", "McpClosePipe", TRUE, 0L, &hMAX_DLL, &pfnMcpClosePipe},
  {NULL, NULL, NULL, FALSE, 0L, NULLHANDLE, NULL}
};

// ========================================================================

static void _cdecl
print (FILE * fp, char *fmt,...)  /* print messages to console */
{
  if (debugging_log)
  {
    va_list args;

    va_start (args, fmt);
    vsprintf (e_input, fmt, args);
    va_end (args);
    fprintf (fp, e_input);
  }
}

// ========================================================================

VOID
LoadOS2DLLs (VOID)              /* loads and checks OS/2 DLLs          */
{
  APIRET rc;
  DLL_LOAD *pDLL;

  /* CHAR    myDir[CCHMAXPATH]; *//* MR 970213 no longer used */
  CHAR DLLName[CCHMAXPATH];

  pDLL = (DLL_LOAD *) & dll_struc[0];  /* point to structure with definitions */

  print (stderr, "\n");         /* one linefeed is ok */

  for (; pDLL->dllname != NULL; pDLL++)  /* scan structure */
  {
    if (pDLL->needname != NULL) /* if another DLL is required */
    {
      HMODULE hmod;

      rc = DosQueryModuleHandle (pDLL->needname, &hmod);
      if (rc != 0)
      {
        print (stdout, "not loading '%s.DLL', 'cause '%s.DLL' not loaded\n",
               pDLL->dllname,
               pDLL->needname);
        continue;               /* required DLL is not loaded at this time */
      }
    }

    strcpy (DLLName, (char *) pDLL->dllname);

    if (!pDLL->use_libpath)
    {
      /* ULONG   DirLen, disk, diskmap; */
      /*
       * note: please do NOT use argv[0] to check the binkley start dir.
       *       With VAC 3.00 it definitely does not (maybe not always)
       *       work in non-PM mode, */

      /* MR 970213 why not taking BINKpath instead of current directory?
       *           BINKpath holds the value of %BINKLEY% if avaible or the
       *           current directory. */

      /*  memset(myDir,0,sizeof(myDir));
       *  DirLen = sizeof(myDir);
       *
       *  rc = DosQueryCurrentDir(0L,myDir,&DirLen);
       *  if (rc == 0)
       *  {
       *    DosQueryCurrentDisk(&disk, &diskmap);
       *    sprintf(DLLName,"%c:\\%s\\%s.DLL",(CHAR)(disk+'@'),
       *    myDir,pDLL->dllname);
       *  }
       *  else
       *    print(stderr,"Can't find binkley start directory\n");
       */

      /* it's easy, isn't it ;) */
      sprintf (DLLName, "%s%s.DLL", BINKpath, pDLL->dllname);
    }

    rc = 0;

    if (*pDLL->hmod == NULLHANDLE)  /* if DLL not already loaded */
      rc = DosLoadModule (NULL, 0L, (PCSZ) DLLName, pDLL->hmod);  /* load DLL now */

    if (rc == 0)
    {
      if (pDLL->use_version)    /* if we should call "GetVersion()" */
      {
        PFN dll_version;

        rc = DosQueryProcAddr (*pDLL->hmod, 0L, (PCSZ) "GetVersion", &dll_version);
        if (rc == 0)
        {
          if (dll_version () != pDLL->use_version)
          {
            print (stderr, "Wrong version: %s!\n", DLLName);
            DosBeep (1000, 100);
            DosFreeModule (*pDLL->hmod);  /* release DLL */
            *pDLL->hmod = NULLHANDLE;
            continue;
          }
        }
        else
        {
          print (stderr, "Wrong DLL: %s!\n", DLLName);
          DosSleep (1000);
          DosFreeModule (*pDLL->hmod);
          *pDLL->hmod = NULLHANDLE;
          continue;
        }
      }

      rc = DosQueryProcAddr (*pDLL->hmod, 0L, pDLL->fn_name, pDLL->pfn);

      if (rc == 0)
        print (stdout, "loaded '%s' from %s\n",
               pDLL->fn_name,
               DLLName);
      else
        print (stderr, "error loading '%s' from %s.DLL\n",
               pDLL->fn_name,
               DLLName);
    }
    else
      print (stderr, "DosLoadModule error SYS%04x (%s)\n",
             (USHORT) rc,
             DLLName);
  }
}

// ========================================================================

VOID
UnLoadOS2DLLs (VOID)            /* called from library exit routine */
{
  DLL_LOAD *pDLL;

  print (stdout, "\n");         /* one linefeed is ok */
  pDLL = (DLL_LOAD *) & dll_struc[0];  /* point to structure with definitions */

  for (; pDLL->dllname != NULL; pDLL++)
  {
    if (*pDLL->hmod != NULLHANDLE)
    {
      DosFreeModule (*pDLL->hmod);
      print (stdout, "unloaded %s.DLL\n", pDLL->dllname);
    }
    *pDLL->hmod = NULLHANDLE;
    pDLL->pfn = NULL;
  }
}

/* --------------------------------------------------------------------- */
/* AW 981117 For Watcom: convert adress of 16-bit function Flat->16:16
 * The only way to explicitly do this conversion is via an ugly casting
 * over a void pointer, but better this way than no way. VAC++ seems to
 * convert implicitly                                                   */

#ifdef __WATCOMC__
#ifdef __FLAT__

APIRET
BtQueryProcAddr (HMODULE hm, ULONG ulOrd, PCSZ procname, PFN * pProcAdr)
{
  void _Far16 **pProcAdrHelper = pProcAdr;
  ULONG ProcType;
  APIRET rc;

  rc = DosQueryProcAddr (hm, ulOrd, procname, pProcAdr);
  if (!rc)
  {
    rc = DosQueryProcType (hm, ulOrd, procname, &ProcType);
    if (ProcType == PT_16BIT)
      *pProcAdrHelper = *pProcAdr;
  }

  return rc;
}

#endif
#endif

/* --------------------------------------------------------------------- */

BOOL
IsPipeRunning (HWND * hwnd)     /* called from binkdlg.c          */
{
  if (pfnIsPipeRunning == NULL) /* check if PFN is valid  */
    return FALSE;
  /* if valid: call DLL fnc */
  return ((BOOL) pfnIsPipeRunning ((HWND *) hwnd));
}

/* --------------------------------------------------------------------- */

VOID
SetWindowTitle (char *status)   /* called from maxmcp.c           */
{
  CHAR windowtitle[256];
  APIRET rc;

  if (pfnSetWindowTitle == NULL)
    return;

  sprintf (windowtitle, title_format, TaskNumber, status);
  rc = (APIRET) pfnSetWindowTitle ((PSZ) windowtitle);
  rc = rc;                      /* just to be able to check the return codes */
}

// ========================================================================

int APIENTRY
McpSendMsg (HPIPE hp, USHORT usType, BYTE * pbMsg, USHORT cbMsg)
{
  int rc;

  if (pfnMcpSendMsg == NULL)    /* check if PFN is valid  */
    return 0;

  /* if valid: call DLL fnc */
  rc = (int) pfnMcpSendMsg ((HPIPE) hp, (USHORT) usType, (BYTE *) pbMsg,
                            (USHORT) cbMsg);
  return (rc);
}

#endif /* OS_2         */

/* $Id: os2_pm.c,v 1.7 1999/03/22 03:47:36 mr Exp $ */
