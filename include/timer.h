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
 * Filename    : $Source: E:/cvs/btxe/include/timer.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:18 $
 * State       : $State: Exp $
 *
 * Description : Timer definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* This union holds a long integer as a long, 2 ints or 4 chars */

typedef union
{
  long l;
  struct
  {
    unsigned char c[4];
  }
  c;
  struct
  {
    unsigned int i[2];
  }
  i;
}
TIMETYPE;

#define PER_WEEK  60480000L
#define PER_DAY    8640000L
#define PER_HOUR    360000L
#define PER_MINUTE    6000L
#define PER_SECOND     100L

/* $Id: timer.h,v 1.2 1999/02/27 01:15:18 mr Exp $ */
