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
 * Filename    : $Source: E:/cvs/btxe/include/debug.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:06 $
 * State       : $State: Exp $
 *
 * Description : debug helpers
 *
 *---------------------------------------------------------------------------*/

#define DEBUGFLAGS_NODISPLAYLIFE     0x0001L
#define DEBUGFLAGS_NOCSS             0x0002L
#define DEBUGFLAGS_NOIDLE            0x0004L
#define DEBUGFLAGS_NOFLUSH           0x0008L

#ifdef DEBUG

FILE *debug_share_fopen (char *fname, char *mode, int shflag, int TaskNumber);
FILE *debug_fopen (char *fname, char *mode, int TaskNumber);
int debug_fclose (FILE * fp, int TaskNumber);

void *debug_calloc (size_t x, size_t y, int TaskNumber, char *file, int line);
void *debug_malloc (size_t amount, int TaskNumber, char *file, int line);
void debug_free (void *p, int TaskNumber, char *file, int line);

#define share_fopen(fn,m,sh) debug_share_fopen(fn,m,sh,TaskNumber)
#define fopen(fn,m)          debug_fopen(fn,m,TaskNumber)
#define fclose(fp)           debug_fclose(fp,TaskNumber)

#define calloc(a,b)          debug_calloc(a,b,TaskNumber,__FILE__,__LINE__)
#define malloc(a)            debug_malloc(a,TaskNumber,__FILE__,__LINE__)
#define free(p)              debug_free(p,TaskNumber,__FILE__,__LINE__)

#endif

/* $Id: debug.h,v 1.2 1999/02/27 01:15:06 mr Exp $ */
