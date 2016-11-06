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
 * Filename    : $Source: E:/cvs/btxe/src/b_spawn.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:16 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm "Spawn" module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#ifdef DOS16

/* Include MOVEAPI file if this is C7 or C8 under DOS. */

# ifndef __WATCOMC__            /* MR 970217 */
#  ifdef OVERLAYS
#   include <../source/move/moveapi.h>
#  endif
# endif

/* We now use Thomas Wagner's EXEC module to do our swapping.
   There's probably no good reason why we can't just hack our
   code to work with his as distributed. We'll put that on the
   wishlist. For now, we hacked his and you'll find the
   appropriate excerpts below.

   EXEC.H: EXEC function with memory swap - Main function header file.

   Public domain software by

   Thomas Wagner
   Ferrari electronic GmbH
   Beusselstrasse 27
   D-1000 Berlin 21
   Germany
 */

extern int do_exec (char *xfn, char *pars, int spawn, unsigned needed,
                    char **envp);

/*
   The EXEC function.

   Parameters:

   xfn      is a string containing the name of the file
   to be executed.

   pars     The program parameters.

   spawn    If 0, the function will terminate after the
   EXECed program returns, the function will not return.

   NOTE: If the program file is not found, the function
   will always return with the appropriate error
   code, even if 'spawn' is 0.

   If non-0, the function will return after executing the
   program. If necessary (see the "needed" parameter),
   memory will be swapped out before executing the program.
   For swapping, spawn must contain a combination of the
   following flags:

   USE_EMS  (0x01)  - allow EMS swap
   USE_XMS  (0x02)  - allow XMS swap
   USE_FILE (0x04)  - allow File swap

   The order of trying the different swap methods can be
   controlled with one of the flags

   EMS_FIRST (0x00) - EMS, XMS, File (default)
   XMS_FIRST (0x10) - XMS, EMS, File

   If swapping is to File, the attribute of the swap file
   can be set to "hidden", so users are not irritated by
   strange files appearing out of nowhere with the flag

   HIDE_FILE (0x40)    - create swap file as hidden

   and the behaviour on Network drives can be changed with

   NO_PREALLOC (0x100) - don't preallocate
   CHECK_NET (0x200)   - don't preallocate if file on net.

   This checking for Network is mainly to compensate for
   a strange slowdown on Novell networks when preallocating
   a file. You can either set NO_PREALLOC to avoid allocation
   in any case, or let the prep_swap routine decide whether
   to do preallocation or not depending on the file being
   on a network drive (this will only work with DOS 3.1 or
   later).

   needed   The memory needed for the program in paragraphs (16 Bytes).
   If not enough memory is free, the program will
   be swapped out.
   Use 0 to never swap, 0xffff to always swap.
   If 'spawn' is 0, this parameter is irrelevant.

   envp     The environment to be passed to the spawned
   program. If this parameter is NULL, a copy
   of the parent's environment is used (i.e.
   'putenv' calls have no effect). If non-NULL,
   envp must point to an array of pointers to
   strings, terminated by a NULL pointer (the
   standard variable 'environ' may be used).

   Return value:

   0x0000..00FF: The EXECed Program's return code
   0x0101:       Error preparing for swap: no space for swapping
   0x0102:       Error preparing for swap: program too low in memory
   0x0200:       Program file not found
   0x03xx:       DOS-error-code xx calling EXEC
   0x0400:       Error allocating environment buffer
   0x0500:       Swapping requested, but prep_swap has not
   been called or returned an error.
   0x0501:       MCBs don't match expected setup
   0x0502:       Error while swapping out
 */

/* Return codes (only upper byte significant) */

# define RC_PREPERR   0x0100
# define RC_NOFILE    0x0200
# define RC_EXECERR   0x0300
# define RC_ENVERR    0x0400
# define RC_SWAPERR   0x0500

/* Swap method and option flags */

# define USE_EMS      0x01
# define USE_XMS      0x02
# define USE_FILE     0x04
# define EMS_FIRST    0x00
# define XMS_FIRST    0x10
# define HIDE_FILE    0x40
# define NO_PREALLOC  0x100
# define CHECK_NET    0x200

# define USE_ALL      (USE_EMS | USE_XMS | USE_FILE)

#endif

void
b_spawn (char *cmd_str)
{
  char this_dir[PATHLEN];

#ifdef DOS16
  char *command;
  char *p;

# ifndef __TURBOC__
  unsigned j;

# endif                         /* ifndef __TURBOC__ */
# ifdef __ZTC__
  char **environ;

# endif
#else /* ifdef DOS16 */
  char *comspec;

#endif /* ifdef DOS16 */

  // TJW980104: see comment in b_initva.c
  // #ifndef UCT_DIFFERENTIAL
  // if (saved_TZ != NULL)
  // {
  //    putenv (saved_TZ);
  //    tzset ();
  // }
  // #endif

  if (cmd_str)
    IPC_SetStatus (MSG_TXT (M_MCP_SPAWNED));
  else
    IPC_SetStatus (MSG_TXT (M_MCP_SHELLED));

  IPC_Close ();
  closelogs ();                 /* TJW 960527 */
  set_prior (PRIO_NORMAL);

  need_update = 0;

  /* Save where we are */
  getcwd (this_dir, 79);

#ifndef DOS16

  if (cmd_str)
    system (cmd_str);
  else
  {
    comspec = getenv ("COMSPEC");
    if (comspec)
      spawnlp (P_WAIT, comspec, comspec, NULL);
  }

# ifdef OS_2
  DosSetDefaultDisk ((USHORT) (*this_dir - 'A' + 1));
# endif
# ifdef _WIN32
  _chdrive ((int) (*this_dir - 'A' + 1));
# endif

  chdir (this_dir);

#else /* ifndef DOS16 */

  if ((p = getenv ("COMSPEC")) == NULL)
    p = "COMMAND.COM";

# ifdef __ZTC__
  environ = NULL;
# endif

  if (swapdir == NULL)
  {
    if (cmd_str != NULL)
      system (cmd_str);
    else
      spawnlp (P_WAIT, p, p, NULL);
  }
  else
  {
# ifndef __WATCOMC__            /* MR 970217 */
#  ifdef OVERLAYS
    _movepause ();
#  endif
# endif
    if (cmd_str != NULL)
    {
      command = calloc (1, strlen (cmd_str) + 4);
      if (command != NULL)
      {
        strcpy (command, "/c ");
        strcat (command, cmd_str);
        do_exec (p, command, USE_ALL, 0xffff, environ);
        free (command);
      }
    }
    else
      do_exec (p, "", USE_ALL, 0xffff, environ);

# ifndef __WATCOMC__            /* MR 970217 */
#  ifdef OVERLAYS
    _moveresume ();
#  endif
# endif
  }

  /* Go back to the proper directory */
# ifdef __TURBOC__
  setdisk ((unsigned) (this_dir[0] - 'A'));
# else                          /* __TURBOC__ */
  _dos_setdrive ((unsigned) ((int) this_dir[0] - 'A' + 1), &j);
# endif                         /* __TURBOC__ */

  chdir (this_dir);

#endif /* DOS16 */

  // TJW980104: see comment in b_initva.c
  // #ifndef UCT_DIFFERENTIAL
  // if (saved_TZ != NULL)
  // {
  //     putenv ("TZ=GMT0");
  //     tzset ();
  // }
  // #endif

  set_prior (PRIO_REGULAR);
  IPC_Open ();
  IPC_SetStatus (MSG_TXT (M_MCP_RETURNING));
  IPC_Ping ();
  openlogs (0);                 /* TJW 960527 reopen the log-files, don't print \n */
  read_in_log ();
#ifdef _WIN32                   //HJK 971002 switch mouse block cursor off again
  SetConsoleMode (GetStdHandle (STD_INPUT_HANDLE), ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
  FlushConsoleInputBuffer (GetStdHandle (STD_INPUT_HANDLE));
#endif
}

#ifdef DOS16

/*
   EXEC.C: EXEC function with memory swap - Prepare parameters.

   Public domain software by

   Thomas Wagner
   Ferrari electronic GmbH
   Beusselstrasse 27
   D-1000 Berlin 21
   Germany

   BIXname: twagner
 */

#define SWAP_FILENAME "$$AAAAAA.AAA"

/* internal flags for prep_swap */

#define CREAT_TEMP      0x0080
#define DONT_SWAP_ENV   0x4000

#ifndef __TURBOC__
#define stpcpy(d,s)     (strcpy (d, s), d + strlen (s))
#endif

#ifdef __cplusplus
extern "C" int
#else
extern int _cdecl
#endif
  do_spawn (int swapping,       /* swap if non-0 */
            char *xeqfn,        /* file to execute */
            char *cmdtail,      /* command tail string */
            unsigned envlen,    /* environment length */
            char *envp);        /* environment pointer */

#ifdef __cplusplus
extern "C" int
#else
extern int _cdecl
#endif
  prep_swap (int method,        /* swap method */
             char *swapfn);     /* swap file name and/or path */

int
do_exec (char *exfn, char *epars, int spwn, unsigned needed, char **envp)
{
  char swapfn[82];              /* AW 980303 made static -> normal */
  char execfn[82];              /* AW 980303 made static -> normal */
  unsigned avail;
  union REGS regs;
  unsigned envlen;
  int rc;
  int idx;
  char **env;
  char *ep, *envptr, *envbuf;
  int swapping;

  strcpy (execfn, exfn);

  /* Now create a copy of the environment if the user wants it. */

  envlen = 0;
  envptr = envbuf = NULL;

  if (envp != NULL)
    for (env = envp; *env != NULL; env++)
      envlen += strlen (*env) + 1;

  if (envlen)
  {
    /* round up to paragraph, and alloc another paragraph leeway */
    envlen = (envlen + 32) & 0xfff0;
    envbuf = (char *) malloc (envlen);
    if (envbuf == NULL)
      return RC_ENVERR;

    /* align to paragraph */
    envptr = envbuf;
    if (FP_OFF (envptr) & 0x0f)
      envptr += 16 - (FP_OFF (envptr) & 0x0f);
    ep = envptr;

    for (env = envp; *env != NULL; env++)
    {
      ep = stpcpy (ep, *env) + 1;
    }
    *ep = 0;
  }

  if (!spwn)
    swapping = -1;
  else
  {
    /* Determine amount of free memory */

    regs.x.ax = 0x4800;
    regs.x.bx = 0xffff;
    intdos (&regs, &regs);
    avail = regs.x.bx;

    /* No swapping if available memory > needed */

    if (needed < avail)
      swapping = 0;
    else
    {
      /* Swapping necessary, use 'TMP' or 'TEMP' environment variable
         to determine swap file path if defined. */

      swapping = spwn;
      if (spwn & USE_FILE)
      {
        strcpy (swapfn, swapdir);

        if (_osmajor >= 3)
          swapping |= CREAT_TEMP;
        else
        {
          strcat (swapfn, SWAP_FILENAME);
          idx = strlen (swapfn) - 1;
          while (dexists (swapfn))
          {
            if (swapfn[idx] == 'Z')
              idx--;
            if (swapfn[idx] == '.')
              idx--;
            swapfn[idx]++;
          }
        }
      }
    }
  }

  /* All set up, ready to go. */

  if (swapping > 0)
  {
    if (!envlen)
      swapping |= DONT_SWAP_ENV;

    rc = prep_swap (swapping, swapfn);
    if (rc < 0)
      return RC_PREPERR | -rc;
  }

  rc = do_spawn (swapping, execfn, epars, envlen, envptr);

  /* Free the environment buffer if it was allocated. */

  if (envlen)
    free (envbuf);

  return rc;
}

#endif /* ifdef DOS16 */
#ifdef OS_2

/* ====================================================================== */

int
ExecSession (unsigned char *name)  /* r. hoerner */
{
  static STARTDATA startd;      // !!!! STATISCH !!!!

  static UCHAR ObjBuf[0x100];
  ULONG SessionID, ProcessID;
  APIRET rc;
  char *p;

  if (name == NULL)
    return (-1);

  memset (&startd, 0, sizeof (STARTDATA));
  memset (&ObjBuf, 0, sizeof (ObjBuf));

  p = (char *) name;
  while (*p && !isspace (*p))
    p++;
  if (*p)                       /* if any other char then \0 */
    *p++ = 0;                   /* zero it and step forward: this is the parameter line */
  else
    p = NULL;

  startd.Length = sizeof (STARTDATA);
  startd.FgBg = SSF_FGBG_BACK;
  startd.Related = SSF_RELATED_INDEPENDENT;
  startd.TraceOpt = SSF_TRACEOPT_NONE;
  startd.InheritOpt = SSF_INHERTOPT_SHELL;
  startd.SessionType = SSF_TYPE_DEFAULT;
  startd.PgmControl = SSF_CONTROL_MINIMIZE | SSF_CONTROL_INVISIBLE;
  startd.ObjectBuffer = (PSZ) & ObjBuf;
  startd.ObjectBuffLen = sizeof (ObjBuf);
  startd.TermQ = NULL;
  startd.PgmName = name;
  startd.PgmInputs = (unsigned char *) p;
  startd.PgmTitle = name;

  rc = DosStartSession (&startd, &SessionID, &ProcessID);
  if (rc != 0)
  {
    status_line ("!cannot execute '%s'[rc=%08x]", name, rc);
  }

  return (rc != 0);
}

#else
#ifdef _WIN32

/* alex, 97-02-14, ExecSession() for Win32 */

int
ExecSession (unsigned char *name)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pinf;
  BOOL rc;

  memset ((void *) &si, 0, sizeof (si));
  si.cb = sizeof (si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  /* HJK 97/10/10 Show the window, but do not activate */
  si.wShowWindow = SW_SHOWNOACTIVATE;
  rc = CreateProcess (NULL, name,
                      NULL, NULL,
                      FALSE,
                      CREATE_NEW_CONSOLE,
                      NULL, NULL,
                      &si,
                      &pinf);

  return (!rc);                 /* API returns TRUE, if everything was Ok */
}

#else

/* ====================================================================== */

int
ExecSession (unsigned char *name)  /* r. hoerner */
{
  name = name;
  return (-1);
}

#endif /* _WIN32 */
#endif /* OS_2 */

/* $Id: b_spawn.c,v 1.5 1999/03/22 03:47:16 mr Exp $ */
