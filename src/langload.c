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
 * Filename    : $Source: E:/cvs/btxe/src/langload.c,v $
 * Revision    : $Revision: 1.8 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 17:40:18 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Language File Loader
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"
#include "lngmagic.h"

/* MMP 9604xx: changes to load language file from (patched) EXE */

/* Read the compiled BinkleyTerm Language file. */

int
load_language (char *exe)
{
  int pointer_size;
  char *memory;
  unsigned int memory_size;
  char *malloc_target;
  char LANGpath[PATHLEN];
  int error;
  int i;
  int iReadT;
  FILE *fpt;                    /* stream pointer           */
  char *Ptr;
  unsigned short size;

#ifdef USELNGFILE               /* TE 041498 */

  long tellsize;

#endif

#ifndef __unix__

  int len;
  long magic;


  strcpy (LANGpath, exe);
  len = strlen (LANGpath);

  if (len < 4 || stricmp (LANGpath + len - 4, ".exe"))
    strcat (LANGpath, ".exe");

  /* Open the input file */

  /* fpt = share_fopen (LANGpath, read_binary, DENY_WRITE); AW 971214 open .EXE file in Compatibility mode */

  fpt = fopen (LANGpath, read_binary);
  if (fpt == NULL)              /* Were we successful?       */
  {
    (void) fprintf (stderr, "load_language: Cannot open input file %s\n", LANGpath);
#ifndef USELNGFILE
    exit (250);
#endif
  }

  /* Determine file size. If we are loading from the .EXE itself, the
   * language file is located at the end of the file followed by its
   * size. */

  if (fpt != NULL)              /* CEH 990104: This should be checked here, too, not only below */
  {
    if (fseek (fpt, -4, SEEK_END) ||
        fread (&magic, 1, 4, fpt) != 4 ||
        magic != LNGMAGIC)
    {
      (void) fprintf (stderr, "load_language: Cannot read language file from %s\n", exe);
      (void) fclose (fpt);
      fpt = NULL;
    }
  }

  if (fpt != NULL)
  {
    if (fseek (fpt, -6, SEEK_END) ||
        fread (&size, 1, 2, fpt) != 2 ||
        fseek (fpt, -(long) size - 6, SEEK_END))
    {
      (void) fprintf (stderr, "load_language: Cannot read language file from %s\n", exe);
      (void) fclose (fpt);
      fpt = NULL;
    }
  }

#else

  fpt = NULL;

#endif

  /* TE 041498: I re-implemented the use of a standalone LNG file because
   * Turbo Debugger does not recognize it's debugging info if the
   * language data  is patched into the executable. */

#ifdef USELNGFILE
  if (fpt == NULL)
  {

    if (strchr (config_name, DIR_SEPC) != NULL)
    {
      char *src, *dst;

      strcpy (LANGpath, config_name);  // CEH 990110: read language-file from the same path as config

      for (src = dst = LANGpath; *src; src++)
        if ((*src) == DIR_SEPC)
          dst = src + 1;
      *dst = 0;
    }
    else
      LANGpath[0] = 0;

    strcat (LANGpath, "binkley.lng");
    fpt = fopen (LANGpath, read_binary);
    if (fpt != NULL)
    {
      if (fseek (fpt, 0, SEEK_END) ||
          (tellsize = ftell (fpt)) == -1 ||
          fseek (fpt, 0, SEEK_SET))
      {
        fprintf (stderr, "load_language: Cannot read language file from BINLKEY.LNG.\n");
        fclose (fpt);
        fpt = NULL;
      }
      else
      {
        size = (unsigned short) tellsize;
#ifndef __unix__
        fprintf (stderr, "load_language: using binkley.lng instead.\n");
#endif
      }
    }
    else
      fprintf (stderr, "load_language: %s not found\n", LANGpath);
  }
#endif

  if (fpt == NULL)              /* Were we successful?       */
    exit (250);

  memory_size = size;

  /* Allocate space for the raw character array and for the
   * pointer and fixup arrays */

  Ptr = malloc_target = calloc (1, memory_size);
  if (malloc_target == NULL)
  {
    fprintf (stderr, "load_language: Unable to allocate string memory\n");
    fclose (fpt);
    exit (250);
  }

  /* Read the entire file into memory now. */

  iReadT = fread (malloc_target, 1, memory_size, fpt);
  if ((unsigned) iReadT != memory_size)
  {
    fprintf (stderr, "load_language: Could not read language data from file %s\n", LANGpath);
    free (malloc_target);
    fclose (fpt);
    exit (250);
  }

  /* Close the file. */

  error = fclose (fpt);
  if (error != 0)
  {
    fprintf (stderr, "load_language: Unable to close language file %s\n", LANGpath);
    free (malloc_target);
    exit (250);
  }

  /* Do fixups on the string pointer array as follows:
   *  1. Get element count from input
   *  2. Start of the string memory immediately follows the strings
   *  3. Apply arithmetic correction to entire array. */

  LangHdr = *(struct _lang_hdr *) malloc_target;  /* TJW 960706 struct ass. */
  msgtxt = (char **) (malloc_target + sizeof (struct _lang_hdr));

  pointer_size = LangHdr.ElemCnt;  /* Count of elements w/o NULL */
  if (pointer_size != X_TOTAL_MSGS)
  {
    fprintf (stderr, "load_language: Count of %d from file does not match %d required\n",
             pointer_size, X_TOTAL_MSGS);
    free (malloc_target);
    exit (250);
  }

  memory = (char *) &msgtxt[pointer_size];  /* Text starts after pointers */
  for (i = 1; i < pointer_size; i++)
    msgtxt[i] = memory + (int) (msgtxt[i] - msgtxt[0]);

  msgtxt[0] = memory;

  /* Process the Terminal Mode Keymap.
   * First comes the integer count.
   * Following that is the array of keymaps. */

  Ptr = (char *) malloc_target;
  Ptr += sizeof (struct _lang_hdr);
  Ptr += sizeof (char *) * (LangHdr.ElemCnt);

  Ptr += LangHdr.PoolSize;

  TrmnlKeyFncHdr.KeyFncCnt = *(short *) Ptr;
  Ptr += sizeof (short);

  TrmnlKeyFncHdr.KeyFncTbl = (struct _key_fnc *) Ptr;

  Ptr += sizeof (struct _key_fnc) * TrmnlKeyFncHdr.KeyFncCnt;

  /* Process the Unattended Mode Keymap.
   * First comes the integer count.
   * Following that is the array of keymaps. */

  UnattendedKeyFncHdr.KeyFncCnt = *(short *) Ptr;
  Ptr += sizeof (short);

  UnattendedKeyFncHdr.KeyFncTbl = (struct _key_fnc *) Ptr;

  Ptr += sizeof (struct _key_fnc) * UnattendedKeyFncHdr.KeyFncCnt;

  /* Process the product code table.
   * This looks a lot like the language stuff above.
   *
   * The first thing we have here is the header (which contains a count).
   * Following that, the pointer array.
   * Finally, the strings themselves.
   *
   * Get all of that sorted out, do fixups on the string array, and
   * everything should be ducky. */

  PrdctHdr = *(struct _lang_hdr *) Ptr;  /* TJW 960706 struct assignment */
  PrdctTbl = (char **) (Ptr + sizeof (struct _lang_hdr));

  memory = (char *) PrdctTbl + (sizeof (char *) * PrdctHdr.ElemCnt);

  memory += strlen (memory) + 1;

  for (i = 1; i < PrdctHdr.ElemCnt; i++)
    PrdctTbl[i] = memory + (int) (PrdctTbl[i] - PrdctTbl[0]);

  PrdctTbl[0] = memory;

  Ptr += sizeof (struct _lang_hdr);
  Ptr += sizeof (char *) * (PrdctHdr.ElemCnt);

  Ptr += PrdctHdr.PoolSize;

  /* Finally set up the ANSI output mapping table.
   * This one is easy. Just point at it. */

  AnsiHdr = *(struct _lang_hdr *) Ptr;  /* TJW 960706 changed to struct assignment */
  AnsiTbl = (char *) (Ptr + sizeof (struct _lang_hdr));

  return (1);
}

/* $Id: langload.c,v 1.8 1999/09/27 17:40:18 ceh Exp $ */
