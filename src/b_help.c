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
 * Filename    : $Source: E:/cvs/btxe/src/b_help.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:11 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm "HELP" Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
main_help ()
{
  char *c;
  int i;

  screen_clear ();
  for (i = 0; i <= 10; i++)
    scr_printf (MSG_TXT (M_MAIN_HELP + i));

  for (i = 0; i < 9; i++)
  {
    if (keys[i] != NULL)
    {
      c = keys[i];
      while (*c && (*c != '\n'))
      {
        if (*c == '\r')
          *c = '|';
        ++c;
      }

      sprintf (e_input, MSG_TXT (M_MACRO_HELP), i + 1, keys[i]);
      e_input[79] = '\0';
      scr_printf (e_input);

      c = keys[i];
      while (*c && (*c != '\n'))
      {
        if (*c == '|')
          *c = '\r';
        ++c;
      }
    }
  }

  scr_printf (MSG_TXT (M_PRESS_ANYKEY));
  FOSSIL_CHAR ();
  scr_printf ("\r\n");
}

void
mailer_help ()
{
  int i;

  screen_clear ();

  for (i = M_MAILER_HELP; i < M_BLANK_HELP; i++)
    scr_printf (MSG_TXT (i));

  for (i = M_MAILER_HELP_EXTRA; i <= M_MAIL_HELP_EXTRA_END; i++)
    if (!strstr (MSG_TXT (i), "Main Help reserved"))
      scr_printf (MSG_TXT (i));

  if (do_screen_blank)
  {
    scr_printf (MSG_TXT (M_BLANK_HELP));
  }

  if (BBSreader != NULL)
  {
    sprintf (e_input, MSG_TXT (M_EDITOR_HELP), BBSreader);
    scr_printf (e_input);
  }

  for (i = 0; i < 9; i++)
  {
    if (shells[i] != NULL)
    {
      sprintf (e_input, MSG_TXT (M_SHELL_HELP), i + 1, shells[i]);
      scr_printf (e_input);
    }
  }

  scr_printf (MSG_TXT (M_PRESS_ANYKEY));
  FOSSIL_CHAR ();
  scr_printf ("\r\n");
  screen_clear ();
  sb_dirty ();
  opening_banner ();
  mailer_banner ();
}

/* $Id: b_help.c,v 1.3 1999/03/22 03:47:11 mr Exp $ */
