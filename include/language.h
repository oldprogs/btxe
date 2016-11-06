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
 * Filename    : $Source: E:/cvs/btxe/include/language.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:13 $
 * State       : $State: Exp $
 *
 * Description : Language definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#define MAX_KEYFNCS 1024
#define MAX_MEMORY  (MAX_STRINGS * 32)
#define MAX_PRDCTS  256
#define MAX_STRINGS 1000
#define MAX_ANSI    256

extern struct _lang_hdr LangHdr;

extern char **pointers;         /* Array of pointers into memory */
extern short pointer_size;      /* Number of pointers            */
extern char *memory;            /* Memory                        */
extern short memory_size;       /* Size of memory                */

extern struct _lang_hdr PrdctHdr;

extern char *PrdctMem;
extern char **PrdctTbl;
extern char *PrdctUnknown;

extern short *TrmnlAccelAry;
extern short TrmnlAccelCnt;
extern struct _key_fnc *TrmnlAccelTbl;

extern short *UnattendedAccelAry;
extern short UnattendedAccelCnt;
extern struct _key_fnc *UnattendedAccelTbl;

extern struct _lang_hdr AnsiHdr;

extern char *AnsiMem;

extern int get_language (char *, char);
extern int put_language (char *);

/* $Id: language.h,v 1.2 1999/02/27 01:15:13 mr Exp $ */
