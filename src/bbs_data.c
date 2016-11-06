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
 * Filename    : $Source: E:/cvs/btxe/src/bbs_data.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:37 $
 * State       : $State: Exp $
 * Orig. Author: C. Eike Hofmeister, 2:2426/2280
 *
 * Description : global bbs-data
 *
 *---------------------------------------------------------------------------*/

#ifdef BBS_INTERNAL

#include "includes.h"
#include "bbs_incl.h"

defaultstr *strval;
long *numval;

enum TerminalEmulations emulation;
enum ScriptEvents eventdetected;

signed long *event_addresses;
short *event_modes;

char timers_used;
char timer_setonkey;
long time_setonkey;

char script_fullscreen;
short bbslocalmode;

#endif

/* $Id: bbs_data.c,v 1.2 1999/02/27 01:15:37 mr Exp $ */
