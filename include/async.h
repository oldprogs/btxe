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
 * Filename    : $Source: E:/cvs/btxe/include/async.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:14:59 $
 * State       : $State: Exp $
 * Orig. Author: Peter Fitzsimmons
 *
 * Description : OS/2 Communications definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#ifndef _ASYNC_H_INCLUDED
#define _ASYNC_H_INCLUDED

#define TRUE  1
#define FALSE 0
#define DCD_ON 0x80
#define ERROR_MORE_DATA 234

#define IOCTL_ASYNC                 0x0001
#define IOCTL_SCR_AND_PTRDRAW       0x0003
#define IOCTL_KEYBOARD              0x0004
#define IOCTL_PRINTER               0x0005
#define IOCTL_LIGHTPEN              0x0006
#define IOCTL_POINTINGDEVICE        0x0007
#define IOCTL_DISK                  0x0008
#define IOCTL_PHYSICALDISK          0x0009
#ifndef __EMX__
#define IOCTL_MONITOR               0x000A
#define IOCTL_GENERAL               0x000B
#endif

/* General Device Control */

#define DEV_FLUSHINPUT              0x0001
#define DEV_FLUSHOUTPUT             0x0002
#define DEV_QUERYMONSUPPORT         0x0060

/* ASYNC_GETCOMMERROR, ASYNC_SETBREAKOFF, ASYNC_SETBREAKON, ASYNC_SETMODEMCTRL */

#define RX_QUE_OVERRUN              0x0001
#define RX_HARDWARE_OVERRUN         0x0002
#define PARITY_ERROR                0x0004
#define FRAMING_ERROR               0x0008

/* ASYNC_GETCOMMEVENT */

#define CHAR_RECEIVED               0x0001
#define LAST_CHAR_SENT              0x0004
#define CTS_CHANGED                 0x0008
#define DSR_CHANGED                 0x0010
#define DCD_CHANGED                 0x0020
#define BREAK_DETECTED              0x0040
#define ERROR_OCCURRED              0x0080
#define RI_DETECTED                 0x0100

/* ASYNC_GETCOMMSTATUS */

#define TX_WAITING_FOR_CTS          0x0001
#define TX_WAITING_FOR_DSR          0x0002
#define TX_WAITING_FOR_DCD          0x0004
#define TX_WAITING_FOR_XON          0x0008
#define TX_WAITING_TO_SEND_XON      0x0010
#define TX_WAITING_WHILE_BREAK_ON   0x0020
#define TX_WAITING_TO_SEND_IMM      0x0040
#define RX_WAITING_FOR_DSR          0x0080

/* ASYNC_GETLINESTATUS */

#define WRITE_REQUEST_QUEUED        0x0001
#define DATA_IN_TX_QUE              0x0002
#define HARDWARE_TRANSMITTING       0x0004
#define CHAR_READY_TO_SEND_IMM      0x0008
#define WAITING_TO_SEND_XON         0x0010
#define WAITING_TO_SEND_XOFF        0x0020

/* fbCtlHndShake */

#define MODE_DTR_CONTROL            0x01
#define MODE_DTR_HANDSHAKE          0x02
#define MODE_CTS_HANDSHAKE          0x08
#define MODE_DSR_HANDSHAKE          0x10
#define MODE_DCD_HANDSHAKE          0x20
#define MODE_DSR_SENSITIVITY        0x40

/* fbFlowReplace */

#define MODE_AUTO_TRANSMIT          0x01
#define MODE_AUTO_RECEIVE           0x02
#define MODE_ERROR_CHAR             0x04
#define MODE_NULL_STRIPPING         0x08
#define MODE_BREAK_CHAR             0x10
#define MODE_RTS_CONTROL            0x40
#define MODE_RTS_HANDSHAKE          0x80
#ifndef __EMX__
#define MODE_TRANSMIT_TOGGLE        0xC0
#endif

/* fbTimeout */

#define MODE_NO_WRITE_TIMEOUT       0x01
#define MODE_READ_TIMEOUT           0x02
#define MODE_WAIT_READ_TIMEOUT      0x04
#define MODE_NOWAIT_READ_TIMEOUT    0x06

#if !defined (BSEDEV_INCLUDED)

typedef struct _LINECONTROL
{                               /* lnctl */
  BYTE bDataBits;
  BYTE bParity;
  BYTE bStopBits;
  BYTE fTransBreak;
}
LINECONTROL;
typedef LINECONTROL FAR *PLINECONTROL;

typedef struct _MODEMSTATUS
{                               /* mdmst */
  BYTE fbModemOn;
  BYTE fbModemOff;
}
MODEMSTATUS;
typedef MODEMSTATUS FAR *PMODEMSTATUS;

#endif /* BSEDEV_INCLUDED not defined */

extern int com_getbuf (void);
extern bool com_in_check (void);

#endif /* _ASYNC_H_INCLUDED */

/* $Id: async.h,v 1.2 1999/02/27 01:14:59 mr Exp $ */
