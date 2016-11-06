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
 * Filename    : $Source: E:/cvs/btxe/include/css.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:05 $
 * State       : $State: Exp $
 * Orig. Author: TS, TJW
 *
 * Description : CSS definition
 *
 *---------------------------------------------------------------------------*/

#if !defined (_CSS_H)
#define _CSS_H

typedef struct XferStats
{
  char fname[40];               /*  current filename, zero terminated, init/invalid: '-' */
  long FilePos;                 /*  offset in file being transfered,   init/invalid: 0  */
  long FileLen;                 /*  of file being transferred          init/invalid: 0  */
  long tot_moh;                 /*  total mail on hold,                init/invalid: 0  *
                                 *  (this is what we sent/got in EMSI and normally      *
                                 *  constant                                            */
  long tot_files;               /*  total  number of files             init/invalid: 0  */
  long cur_mxfrd;               /*  current amount of mail xferred     init/invalid: 0  *
                                 *  (remains constant while a file is transferred)      *
                                 *  init: 0, then after a file is                       *
                                 *  successfully transferred: +=FileLen                 */
  long cur_fxfrd;               /*  current number of files xferred    init/invalid: 0  */
  long cur_cps;                 /*  current cps rate                   init/invalid: 0  */
  long low_cps;                 /*  lowest  cps rate so far            init/invalid: 0  */
  long high_cps;                /*  highest cps rate so far            init/invalid: 0  */
  long tot_cps;                 /*  total   cps rate so far            init/invalid: 0  */
  int cur_errors;               /*  current errors                     init/invalid: 0  */
  int tot_errors;               /*  total   errors                     init/invalid: 0  */
  long cur_time;                /*  seconds remaining current file     init/invalid: 0  */
  long tot_time;                /*  seconds remaining all files        init/invalid: 0  */

}
XFERSTATS;

typedef struct
{
  char remote[64];              /* remote address, name etc.        */
  char protocol[16];            /* transfer protocol, e.g. "Hydra"  */
}
GENSTATS;

void css_init (void);
void css_upd (void);
void css_done (void);

extern XFERSTATS TxStats, RxStats;  /* global */
extern GENSTATS GenStats;

#endif /*  !defined (_CSS_H) */

/* $Id: css.h,v 1.2 1999/02/27 01:15:05 mr Exp $ */
