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
 * Filename    : $Source: E:/cvs/btxe/src/b_proto.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:13 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm External Protocol Handler Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
do_extern (char *cmd, int prot, char *name)
{
  int j;
  struct baud_str *i;
  char rgchT[100];
  char *c, *p, *m;
  FILE *ctlfile;

  memset (rgchT, 0, sizeof (rgchT));

  for (j = 0; j < 5; j++)
  {
    char ch;                    /* AW 980521 don't "optimize" here   */

    ch = protos[j].first_char;  /* otherwise Watcom compiler crashes */
    if (ch == (char) prot)
      break;
  }

  j = protos[j].entry;
  c = m = protocols[j];
  p = NULL;
  while (*m)                    /* Until end of string  */
  {
    if ((*m == DIR_SEPC) || (*m == ':'))  /* Look for last path */
      p = m;                    /* Delimiter            */
    m++;
  }
  m = &rgchT[0];

  /* It doesn't pay to be too smart here. Dots can appear in dir names */

  while (c != p)                /* Copy to last '\'     */
    *m++ = *c++;
  while (*c != '.')             /* Then to the dot      */
    *m++ = *c++;
  strcat (&rgchT[0], ".ctl");   /* Then add extension   */

  /*
   * At this point we have the Control File name in (rgchT), Now let's open
   * the file and put our good stuff in there.
   */

  unlink (rgchT);               /* Delete old copies    */
  if ((ctlfile = fopen (rgchT, append_ascii)) == NULL)  /* Try to open it  */
  {
    status_line (MSG_TXT (M_NO_CTL_FILE), rgchT);
    return;
  }

  fprintf (ctlfile, "Port %d\n", (port_ptr + 1));  /* Port n       */
  if (lock_baud && (cur_baud.rate_value >= lock_baud))
    i = &max_baud;
  else
    i = &pbtypes[baud];

#ifndef __unix__
  fprintf (ctlfile, "Modem %lx %lx %x %x %lx\n",  /* All modem params */
           (unsigned long) hfComHandle, i->rate_value, handshake_mask,
           carrier_mask, i->rate_mask);  /*  TS/TJW 960622 */
#else
  fprintf (ctlfile, "Modem %s %lx %x %x %lx\n",  /* All modem params */
           port_device, i->rate_value, handshake_mask,
           carrier_mask, i->rate_mask);  /*  TS/TJW 960622 */
#endif

  fprintf (ctlfile, "Baud %lu\n", i->rate_value);  /* Baud Rate    */
  fprintf (ctlfile, "%s %s\n", cmd, name);  /* Actual command   */
  fclose (ctlfile);

  /* The file is now written. Turn off everything. */

  close_up (share);

  /* We're all set. Build the command and execute it. */

  c = protocols[j];

#ifndef __unix__
  sprintf (e_input, "%s %s -p%lu -b%lu %s",
           c, c, (unsigned long) hfComHandle,
           i->rate_value, rgchT);  /*  TS/TJW 960622 */
#else
  sprintf (e_input, "%s %s -p%s -b%lu %s",
           c, c, port_device,
           i->rate_value, rgchT);  /*  TS/TJW 960622 */
#endif

  b_spawn (e_input);            /* Execute command  */

  /* Back from external protocol. Turn it all back on. */

  come_back ();
  XON_ENABLE ();                /* and reenable XON */
}

/* $Id: b_proto.c,v 1.5 1999/03/22 03:47:13 mr Exp $ */
