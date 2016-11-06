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
 * Filename    : $Source: E:/cvs/btxe/include/asyos2v1.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:14:59 $
 * State       : $State: Exp $
 * Orig. Author: MR
 *
 * Description : define some things missing in the OS/2 v1.x header files...
 *
 *---------------------------------------------------------------------------*/

#ifndef _ASYNC_OS2v1x_H_INCLUDED
#define _ASYNC_OS2v1x_H_INCLUDED

/* Input and Output Control Categories */

#define IOCTL_ASYNC                        0x0001
#define IOCTL_SCR_AND_PTRDRAW              0x0003
#define IOCTL_KEYBOARD                     0x0004
#define IOCTL_PRINTER                      0x0005
#define IOCTL_LIGHTPEN                     0x0006
#define IOCTL_POINTINGDEVICE               0x0007
#define IOCTL_DISK                         0x0008
#define IOCTL_PHYSICALDISK                 0x0009
#define IOCTL_MONITOR                      0x000A
#define IOCTL_GENERAL                      0x000B

/* Serial-Device Control */

#define ASYNC_SETBAUDRATE                  0x0041
#define ASYNC_SETLINECTRL                  0x0042
#define ASYNC_TRANSMITIMM                  0x0044
#define ASYNC_SETBREAKOFF                  0x0045
#define ASYNC_SETMODEMCTRL                 0x0046
#define ASYNC_SETBREAKON                   0x004B
#define ASYNC_STOPTRANSMIT                 0x0047
#define ASYNC_STARTTRANSMIT                0x0048
#define ASYNC_SETDCBINFO                   0x0053
#define ASYNC_GETBAUDRATE                  0x0061
#define ASYNC_GETLINECTRL                  0x0062
#define ASYNC_GETCOMMSTATUS                0x0064
#define ASYNC_GETLINESTATUS                0x0065
#define ASYNC_GETMODEMOUTPUT               0x0066
#define ASYNC_GETMODEMINPUT                0x0067
#define ASYNC_GETINQUECOUNT                0x0068
#define ASYNC_GETOUTQUECOUNT               0x0069
#define ASYNC_GETCOMMERROR                 0x006D
#define ASYNC_GETCOMMEVENT                 0x0072
#define ASYNC_GETDCBINFO                   0x0073

/* fbCtlHndShake */
#define MODE_DTR_CONTROL                   0x01
#define MODE_DTR_HANDSHAKE                 0x02
#define MODE_CTS_HANDSHAKE                 0x08
#define MODE_DSR_HANDSHAKE                 0x10
#define MODE_DCD_HANDSHAKE                 0x20
#define MODE_DSR_SENSITIVITY               0x40

/* fbFlowReplace */
#define MODE_AUTO_TRANSMIT                 0x01
#define MODE_AUTO_RECEIVE                  0x02
#define MODE_ERROR_CHAR                    0x04
#define MODE_NULL_STRIPPING                0x08
#define MODE_BREAK_CHAR                    0x10
#define MODE_RTS_CONTROL                   0x40
#define MODE_RTS_HANDSHAKE                 0x80
#define MODE_TRANSMIT_TOGGLE               0xC0

/* fbTimeout */
#define MODE_NO_WRITE_TIMEOUT              0x01
#define MODE_READ_TIMEOUT                  0x02
#define MODE_WAIT_READ_TIMEOUT             0x04
#define MODE_NOWAIT_READ_TIMEOUT           0x06

typedef struct _DCBINFO         /* dcbinf */
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
DCBINFO, *PDCBINFO;

/* MODEMSTATUS.fbModemOn, ASYNC_GETMODEMOUTPUT */
#define DTR_ON                             0x01
#define RTS_ON                             0x02

/* MODEMSTATUS.fbModemOff */
#define DTR_OFF                            0xFE
#define RTS_OFF                            0xFD

typedef struct _MODEMSTATUS
{                               /* mdmst */
  BYTE fbModemOn;
  BYTE fbModemOff;
}
MODEMSTATUS;
typedef MODEMSTATUS FAR *PMODEMSTATUS;

#endif /* _ASYNC_OS2v1x_H_INCLUDED */

/* $Id: asyos2v1.h,v 1.2 1999/02/27 01:14:59 mr Exp $ */
