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
 * Filename    : $Source: E:/cvs/btxe/include/pktmsgs.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:15 $
 * State       : $State: Exp $
 *
 * Description : pkt definition
 *
 *---------------------------------------------------------------------------*/

struct S_PKTh
{
  FILE *handle;
  char *pktname;
  int isecho;
  ADDR originaddress, targetaddress;
  char echotag[65];
};

void FindUniquePKT (char *, char *);
struct S_PKTh *CreateMSGinPKT (char *, ADDR, ADDR, char *, ADDR,
                               char *, ADDR, char *, word, char *, char *,
                               struct tm *);
int WriteToPKT (struct S_PKTh *, char *,...);
int ClosePKT (struct S_PKTh *);

/* MR 970213 removed leading "0"s */

#define MSG_PRIVATE         1
#define MSG_CRASH           2
#define MSG_RCVD            4
#define MSG_SENT            8
#define MSG_ATTACH         16
#define MSG_TRANSIT        32
#define MSG_ORPHAN         64
#define MSG_KILLSENT      128
#define MSG_LOCAL         256
#define MSG_HOLD          512
#define MSG_UNUSED1      1024
#define MSG_FREQ         2048
#define MSG_RRR          4096
#define MSG_IRP          8192
#define MSG_AUDIT       16384
#define MSG_UPDATEREQ   32768

struct S_Packed
{
  unsigned short Signature;
  unsigned short OrigNode;
  unsigned short DestNode;
  unsigned short OrigNet;
  unsigned short DestNet;
  unsigned short Attribs;
  unsigned short Cost;
  char DateTime[20];
};

/* $Id: pktmsgs.h,v 1.2 1999/02/27 01:15:15 mr Exp $ */
