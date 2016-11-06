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
 * Filename    : $Source: E:/cvs/btxe/include/com_dos.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:03 $
 * State       : $State: Exp $
 *
 * Description : Communications (FOSSIL) definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Baud rate masks */

#define BAUD_300        0x040
#define BAUD_1200       0x080
#define BAUD_2400       0x0A0
#define BAUD_4800       0x0C0
#define BAUD_9600       0x0E0
#define BAUD_19200      0x000
#define BAUD_38400      0x020

#define EXTENDED_BAUDRATES
#define BAUD_EXT_110    0x00
#define BAUD_EXT_150    0x01
#define BAUD_EXT_300    0x02
#define BAUD_EXT_600    0x03
#define BAUD_EXT_1200   0x04
#define BAUD_EXT_2400   0x05
#define BAUD_EXT_4800   0x06
#define BAUD_EXT_9600   0x07
#define BAUD_EXT_19200  0x08
#define BAUD_EXT_28800  0x80
#define BAUD_EXT_38400  0x81
#define BAUD_EXT_57600  0x82
#define BAUD_EXT_76800  0x83    /* With standard UART this isn't possible */
#define BAUD_EXT_115200 0x84

/* Bit definitions for the Line Status Register */

#define DATA_READY      0x0100
#define OVERRUN_ERR     0x0200
#define PARITY_ERR      0x0400
#define FRAMING_ERR     0x0800
#define BREAK_INT       0x1000
#define TX_HOLD_EMPTY   0x2000
#define TX_SHIFT_EMPTY  0x4000

int com_getc (int);
unsigned Cominit (int, int);
unsigned Com_ (char, byte);
void MDM_DISABLE (void);
void MDM_ENABLE (unsigned long);
int MODEM_IN (void);
int MODEM_STATUS (void);
int PEEKBYTE (void);
void SENDBYTE (unsigned char);
void CLEAR_INBOUND (void);
void CLEAR_OUTBOUND (void);
void SENDCHARS (char far *, unsigned int, int);
void BUFFER_BYTE (unsigned char);
void UNBUFFER_BYTES (void);
void do_break (int);
extern USHORT ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer);
extern USHORT ComTXRemain (void);

/*--------------------------------------------------------------------------*/
/* MACROS that call the routines in COM_ASM                                 */
/*--------------------------------------------------------------------------*/

extern unsigned Com_ (char, byte);
extern unsigned Cominit (int, int);
extern int READBYTE ();

/*#define M_INSTALL(b)       {Cominit(com_port);Com_(0x0f,0);Com_(0x0f,handshake_mask);new_baud(0,b);} */

/*-----------------------------------------------*/
/* Service 0: SET BAUD(etc)                      */
/*-----------------------------------------------*/

/*#define MDM_ENABLE(b)      (Com_(0x00,b|NO_PARITY|STOP_1|BITS_8)) */

/*-----------------------------------------------*/
/* Service 1: SEND CHAR (wait)                   */
/*-----------------------------------------------*/

/*#define SENDBYTE(x)    (Com_(0x01,x)) */

/*-----------------------------------------------*/
/* Service 2: GET CHAR (wait)                    */
/*-----------------------------------------------*/

/*#define MODEM_IN()         (Com_(0x02)&0x00ff) */

/*-----------------------------------------------*/
/* Service 3: GET STATUS                         */
/*-----------------------------------------------*/

/*#define MODEM_STATUS()     (Com_(0x03)) */
#define CARRIER            (Com_(0x03,0)&carrier_mask)
#define CHAR_AVAIL()       (MODEM_STATUS()&DATA_READY)
#define OUT_EMPTY()        (Com_(0x03,0)&TX_SHIFT_EMPTY)
#define OUT_FULL()         (!(Com_(0x03,0)&TX_HOLD_EMPTY))

/*-----------------------------------------------*/
/* Service 4: INIT/INSTALL                       */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 5: UNINSTALL                          */
/*-----------------------------------------------*/

/*#define MDM_DISABLE()      (Com_(0x05,BAUD_2400|NO_PARITY|STOP_1|BITS_8)) */

/*-----------------------------------------------*/
/* Service 6: SET DTR                            */
/*-----------------------------------------------*/

#define LOWER_DTR()        ((void) Com_(0x06,0))
#define RAISE_DTR()        ((void) Com_(0x06,1))

/*-----------------------------------------------*/
/* Service 7: GET TIMER TICK PARMS               */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 8: FLUSH OUTBOUND RING-BUFFER         */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 9: NUKE OUTBOUND RING-BUFFER          */
/*-----------------------------------------------*/

/*#define CLEAR_OUTBOUND()   (Com_(0x09,0)) */

/*-----------------------------------------------*/
/* Service a: NUKE INBOUND RING-BUFFER           */
/*-----------------------------------------------*/

/*#define CLEAR_INBOUND()    (Com_(0x0a,0)) */

/*-----------------------------------------------*/
/* Service b: SEND CHAR (no wait)                */
/*-----------------------------------------------*/

#define Com_Tx_NW(c)       (Com_(0x0b,c))

/*-----------------------------------------------*/
/* Service c: GET CHAR (no wait)                 */
/*-----------------------------------------------*/

/*#define PEEKBYTE()         (Com_(0x0c,0)) */

/*-----------------------------------------------*/
/* Service d: GET KEYBOARD STATUS                */
/*-----------------------------------------------*/

#define KEYPRESS()         (Com_(0x0d,0)!=(0xffff))
#define FOSSIL_PEEKKB()    (Com_(0x0d,0))

/*-----------------------------------------------*/
/* Service e: GET KEYBOARD CHARACTER (wait)      */
/*-----------------------------------------------*/

#define READKB()           (Com_(0x0e,0)&0xff)
#define READSCAN()         (Com_(0x0e,0))

#define FOSSIL_CHAR()      READSCAN()

/*-----------------------------------------------*/
/* Service f: SET/GET FLOW CONTROL STATUS        */
/*-----------------------------------------------*/

#define XON_ENABLE()       ((void) Com_(0x0f,(byte)handshake_mask))
#define XON_DISABLE()      ((void) Com_(0x0f,(byte)(handshake_mask&(~USE_XON))))
#define IN_XON_ENABLE()    ((void) Com_(0x0f,(byte)(handshake_mask|OTHER_XON)))
#define IN_XON_DISABLE()   ((void) Com_(0x0f,(byte)(handshake_mask&(~OTHER_XON))))

/*-----------------------------------------------*/
/* Service 10: SET/GET CTL-BREAK CONTROLS        */
/*             Note that the "break" here refers */
/*             to ^C and ^K rather than the      */
/*             tradition modem BREAK.            */
/*-----------------------------------------------*/

#define _BRK_ENABLE()      (Com_(0x10,BRK))
#define _BRK_DISABLE()     (Com_(0x10,0))
#define RECVD_BREAK()      (Com_(0x10,BRK)&BRK)

/*-----------------------------------------------*/
/* Service 11: SET LOCAL VIDEO CURSOR POSITION   */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 12: GET LOCAL VIDEO CURSOR POSITION   */
/*-----------------------------------------------*/

/*-----------------------------------------------*/
/* Service 13: WRITE LOCAL ANSI CHARACTER        */
/*-----------------------------------------------*/

#define WRITE_ANSI(c)    ((void) Com_(0x13,(byte)(c)))

/*-----------------------------------------------*/
/* Service 14: WATCHDOG on/off                   */
/*-----------------------------------------------*/

#define FOSSIL_WATCHDOG(x) (Com_(0x14,x))

/*-----------------------------------------------*/
/* Service 15: BIOS write to screen              */
/*-----------------------------------------------*/

#define WRITE_BIOS(c) (Com_(0x15,(byte)c))

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* A no-stall ReadByte routine might look like this:                        */
/*                                                                          */
/*    int READBYTE()                                                        */
/*    {                                                                     */
/*       return( CHAR_AVAIL() ? MODEM_IN() : (-1) );                        */
/*    }                                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/* $Id: com_dos.h,v 1.2 1999/02/27 01:15:03 mr Exp $ */
