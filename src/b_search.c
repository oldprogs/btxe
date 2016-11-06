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
 * Filename    : $Source: E:/cvs/btxe/src/b_search.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:15 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Phone list Search Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static int LOCALFUNC SaveScanList (int);
static void LOCALFUNC wait_for_keypress (void);

int
list_search ()
{
  int saved_baud;
  long t1;
  int i, k, l;
  int dirty;
  ADDR addr;
  ADDRP addrp;
  unsigned int kbd_input;
  char junk[256];

  static unsigned int save_chars[] =
  {
    SHF1, SHF2, SHF3, SHF4, SHF5,
    SHF6, SHF7, SHF8, SHF9, SHF10
  };

  static unsigned int load_chars[] =
  {
    ALTF1, ALTF2, ALTF3, ALTF4, ALTF5,
    ALTF6, ALTF7, ALTF8, ALTF9, ALTF10
  };

  static unsigned int command_chars[] =
  {
    PF1, PF2, PF3, PF4, PF5,
    PF6, PF7, PF8, PF9, PF10
  };

  /* Input the phone numbers we want to scan */

  dirty = 1;

  /* Print out a null string in a nice way for unforgiving OS. */

#define NICE_TEXT(n) (((n) == NULL) ? "(none)" : n)

  for (;;)
  {
    if (dirty)
    {
      screen_clear ();
      printf (MSG_TXT (M_PHONE_HELP));
      printf (MSG_TXT (M_PHONE_HELP2));
      if (set_loaded)
        printf (MSG_TXT (M_LAST_SET), set_loaded);
      printf (MSG_TXT (M_CURRENT_PHONES));
      for (k = 0; k < 10; k += 2)
      {
        printf ("%2d: %35s %2d: %35s\n", k + 1,
                NICE_TEXT (scan_list[k]), k + 2,
                NICE_TEXT (scan_list[k + 1]));
      }
      printf (MSG_TXT (M_INPUT_COMMAND));
      dirty = 0;
    }

    while (!KEYPRESS ())
      time_release ();
    kbd_input = FOSSIL_CHAR ();

    if (((kbd_input & 0xff) == '\n') || ((kbd_input & 0xff) == '\r'))
      break;

    if ((kbd_input & 0xff) == ESC)
      return (0);

    for (k = 0; k < 10; k++)
    {
      if (kbd_input == save_chars[k])  /* Save into a set?     */
      {
        SaveScanList (k);       /* Yes, do it           */
        dirty = 1;              /* Force redisplay      */
        k = 10;                 /* Then fool the logic  */
        break;
      }

      if (kbd_input == load_chars[k])  /* Load from a set?     */
      {
        LoadScanList (k, 1);    /* Yes, do it           */
        dirty = 1;              /* Force redisplay      */
        k = 10;                 /* Then fool the logic  */
        break;
      }

      if (kbd_input == command_chars[k])  /* Plain old Fkey?      */
        break;                  /* Yup, get out now     */
    }

    if (k == 10)                /* Not a function key   */
    {
      k = (kbd_input & 0xff) - '0';  /* Convert from digit   */
      if ((k < 0) || (k > 9))   /* Was it a digit?      */
        continue;               /* No, throw char out   */
      if (!k)                   /* Is it a zero?        */
        k = 9;                  /* That's 9 to us       */
      else
        --k;                    /* Else make zero-rel   */
    }

    printf (MSG_TXT (M_ELEMENT_CHOSEN), k + 1);
    if (scan_list[k] != NULL)
    {
      printf (MSG_TXT (M_CURRENTLY_CONTAINS), scan_list[k]);
      printf (MSG_TXT (M_PHONE_HELP3));
    }
    else
    {
      printf (MSG_TXT (M_PHONE_HELP4));
    }
    // gets (junk);                         /* Get user's input     */
    // CEH 990104: this was not safe
    fgets (junk, sizeof (junk), stdin);
    ++dirty;                    /* Always redisplay     */
    if ((l = (int) strlen (junk)) == 0)  /* If nothing there,    */
      continue;                 /* move along           */

    if (l == 1 && *junk == ' ') /* If just a space...   */
    {
      if (scan_list[k] != NULL) /* Delete old number    */
        free (scan_list[k]);
      scan_list[k] = NULL;      /* Clean up the ref     */
      continue;                 /* End this iteration   */
    }

    /* Get rid of old num if any */

    if (scan_list[k] != NULL)
    {
      free (scan_list[k]);
    }

    /* Allocate space for new num */

    if ((scan_list[k] = calloc (1, (unsigned int) (++l))) == NULL)
    {
      printf (MSG_TXT (M_MEM_ERROR));
      return (0);               /* Get out for error    */
    }
    strcpy (scan_list[k], junk);  /* Save new number      */
  }

  /* Actual Search logic */

  status_line (MSG_TXT (M_STARTING_SCAN));
  for (;;)
  {
    l = 0;
    for (k = 0; k < 10; k++)
    {
      if (scan_list[k] == NULL)
        continue;
      addrp = NULL;
      strcpy (junk, scan_list[k]);
      if (!isdigit (junk[0]) && junk[0] != '\"')
      {
        (*userfunc) (junk, &addr);
        if ((addr.Net != 0xffff)
            && (addr.Node != 0xffff)
            && (addr.Zone != 0xffff))
        {
          sprintf (junk, "%s", Full_Addr_Str (&addr));
          addrp = &addr;
        }
        else
          continue;
      }
      if (strchr (junk, '/') != NULL)
      {
        char *c;

        c = skip_blanks (junk);
        if (find_address (c, &addr))
          addrp = &addr;

        if (!nodeproc (junk))
          break;

        status_line (MSG_TXT (M_PROCESSING_NODE),
                     Full_Addr_Str (&addr),
                     newnodedes.SystemName);

        strcpy (junk, (char *) (newnodedes.PhoneNumber));
      }
      saved_baud = baud;

      if (try_1_connect (junk, addrp))  /* Attempt to connect */
      {
        status_line (MSG_TXT (M_CONNECTED_TO_ITEM), k + 1);
        free (scan_list[k]);
        scan_list[k] = NULL;
        gong ();
        return (1);
      }

      ++l;
      baud = saved_baud;
      program_baud ();
      cur_baud = pbtypes[baud];
      t1 = timerset (2 * PER_SECOND);

      while (!timeup (t1))      /* pause for 2 seconds  */
      {
        if (KEYPRESS ())
        {
          i = FOSSIL_CHAR () & 0xff;
          if (i == ESC)         /* Abort for ESCape     */
          {
            status_line (MSG_TXT (M_CONNECT_ABORTED));
            return (0);
          }
        }
        time_release ();
      }
    }

    if (!l)
      break;
  }

  return (0);
}

int
LoadScanList (int number, int report_errors)
{
  int k, l;
  FILE *ScanFile;
  char junk[256];

  sprintf (junk, "%sBinkScan.LS%c", BINKpath, number + '0');
  if ((ScanFile = fopen (junk, read_binary)) == NULL)
  {
    if (report_errors)
    {
      printf (MSG_TXT (M_UNABLE_TO_OPEN), junk);
      printf ("\n");
      wait_for_keypress ();
    }
    return (0);
  }

  for (k = 0; k < 10; k++)
  {
    if (fread (junk, 36, 1, ScanFile) != 1)
    {
      if (report_errors)
      {
        printf (MSG_TXT (M_SET_READ_ERROR), number + 1);
        wait_for_keypress ();
      }
      fclose (ScanFile);
      return (0);
    }

    if (scan_list[k] != NULL)
    {
      free (scan_list[k]);
      scan_list[k] = NULL;
    }

    l = (int) strlen (junk);
    if (l)
    {
      if ((scan_list[k] = calloc (1, (unsigned int) (++l))) == NULL)  /* Allocate space    */
      {
        if (report_errors)
        {
          printf (MSG_TXT (M_MEM_ERROR));
          wait_for_keypress ();
        }
        fclose (ScanFile);
        return (0);
      }

      strcpy (scan_list[k], junk);  /* Save new number   */
    }
  }

  l = fclose (ScanFile);
  if (report_errors)
  {
    if (l)
      printf (MSG_TXT (M_SET_CLOSE_ERR), number + 1);
    else
    {
      printf (MSG_TXT (M_SET_LOADED), number + 1);
      set_loaded = number + 1;
    }
    wait_for_keypress ();
  }

  return (l);
}

static int LOCALFUNC
SaveScanList (int number)
{
  int k, l;
  FILE *ScanFile;
  char junk[256];

  sprintf (junk, "%sBinkScan.LS%c", BINKpath, number + '0');
  if ((ScanFile = fopen (junk, write_binary)) == NULL)
  {
    printf (MSG_TXT (M_UNABLE_TO_OPEN), junk);
    printf ("\n");
    wait_for_keypress ();
    return (0);
  }

  for (k = 0; k < 10; k++)
  {
    for (l = 0; l < 36; l++)
      junk[l] = '\0';

    if (scan_list[k] != NULL)
      strcpy (junk, scan_list[k]);

    if (fwrite (junk, 36, 1, ScanFile) != 1)
    {
      printf (MSG_TXT (M_SET_WRITE_ERROR), number + 1);
      wait_for_keypress ();
      fclose (ScanFile);
      return (0);
    }
  }

  l = fclose (ScanFile);
  if (l)
    printf (MSG_TXT (M_SET_CLOSE_ERR), number + 1);
  else
  {
    printf (MSG_TXT (M_SET_SAVED), number + 1);
    set_loaded = number + 1;
  }

  wait_for_keypress ();
  return (l);
}

static void LOCALFUNC
wait_for_keypress (void)
{
  printf ("%s\n", MSG_TXT (M_PRESS_ENTER));
  while (!KEYPRESS ())
    time_release ();
  FOSSIL_CHAR ();
}

/* $Id: b_search.c,v 1.4 1999/03/22 03:47:15 mr Exp $ */
