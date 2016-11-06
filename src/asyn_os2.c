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
 * Filename    : $Source: E:/cvs/btxe/src/asyn_os2.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:07 $
 * State       : $State: Exp $
 * Orig. Author: Peter Fitzsimmons
 *
 * Description : BinkleyTerm OS/2 Async Comm I/O Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/*
 * code snippet from Harald Pollack, 2:310/14.59
 * intializes com port to use fifo, CTS/RTS, 8n1
 *  
 * APIRET APIENTRY comport_init(BOOL fifo)
 * {
 *   APIRET      rc;
 *   LINECONTROL LineCntrl;
 * 
 *   if (fifo)
 *   {
 *     if (dcbCom.fbTimeout & 0x18) // DISABLE, ENABLE or AUTO
 *     {
 *       status_line (">PortInit Enable Fifo Rx=8 Tx=16");
 *       dcbCom.fbTimeout &= ~0x18; // DISABLE
 *       dcbCom.fbTimeout |= 0x10; // ENABLE
 *       dcbCom.fbTimeout &= ~0x60; // Rx=1
 *       dcbCom.fbTimeout |= 0x40; // Rx=8
 *       dcbCom.fbTimeout |= 0x80; // Tx=16
 *     }
 *     else
 *       status_line (">No Fifo detected!");
 *   }
 * 
 *   status_line (">PortInit CTS/RTS-Handshake DTR-Control XON/XOFF disabled");
 *   dcbCom.fbCtlHndShake = MODE_CTS_HANDSHAKE | MODE_DTR_CONTROL;
 *   dcbCom.fbFlowReplace = MODE_RTS_HANDSHAKE;
 *   rc = DevIOCtl(NULL,0,&dcbCom,sizeof(dcbCom),ASYNC_SETDCBINFO,IOCTL_ASYNC,hTTY);
 *   if (rc) return rc;
 * 
 *   status_line (">PortInit DataBits 8n1");
 *   LineCntrl.bDataBits  = 8;     // 8 Databits
 *   LineCntrl.bParity    = 0;     // no Parity
 *   LineCntrl.bStopBits  = 0;     // 1 Stopbit
 *   rc = DevIOCtl(NULL,0,&LineCntrl,sizeof(LineCntrl),ASYNC_SETLINECTRL,IOCTL_ASYNC,hTTY);
 * 
 *   rc = raise_dtr();
 *   if (rc) return rc;
 * 
 *   return(0);
 * }
 */

#ifndef NEED_OS2COMMS

/* These must be in "includes.h" for C7 precompiled headers to work (as if
   we'll ever get any MS support for OS/2-targetted C7!!!)

   #define INCL_DOS
   #define INCL_DOSERRORS
   #define INCL_DOSDEVIOCTL
   #include "includes.h"   (And this I put up top to avoid confusion) */

/* This module by Peter Fitzsimmons */
/* Modified 6/13/92 for exit (3) WRA */

char ascii_comport[] = "COM";

HCOMM hcModem = 0;              /* comm.dll handle */

#define RBSIZE  8200            /* MB */
#define TBSIZE  8200            /* MB */

USHORT rBufsize = RBSIZE;
USHORT tBufsize = TBSIZE;

/* CFS 961120 - an attempt to make BT-XE work without CFOS's -kx */
/* (SIO compatibility). */

void
com_DTR_on (void)
{
  MODEMSTATUS ModemStatus;
  USHORT usComError;
  DCBINFO sDCB;
  USHORT rc;

  if (NoModem)
    return;

  if (SIOMode)
  {
    ModemStatus.fbModemOn = DTR_ON;
    ModemStatus.fbModemOff = 0;
    DosDevIOCtl (ComGetFH (hcModem), IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
                 (PVOID) & ModemStatus, sizeof (MODEMSTATUS), NULL,
                 (PVOID) & usComError, sizeof (USHORT), NULL);
  }
  else
  {
    if (!(rc = ComGetDCB (hcModem, &sDCB)))
    {
      sDCB.fbCtlHndShake |= MODE_DTR_CONTROL;  /* raise DTR */
      ComSetDCB (hcModem, &sDCB);
    }
    else
    {
      status_line ("!SYS%04hu: ComGetDCB()", rc);
      exit (3);
    }
  }
}

void
com_DTR_off (void)
{
  MODEMSTATUS ModemStatus;
  USHORT usComError;
  DCBINFO sDCB;
  USHORT rc;

  if (NoModem)
    return;

  if (SIOMode)
  {
    ModemStatus.fbModemOn = 0;
    ModemStatus.fbModemOff = DTR_OFF;
    DosDevIOCtl (ComGetFH (hcModem), IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
                 (PVOID) & ModemStatus, sizeof (MODEMSTATUS), NULL,
                 (PVOID) & usComError, sizeof (USHORT), NULL);
  }
  else
  {
    if (!(rc = ComGetDCB (hcModem, &sDCB)))
    {
      sDCB.fbCtlHndShake &= ~MODE_DTR_CONTROL;  /* lower DTR */
      ComSetDCB (hcModem, &sDCB);
      com_kick ();
    }
    else
    {
      status_line ("!SYS%04hu: ComGetDCB()", rc);
      exit (3);
    }
  }
}

void
com_XON_disable (void)
{
  DCBINFO sDCB;
  USHORT rc;

  if (NoModem)
    return;

  if (!(rc = ComGetDCB (hcModem, &sDCB)))
  {
    /* disable auto Xmit and recv flow control */

    sDCB.fbFlowReplace &= ~(MODE_AUTO_TRANSMIT | MODE_AUTO_RECEIVE);
    ComSetDCB (hcModem, &sDCB);
    com_kick ();
  }
  else
  {
    status_line ("!SYS%04hu: ComGetDCB()", rc);
    exit (3);
  }
}

void
com_XON_enable (void)
{
  DCBINFO sDCB;
  USHORT rc;

  if (NoModem)
    return;

  if (!(rc = ComGetDCB (hcModem, &sDCB)))
  {
    /* enable auto Xmit and recv flow control */

    sDCB.fbFlowReplace |= MODE_AUTO_TRANSMIT;  /*PLF Wed  04-04-1990  02:35:41 */
    ComSetDCB (hcModem, &sDCB);
  }
  else
  {
    status_line ("!SYS%04hu: ComGetDCB()", rc);
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
  ULONG cmd;
  HFILE hf;
  PVOID ParmList;
  ULONG ParmLengthMax;
  ULONG ParmLengthInOut;
  UCHAR DataArea[2];
  ULONG DataLengthMax;
  ULONG DataLengthInOut;
  APIRET rc;

  if (NoModem)
    return;

  ParmList = 0;
  ParmLengthInOut = 0;
  ParmLengthMax = 0;

  DataLengthInOut = 0;
  DataLengthMax = 2;

  cmd = (on) ? ASYNC_SETBREAKON : ASYNC_SETBREAKOFF;
  hf = ComGetFH (hcModem);
  if (hf)
  {
    /* rc = DosDevIOCtl(&comerr, 0L, cmd, IOCTL_ASYNC, hf); */
    rc = DosDevIOCtl (hf, IOCTL_ASYNC, cmd, ParmList, ParmLengthMax,
                      &ParmLengthInOut, DataArea, DataLengthMax,
                      &DataLengthInOut);
    if (rc)
      status_line ("!SYS%04ld: ASYNC_SETBREAK", rc);
  }
}

void
MDM_ENABLE (unsigned long rate)
{
  char _parity;
  short databits;
  short stopbits;

  if (NoModem)
    return;

  if (!hcModem)
    return;

  if (lock_baud && rate != max_baud.rate_value)
  {
    status_line (">Pretending baud is %lu (locked at %lu)", rate, max_baud.rate_value);
    rate = max_baud.rate_value;
  }

  databits = 7 + (comm_bits == BITS_8);
  stopbits = 1 + (stop_bits == STOP_2);

  switch (parity)
  {
  case NO_PARITY:
    _parity = 'N';
    break;
  case ODD_PARITY:
    _parity = 'O';
    break;
  case EVEN_PARITY:
    _parity = 'E';
    break;
  default:
    _parity = 'N';
    status_line ("!Invalid parity");
  }

  ComSetBaudRate (hcModem, (long) rate, _parity, (USHORT) databits, (USHORT) stopbits);
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
  DCBINFO dcb;
  USHORT rc;
  char *On = "On";
  char *Off = "Off";
  char *dtr;
  char *rts;
  char *buffer;
  short Rx = 0, Tx;

  if (NoModem)
    return;

  if (!(rc = ComGetDCB (hcModem, &dcb)))
  {
    status_line (">Modem: TO=%s,XON(Rx)=%s,XON(Tx)=%s",
                 (dcb.fbTimeout & MODE_NO_WRITE_TIMEOUT) ? On : Off,
                 (dcb.fbFlowReplace & MODE_AUTO_RECEIVE) ? On : Off,
                 (dcb.fbFlowReplace & MODE_AUTO_TRANSMIT) ? On : Off);
    status_line (">Modem: IDSR=%s,ODSR=%s,OCTS=%s",
                 (dcb.fbCtlHndShake & MODE_DSR_SENSITIVITY) ? On : Off,
                 (dcb.fbCtlHndShake & MODE_DSR_HANDSHAKE) ? On : Off,
                 (dcb.fbCtlHndShake & MODE_CTS_HANDSHAKE) ? On : Off);
    switch (dcb.fbCtlHndShake & (MODE_DTR_CONTROL | MODE_DTR_HANDSHAKE))
    {
    case 0:
      dtr = Off;
      break;
    case MODE_DTR_CONTROL:
      dtr = On;
      break;
    case MODE_DTR_HANDSHAKE:
      dtr = "IHS";
      break;                    /* input handshaking */
    default:
      dtr = "??";
      break;
    }

    switch (dcb.fbFlowReplace & (MODE_RTS_CONTROL | MODE_RTS_HANDSHAKE | MODE_TRANSMIT_TOGGLE))
    {
    case 0:
      rts = Off;
      break;
    case MODE_RTS_CONTROL:
      rts = On;
      break;
    case MODE_RTS_HANDSHAKE:
      rts = "IHS";
      break;
    case MODE_TRANSMIT_TOGGLE:
      rts = "TOG";
      break;
    default:
      rts = "??";
      break;
    }

    switch (dcb.fbTimeout & 0x18)
    {
    case 0x08:
      buffer = Off;
      break;
    case 0x10:
      buffer = On;
      break;
    case 0x18:
      buffer = "Auto";
      break;
    default:
      buffer = "??";
      break;
    }

    switch (dcb.fbTimeout & 0x60)
    {
    case 0:
      Rx = 1;
      break;
    case 0x20:
      Rx = 4;
      break;
    case 0x40:
      Rx = 8;
      break;
    case 0x60:
      Rx = 14;
      break;
    }

    Tx = (dcb.fbTimeout & 0x80) ? 16 : 1;
    status_line (">Modem: DTR=%s,RTS=%s,BUFFER=%s (Rx=%hd, Tx=%hd)",
                 dtr,
                 rts,
                 buffer,
                 Rx, Tx);
    status_line (">Modem: Buffers R: %hu T: %hu", rBufsize, tBufsize);  /* MB 94-01-02 */
  }
  else
  {
    status_line ("!SYS%04hu: ComGetDCB()", rc);
    exit (3);
  }
}

unsigned
Com_init (int port)
{
  USHORT rc;
  char *s;
  char dev[256];

  if (NoModem)
    return (0);


  if (port_device == NULL)
    port_device = ascii_comport;

  sprintf (dev, "%s%d", port_device, port + 1);

  printf ("Async device %s\r\n", dev);

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

  rc = ComOpen ((PSZ) dev, &hcModem, rBufsize, tBufsize);
  if (rc)
  {
    status_line ("!SYS%04hu:  ComOpen(%s)", rc, dev);
    return (rc);
  }
  else
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
      ret = 0;
  }
  else
    ComResume (hcModem);

  return ret;
}

/* force transmitter to go */
void
com_kick (void)
{
  HFILE hf;
  PVOID ParmList;
  ULONG ParmLengthMax;
  ULONG ParmLengthInOut;
  PVOID DataArea;
  ULONG DataLengthMax;
  ULONG DataLengthInOut;
  APIRET rc;

  if (NoModem)
    return;

  ParmList = 0;
  ParmLengthInOut = 0;
  ParmLengthMax = 0;

  DataArea = 0;
  DataLengthInOut = 0;
  DataLengthMax = 0;

  if (hcModem)
  {
    hf = ComGetFH (hcModem);
    if (hf)
    {
      rc = DosDevIOCtl (hf, IOCTL_ASYNC, ASYNC_STARTTRANSMIT, ParmList, ParmLengthMax,
                        &ParmLengthInOut, DataArea, DataLengthMax,
                        &DataLengthInOut);
      if (rc && rc != 31)
      {
        status_line ("!SYS%04ld: ASYNC_STARTTRANSMIT", rc);
        /* exit (3); */
      }
    }
  }
}

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

#else // NEED_OS2COMMS

#define __BTPE_DEBUG
#define __IOCTL_DEBUG
#define __LOG_TO_FILE

#include "async.h"

/* Private data */
HFILE hfComHandle = NULLHANDLE; /* handle from DosOpen() */
char ascii_comport[] = "COM";

/* transmitter stuff */
#define TSIZE 8192
static unsigned char *tBuf = NULL;
static unsigned char *zTxBuf = NULL;
static int zpos = 0;
static int tBufsize = 0;
static ULONG tpos = 0;
static ULONG tlen = 0;
static BOOL32 thread = FALSE;
HEV buffer_empty, buffer_full;

/* receiver stuff */
#define RSIZE 32768
static unsigned char *rbuf = NULL;
static ULONG rpos = 0;
static int rbufsize = 0;
static ULONG Rbytes = 0;

/* prototypes of local functions */

static APIRET IOCTL (HFILE, ULONG, ULONG, PVOID, ULONG, PULONG, PVOID,
                     ULONG, PULONG, PCHAR);
static int com_getDCB (DCBINFO * dcb);
static int com_setDCB (DCBINFO * dcb);
static void ShowMdmSettings (void);
static void setup_port (void);
static void HydraWrite (PVOID);

#ifdef  BTPE_DEBUG
#define debug_msg(m,c)  status_line(">" m, c)
#else
#define debug_msg(m,c)
#endif

#ifdef LOG_TO_FILE
static FILE *every = NULL;
static char *dbgfnm;
static void logAll (int ch, int type);
static int peeking = 0;

#endif

/* com_init() : Intialize communications port. Baud rate is preserved.
 *            int port  : Hardware port (0 based) to init
 *    -OR-  char *unc   : Full UNC \\networkId\modemId to init.
 *
 * if unc==NULL, port is used. if unc != NULL, unc is used
 */
int
com_init (int port, char *unc)
{
  static char did_it = 0;
  char str[30];
  ULONG ActionTaken, share_mode;
  APIRET ulrc;

  if (!did_it)
  {
    printf ("\033[1;33mOS/2 FOSSIL emulator for BinkleyTerm, Peter Fitzsimmons  (1:250/628)\033[0m\r\n");
    printf ("\033[1;33mSupport for Hydra added and bugfixes, Steffen Motzer     (2:2471/1071.3)\033[0m\r\n");
    did_it = 1;
  }

  if (port_device == NULL)
    port_device = ascii_comport;

  sprintf (str, "%s%d", port_device, port + 1);

  if (!unc)
    unc = str;

  share_mode = share_port ? OPEN_SHARE_DENYNONE : OPEN_SHARE_DENYREADWRITE;
#ifdef BTPE_DEBUG
  ulrc = DosOpen ((PSZ) unc, &hfComHandle, &ActionTaken, 0L,
                  FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_FLAGS_WRITE_THROUGH | share_mode |
                  OPEN_ACCESS_READWRITE, 0L);
#else
  ulrc = DosOpen ((PSZ) unc, &hfComHandle, &ActionTaken, 0L,
                  FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_FLAGS_WRITE_THROUGH | share_mode |
                  OPEN_ACCESS_READWRITE | OPEN_FLAGS_FAIL_ON_ERROR, 0L);
#endif
  if (ulrc != NO_ERROR)
  {
    hfComHandle = NULLHANDLE;
    status_line ("!com_init() : DosOpen() error 0x%x on '%s'\n", ulrc, unc);
    return ulrc;
  }

  if (rbuf == NULL)
  {
    char *s = getenv ("RBUF");

    if (s)
      rbufsize = max (H_MAXBLKLEN * 3, atoi (s));
    else
      rbufsize = RSIZE;
    rbuf = malloc (rbufsize + 1);
  }
  if (zTxBuf == NULL)
  {
    char *s = getenv ("TBUF");

    if (s)
      tBufsize = min (TSIZE, atoi (s));
    else
      tBufsize = TSIZE;
    zTxBuf = malloc (tBufsize + 1);
  }
  setup_port ();
  ShowMdmSettings ();
  return (NO_ERROR);
}

void
com_DTR_on (void)
{
  DCBINFO sDCB;
  MODEMSTATUS ModemStatus;
  ULONG Data;

  if (SIOMode)
  {
    ModemStatus.fbModemOn = DTR_ON;
    ModemStatus.fbModemOff = 0;
    IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
           (PVOID) & ModemStatus, sizeof (MODEMSTATUS), NULL,
           (PVOID) & Data, sizeof (ULONG), NULL, "com_DTR_on()");
  }
  else
  {
    com_getDCB (&sDCB);
    sDCB.fbCtlHndShake |= MODE_DTR_CONTROL;  /* raise DTR */
    com_setDCB (&sDCB);
  }
  debug_msg ("DTR %s", "on");
}

void
com_DTR_off (void)
{
  DCBINFO sDCB;
  MODEMSTATUS ModemStatus;
  ULONG Data;

  if (SIOMode)
  {
    ModemStatus.fbModemOn = 0;
    ModemStatus.fbModemOff = DTR_OFF;
    IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
           (PVOID) & ModemStatus, sizeof (MODEMSTATUS), NULL,
           (PVOID) & Data, sizeof (ULONG), NULL, "com_DTR_off()");
  }
  else
  {
    com_getDCB (&sDCB);
    sDCB.fbCtlHndShake &= ~MODE_DTR_CONTROL;  /* lower DTR */
    com_setDCB (&sDCB);
  }

  debug_msg ("DTR %s", "off");

#ifdef LOG_TO_FILE
  if (every != NULL)
  {
    fflush (every);
  }
#endif
  zpos = 0;                     /* "remove" the characters from the buffer */
}

/* close communications channel. Baud rate is preserved. */
int
com_fini (void)
{
  int stat;

  if (hfComHandle != NULLHANDLE)
  {
    if (thread)
    {
      thread = FALSE;
      DosPostEventSem (buffer_full);
      DosSleep (500);
    }
    com_wait ();
    com_clear_in ();
    com_clear_out ();
    stat = DosClose (hfComHandle);
    hfComHandle = NULLHANDLE;
#ifdef LOG_TO_FILE
    logAll (0, '-');
#endif
    if (stat)
    {
      debug_msg ("Async: DosClose() error 0x%x", stat);
      return (FALSE);
    }
  }
  return (TRUE);
}

long
com_cur_baud (void)
{
  EXTBAUDRATE extrate;

  if (hfComHandle == NULLHANDLE)
    return (0);

  IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_EXTGETBAUDRATE,
         NULL, 0, NULL, &extrate, sizeof extrate, NULL, "com_cur_baud()");
  return ((long) extrate.cur_bps);
}

/* com_set_baud() :

 *  rate = 110..19200
 *  parity = N, E, O, M, S (none,even, odd, mark, space)
 *  databits = 5..8
 *  stopbits = 1..2
 *
 */
int
com_set_baud (unsigned long ratei, char parity, int databits, int stopbits)
{
  APIRET stat;
  ULONG ratel = ratei;
  struct _LINECONTROL
  {
    BYTE bDataBits;
    BYTE bParity;
    BYTE bStopBits;
    BYTE fbTransBreak;
  }
  lc;

#ifdef BTPE_DEBUG
  static long cnt = 0;

  cnt++;
#endif

  if (hfComHandle == NULLHANDLE)
    return (0);

  stat = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_EXTSETBAUDRATE,
                &ratel, sizeof ratel, NULL, NULL, 0, NULL, "com_set_baud()");
  if (stat)
  {
#ifdef BTPE_DEBUG
    if (cnt <= 2)
      printf (">status from setbaudrate was %d\n", stat);
    else
      status_line (" status from setbaudrate was %d", stat);
#endif
    return (FALSE);
  }

  lc.bDataBits = (BYTE) databits;

  switch (stopbits)
  {
  case 1:
    lc.bStopBits = 0;
    break;
  case 2:
    lc.bStopBits = 2;
    break;
  default:
    if (databits == 5)
      lc.bStopBits = 1;
  }

  lc.fbTransBreak = 0;

  switch (toupper (parity))
  {
  case 'N':
    lc.bParity = 0;
    break;
  case 'O':
    lc.bParity = 1;
    break;
  case 'E':
    lc.bParity = 2;
    break;
  case 'M':
    lc.bParity = 3;
    break;
  case 'S':
    lc.bParity = 4;
    break;
  default:
    debug_msg ("Bad parity '%c'", parity);
    return (FALSE);
  }

  stat = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_SETLINECTRL,
                &lc, sizeof lc, NULL, NULL, 0, NULL, "com_set_baud()");

  return (!stat);
}

static int
com_getDCB (DCBINFO * dcb)
{
  APIRET stat;

  if (hfComHandle == NULLHANDLE)
    return (0);

  stat = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETDCBINFO,
                NULL, 0, NULL, dcb, sizeof *dcb, NULL, "com_getDCB()");

  return (!stat);
}

static int
com_setDCB (DCBINFO * dcb)
{
  APIRET stat;

  if (hfComHandle == NULLHANDLE)
    return (0);

  stat = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_SETDCBINFO,
                dcb, sizeof *dcb, NULL, NULL, 0, NULL, "com_setDCB()");

  return (!stat);
}

void
com_XON_disable (void)
{
  DCBINFO sDCB;

  if (com_getDCB (&sDCB))
  {
    /* disable auto Xmit and recv flow control */
    sDCB.fbFlowReplace &= ~(MODE_AUTO_TRANSMIT | MODE_AUTO_RECEIVE);
    com_setDCB (&sDCB);
  }
  com_kick ();

}

void
com_XON_enable (void)
{
  DCBINFO sDCB;

  if (com_getDCB (&sDCB))
  {
    /* enable auto Xmit and recv flow control */
    sDCB.fbFlowReplace |= (MODE_AUTO_TRANSMIT | MODE_AUTO_RECEIVE);
    com_setDCB (&sDCB);
  }
}

/* nuke receive buffer */
void
com_clear_in (void)
{
  char FlushParm = 0;           /* param to flush IOCTL function */

  if (hfComHandle == NULLHANDLE)
    return;

  Rbytes = rpos = 0;
  IOCTL (hfComHandle, IOCTL_GENERAL, DEV_FLUSHINPUT,
         (PVOID) & FlushParm, sizeof FlushParm, NULL,
         NULL, 0, NULL, "com_clear_in()");
}

/* com_getbuf() : return negative value if error */
int
com_getbuf (void)
{
  APIRET stat = 0;
  ULONG bytes;
  RXQUEUE q;

  if (hfComHandle == NULLHANDLE)
    return (-1);

  if (rpos > 1024)
  {
    memmove (rbuf, rbuf + rpos, Rbytes - rpos);  /* SM 961127 */
    Rbytes -= rpos;
    rpos = 0;
  }

  if (!IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
              NULL, 0, NULL, &q, sizeof q, NULL, "com_getbuf()"))
  {
    if (q.cch > 0)
    {
      stat = DosRead (hfComHandle, rbuf + Rbytes, (USHORT) min (rbufsize -
                                                                Rbytes,
                                                            q.cch), &bytes);
    }
    else
      return (-1);
  }
  else
    return (-1);

  if (stat && !bytes && (!(stat == ERROR_MORE_DATA)))
  {
    debug_msg ("DosRead() error 0x%x", stat);
    return (-1);
  }

  Rbytes += bytes;
  return (TRUE);
}

/* com_getchar() : return negative value if error */
int
com_getchar (void)
{
  int ret;

  if (rpos == Rbytes)
    com_getbuf ();

  if (rpos < Rbytes)
    ret = rbuf[rpos++];
  else
    ret = -1;

#ifdef LOG_TO_FILE
  if ((ret != -1) && !peeking)
  {
    logAll (ret, 'R');
  }
  peeking = 0;
#endif
  return (ret);
}

/*non destructive read ahead; no wait */
int
com_peek (void)
{

#ifdef LOG_TO_FILE
  peeking = 1;
#endif

  if (rpos == Rbytes)
    com_getbuf ();
  return (rpos == Rbytes) ? -1 : rbuf[rpos];
}

/* if there are enough characters in RXQueue , return TRUE */
bool
com_in_check (void)
{
  RXQUEUE q;
  APIRET stat;

  if (hfComHandle == NULLHANDLE)
    return (FALSE);

  stat = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
                NULL, 0, NULL, &q, sizeof q, NULL, "com_in_check()");
  if (stat)
    return (FALSE);
  return (q.cch > (q.cb / 2)) ? TRUE : FALSE;
}

/* return number of chars in input buffer */
int
com_char_avail (void)
{
  RXQUEUE q;

  if (rpos < Rbytes)
    return (Rbytes - rpos);

  if (hfComHandle == NULLHANDLE)
    return (0);

  if (IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
             NULL, 0, NULL, &q, sizeof q, NULL, "com_char_avail()"))
    return (0);

  return (q.cch);
}

/* return non 0 value if carrier detected */
bool
com_online (void)
{
  BYTE msr;
  APIRET rc;

  if (hfComHandle == NULLHANDLE)
    return (FALSE);

  rc = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
              NULL, 0, NULL, &msr, sizeof msr, NULL, "com_online()");

  if (rc)
    return (FALSE);

  return ((msr & DCD_ON) == DCD_ON);
}

/* com_break() : start break if on==TRUE, stop break if on==FALSE */
void
com_break (int on)
{
  int cmd;
  USHORT comerr;

  cmd = (on) ? ASYNC_SETBREAKON : ASYNC_SETBREAKOFF;
  IOCTL (hfComHandle, IOCTL_ASYNC, cmd,
         NULL, 0, NULL, &comerr, sizeof comerr, NULL, "com_break()");
}

/* com_out_empty() : return TRUE if output buffer is empty */
bool
com_out_empty (void)
{
  RXQUEUE q;

  if (hfComHandle == NULLHANDLE)
    return (TRUE);

  /* Service Inbound side if necessary */
  if (com_in_check ())
    com_getbuf ();

  if (zpos != 0)
    UNBUFFER_BYTES ();

  if (IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETOUTQUECOUNT,
             NULL, 0, NULL, &q, sizeof q, NULL, "com_out_empty()"))

    return (TRUE);

  return ((bool) (q.cch == 0));
}

/* nuke transmit buffer */
void
com_clear_out (void)
{
  char FlushParm = 0;           /* param to flush IOCTL function */

  zpos = 0;

  if (hfComHandle == NULLHANDLE)
    return;
  IOCTL (hfComHandle, IOCTL_GENERAL, DEV_FLUSHOUTPUT,
         (PVOID) & FlushParm, sizeof FlushParm, NULL,
         NULL, 0, NULL, "com_clear_out()");
  com_kick ();
}

/* com_putc() : output to com port */
/* This function is very slow..where possile, write to com port in blocks
 * using com_write() instead...especially above 2400 bps
 */
void
com_putc (byte c)
{
  ULONG bytes;
  APIRET stat;
  RXQUEUE q;

  if (hfComHandle == NULLHANDLE)
    return;

#ifdef LOG_TO_FILE
  logAll (c, 'S');
#endif
  if (com_in_check ())
    com_getbuf ();
  do
  {
    if (com_in_check ())
      com_getbuf ();
    stat = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETOUTQUECOUNT,
                  NULL, 0, NULL, &q, sizeof q, NULL, "com_putc()");
    if (stat != NO_ERROR)
      return;

    if (q.cch >= q.cb)
    {
      com_kick ();
      DosSleep (1L);
    }
  }
  while ((q.cch >= q.cb) && com_online ());

  stat = DosWrite (hfComHandle, &c, 1L, &bytes);
  if (stat)
  {
    stat = stat;                /* to prevent a compiler warning in case of no DEBUG */
    debug_msg ("DosWrite() err 0x%x", stat);
  }
}

/* com_write() : buffered block write */
void
com_write (char *buf, unsigned int num, int carcheck)
{
  ULONG bytes;
  APIRET rc;

#ifdef LOG_TO_FILE
  int x;

#endif

  if (hfComHandle == NULLHANDLE)
    return;

#ifdef LOG_TO_FILE
  logAll ((unsigned char) buf[0], 'S');
  if (every != NULL)
  {
    for (x = 1; x < num; x++)
    {
      logAll ((unsigned char) buf[x], 'S');
    }
  }
#endif

  if ((carcheck) && !com_online ())
    return;

  if (com_in_check ())
    com_getbuf ();

  rc = DosWrite (hfComHandle, buf, (ULONG) num, &bytes);
  if (rc)
  {
    rc = rc;                    /* to prevent a compiler warning in case of no DEBUG */
    debug_msg ("DosWrite() err 0x%x", rc);
  }
  rc = rc;
}

/* wait for output buffer to empty */
void
com_wait (void)
{
  while ((!com_out_empty ()) && com_online ())
    DosSleep (1L);
}

/* force transmitter to go */
void
com_kick (void)
{
  if (hfComHandle == NULLHANDLE)
    return;

  IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_STARTTRANSMIT,
         NULL, 0, NULL, NULL, 0, NULL, "com_kick()");
}

extern unsigned int comm_bits;
extern unsigned int parity;
extern unsigned int stop_bits;
extern struct baud_str btypes[];

void
MDM_ENABLE (unsigned long rate)
{
  char _parity;
  int databits;
  int stopbits;

  if (lock_baud && rate != max_baud.rate_value)
  {
    status_line (">Pretending baud is %lu (locked at %lu)", rate, max_baud.rate_value);
    rate = max_baud.rate_value;
  }

  databits = 7 + (comm_bits == BITS_8);
  stopbits = 1 + (stop_bits == STOP_2);

  switch (parity)
  {
  case NO_PARITY:
    _parity = 'N';
    break;
  case ODD_PARITY:
    _parity = 'O';
    break;
  case EVEN_PARITY:
    _parity = 'E';
    break;
  default:
    _parity = 'N';
    status_line ("!Invalid parity");
  }

  com_set_baud (rate, _parity, databits, stopbits);
}

void
MDM_DISABLE (void)
{
  if (hfComHandle == NULLHANDLE)
    return;

  com_clear_out ();
  com_clear_in ();
  com_fini ();
}

/* zsend.c uses BUFFER_BYTE and UNBUFFER_BYTES...good idea. */
void
BUFFER_BYTE (unsigned char ch)
{
  if (zpos == tBufsize)
    UNBUFFER_BYTES ();
  zTxBuf[zpos++] = ch;
}

void
UNBUFFER_BYTES (void)
{
  if (com_online () && zpos)
    (zpos == 1) ? (com_putc (zTxBuf[0])) :
      (com_write ((char *) zTxBuf, zpos, 1));
  zpos = 0;
}

unsigned
Cominit (int port, int failsafe)
{
  failsafe = failsafe;

  if (NoModem)
    return (0x1954);

  if (hfComHandle == NULLHANDLE)
  {
    if (com_init (port, NULL))
      return 0;
  }
  else
    setup_port ();

  return (0x1954);
}

int
com_getc (int t)
{
  long t1;

  if (NoModem)
    return (0);

  if (!CHAR_AVAIL ())
  {
    t1 = timerset ((unsigned int) (t * PER_SECOND));
    while (!CHAR_AVAIL ())
    {
      if (timeup (t1))
      {
        return (EOF);
      }

      /* This should work because we only do TIMED_READ when we have
       * carrier */
      if (!CARRIER)
      {
        return (EOF);
      }
      time_release ();
    }
  }
  return ((unsigned int) (MODEM_IN ()) & 0x00ff);
}

#ifdef LOG_TO_FILE

static void
logAll (int ch, int type)
{
  static int lastType = 0;
  static int lastchar = 0;
  static time_t lastTime = 0;
  time_t tt;
  struct tm *tms;

  if (debugging_log && (every == NULL))
  {
    dbgfnm = getenv ("BINKDBG");
    if (dbgfnm != NULL)
    {
      every = fopen (dbgfnm, append_ascii);
      if (every != NULL)
      {
        setvbuf (every, NULL, _IOFBF, 1000000);
        logAll (0, 'X');        /* REKURSIVE! */
      }
    }
  }

  if (every == NULL)
    return;

  if ((ch != 0x0d) && (ch != 0x0a))
  {
    if ((lastchar == 0x0d) || (lastchar == 0x0a))
    {
      lastType = 0;
    }
  }

  time (&tt);
  if (tt != lastTime)
  {
    tms = localtime (&tt);
    fprintf (every, "\n\nTime: %.2d:%.2d:%.2d\n",
             tms->tm_hour, tms->tm_min, tms->tm_sec);
    lastTime = tt;
  }

  if (type != lastType)
  {
    if (type == 'S')
    {
      fprintf (every, "\nSEND: ");
    }
    else if (type == 'R')
    {
      fprintf (every, "\nRECV: ");
    }
    else if (type == 'X')
    {
      fprintf (every, "\nSTART\n");
    }
    else if (type == '-')
    {
      fprintf (every, "\nFINISHED\n");
    }
    lastType = type;
  }

  if (type != 'X')
  {
    if ((ch > 0x1f) && (ch < 0x255))
    {
      fprintf (every, "%c", (char) ch);
    }
    else
    {
      if ((ch == 0x0d) || (ch == 0x0a))
      {
        fprintf (every, " (CR)");
      }
      else
      {
        fprintf (every, " 0x%02x,", ch);
      }
    }
    lastchar = ch;
  }
  if (type == '-')
  {
    fclose (every);
    every = NULL;
  }

  return;
}

#endif

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
  ULONG ulPostCt = 0;
  APIRET ulrc;
  ULONG bytes, tpos2;

  if (hfComHandle == NULLHANDLE)
    return (0);

  if (cbBuf == 0 || ulTimer == 0)
    return 0;                   /* nothing to do */

#ifdef LOG_TO_FILE
  logAll ((unsigned char) lpBuf[0], 'S');
  if (every != NULL)
  {
    int x;

    for (x = 1; x < cbBuf; x++)
    {
      logAll ((unsigned char) lpBuf[x], 'S');
    }
  }
#endif

  tpos2 = 0;

  if (!thread)
  {
    DosCreateEventSem (NULL, &buffer_empty, 0, FALSE);
    DosCreateEventSem (NULL, &buffer_full, 0, FALSE);
    thread = TRUE;
    ulrc = _beginthread (HydraWrite, NULL, 32768L, NULL);
    if (ulrc == -1)
    {
      status_line ("!could not create thread HydraWrite()");
      thread = FALSE;
      return 0;
    }
  }

  if (cbBuf < 1024 && tpos == 0)  /* small chunks of data should be send */
  {
    /* as fast as possible                 */
    do
    {
      if (com_in_check ())
        com_getbuf ();
      ulrc = DosWrite (hfComHandle, lpBuf + tpos2, cbBuf - tpos2, &bytes);
      if (ulrc != NO_ERROR || bytes == 0)
        DosSleep (1);
      if (ulrc == NO_ERROR)
        tpos2 += bytes;
    }
    while ((ulTimer > unix_time (NULL)) && (tpos2 != cbBuf) && com_online ());
  }
  else
  {
    BOOL32 abort = FALSE;
    BOOL32 timeout = FALSE;

    DosWaitEventSem (buffer_empty, SEM_INDEFINITE_WAIT);
    DosResetEventSem (buffer_empty, &ulPostCt);
    tBuf = lpBuf;
    tlen = cbBuf;
    DosPostEventSem (buffer_full);

    do
    {
      DosQueryEventSem (buffer_empty, &ulPostCt);
      if (com_in_check ())
        com_getbuf ();
      abort = keyabort ();
      timeout = ulTimer <= unix_time (NULL);
      time_release ();
    }
    while (ulPostCt < 1 && com_online () && !abort && !timeout);

    if (!com_online () || abort || timeout)
    {
      thread = FALSE;
      DosPostEventSem (buffer_full);
      DosSleep (500);
      if (abort)
        _ungetch (27);
    }
  }

  return cbBuf;
}

void
HydraWrite (PVOID ignore_me)
{
  ULONG bytes, ulPostCt;
  APIRET ulrc;

  ignore_me = ignore_me;        /* make the compiler happy */
  DosPostEventSem (buffer_empty);

  while (thread)                /* if we can write to the port */
  {
    DosWaitEventSem (buffer_full, SEM_INDEFINITE_WAIT);
    DosResetEventSem (buffer_full, &ulPostCt);

    if (tBuf != NULL)
    {
      while (tpos < tlen && thread)  /* buffer not empty */
      {
        ulrc = DosWrite (hfComHandle, tBuf + tpos, tlen - tpos, &bytes);
        if (ulrc != NO_ERROR || bytes == 0)
          DosSleep (1);
        if (ulrc == NO_ERROR)
          tpos += bytes;
      }
    }

    tpos = 0;
    tlen = 0;
    DosPostEventSem (buffer_empty);
  }

  DosCloseEventSem (buffer_empty);
  DosCloseEventSem (buffer_full);
  thread = FALSE;               /* comTXBlockTimeout can start an other */
  /* incarnation of this thread later     */
  _endthread ();
}

USHORT
ComTXRemain (void)              /* SM 961109 */
{
  return (Rbytes - rpos) < (H_MAXBLKLEN * 2) ? 0 : H_MAXBLKLEN + 1;
}

USHORT
ComOutSpace (HFILE hfComHandle)    /* SM 961109 */
{
  RXQUEUE q;
  APIRET rc;

  if (hfComHandle == NULLHANDLE)
    return (0);

  rc = IOCTL (hfComHandle, IOCTL_ASYNC, ASYNC_GETOUTQUECOUNT,
              NULL, 0, NULL, &q, sizeof q, NULL, "ComOutSpace()");
  if (rc)
    return (0);
  return (q.cb - q.cch);
}

APIRET
IOCTL (HFILE hDevice, ULONG category, ULONG function,
       PVOID pParams, ULONG cbParmLenMax, PULONG pcbParmLen,
       PVOID pData, ULONG cbDataLenMax, PULONG pcbDataLen,
       PCHAR errtext)
{
  APIRET ulrc;
  ULONG parmlen = cbParmLenMax;
  ULONG datalen = cbDataLenMax;

  if (pParams != NULL && pcbParmLen == NULL)
    pcbParmLen = &parmlen;
  if (pData != NULL && pcbDataLen == NULL)
    pcbDataLen = &datalen;

  ulrc = DosDevIOCtl (hDevice, category, function,
                      pParams, cbParmLenMax, pcbParmLen,
                      pData, cbDataLenMax, pcbDataLen);
#ifdef IOCTL_DEBUG
  if (ulrc != NO_ERROR)
  {
    status_line ("!%s", errtext);
    status_line ("!=================================");
    status_line ("!DosDevIOCtl() error    : %u", ulrc);
    status_line ("!DosDevIOCtl() category : %u", category);
    status_line ("!DosDevIOCtl() function : %u", function);
    status_line ("!=================================");
  }
#else
  errtext = errtext;
#endif
  return ulrc;
}

void
ShowMdmSettings (void)
{
  DCBINFO sDCB;
  char *On = "On";
  char *Off = "Off";
  char *dtr;
  char *rts;
  char *buffer;
  short Rx = 0, Tx;

  if (NoModem)
    return;

  if (com_getDCB (&sDCB))
  {
    status_line (">Modem: TO=%s,XON(Rx)=%s,XON(Tx)=%s",
                 (sDCB.fbTimeout & MODE_NO_WRITE_TIMEOUT) ? On : Off,
                 (sDCB.fbFlowReplace & MODE_AUTO_RECEIVE) ? On : Off,
                 (sDCB.fbFlowReplace & MODE_AUTO_TRANSMIT) ? On : Off);
    status_line (">Modem: IDSR=%s,ODSR=%s,OCTS=%s",
                 (sDCB.fbCtlHndShake & MODE_DSR_SENSITIVITY) ? On : Off,
                 (sDCB.fbCtlHndShake & MODE_DSR_HANDSHAKE) ? On : Off,
                 (sDCB.fbCtlHndShake & MODE_CTS_HANDSHAKE) ? On : Off);

    switch (sDCB.fbCtlHndShake & (MODE_DTR_CONTROL | MODE_DTR_HANDSHAKE))
    {
    case 0:
      dtr = Off;
      break;
    case MODE_DTR_CONTROL:
      dtr = On;
      break;
    case MODE_DTR_HANDSHAKE:
      dtr = "IHS";
      break;                    /* input handshaking */
    default:
      dtr = "??";
      break;
    }

    switch (sDCB.fbFlowReplace & (MODE_RTS_CONTROL | MODE_RTS_HANDSHAKE | MODE_TRANSMIT_TOGGLE))
    {
    case 0:
      rts = Off;
      break;
    case MODE_RTS_CONTROL:
      rts = On;
      break;
    case MODE_RTS_HANDSHAKE:
      rts = "IHS";
      break;
    case MODE_TRANSMIT_TOGGLE:
      rts = "TOG";
      break;
    default:
      rts = "??";
      break;
    }

    switch (sDCB.fbTimeout & 0x18)
    {
    case 0x08:
      buffer = Off;
      break;
    case 0x10:
      buffer = On;
      break;
    case 0x18:
      buffer = "Auto";
      break;
    default:
      buffer = "??";
      break;
    }

    switch (sDCB.fbTimeout & 0x60)
    {
    case 0:
      Rx = 1;
      break;
    case 0x20:
      Rx = 4;
      break;
    case 0x40:
      Rx = 8;
      break;
    case 0x60:
      Rx = 14;
      break;
    }

    Tx = (sDCB.fbTimeout & 0x80) ? 16 : 1;
    status_line (">Modem: DTR=%s,RTS=%s,BUFFER=%s (Rx=%hd, Tx=%hd)",
                 dtr,
                 rts,
                 buffer,
                 Rx, Tx);
    status_line (">Modem: Buffers R: %hu T: %hu", rbufsize, tBufsize);
  }
}

void
setup_port ()
{
  DCBINFO sDCB;

  if (NoModem)
    return;

  com_getDCB (&sDCB);
  sDCB.usWriteTimeout = 1000;
  sDCB.usReadTimeout = 1000;
  sDCB.fbCtlHndShake |= (MODE_DTR_CONTROL | MODE_CTS_HANDSHAKE);
  sDCB.fbCtlHndShake &= ~(MODE_DSR_SENSITIVITY | MODE_DSR_HANDSHAKE | MODE_DTR_HANDSHAKE);
  sDCB.fbFlowReplace |= (MODE_RTS_HANDSHAKE);
  sDCB.fbFlowReplace &= ~(MODE_AUTO_TRANSMIT | MODE_AUTO_RECEIVE | MODE_RTS_CONTROL);
  sDCB.fbTimeout = (MODE_NO_WRITE_TIMEOUT | MODE_READ_TIMEOUT | 0x10 | 0x40
                    | 0x80);
  com_setDCB (&sDCB);
}

# endif                         /* NEED_OS2COMMS */

/* $Id: asyn_os2.c,v 1.2 1999/03/22 03:47:07 mr Exp $ */
