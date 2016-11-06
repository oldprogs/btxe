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
 * Filename    : $Source: E:/cvs/btxe/include/com_lnx.h,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/31 22:16:29 $
 * State       : $State: Exp $
 * Orig. Author: Ben Stuyts and Louis Lagendijk
 *
 * Description : Linux Communications definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#ifndef BOOLEAN
#define BOOLEAN unsigned char
#endif

/* Baud rate masks */

#define BAUD_300        B300
#define BAUD_1200       B1200
#define BAUD_2400       B2400
#define BAUD_4800       B4800
#define BAUD_9600       B9600
#define BAUD_19200      B19200
#define BAUD_38400      B38400
#define BAUD_57600      B57600
#define BAUD_115200     B115200

#define BITS_7          0x20
#define BITS_8          0x00
#define STOP_1          0x08
#define STOP_2          0x18
#define ODD_PARITY      0x06
#define EVEN_PARITY     0x04
#define NO_PARITY       0x00
#define USE_XON         0x01
#define USE_CTS         0x02
/* #define USE_DSR         0xXX */
/* #define OTHER_XON       0xXX */
#define BRK             0x01
#define MDM             0x02

unsigned Cominit (int, int);

#define CARRIER (com_online())

USHORT com_online (void);
BOOLEAN CHAR_AVAIL (void);
void SENDBYTE (unsigned char);
void SENDCHARS (char *, size_t, BOOLEAN);
BOOLEAN OUT_EMPTY (void);
void CLEAR_OUTBOUND (void);
void CLEAR_INBOUND (void);
int MODEM_IN (void);
int PEEKBYTE (void);
int com_getc (int);
void RAISE_DTR (void);
void LOWER_DTR (void);
void XON_ENABLE (void);
void IN_XON_ENABLE (void);
void XON_DISABLE (void);
void MDM_ENABLE (unsigned long);
void MDM_DISABLE (void);
void com_break (int);

#define do_break(i) com_break(i)

void BUFFER_BYTE (unsigned char);
void UNBUFFER_BYTES (void);
USHORT ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer);
USHORT ComTXRemain (void);

void linux_pause ();

BOOLEAN KEYPRESS (void);
int get_key (void);

#define FOSSIL_CHAR get_key
#define READKB get_key

void WRITE_ANSI (int);

char *strupr (char *);
char *strlwr (char *);
int memicmp (const void *__s1, const void *__s2, size_t __n);
char *ultoa (unsigned long, char *, int);
size_t filelength (int handle);

#define spawnv(a,b,c) execv(b,c)
#define spawnlp(a,b,c,d) execlp(b,c,d)

/* $Id: com_lnx.h,v 1.5 1999/03/31 22:16:29 ceh Exp $ */
