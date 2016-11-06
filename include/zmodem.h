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
 * Filename    : $Source: E:/cvs/btxe/include/zmodem.h,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/04/20 18:46:40 $
 * State       : $State: Exp $
 * Orig. Author: Wynn Wagner III
 *
 * Description : Zmodem definitions for BinkleyTerm
 *
 * Note        :
 *   This module is based largely on a similar module in OPUS-CBCS V1.03b.
 *   The original work is (C) Copyright 1986, Wynn Wagner III. The original
 *   authors have graciously allowed us to use their code in this work.
 *
 *---------------------------------------------------------------------------*/

#ifndef _ZMODEM_H_INCLUDED
#define _ZMODEM_H_INCLUDED

#define flushmo()   wait_for_clear()
#define Z_UpdateCRC(cp,crc)     (crctab[((crc >> 8) & 255) ^ cp] ^ (crc << 8))
#define Z_32UpdateCRC(c,crc)    (cr3tab[((int) crc ^ c) & 0xff] ^ ((crc >> 8) & 0x00FFFFFFUL))

/*--------------------------------------------------------------------------*/

#define ZPAD      '*'           /* 052 Padding character begins frames      */
#define ZDLE      030           /* Ctrl-X Zmodem escape - `ala BISYNC DLE   */
#define ZDLEE  (ZDLE^0100)      /* Escaped ZDLE as transmitted              */
#define ZBIN      'A'           /* Binary frame indicator                   */
#define ZHEX      'B'           /* HEX frame indicator                      */
#define ZBIN32    'C'           /* Binary frame with 32 bit FCS             */

/*--------------------------------------------------------------------------*/
/* Frame types (see array "frametypes" in zm.c)                             */
/*--------------------------------------------------------------------------*/

#define ZRQINIT      0          /* Request receive init                     */
#define ZRINIT       1          /* Receive init                             */
#define ZSINIT       2          /* Send init sequence (optional)            */
#define ZACK         3          /* ACK to above                             */
#define ZFILE        4          /* File name from sender                    */
#define ZSKIP        5          /* To sender: skip this file                */
#define ZNAK         6          /* Last packet was garbled                  */
#define ZABORT       7          /* To Sender: Abort batch transfers         */
#define ZFIN         8          /* To Receiver: Finish session              */
#define ZRPOS        9          /* To Sender: Resume data at this position  */
#define ZDATA       10          /* Data packet(s) follow                    */
#define ZEOF        11          /* End of file                              */
#define ZFERR       12          /* Fatal Read or Write error Detected.As ZABORT */
#define ZCRC        13          /* Request for file CRC and response        */
#define ZCHALLENGE  14          /* Receiver's Challenge                     */
#define ZCOMPL      15          /* Request is complete                      */
#define ZCAN        16          /* Resp. to other end canned session with CAN*5 */
#define ZFREECNT    17          /* Request for free bytes on filesystem     */
#define ZCOMMAND    18          /* Command from sending program             */
#define ZSTDERR     19          /* Output to standard error, data follows   */

/*--------------------------------------------------------------------------*/
/* ZDLE sequences                                                           */
/*--------------------------------------------------------------------------*/

#define ZCRCE      'h'          /* CRC next, frame ends, header packet follows */
#define ZCRCG      'i'          /* CRC next, frame continues nonstop        */
#define ZCRCQ      'j'          /* CRC next, frame continues, ZACK expected */
#define ZCRCW      'k'          /* CRC next, ZACK expected, end of frame    */
#define ZRUB0      'l'          /* Translate to rubout 0177                 */
#define ZRUB1      'm'          /* Translate to rubout 0377                 */

/*--------------------------------------------------------------------------*/
/* Z_GetZDL return values (internal)                                        */
/* -1 is general error, -2 is timeout                                       */
/*--------------------------------------------------------------------------*/

#define GOTOR      0400
#define GOTCRCE   (ZCRCE|GOTOR) /* ZDLE-ZCRCE received */
#define GOTCRCG   (ZCRCG|GOTOR) /* ZDLE-ZCRCG received */
#define GOTCRCQ   (ZCRCQ|GOTOR) /* ZDLE-ZCRCQ received */
#define GOTCRCW   (ZCRCW|GOTOR) /* ZDLE-ZCRCW received */
#define GOTCAN    (GOTOR|030)   /* CAN*5 seen          */

/*--------------------------------------------------------------------------*/
/* Byte positions within header array                                       */
/*--------------------------------------------------------------------------*/

#define ZF0          3          /* First flags byte                         */
#define ZF1          2
#define ZF2          1
#define ZF3          0
#define ZP0          0          /* Low order 8 bits of position             */
#define ZP1          1
#define ZP2          2
#define ZP3          3          /* High order 8 bits of file position       */

/*--------------------------------------------------------------------------*/
/* Bit Masks for ZRINIT flags byte ZF0                                      */
/*--------------------------------------------------------------------------*/

#define CANFDX       01         /* Rx can send and receive true FDX         */
#define CANOVIO      02         /* Rx can receive DATA during disk I/O      */
#define CANBRK       04         /* Rx can send a break signal               */
#define CANCRY      010         /* Receiver can decrypt                     */
#define CANLZW      020         /* Receiver can uncompress                  */
#define CANFC32     040         /* Receiver can use 32 bit Frame Check      */
#define ESCCTL     0100         /* Receiver expects ctl chars to be escaped */
#define ESC8       0200         /* Receiver expects 8th bit to be escaped   */

/*--------------------------------------------------------------------------*/
/* PARAMETERS FOR ZFILE FRAME...                                            */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Conversion options one of these in ZF0                                   */
/*--------------------------------------------------------------------------*/

#define ZCBIN         1         /* Binary transfer - inhibit conversion     */
#define ZCNL          2         /* Convert NL to local end of line convention */
#define ZCRESUM       3         /* Resume interrupted file transfer         */

/*--------------------------------------------------------------------------*/
/* Management include options, one of these ored in ZF1                     */
/*--------------------------------------------------------------------------*/

#define ZMSKNOLOC  0200         /* Skip file if not present at rx           */

/*--------------------------------------------------------------------------*/
/* Management options, one of these in ZF1                                  */
/*--------------------------------------------------------------------------*/

#define ZMMASK      037         /* Mask for the choices below               */
#define ZMNEWL        1         /* Transfer if source newer or longer       */
#define ZMCRC         2         /* Transfer if different file CRC or length */
#define ZMAPND        3         /* Append contents to existing file (if any) */
#define ZMCLOB        4         /* Replace existing file                    */
#define ZMNEW         5         /* Transfer if source newer                 */
#define ZMDIFF        6         /* Transfer if dates or lengths different   */
#define ZMPROT        7         /* Protect destination file                 */

/*--------------------------------------------------------------------------*/
/* Transport options, one of these in ZF2                                   */
/*--------------------------------------------------------------------------*/

#define ZTLZW         1         /* Lempel-Ziv compression                   */
#define ZTCRYPT       2         /* Encryption                               */
#define ZTRLE         3         /* Run Length encoding                      */

/*--------------------------------------------------------------------------*/
/* Extended options for ZF3, bit encoded                                    */
/*--------------------------------------------------------------------------*/

#define ZXSPARS      64         /* Encoding for sparse file operations      */

/*--------------------------------------------------------------------------*/
/* Parameters for ZCOMMAND frame ZF0 (otherwise 0)                          */
/*--------------------------------------------------------------------------*/

#define ZCACK1        1         /* Acknowledge, then do command             */

/*--------------------------------------------------------------------------*/
/* Miscellaneous definitions (not in original zmodem.h)                     */
/*--------------------------------------------------------------------------*/

#define OK              0
#define ERROR           (-1)
#define TIMEOUT         (-2)
#define RCDO            (-3)
#define FUBAR           (-4)

#define XON             ('Q'&037)
#define XOFF            ('S'&037)
#define CPMEOF          ('Z'&037)

#define RXBINARY        FALSE   /* is dummy-used (r. hoerner) */
/* #define RXASCII         FALSE               unused        (r. hoerner) */
#define LZCONV          0
#define LZMANAG         0
#define LZTRANS         0
#define PATHLEN         256     /* was: 128      (r. hoerner) */
#define KSIZE           1024
#define WAZOOMAX        8192

/*--------------------------------------------------------------------------*/
/* Parameters for calling ZMODEM routines                                   */
/*--------------------------------------------------------------------------*/

#define ZMODEM_INIT       0
#define ZMODEM_ENDBATCH  -1
#define ZMODEM_END       -2
#define DO_WAZOO        TRUE

/*--------------------------------------------------------------------------*/
/* ASCII MNEMONICS                                                          */
/*--------------------------------------------------------------------------*/

#define NUL 0x00
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define BEL 0x07
#define BKS 0x08
#define HT  0x09
#define LF  0x0a
#define VT  0x0b
#define FF  0x0c
#define CR  0x0d
#define SO  0x0e
#define SI  0x0f
#define DLE 0x10
#define DC1 0x11
#define DC2 0x12
#define DC3 0x13
#define DC4 0x14
#define NAK 0x15
#define SYN 0x16
#define ETB 0x17
#define CAN 0x18
#define EM  0x19
#define SUB 0x1a
#define ESC 0x1b
#define FS  0x1c
#define GS  0x1d
/* #define RS  0x1e */
/* CEH 990401: not used and already defined in HPUX, differently */
#define US  0x1f

#endif /* _ZMODEM_H_INCLUDED */

/* $Id: zmodem.h,v 1.4 1999/04/20 18:46:40 ceh Exp $ */
