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
 * Filename    : $Source: E:/cvs/btxe/include/com_w32.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/06 17:11:47 $
 * State       : $State: Exp $
 * Orig. Author: Peter Fitzsimmons
 *
 * Description : Win32  Communications definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Baud rate masks */

#ifndef USE_WINFOSSIL           /* if we don't want WinFossil, we use NTCOMM.DLL */

#include "third\ntcomm.h"

extern HCOMM hcModem;

typedef unsigned char bool;

extern void com_XON_enable (void);
extern void com_XON_disable (void);
extern void com_DTR_on (void);
extern void com_DTR_off (void);
extern void com_break (int on);
extern int com_getc (int);
extern unsigned Cominit (int, int);
extern void MDM_DISABLE (void);
extern void MDM_ENABLE (unsigned long);

extern short KBHit (void);
extern short GetKBKey (void);
extern USHORT ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer);
extern USHORT ComTXRemain (void);

/* translate binkley fossil stuff to my async package */

#define CARRIER             ComIsOnline(hcModem)
#define CHAR_AVAIL()        ComInCount(hcModem)
#define OUT_EMPTY()         (ComOutCount(hcModem)==0)
#define OUT_FULL()          (ComOutSpace(hcModem)==0)
#define LOWER_DTR()         com_DTR_off()
#define RAISE_DTR()         com_DTR_on()
#define CLEAR_OUTBOUND()    ComPurge(hcModem, COMM_PURGE_TX)
#define CLEAR_INBOUND()     ComPurge(hcModem, COMM_PURGE_RX)
#define KEYPRESS()          KBHit()
#define READKB()            GetKBKey()
#define FOSSIL_CHAR()       GetKBKey()
#define XON_ENABLE()        com_XON_enable()
#define IN_XON_ENABLE()
#define XON_DISABLE()       com_XON_disable()
#define _BRK_DISABLE()
#define FOSSIL_WATCHDOG(x)
#define SENDBYTE(c)         ComPutc(hcModem, c)
#define BUFFER_BYTE(c)      ComPutc(hcModem, c)
#define UNBUFFER_BYTES()    ComTxWait(hcModem, 1L)  /* yield cpu for a moment */
#define MODEM_IN()          ComGetc(hcModem)
#define WRITE_ANSI(c)       putch (c)
#define PEEKBYTE()          ComPeek(hcModem)
#define do_break(on)        com_break(on)
#define SENDCHARS(buf, size, carcheck)  ComWrite(hcModem, buf, size)
#define hfComHandle         ComGetHandle(hcModem)

/* some useful bits */

/* (MSR) Received line signal detect sometimes called Carrier Detect */
#define RLSD    0x80
/* (LSR) Transmitter holding register empty (ready for another byte) */
#define THRE    0x20
/* (LSR) Data ready indicator */
#define DR      0x1
/* (LSR) Overrun error! We are not reading bytes fast enuf */
#define OE      0x2

void ShowMdmSettings (void);

#else /* use normal comm stuff (winfossil) */

typedef unsigned char bool;

#if !defined( BAUD_300 )
/* Baud rate masks */

#define BAUD_300        300
#define BAUD_1200       1200
#define BAUD_2400       2400
#define BAUD_4800       4800
#define BAUD_9600       9600
#define BAUD_19200      19200
#define BAUD_38400      38400
#define BAUD_57600      57600
#define BAUD_115200     115200

#endif

#if !defined( _WINDOWS_ )

typedef unsigned long HANDLE;

typedef struct _OVERLAPPED
{
  DWORD Internal;
  DWORD InternalHigh;
  DWORD Offset;
  DWORD OffsetHigh;
  HANDLE hEvent;
}
OVERLAPPED, *LPOVERLAPPED;

#endif

typedef struct tagFOSSIL_PORTINFO
{
  ULONG ulPortId;
  HANDLE hDevice;
  OVERLAPPED ov;

}
FOSSIL_PORTINFO, *PFOSSIL_PORTINFO;

extern HANDLE hcModem;

extern short KBHit (void);
extern short GetKBKey (void);

unsigned short Cominit (int, int);
extern short ComSetParms (HANDLE, ULONG);
extern void ComDeInit (HANDLE);
extern short ComCarrier (HANDLE);
extern short ComInCount (HANDLE);
extern short ComOutCount (HANDLE);
extern short ComOutSpace (HANDLE);
extern void ComDTROff (HANDLE);
extern void ComDTROn (HANDLE);
extern void ComTXPurge (HANDLE);
extern void ComRXPurge (HANDLE);
extern void ComXONEnable (HANDLE);
extern void ComXONDisable (HANDLE);
extern short ComPutc (HANDLE, byte);
extern short ComBufferByte (HANDLE, byte);
extern void ComTxWait (HANDLE, ULONG);
extern int ComRxWait (HANDLE, ULONG);
extern unsigned short ComGetc (HANDLE);
extern short ComPeek (HANDLE);
extern void ComBreak (HANDLE, int);
extern void ComWrite (HANDLE, void *, USHORT);
extern int ComGetFH (HANDLE);
extern int ComPause (HANDLE);
extern int ComResume (HANDLE);
extern int com_getc (int);
extern USHORT ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer);
extern USHORT ComTXRemain (void);

#define com_DTR_off ComDTROff
#define com_DTR_on  ComDTROn

/* translate binkley fossil comm stuff to ASYN_W32.C calls */

/*-----------------------------------------------*/
/* Service 0: SET BAUD(etc)                      */
/*-----------------------------------------------*/

#define MDM_ENABLE(b)       (ComSetParms (hcModem, b))

/*-----------------------------------------------*/
/* Service 1: SEND CHAR (wait)                   */
/*-----------------------------------------------*/

#define SENDBYTE(c)         (ComPutc (hcModem, c))

/*-----------------------------------------------*/
/* Service 2: GET CHAR (wait)                    */
/*-----------------------------------------------*/

#define MODEM_IN()          (ComGetc (hcModem))

/*-----------------------------------------------*/
/* Service 3: GET STATUS                         */
/*-----------------------------------------------*/

#define CARRIER             (ComCarrier (hcModem))
#define CHAR_AVAIL()        (ComInCount (hcModem))
#define OUT_EMPTY()         (ComOutCount (hcModem) == 0)
#define OUT_FULL()          (ComOutSpace (hcModem) == 0)

/*-----------------------------------------------*/
/* Service 4: INIT/INSTALL                       */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 5: UNINSTALL                          */
/*-----------------------------------------------*/

#define MDM_DISABLE()       (ComDeInit (hcModem))

/*-----------------------------------------------*/
/* Service 6: SET DTR                            */
/*-----------------------------------------------*/

#define LOWER_DTR()         (ComDTROff (hcModem))
#define RAISE_DTR()         (ComDTROn (hcModem))

/*-----------------------------------------------*/
/* Service 7: GET TIMER TICK PARMS               */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 8: FLUSH OUTBOUND RING-BUFFER         */
/*-----------------------------------------------*/

#define UNBUFFER_BYTES()    (ComTxWait (hcModem, 1L))

/*-----------------------------------------------*/
/* Service 9: NUKE OUTBOUND RING-BUFFER          */
/*-----------------------------------------------*/

#define CLEAR_OUTBOUND()    (ComTXPurge (hcModem))

/*-----------------------------------------------*/
/* Service a: NUKE INBOUND RING-BUFFER           */
/*-----------------------------------------------*/

#define CLEAR_INBOUND()     (ComRXPurge (hcModem))

/*-----------------------------------------------*/
/* Service b: SEND CHAR (no wait)                */
/*-----------------------------------------------*/

#define BUFFER_BYTE(c)      (ComBufferByte (hcModem, c))

/*-----------------------------------------------*/
/* Service c: GET CHAR (nondestructive, no wait) */
/*-----------------------------------------------*/

#define PEEKBYTE()          (ComPeek (hcModem))

/*-----------------------------------------------*/
/* Service d: GET KEYBOARD STATUS                */
/*-----------------------------------------------*/

#define KEYPRESS()          (KBHit ())

/*-----------------------------------------------*/
/* Service e: GET KEYBOARD CHARACTER (wait)      */
/*-----------------------------------------------*/

#define READKB()            (GetKBKey ())
#define FOSSIL_CHAR()       (GetKBKey ())

/*-----------------------------------------------*/
/* Service f: SET/GET FLOW CONTROL STATUS        */
/*-----------------------------------------------*/

#define XON_ENABLE()        (ComXONEnable (hcModem))
#define IN_XON_ENABLE()
#define XON_DISABLE()       (ComXONDisable (hcModem))
#define IN_XON_DISABLE()

/*-----------------------------------------------*/
/* Service 10: SET/GET CTL-BREAK CONTROLS        */
/*             Note that the "break" here refers */
/*             to ^C and ^K rather than the      */
/*             tradition modem BREAK.            */
/*-----------------------------------------------*/

#define _BRK_ENABLE()
#define _BRK_DISABLE()

/*-----------------------------------------------*/
/* Service 11: SET LOCAL VIDEO CURSOR POSITION   */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 12: GET LOCAL VIDEO CURSOR POSITION   */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 13: WRITE LOCAL ANSI CHARACTER        */
/*-----------------------------------------------*/

#define WRITE_ANSI(c)       (putchar (c))

/*-----------------------------------------------*/
/* Service 14: WATCHDOG on/off                   */
/*-----------------------------------------------*/

#define FOSSIL_WATCHDOG(x)

/*-----------------------------------------------*/
/* Service 18: Write buffer, no wait             */
/*-----------------------------------------------*/

#define SENDCHARS(buf, size, carcheck)  (ComWrite (hcModem, buf, (USHORT)size))

/*-----------------------------------------------*/
/* Service 1a: Break on/off                      */
/*-----------------------------------------------*/

#define do_break(on)        (ComBreak (hcModem, on))

#define hfComHandle         (ComGetFH (hcModem))

#endif /* USE_WINFOSSIL */

/* $Id: com_w32.h,v 1.2 1999/03/06 17:11:47 hjk Exp $ */
