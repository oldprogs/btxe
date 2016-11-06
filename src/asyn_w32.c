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
 * Filename    : $Source: E:/cvs/btxe/src/asyn_w32.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/06/01 18:50:26 $
 * State       : $State: Exp $
 *
 * Description : async module for Win32
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */
#include "includes.h"

#ifndef USE_WINFOSSIL
#pragma message("Building ASYNC module for NTCOMM.DLL")

#include "third\ntcomm.h"

/*
 * adopted to BTXE/Win32 and ntcomm.dll by alexander sanda, 97-02-22
 * thanks to scott for providing the necessary information about
 * ntcomm.dll
 */

// Rewrote parts of the async routines for ntcomm.dll and made it work
// finally also under Win95 - HJK 98/2/7

char ascii_comport[] = "COM";

HCOMM hcModem = 0;              /* comm.dll handle */

#define RBSIZE  8200
#define TBSIZE  8200

DWORD rBufsize = RBSIZE;
DWORD tBufsize = TBSIZE;

// HJK 99/01/25 - an attempt to make Bink work with CFos

void
com_DTR_on (void)
{
  DCB dcb;
  DWORD dwIgnore;

  if (NoModem)
    return;

  if (SIOMode)
  {
    ClearCommError (ComGetHandle (hcModem), &dwIgnore, NULL);

    if (!EscapeCommFunction (ComGetHandle (hcModem), SETDTR))
    {
      status_line ("!SYS%04hu: EscapeCommFunction(SETDTR)", GetLastError ());
      exit (3);
    }
  }
  else
  {
    if (ComGetDCB (hcModem, &dcb))
    {
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;  /* raise DTR */
      ComSetDCB (hcModem, &dcb);
    }
    else
    {
      status_line ("!SYS%04hu: ComGetDCB()", GetLastError ());
      exit (3);
    }
  }
}

void
com_DTR_off (void)
{
  DCB dcb;
  DWORD dwIgnore;

  if (NoModem)
    return;
  if (SIOMode)
  {
    ClearCommError (ComGetHandle (hcModem), &dwIgnore, NULL);

    if (!EscapeCommFunction (ComGetHandle (hcModem), CLRDTR))
    {
      status_line ("!SYS%04hu: EscapeCommFunction(CLRDTR)", GetLastError ());
      exit (3);
    }
  }
  else
  {
    if (ComGetDCB (hcModem, &dcb))
    {
      dcb.fDtrControl = DTR_CONTROL_DISABLE;  /* lower DTR */
      ComSetDCB (hcModem, &dcb);
      com_kick ();
    }
    else
    {
      status_line ("!SYS%04hu: ComGetDCB()", GetLastError ());
      exit (3);
    }
  }
}

void
com_XON_disable (void)
{
  DCB dcb;

  if (NoModem)
    return;

  if (ComGetDCB (hcModem, &dcb))
  {
    // disable auto Xmit and recv flow control

    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    ComSetDCB (hcModem, &dcb);
    com_kick ();
  }
  else
  {
    status_line ("!SYS%04hu: ComGetDCB()", GetLastError ());
    exit (3);
  }
}

void
com_XON_enable (void)
{
  DCB dcb;

  if (NoModem)
    return;

  if (ComGetDCB (hcModem, &dcb))
  {
    // enable auto Xmit and recv flow control

    dcb.fOutX = TRUE;
    dcb.fInX = TRUE;
    ComSetDCB (hcModem, &dcb);
  }
  else
  {
    status_line ("!SYS%04hu: ComGetDCB()", GetLastError ());
    exit (3);
  }
}

int
com_getc (int t)
{
  if (NoModem)
    return (0);

  if (t)
    ComRxWait (hcModem, t * 1000L);
  return ComGetc (hcModem);
}

/* com_break() : start break if on==TRUE, stop break if on==FALSE */

void
com_break (int on)
{
  DWORD dwIgnore;

  if (NoModem)
    return;

  if (on)
    SetCommBreak (ComGetHandle (hcModem));
  else
    ClearCommBreak (ComGetHandle (hcModem));

  ClearCommError (ComGetHandle (hcModem), &dwIgnore, NULL);
}

void
MDM_ENABLE (unsigned long rate)
{
  BYTE _parity;
  BYTE databits;
  BYTE stopbits;

  if (NoModem)
    return;

  if (!hcModem)
    return;

  if (lock_baud && rate != max_baud.rate_value)
  {
    status_line (">Pretending baud is %lu (locked at %lu)", rate, max_baud.rate_value);
    rate = max_baud.rate_value;
  }

  databits = 8;
  stopbits = ONESTOPBIT;

  switch (parity)
  {
  case NO_PARITY:
    _parity = NOPARITY;
    break;
  case ODD_PARITY:
    _parity = ODDPARITY;
    break;
  case EVEN_PARITY:
    _parity = EVENPARITY;
    break;
  default:
    _parity = NOPARITY;
    status_line ("!Invalid parity");
  }

  ComSetBaudRate (hcModem, rate, _parity, databits, stopbits);
}

void
MDM_DISABLE (void)
{
  if (NoModem)
    return;

  if (hcModem)
  {
    ComClose (hcModem);
    hcModem = 0;
  }
}

void
ShowMdmSettings (void)
{
  DCB dcb;
  char *On = "On";
  char *Off = "Off";

  if (NoModem)
    return;

  if (ComGetDCB (hcModem, &dcb))
  {
    status_line (">Modem: XON(Rx)=%s, XON(Tx)=%s",
                 dcb.fInX ? On : Off,
                 dcb.fOutX ? On : Off);
    status_line (">Modem: IDSR=%s, ODSR=%s, OCTS=%s",
                 dcb.fDsrSensitivity ? On : Off,
                 dcb.fOutxDsrFlow ? On : Off,
                 dcb.fOutxCtsFlow ? On : Off);
    status_line (">Modem: Buffers R: %hu T: %hu", rBufsize, tBufsize);
  }
  else
  {
    status_line ("!SYS%04hu: ComGetDCB()", GetLastError ());
    exit (3);
  }
}

unsigned
Com_init (int port)
{
  USHORT rc;
  char *s;
  char dev[256];
  DCB dcb;

  if (NoModem)
    return (0);

  if (port_device == NULL)
    port_device = ascii_comport;

  sprintf (dev, "%s%d", port_device, port + 1);

  printf ("Attempting to open async device %s\r\n", dev);

  // this should also work under Win32, so I didn't see any reason for removing it - alex

  s = getenv ("RBUF");          /* MB 94-01-02 */
  if (s)
    rBufsize = (USHORT) min (32000, atoi (s));
  else
    rBufsize = RBSIZE;

  s = getenv ("TBUF");
  if (s)
    tBufsize = (USHORT) min (32000, atoi (s));
  else
    tBufsize = TBSIZE;

  // alex, 97-02-22: Instead of returning a error code, Win32 APIs always
  // return TRUE if everything went ok, FALSE otherwise.
  // You have to explicitely obtain the error code with GetLastError()

  rc = (!ComOpen ((LPTSTR) dev, &hcModem, rBufsize, tBufsize));
  if (rc)
  {
    status_line ("!SYS%04hu:  ComOpen(%s)", GetLastError (), dev);
    return (rc);
  }

  // HJK - 98/3/23 - Solves a bug with Windows 95 which caused Bink to
  // hang at exit. Fixed in new NTCOMM.DLL (98/05/11) but Com needs to be
  // closed at exit now.

  atexit (MDM_DISABLE);

  // Some specific COM settings, because MODE.COM from Windows95
  // has not all features which NT has - HJK, 98/6/12

  if (ComGetDCB (hcModem, &dcb))
  {
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fOutxCtsFlow = TRUE;
    dcb.fDsrSensitivity = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    ComSetDCB (hcModem, &dcb);
  }
  else
  {
    status_line ("!SYS%04hu: ComGetDCB()", GetLastError ());
    exit (3);
  }

  ShowMdmSettings ();

  return (rc);
}

unsigned
Cominit (int port, int failsafe)
{
  int ret = 0x1954;

  failsafe = failsafe;

  if (NoModem)
    return (ret);

  if (!hcModem)
  {
    if (Com_init (port))
    {
      ret = 0;
    }
  }
  else
    ComResume (hcModem);

  return ret;
}

/* force transmitter to go */

void
com_kick (void)
{
  if (NoModem)
    return;

  ComPurge (hcModem, COMM_PURGE_RX);  /* was: COMM_PURGE_ALL */
}

/*
 * the following two functions were originally written by MB. Because ntcomm.dll works
 * very similar to maxcomm.dll, they work under Win32 too - alex.
 */

/* Currently only used by HYDRA */

/*
 * This is a lot like ComWrite, but will return the sent-character
 * count if either a timeout occurs or carrier is lost. The timer
 * is specified by passing in a time_t for end-of-time. If zero is
 * passed for the timer, we'll just check for carrier loss.
 *
 */

USHORT
ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer)
{
  USHORT cbRemaining = cbBuf;
  USHORT cbToSend;
  int fTimedOut;

  if (NoModem)
    return (0);

  fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  while (!fTimedOut && CARRIER && cbRemaining)
  {
    cbToSend = ComOutSpace (hcModem);
    if ((--cbToSend) > 0)
    {
      if (cbToSend > cbRemaining)
        cbToSend = cbRemaining;
      ComWrite (hcModem, lpBuf, cbToSend);
      lpBuf += cbToSend;
      cbRemaining -= cbToSend;
    }
    else
      time_release ();
    fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  }

  return (cbBuf - cbRemaining);
}

USHORT
ComTXRemain (void)              /* MB 93-12-03 */
{
  if (NoModem)
    return (0);

  return (tBufsize - ComOutSpace (hcModem));
}

#else /* USE_NTCOMM */

#pragma message("Building ASYNC module for WinFossil")
/*
 * begin of the WinFossil comm stuff.
 */

#include "third\wnfossil.h"

typedef unsigned char byte;
extern unsigned int comm_bits;
extern unsigned int parity;
extern unsigned int stop_bits;
extern void _cdecl status_line (char *,...);
extern long timerset (unsigned int);
extern bool timeup (long);      /* HJK 040897 */
extern void time_release (void);

#define FOSSIL_BUFFER_SIZE 128
extern char fossil_buffer[];
extern char *fossil_fetch_pointer;
extern int fossil_count;

extern char out_buffer[];
extern char *out_send_pointer;
extern int out_count;


HANDLE hcModem = 0;             /* comm handle            */
HANDLE hRead = 0;               /* Read event handle      */
HANDLE hWrite = 0;              /* Write event handle     */
OVERLAPPED ovRead;              /* Read overlapped stuct  */
OVERLAPPED ovWrite;             /* Write overlapped stuct */
DWORD FailSafeTimer = 0;

BOOL fWriteWait = FALSE;        /* Now waiting for write  */

static ULONG SavedRate = 0;     /* Last set baud rate     */

DWORD FAR PASCAL ComWatchProc (LPVOID nothing);
HANDLE hWatchThread = 0;        /* Watch comm events      */
DWORD dwWatchThread = 0;        /* Thread identifier      */
CRITICAL_SECTION csWatchThread; /* Critical section for watched events */

extern int happy_compiler;      /* alex, 97-02-12 */

#define RBSIZE    8192L
#define TBSIZE    8192L

USHORT rBufsize = RBSIZE;       /* alex */
USHORT tBufsize = TBSIZE;

#define INBUF_SIZE RBSIZE
#define OUTBUF_SIZE TBSIZE

/* ======================================================================== */
/* ======================================================================== */
/* ==                                                                    == */
/* == The following code was added by Bryan Woodruff to permit BT32 to   == */
/* == use communications services provided by his WinFOSSIL driver, and  == */
/* == to thereby allow BT32 to be the frontend for a DOS FOSSIL-based    == */
/* == BBS system.                                                        == */
/* ==                                                                    == */
/* == Please contact Bryan at 1:343/294@fidonet for details.             == */
/* ==                                                                    == */
/* ======================================================================== */
/* ======================================================================== */

BOOL
ControlWait (HANDLE hFOSSIL, DWORD dwIoControlCode, PVOID pvIn, ULONG
             cbIn, PVOID pvOut, ULONG cbOut, PULONG pcbReturned)
{
  BOOL fResult;

  if (NoModem)
    return (TRUE);

  fResult = DeviceIoControl (((PFOSSIL_PORTINFO) hFOSSIL)->hDevice,
                             dwIoControlCode,
                             pvIn,
                             cbIn,
                             pvOut,
                             cbOut,
                             pcbReturned,
                             &((PFOSSIL_PORTINFO) hFOSSIL)->ov);

  if (!fResult)
  {
    if (ERROR_IO_PENDING == GetLastError ())
    {
      WaitForSingleObject (((PFOSSIL_PORTINFO) hFOSSIL)->ov.hEvent, INFINITE);
      fResult = TRUE;
    }
    else
      fResult = FALSE;

    ResetEvent (((PFOSSIL_PORTINFO) hFOSSIL)->ov.hEvent);
  }

  return fResult;
}

void
ComDTROn (HANDLE hFOSSIL)
{
  FOSSIL_CONTROL DeviceControl;
  ULONG cbReturned;

  if (NoModem)
    return;

  DeviceControl.ulControl = FOSSIL_CONTROL_DTR;
  DeviceControl.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  DeviceControl.ulParam = 1;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_CONTROL,
                    &DeviceControl,
                    sizeof (DeviceControl),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(DTR-ON)", GetLastError ());
    exit (3);
  }
}

void
ComDTROff (HANDLE hFOSSIL)
{
  FOSSIL_CONTROL DeviceControl;
  ULONG cbReturned;

  if (NoModem)
    return;

  DeviceControl.ulControl = FOSSIL_CONTROL_DTR;
  DeviceControl.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  DeviceControl.ulParam = 0;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_CONTROL,
                    &DeviceControl,
                    sizeof (DeviceControl),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(DTR-OFF)", GetLastError ());
    exit (3);
  }
}

void
ComXONDisable (HANDLE hFOSSIL)
{
  FOSSIL_CONTROL DeviceControl;
  ULONG cbReturned;

  if (NoModem)
    return;

  DeviceControl.ulControl = FOSSIL_CONTROL_FLOWCTL;
  DeviceControl.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  DeviceControl.ulParam = FOSSIL_FLOWCTLF_RTSCTS;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_CONTROL,
                    &DeviceControl,
                    sizeof (DeviceControl),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(DISABLEXON)", GetLastError ());
    exit (3);
  }
}

void
ComXONEnable (HANDLE hFOSSIL)
{
  FOSSIL_CONTROL DeviceControl;
  ULONG cbReturned;

  if (NoModem)
    return;

  DeviceControl.ulControl = FOSSIL_CONTROL_FLOWCTL;
  DeviceControl.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  DeviceControl.ulParam = FOSSIL_FLOWCTLF_RTSCTS |
    FOSSIL_FLOWCTLF_INX |
    FOSSIL_FLOWCTLF_OUTX;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_CONTROL,
                    &DeviceControl,
                    sizeof (DeviceControl),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(ENABLEXON)", GetLastError ());
    exit (3);
  }
}

void
ComBreak (HANDLE hFOSSIL, int on)
{
  FOSSIL_CONTROL DeviceControl;
  ULONG cbReturned;

  if (NoModem)
    return;

  DeviceControl.ulControl = FOSSIL_CONTROL_BREAK;
  DeviceControl.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  DeviceControl.ulParam = on;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_CONTROL,
                    &DeviceControl,
                    sizeof (DeviceControl),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(BREAK)", GetLastError ());
    exit (3);
  }
}

short
ComSetParms (HANDLE hFOSSIL, ULONG rate)
{
  FOSSIL_SETPARAMS SetParams;
  ULONG cbReturned;

  if (NoModem)
    return (1);

  SetParams.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  SetParams.ByteSize = (comm_bits == 3) ? 8 : 7;
  SetParams.StopBits = (stop_bits == 4) ? TWOSTOPBITS : ONESTOPBIT;

  switch (parity)
  {
  case 0x08:
    SetParams.Parity = ODDPARITY;
    break;

  case 0x18:
    SetParams.Parity = EVENPARITY;
    break;

  default:
    SetParams.Parity = NOPARITY;
    break;
  }

  SetParams.BaudRate = rate;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_SETPARAMS,
                    &SetParams,
                    sizeof (SetParams),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(SETPARAMS)", GetLastError ());
    exit (3);
  }
  else
    SavedRate = rate;

  return 1;
}

void
ComDeInit (HANDLE hFOSSIL)
{
  ULONG cbReturned;

  if (NoModem)
    return;

  if (hFOSSIL)
  {
    ComTXPurge (hFOSSIL);
    ComRXPurge (hFOSSIL);

    if (!ControlWait (hFOSSIL,
                      IOCTL_FOSSIL_DEACTIVATE_PORT,
                      &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                      sizeof (ULONG),
                      NULL,
                      0,
                      &cbReturned))
    {
      status_line ("!SYS%08u: Control(DEACTIVATEPORT)", GetLastError ());
      exit (3);
    }

    CloseHandle (((PFOSSIL_PORTINFO) hFOSSIL)->hDevice);
    CloseHandle (((PFOSSIL_PORTINFO) hFOSSIL)->ov.hEvent);
    HeapFree (GetProcessHeap (), 0, hFOSSIL);
    hcModem = 0;
  }
}

int
com_getc (int t)
{
  long t1 = 0;

  if (NoModem)
    return (0);

  if (fossil_count == 0)
  {
    ComRxWait (hcModem, 0);

    while (fossil_count == 0)
    {
      if (!t1)
        t1 = timerset ((unsigned int) (t * 1000));
      else if (timeup (t1))
      {
        return (-1);
      }

      /*
       * This should work because we only do TIMED_READ when we have
       * carrier
       */

      if (!ComCarrier (hcModem))
      {
        return (-1);
      }

      time_release ();
      ComRxWait (hcModem, 0);
    }
  }
  return ComGetc (hcModem);
}

int
ComGetFH (HANDLE hFOSSIL)
{
  return ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId + 1;
}

short
ComCarrier (HANDLE hFOSSIL)
{
  ULONG cbReturned, ulStatus;

  if (NoModem)
    return (0);

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_GETPORTSTATUS,
                    &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                    sizeof (ULONG),
                    &ulStatus,
                    sizeof (ULONG),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(SETDTR)", GetLastError ());
    exit (3);
  }

  return (SHORT) (ulStatus & FOSSIL_STATUSF_MS_RLSD_ON);
}

unsigned short
ComGetc (HANDLE hFOSSIL)
{
  unsigned char c;

  if (NoModem)
    return (0);

  while (fossil_count == 0)
  {
    time_release ();
    ComRxWait (hFOSSIL, 0);
  }

  --fossil_count;
  c = (unsigned char) *fossil_fetch_pointer++;
  return ((unsigned short) c);
}

short
ComInCount (HANDLE hFOSSIL)
{
  if (NoModem)
    return (0);

  ComRxWait (hFOSSIL, 0L);
  return fossil_count;
}

short
ComOutCount (HANDLE hFOSSIL)
{
  FOSSIL_INFORMATION Information;
  ULONG cbReturned;

  if (NoModem)
    return (0);

  /* Special case of ComTxWait ... it's OK to return 0 if
     nothing is pending and 1 if anything is... */

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_GETPORTINFO,
                    &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                    sizeof (ULONG),
                    &Information,
                    sizeof (FOSSIL_INFORMATION),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(GETPORTINFO) (%d)", GetLastError (),
                 ComGetFH (hFOSSIL));
    exit (3);
  }

  if (Information.cbTransmitFree != Information.cbTransmitBuffer)
    return 1;

  if (out_count == 0)
    return 0;

  ComWrite (hFOSSIL, NULL, 0);

  return ComOutCount (hFOSSIL);
}

short
ComOutSpace (HANDLE hFOSSIL)
{
  FOSSIL_INFORMATION Information;
  ULONG cbReturned;

  if (NoModem)
    return (0);

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_GETPORTINFO,
                    &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                    sizeof (ULONG),
                    &Information,
                    sizeof (FOSSIL_INFORMATION),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(GETPORTINFO) (%d)", GetLastError (),
                 ComGetFH (hFOSSIL));
    exit (3);
  }

  return Information.cbTransmitFree;
}

int
ComPause (HANDLE hFOSSIL)
{
  if (NoModem)
    return (0);

  ComRXPurge (hFOSSIL);
  return 0;
}

short
ComPeek (HANDLE hFOSSIL)
{
  unsigned char c;

  if (NoModem)
    return (-1);

  if (fossil_count == 0)
  {
    if (!ComRxWait (hFOSSIL, 0L))
      return (unsigned short) -1;
  }
  c = (unsigned char) *fossil_fetch_pointer;
  return ((unsigned short) c);
}

void
ComTXPurge (HANDLE hFOSSIL)
{
  FOSSIL_CONTROL DeviceControl;
  ULONG cbReturned;

  if (NoModem)
    return;

  DeviceControl.ulControl = FOSSIL_CONTROL_PURGE;
  DeviceControl.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  DeviceControl.ulParam = FOSSIL_PURGE_TRANSMIT;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_CONTROL,
                    &DeviceControl,
                    sizeof (DeviceControl),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(PURGERX)", GetLastError ());
    exit (3);
  }

  out_send_pointer = out_buffer;
  out_count = 0;
}

void
ComRXPurge (HANDLE hFOSSIL)
{
  FOSSIL_CONTROL DeviceControl;
  ULONG cbReturned;

  if (NoModem)
    return;

  DeviceControl.ulControl = FOSSIL_CONTROL_PURGE;
  DeviceControl.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  DeviceControl.ulParam = FOSSIL_PURGE_RECEIVE;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_CONTROL,
                    &DeviceControl,
                    sizeof (DeviceControl),
                    NULL,
                    0,
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(PURGERX)", GetLastError ());
    exit (3);
  }

  fossil_fetch_pointer = fossil_buffer;
  fossil_count = 0;
}

int
ComResume (HANDLE hFOSSIL)
{
  FOSSIL_INFORMATION Information;
  ULONG cbReturned;

  if (NoModem)
    return (1);

  if (!hFOSSIL ||
      !ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_ACTIVATE_PORT,
                    &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                    sizeof (ULONG),
                    &Information,
                    sizeof (FOSSIL_INFORMATION),
                    &cbReturned) || !cbReturned)
  {
    status_line ("!SYS%08u: Control(REACTIVATE) (%d)", GetLastError (),
                 ComGetFH (hFOSSIL));
    exit (3);
  }

  return 1;
}

void
ComTxWait (HANDLE hFOSSIL, ULONG interval)
{
  FOSSIL_INFORMATION Information;
  ULONG cbReturned;

  interval = interval;          /* make compiler happy */

  if (NoModem)
    return;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_GETPORTINFO,
                    &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                    sizeof (ULONG),
                    &Information,
                    sizeof (FOSSIL_INFORMATION),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(GETPORTINFO) (%d)", GetLastError (),
                 ComGetFH (hFOSSIL));
    exit (3);
  }

  if (!out_count || (Information.cbTransmitFree < (ULONG) out_count))
    return;

  ComWrite (hFOSSIL, NULL, 0);
}

int
ComRxWait (HANDLE hFOSSIL, ULONG interval)
{
  FOSSIL_INFORMATION Information;
  FOSSIL_BUFFER Buffer;
  ULONG cbRead, cbReturned;

  interval = interval;          /* make compiler happy */

  if (NoModem)
    return (0);

  if (fossil_count)
    return fossil_count;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_GETPORTINFO,
                    &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                    sizeof (ULONG),
                    &Information,
                    sizeof (FOSSIL_INFORMATION),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(GETPORTINFO) (%d)", GetLastError (),
                 ComGetFH (hFOSSIL));
    exit (3);
  }

  cbRead = Information.cbReceiveBuffer - Information.cbReceiveFree;
  if (!cbRead)
    return 0;

  Buffer.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
  Buffer.pchBuffer = fossil_buffer;
  Buffer.cbBuffer = min (cbRead, FOSSIL_BUFFER_SIZE);

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_BLOCKRECV,
                    &Buffer,
                    sizeof (Buffer),
                    &cbRead,
                    sizeof (ULONG),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(BLOCKRECV) (%d)", GetLastError (),
                 ComGetFH (hFOSSIL));
    exit (3);
  }

  fossil_fetch_pointer = fossil_buffer;
  fossil_count = cbRead;

  return fossil_count;
}

short
ComPutc (HANDLE hFOSSIL, byte c)
{
  byte b = c;

  if (NoModem)
    return (1);

  ComWrite (hFOSSIL, &b, 1);
  return 1;
}

short
ComBufferByte (HANDLE hFOSSIL, byte c)
{
  if (NoModem)
    return (1);

  if (out_count == FOSSIL_BUFFER_SIZE)
    ComWrite (hFOSSIL, NULL, 0);

  out_count++;
  *out_send_pointer++ = c;
  return 1;
}

void
ComWriteEx (HANDLE hFOSSIL, void *pvBuffer, USHORT cbBuffer)
{
  FOSSIL_BUFFER Buffer;
  FOSSIL_INFORMATION Information;
  ULONG cbReturned, cbWrote;

  if (NoModem)
    return;

  if (!ControlWait (hFOSSIL,
                    IOCTL_FOSSIL_GETPORTINFO,
                    &((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId,
                    sizeof (ULONG),
                    &Information,
                    sizeof (FOSSIL_INFORMATION),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(GETPORTINFO) (%d)", GetLastError (),
                 ComGetFH (hFOSSIL));
    exit (3);
  }

  if (!cbBuffer || (Information.cbTransmitFree < (ULONG) cbBuffer))
    return;

  while (cbBuffer)
  {
    Buffer.ulPortId = ((PFOSSIL_PORTINFO) hFOSSIL)->ulPortId;
    Buffer.pchBuffer = pvBuffer;
    Buffer.cbBuffer = cbBuffer;

    if (!ControlWait (hFOSSIL,
                      IOCTL_FOSSIL_BLOCKXMIT,
                      &Buffer,
                      sizeof (Buffer),
                      &cbWrote,
                      sizeof (ULONG),
                      &cbReturned))
    {
      status_line ("!SYS%08u: Control(BLOCKXMIT) (%d)", GetLastError (),
                   ComGetFH (hFOSSIL));
      exit (3);
    }

    cbBuffer -= (USHORT) cbWrote;
  }
}

void
ComWrite (HANDLE hFOSSIL, void *pvBuffer, USHORT cbBuffer)
{
  if (NoModem)
    return;

  if (out_count)
  {
    ComWriteEx (hFOSSIL, out_buffer, (USHORT) out_count);

    out_send_pointer = out_buffer;
    out_count = 0;
  }

  if (cbBuffer)
  {
    ComWriteEx (hFOSSIL, pvBuffer, cbBuffer);
  }

  return;
}

unsigned short
Cominit (int port, int failsafe)
{
  FOSSIL_INFORMATION Information;
  ULONG cbReturned, ulId = port;

  FailSafeTimer = failsafe;

  if (NoModem)
    return (0x1954);

  hcModem =
    (HANDLE) HeapAlloc (GetProcessHeap (), 0, sizeof (FOSSIL_PORTINFO));

  ((PFOSSIL_PORTINFO) hcModem)->hDevice =
    CreateFile ("\\\\.\\FOSSIL",
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                NULL);

  if (((PFOSSIL_PORTINFO) hcModem)->hDevice == (HANDLE) - 1)
  {
    status_line ("!SYS%08u: CreateFile 'FOSSIL'", GetLastError ());
    return (FALSE);             /* NS971214  no fossil -> get out */
  }

  RtlZeroMemory (&((PFOSSIL_PORTINFO) hcModem)->ov, sizeof (OVERLAPPED));

  ((PFOSSIL_PORTINFO) hcModem)->ov.hEvent =
    CreateEvent (NULL, TRUE, FALSE, NULL);

  if ((((PFOSSIL_PORTINFO) hcModem)->ov.hEvent) == NULL)
  {
    status_line ("!SYS%08u: CreateEvent", GetLastError ());
    return (FALSE);             /* NS971214  no fossil -> get out */
  }

  ((PFOSSIL_PORTINFO) hcModem)->ulPortId = ulId;

  if (!ControlWait (hcModem,
                    IOCTL_FOSSIL_ACTIVATE_PORT,
                    &ulId,
                    sizeof (ULONG),
                    &Information,
                    sizeof (FOSSIL_INFORMATION),
                    &cbReturned))
  {
    status_line ("!SYS%08u: Control(ACTIVATEPORT) (%d)", GetLastError (), ulId);
    return (FALSE);             /* NS971214  no fossil -> get out */
  }

  fossil_fetch_pointer = fossil_buffer;
  fossil_count = 0;

  out_send_pointer = out_buffer;
  out_count = 0;

  return ((Information.wSignature == 0x4257) ? 0x1954 : 0);
}

void
com_kick (void)
{
  if (NoModem)
    return;

  ComTXPurge (hcModem);
}

/* Currently only used by HYDRA */

/*
 * This is a lot like ComWrite, but will return the sent-character
 * count if either a timeout occurs or carrier is lost. The timer
 * is specified by passing in a time_t for end-of-time. If zero is
 * passed for the timer, we'll just check for carrier loss.
 *
 */


/* added: alex, 97-02-09 */
/* changed: HJK, 970831 */

unsigned short
ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer)
{
  USHORT cbRemaining = cbBuf;
  USHORT cbToSend;
  BOOL fTimedOut;

  if (NoModem)
    return (0);

  fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  while (!fTimedOut && CARRIER && cbRemaining)
  {
    cbToSend = ComOutSpace (hcModem);
    if ((--cbToSend) > 0)
    {
      if (cbToSend > cbRemaining)
        cbToSend = cbRemaining;
      ComWriteEx (hcModem, lpBuf, cbToSend);
      lpBuf += cbToSend;
      cbRemaining -= cbToSend;
    }
    else
      time_release ();

    fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  }

  return (cbBuf - cbRemaining);
}

USHORT
ComTXRemain (void)              /* MB 93-12-03 */
{
  COMSTAT cst;
  DWORD errors;

  if (NoModem)
    return (0);

  ClearCommError (hcModem, &errors, &cst);
  return (cst.cbOutQue);
}

#endif /* USE_NTCOMM */

/* $Id: asyn_w32.c,v 1.3 1999/06/01 18:50:26 hjk Exp $ */
