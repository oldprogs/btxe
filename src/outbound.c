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
 * Filename    : $Source: E:/cvs/btxe/src/outbound.c,v $
 * Revision    : $Revision: 1.18 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:37 $
 * State       : $State: Exp $
 * Orig. Author:
 *   Vince Perriello, Bob Hartman, Holger Schurig, Michael Buenter and
 *   Doug Boone
 *
 * Description : BinkleyTerm Outbound Area Manipulation
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

extern char *mail_status_chars (unsigned int, char *msc);

/* local prototypes */

int mail_change (BINK_SAVEP, int);
int mail_delreq (BINK_SAVEP, int);
int mail_stop (BINK_SAVEP, int);
int mail_addr (BINK_SAVEP, int);
int Do_Info (BINK_SAVEP p, int x);
int Do_Edit (BINK_SAVEP p, int x);

// static int LOCALFUNC mail_addsend (ADDRP, char *, char *); *** VRP 990826
static void LOCALFUNC zoom_window (REGIONP, MAILP, MAILP);
static int LOCALFUNC mail_addrequest (ADDRP, char *, char *, char *);
static int LOCALFUNC kill_node (char *);
static void LOCALFUNC call_tries (ADDRP baddr, int *calls, int *badcalls);
static int LOCALFUNC get_NodeAdr (char *);
static int LOCALFUNC sb_input_address (REGIONP, short, short, char *, short);

/* local variables */
static int curl;
static MAILP oldp, oldcur;
static ADDR other_addr;
static REGIONP zoomwin;

#define ZHELPSIZE 9

char space66[] = "                                                                  ";
char space47[] = "                                              ";

int
Do_Zoom (BINK_SAVEP rp, int x)
{
  MAILP p;
  long t_idle;
  int i, c;
  int changed = 0;
  short z;

  x = x;

  /* is there a window */
  if (rp == NULL)
    return (0);

  zoomwin = rp->region;

  /* draw a box */
  sb_fill (zoomwin, ' ', colors.hold);
  sb_box (zoomwin, boxtype, colors.frames);

  /* give it a title */
  sb_caption (zoomwin, MSG_TXT (M_ZOOM_TITLE), colors.frames);

  /* clear the inside of zoomed outbound */
  zoomwin->sr0++;
  zoomwin->sr1--;
  zoomwin->sc0++;
  zoomwin->sc1--;
  sb_fillc (zoomwin, ' ');
  zoomwin->sr0--;
  zoomwin->sr1++;
  zoomwin->sc0--;
  zoomwin->sc1++;

  /* Put the help text at the bottom of the window */
  c = (int) (zoomwin->sr1 - 1);
  i = M_ZOOM_HELP;

  for (z = zoomwin->sr1 - ZHELPSIZE + 1; z < (short) c; i++, z++)
    sb_move_puts (zoomwin, z, 1, MSG_TXT (i));

  /* is there a rescan flag? */
  if (forcerescancheck ())
  {
    sb_move_puts (zoomwin, 1, 2, MSG_TXT (M_ZOOM_WAIT));
    sb_show ();
    xmit_reset (0, 0);
  }

  /* RDH *** Changes begin */
  show_alive ();
  /* RDH *** Changes end */

  /* is there something in the outbound? */
  if (mail_top == NULL)
  {
    sb_move_puts (zoomwin, 10, 10, MSG_TXT (M_NOTHING_IN_OUTBOUND));
    sb_show ();
    wait_for_key (PER_SECOND);
    goto outside;
  }

  p = next_mail = mail_top;
  oldp = oldcur = NULL;
  curl = 2;
  zoom_window (zoomwin, p, next_mail);

  t_idle = timerset (PER_MINUTE);
  inzoomwindow = 1;

  while (!timeup (t_idle) && (PEEKBYTE () == (short) 0xffff))
  {
    if (KEYPRESS ())
    {
      t_idle = timerset (30 * PER_SECOND);

      c = (int) KbRemap (FOSSIL_CHAR ());

      switch ((unsigned) c)
      {
      case F_PEND_PGUP:
        for (i = 2; i < (zoomwin->sr1 - ZHELPSIZE); i++)
        {
          if (p->ptrval.ptrnp.prev != NULL)
            p = p->ptrval.ptrnp.prev;
          if (next_mail->ptrval.ptrnp.prev != NULL)
            next_mail = next_mail->ptrval.ptrnp.prev;
        }
        break;

      case F_PEND_PGDN:
        for (i = 2; i < (zoomwin->sr1 - ZHELPSIZE); i++)
        {
          if (p->ptrval.ptrnp.next != NULL)
            p = p->ptrval.ptrnp.next;
          if (next_mail->ptrval.ptrnp.next != NULL)
            next_mail = next_mail->ptrval.ptrnp.next;
        }
        break;

      case F_PEND_UPAR:
        if (curl > 2)
          next_mail = next_mail->ptrval.ptrnp.prev;
        else
        {
          if (p->ptrval.ptrnp.prev != NULL)
            p = next_mail = p->ptrval.ptrnp.prev;
        }
        break;

      case F_PEND_DNAR:
        if (curl < (zoomwin->sr1 - 1 - ZHELPSIZE))
        {
          if (next_mail->ptrval.ptrnp.next != NULL)
            next_mail = next_mail->ptrval.ptrnp.next;
        }
        else
        {
          if (p->ptrval.ptrnp.next != NULL)
            p = p->ptrval.ptrnp.next;
          if (next_mail->ptrval.ptrnp.next != NULL)
            next_mail = next_mail->ptrval.ptrnp.next;
        }
        break;

      case F_PEND_HOME:
        p = next_mail = mail_top;
        break;

      case F_PEND_END:
        while (p->ptrval.ptrnp.next != NULL)
          p = p->ptrval.ptrnp.next;
        next_mail = p;
        for (i = 2; i < (zoomwin->sr1 - 1 - ZHELPSIZE); i++)
        {
          if (p->ptrval.ptrnp.prev != NULL)
            p = p->ptrval.ptrnp.prev;
        }
        break;

      case F_ZOOM_ADDRESS:     /* readdress */
        changed = sb_popup (10, 5, 4, 70, mail_addr, 0);
        break;

      case F_UN_CALLRIGHTNOW:
      case F_ZOOM_CRASH:       /* crash */
        changed = sb_popup (10, 5, 4, 70, mail_change, (int) 'c');
        break;

      case F_ZOOM_DIRECT:      /* direct */
        changed = sb_popup (10, 5, 4, 70, mail_change, (int) 'd');
        break;

      case F_UN_GETFILE:
      case F_ZOOM_GET:         /* get files */
        changed = sb_popup (10, 5, 9, 70, Do_Get, 1);  /* RDH *** 26.May.1996 (2 Lines more please ) */
        break;

      case F_ZOOM_HOLD:        /* hold */
        changed = sb_popup (10, 5, 4, 70, mail_change, (int) 'h');
        break;

      case F_ZOOM_KILLTRIES:   /* reset dial tries */
        changed = sb_popup (10, 5, 4, 70, mail_stop, 0);
        break;

      case F_ZOOM_NORMAL:      /* normal */
        changed = sb_popup (10, 5, 4, 70, mail_change, (int) 'n');
        break;

      case F_UN_POLLPKT:
      case F_ZOOM_POLL:        /* poll */
        changed = sb_popup (10, 5, 4, 70, Do_Poll_Packet, 1);
        break;

      case F_ZOOM_KILLREQ:     /* delete request */
        changed = sb_popup (10, 5, 4, 70, mail_delreq, 0);
        break;

      case F_UN_SENDFILE:
      case F_ZOOM_SEND:        /* send files */
        changed = sb_popup (10, 5, 6, 70, Do_Send, 1);
        break;

      case F_ZOOM_STOPMAIL:    /* stop mail */
        changed = sb_popup (10, 5, 4, 70, mail_stop, 1);
        break;

      case F_ZOOM_KILLNODE:
      case F_UN_KILLNODESMAIL:
        changed = !sb_popup (10, 5, 4, 70, Do_Kill, 1);
        break;

      case F_ZOOM_NODEINFO:    /* ALT-I info about node */
        sb_popup (SB_ROWS - 10, 1, 8, 78, Do_Info, 1);
        changed = 1;
        break;

      case F_ZOOM_NODEEDIT:    /* ALT-E edit nodes flags */
        sb_popup (5, 46, 14, 32, Do_Edit, 1);
        break;

      default:                 /* Space, ESCape or Return terminates */
        c &= 0x00ff;
        if ((c == 0x20) || (c == ESC) || (c == 0x0d))
          goto outside;
        break;
      }

      if (changed != 0)
      {
        sb_move (zoomwin, (short) curl, 1);
        sb_wa (zoomwin, colors.hold, 78);
        sb_move_puts (zoomwin, 1, 2, MSG_TXT (M_ZOOM_WAIT));
        sb_show ();
        xmit_reset (0, 1);
        t_idle = timerset (PER_MINUTE);

        /* is there something in the outbound ? */
        if (mail_top == NULL)
        {
          sb_move_puts (zoomwin, 10, 10, MSG_TXT (M_NOTHING_IN_OUTBOUND));
          sb_show ();
          wait_for_key (PER_SECOND);
          goto outside;
        }
        else
        {
          oldp = oldcur = NULL;
          p = next_mail = mail_top;
          curl = 2;
          changed = 0;
        }
      }

      if ((oldp != p) || (oldcur != next_mail))
        zoom_window (zoomwin, p, next_mail);

    }

    time_release ();
  }

outside:
  inzoomwindow = 0;
  return (1);
}

static void LOCALFUNC
zoom_window (REGIONP zoomwin, MAILP p, MAILP cur)
{
  int i;
  int calls, badcalls;
  int zsize = (zoomwin->sr1) - ZHELPSIZE;

  char out_str[100];            /* TJW 960427 increased size */

  if (p == NULL || cur == NULL)
    return;

  if (oldp == p)
  {
    oldcur = cur;
    sb_move (zoomwin, (short) curl, 1);
    sb_wa (zoomwin, colors.hold, 78);
    i = 2;

    while ((i < zsize) && (p != cur))
    {
      i++;
      p = p->ptrval.ptrnp.next;
    }

    sb_move (zoomwin, (short) i, 1);
    sb_wa (zoomwin, colors.calling, 78);
    curl = i;
    sb_show ();
    return;
  }

  /* save old status */
  oldp = p;
  oldcur = cur;

  /* give it a header */
  sb_move_puts (zoomwin, 1, 1, MSG_TXT (M_ZOOM_HEADER));
  sb_move (zoomwin, (short) curl, 1);
  sb_wa (zoomwin, colors.hold, 78);

  for (i = 2; i < zsize; i++)
  {
    long age;
    char msc[8];                /* AW 981201 */

    if (p == NULL)
      break;

    sb_move (zoomwin, (short) i, 1);

    if (p == cur)
    {
      sb_wa (zoomwin, colors.calling, 78);
      curl = i;
    }

    call_tries (&(p->mail_addr), &calls, &badcalls);

    age = age_seconds (p->oldest) / 86400L;

    /* TJW 960427 minor optimizations */
    sprintf (out_str, " %-34s%4u %9lu %3lu %5d %5d %5s%c ",
             Full_Addr_Str (&(p->mail_addr)),
             p->numfiles, p->mailsize, age, calls, badcalls,
             mail_status_chars (p->mailtypes, msc),
             mail_stat (p));

    sb_puts (zoomwin, out_str);
    p = p->ptrval.ptrnp.next;
  }

  /* clear the rest of the zoomed outbound */
  if (i <= zsize)
  {
    zoomwin->sr0 += i;
    zoomwin->sr1 -= ZHELPSIZE;
    zoomwin->sc0++;
    zoomwin->sc1--;
    sb_fillc (zoomwin, ' ');
    zoomwin->sr0 -= i;
    zoomwin->sr1 += ZHELPSIZE;
    zoomwin->sc0--;
    zoomwin->sc1++;
  }

  sb_show ();
}

#define PRINT(x,y,s) sb_move_puts (r, x, y,s)

/* show nodelist info in zoomed outbound window / r. hoerner */

/* VRP 990903 - is there a hidden line? */
static int hidden_no = 0;

int
Do_Info (BINK_SAVEP p, int search)
{
  REGIONP r;
  char junk[80];
  char pw[9];
  int i;

  search = search;
  r = p->region;

  if (!version7)
    return (0);

  if (search)
  {
    if (!ver7find (&next_mail->mail_addr, 1))  /* if we can't find the node  */
    {
      sb_fill (r, ' ', colors.popup);
      sb_box (r, boxtype, colors.popup);

      i = GetAltPhone ((char *) &newnodedes.PhoneNumber, &next_mail->mail_addr);
      GetAltModemflag ((word *) & newnodedes.ModemType, &next_mail->mail_addr);
      GetAltFidoFlag ((word *) & newnodedes.NodeFlags, &next_mail->mail_addr);
      if (GetAltOnLineTime (&newnodedes.online_start, &newnodedes.online_end, &next_mail->mail_addr))
      {
        newnodedes.NodeFlags |= B_CM;  /* VRP 990820 */
      }

      sprintf (newnodedes.SystemName, "(no data)");
      sprintf (newnodedes.MiscInfo, "(no data)");
      sprintf (lastfound_SysopName, "(no data)");
      newnodedes.BaudRate = 0;
      newnodedes.RealCost = 0;
      found_zone = next_mail->mail_addr.Zone;
      newnodedes.NetNumber = next_mail->mail_addr.Net;
      newnodedes.NodeNumber = next_mail->mail_addr.Node;
      newnodedes.HubNode = next_mail->mail_addr.Point;

      if (i)
        goto overridden;

      sprintf (junk, "Node %d:%d/%d.%d not found in nodelist",
               next_mail->mail_addr.Zone,
               next_mail->mail_addr.Net,
               next_mail->mail_addr.Node,
               next_mail->mail_addr.Point);
      PRINT (1, 1, junk);
    }
    else
      get_NodeAdr (lastfound_SysopName);

    goto endlabel;
  }

  sb_fill (r, ' ', colors.popup);
  sb_box (r, boxtype, colors.popup);

  if (!ver7find (&next_mail->mail_addr, 1))  /* if we can't find the node  */
  {
    i = GetAltPhone ((char *) &newnodedes.PhoneNumber, &next_mail->mail_addr);
    GetAltModemflag ((word *) & newnodedes.ModemType, &next_mail->mail_addr);
    GetAltFidoFlag ((word *) & newnodedes.NodeFlags, &next_mail->mail_addr);

    sprintf (newnodedes.SystemName, "(using 'override')");
    sprintf (newnodedes.MiscInfo, "(not found in nodelist)");
    newnodedes.BaudRate = 0;
    newnodedes.RealCost = 0;
    lastfound_SysopName[0] = 0;

    found_zone = next_mail->mail_addr.Zone;
    newnodedes.NetNumber = next_mail->mail_addr.Net;
    newnodedes.NodeNumber = next_mail->mail_addr.Node;
    newnodedes.HubNode = next_mail->mail_addr.Point;

    if (i)
      goto overridden;

    sprintf (junk, "Node %d:%d/%d.%d not found in nodelist",
             next_mail->mail_addr.Zone,
             next_mail->mail_addr.Net,
             next_mail->mail_addr.Node,
             next_mail->mail_addr.Point);
    PRINT (1, 1, junk);
    goto endlabel;
  }

  n_getpassword (&next_mail->mail_addr);  /* reads from password file,too! */

  /* these must be called after that,
   * 'cause n_getpassword reads the nodelist */

  GetAltPhone ((char *) &newnodedes.PhoneNumber, &next_mail->mail_addr);
  GetAltModemflag ((word *) & newnodedes.ModemType, &next_mail->mail_addr);
  GetAltFidoFlag ((word *) & newnodedes.NodeFlags, &next_mail->mail_addr);

overridden:

  /* VRP 990903 start */

  if (GetAltOnLineTime (&newnodedes.online_start, &newnodedes.online_end, &next_mail->mail_addr))
  {
    newnodedes.NodeFlags |= B_CM;
  }

  /*
   * Hidden line support
   *
   */

  if (hidden_no > 0)
  {
    i = GetHiddenInfo (hidden_no, found_zone, &newnodedes);
    strcpy (junk, "Hidden    :");
    if (i != DAY_ALL)
    {
      if (i & DAY_SUNDAY)
      {
        strcat (junk, " Su");
      }
      if (i & DAY_MONDAY)
      {
        strcat (junk, " Mo");
      }
      if (i & DAY_TUESDAY)
      {
        strcat (junk, " Tu");
      }
      if (i & DAY_WEDNESDAY)
      {
        strcat (junk, " We");
      }
      if (i & DAY_THURSDAY)
      {
        strcat (junk, " Th");
      }
      if (i & DAY_FRIDAY)
      {
        strcat (junk, " Fr");
      }
      if (i & DAY_SATURDAY)
      {
        strcat (junk, " Sa");
      }
    }
    else
    {
      strcat (junk, " Week");
    }

    PRINT (6, (SB_COLS >> 1) + 1, junk);
    hidden_no = 0;
  }

  /* VRP 990903 end */

  sprintf (junk, "Sysop     : %s", lastfound_SysopName);
  PRINT (1, 1, junk);

  if ((newnodedes.NodeFlags & B_point) && newnodedes.HubNode)
    sprintf (junk, "Point     : %u:%u/%u.%u", found_zone,
             newnodedes.NetNumber, newnodedes.NodeNumber,
             newnodedes.HubNode);
  else
    sprintf (junk, "Node      : %u:%u/%u.0", found_zone,
             newnodedes.NetNumber, newnodedes.NodeNumber);
  PRINT (2, 1, junk);

  sprintf (junk, "SystemName: %s", newnodedes.SystemName);
  PRINT (3, 1, junk);

  sprintf (junk, "Phone     : %s", newnodedes.PhoneNumber);
  PRINT (4, 1, junk);

  sprintf (junk, "Misc info : %s", newnodedes.MiscInfo);
  PRINT (5, 1, junk);

  if (newnodedes.NodeFlags & B_CM)
    sprintf (junk, "Online    : 24h");
  else if (newnodedes.online_start == newnodedes.online_end)
    sprintf (junk, "Online    : ZMH");
  else
    sprintf (junk, "Online    : %02d:%02d-%02d:%02d (UTC)",
             newnodedes.online_start / 60, newnodedes.online_start % 60,
             newnodedes.online_end / 60, newnodedes.online_end % 60);
  PRINT (6, 1, junk);

  memset (pw, 0, sizeof (pw));
  memmove (pw, newnodedes.Password, 8);

  if (!pw[0])
    sprintf (pw, " <none> ");

  if (!show_password)
  {
    if (pw[0])
      sprintf (pw, "-hidden-");
  }
  else
  {
    pw[8] = 0;
    for (i = 0; i < 8; i++)
      if (pw[i] == '%')         /* help sprintf to not make errors */
        pw[i] = '?';
  }

  sprintf (junk, "Password  : %s", pw);
  PRINT (1, (SB_COLS >> 1) + 1, junk);

  sprintf (junk, "Baudrate  : %ld", (300L * newnodedes.BaudRate));
  PRINT (2, (SB_COLS >> 1) + 1, junk);

  sprintf (junk, "Modemtype : %d ", newnodedes.ModemType);
  get_modem_flags (junk + strlen (junk));
  PRINT (3, (SB_COLS >> 1) + 1, junk);

  sprintf (junk, "Node flags: 0x%04x", newnodedes.NodeFlags);
  if (newnodedes.NodeFlags & B_CM)
    strcat (junk, ",CM");
  if (newnodedes.NodeFlags & B_hub)
    strcat (junk, ",(HUB)");
  if (newnodedes.NodeFlags & B_host)
    strcat (junk, ",(HOST)");
  if (newnodedes.NodeFlags & B_region)
    strcat (junk, ",(RC)");
  if (newnodedes.NodeFlags & B_zone)
    strcat (junk, ",(ZC)");
  if (newnodedes.NodeFlags & B_point)
    strcat (junk, ",(POINT)");
  PRINT (4, (SB_COLS >> 1) + 1, junk);

  sprintf (junk, "Real cost : %u", newnodedes.RealCost);
  PRINT (5, (SB_COLS >> 1) + 1, junk);

endlabel:
  wait_for_key (1 * PER_MINUTE);
  return (1);
}

#undef PRINT
#define PRINT(x,s) sb_move_puts (r, x, 1, s)

/* set node dependend and/or global protocol flags on
 * the fly in zoomed outbound window     / r.hoerner */

void
setnodeflagnow (int set, unsigned long flag, ADDRP addrp)
{
  NODESTRUC *qnode;

  /* MR 970314 query node structure; if addr does not yet exist, create new
   *           struct and add it to our list. */

  qnode = QueryNodeStruct (addrp, 1);

  if (qnode)
  {
    if (set)
      qnode->Flags |= flag;
    else
      qnode->Flags &= ~flag;
  }
}

int
Do_Edit (BINK_SAVEP p, int x)
{
  REGIONP r;
  char junk[80];
  long noZed;
  long noZip;
  long noJan;
  long noHyd;
  long noPick;
  long freeReq;
  long noReq;
  long noDiet;
  long noWazoo;
  long noEmsi;
  long noSEA;
  long maxwait;
  int j;
  char c;

  x = x;
  r = p->region;
  sb_fill (r, ' ', colors.popup);
  sb_box (r, boxtype, colors.popup);

  for (;;)
  {
    noZed = QueryNodeFlag (no_zapzed, NOZED, &next_mail->mail_addr);
    noZip = QueryNodeFlag (no_zipzed, NOZIP, &next_mail->mail_addr);
    noJan = QueryNodeFlag (no_janus, NOJANUS, &next_mail->mail_addr);
    noHyd = QueryNodeFlag (no_hydra, NOHYDRA, &next_mail->mail_addr);
    noPick = QueryNodeFlag (no_pickup, NOPICK, &next_mail->mail_addr);
    freeReq = QueryNodeFlag (on_our_nickel, FREEREQ, &next_mail->mail_addr);
    noReq = QueryNodeFlag (no_requests, NOREQ, &next_mail->mail_addr);
    noDiet = QueryNodeFlag (no_dietifna, NODIET, &next_mail->mail_addr);
    noWazoo = QueryNodeFlag (no_WaZOO, NOWAZOO, &next_mail->mail_addr);
    noEmsi = QueryNodeFlag (no_EMSI, NOEMSI, &next_mail->mail_addr);
    noSEA = QueryNodeFlag (no_sealink, NOSEA, &next_mail->mail_addr);

    sprintf (junk, " %u:%u/%u.%u", next_mail->mail_addr.Zone,
             next_mail->mail_addr.Net,
             next_mail->mail_addr.Node,
             next_mail->mail_addr.Point);
    PRINT (1, junk);

    sb_move_puts (r, 1, 22, " -all-");

    sprintf (junk, " NoEMSI......(%s)[0]  (%s)[A]", noEmsi ? HAS : HAS_NOT,
             no_EMSI ? HAS : HAS_NOT);
    PRINT (2, junk);
    sprintf (junk, " NoWaZoo.....(%s)[1]  (%s)[B]", noWazoo ? HAS : HAS_NOT,
             no_WaZOO ? HAS : HAS_NOT);
    PRINT (3, junk);
    sprintf (junk, " NoSEAlink...(%s)[2]  (%s)[C]", noSEA ? HAS : HAS_NOT,
             no_sealink ? HAS : HAS_NOT);
    PRINT (4, junk);
    sprintf (junk, " NoJanus.....(%s)[3]  (%s)[D]", noJan ? HAS : HAS_NOT,
             no_janus ? HAS : HAS_NOT);
    PRINT (5, junk);
    sprintf (junk, " NoHydra.....(%s)[4]  (%s)[E]", noHyd ? HAS : HAS_NOT,
             no_hydra ? HAS : HAS_NOT);
    PRINT (6, junk);
    sprintf (junk, " NoZedZap....(%s)[5]  (%s)[F]", noZed ? HAS : HAS_NOT,
             no_zapzed ? HAS : HAS_NOT);
    PRINT (7, junk);
    sprintf (junk, " NoZedZip....(%s)[6]  (%s)[G]", noZip ? HAS : HAS_NOT,
             no_zipzed ? HAS : HAS_NOT);
    PRINT (8, junk);
    sprintf (junk, " NoDietIFNA..(%s)[7]  (%s)[H]", noDiet ? HAS : HAS_NOT,
             no_dietifna ? HAS : HAS_NOT);
    PRINT (9, junk);
    sprintf (junk, " ReqOnUs.....(%s)[8]  (%s)[I]", freeReq ? HAS : HAS_NOT,
             on_our_nickel ? HAS : HAS_NOT);
    PRINT (10, junk);
    sprintf (junk, " NoRequests..(%s)[9]  (%s)[J]", noReq ? HAS : HAS_NOT,
             no_requests ? HAS : HAS_NOT);
    PRINT (11, junk);
    sprintf (junk, " NoPickup....(%s)[-]  (%s)[K]", noPick ? HAS : HAS_NOT,
             no_pickup ? HAS : HAS_NOT);
    PRINT (12, junk);
    sprintf (junk, " press key to edit, ESC: quit");
    sb_move_puts (r, 13, 2, junk);
    sb_show ();

    c = 0;
    maxwait = longtimerset (15L * (long) PER_MINUTE);
    while (!longtimeup (maxwait))
    {
      time_release ();
      show_alive ();
      if (KEYPRESS ())
      {
        j = FOSSIL_CHAR ();
        c = (char) j;
        if (c != 0)
        {
          maxwait = longtimerset (15L * (long) PER_MINUTE);
          if (c == ESC)
            goto endlabel;
          else
          {
            c = (char) toupper ((int) c);
            break;
          }
        }
      }
    }

    switch (c)
    {
    case '0':
      setnodeflagnow (!noEmsi, NOEMSI, &next_mail->mail_addr);
      break;

    case '1':
      setnodeflagnow (!noWazoo, NOWAZOO, &next_mail->mail_addr);
      break;

    case '2':
      setnodeflagnow (!noSEA, NOSEA, &next_mail->mail_addr);
      break;

    case '3':
      setnodeflagnow (!noJan, NOJANUS, &next_mail->mail_addr);
      break;

    case '4':
      setnodeflagnow (!noHyd, NOHYDRA, &next_mail->mail_addr);
      break;

    case '5':
      setnodeflagnow (!noZed, NOZED, &next_mail->mail_addr);
      break;

    case '6':
      setnodeflagnow (!noZip, NOZIP, &next_mail->mail_addr);
      setnodeflagnow (!noZip, NOZED, &next_mail->mail_addr);
      setnodeflagnow (!noZip, NOEMSI, &next_mail->mail_addr);
      break;

    case '7':
      setnodeflagnow (!noDiet, NODIET, &next_mail->mail_addr);
      break;

    case '8':
      setnodeflagnow (!freeReq, FREEREQ, &next_mail->mail_addr);
      break;

    case '9':
      setnodeflagnow (!noReq, NOREQ, &next_mail->mail_addr);
      break;

    case '-':
      setnodeflagnow (!noPick, NOPICK, &next_mail->mail_addr);
      break;

    case 'A':
      no_EMSI = !no_EMSI;
      break;

    case 'B':
      no_WaZOO = !no_WaZOO;
      break;

    case 'C':
      no_sealink = !no_sealink;
      break;

    case 'D':
      no_janus = !no_janus;
      break;

    case 'E':
      no_hydra = !no_hydra;
      break;

    case 'F':
      no_zapzed = !no_zapzed;
      break;

    case 'G':
      no_zipzed = !no_zipzed;
      no_zapzed = no_zipzed;
      no_EMSI = no_zipzed;
      break;

    case 'H':
      no_dietifna = !no_dietifna;
      break;

    case 'I':
      on_our_nickel = !on_our_nickel;
      break;

    case 'J':
      no_requests = !no_requests;
      break;

    case 'K':
      no_pickup = !no_pickup;
      break;

    default:
      break;
    }
  }

endlabel:
  return (1);
}

#undef PRINT

int
mail_change (BINK_SAVEP p, int x)
{
  REGIONP r;
  char iname[PATHLEN], oname[PATHLEN], tname[PATHLEN];
  char *HoldName, *result, *temp_buffer, junk[256];
  struct FILEINFO fileinfo;
  FILE *ihandle, *ohandle;
  int check;
  int written = 1;
  int buff_size;
  int change = 0;

  if (p == NULL)
    return (change);

  r = p->region;

  sb_fill (r, ' ', colors.popup);
  sb_box (r, boxtype, colors.popup);

  switch ((char) x)
  {
  case 'c':
    strcpy (oname, "Crash");
    break;

  case 'd':
    strcpy (oname, "Direct");
    break;

  case 'h':
    strcpy (oname, "Hold");
    break;

  case 'n':
    strcpy (oname, "Normal");
    x = (int) 'f';
    break;
  }

  sprintf (junk, MSG_TXT (M_ZOOM_CHANGE),
           Full_Addr_Str (&(next_mail->mail_addr)), oname);

  sb_caption (r, junk, colors.popup);
  sb_move_puts (r, 1, 2, MSG_TXT (M_ARE_YOU_SURE));

  junk[0] = '\0';
  // HJK 980805 - Input was not at right place in other languages
  sb_input_chars (r, 1, strlen (MSG_TXT (M_ARE_YOU_SURE)) + 3, junk, 1);

  /* If user says 'no', get out */
  if (junk[0] != (char) toupper (MSG_TXT (M_YES)[0]))
    return (change);

  if (flag_file (TEST_AND_SET, &(next_mail->mail_addr), 0))
  {
    sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
    wait_for_key (PER_SECOND);
    return (0);
  }

  /* first append/rename ?LO */
  HoldName = HoldAreaNameMunge (&(next_mail->mail_addr));
  sprintf (tname, "%s%s.?lo", HoldName,
           Hex_Addr_Str (&(next_mail->mail_addr)));
  sprintf (oname, "%s%s.%clo", HoldName,
           Hex_Addr_Str (&(next_mail->mail_addr)), (char) x);

  if (!dfind (&fileinfo, tname, 0))
  {
    do
    {
      buff_size = (int) fileinfo.size;
      strcpy (iname, fileinfo.name);
      result = strchr (iname, '.');
      result++;

      if (*result != (char) x)
      {
        if (next_mail->mail_addr.Point != 0)
        {
          sprintf (iname, "%s%04x%04x.pnt" DIR_SEPS "%s",
                   HoldName, next_mail->mail_addr.Net,
                   next_mail->mail_addr.Node, fileinfo.name);
        }
        else
          sprintf (iname, "%s%s", HoldName, fileinfo.name);

        if (rename (iname, oname) != 0)
        {
          temp_buffer = (char *) malloc (buff_size + 1);
          ihandle = fopen (iname, read_binary);
          ohandle = fopen (oname, append_binary);
          sprintf (junk, MSG_TXT (M_ZOOM_APPEND), MSG_TXT (M_ZOOM_MAIL_ATT));
          sb_move_puts (r, 2, 2, junk);

          do
          {
            check = fread (temp_buffer, sizeof (char), buff_size, ihandle);

            if (check)
              written = fwrite (temp_buffer, sizeof (char), check, ohandle);
          }
          while (check && !feof (ihandle) && !ferror (ihandle));

          fclose (ihandle);
          fclose (ohandle);

          if (written > 0)
            unlink (iname);

          free (temp_buffer);
        }
        else
        {
          sprintf (junk, MSG_TXT (M_ZOOM_RENAME), MSG_TXT (M_ZOOM_MAIL_ATT));
          sb_move_puts (r, 2, 2, junk);
        }

        change = 1;
      }
    }
    while (!dfind (&fileinfo, NULL, 1));

    (dfind (&fileinfo, NULL, 2));
  }

  /* now rename ?UT */

  if ((char) x == 'f')
    x = (int) 'o';
  sprintf (tname, "%s%s.?ut", HoldName,
           Hex_Addr_Str (&(next_mail->mail_addr)));
  sprintf (oname, "%s%s.%cut", HoldName,
           Hex_Addr_Str (&(next_mail->mail_addr)), (char) x);
  if (!dfind (&fileinfo, tname, 0))
  {
    do
    {
      buff_size = (int) fileinfo.size;
      strcpy (iname, fileinfo.name);
      result = strchr (iname, '.');
      result++;
      if (*result != (char) x)
      {
        if (next_mail->mail_addr.Point != 0)
        {
          sprintf (iname, "%s%04x%04x.pnt" DIR_SEPS "%s",
                   HoldName, next_mail->mail_addr.Net,
                   next_mail->mail_addr.Node, fileinfo.name);
        }
        else
          sprintf (iname, "%s%s", HoldName, fileinfo.name);

        if ((rename (iname, oname)) != 0)
        {
          if (buff_size > 60)
            append_pkt (r, buff_size, iname, oname);
        }                       /* do append if rename failed */
        else
        {
          sprintf (junk, MSG_TXT (M_ZOOM_RENAME), MSG_TXT (M_ZOOM_MAIL_PKT));
          sb_move_puts (r, 2, 2, junk);
        }

        change = 1;
      }
    }
    while (!dfind (&fileinfo, NULL, 1));

    (dfind (&fileinfo, NULL, 2));
  }                             /* end of ?UT handling */

  flag_file (CLEAR_FLAG, &(next_mail->mail_addr), 0);
  wait_for_key (PER_SECOND);
  return (change);
}

void
append_pkt (REGIONP r, unsigned int size, char *iname, char *oname)
{
  FILE *ihandle, *ohandle;
  struct tm *dt;
  time_t now;
  struct _pkthdr *header;
  char *copy, junk[256];
  unsigned int buff_size, go;

  if (r)
  {
    sprintf (junk, MSG_TXT (M_ZOOM_APPEND), MSG_TXT (M_ZOOM_MAIL_PKT));
    sb_move_puts (r, 2, 2, junk);
  }

  header = (struct _pkthdr *) malloc (sizeof (struct _pkthdr) + 1);

  ohandle = fopen (oname, read_binary_plus);
  ihandle = fopen (iname, read_binary);

  go = fread (header, sizeof (struct _pkthdr), 1, ohandle);

  fseek (ohandle, 0L, SEEK_SET);

  /* Update the packet header to reflect that its been messed with */

  unix_time (&now);
  dt = unix_localtime (&now);

  header->year = dt->tm_year + ((dt->tm_year < 70) ? 2000 : 1900);  /* AG 990120 Y2K fix */
  header->month = dt->tm_mon;   /* TJW 971020 bugfix */
  header->day = dt->tm_mday;
  header->hour = dt->tm_hour;
  header->minute = dt->tm_min;
  header->second = dt->tm_sec;
  header->product = PRDCT_CODE;

  go = fwrite (header, sizeof (struct _pkthdr), 1, ohandle);

  free (header);
  fseek (ohandle, -2L, SEEK_END);
  fseek (ihandle, ((long) sizeof (struct _pkthdr)), SEEK_SET);
  size -= sizeof (struct _pkthdr);

  buff_size = size;
  if ((copy = (char *) malloc (buff_size)) == NULL)
  {
    do
    {
      buff_size = (buff_size / 10) * 9;
      copy = (char *) malloc (buff_size);
    }
    while (copy == NULL);
  }

  do
  {
    go = fread (copy, sizeof (char), buff_size, ihandle);
    go = fwrite (copy, sizeof (char), go, ohandle);

    size -= buff_size;
    if (buff_size > size)
      buff_size = size;
  }
  while (size > 0);

  free (copy);
  fclose (ihandle);
  fclose (ohandle);

  if (go > 0)
    unlink (iname);
  else
  {
    sprintf (junk, MSG_TXT (M_ZOOM_ERRAPP), MSG_TXT (M_ZOOM_MAIL_PKT));
    if (r)
      sb_move_puts (r, 2, 2, junk);
    else
      status_line (junk);
  }
}

int
mail_stop (BINK_SAVEP p, int x)
{
  REGIONP r;
  int change = 0;
  char stop[PATHLEN];
  char *HoldName;
  char sure[2];
  FILE *fp;
  struct FILEINFO fileinfo;

  if (p == NULL)
    return (change);

  r = p->region;

  sb_fill (r, ' ', colors.popup);
  sb_box (r, boxtype, colors.popup);

  if (x)
    sprintf (stop, MSG_TXT (M_ZOOM_STOP),
             Full_Addr_Str (&(next_mail->mail_addr)));
  else
    sprintf (stop, MSG_TXT (M_ZOOM_DELETE),
             Full_Addr_Str (&(next_mail->mail_addr)));

  sb_caption (r, stop, colors.popup);
  sb_move_puts (r, 1, 2, MSG_TXT (M_ARE_YOU_SURE));

  sure[0] = '\0';
  // HJK 980805 - Input was not at right place in other languages
  sb_input_chars (r, 1, strlen (MSG_TXT (M_ARE_YOU_SURE)) + 3, sure, 1);

  /* If user says 'no', get out */

  if (sure[0] != (char) toupper (MSG_TXT (M_YES)[0]))
    return (change);

  if (flag_file (TEST_AND_SET, &(next_mail->mail_addr), 0))
  {
    sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
    wait_for_key (PER_SECOND);
    return (0);
  }

  HoldName = HoldAreaNameMunge (&(next_mail->mail_addr));
  sprintf (stop, "%s%s.$$?", HoldName,
           Hex_Addr_Str (&(next_mail->mail_addr)));

  if (!dfind (&fileinfo, stop, 0))
  {
    do
    {
      if (next_mail->mail_addr.Point != 0)
      {
        sprintf (stop, "%s%04x%04x.pnt" DIR_SEPS "%s",
                 HoldName, next_mail->mail_addr.Net,
                 next_mail->mail_addr.Node, fileinfo.name);
      }
      else
        sprintf (stop, "%s%s", HoldName, fileinfo.name);

      unlink (stop);
    }
    while (!dfind (&fileinfo, NULL, 1));

    (dfind (&fileinfo, NULL, 2));
    sb_move_puts (r, 2, 2, MSG_TXT (M_ZOOM_DELCOUNT));
    change = 1;
  }

  if (x)
  {
    sprintf (stop, "%s%s.$$9", HoldName,
             Hex_Addr_Str (&(next_mail->mail_addr)));
    sb_move_puts (r, 2, 2, MSG_TXT (M_ZOOM_NOSEND));

    fp = share_fopen (stop, append_ascii, DENY_WRITE);
    if (fp == (FILE *) NULL)
      sb_move_puts (r, 2, 2, MSG_TXT (M_ZOOM_ERRNOSEND));
    else
    {
      change = 1;
      fclose (fp);
    }
  }

  flag_file (CLEAR_FLAG, &(next_mail->mail_addr), 0);
  wait_for_key (PER_SECOND);
  return (change);
}

int
mail_delreq (BINK_SAVEP p, int x)
{
  REGIONP r;
  FILE *rhandle;
  char req[PATHLEN];
  char sure[2], junk[256];
  char *HoldName;

  if (p == NULL)
    return (0);

  r = p->region;
  x = 0;

  sb_fill (r, ' ', colors.popup);
  sb_box (r, boxtype, colors.popup);

  sprintf (junk, MSG_TXT (M_ZOOM_DELREQ),
           Full_Addr_Str (&(next_mail->mail_addr)));

  sb_caption (r, junk, colors.popup);
  sb_move_puts (r, 1, 2, MSG_TXT (M_ARE_YOU_SURE));

  sure[0] = '\0';
  // HJK 980805 - Input was not at right place in other languages
  sb_input_chars (r, 1, strlen (MSG_TXT (M_ARE_YOU_SURE)) + 3, sure, 1);

  /* If user says 'no', get out */

  if (sure[0] != (char) toupper (MSG_TXT (M_YES)[0]))
    return (x);

  if (flag_file (TEST_AND_SET, &(next_mail->mail_addr), 0))
  {
    sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
    wait_for_key (PER_SECOND);
    return (0);
  }

  HoldName = HoldAreaNameMunge (&(next_mail->mail_addr));
  sprintf (req, "%s%s.req", HoldName,
           Hex_Addr_Str (&(next_mail->mail_addr)));

  if (dexists (req))
  {
    rhandle = fopen (req, read_binary);
    if (rhandle == NULL)
      sb_move_puts (r, 2, 2, MSG_TXT (M_ZOOM_ERRREQ));
    else
    {
      fclose (rhandle);
      unlink (req);
      sb_move_puts (r, 2, 2, MSG_TXT (M_ZOOM_DEL_REQ));
      x = 1;
    }

    wait_for_key (PER_SECOND);
  }

  flag_file (CLEAR_FLAG, &(next_mail->mail_addr), 0);
  return (x);
}

static void LOCALFUNC
append_fil (REGIONP r, unsigned int size, char *iname, char *oname, char *desc)
{
  char *temp_buffer;
  char temp[50];
  FILE *ihandle, *ohandle;
  int check;
  int written = 1;

  temp_buffer = (char *) malloc (size + 1);
  ihandle = fopen (iname, read_binary);
  ohandle = fopen (oname, append_binary);
  sprintf (temp, MSG_TXT (M_ZOOM_APPEND), desc);
  sb_move_puts (r, 2, 2, temp);

  do
  {
    check = fread (temp_buffer, sizeof (char), size, ihandle);

    if (check)
      written = fwrite (temp_buffer, sizeof (char), check, ohandle);
  }
  while (check && !feof (ihandle) && !ferror (ihandle));

  fclose (ihandle);
  fclose (ohandle);

  if (written > 0)
    unlink (iname);

  free (temp_buffer);
}

static int LOCALFUNC
appren_mail (REGIONP r, ADDRP fromaddr, ADDRP toaddr, char *ftype, char *desc)
{
  char iname[PATHLEN], oname[PATHLEN], tname[PATHLEN];
  char *HoldName;
  struct FILEINFO fileinfo;
  int buff_size;
  int change = 0;

  HoldName = HoldAreaNameMunge (fromaddr);
  sprintf (tname, "%s%s.%s", HoldName, Hex_Addr_Str (fromaddr), ftype);

  if (!SameAddress (fromaddr, toaddr))
  {
    if (!dfind (&fileinfo, tname, 0))
    {
      do
      {
        buff_size = (int) fileinfo.size;
        HoldName = HoldAreaNameMunge (toaddr);
        sprintf (oname, "%s%s.%s", HoldName, Hex_Addr_Str (toaddr),
                 &(fileinfo.name[9]));
        HoldName = HoldAreaNameMunge (fromaddr);

        if (next_mail->mail_addr.Point != 0)
        {
          sprintf (iname, "%s%04x%04x.pnt" DIR_SEPS "%s",
                   HoldName, fromaddr->Net, fromaddr->Node,
                   fileinfo.name);
        }
        else
          sprintf (iname, "%s%s", HoldName, fileinfo.name);

        if (rename (iname, oname) != 0)
        {
          if (!strcmp (ftype, "?ut"))  /* mail packet */
          {
            if (buff_size > 60)
              append_pkt (r, buff_size, iname, oname);
          }
          else                  /* other file */
            append_fil (r, buff_size, iname, oname, desc);
        }
        else
        {
          sprintf (iname, MSG_TXT (M_ZOOM_RENAME), desc);
          sb_move_puts (r, 2, 2, iname);
        }

        change = 1;
      }
      while (!dfind (&fileinfo, NULL, 1));

      (dfind (&fileinfo, NULL, 2));
    }
  }

  return (change);
}

int
mail_addr (BINK_SAVEP p, int use_other_addr)
{
  REGIONP r;
  ADDR addr;
  char iname[PATHLEN], sure[2];
  int answered;

  if (p == NULL)
    return (0);

  r = p->region;

  sb_fill (r, ' ', colors.popup);
  sb_box (r, boxtype, colors.popup);
  sb_caption (r, MSG_TXT (M_ZOOM_DESTINATION), colors.popup);
  sb_move_puts (r, 1, 2, MSG_TXT (M_ZOOM_NEWADDR));

  if (use_other_addr)
  {
    answered = 1;
    addr = other_addr;
  }
  else
  {
    addr = next_mail->mail_addr;

    if (hidedomains)
      addr.Domain = NULL;

    strntcpy (iname, Full_Addr_Str (&addr), 35);
    answered = !sb_input_address (r, 1, 18, iname, 36);

    if (answered)
      if (!find_address (iname, &addr))
        answered = 0;
  }

  if (answered)
  {
    if (nodefind (&addr, 0))
    {
      sprintf (iname, "Node: %-18.18s - %-20.20s",
               Full_Addr_Str (&addr), newnodedes.SystemName);
      sb_move_puts (r, 1, 2, iname);
    }

    sb_move_puts (r, 2, 2, MSG_TXT (M_ARE_YOU_SURE));

    sure[0] = '\0';
    // HJK 980805 - Input was not at right place in other languages
    sb_input_chars (r, 2, strlen (MSG_TXT (M_ARE_YOU_SURE)) + 3, sure, 1);

    /* If user says 'yes', continue */
    if (sure[0] == (char) toupper (MSG_TXT (M_YES)[0]))
    {
      if (flag_file (TEST_AND_SET, &(next_mail->mail_addr), 0))
      {
        sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
        wait_for_key (PER_SECOND);
      }
      else
      {
        if (flag_file (TEST_AND_SET, &addr, 0))
        {
          flag_file (CLEAR_FLAG, &(next_mail->mail_addr), 0);
          sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
          wait_for_key (PER_SECOND);
        }
        else
        {
          /* first append/rename ?LO */
          appren_mail (r, &(next_mail->mail_addr), &addr, "?lo",
                       "mail attaches");

          /* append/rename REQ */
          appren_mail (r, &(next_mail->mail_addr), &addr, "req",
                       "request files");

          /* now rename ?UT */
          appren_mail (r, &(next_mail->mail_addr), &addr, "?ut",
                       "mail packets ");

          flag_file (CLEAR_FLAG, &addr, 0);
          flag_file (CLEAR_FLAG, &(next_mail->mail_addr), 0);
          wait_for_key (PER_SECOND);

          return (1);
        }
      }
    }
  }                             /* end of address input */

  return (0);
}

int
Do_Get (BINK_SAVEP p, int use_default)
{
  REGIONP r;
  ADDR addr;
  char SystemName[128];         /* RDH *** 26.May.1996 hold the output string */
  char node[46];
  char file[46];
  char fileparam[20];
  char flavor[2];
  char yesno[2];
  char more[2];
  char junk[256];
  int busy = 0;
  int did_it = 0;
  int addr_ok = 0;
  short i = 0;

  if (p == NULL)
    return (0);

  r = p->region;

  if (next_mail == NULL)
    use_default = 0;

  for (;;)
  {
    /* First fill it all with blanks */
    sb_fill (r, ' ', colors.popup);

    /* Now draw the box */
    sb_box (r, boxtype, colors.popup);
    sb_caption (r, MSG_TXT (M_GET_FILE), colors.popup);

    /* Now do the fields */
    sb_move_puts (r, 1, 2, MSG_TXT (M_ADDRESS_TO_GET_FROM));

    if (!addr_ok)               /* we haven't a valid address (any more) */
    {
      if (use_default)
      {
        addr = next_mail->mail_addr;

        if (hidedomains)
          addr.Domain = NULL;

        strntcpy (node, Full_Addr_Str (&addr), 44);
      }
      else
        node[0] = '\0';

      if (sb_input_address (r, 1, 23, node, 45))
        break;

      if (!find_address (node, &addr))
      {
        if (!did_it)
          did_it = -1;
        break;
      }
    }

    /* RDH *** changes begin */
    if (!nodefind (&addr, (int) 0))
    {
      sb_move_puts (r, 1, 23 + strlen (node) + 1, MSG_TXT (M_NODE_NOT_FOUND));
      sprintf (junk, MSG_TXT (M_SHOULD_I_USE), Full_Addr_Str (&addr));
      sb_move_puts (r, 2, 2, junk);

      yesno[0] = '\0';
      sb_input_chars (r, 2, strlen (junk) + 3, yesno, 1);

      if ((char) toupper (yesno[0]) == MSG_TXT (M_YES)[0])
      {
        sb_move_puts (r, 1, 22, space47);
        sb_move_puts (r, 2, 1, space66);
        sb_move_puts (r, 1, 23, Full_Addr_Str (&addr));
        addr_ok = 1;
      }
      else
        sb_move_puts (r, 2, 1, space66);
    }
    else
    {
      /* show systemName */
      sb_move_puts (r, 1, 22, space47);
      sprintf (SystemName, "%s, %s", Full_Addr_Str (&addr), newnodedes.SystemName);
      SystemName[47] = '\0';
      sb_move_puts (r, 1, 23, SystemName);
      addr_ok = 1;
    }
    /* RDH *** changes end */

    if (addr_ok)                /* do we have a valid address now??? */
    {
      /* Busy the node if we can. */
      if (flag_file (TEST_AND_SET, &addr, 0))
      {
        sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
        wait_for_key (PER_SECOND);
        if (!did_it)
          did_it = -1;
        break;
      }

      busy = 1;

      // HJK - 99/01/22 - 2 possibilities now, the original (2.60) version
      // for Do_Get, which makes it also possible to request longfilenames
      if (freqlongnames)
      {
        file[0] = '\0';
        fileparam[0] = '\0';
        flavor[0] = '\0';

        sb_move_puts (r, 2, 2, MSG_TXT (M_FILE_TO_RECEIVE));

        /* Get the filename */
        if (sb_input_chars (r, 2, 23, file, 45))
          break;

        /* Get the password if any */
        sb_move_puts (r, 3, 2, MSG_TXT (M_PASSWORD_TO_USE));
        sb_input_chars (r, 3, 23, fileparam, 18);

        /* Get the size if any */
        sb_move_puts (r, 4, 2, MSG_TXT (M_GET_FLAVOR));
        sb_input_chars (r, 4, 41, flavor, 1);
        flavor[0] = tolower (flavor[0]);

        /* Get the file if we can */
        if (flavor[0] != 'q')
        {
          if (mail_addrequest (&addr, file, fileparam, flavor) == 0)
            did_it = 1;
          else if (!did_it)
            did_it = -1;
        }
      }
      else
      {                         /* !freqlongnames */
        /* Ask for the filename and password/size */
        sb_move_puts (r, 3, 2, MSG_TXT (M_GETFILE_ANDPWD));

        for (i = 0; i < 3; i++)
        {
          file[0] = '\0';
          fileparam[0] = '\0';
          flavor[0] = '\0';

          /* Get the filename */
          if (sb_input_chars (r, 4 + i, 2, file, 12))
            break;

          /* Get the password/size if any */
          sb_input_chars (r, 4 + i, 18, fileparam, 18);
          sb_input_chars (r, 4 + i, 37, flavor, 1);
          flavor[0] = tolower (flavor[0]);

          /* Get the file if we can */
          if (flavor[0] != 'q')
          {
            if (mail_addrequest (&addr, file, fileparam, flavor) == 0)
              did_it = 1;
            else if (!did_it)
              did_it = -1;
          }
        }
      }

      /* Anything more to do? */
      more[0] = '\0';

      sb_move_puts (r, 7, 2, MSG_TXT (M_GET_MORE));
      // HJK 980805 - Input was not at right place in other languages
      sb_input_chars (r, 7, strlen (MSG_TXT (M_GET_MORE)) + 3, more, 1);

      if (more[0] == ('O'))
      {
        addr_ok = 0;            /* user wants to enter new address  */
        use_default = 0;        /* and we don't use the default one */
        busy = 0;
        flag_file (CLEAR_FLAG, &addr, 0);
        continue;
      }
      if (more[0] == (char) toupper (MSG_TXT (M_NO)[0]))
        break;
      if (more[0] == (char) toupper (MSG_TXT (M_YES)[0]))
        continue;
      if (i < 3)
        break;

      /* RDH *** changes end */
    }
  }

  /* If we busied something, get rid of it. */
  if (busy)
    flag_file (CLEAR_FLAG, &addr, 0);

  return (did_it);
}

static int LOCALFUNC
do_flavor (char *flavor)
{
  if (flavor[0] == '\0')
    flavor[0] = 'n';

  if ((flavor[0] != 'c')
      && (flavor[0] != 'h')
      && (flavor[0] != 'n')
      && (flavor[0] != 'f')     /* VRP 990826 */
      && (flavor[0] != 'd'))
    return (1);

  if (flavor[0] == 'n')
    flavor[0] = 'f';

  flavor[1] = '\0';
  return (0);
}

static int LOCALFUNC
mail_addrequest (ADDRP pAddr, char *file, char *fileparam, char *flavor)
{
  char *HoldName;
  char fname[PATHLEN];
  FILE *fp;

  if (do_flavor (flavor))
    return (-3);

  HoldName = HoldAreaNameMunge (pAddr);

  /* Now see if we should send anything back to him */

  sprintf (fname, "%s%s.req", HoldName, Hex_Addr_Str (pAddr));
  fp = fopen (fname, append_binary);
  if (fp == NULL)
    return (-2);

  fprintf (fp, "%s", file);

  if (fileparam[0] != '\0')
    fprintf (fp, " %s", fileparam);

  fprintf (fp, "\r\n");
  fclose (fp);

  mail_addsend (pAddr, "", flavor);
  return (0);
}

int
Do_Poll_Packet (BINK_SAVEP p, int use_default)
{
  REGIONP r;
  ADDR addr;
  char node[41];
  int did_it = 0;
  char flavor[2] = "c";

  if (p == NULL)
    return (0);

  r = p->region;

  if (next_mail == NULL)
    use_default = 0;

  /* First fill it all with blanks */
  sb_fill (r, ' ', colors.popup);

  /* Now draw the box */
  sb_box (r, boxtype, colors.popup);
  sb_caption (r, MSG_TXT (M_ZOOM_POLL), colors.popup);

  /* Now do the fields */
  sb_move_puts (r, 1, 2, MSG_TXT (M_POLL_WHOM));

  if (use_default)
  {
    addr = next_mail->mail_addr;

    if (hidedomains)
      addr.Domain = NULL;

    strntcpy (node, Full_Addr_Str (&addr), 39);
  }
  else
    node[0] = '\0';

  if (!sb_input_address (r, 1, 14, node, 40))
  {
    if (!find_address (node, &addr))
    {
      if (!did_it)
        did_it = -1;
    }
    else
    {
      /* Busy the node if we can. */
      if (flag_file (TEST_AND_SET, &addr, 0))
      {
        sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
        wait_for_key (PER_SECOND);
        if (!did_it)
          did_it = -1;
      }
      else
      {
        if (mail_addsend (&addr, "", flavor) == 0)
          did_it = 1;
        else if (!did_it)
          did_it = -1;

        flag_file (CLEAR_FLAG, &addr, 0);
      }
    }
  }

  return (did_it);
}

int
Do_Send (BINK_SAVEP p, int use_default)
{
  REGIONP r;
  ADDR addr;
  char node[47];
  char file[47];
  char flavor[2];
  char yesno[2];
  char SystemName[128];         /* RDH *** 26.May.96 */
  char more[2];
  int busy = 0;
  int did_it = 0;
  int addr_ok = 0;
  char junk[256];

  /* RDH *** changes begin */
  struct FILEINFO fileinfo;
  int rc;

  /* RDH *** changes end */

  if (p == NULL)
    return (0);

  r = p->region;

  if (next_mail == NULL)
    use_default = 0;

  for (;;)
  {
    /* First fill it all with blanks */
    sb_fill (r, ' ', colors.popup);

    /* Now draw the box */
    sb_box (r, boxtype, colors.popup);
    sb_caption (r, MSG_TXT (M_SEND_FILE), colors.popup);

    /* Now do the fields */
    sb_move_puts (r, 1, 2, MSG_TXT (M_ADDRESS_TO_SEND_TO));

    if (!addr_ok)
    {
      if (use_default)
      {
        addr = next_mail->mail_addr;

        if (hidedomains)
          addr.Domain = NULL;

        strntcpy (node, Full_Addr_Str (&addr), 45);
      }
      else
        node[0] = '\0';

      if (sb_input_address (r, 1, 22, node, 46))
        break;

      if (!find_address (node, &addr))
      {
        if (!did_it)
          did_it = -1;
        break;
      }
    }

    if (!nodefind (&addr, (int) 0))
    {
      sb_move_puts (r, 1, 22 + strlen (node) + 1, MSG_TXT (M_NODE_NOT_FOUND));
      sprintf (junk, MSG_TXT (M_SHOULD_I_USE), Full_Addr_Str (&addr));
      sb_move_puts (r, 2, 2, junk);

      yesno[0] = '\0';
      sb_input_chars (r, 2, strlen (junk) + 3, yesno, 1);

      if ((char) toupper (yesno[0]) == MSG_TXT (M_YES)[0])
      {
        sb_move_puts (r, 1, 22, space47);
        sb_move_puts (r, 2, 1, space66);
        sb_move_puts (r, 1, 23, Full_Addr_Str (&addr));
        addr_ok = 1;
      }
      else
        sb_move_puts (r, 2, 1, space66);
    }
    else
    {
      /* show systemName */
      sb_move_puts (r, 1, 22, space47);
      sprintf (SystemName, "%s, %s", Full_Addr_Str (&addr), newnodedes.SystemName);
      SystemName[47] = '\0';
      sb_move_puts (r, 1, 23, SystemName);
      addr_ok = 1;
    }

    /* RDH *** changes end */

    if (addr_ok)
    {
      /* Busy the node if we can. */
      if (flag_file (TEST_AND_SET, &addr, 0))
      {
        sb_move_puts (r, 2, 2, MSG_TXT (M_NODE_BUSY));
        wait_for_key (PER_SECOND);
        if (!did_it)
          did_it = -1;
        break;
      }

      busy = 1;
      rc = 1;

      /* Ask for the filename */
      /* RDH *** changes begin */

      while (rc)
      {
        sb_move_puts (r, 2, 2, MSG_TXT (M_FILE_TO_SEND));

        if (sb_input_fname (r, 2, 22, file, 46))  /* RDH *** 26-Apr-96 */
          break;

        rc = dfind (&fileinfo, file, 0);
        if (!rc)                /* File Found */
        {
          int ii = strlen (file);

          while (ii >= 0 && (file[ii] != DIR_SEPC) && (file[ii] != ':'))
            ii--;

          file[ii + 1] = '\0';
          strcat (file, fileinfo.name);
          dfind (&fileinfo, NULL, 2);  /* TE980429 */
        }

        sb_move_puts (r, 2, 22, file);
      }

      /* RDH *** changes end */

      /* r. hoerner changes start ;) */

      if (strlen (file) == 0)
      {
        did_it = -1;
        break;
      }

      /* r. hoerner changes end #) */

      /* Get the flavor or quit */
      sb_move_puts (r, 3, 2, MSG_TXT (M_SEND_FLAVOR));

      flavor[0] = '\0';
      // HJK 980805 - Input was not at right place in other languages
      sb_input_chars (r, 3, strlen (MSG_TXT (M_SEND_FLAVOR)) + 3, flavor, 1);
      flavor[0] = tolower (flavor[0]);

      /* Send the file if we can */
      if (flavor[0] != 'q')
      {
        if (mail_addsend (&addr, file, flavor) == 0)
          did_it = 1;
        else if (!did_it)
          did_it = -1;
      }

      /* Anything more to do? */
      sb_move_puts (r, 4, 2, MSG_TXT (M_SEND_MORE));

      /* RDH *** changes begin */

      more[0] = '\0';
      // HJK 980805 - Input was not at right place in other languages
      sb_input_chars (r, 4, strlen (MSG_TXT (M_SEND_MORE)) + 3, more, 1);

      if (more[0] == 'O')
      {
        addr_ok = 0;            /* user wants to enter new address  */
        use_default = 0;        /* and we don't use the default one */
        busy = 0;
        flag_file (CLEAR_FLAG, &addr, 0);
      }
      else if (more[0] != (char) toupper (MSG_TXT (M_YES)[0]))
        break;

      /* RDH *** changes end */
    }
  }

  /* If we busied something, get rid of it. */
  if (busy)
    flag_file (CLEAR_FLAG, &addr, 0);

  return (did_it);
}

int                             /* VRP 990826 */
mail_addsend (ADDRP pAddr, char *file, char *flavor)
{
  char *HoldName;
  char fname[PATHLEN];
  FILE *fp;

  if (do_flavor (flavor))
    return (-3);

  HoldName = HoldAreaNameMunge (pAddr);
  sprintf (fname, "%s%s.%slo", HoldName, Hex_Addr_Str (pAddr), flavor);

  fp = fopen (fname, append_binary);
  if (fp == NULL)
    return (-2);
  else
  {
    if (file)
      if (strlen (file))
        fprintf (fp, "%s\r\n", file);
    fclose (fp);
  }

  status_line (">File %s created", fname);

  return (0);
}

int
Do_Kill (BINK_SAVEP p, int use_default)
{
  REGIONP r;
  ADDR addr;
  char node[50];
  char sure[2];
  int did_it;

  if (p == NULL)
    return (0);

  r = p->region;

  if (next_mail == NULL)
    use_default = 0;

  /* First fill it all with blanks */
  sb_fill (r, ' ', colors.popup);

  /* Now draw the box */
  sb_box (r, boxtype, colors.popup);
  sb_caption (r, MSG_TXT (M_KILL_MAIL), colors.popup);

  /* Now do the fields */
  sb_move_puts (r, 1, 2, MSG_TXT (M_ADDRESS_TO_KILL));

  if (use_default)
  {
    addr = next_mail->mail_addr;

    if (hidedomains)
      addr.Domain = NULL;

    strntcpy (node, Full_Addr_Str (&addr), 48);
  }
  else
    node[0] = '\0';

  if (sb_input_address (r, 1, 19, node, 49))
    did_it = -1;
  else
  {
    sb_move_puts (r, 2, 2, MSG_TXT (M_KILL_ALL_MAIL));
    sb_puts (r, MSG_TXT (M_ARE_YOU_SURE));

    sure[0] = '\0';
    // HJK 980805 - Input was not at right place in other languages
    sb_input_chars (r, 2, strlen (MSG_TXT (M_KILL_ALL_MAIL)) + strlen
                    (MSG_TXT (M_ARE_YOU_SURE)) + 3, sure, 1);

    if (sure[0] == (char) toupper (MSG_TXT (M_YES)[0]))
      did_it = kill_node (node);
    else
      did_it = 0;
  }

  return (did_it);
}

int
Do_Poll (BINK_SAVEP p, int use_default)
{
  char node[41];
  ADDR addr;
  REGIONP r;

  if (p == NULL)
    return (FALSE);

  r = p->region;

  if (next_mail == NULL)
    use_default = 0;

  /* First fill it all with blanks */
  sb_fill (r, ' ', colors.popup);

  /* Now draw the box */
  sb_box (r, boxtype, colors.popup);
  sb_caption (r, MSG_TXT (M_NODE_TO_POLL), colors.popup);

  /* Now do the fields */
  sb_move_puts (r, 1, 2, MSG_TXT (M_POLL_WHOM));

  if (use_default)
  {
    addr = next_mail->mail_addr;

    if (hidedomains)
      addr.Domain = NULL;

    strntcpy (node, Full_Addr_Str (&addr), 39);
  }
  else
    node[0] = '\0';

  if (sb_input_address (r, 1, 14, node, 40))
    return (FALSE);

  return (find_address (node, &next_addr));
}

static int LOCALFUNC
kill_node (char *node)
{
  ADDR addr;
  char *HoldName;
  char *p;
  char fname[PATHLEN];
  FILE *in;
  char s[PATHLEN], *s2;
  struct FILEINFO fileinfo;
  int j;

  if (find_address (node, &addr))
  {
    if (flag_file (TEST_AND_SET, &addr, 0))
      return (-1);

    HoldName = HoldAreaNameMunge (&addr);

    sprintf (fname, "%s%s.*", HoldName, Hex_Addr_Str (&addr));
    j = 0;
    while (!dfind (&fileinfo, fname, j))
    {
      j = 1;

      /* Don't delete the .bsy flags yet */

      if ((p = strchr (fileinfo.name, '.')) != NULL)
      {
        if (strcmp (p, ".bsy") == 0)
          continue;
      }

      if (addr.Point != 0)
      {
        sprintf (fname, "%s%04hx%04hx.pnt" DIR_SEPS "%s",
                 HoldName, addr.Net, addr.Node, fileinfo.name);
      }
      else
        sprintf (fname, "%s%s", HoldName, fileinfo.name);

      /* CFS 970629 - truncate/kill files when required */
      in = share_fopen (fname, read_binary_plus, DENY_WRITE);
      if (in)
      {
        while (!feof (in))
        {
          s[0] = 0;
          fgets (s, PATHLEN - 1, in);
          s2 = s;
          while (*s2)
          {
            if (*s2 < ' ')
            {
              *s2 = 0;
              break;
            }

            s2++;
          }

          if (s[0] == TRUNC_AFTER)
          {
            FILE *tmp;

            CLEAR_IOERR ();
            tmp = fopen (s + 1, write_binary);
            fclose (tmp);
          }
          else if (s[0] == SHOW_DELETE_AFTER || s[0] == DELETE_AFTER)
          {
            CLEAR_IOERR ();
            unlink (s + 1);
          }
        }                       /* while */

        fclose (in);
      }

      unlink (fname);
    }

    if (j)
      dfind (&fileinfo, NULL, 2);

    flag_file (CLEAR_FLAG, &addr, 0);
    return (0);
  }

  /* else */
  return (-1);
}

static void LOCALFUNC
call_tries (ADDRP baddr, int *calls, int *badcalls)
{
  int res;
  int i, j;
  struct FILEINFO bad_dta;
  char *HoldName;
  char fname[PATHLEN];
  FILE *fhandle;

  HoldName = HoldAreaNameMunge (baddr);
  sprintf (fname, "%s%s.$$?", HoldName, Hex_Addr_Str (baddr));
  j = (int) strlen (fname) - 1; /* Point at ?          */
  res = -1;                     /* Initialize to fail  */

  i = 0;                        /* This says findfirst */
  while (!dfind (&bad_dta, fname, i))  /* as long as we match */
  {
    i = 1;                      /* switch to findnext */
    if (isdigit (bad_dta.name[11]))  /* is there a digit?   */
    {
      fname[j] = bad_dta.name[11];  /* Yes, copy to fname  */
      res = fname[j] - '0';     /* Save it for testing */
      break;                    /* Get out of while    */
    }
  }

  if (i)
    dfind (&bad_dta, NULL, 2);

  /* Initialize the return values */
  *calls = *badcalls = 0;

  /* Is it automatically ok (no .$$ file there) ? */
  if (res == -1)
    return;

  /* We now have the number of bad calls (calls with carrier) */
  *badcalls = res;

  /* Open the file and find out how many total calls were made */
  fhandle = fopen (fname, read_binary);
  if (fhandle != NULL)
  {
    res = 0;
    fread ((char *) &res, sizeof (int), 1, fhandle);

    fclose (fhandle);
  }

  *calls = res;
  return;
}

#if 0
static char LOCALFUNC xlat_flavor (char flavor);

static char LOCALFUNC
xlat_flavor (char flavor)
{
  char *real_flavor = "chdn";
  char *user_flavor;

  user_flavor = MSG_TXT (M_CHDN_FLAGS);
  for (;;)
  {
    if (!*real_flavor)
      break;

    if (*user_flavor == flavor)
      return *real_flavor;

    user_flavor++;
    real_flavor++;
  }

  return (0);
}
#endif

typedef struct SEntry
{
  char Name[28];
  char Nodestr[24];
  char Flgstr[20];
  struct SEntry *Priv;
  struct SEntry *Next;
}
SENTRY, *pSENTRY;

static int LOCALFUNC
  get_node_adr_by_sysopname (char *, char *, int, int, SENTRY **);

void LOCALFUNC
dispentry (REGIONP r, SENTRY * pActEntry, int line)
{
  sb_move_puts (r, line, 1, pActEntry->Name);
  sb_move_puts (r, line, 25, pActEntry->Nodestr);
  sb_move_puts (r, line, 46, pActEntry->Flgstr);
}


void LOCALFUNC
lookupbox (REGIONP r, pSENTRY Entries, int start)
{
  int i;
  SENTRY *TmpEntry;

  /* First fill it all with blanks */
  sb_fill (r, ' ', colors.popup);

  /* Now draw the box */
  sb_box (r, boxtype, colors.popup);
  sb_caption (r, MSG_TXT (M_SELECTANODE), colors.popup);
  sb_move_puts (r, 1, 1, MSG_TXT (M_LOOKUPTITLE));

  TmpEntry = Entries;

  /* skip to start */
  for (i = 0; (i < start) && (TmpEntry != NULL); i++)
    TmpEntry = TmpEntry->Next;

  /* display entries */
  for (i = 0; (i < 12) && (TmpEntry != NULL); i++)
  {
    dispentry (r, TmpEntry, 2 + i);
    TmpEntry = TmpEntry->Next;
  }
}

/* RDH *** changes 22.Jun.96 */
static int LOCALFUNC
get_NodeAdr (char *in)
{
  BINK_SAVEP p;                 /* pointer to save scrren   */
  REGIONP r;                    /* regionpointer            */
  SENTRY SelEntry;
  SENTRY *pActEntry;            /* pointer to actuell Entry */
  SENTRY *pTmpEntry;            /* Pointer to Main Entry    */
  char *tmpptr;
  int c;
  int i, count;
  int row, entry, retc, done;
  long t_idle;
  char *hid_ptr = NULL;         /* VRP 990903 - is there a hidden line? */

  row = entry = done = 0;
  retc = 1;

  memset (&SelEntry, '\0', sizeof (SENTRY));
  SelEntry.Priv = NULL;
  SelEntry.Next = NULL;
  pActEntry = &SelEntry;
  tmpptr = skip_blanks (in);

  if (!isdigit (*tmpptr) && (*tmpptr != '.'))  /* Sysopname used */
  {
    if (V7Initialize () != V7_SUCCESS)
      return (1);

    /* AW 980124 search in all nodelists */
    count = 0;
    if (domain_nodelist[0] == NULL)
    {
      count = get_node_adr_by_sysopname (in, nodelist_base, 0, count, &pActEntry);
    }
    else
    {
      for (i = 0; (i < DOMAINS) && ((tmpptr = domain_nodelist[i]) != NULL);
           i++)
      {
        if (i == 0 || stricmp (tmpptr, domain_nodelist[0]))
          count = get_node_adr_by_sysopname (in, tmpptr, i, count, &pActEntry);
      }
    }

    V7Finish ();
    if (!count)
      return 1;

    p = sb_save (5, 5, 15, 70);
    if (p == NULL)
      return (1);

    r = p->region;
    lookupbox (r, &SelEntry, entry);

    /* mark entry */
    sb_move (r, 2, 1);
    sb_wa (r, colors.hold, 68);
    sb_show ();

    t_idle = timerset (PER_MINUTE);
    while (!done && !timeup (t_idle) && (PEEKBYTE () == (short) 0xffff))
    {
      if (KEYPRESS ())
      {
        t_idle = timerset (30 * PER_SECOND);

        c = (int) KbRemap (FOSSIL_CHAR ());
        switch ((unsigned) c)
        {
        case F_PEND_HOME:
          if (entry)
          {
            entry = row = 0;
            lookupbox (r, &SelEntry, entry);
          }
          else if (row)
          {
            sb_move (r, 2 + row, 1);
            sb_wa (r, colors.popup, 68);
            row = 0;
          }
          break;

        case F_PEND_END:
          if (entry + 11 < count - 1)
          {
            entry = count - 12;
            row = 11;
            lookupbox (r, &SelEntry, entry);
          }
          else if (row < 11)
          {
            sb_move (r, 2 + row, 1);
            sb_wa (r, colors.popup, 68);
            if (row + 11 > count - 1)
              row = count - 1;
            else
              row = 11;
          }
          break;

        case F_PEND_PGUP:
          if (row)
          {
            sb_move (r, 2 + row, 1);
            sb_wa (r, colors.popup, 68);
            row = 0;
          }
          else if (entry)
          {
            entry -= 11;
            if (entry < 0)
              entry = 0;
            lookupbox (r, &SelEntry, entry);
          }
          break;

        case F_PEND_UPAR:
          if (row)
          {
            sb_move (r, 2 + row, 1);
            sb_wa (r, colors.popup, 68);
            row--;
          }
          else if (entry)
            lookupbox (r, &SelEntry, --entry);
          break;

        case F_PEND_PGDN:
          if (row < 11)
          {
            sb_move (r, 2 + row, 1);
            sb_wa (r, colors.popup, 68);
            row = 11;
            if (row + entry > count - 1)
              row = count - entry - 1;
          }
          else
          {
            entry += 11;
            if ((row + entry > count - 1))
              entry = count - row - 1;
            lookupbox (r, &SelEntry, entry);
          }
          break;

        case F_PEND_DNAR:
          if ((row < 11) && (row + entry < count - 1))
          {
            sb_move (r, 2 + row, 1);
            sb_wa (r, colors.popup, 68);
            row++;
          }
          else if ((row + entry < count - 1))
            lookupbox (r, &SelEntry, ++entry);
          break;

        case F_ZOOM_NODEINFO:  /* ALT-I info about node */
          {
            MAILP nCur = next_mail;
            char tempstr[26];

            if (next_mail == NULL)
              next_mail = (MAILP) calloc (1, sizeof (MAIL));

            if (next_mail == NULL)
              break;

            pActEntry = &SelEntry;

            for (i = 0; i < (entry + row); i++)
              pActEntry = pActEntry->Next;

            if (pActEntry == NULL)
              break;

            sprintf (tempstr, pActEntry->Nodestr);
            tempstr[25] = 0;
            sscanf (tempstr, "%hd:%hd/%hd.%hd",
                    &next_mail->mail_addr.Zone,
                    &next_mail->mail_addr.Net,
                    &next_mail->mail_addr.Node,
                    &next_mail->mail_addr.Point);
            next_mail->mail_addr.Domain = NULL;

            /* VRP 990903 start */

            /*
             * Hidden line support
             *
             */

            hid_ptr = strstr (pActEntry->Nodestr, "(h");
            if (hid_ptr != NULL)
            {
              hidden_no = atoi (&hid_ptr[2]);
            }
            else
            {
              hidden_no = 0;    /* Main line */
            }

            /* VRP 990903 end */

            sb_popup (SB_ROWS - 10, 1, 8, 78, Do_Info, 0);

            if (nCur == NULL)
            {
              free (next_mail);
              next_mail = NULL;
            }
          }
          break;

          /* HJK 98/07/20 - Why should you want to use readdress here? */
          /* There is nothing to readdress */

          /* case F_ZOOM_ADDRESS:   // readdress
           * {
           *   char  tempstr[26];
           *
           *   pActEntry = &SelEntry;
           *
           *   for (i=0; i<(entry+row); i++)
           *     pActEntry = pActEntry->Next;
           *
           *   if (pActEntry == NULL)
           *     break;
           *
           *   sprintf(tempstr,"%s",pActEntry->Nodestr);
           *   tempstr[25] = 0;
           *   sscanf (tempstr, "%hd:%hd/%hd.%hd",
           *   &other_addr.Zone,
           *   &other_addr.Net,
           *   &other_addr.Node,
           *   &other_addr.Point);
           *   other_addr.Domain = NULL;
           *
           *   if (sb_popup (10, 5, 4, 70, mail_addr, 1))
           *   {
           *     BINK_SAVEP tmp = NULL;
           *
           *     tmp = sb_hide_popup(p);
           *     xmit_reset(0,1);
           *     oldcur    = NULL;
           *     next_mail = mail_top;
           *     curl      = 2;
           *     zoom_window (zoomwin, next_mail, next_mail);
           *     if (tmp != NULL)
           *     sb_show_popup(p,tmp);
           *   }
           * }
           *
           * break;
           */

        default:
          c &= 0x00ff;
          if (c == 0x0d)
          {
            done = 1;
            retc = 0;
          }
          else if (c == ESC)
            done = 1;

          break;

        }                       /* switch input */

        sb_move (r, 2 + row, 1);
        sb_wa (r, colors.hold, 68);
        sb_show ();
      }                         /* if keypressed */

      time_release ();
      show_alive ();
    }                           /* while !done */

    if (!retc)
    {
      pActEntry = &SelEntry;

      for (i = 0; i < entry; i++)
        pActEntry = pActEntry->Next;

      for (i = 0; i < row; i++)
        pActEntry = pActEntry->Next;

      strcpy (in, pActEntry->Nodestr);
    }

    /* Free all allocated Menory */
    pActEntry = &SelEntry;
    pTmpEntry = pActEntry->Next;

    /* Now, free the list */
    while (pTmpEntry != NULL)
    {
      pActEntry = pTmpEntry;
      pTmpEntry = pTmpEntry->Next;
      free (pActEntry);
    }

    sb_restore (p);
  }                             /* if isdigit */
  else
    retc = 0;

  return (retc);
}

/* AW 980124 */
/* read all matching nodelist entries for one nodelist and add them to the */
/* linked list                                                             */

static int LOCALFUNC
get_node_adr_by_sysopname (char *searchname, char *nodelistbase,
                           int use_no_sysopndx, int count,
                           SENTRY ** ppActEntry)
{
  V7RCB V7rcb;                  /* request block            */
  V7NODE V7node;                /* V7 format node record    */
  int V7rc;                     /* return code              */
  SENTRY *pActEntry = (*ppActEntry);

  /* VRP 990901 start */

  /*
   * Hidden Lines Support
   */

  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;
  word Real_Modem, Real_Fido;
  ADDR addr, curaddr;
  int i;

  memset (&addr, '\0', sizeof (ADDR));
  addr.Domain = NULL;
  memcpy (&curaddr, &addr, sizeof (ADDR));

  /* VRP 990901 end */

  memset (&V7rcb, '\0', sizeof (V7RCB));
  V7rcb.pszPath = (PSZ) net_info;  /* take nodelist path */
  V7rcb.pszName = (PSZ) nodelistbase;  /* add in the filename */
  V7rcb.whichindex = WI_sdx;    /* sysop index lookup */
  V7rcb.miscflags |= (version7 > 1) ? 1 : 0;
  V7rcb.miscflags |= (sysopndx && !use_no_sysopndx) ? MF_sysopndx : 0;
  V7rcb.pV7Node = &V7node;
  memset (&V7node, '\0', sizeof (V7NODE));
  strntcpy (V7node.SysopName, searchname, 26);

  V7rc = V7FindFirst (&V7rcb);

  while (V7rc == V7_SUCCESS)
  {
    if (count++)
    {
      pActEntry->Next = calloc (1, sizeof (SENTRY));
      if (pActEntry->Next == NULL)
        break;
      pActEntry->Next->Priv = pActEntry;
      pActEntry->Next->Next = NULL;
      pActEntry = pActEntry->Next;
    }

    /* VRP 990902 start */

    /*
     * Some minor changes to show real (if overriden) modem and fido flags
     *
     */

    curaddr.Zone = V7node.ZoneNumber;
    curaddr.Net = V7node.NetNumber;
    curaddr.Node = V7node.NodeNumber;
    curaddr.Point = (V7node.NodeFlags & B_point) ? V7node.HubNode : 0;
    Real_Modem = V7node.ModemType;
    Real_Fido = V7node.NodeFlags;

    GetAltModemflag (&Real_Modem, &curaddr);
    GetAltFidoFlag (&Real_Fido, &curaddr);

    strcpy (pActEntry->Name, V7node.rawSysopName);
    sprintf (pActEntry->Nodestr, "%2d:%d/%d.%d",
             curaddr.Zone, curaddr.Net, curaddr.Node, curaddr.Point);
    sprintf (pActEntry->Flgstr, "%4s  %5u  %3d  %c",
             (Real_Fido & B_CM) ? "CM" : "  ",
             V7node.BaudRate,
             Real_Modem,
             (V7node.Password[0] != '\0') ? '+' : ' ');

    V7rc = V7FindNext (&V7rcb);

    /*
     * Check if there are hidden lines for the address found
     *
     */

    if (SameAddress (&curaddr, &addr))
    {
      continue;
    }

    memcpy (&addr, &curaddr, sizeof (ADDR));

    for (hidden = hidden1; hidden; hidden = hidden->next)
    {
      if (SameAddress ((ADDR *) hidden, &addr))
      {
        for (hidden_line = hidden->hidden_line1, i = 1;
             hidden_line;
             hidden_line = hidden_line->next, i++)
        {
          if (count++)
          {
            pActEntry->Next = calloc (1, sizeof (SENTRY));
            if (pActEntry->Next == NULL)
              break;
            pActEntry->Next->Priv = pActEntry;
            pActEntry->Next->Next = NULL;
            pActEntry = pActEntry->Next;
          }

          strcpy (pActEntry->Name, V7node.rawSysopName);
          sprintf (pActEntry->Nodestr, "%2d:%d/%d.%d (h%d)",
                   addr.Zone, addr.Net, addr.Node, addr.Point, i);
          sprintf (pActEntry->Flgstr, "%4s  %5u  %3d  %c",
                   (hidden_line->fido & B_CM) ? "CM" : "  ",
                   V7node.BaudRate,
                   hidden_line->modem,
                   (V7node.Password[0] != '\0') ? '+' : ' ');
        }

        break;
      }
    }

    /* VRP 990902 end */
  }

  V7FindClose (&V7rcb);
  (*ppActEntry) = pActEntry;

  return count;
}


/* MR 970517                   */
/* function to get an address  */

static int LOCALFUNC
sb_input_address (REGIONP win, short row, short col, char *str, short len)
{
  for (;;)
  {
    if (sb_input_chars (win, row, col, str, len))
      return 1;
    else
    {
      if (version7)
      {
        if (get_NodeAdr (str))
        {
          sb_move_puts (win, row,
                        col + len - strlen (MSG_TXT (M_NODE_NOT_FOUND)),
                        MSG_TXT (M_NODE_NOT_FOUND));
          wait_for_key (PER_SECOND);
          str[0] = '\0';
        }
        else
          return 0;
      }
      else
        return 0;
    }
  }
}

/* $Id: outbound.c,v 1.18 1999/09/27 20:51:37 mr Exp $ */
