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
 * Filename    : $Source: E:/cvs/btxe/include/com_os2.h,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/06 15:58:21 $
 * State       : $State: Exp $
 * Orig. Author: Peter Fitzsimmons
 *
 * Description : OS/2 Communications definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#ifndef NEED_OS2COMMS

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

unsigned Cominit (int, int);
void MDM_DISABLE (void);
void MDM_ENABLE (unsigned long);

typedef unsigned char bool;

extern void com_XON_enable (void);
extern void com_XON_disable (void);
extern void com_DTR_on (void);
extern void com_DTR_off (void);
extern void com_break (int on);
extern int com_getc (int);

extern void MDM_ENABLE (unsigned long rate);
extern void MDM_DISABLE (void);
unsigned Cominit (int port, int failsafe);
extern int get_key (void);
extern USHORT ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer);
extern USHORT ComTXRemain (void);

/* translate binkley fossil stuff to my async package */

#define CARRIER             (ComIsOnline(hcModem))
#define CHAR_AVAIL()        ComInCount(hcModem)
#define OUT_EMPTY()         (ComOutCount(hcModem)==0)
#define OUT_FULL()          (ComOutSpace(hcModem)==0)
#define LOWER_DTR()         com_DTR_off()
#define RAISE_DTR()         com_DTR_on()
#define CLEAR_OUTBOUND()    ComPurge(hcModem, COMM_PURGE_TX)
#define CLEAR_INBOUND()     ComPurge(hcModem, COMM_PURGE_RX)
#define KEYPRESS()          kbhit()
#define READKB()            get_key()
#define FOSSIL_CHAR()       get_key()
#define XON_ENABLE()        com_XON_enable()
#define IN_XON_ENABLE()
#define XON_DISABLE()       com_XON_disable()
#define _BRK_DISABLE()
#define FOSSIL_WATCHDOG(x)
#define SENDBYTE(c)         ComPutc(hcModem, c)
#define BUFFER_BYTE(c)      ComPutc(hcModem, c)
#define UNBUFFER_BYTES()    ComTxWait(hcModem, 1L)  /* yield cpu for a moment */
#define MODEM_IN()          ComGetc(hcModem)
#ifdef __BORLANDC__
#define NEED_WRITE_ANSI
#else
#define WRITE_ANSI(c)       putch (c)
#endif
#define PEEKBYTE()          ComPeek(hcModem)
#define do_break(on)        com_break(on)
#define SENDCHARS(buf, size, carcheck)  ComWrite(hcModem, buf, (USHORT)size)
#define hfComHandle         ComGetFH(hcModem)
#define real_flush(fh)      DosResetBuffer((HFILE)fh)

/* some useful bits */

/* (MSR) Received line signal detect sometimes called Carrier Detect */
#define RLSD    0x80

/* (LSR) Transmitter holding register empty (ready for another byte) */
#define THRE    0x20

/* (LSR) Data ready indicator */
#define DR      0x1

/* (LSR) Overrun error! We are not reading bytes fast enuf           */
#define OE      0x2

void ShowMdmSettings (void);

#else // NEED_OS2COMMS

#define TRUE 1
#define FALSE 0

#define BAUD_300        300
#define BAUD_1200       1200
#define BAUD_2400       2400
#define BAUD_4800       4800
#define BAUD_9600       9600
#define BAUD_19200      19200
#define BAUD_38400      38400
#define BAUD_57600      57600
#define BAUD_115200     115200

typedef unsigned char bool;

/*
   typedef unsigned char byte;
   typedef unsigned short word;
 */

extern int com_getc (int);
extern void com_putc (unsigned char c);
extern unsigned com_putc_now (unsigned char c);
extern void com_write (char *buf, unsigned int bytes, int carcheck);
extern void com_wait (void);
extern int com_getchar (void);
extern int com_peek (void);
extern int com_char_avail (void);
extern bool com_out_empty (void);
extern bool com_online (void);
extern void com_clear_out (void);
extern void com_clear_in (void);
extern void com_XON_enable (void);
extern void com_XON_disable (void);
extern long com_cur_baud (void);
extern int com_init (int port, char *unc);
extern int com_fini (void);
extern int com_set_baud (unsigned long rate,
                         char parity, int databits, int stopbits);
extern void com_DTR_on (void);
extern void com_DTR_off (void);
extern void com_break (int on);

extern void MDM_ENABLE (unsigned long rate);
extern void MDM_DISABLE (void);
extern void BUFFER_BYTE (unsigned char ch);
extern void UNBUFFER_BYTES (void);
extern unsigned Cominit (int port, int failsafe);
extern int get_key (void);

/* translate binkley fossil stuff to my async package */
#define CARRIER            (com_online())
#define MODEM_STATUS       (com_online())
#define CHAR_AVAIL()       com_char_avail()
#define OUT_EMPTY()        com_out_empty()
#define LOWER_DTR()        com_DTR_off()
#define RAISE_DTR()        com_DTR_on()
#define CLEAR_OUTBOUND()   com_clear_out()
#define CLEAR_INBOUND()    com_clear_in()
#define KEYPRESS()         kbhit()
#define READKB()           get_key()
#define FOSSIL_CHAR()      get_key()
#define XON_ENABLE()       com_XON_enable()
#define IN_XON_ENABLE()    com_XON_enable()
#define XON_DISABLE()      com_XON_disable()
#define _BRK_DISABLE()
#define FOSSIL_WATCHDOG(x)
#define SENDBYTE(c)        com_putc(c)
#define MODEM_IN()         com_getchar()
#define WRITE_ANSI(c)      putch(c)
#define PEEKBYTE()         com_peek()
#define do_break(on)       com_break(on)
#define SENDCHARS(buf, size, carcheck) com_write(buf, size, carcheck)
#define ComPause(x)
#define ComResume(x)

extern USHORT ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer);
extern USHORT ComTXRemain (void);

#define real_flush(fh)     DosResetBuffer(fh)

/* some usefull bits */

/* (MSR) Received line signal detect, sometimes called Carrier Detect */
#define RLSD  0x80
/* (LSR) Transmitter holding register empty (ready for another byte) */
#define THRE 0x20
/* (LSR) Data ready indicator */
#define DR   0x1
/* (LSR) Overrun error! We are not reading the bytes fast enuf */
#define OE   0x2

extern HFILE hfComHandle;
extern unsigned long WriteSem;

typedef struct
{
  ULONG cur_bps;
  CHAR bps_fract;
  ULONG min_bps;
  CHAR min_bps_frac;
  ULONG max_bps;
  CHAR max_bps_frac;
}
EXTBAUDRATE, *PEXTBAUDRATE;



#if defined(OS2_INCLUDED) && !defined(BSEDEV_INCLUDED) && !defined(ASYNC_SETBAUDRATE)

/* OS/2 Category 1 DosDevIoctl() calls */
#define SERIAL          1       /* category: serial device control */
#define ASYNC_SETBAUDRATE    0x0041  /* Sets baud rate                                 */
#define ASYNC_SETLINECTRL    0x0042  /* Sets line characteristics for serial
                                        * device    */
#define ASYNC_TRANSMITIMM    0x0044  /* Transmits byte immediately                     */
#define ASYNC_SETBREAKOFF    0x0045  /* Sets break off                                 */
#define ASYNC_SETMODEMCTRL   0x0046  /* Sets modem control register                    */
#define ASYNC_STOPTRANSMIT   0x0047  /* Stops device from transmitting                 */
#define ASYNC_STARTTRANSMIT  0x0048  /* Starts a transmission                          */
#define ASYNC_SETBREAKON     0x004B  /* Sets break on                                  */
#define ASYNC_SETDCBINFO     0x0053  /* Sets device control block information          */
#define ASYNC_GETBAUDRATE    0x0061  /* Retrieves current baud rate                    */
#define ASYNC_GETLINECTRL    0x0062  /* Gets line characterstics of serial
                                        * device      */
#define ASYNC_GETCOMMSTATUS  0x0064  /* Retrieves communications status                */
#define ASYNC_GETLINESTATUS  0x0065  /* Retrieves current line status                  */
#define ASYNC_GETMODEMOUTPUT 0x0066  /* Retrieves modem-control output signals         */
#define ASYNC_GETMODEMINPUT  0x0067  /* Retrieves modem control register               */
#define ASYNC_GETINQUECOUNT  0x0068  /* Retrieves number of characters in
                                        * input queue  */
#define ASYNC_GETOUTQUECOUNT 0x0069  /* Retrieves number of characters in
                                        * output q     */
#define ASYNC_GETCOMMERROR   0x006D  /* Retrieves communications error                 */
#define ASYNC_GETCOMMEVENT   0x0072  /* Retrieves communications event mask            */
#define ASYNC_GETDCBINFO     0x0073  /* Retrieves device control block
                                        * information     */

#define GENERIC         11      /* category: generic device control commands */
#define FLUSHINPUT      1       /* function: flush input buffer */
#define FLUSHOUTPUT     2       /* function: flush output buffer */

typedef struct _DCBINFO
{
  USHORT usWriteTimeout;
  USHORT usReadTimeout;
  BYTE fbCtlHndShake;
  BYTE fbFlowReplace;
  BYTE fbTimeout;
  BYTE bErrorReplacementChar;
  BYTE bBreakReplacementChar;
  BYTE bXONChar;
  BYTE bXOFFChar;
}
DCBINFO;

typedef struct _RXQUEUE
{
  USHORT cch;                   /* number of bytes queued */
  USHORT cb;                    /* queue size             */
}
RXQUEUE;

#else
#if !defined(BSEDEV_INCLUDED)
#define BSEDEV_INCLUDED
#endif
#endif

#endif // NEED_OS2COMMS

/* $Id: com_os2.h,v 1.5 1999/03/06 15:58:21 hjk Exp $ */
