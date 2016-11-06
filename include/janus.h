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
 * Filename    : $Source: E:/cvs/btxe/include/janus.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:10 $
 * State       : $State: Exp $
 *
 * Description : Janus definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#ifndef _JANUS_H_INCLUDED
#define _JANUS_H_INCLUDED

/* Misc. Constants */

#define BUFMAX 2048             /* Max packet contents length                   */
#define NUM_FLAGS 4             /* Number of mail-type flag characters (O,D,C,H) */
#define JANUS_EFFICIENCY 95     /* Estimate Janus xfers at 95% throughput       */

/* Positions of transfer status line fields */

#define POS_X 20
#define ETA_X 30
#define MSG_X 49

/* File Transmission States */

#define XDONE           0       /* All done, no more files to transmit          */
#define XSENDFNAME      1       /* Send filename packet                         */
#define XRCVFNACK       2       /* Wait for filename packet ACK                 */
#define XSENDBLK        3       /* Send next block of file data                 */
#define XRCVEOFACK      4       /* Wait for EOF packet ACK                      */
#define XSENDFREQNAK    5       /* Send file request NAK (no matches found)     */
#define XRCVFRNAKACK    6       /* Wait for ACK to file request NAK             */

/* File Reception States */

#define RDONE           0       /* All done, nothing more to receive            */
#define RRCVFNAME       1       /* Wait for filename packet                     */
#define RRCVBLK         2       /* Wait for next block of file data             */

/* Packet Types */

#define NOPKT           0       /* No packet received yet; try again later      */
#define BADPKT          '@'     /* Bad packet received; CRC error, overrun, etc. */
#define FNAMEPKT        'A'     /* Filename info packet                         */
#define FNACKPKT        'B'     /* Filename packet ACK                          */
#define BLKPKT          'C'     /* File data block packet                       */
#define RPOSPKT         'D'     /* Transmitter reposition packet                */
#define EOFACKPKT       'E'     /* EOF packet ACK                               */
#define HALTPKT         'F'     /* Immediate screeching halt packet             */
#define HALTACKPKT      'G'     /* Halt packet ACK for ending batch             */
#define FREQPKT         'H'     /* File request packet                          */
#define FREQNAKPKT      'I'     /* File request NAK (no matches for your req)   */
#define FRNAKACKPKT     'J'     /* ACK to file request NAK                      */

/* Non-byte values returned by rcvbyte() */

#define BUFEMPTY    (-1)
#define PKTSTRT     (-2)
#define PKTEND      (-3)
#define NOCARRIER   (-4)
#define PKTSTRT32   (-5)

/* Bytes we need to watch for */

#define PKTSTRTCHR      'a'
#define PKTENDCHR       'b'
#define PKTSTRTCHR32    'c'

/* Various action flags */

#define GOOD_XFER       0
#define FAILED_XFER     1
#define INITIAL_XFER    2
#define ABORT_XFER      3

/* #define DELETE_AFTER    '-'   already defined in xfer.h (r. hoerner) */
/* #define TRUNC_AFTER     '#'   already defined in xfer.h */

/* Protocol capability flags */

#define CANCRC32    0x80
#define CANFREQ     0x40

#define OURCAP  (CANCRC32 | CANFREQ)

#endif /* _JANUS_H_INCLUDED */

/* $Id: janus.h,v 1.2 1999/02/27 01:15:10 mr Exp $ */
