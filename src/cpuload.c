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
 * Filename    : $Source: E:/cvs/btxe/src/cpuload.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:48 $
 * State       : $State: Exp $
 *
 * Description : calculate CPU load (OS/2)
 *
 * Note        :
 *   you need OS/2 Warp 4.0, OS/2 Warp 3.0 + FP29 or OS/2 Warp SMP to use
 *   DosPerfSysCall. DosPerfSysCall (DOSCALLS.976) is documented in SMP.INF
 *
 *---------------------------------------------------------------------------*/

#ifdef SMP_API

#include "includes.h"

#define CMD_KI_RDCNT 0x63
typedef struct _CPUUTIL
{
  ULONG ulTimeLow;
  ULONG ulTimeHigh;
  ULONG ulIdleLow;
  ULONG ulIdleHigh;
  ULONG ulBusyLow;
  ULONG ulBusyHigh;
  ULONG ulIntrLow;
  ULONG ulIntrHigh;
}
CPUUTIL;

#define LL2F(high,low) (4294967296.0*high+low)
#define SLEEP_SEC      2
#define CPU            1

/* local functions */

static VOID cpu_time (PVOID);

/* global variables */

static BOOL32 running = FALSE;
static HMTX hmtx = NULLHANDLE;
static double idle = 100.0;
static double busy = 0.0;
static double intr = 0.0;

/* start the thread */

BOOL32
smp_init (void)
{
  if (hmtx == NULLHANDLE)
    DosCreateMutexSem (NULL, &hmtx, 0, FALSE);
  if (!running)
    return (_beginthread (cpu_time, NULL, 32768L, NULL) != -1);
  else
    return TRUE;
}

/* this thread calculates the time that the CPU is idle, busy or */
/* processing interrupts                                         */

VOID
cpu_time (PVOID ignore_me)
{
  BOOL32 calc = FALSE;
  APIRET ulrc;
  UCHAR LoadError[26];
  HMODULE hmodule;
  PFN DosPerfSysCall;

  double ts_val, ts_val_prev;
  double idle_val, idle_val_prev;
  double busy_val, busy_val_prev;
  double intr_val, intr_val_prev;
  CPUUTIL CPUUtil[CPU];

  ignore_me = ignore_me;

  ulrc = DosLoadModule (LoadError, sizeof (LoadError), "DOSCALLS", &hmodule);
  if (ulrc)
    _endthread ();
  ulrc = DosQueryProcAddr (hmodule, 976, NULL, &DosPerfSysCall);
  if (ulrc)
    _endthread ();

  do
  {
    ulrc = DosPerfSysCall (CMD_KI_RDCNT, (ULONG) CPUUtil, 0, 0);
    if (!ulrc)
    {
      running = TRUE;

      ts_val = LL2F (CPUUtil->ulTimeHigh, CPUUtil->ulTimeLow);
      idle_val = LL2F (CPUUtil->ulIdleHigh, CPUUtil->ulIdleLow);
      busy_val = LL2F (CPUUtil->ulBusyHigh, CPUUtil->ulBusyLow);
      intr_val = LL2F (CPUUtil->ulIntrHigh, CPUUtil->ulIntrLow);

      if (calc)
      {
        double ts_delta = ts_val - ts_val_prev;

        if (DosRequestMutexSem (hmtx, SEM_INDEFINITE_WAIT) == NO_ERROR)
        {
          idle = (idle_val - idle_val_prev) / ts_delta * 100.0;
          busy = (busy_val - busy_val_prev) / ts_delta * 100.0;
          intr = (intr_val - intr_val_prev) / ts_delta * 100.0;
          DosReleaseMutexSem (hmtx);
        }
      }

      ts_val_prev = ts_val;
      idle_val_prev = idle_val;
      busy_val_prev = busy_val;
      intr_val_prev = intr_val;

      calc = TRUE;
      DosSleep (1000 * SLEEP_SEC);
    }
  }
  while (!ulrc);
  running = FALSE;
  _endthread ();
}

/* query the values for idle, busy and intr */
BOOL32
query_cpu (double *pidle, double *pbusy, double *pintr)
{
  if (running)
  {
    if (DosRequestMutexSem (hmtx, SEM_INDEFINITE_WAIT) == NO_ERROR)
    {
      *pidle = idle;
      *pbusy = busy;
      *pintr = intr;
      DosReleaseMutexSem (hmtx);
      /* status_line(">idle : %4.2f%% busy : %4.2f%% intr : %4.2f%%", */
      /*             idle,busy,intr);                                 */
      return TRUE;
    }
  }
  return FALSE;
}

#endif

/* $Id: cpuload.c,v 1.3 1999/02/27 01:15:48 mr Exp $ */
