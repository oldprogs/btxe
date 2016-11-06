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
 * Filename    : $Source: E:/cvs/btxe/include/xfer.h,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/05 02:24:42 $
 * State       : $State: Exp $
 * Orig. Author: Wynn Wagner III
 *
 * Description : Definitions used in BinkleyTerm File Transfer logic
 *
 * Note        :
 *   This module is based largely on a similar module in OPUS-CBCS V1.03b.
 *   The original work is (C) Copyright 1987, Wynn Wagner III. The original
 *   author has graciously allowed us to use his code in this work.
 *
 *---------------------------------------------------------------------------*/

#define stcgfn                  strcpy
#define wherex()                fossil_wherex()
#define wherey()                fossil_wherey()
#define gotoxy(x,y)             fossil_gotoxy(x,y)
#define CLEAR_IOERR()           errno = 0
#define wait_for_clear()        while (!OUT_EMPTY())
#define TIMED_READ(a)           (int) com_getc(a)

/*--------------------------------------------------------------------------*/
/* FLO-file tokens                                                          */
/*--------------------------------------------------------------------------*/

#define SHOW_DELETE_AFTER   '^'
#define DELETE_AFTER        '-'
#define TRUNC_AFTER         '#'
#define NOTHING_AFTER       '@'
#define FILE_ALREADY_SENT   '~'

/*--------------------------------------------------------------------------*/
/* resultcodes for transfer protocols.                                      */
/* *each* and *every* protocol should return resultcodes from this set!     */
/*--------------------------------------------------------------------------*/
#define FILE_SKIPPED     2      /* session ok, but don't truncate file      */
/*--------------------------------------------------------------------------*/
#define FILE_SENT_OK     1      /* file sent, ok to delete                  */
#define NOTHING_SENT     0      /* nothing sent. Mutually an error          */
#define CANNOT_SEND     -1      /* an error occured. ok to abort            */
/*--------------------------------------------------------------------------*/
#define FILE_RECV_OK     1      /* file sent, ok to delete                  */
#define NOTHING_RECVED   0      /* nothing sent. Mutually an error          */
#define CANNOT_RECV     -1      /* an error occured. ok to abort            */
/*--------------------------------------------------------------------------*/

/* $Id: xfer.h,v 1.3 1999/03/05 02:24:42 mr Exp $ */
