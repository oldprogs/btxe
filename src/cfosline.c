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
 * Filename    : $Source: E:/cvs/btxe/src/cfosline.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:45 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : CFOS/2 support module
 *
 * Note        :
 *   Be aware that as of today this module is based on a beta version of
 *   CFOS/2. Bink is the first program supporting this new feature of CFOS/2
 *   and it is being used as play field. As this time everything works ok;
 *   but this module will have to be updated as new betas of CFOS are
 *   released; the functions in CFOS/2 to support external programs
 *   should not be considered to be final or stable yet.
 *   Minimum CFOS/2 build required: 1214
 *
 *---------------------------------------------------------------------------*/

#ifdef OS_2

#include "includes.h"

#define CFOSCATEGORY 0x90
#define CFOSFUNCTION 0x42

#define CFOS_SUCCESS            0
#define CFOS_NOCONNECTION       1
#define CFOS_INPROGRESS         2
#define CFOS_NOTAVAILABLE       3
#define CFOS_NONSENSE         100
#define CFOS_LASTBCHANNEL  0xFFFF

#ifndef NEED_OS2COMMS
extern HCOMM hcModem;

#define PortHandle ComGetFH (hcModem)
#else
extern HFILE hfComHandle;

#define PortHandle hfComHandle
#endif


typedef struct
{
  word ax, bx, cx, _res1, _res2, _res3;
  word buflen;
}
APPCALL;

typedef struct
{
  word ax, bx, cx, dx, _res2, _res3;
  union
  {
    char raw[512];
    struct
    {
      word offadd;
      dword zero;
      byte bchannels;
      dword conninfo;
      word CPS;
      dword B1rate;
      char unknown;
    }
    cooked;
  }
  buffer;
}
APPRET;

/* -------------------------------------------------------------------- */
/* First, some screen support. We can't use any of the SBUF.C functions */
/* because they use a lot of variable variables which fucks up          */
/* multithreading. We have to fix that!                                 */
/* -------------------------------------------------------------------- */
extern CELLP Scrnbuf;
extern BUFFER Sbuf;

void
NotifyChar (char ch, int x)
{
  (Scrnbuf + (SB_ROWS - 1) * SB_COLS + x)->b.ch = (unsigned char) ch;
  if (x < Sbuf.lcol[SB_ROWS - 1])
    Sbuf.lcol[SB_ROWS - 1] = x;
  if (x > Sbuf.rcol[SB_ROWS - 1])
    Sbuf.rcol[SB_ROWS - 1] = x;
  Sbuf.flags |= SB_DELTA;
}

void
NotifyAt (char *s, int x)
{
  while (*s && x < SB_COLS)
  {
    NotifyChar (*s++, x);
    x++;
  }
}

void
Notify (char *s)
{
  char us[81];

  NotifyAt (s, 0);
  memset (us, ' ', 80);
  us[80 - strlen (s)] = 0;
  NotifyAt (us, strlen (s));
}

/* -------------------------------------------------------------------- */
/*  CFOS functions.                                                     */
/* -------------------------------------------------------------------- */
int
CFOSResetHardware (void)
{
  APPCALL input;
  APPRET output;
  ULONG LParam, LReturn;

  input.ax = 0x9001;
  DosDevIOCtl2 (PortHandle, CFOSCATEGORY, CFOSFUNCTION, &input,
                sizeof (input), &LParam, &output, sizeof (output), &LReturn);
  return output.ax;
}

word
CFOSAddOrRemoveChannel (int what)
{
  APPCALL input;
  APPRET output;
  ULONG LParam, LReturn;

  if (what != 1 && what != -1)
    return CFOS_NONSENSE;
  input.ax = 0x9004;
  if (what == 1)
    input.bx = 1;
  else
    input.bx = 2;

  DosDevIOCtl2 (PortHandle, CFOSCATEGORY, CFOSFUNCTION, &input,
                sizeof (input), &LParam, &output, sizeof (output), &LReturn);

  return output.ax;
}

/* -------------------------------------------------------------------- */
/* This function is the heart of the CFOS support. It must be started   */
/* as thread.                                                           */
/* -------------------------------------------------------------------- */
void
CFOSMonitorThread (PVOID x)
{
  int keepon = 1, appendage = 0, ec = 0;
  ULONG LParam, LReturn;
  char buffer[81], *grial;
  APPCALL input;
  APPRET output;
  static time_t it, diff;
  struct tm *tim;
  word res;

  x = x;                        /*  Don't bother me with 'you are not using - 'x' */
  printf ("CFOS Monitor thread successfully started.\n");
  do
  {
    DosSleep (100);             /* Updating the information line every 100 ms is enough */
    if (PortHandle == NULLHANDLE)  /* Port is not ready yet *//* TE011097: NULL->NULLHANDLE */
    {
      if (wholewin)             /* So we write the usual bottom line, if we can */
        bottom_line (1);
    }
    else
    {
      if (!appendage)           /* We don't know if there is a CFOS supporting */
        /* external peek */
      {
        input.ax = 0x9000;
        output.ax = output.cx = output.dx = 0;
        DosDevIOCtl2 (PortHandle, CFOSCATEGORY, CFOSFUNCTION, &input,
               sizeof (input), &LParam, &output, sizeof (output), &LReturn);
        if (output.ax != 0x1969 || output.cx != 0x6346 || output.dx != 0x6f73)
        {
          /* No CFOS with appendage support */
          while (!wholewin)     /* Wait until we can report it */
          {
            printf ("Waiting for wholewin.\n");
            DosSleep (100);
          }
          sprintf (buffer, " There is no CFOS with appendage support. AX=%hX, CX=%hX, DX=%hX",
                   output.ax, output.cx, output.dx);
          Notify (buffer);
          DosSleep (5000);      /* Wait 5 seconds so the user can read it */
          Notify ("                                                                              ");
          keepon = 0;           /* Let thread die */
          MonitorCFOS = 0;      /* Make last line available */
          bottom_line (0);      /* Update it */
        }
        else
        {
          /* OK, CFOS is here */
          while (!wholewin)     /* Wait until we can report it */
            DosSleep (100);
          Notify (" CFOS detected.");
          DosSleep (5000);      /* Wait 5 seconds so the user can read it */
          appendage = 1;
        }
      }
      else
      {
        memset (buffer, ' ', 80);
        buffer[80] = 0;

        /* Get ISDN time */
        input.ax = 0x9005;
        input.bx = 1;           /* Don't reset CFOS timediff */
        DosDevIOCtl2 (PortHandle, CFOSCATEGORY, CFOSFUNCTION, &input,
               sizeof (input), &LParam, &output, sizeof (output), &LReturn);
        time (&it);
        if (output.cx != 0x7fff || output.bx != 0xffff)
        {
          diff = output.cx << 16;
          diff += output.bx;
          it += diff;
          tim = localtime (&it);
          if (tim != NULL)
            sprintf (buffer, "%02d/%02d/%02d %02d:%02d³", tim->tm_year % 100,
                  tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min);
          else
            strcpy (buffer, " No localtime ³");
        }
        else
          strcpy (buffer, " No time info ³");
        /* Process user requests, if any */
        if (ReqHardwareReset)
        {
          char *ct = buffer + strlen (buffer);

          sprintf (ct, " Received user request to reset ISDN hardware.");
          Notify (buffer);
          DosSleep (2000);
          if (CFOSResetHardware ())
            sprintf (ct, " CFOS failed to reset hardware.");
          else
            sprintf (ct, " ISDN hardware successfully reset by CFOS.");
          Notify (buffer);
          DosSleep (2000);
          ReqHardwareReset = 0;
        }
        else if (ReqChannelChange)
        {
          char *ct = buffer + strlen (buffer);

          if (ReqChannelChange != -1 && ReqChannelChange != 1)
            sprintf (ct, " Absurd value (%d) in ReqChannelChange, ignored.", ReqChannelChange);
          else
          {
            if (ReqChannelChange == 1)
              sprintf (ct, " Received user request to add a b-channel.");
            else
              sprintf (ct, " Received user request to remove a b-channel.");
            Notify (buffer);
            DosSleep (2000);
            switch ((res = CFOSAddOrRemoveChannel (ReqChannelChange)))
            {
            case CFOS_SUCCESS:
              sprintf (ct, " Request succeeded.");
              break;
            case CFOS_NOCONNECTION:
              sprintf (ct, " Failed: No active connection.");
              break;
            case CFOS_INPROGRESS:
              sprintf (ct, " Failed: Connect/disconnect already in progress.");
              break;
            case CFOS_NOTAVAILABLE:
              sprintf (ct, " Failed: Not available in this version of CFOS.");
              break;
            case CFOS_NONSENSE:
              sprintf (ct, " CFOS was called with a nonsense value.");
              break;
            case CFOS_LASTBCHANNEL:
              sprintf (ct, " Not supported or attempted to remove last b-channel.");
              break;
            default:
              sprintf (ct, " CFOS returned an unknown value (%u).", res);
              break;
            }
          }
          Notify (buffer);
          DosSleep (2000);
          ReqChannelChange = 0;
        }
        else
        {
          /* Get connection info block */
          input.ax = 0x9002;
          input.cx = sizeof (output.buffer);
          input.buflen = sizeof (output.buffer);
          DosDevIOCtl2 (PortHandle, CFOSCATEGORY, CFOSFUNCTION, &input,
               sizeof (input), &LParam, &output, sizeof (output), &LReturn);
          if (output.ax)        /* Function failed */
          {
            ec++;
            sprintf (buffer, " Call for querying info failed. AX=%hX (%d)",
                     output.ax, ec);
            Notify (buffer);
            DosSleep (5000);
          }
          else
          {
            if (!output.buffer.cooked.bchannels)
            {
              strcat (buffer, " No active connection.");
              buffer[strlen (buffer)] = ' ';
            }
            else
            {
              grial = &output.buffer.cooked.unknown + 5;
              sprintf (buffer + strlen (buffer), "%s:%-14s³Ch:%02d³%s³%s³CPS:%5hu³%6hu³",
                       (output.buffer.cooked.conninfo & 4) ? "Out" : " In",
                       (char *) grial, output.buffer.cooked.bchannels,
                       (output.buffer.cooked.conninfo & 1) ? "Ctr" : "   ",
                       (output.buffer.cooked.conninfo & 2) ? "EC" : "  ",
                       output.buffer.cooked.CPS,
                       (unsigned int) output.buffer.cooked.B1rate
                );

              /* Get cost data - I don't have cost info here so I don't
                 know if this works, please test */
              input.ax = 0x9003;
              input.bx = 0xffff;
              input.buflen = sizeof (output.buffer);
              DosDevIOCtl2 (PortHandle, CFOSCATEGORY, CFOSFUNCTION, &input,
               sizeof (input), &LParam, &output, sizeof (output), &LReturn);
              if (output.ax)
              {
                sprintf (buffer + strlen (buffer), "No cost info");
                buffer[strlen (buffer)] = ' ';
              }
              else
                sprintf (buffer + strlen (buffer), "Cst:%06u³%05u",
                         output.bx, output.cx);
            }
            Notify (buffer);
          }                     /* info block correctly retrieved */
        }                       /* No requests to change conditions */
      }                         /* appendage found */
    }
  }
  while (keepon);
}

int
CFOSMonitorStart (void)
{
  _beginthread (CFOSMonitorThread, NULL, 32768, NULL);
  /* We got your message on the radio, condition's normal and you're */
  /* coming home */
  return 1;
}

#endif

/* $Id: cfosline.c,v 1.3 1999/02/27 01:15:45 mr Exp $ */
