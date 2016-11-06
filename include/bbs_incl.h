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
 * Filename    : $Source: E:/cvs/btxe/include/bbs_incl.h,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/06/10 22:21:15 $
 * State       : $State: Exp $
 * Orig. Author: CEH
 *
 * Description : bbs include
 *
 *---------------------------------------------------------------------------*/

/*** defines ******************************************************************************/

#define defaultstrlen          256

/*** emnus ********************************************************************************/

enum TerminalEmulations
{
  emul_not_set,
  emul_ascii,
  emul_ansi
};

enum ScriptEvents
{
  ev_none = -1,
  ev_carrier_lost,
  ev_carrier_found,
  ev_fileerror,
  ev_internalerror,
  ev_f1_pressed,
  ev_f2_pressed,
  ev_f3_pressed,
  ev_f4_pressed,
  ev_f5_pressed,
  ev_f6_pressed,
  ev_f7_pressed,
  ev_f8_pressed,
  ev_f9_pressed,
  ev_f10_pressed,
  ev_timer_expired,
  ev_timer_expired_last = ev_timer_expired + 63,
  ev_number_of_scriptevents
};

/*** structs ******************************************************************************/

/* none                                                                                   */

/*** typedefs *****************************************************************************/

typedef char defaultstr[defaultstrlen];

/*** externs ******************************************************************************/

#define FIRSTUSERSTRINGVAR 7

extern defaultstr *strval;
extern long *numval;

extern enum TerminalEmulations emulation;
extern enum ScriptEvents eventdetected;

extern signed long *event_addresses;
extern short *event_modes;

extern char timers_used;
extern char timer_setonkey;
extern long time_setonkey;

extern char script_fullscreen;
extern short bbslocalmode;

/*** prototypes ***************************************************************************/

/* bbs_io.c */

int strcmpcase (char *a, char *b);
void TimeCheck (char init);
char *str_address (char);
long *num_address (char);
void createrealstring (char *now, char *old, int maxlen);
void bbs_initoutput ();
void bbssend (char *this);
void bbsgetinputline (char *this, char spacechar, char *special, int maxlength);
void file_removeline (FILE * file, long howmany);
void file_replaceline (FILE * file, char *this);
void file_insertline (FILE * file, char *this);
void bbs_calculate (char *instr, long *outnum);
void filegetdescriptiondone ();
void filegetdescriptioninit (char *infile, char *filesbbs, long *lines, long *size, long *date, long *counter);
void filegetdescriptiontext (char *info);
void bbs_printdate (time_t * longtime, char *mode, char *info);
void getfileinfosearch (char *find);
signed int bbs_sendfile (char *filename, char prot);
signed int bbs_receivefile (char *filename, char prot);

/* bbs_ctra.c */

void bbsLoadTranslation (char this);
char transout (char c);
char transin (char c);

/* b_term.c: cool, some stuff i don't have to re-invent :) */

int Upload (char *filepath, int prot, char *extern_ptr);
int Download (char *filepath, int prot, char *extern_ptr);

/* $Id: bbs_incl.h,v 1.4 1999/06/10 22:21:15 ceh Exp $ */
