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
 * Filename    : $Source: E:/cvs/btxe/src/b_wzsend.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:21 $
 * State       : $State: Exp $
 * Orig. Author: Wynn Wagner III
 *
 * Description : BinkleyTerm WaZOO/EMSI Session Logic
 *
 * Note        :
 *   This module is based largely on a similar module in OPUS-CBCS V1.03b.
 *   The original work is (C) Copyright 1987, Wynn Wagner III. The original
 *   author has graciously allowed us to use his code in this work.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

int WaZOO_callback (char *);
int WaZOO_time (long);

static void LOCALFUNC InitExtAgent (void);
static void LOCALFUNC DeinitExtAgent (void);
int EXTMAIL_callback (char *);

/*--------------------------------------------------------------------------*/
/* SEND WaZOO (send another WaZOO or EMSI-capable system its mail)          */
/*--------------------------------------------------------------------------*/

MISSION_ERR
send_WaZOO (short session)
{
  char *HoldName;
  char fname[PATHLEN];
  char fname2[PATHLEN];
  char s[PATHLEN];
  struct stat buf;
  ADDR saved_addr;
  ADDRP addrp = &called_addr;
  int akas;
  int c;
  int check_again;
  int i;
  int z;
  long DontPick;
  long OkToRequest;
  MISSION_ERR error = mission_ok;

  /* !!!TJW added: fsent = 0 */
  /* r.hoerner: thomas: this is unnecessary: b_sessio.c already initializes
   *                    fsent to 0 at the start of a session.
   *                    see note in ..\doc\fsent.doc */

  /*r. hoerner */
  DontPick = QueryNodeFlag (no_pickup, NOPICK, &called_addr);
  OkToRequest = !DontPick && requests_ok && pickup_ok;
  /* cfg-flag + --- event def ----  */

  session_method = session;

  switch (remote_pickup)
  {
  case -2:
    InitExtAgent ();            /* External Mail session */
    /* ExtMail is handled like WaZOO. Fall through to that case. */

  case -1:
    akas = 1;                   /* WaZOO Session */
    remote_akas[0] = called_addr;
    break;

  case 0:
    akas = 0;                   /* remote has nopickup flag */
    break;

  case 1:
    akas = 1;
    break;

  case 2:
    akas = num_rakas;
    break;

  default:
    akas = 0;
    break;
  }

  saved_addr = called_addr;


#ifdef HAVE_HYDRA
  if (session == HYDRA_SESSION)
  {
    hydra_init (hydra_options);
  }
#endif

  for (i = 0; i < akas; i++)
  {
    addrp = &remote_akas[i];
    HoldName = HoldAreaNameMunge (addrp);
    status_line (MSG_TXT (M_EMSI_PROC_NODE), Full_Addr_Str (addrp), HoldName);
    check_again = 0;

    /* -------------------------------------------------------------------- */
    /* TJW 960528 moved sending out of FREQ to beginning of send batch      */
    /* to make xHydra easier to implement                                   */
    /* -------------------------------------------------------------------- */
    /* Send our File requests to other system                               */
    /* -------------------------------------------------------------------- */
    if (OkToRequest)            /* if I may request            */
    {
      sprintf (fname, "%s%s.req", HoldName, Hex_Addr_Str (addrp));
      sprintf (fname2, "%s%s.rqf", HoldName, Hex_Addr_Str (addrp));
      if (!unix_stat (fname, &buf))  /* and if I have some requests */
      {                         /* but other side does not allow requests */
        if (!(remote_capabilities & WZ_FREQ))
        {                       /* then give up. It is useless */
          status_line (MSG_TXT (M_FREQ_DECLINED));
          break;
        }
        else
        {
          status_line (MSG_TXT (M_OUT_REQUESTS));
          cleanup_req (fname);  /* TJW 960819 remove size info in REQ */
          ++made_request;
          z = FILE_SKIPPED;

#ifdef HAVE_HYDRA
          if (session == HYDRA_SESSION)
          {
            z = hydra (fname, NULL);
          }
          else
#endif /* HAVE_HYDRA */
          {
            z = Send_Zmodem (fname, NULL, zmodem_state++, DO_WAZOO);
          }

          if (z == CANNOT_SEND) /* comm problems. Abort */
          {
            status_line (">Net_Problems in %s-Session",
                         session == HYDRA_SESSION ? "Hydra" : "Zmodem");
            error = mission_error;
            goto abort;
          }

          if (z == FILE_SENT_OK)
            fsent++;
          rename (fname, fname2);

        }                       /* remote allows requests */
      }                         /* REQ found */
    }                           /* OkToRequest   */
  }                             /* for all his akas  */

#ifdef HAVE_HYDRA
  if ((session == HYDRA_SESSION) && do_RH1hydra)
  {
    if (hydra (NULL, NULL) == CANNOT_SEND)  /* RH1hydra: End of first batch */
      error = mission_error;
    SyncHydraSession = 0;       /* no sync in first HYDRA batch */
  }
#endif /* HAVE_HYDRA */

  if (remote_pickup)            /* if remote has not "NPU" flag set */
    for (i = 0; i < akas; i++)
    {
      addrp = &remote_akas[i];
      HoldName = HoldAreaNameMunge (addrp);
      status_line (MSG_TXT (M_EMSI_PROC_NODE), Full_Addr_Str (addrp), HoldName);
      check_again = 0;

    just_make_sure:

      /* ------------------------------------------------------------------ */
      /* Send all waiting ?UT files (mail packets)                          */
      /* ------------------------------------------------------------------ */
      *ext_flags = 'o';
      for (c = 0; c < NUM_FLAGS; c++)
      {
        if ((caller && !SendHoldIfCaller) && (ext_flags[c] == 'h'))  /* VRP 990824 */
          continue;

        sprintf (fname, "%s%s.%cut", HoldName, Hex_Addr_Str (addrp),
                 ext_flags[c]);

        if (!unix_stat (fname, &buf))
        {
          /* --- Build a dummy PKT file name --- */
          invent_pkt_name (s);

          status_line (MSG_TXT (M_PACKET_MSG));

          z = FILE_SKIPPED;

          /* Do this using the external mail agent if need be */
          if (remote_pickup == -2)
          {
            z = EXTMAIL_callback (fname);
          }
          else
#ifdef HAVE_HYDRA
          if (session == HYDRA_SESSION)
          {
            z = hydra (fname, s);
          }
          else
#endif /* HAVE_HYDRA */
          {
            z = Send_Zmodem (fname, s, zmodem_state++, DO_WAZOO);
          }

          if (z == CANNOT_SEND) /* comm problems: abort */
          {
            status_line (">Net_Problems in %s-Session",
                         session == HYDRA_SESSION ? "Hydra" : "Zmodem");
            error = mission_error;
            goto abort;
          }

          if (z == FILE_SENT_OK)  /* else: skipped or not found   */
          {
            fsent++;
            unlink (fname);     /* only delete SENT mail/files */
          }

          CLEAR_IOERR ();

        }                       /* mailpkt found  */
      }                         /* for each mail flag */

      /* ------------------------------------------------------------------ */
      /* Send files listed in ?LO files (attached files)                    */
      /* ------------------------------------------------------------------ */
      *ext_flags = 'f';
      called_addr = *addrp;

      if (!bttask)
        if (!(fsent && SyncHydraSession))
        {
          if (!do_FLOfile (ext_flags, WaZOO_callback))
          {
            called_addr = saved_addr;
            error = mission_error;
            goto abort;
          }
        }

      called_addr = saved_addr;
      if (fsent && !check_again)
      {
        check_again++;
        goto just_make_sure;
      }

    }                           /* for all of his akas */

  status_line (">WaZoo BATCH 2 ended");

  /* END of active SENDING loop                */
  /* This is as far as we go for external mail */

  if (remote_pickup == -2)
  {
    DeinitExtAgent ();
    goto success;
  }

#ifdef HAVE_HYDRA
  if ((session == HYDRA_SESSION) && !do_RH1hydra)
  {
    if (hydra (NULL, NULL) == CANNOT_SEND)  /* !do_RH1hydra: End of first batch */
      error = mission_error;
  }
#endif /* HAVE_HYDRA */

  if (!bttask)
    if (!(fsent && SyncHydraSession))
    {
      fsent += respond_to_file_requests (0, WaZOO_callback, WaZOO_time);
    }

abort:                         /* jump here in case of trouble and de-init your protocols! */

#ifdef HAVE_HYDRA
  if (session == HYDRA_SESSION)
  {
    if (hydra (NULL, NULL) == CANNOT_SEND)
      error = mission_error;
    hydra_deinit ();
  }
  else
#endif /* HAVE_HYDRA */
  {
    int zmodem;

    zmodem = fsent ? ZMODEM_ENDBATCH : ZMODEM_END;
    z = Send_Zmodem (NULL, NULL, zmodem, DO_WAZOO);
    zmodem_state = ZMODEM_INIT;
  }

success:

  if (error == mission_ok)
  {
    /* MR 970810 only tell "nothing to send" if session was ok */
    if (!fsent)                 /* TJW 960429 after idea of Andrew Clarke vvvvvvvvvvvv */
      status_line (MSG_TXT (M_NOTHING_TO_SEND), Full_Addr_Str (&called_addr));

    sent_mail = 1;              /* MR 970803 only set it after successful session */
    status_line (">WaZoo END, no problems");
  }
  else
    status_line (">WaZoo END, error(s)");

  return error;
}                               /* WaZOO */


/*
 * WaZOO_callback () -- send requested file using WaZOO/EMSI method.
 */

int
WaZOO_callback (char *reqs)
{
  if (remote_pickup == -2)
    return EXTMAIL_callback (reqs);

#ifdef HAVE_HYDRA
  if (session_method == HYDRA_SESSION)
    return (hydra (reqs, NULL));
#endif /* HAVE_HYDRA */

  return (Send_Zmodem (reqs, NULL, zmodem_state++, DO_WAZOO));
}


int
WaZOO_time (long filesize)
{
  long ltemp;

#ifdef HAVE_HYDRA
  if ((session_method == HYDRA_SESSION) && (remote_pickup != -2))
    ltemp = remaining (filesize, 95L);
  else
#endif /* HAVE_HYDRA */
    ltemp = remaining (filesize, 100L);

  return (ltemp < 20L) ? 20 : (int) ltemp;
}


static void LOCALFUNC
InitExtAgent ()
{
  status_line (MSG_TXT (M_START_EXTMAIL), Full_Addr_Str (&remote_addr));
  modem_busy ();
  vfossil_cursor (1);
  close_up (share);
}


int
EXTMAIL_callback (char *reqs)
{
  unsigned char *p = bufp->extmail._p;  /* TJW 960416 */
  unsigned char *q = bufp->extmail._q;  /* TJW 960416 */

  struct stat f;
  long ltime;

  ltime = (long) unix_time (NULL);

  /* VRP 990915: changed external session success flag check.
   * External session success is now determined by the presense of
   * esess_ok.%task% flag in the flag dir.
   *
   * Was: sprintf ((char *) q, "%s.%03x", ExtMailAgent, TaskNumber);
   */

  sprintf ((char *) q, "%sesess_ok.%03x", flag_dir, TaskNumber);

  /* VRP 990915 end */

  unlink ((char *) q);

  unix_stat (reqs, &f);

  sprintf ((char *) p, "%s %s %03x %s", ExtMailAgent,
           Full_Addr_Str (&remote_addr), TaskNumber, reqs);

  status_line (">run %s", (char *) p);

  b_spawn ((char *) p);

  if (!dexists ((char *) q))
  {
    status_line (">%s not found", (char *) q);  /* VRP 990915 */
    DeinitExtAgent ();
    return CANNOT_SEND;
  }

  status_line (">%s found", (char *) q);  /* VRP 990915 */
  hist.files_out++;
  hist.size_out += f.st_size;
  hist.time_out += (long) unix_time (NULL) - ltime;

  return FILE_SENT_OK;
}


static void LOCALFUNC
DeinitExtAgent ()
{
  come_back ();
  modem_init ();
  status_line (MSG_TXT (M_END_EXTMAIL), Full_Addr_Str (&remote_addr));
}

/* $Id: b_wzsend.c,v 1.7 1999/09/27 20:51:21 mr Exp $ */
