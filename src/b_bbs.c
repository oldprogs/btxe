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
 * Filename    : $Source: E:/cvs/btxe/src/b_bbs.c,v $
 * Revision    : $Revision: 1.10 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 11:15:17 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm BBS / UUCP Session Initiator
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#ifdef NEED_OS2COMMS
#define  hcModem hfComHandle
#endif

void LOCALFUNC                  /* CEH 990107 */
createcmndstring (char *command, char *name, unsigned long baudrate,
                  int el, int cid)
{
  char temp[33];

  /* MR 961122 added addBINKpath */
  sprintf (command, "%s%s %lu %lu ", addBINKpath, name,
           baudrate, cur_baud.rate_value);

#ifndef __unix__
  strcat (command, ultoa ((unsigned long) hfComHandle, temp, 10));
#else
  strcat (command, port_device);
#endif

  strcat (command, " ");
  strcat (command, itoa (time_to_next (1), temp, 10));

  if (el != -1)
  {
    strcat (command, " ");
    strcat (command, itoa (el, temp, 10));
  }

  strcat (command, " ");
  strcat (command, mdm_reliable);

  /* SMS 980916 added CurrentCID */
  if (cid && CurrentCID)
  {
    strcat (command, " ");
    strcat (command, CurrentCID);
  }
}


void
BBSexit ()
{
  long retval;
  int xit = 0;
  unsigned long baudrate;
  char command[PATHLEN];
  FILE *bbsbatch = NULL;
  long t1, t2;

  if (BBSnote != NULL)
  {
    SendBanner ((char far *) "\r\n", 1);  /* Start with newline   */
    SendBanner ((char far *) BBSnote, 1);  /* Tell user we're going */
    SendBanner ((char far *) "\r\n", 1);  /* End it with a newline */
  }

  t1 = timerset (100);
  t2 = timerset (400);

  /* Wait for output complete, 4 seconds, or carrier loss */
  while (!OUT_EMPTY () && !timeup (t2) && CARRIER)
    time_release ();

  /* Make sure we waited at least 1 second */
  while (CARRIER && !timeup (t1))
    time_release ();

  /* Now if there is no carrier bag it */
  if (!CARRIER)
  {
    status_line (MSG_TXT (M_NO_CARRIER));
    return;
  }

  ++hist.bbs_calls;

  // MR 970502 do not update because css is already shown!
  // do_today(0);       /* TJW 960903 update / don't show bmf counters */

  if ((lock_baud && (pbtypes[baud].rate_value >= lock_baud))
      || (ARQ_lock != 0))
    baudrate = max_baud.rate_value;
  else
    baudrate = cur_baud.rate_value;

  if (BBSopt == OPT_BATCH)      /* if BBS flag is batch *//* MR 961122 */
  {
    /* TJW 960722 BBSBATCH -> BBSBAT## */
    /* MR 961122 added addBINKpath */

    sprintf (command, "%s%sbbsbat%02x%s",
             addBINKpath, COMFILE_PREFIX, TaskNumber, COMFILE_SUFFIX);

    unlink (command);           /* Delete old copies        */

    /* Try to open the file */
    bbsbatch = fopen (command, write_ascii);
    if (bbsbatch == NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), command);
      return;
    }

    errno = 0;
    createcmndstring (command, "spawnbbs", baudrate, -1, 0);
    fprintf (bbsbatch, "%s\n", command);
    fclose (bbsbatch);
    xit = 1;
  }

#ifdef BINKLEY_SOUNDS
  Make_Sound (fnBBSSound);
#endif

  if ((BBSopt == OPT_EXIT) || xit)  /* exit or batch? *//* MR 961122 */
  {
    /* MR 961123 use const errorlvl with BBS Batch to make the batch easier */
    if (xit)
      retval = 101;
    else
      retval = baudrate / 100L;

    status_line (MSG_TXT (M_BBS_EXIT), (int) retval);

    if (vfossil_installed)
      vfossil_close ();

    if (!share)                 /* Turn off FOSSIL,         */
      MDM_DISABLE ();           /* if need be               */

    write_sched ();
    no_longer_alive ();         /* MR 961123                */
    exit ((int) retval);        /* and exit with error      */
  }
  else if (BBSopt == OPT_SPAWN) /* if BBS flag is spawn *//* MR 961122 */
  {
    status_line (MSG_TXT (M_BBS_SPAWN));
    gotoxy (0, SB_ROWS - 1);    /* TJW 960517 */
    cputs ("\n");               /* TJW 960517 */
    close_up (share);
    createcmndstring (command, "spawnbbs", baudrate, -1, 1);
    b_spawn (command);          /* Run the BBS */
    come_back ();
    status_line (MSG_TXT (M_BBS_RETURN));
  }
#ifdef BBS_INTERNAL             /* EH */
  else if (BBSopt == OPT_INTERNAL)  /* if BBS flag is internal */
  {
    css_done ();
    status_line (":Calling internal BBS");
    bbsinternal (0);
    status_line (MSG_TXT (M_BBS_RETURN));
    css_init ();
  }
#endif
}

void
close_up (int share_com)
{
  // HJK 98/06/13 - Problems should be fixed by new NTCOMM.DLL version
  // and shareport can now be used in Windows
  // #ifdef _WIN32               /* AS, 970623, ugly hack, but necessary to fix a BIG memory leak (probably caused by cominit/deinit code in NTCOMM) */
  //        COMPAUSE (hcModem);
  // #else
  if (!share_com)
    MDM_DISABLE ();
#if defined(OS_2) | defined(_WIN32)
  else
    ComPause (hcModem);         /* AG 990124 standard function case */
#endif
  // #endif

  vfossil_close ();

#ifdef SET_CONMODE
  setmode (fileno (stdin), O_TEXT);
  setmode (fileno (stdout), O_TEXT);
#endif
}

void
come_back ()
{
  /* Re-enable ourselves */
  if (Cominit (port_ptr, buftmo) != 0x1954)
  {
    printf ("\n%s\n", MSG_TXT (M_FOSSIL_GONE));
    exit (1);
  }

#if defined(OS_2) | defined(_WIN32)
  ComResume (hcModem);          /* AG 990124 standard function case */
#endif

  program_baud ();
  vfossil_init ();

#ifdef SET_CONMODE
  setmode (fileno (stdin), O_BINARY);
  setmode (fileno (stdout), O_BINARY);
#endif

  if (un_attended)
  {
    vfossil_cursor (0);
    screen_clear ();
    sb_dirty ();
    // opening_banner ();  /* MR 970803 removed, because clears screen in terminal mode */
    // mailer_banner ();   /* MR 970803 removed, because sets (wrong) baud rate
    sb_show ();
  }
}

void
UUCPexit (int errlevel, int fCheckCarrier)
{
  unsigned long baudrate;
  char command[PATHLEN];
  FILE *mailbatch;
  long t1, t2;

  /* We check carrier for all applications where we would want to send
     MAILnote. Good thing. SendBanner is highly sensitive to carrier. */

  if (MAILnote != NULL && fCheckCarrier)
  {
    SendBanner ((char far *) "\r\n", 1);  /* Start with newline   */
    SendBanner ((char far *) MAILnote, 1);  /* Say it's mail time */
    SendBanner ((char far *) "\r\n", 1);  /* End with a newline   */
  }

  t1 = timerset (PER_SECOND);
  t2 = timerset (4 * PER_SECOND);

  /* Wait for output complete, 4 seconds, or carrier loss */
  while (!OUT_EMPTY () && !timeup (t2) && (!fCheckCarrier || CARRIER))
    time_release ();

  /* Make sure we waited at least 1 second */
  while ((!fCheckCarrier || CARRIER) && !timeup (t1))
    time_release ();

  /* Now if there is no carrier bag it */
  if (fCheckCarrier && !CARRIER)
  {
    status_line (MSG_TXT (M_NO_CARRIER));
    return;
  }

  if ((lock_baud && (pbtypes[baud].rate_value >= lock_baud))
      || (ARQ_lock != 0))
    baudrate = max_baud.rate_value;
  else
    baudrate = cur_baud.rate_value;

#ifdef BINKLEY_SOUNDS
  Make_Sound (fnEXTSound);
#endif

  if (EXTopt == OPT_SPAWN)      /* opt=spawn *//* MR 961122 */
  {
    status_line (MSG_TXT (M_EXT_SPAWN));
    gotoxy (0, SB_ROWS - 1);    /* TJW 960517 */
    cputs ("\n");               /* TJW 960517 */
    close_up (share);
    createcmndstring (command, "extmail", baudrate, errlevel, 0);
    b_spawn (command);          /* Run the External Program */
    come_back ();
    status_line (MSG_TXT (M_EXT_RETURN));
    return;
  }

  strcpy (command, addBINKpath);  /* MR 961122 added addBINKpath */
  strcat (command, COMFILE_PREFIX);  /* Start of batch filename  */
  strcat (command, "mailbat");  /* Meat of Batch filename   */
  strcat (command, COMFILE_SUFFIX);  /* End of batch filename    */
  unlink (command);             /* Delete old copies    */

  /* Try to open the file */
  mailbatch = fopen (command, append_ascii);
  if (mailbatch == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), command);
    return;
  }

  createcmndstring (command, "extmail", baudrate, errlevel, 1);
  fprintf (mailbatch, "%s\n", command);
  fclose (mailbatch);
  status_line (MSG_TXT (M_EXT_MAIL), errlevel);

  if (vfossil_installed)
    vfossil_close ();

  if (!share)                   /* Turn off FOSSIL,         */
    MDM_DISABLE ();             /* if need be               */

  write_sched ();               /* Save scheduler info      */
  no_longer_alive ();           /* MR 961123                */
  exit (errlevel);              /* and exit with UUCP error */
}

/* $Id: b_bbs.c,v 1.10 1999/03/23 11:15:17 hjk Exp $ */
