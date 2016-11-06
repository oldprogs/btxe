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
 * Filename    : $Source: E:/cvs/btxe/include/com.h,v $
 * Revision    : $Revision: 1.6 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/06 15:58:20 $
 * State       : $State: Exp $
 *
 * Description : Communications definitions for BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#ifndef __unix__

#define BITS_7          0x02
#define BITS_8          0x03
#define STOP_1          0x00
#define STOP_2          0x04
#define ODD_PARITY      0x08
#define EVEN_PARITY     0x18
#define NO_PARITY       0x00

/* Bit definitions for the driver flags */

#define USE_XON         0x01
#define USE_CTS         0x02
#define USE_DSR         0x04
#define OTHER_XON       0x08

#define BRK             0x01
#define MDM             0x02

#endif

struct LINE_CONTROL
{
  int carrier_mask;
  int handshake_mask;
};

struct baud_str
{
  unsigned long rate_value;
  unsigned long rate_mask;
};

struct secure
{
  char *rq_OKFile;
#ifdef SPECIALMAGICS
  char *rq_FILES;
  char *rq_About;
#endif
  char *rq_Template;
  char *sc_Inbound;
  int rq_Limit;
  int time_Limit;
  long byte_Limit;
  int rq_Cum_Limit;
  long time_Cum_Limit;
  long byte_Cum_Limit;
  short security;               /* Maximus Security                 */
  dword xkeys;                  /* AW980219 Maximus Keys            */
  char *class;                  /* AW980219 Maximus userclass/flags */
};

struct req_accum
{
  long CumBytes;
  long CumFiles;
  long CumTime;

  /*
   * This is measured in SECONDS of request processing time, as
   * opposed to time, below, which is the "here and now" time.
   * The reason for the Cum v non-Cum stuff is to allow for two sets of
   * verbs - one for a session limit, one for a cumulative limit.
   * We also need a verb to shut the accumulation shit off.
   */

  int files;
  long bytes;
  long time;
  long LastTime;
};

#ifdef DOS16
#include "com_dos.h"
#endif

#ifdef OS_2
#include "com_os2.h"
#endif

#ifdef __unix__
#include "com_lnx.h"
#endif

#ifdef _WIN32
#include "com_w32.h"
#endif

/* $Id: com.h,v 1.6 1999/03/06 15:58:20 hjk Exp $ */
