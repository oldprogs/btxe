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
 * Filename    : $Source: E:/cvs/btxe/src/b_termov.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:17 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Terminal Overlay
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

void
ansi_map (unsigned ScanVal)
{
  unsigned KeyCode;

  int i;
  int Len = 0;
  char *Ptr = AnsiTbl;

  if (AnsiHdr.ElemCnt == 0)
    return;

  for (i = 0; i < AnsiHdr.ElemCnt; i++)
  {
    KeyCode = *((unsigned int *) Ptr);
    Ptr += sizeof (unsigned int);

    Len = (int) *Ptr++;

    if (ScanVal == KeyCode)
      break;

    Ptr += Len;
  }

  if (i == AnsiHdr.ElemCnt)
    return;

  SENDCHARS (Ptr, Len, 0);      /* Send the mapped string   */
}

int
term_overlay (short k)
{
  ADDR addr;
  ADDRP addrp;
  int c;
  char junk[256];

  switch ((unsigned short) k)
  {
  case F_TERM_SETBAUD:
    ++baud;
    if (!pbtypes[baud].rate_value)
      baud = 0;
    program_baud ();
    cur_baud = pbtypes[baud];
    scr_printf (MSG_TXT (M_BINK_NOW_AT));
    sprintf (junk, "%lu baud.\r\n", pbtypes[baud].rate_value);
    scr_printf (junk);
    break;

  case F_TERM_COMMCONFIG:
    scr_printf ("\r\n7 or 8 data bits? ");
    fgets (junk, 100, stdin);
    comm_bits = (atoi (junk) == 7) ? BITS_7 : BITS_8;

    if (comm_bits == BITS_8)
    {
      parity = NO_PARITY;
    }
    else
    {
      scr_printf ("(E)ven, (O)dd, or (N)o parity? ");
      fgets (junk, 100, stdin);
      strupr (junk);
      parity = (junk[0] == 'E') ? EVEN_PARITY :
        ((junk[0] == 'O') ? ODD_PARITY : NO_PARITY);
    }

    scr_printf ("1 or 2 stop bits? ");
    fgets (junk, 100, stdin);
    stop_bits = (atoi (junk) == 1) ? STOP_1 : STOP_2;

    program_baud ();
    break;

  case F_TERM_DIALOUT:
    scr_printf (MSG_TXT (M_PHONE_OR_NODE));

    if (!get_number (junk))
    {
      sprintf (junk, "\r\n%s\r\n", MSG_TXT (M_NODE_NOT_FOUND));
      scr_printf (junk);
      break;
    }

    addrp = NULL;

    if (strchr (junk, '/') != NULL)
    {
      if (find_address (junk, &addr))
        addrp = &addr;

      if (nodeproc (junk))
      {
        do_dial_strings (1);
        sprintf (junk, "\r\n%s, %s, %s.\r\n%s", lastfound_SysopName,
                 newnodedes.SystemName, newnodedes.PhoneNumber,
                 MSG_TXT (M_ARE_YOU_SURE));
        scr_printf (junk);
        fgets (junk, 100, stdin);
        scr_printf ("\r\n");
        if (toupper (junk[0]) != (char) toupper (MSG_TXT (M_YES)[0]))
          break;

        strcpy (junk, (char *) (newnodedes.PhoneNumber));
      }
      else
        break;
    }

    try_2_connect (junk, addrp);  /* try to connect  *//* SM 961110 */
    gong ();
    break;

  case F_TERM_CLEARSCREEN:
    screen_clear ();
    break;

  case F_TERM_HANGUP:
    modem_hangup ();
    status_line (MSG_TXT (M_END_OF_CONNECT));
    scr_printf ("\r\n");
    RAISE_DTR ();
    break;

  case F_TERM_REINITMODEM:
    modem_init ();
    break;

  case F_TERM_SHELL:
    modem_busy ();
    close_up (0);
    scr_printf ("\r\n");
    scr_printf (&(MSG_TXT (M_SHELLING)[1]));
    scr_printf (MSG_TXT (M_TYPE_EXIT));
    change_prompt ();
    b_spawn (NULL);
    come_back ();
    modem_init ();
    scr_printf (MSG_TXT (M_WELCOME_BACK));
    break;

  case F_TERM_CHANGEPORT:
    c = port_ptr;
    if (++port_ptr == MAXport)
      port_ptr = 0;

    MDM_DISABLE ();
    if (Cominit (port_ptr, buftmo) != 0x1954)
    {
      char junk[100];

#ifdef DOS16
      sprintf (junk, MSG_TXT (M_DRIVER_DEAD_1), "COM", 1 + port_ptr);
#else
      sprintf (junk, MSG_TXT (M_DRIVER_DEAD_1), port_device, 1 + port_ptr);
#endif
      scr_printf (junk);
      scr_printf (MSG_TXT (M_DRIVER_DEAD_2));
      scr_printf (MSG_TXT (M_DRIVER_DEAD_3));
      port_ptr = c;
      Cominit (port_ptr, buftmo);
    }

    program_baud ();
    RAISE_DTR ();
    scr_printf (MSG_TXT (M_BINK_NOW_USING));
    WRITE_ANSI (' ');
    scr_printf (port_device);
    WRITE_ANSI ((char) (port_ptr + '1'));
    scr_printf (":\r\n");
    XON_ENABLE ();
    break;

  case F_TERM_DOBREAK:
    scr_printf (MSG_TXT (M_SENDING_BREAK));
    send_break (100);
    scr_printf (MSG_TXT (M_DONE));
    break;

  case F_TERM_VERSION:
    scr_printf (MSG_TXT (M_YOU_ARE_USING));
    scr_printf (ANNOUNCE);
    scr_printf ("\r\n");
    scr_printf ("Compiled ");
    scr_printf (compiledatetime);
    scr_printf (".\r\n\r\n");
    break;

  case F_TERM_MACRO1:
  case F_TERM_MACRO2:
  case F_TERM_MACRO3:
  case F_TERM_MACRO4:
  case F_TERM_MACRO5:
  case F_TERM_MACRO6:
  case F_TERM_MACRO7:
  case F_TERM_MACRO8:
  case F_TERM_MACRO9:
    c = (int) ((unsigned short) k - (unsigned short) F_TERM_MACRO1);
    if (keys[c] != NULL)
    {
      SENDCHARS (keys[c], strlen (keys[c]), 0);
    }
    break;

  default:
    return (1);
  }

  return (0);
}

/* $Id: b_termov.c,v 1.5 1999/03/22 03:47:17 mr Exp $ */
