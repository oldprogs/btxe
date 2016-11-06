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
 * Filename    : $Source: E:/cvs/btxe/include/faxproto.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:07 $
 * State       : $State: Exp $
 * Orig. Author: Michael Buenter
 *
 * Description : Fax definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Constants for Class 2 commands */

/* exit codes  */

#define FAXSENT     0
#define FAXINSYNC   0
#define FAXNOSYNC   1
#define FAXNODIAL   2
#define FAXBUSY     3
#define FAXHANG     4
#define FAXERROR    5

/* My own page reception codes */

#define PAGE_GOOD           0
#define ANOTHER_DOCUMENT    1
#define END_OF_DOCUMENT     2
#define PAGE_HANGUP         4
#define PAGE_ERROR          5

/********************************************************/
/* Class 2 session parameters                           */

/* Set desired transmission params with +FDT=DF,VR,WD,LN
 * DF = Data Format :   0  1-d huffman
 *                      *1 2-d modified Read
 *                      *2 2-d uncompressed mode
 *                      *3 2-d modified modified Read
 *
 * VR = Vertical Res :  0 Normal, 98 lpi
 *                      1 Fine, 196 lpi
 *
 * WD = width :         0  1728 pixels in 215 mm
 *                      *1 2048 pixels in 255 mm
 *
 * LN = page length :   0 A4, 297 mm
 *                      1 B4, 364 mm
 *                      2  Unlimited
 *
 * EC = error correction :      0 disable ECM
 *
 * BF = binary file transfer :  0 disable BFT
 *
 * ST = scan time/line :        VR = normal     VR = fine
 *                         0    0 ms            0 ms
 *
 */

/* data format */

#define DF_1DHUFFMAN    0
#define DF_2DMREAD      1
#define DF_2DUNCOMP     2
#define DF_2DMMREAD     3

/* vertical resolution */

#define VR_NORMAL       0
#define VR_FINE         1

/* width */

#define WD_1728         0
#define WD_2048         1

/* page length */

#define LN_A4           0
#define LN_B4           1
#define LN_UNLIMITED    2

/* Baud rate */

#define BR_2400         0
#define BR_4800         1
#define BR_7200         2
#define BR_9600         3

/* A T.30 DIS frame, as sent by the remote fax machine */

struct T30Params
{
  int vr;
  /* VR = Vertical Res :             */
  /*     0 Normal, 98 lpi            */
  /*     1 Fine, 196 lpi             */

  int br;
  /* BR = Bit Rate :    br * 2400BPS */

  int wd;
  /* Page Width :                    */
  /*     0 1728 pixels in 215 mm     */
  /*     1 2048 pixels in 255 mm     */
  /*     2 2432 pixels in 303 mm     */

  int ln;
  /* Page Length :                   */
  /*     0 A4, 297 mm                */
  /*     1 B4, 364 mm                */
  /*     2 unlimited                 */

  int df;
  /* Data compression format :       */
  /*     0 1-D modified Huffman      */
  /*     1 2-D modified ReAd         */
  /*     2 2-D unompressed mode (?)  */

  int ec;
  /* Error Correction :              */
  /*     0 disable ECM               */
  /*     1 enable ECM 64 bytes/frame */
  /*     2 enable CM 256B/frame      */

  int bf;
  /* Binary File Transfer            */
  /*     0 disable BFT               */
  /*     1 enable BFT                */

  int st;
  /* Scan Time (ms) :                */
  /*     VR   Normal  Fine           */
  /*     0    0       0 ms           */
  /*     1    5       5              */
  /*     2    10      5              */
  /*     3    10      10             */
  /*     4    20      10             */
  /*     5    20      20             */
  /*     6    40      20             */
  /*     7    40      40             */
};

struct faxmodem_response
{
  char remote_id[50];           /* +FCSI remote id      */
  int hangup_code;              /* +FHNG code           */
  int post_page_response_code;  /* +FPTS code           */
  int post_page_message_code;   /* +FET code            */
  int fcon;                     /* Boolean; TRUE if +FCON  seen */
  int connect;                  /* Boolean; TRUE if CONNECT msg seen */
  int ok;                       /* Boolean; TRUE if OK seen */
  int error;                    /* Boolean; TRUE if ERROR or NO CARRIER seen */

  /* Session params; parsed from +FDCS */

  struct T30Params T30;
};

/* $Id: faxproto.h,v 1.2 1999/02/27 01:15:07 mr Exp $ */
