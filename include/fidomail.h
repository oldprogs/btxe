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
 * Filename    : $Source: E:/cvs/btxe/include/fidomail.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:08 $
 * State       : $State: Exp $
 *
 * Description : fidomail definition
 *
 *---------------------------------------------------------------------------*/

#ifndef FIDOMAIL_INCLUDED
#define FIDOMAIL_INCLUDED

/* this data is extracted from fsc-0001.txt */

#define FM_PRIVATE         0x0001
#define FM_CRASH           0x0002
#define FM_MSG_RECVED      0x0004
#define FM_MSG_SENT        0x0008
#define FM_FILE_ATTACHED   0x0010
#define FM_INTRANSIT       0x0020
#define FM_ORPHAN          0x0040
#define FM_KILL_SENT       0x0080
#define FM_LOCAL_MSG       0x0100
#define FM_HOLD_MSG        0x0200
#define FM________unused   0x0400
#define FM_FILE_REQUEST    0x0800
#define FM_REQ_RET_RECEIPT 0x1000
#define FM_RET_RECEIPT     0x2000
#define FM_AUDIT_REQ       0x4000
#define FM_UPDATE_REQUEST  0x8000

typedef struct
{
  char sender[36];
  char receiver[36];
  char subject[72];
  char date_time_written[20];
  unsigned short times_read;    /* You can NOT rely on this!! */
  unsigned short destnode;
  unsigned short orignode;
  short cost;
  unsigned short orignet;
  unsigned short destnet;
  unsigned long opus_written;   /* these both longs are    */
  unsigned long date_arrived;   /* not defined in fsc-0001 */
  unsigned short reply;
  unsigned short attribute;
  unsigned short nextreply;

}
FIDOMSG, *PFIDOMSG;

#endif

/* $Id: fidomail.h,v 1.2 1999/02/27 01:15:08 mr Exp $ */
