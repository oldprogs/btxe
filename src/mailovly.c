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
 * Filename    : $Source: E:/cvs/btxe/src/mailovly.c,v $
 * Revision    : $Revision: 1.15 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:29 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Mail Control Routines
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static int LOCALFUNC xmit_install (MAILP, ADDRP);
static int LOCALFUNC any_mail (MAILP);
static void LOCALFUNC do_xmit_line (char *, MAILP);
static void LOCALFUNC xmit_sort (void);
static void LOCALFUNC addr_sort (void);
static void LOCALFUNC xmit_delete_one (void);
static int LOCALFUNC CanSendMail (MAILP p);
char *mail_status_chars (unsigned short, char *msc);
static MAILP LOCALFUNC xmit_find (MAILP, ADDRP);
static MAILP LOCALFUNC kill_one (MAILP, ADDRP);
static void LOCALFUNC do_for_all_domains (MAILP, MAILP (LOCALFUNC *) (MAILP, ADDRP));
static int LOCALFUNC xmit_perform_reread (int, char *);
static int LOCALFUNC xmit_perform_rescan (int, char *, time_t);
static void LOCALFUNC xmit_clearqueue (void);

static char *HoldFmtStr = "%-19.19s %3d %4.4s %3d %5s%c";
static char *HoldNoSize = "%-32.32s %5s%c";


static void LOCALFUNC
do_for_all_domains (MAILP p, MAILP (LOCALFUNC * callback) (MAILP, ADDRP))
{
  int k, zone;
  char *q, *s;
  char *domain;
  char *HoldName;
  char pointspec[PATHLEN];
  ADDR tmp;

#ifdef CACHE

  DIRBUF zone_dir;
  DIRBUF pnt_dir;

#else

  int j;
  struct FILEINFO zone_dir;
  struct FILEINFO pnt_dir;

#endif
  long longzone;

  /* Initialize domain to scan. Choose (of course) the
   * domain of our primary address.
   * This domain is special in that its outbound is hold_area.
   * All the other domains use their abbreviation as the name
   * of their outbounds. */

  k = 0;
  domain = domain_name[0];
  strcpy (pointspec, hold_area);
  q = &(pointspec[strlen (pointspec)]) - 1;

  do
  {
    /* Initialize scan of zones in this domain. Using findfirst/findnext,
     * get all the matching directories. */

#ifndef __unix__
    strcpy (q, ".*");
#else
    strcpy (q, "*");            /* there is a difference between outb* and outb.* */
#endif

#ifdef CACHE
    dirinit (&zone_dir, pointspec);
    while (!dirread (&zone_dir))
    {
#else
    j = 0;
    while (!dfind (&zone_dir, pointspec, j))
    {

#ifdef __unix__
      /* CEH 990412:
       * By finding outbound* and not outbound.* we possibly get more directories
       * that we want. Example:
       * The outbound is /var/spool/fido/out/ and an additional directory
       * /var/spool/fido/outxyz/ does exist. This is found, too. It does not
       * contain a dot, so the main outbound will be rescanned twice.
       * We avoid this by comparing the length of the found name with the name
       * scanned for (it should be one character shorter because of the asterics).
       * Note that the linux-version does NOT use the outbound cache.
       */

      int lenpointspec;
      char *p1;

      if (!j)
      {
        p1 = strrchr (pointspec, DIR_SEPC);
        if (p1 == NULL)
          p1 = pointspec;
        else
          p1++;
        lenpointspec = strlen (p1) - 1;
      }
#endif
      j = 1;                    /* Flip findfirst/findnext to findnext */
#ifdef __unix__
      p1 = strrchr (zone_dir.name, '.');
      if (p1 != NULL)
      {
        if (lenpointspec != strlen (zone_dir.name) - strlen (p1))
          continue;
      }
      else
      {
        if (lenpointspec != strlen (zone_dir.name))
          continue;
      }
#endif

#endif

      /* We have a match on the outbound spec. Make sure it's a directory.
       * Then:
       *   1) If no extension, we may only use it if this is alias 0
       *   2) If an extension, it must be a 3-digit hex number
       * If the extension passes one of these tests, get to work! */

      if (!(zone_dir.attr & FA_SUBDIR))
        continue;

      q = strchr (zone_dir.name, '.');

      if (q == NULL)
      {
        if (k != 0)
          continue;
        zone = (int) alias->Zone;
      }
      else
      {
        s = ++q;
        longzone = strtol (q, &q, 16);
        if ((s + 3) != q)
          continue;
        zone = (int) longzone;

        /* To avoid duplicating the primary scan,
         * make sure that this isn't "domainname[0].zone[0]". */

        if (!k && zone == (int) alias->Zone)
          continue;
      }

      /* OK. We have a domain, an outbound directory, and a zone.
       * That means there's an outbound to scan.
       * Start by scanning the nodes. */

      tmp.Zone = zone;
      tmp.Domain = domain;
      tmp.Net = tmp.Node = tmp.Point = 0;
      p = callback (p, &tmp);

      /* Now we do the points contained in this outbound. */

      if (pvtnet <= 0)
      {
#ifndef CACHE
        int f = 0;

#endif
        HoldName = HoldAreaNameMunge (&tmp);
        sprintf (pointspec, "%s*.pnt", HoldName);

#ifdef CACHE
        dirinit (&pnt_dir, pointspec);
        while (!dirread (&pnt_dir))
        {
#else
        while (!dfind (&pnt_dir, pointspec, f))
        {
          f = 1;
#endif
          if (sscanf (pnt_dir.name, "%04hx%04hx.", &(tmp.Net), &(tmp.Node)) != 2)
            continue;
          tmp.Point = 1;
          p = callback (p, &tmp);
        }                       /* got one */

#ifdef CACHE
        dirquit (&pnt_dir, 0);
#else
        if (f)
          dfind (&pnt_dir, NULL, 2);
#endif
      }
    }                           /* while !dfind (...) to get outbounds */

#ifdef CACHE
    dirquit (&zone_dir, 0);
    cacheflush ();
#else
    if (j)
      dfind (&zone_dir, NULL, 2);
#endif

    /* See if there are any more domains. If so, set up the right name
     * for the outbound, so we can find 'em. */

    if (k < DOMAINS - 1)
    {
      k++;
      if ((domain = domain_name[k]) != NULL)
      {
        /* CEH 19990514: this is no longer needed */
        /* *domain_loc = '\0'; */

        strcpy (pointspec, domain_area);
        q = &(pointspec[strlen (pointspec)]);
        s = domain_abbrev[k];
        if (s != NULL)
          while (*s)
            *q++ = *s++;
      }
    }
    else
      domain = NULL;
  }
  while (domain != NULL);

}

static int LOCALFUNC
CanSendMail (MAILP p)
{
  unsigned short stop;

  stop = p->mailtypes & (MAIL_TOOBAD | MAIL_UNKNOWN | MAIL_CANTDIAL |
                         MAIL_COST | MAIL_QSMALL);
  return ((p->mailtypes & MAIL_WILLGO) && (!stop));
}

void
xmit_sameplace (void)
{
  MAILP p, p1;

  /* Find the guy we just gave mail to */

  p = find_mail (&remote_addr);
  remote_addr.Zone = remote_addr.Net = remote_addr.Node = remote_addr.Point = 0;
  remote_addr.Domain = NULL;

  if (p == NULL)                /* He is not there */
    return;

  /* Save our current pointer */

  p1 = next_mail;
  if (p != next_mail)
  {
    /* If it is not the one we just gave mail to, save ptr and delete */
    next_mail = p;
    xmit_delete ();
    next_mail = p1;
  }
  else
  {
    /* It was the guy at the head of the list, so just delete him */
    xmit_delete ();
  }

  /* If we came in with a null, leave with a null */

  if (p1 == NULL)
    next_mail = NULL;

  return;
}

MAILP
find_mail (ADDRP address)
{
  MAILP p;

  for (p = mail_top; p != NULL; p = p->ptrval.ptrnp.next)
  {
    if ((p->mail_addr.Zone == address->Zone) &&
        (p->mail_addr.Net == address->Net) &&
        (p->mail_addr.Node == address->Node) &&
        (p->mail_addr.Point == address->Point) &&
        ((p->mail_addr.Domain == address->Domain) ||
         ((p->mail_addr.Domain == my_addr.Domain) &&
          (address->Domain == NULL))))
      break;
  }

  return (p);
}

static void LOCALFUNC
xmit_setflags (MAILP p1, int listed, byte ModemType, word NodeFlags, word RealCost)
{
  MDM_TRNS *m;                  /* MB 93-11-27 */
  long totcost = 0, dummy;
  word cost = 0;
  char *ename;
  short C_Mail, CM;
  short C_event, K_event, M_event, H_event;

  /* VRP 990820 start */

  short is_online_now;
  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;
  int wday, hours, mins, secs, ths;

  is_online_now = 0;
  status_line (">Mail to: %s", Full_Addr_Str (&p1->mail_addr));

  /* VRP 990820 end */

  /* ------------------------------------------------------------------- */
  /* If unlisted system, don't send                                      */
  /* ------------------------------------------------------------------- */
  if (!listed)
  {
    p1->mailtypes |= MAIL_UNKNOWN;
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "system unlisted, no send.");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* Can we dial that system?                                            */
  /* ------------------------------------------------------------------- */
  p1->mailtypes &= ~MAIL_UNKNOWN;

  p1->mailtypes &= ~MAIL_CANTDIAL;  /* rhoerner: default is "dialable" */

  /* if modem type is undialable, don't call *//* MB 93-11-27 */
  /* changed by r. hoerner */
  if ((m = find_mdm_trans (ModemType)) != NULL)  /* if 'ModemTrans' defined */
  {
    if (!m->pre[0])             /* is there no predial */
      p1->mailtypes |= MAIL_CANTDIAL;
  }

  if (p1->mailtypes & MAIL_CANTDIAL)
  {
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "cannot dial to that system");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* the following section checks, whether the current event setting     */
  /* allows sending of mail.                                             */
  /* ------------------------------------------------------------------- */
  /* This is what bt_ref.doc says about the various flavors:             */
  /*                                                                     */
  /* .OUT . . . means it is a Normal packet.                             */
  /* .HUT . . . Hold this packet for pickup by the remote system.        */
  /* .CUT . . . The other system can receive Continuous Mail.            */
  /* .DUT . . . Direct, the other system can NOT receive Continuous Mail */
  /* ------------------------------------------------------------------- */
  /* This definition of "DUT" means, that we can send DIRECT mail        */
  /* in a MAT_NOMAIL24 event ONLY                                        */
  /*                                                                     */
  /* Common practice is to flag a PKT "direct" only to prevent the       */
  /* tosser to toss it to another nodes (your HUBs) packet.              */
  /*                                                                     */
  /* Common practice is to rename a PKT to "CUT" to force a direct and   */
  /* immediate send to the other system ("Crashmail")                    */
  /*                                                                     */
  /* MAT_NOMAIL24: send to everybody ("Not Only MAIL24")                 */
  /* MAT_NOCM    : do not send to CM nodes now                           */
  /* MAT_CM      : send C??-Mail only, regard the costs                  */
  /* MAT_HIPROCM : send C?? mail without regarding the costs             */
  /* ------------------------------------------------------------------- */
  /* That's all theory. What does BinkleyTerm 2.60 do? This one:         */
  /*                                                                     */
  /*  "Hold" or REQ?                             no send                 */
  /*  if ( H_event &&  C_Mail && (CM || online))    send                 */
  /*  if ( C_event && !C_Mail)                   no send                 */
  /*  if (!M_event &&           (CM || online))     send                 */
  /*  if ( K_event &&            CM)    no send                          */
  /*  if ( R_event)                     no send                          */
  /*                                                                     */
  /*  if (L defined) check costs, conditionally "no send"                */
  /*                                                                     */
  /*  if (bad_call())                   no send                          */
  /*  if (not enough mail)              no send                          */
  /*                                                                     */
  /*  else                                 send!                         */
  /* ------------------------------------------------------------------- */
  /*  Note, that BinkleyTerm 2.60 will send D?? and O?? pakets if the    */
  /*  cost and/or 'C' event does not prevent it.                         */
  /* ------------------------------------------------------------------- */
  /*  Note, that BinkleyTerm 2.60 will never send mail to non-CM nodes,  */
  /*  if the "M" event is protected by a "L" (cost) and the actual cost  */
  /*  definition says: to expensive.                                     */
  /*  I changed it so, that a "M H" event will do the same for C-Mail to */
  /*  non-CM nodes. Other mail can still stay cost protected             */
  /* ------------------------------------------------------------------- */

  /* ------------------------------------------------------------------- */
  /* See if we have not only requests / mail on hold                     */
  /* ------------------------------------------------------------------- */

  // MR 970724 replaced:
  //
  //  if ((e_ptrs[cur_event].behavior & MAT_NOOUTREQ)
  //  &&   p1->mailsize                  /* we *have* mail (not empty CLO) */
  //  &&  (p1->mailsize == p1->req_size))/* but that's only requests       */
  //  {
  //      status_line(">set_flags: %s : %s",Full_Addr_Str(&p1->mail_addr),
  //                  "requests only, No request event. no send.");
  //      return;
  //  }
  //
  // with:

  if (p1->mailtypes == (p1->mailtypes & (MAIL_REQUEST | MAIL_HOLD)))
  {
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "only requests or mail on hold, no send.");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* If there's no event, set all but "HOLD" mail to 'go'                */
  /* ------------------------------------------------------------------- */
  if (cur_event < 0)            /* remember: hold mail is already handled! */
  {
    p1->mailtypes &= ~MAIL_COST;
    p1->mailtypes &= ~MAIL_QSMALL;
    p1->mailtypes |= MAIL_WILLGO;
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "no event setup, send!");
    return;
  }

  /* ------------------------------------------------------------------- */
  /*  else: make it easy for you                                         */
  /* ------------------------------------------------------------------- */
  CM = NodeFlags & B_CM;

  /* VRP 990820 start */

  if (!CM)
  {
    if (GetAltOnLineTime (&newnodedes.online_start,
                          &newnodedes.online_end,
                          &p1->mail_addr) != 0)
    {
      CM = 1;
      NodeFlags |= B_CM;
      newnodedes.NodeFlags |= B_CM;
    }
    else
    {
      is_online_now = IsOnLine (newnodedes.online_start, newnodedes.online_end);
      status_line (">OnLine: %s : %s", Full_Addr_Str (&p1->mail_addr),
                   ((is_online_now) ? "Yes" : "No"));
    }
  }

  /*
   * If main line isn't online now check if there are online hidden lines
   *
   */

  if (is_online_now == 0)
  {
    for (hidden = hidden1; hidden; hidden = hidden->next)
    {
      status_line (">Hidden line(s) for %d:%d/%d.%d found",
                   hidden->Zone, hidden->Net, hidden->Node, hidden->Point);

      if (SameAddress ((ADDR *) hidden, &p1->mail_addr))
      {
        for (hidden_line = hidden->hidden_line1;
             hidden_line;
             hidden_line = hidden_line->next)
        {
          dostime (&wday, &hours, &mins, &secs, &ths);
          wday = (short) weekdays[wday % 7];

          status_line (">Phone: %s. Start: %d. End: %d. Flags: %04X. Days: %04X.",
                       hidden_line->num, hidden_line->start,
                       hidden_line->end, hidden_line->fido,
                       hidden_line->days);

          if ((hidden_line->days & wday) &&
              ((hidden_line->fido & B_CM) ||
               IsOnLine (hidden_line->start, hidden_line->end)))
          {
            is_online_now = 1;
            break;
          }
        }

        break;
      }
    }
  }

  /* VRP 990820 end */

  C_Mail = p1->mailtypes & MAIL_CRASH;  /* CUT or CLO  */

  C_event = e_ptrs[cur_event].behavior & MAT_CM;  /* 'C' event */
  M_event = e_ptrs[cur_event].behavior & MAT_NOMAIL24;  /* 'M' event */
  K_event = e_ptrs[cur_event].behavior & MAT_NOCM;  /* 'K' event */
  H_event = e_ptrs[cur_event].behavior & MAT_HIPRICM;  /* 'H' event */

  /* ------------------------------------------------------------------- */
  /* 'H' event, node is CM, we have crashmail:send                       */
  /* ------------------------------------------------------------------- */

  if (H_event && (CM || is_online_now) && C_Mail)  /* VRP 990820 - add is_online_now */
  {
    p1->mailtypes &= ~MAIL_COST;
    p1->mailtypes &= ~MAIL_QSMALL;
    p1->mailtypes |= MAIL_WILLGO;
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "H event, CM, crash mail, send!");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* 'H' event and 'M' event, node is !CM, we have crashmail:send        */
  /* ------------------------------------------------------------------- */
  if (H_event && M_event && !CM && C_Mail)
  {
    p1->mailtypes &= ~MAIL_COST;
    p1->mailtypes &= ~MAIL_QSMALL;
    p1->mailtypes |= MAIL_WILLGO;
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "'H M'event, !CM, crash mail, send!");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* If this is a "receive only" event then abort                        */
  /* ------------------------------------------------------------------- */
  if (e_ptrs[cur_event].behavior & MAT_NOOUT)
  {
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "receive only event, no send.");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* 'C' event, we have no crashmail: no send                            */
  /* ------------------------------------------------------------------- */
  if (C_event && !C_Mail)
  {
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "'C' event, no crash mail, no send!");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* not a 'M'event and node not CM                                      */
  /* ------------------------------------------------------------------- */
  if (!M_event && !CM)
  {
    if (is_online_now)          /* VRP 990820 start */
    {
      p1->mailtypes &= ~MAIL_COST;
      p1->mailtypes &= ~MAIL_QSMALL;
      p1->mailtypes |= MAIL_WILLGO;

      status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                   "not 'M' event, not CM, but online, send!");
      return;
    }                           /* VRP 990820 end */
    else
    {
      status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                   "not 'M' event, not CM, no send!");
      return;
    }
  }

  /* ------------------------------------------------------------------- */
  /* 'K'event and node CM                                                */
  /* ------------------------------------------------------------------- */
  if (K_event && CM)
  {
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "non-CM event, node CM, no send!");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* See if we have enough mail to send                                  */
  /* ------------------------------------------------------------------- */
  if (!no_size && (e_ptrs[cur_event].mailqsize > p1->callsize))
  {
    p1->mailtypes |= MAIL_QSMALL;
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "mail size is too low, no send.");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* Check, whether there is a cost definition ("local event")           */
  /* ------------------------------------------------------------------- */
  if (e_ptrs[cur_event].behavior & MAT_LOCAL)
  {
    switch (e_ptrs[cur_event].extramask & EXTR_LBHVMASK)
    {
    case EXTR_LINDEXCOST:
      totcost = RealCost;
      break;

    case EXTR_LMINUTECOST:
      calc_cost (1, 0, 60,
                 &dummy, &totcost, &dummy, &cost, &ename, RealCost);
      break;

    case EXTR_LTOTALCOST:
      status_line (">set_flags: %s : callsize: %d",
                   Full_Addr_Str (&p1->mail_addr), p1->callsize);
      calc_cost (1, 0, p1->callsize / costcps,
                 &dummy, &totcost, &dummy, &cost, &ename, RealCost);
      break;
    }

    switch (e_ptrs[cur_event].extramask & EXTR_LRELMASK)
    {                           /* l relation ? */
    case EXTR_LLESS:           /* l<xxx */
      if (totcost >= e_ptrs[cur_event].node_cost)
      {
        p1->mailtypes |= MAIL_COST;
        p1->mailtypes &= ~MAIL_WILLGO;
        status_line (">set_flags: %s : %s (%ld > %d)",
                     Full_Addr_Str (&p1->mail_addr),
                     "cost too high, no send.",
                     totcost, e_ptrs[cur_event].node_cost);
        return;
      }
      break;

    case EXTR_LGREATER:        /* l>xxx */
      if (totcost <= e_ptrs[cur_event].node_cost)
      {
        p1->mailtypes |= MAIL_COST;
        p1->mailtypes &= ~MAIL_WILLGO;
        status_line (">set_flags: %s : %s (%ld < %d)",
                     Full_Addr_Str (&p1->mail_addr),
                     "cost too low, no send.",
                     totcost, e_ptrs[cur_event].node_cost);
        return;
      }
      break;

    case EXTR_LEQUALS:         /* l=xxx */
      if (totcost != e_ptrs[cur_event].node_cost)
      {
        p1->mailtypes |= MAIL_COST;
        p1->mailtypes &= ~MAIL_WILLGO;
        status_line (">set_flags: %s : %s (%ld != %d)",
                     Full_Addr_Str (&p1->mail_addr),
                     "event cost exact, other cost.",
                     totcost, e_ptrs[cur_event].node_cost);
        return;
      }
      break;
    }
  }

  status_line (">set_flags: %s : totcost=%ld, event-cost=%d",
               Full_Addr_Str (&p1->mail_addr),
               totcost, e_ptrs[cur_event].node_cost);

  /* ------------------------------------------------------------------- */
  /* See if we spent too much calling him already                        */
  /* ------------------------------------------------------------------- */
  if (bad_call (&(p1->mail_addr), 0))
  {
    p1->mailtypes |= MAIL_TOOBAD;
    status_line (">set_flags: %s : %s", Full_Addr_Str (&p1->mail_addr),
                 "too many failed connects, no send.");
    return;
  }

  /* ------------------------------------------------------------------- */
  /* We tried hard, but there is no reason to not send the mail now!     */
  /* ------------------------------------------------------------------- */
  p1->mailtypes &= ~MAIL_COST;
  p1->mailtypes &= ~MAIL_QSMALL;
  p1->mailtypes |= MAIL_WILLGO;

  status_line (">set_flags: %s : %s (%04x)",
               Full_Addr_Str (&p1->mail_addr),
               "nothing special, send!",
               p1->mailtypes);
  return;
}

static int LOCALFUNC
xmit_install (MAILP p, ADDRP addr)
{
  MAILP p1, p2;
  int rettype;
  long sztemp, sztemp2;

  p2 = find_mail (addr);

  if (p2 == NULL)
  {
    /* We didn't find it in what we have already */
    p1 = p;
    p1->mail_addr = *addr;
    p1->oldest = (unsigned long) unix_time (NULL);
    rettype = 0;
  }
  else
  {
    /* We found it, so we have to make sure the higher level routine knows */
    p1 = p2;
    rettype = 1;
  }

  /* Get the size of the entry. If it's a FLO-type file,
   * call netsize to find out how big the stuff contained in it
   * actually is. If it's a packet, just take its size.
   *
   * Hold packets don't count. */

  if (!no_size)
  {
    sztemp = 0L;

    if (!strncmp (&(dta_str.name[10]), "lo", 2)
        || !strncmp (&(dta_str.name[9]), "req", 3))
    {
      sztemp2 = netsize (p1, &dta_str.name[9], 0);
      sztemp += sztemp2;
      p1->mailsize += sztemp2;
      if (!strncmp (&(dta_str.name[9]), "req", 3))
        p1->req_size += sztemp2;
    }

    if (!strncmp (&(dta_str.name[10]), "ut", 2)
        || !strncmp (&(dta_str.name[9]), "req", 3))
    {
      struct tm tmstruc;
      time_t curr_time;

      p1->numfiles++;
      sztemp2 = dta_str.size;
      sztemp += sztemp2;
      p1->mailsize += sztemp2;
      if (!strncmp (&(dta_str.name[9]), "req", 3))
        p1->req_size += sztemp2;

      curr_time = unix_time (NULL);
      tmstruc = *unix_localtime (&curr_time);  /* Structure assignment */

      tmstruc.tm_year = (int) (dta_str.time >> 25) + 80;
      tmstruc.tm_mon = (int) ((dta_str.time >> 21) & 0x0f) - 1;
      tmstruc.tm_mday = (int) (dta_str.time >> 16) & 0x1f;

      tmstruc.tm_hour = (int) (dta_str.time >> 11) & 0x1f;
      tmstruc.tm_min = (int) (dta_str.time >> 5) & 0x3f;
      tmstruc.tm_sec = (int) (dta_str.time & 0x1f) * 2;
      /* TJW 960427 * 2 because of double-seconds ! */

      curr_time = unix_mktime (&tmstruc);
      p1->oldest = min ((unsigned long) curr_time, p1->oldest);
    }

    if (dta_str.name[9] != 'h')
      p1->callsize += sztemp;
  }

  switch (dta_str.name[9])
  {
  case 'c':
    p1->mailtypes |= MAIL_CRASH;
    break;

  case 'h':
    p1->mailtypes |= MAIL_HOLD;
    break;

  case 'f':
  case 'o':
    p1->mailtypes |= MAIL_NORMAL;
    break;

  case 'd':
    p1->mailtypes |= MAIL_DIRECT;
    break;

  case 'r':
    p1->mailtypes |= MAIL_REQUEST;
    break;
  }

  return (rettype);
}

char
mail_stat (MAILP p)
{
  if (p->mailtypes & MAIL_UNKNOWN)
    return ('!');
  if (p->mailtypes & MAIL_TOOBAD)
    return ('x');
  if (p->mailtypes & MAIL_TRIED)
    return ('#');
  if (p->mailtypes & MAIL_WILLGO)
    return ('*');
  if (p->mailtypes & MAIL_QSMALL)
    return ('<');
  if (p->mailtypes & MAIL_COST)
    return ('$');               /* TJW 960731 */
  if (p->mailtypes & MAIL_CANTDIAL)
    return ('?');
  return ('-');
}

char *
mail_status_chars (unsigned short p, char *msc)
{
  char *q = msc;

  /* changed for microsoft-compiler */

  if (p & MAIL_HOLD)
    *q++ = 'H';
  else
    *q++ = ' ';
  if (p & MAIL_NORMAL)
    *q++ = 'N';
  else
    *q++ = ' ';
  if (p & MAIL_DIRECT)
    *q++ = 'D';
  else
    *q++ = ' ';
  if (p & MAIL_REQUEST)
    *q++ = 'R';
  else
    *q++ = ' ';
  if (p & MAIL_CRASH)
    *q++ = 'C';
  else
    *q++ = ' ';
  *q++ = '\0';

  return (msc);
}

void
xmit_window (MAILP p)
{
  short i;
  char j1[40];

  sb_fillc (holdwin, ' ');

  if (p == NULL)
  {
    sb_move_puts (holdwin, 2, 7, MSG_TXT (M_NOTHING_IN_OUTBOUND));
    return;
  }

  sb_move_puts (holdwin, 0, 0, MSG_TXT (M_OUTBOUND_HEADER));

  for (i = 1; i < SB_ROW_HOLD; i++)
  {
    do_xmit_line (j1, p);
    sb_move_puts (holdwin, i, 0, j1);

    if (p != NULL)
      p = p->ptrval.ptrnp.next;
  }

  sb_show ();
}

static void LOCALFUNC
do_xmit_line (char *line, MAILP p)
{
  if (p != NULL)
  {
    ADDR addrT;
    char msc[8];                /* AW 981201 */

    if (no_size)
    {
      sprintf (line, HoldNoSize,
               Full_Addr_Str (&(p->mail_addr)),
               mail_status_chars (p->mailtypes, msc),
               mail_stat (p));
    }
    else
    {
      int age = (int) (age_seconds (p->oldest) / 86400L);

      addrT = p->mail_addr;
      /* this NULL prevents displaying of domains in outbound-window!
       * r. hoerner */

      if (hidedomains)
        addrT.Domain = NULL;

      sprintf (line, HoldFmtStr,
               Full_Addr_Str (&addrT),
               (p->numfiles > 999 ? 999 : p->numfiles),
               numdisp (p->mailsize, 4),
               (age > 999 ? 999 : age),  /* TJW 960506 "Age" */
               mail_status_chars (p->mailtypes, msc),
               mail_stat (p));
    }
  }
  else
  {
    int i;

    for (i = 0; i < 38; i++)
      line[i] = ' ';

    line[i] = '\0';
  }
}

static void LOCALFUNC
xmit_sort ()
{
  MAILP p, p1, p2;

  addr_sort ();                 /* jl 960710: sorted outbound */

  p = mail_top;

  /* Find the first that is sendable */
  while (p != NULL)
  {
    if (CanSendMail (p))
      break;
    p = p->ptrval.ptrnp.next;
  }

  if (p == NULL)
    return;

  /* Put the first sendable one on top */
  if (p != mail_top)
  {
    p->ptrval.ptrnp.prev->ptrval.ptrnp.next = p->ptrval.ptrnp.next;
    if (p->ptrval.ptrnp.next != NULL)
      p->ptrval.ptrnp.next->ptrval.ptrnp.prev = p->ptrval.ptrnp.prev;
    p->ptrval.ptrnp.prev = NULL;
    p->ptrval.ptrnp.next = mail_top;
    mail_top->ptrval.ptrnp.prev = p;
    mail_top = p;
  }

  p1 = p;
  p = p1->ptrval.ptrnp.next;
  while (p != NULL)
  {
    if (CanSendMail (p))
    {
      if (p->ptrval.ptrnp.prev == p1)
      {
        p1 = p;
        p = p->ptrval.ptrnp.next;
      }
      else
      {
        p2 = p->ptrval.ptrnp.next;
        p->ptrval.ptrnp.prev->ptrval.ptrnp.next = p->ptrval.ptrnp.next;
        if (p->ptrval.ptrnp.next != NULL)
          p->ptrval.ptrnp.next->ptrval.ptrnp.prev = p->ptrval.ptrnp.prev;
        p->ptrval.ptrnp.next = p1->ptrval.ptrnp.next;
        if (p1->ptrval.ptrnp.next != NULL)
          p1->ptrval.ptrnp.next->ptrval.ptrnp.prev = p;
        p->ptrval.ptrnp.prev = p1;
        p1->ptrval.ptrnp.next = p;
        p1 = p;
        p = p2;
      }
    }
    else
      p = p->ptrval.ptrnp.next;
  }
}

/* jl 960710: sorted outbound, start ------------------------------------- */
/* qsort function to compare two 5d addresses.  used by addr_sort()        */

int
cmp_mail (const void *p1, const void *p2)
{
  ADDRP a1, a2;
  int i;

  a1 = &(*(MAILP *) p1)->mail_addr;
  a2 = &(*(MAILP *) p2)->mail_addr;

  if (a1->Zone == a2->Zone)
  {                             /* TJW960727 only sort Domains in same zone */
    if (a1->Domain != NULL)
    {
      if (a2->Domain != NULL)
      {
        if ((i = stricmp (a1->Domain, a2->Domain)) != 0)
          return (i);
      }
      else
        return (1);
    }
    else if (a2->Domain != NULL)
      return (-1);
  }

  if (a1->Zone != a2->Zone)
    return (a1->Zone > a2->Zone ? 1 : -1);

  if (a1->Net != a2->Net)
    return (a1->Net > a2->Net ? 1 : -1);

  if (a1->Node != a2->Node)
    return (a1->Node > a2->Node ? 1 : -1);

  if (a1->Point != a2->Point)
    return (a1->Point > a2->Point ? 1 : -1);

  return (0);
}

/* ----------------------------------------------------------------------- */

static void LOCALFUNC
addr_sort (void)
{
  size_t nelt;
  MAILP *qbuf, *q;
  MAILP p;

  /* no entries in list?  don't sort them */
  if (mail_top == NULL)
    return;

  /* count entries in mail_top */
  nelt = 0;
  p = mail_top;

  while (p != NULL)
  {
    nelt++;
    p = p->ptrval.ptrnp.next;
  }

  /* allocate memory for qsort buffer */
  qbuf = malloc (nelt * sizeof (*qbuf));

  /* no mem?  don't do anything */
  if (qbuf == NULL)
    return;

  /* fill qsort buffer */
  p = mail_top;
  q = qbuf;

  while (p != NULL)
  {
    *q = p;

    p = p->ptrval.ptrnp.next;
    q++;
  }

  /* sort the stuff */
  qsort (qbuf, nelt, sizeof (*qbuf), cmp_mail);

  /* build sorted mail_top list */
  mail_top = *qbuf;
  q = qbuf;

  while (nelt-- != 0)
  {
    p = *q;

    if (p == mail_top)
      p->ptrval.ptrnp.prev = NULL;
    else
      p->ptrval.ptrnp.prev = *(q - 1);

    if (nelt == 0)
      p->ptrval.ptrnp.next = NULL;
    else
      p->ptrval.ptrnp.next = *(q + 1);

    q++;
  }

  /* free qsort buffer */
  free (qbuf);
}

/* jl 960710: sorted outbound, end --------------------------------------- */


void
global_rescan (void)            /* TJW 960722 global_rescan for all lines */
{                               /* AW 981215 made it more network/mt-friendly */
  struct utimbuf times;
  char flgfname[PATHLEN];
  int tries;
  FILE *fh;

  times.UT_ACTIME = times.modtime = unix_time (NULL);
  sprintf (flgfname, "%sbtrescan.flg", flag_dir);
  for (tries = 0; unix_utime (flgfname, &times) && tries < 3; tries++)
  {
    if (errno == ENOENT)
    {
      fh = fopen (flgfname, write_binary);
      if (fh != NULL)
      {
        fclose (fh);
        break;
      }
    }
    else if (errno == EACCES)
      timer (10);
  }
}


int
xmit_reset (int display, int forcerescan)
{
  char dmpfname[PATHLEN], flgfname[PATHLEN];
  struct stat dmpbuf, flgbuf;
  time_t tnow;
  time_t newtstamp;
  int ret = 0;
  int no_flg;
  int no_dmp;

  if (forcerescan)              /* force rescan = recreate/touch BTRESCAN.FLG */
    global_rescan ();

  /* AW 981220 rewritten xmit_reset
   * decide if reread or rescan needed
   * rescan, if rescanforced
   *            or btrescan.dmp doesn't exist
   *            or btrescan.dmp has 0 bytes
   *            or timestamp(btrescan.flg) != timestamp(btrescan.dmp)
   *            or timestamp(brescan.dmp) older than readholdtime
   * reread otherwise
   *
   * rescan:
   *   open btrescan.dmp writeonly/denyread, return on failure
   *   rescan outbound
   *   write btrescan.dmp
   *   close btrescan.dmp
   *   set timestamp of btrescan.dmp to timestamp of btrescan.flg
   *
   * reread:
   *   open btrescan.dmp readonly/denywrite, return on failure
   *   read btrescan.dmp
   *   close btrescan.dmp
   */

  sem_clr (flag_dir, "btrescan", NULL);  /* delete task rescan flg */
  sprintf (flgfname, "%sbtrescan.flg", flag_dir);
  sprintf (dmpfname, "%sbtrescan.dmp", flag_dir);

  tnow = unix_time (NULL) & ~1;
  no_flg = unix_stat (flgfname, &flgbuf);
  no_dmp = unix_stat (dmpfname, &dmpbuf);
  newtstamp = no_flg ? tnow : flgbuf.st_mtime;

  if (rescanforced || no_dmp ||
      dmpbuf.st_size == 0 ||
      dmpbuf.st_mtime != (no_flg ? last_rescan_tstamp : flgbuf.st_mtime))
    ret = xmit_perform_rescan (display, dmpfname, newtstamp);
  else
  {
    if (dmpbuf.st_mtime + (readholdtime / 100) < tnow)
    {
      struct utimbuf times;

      times.UT_ACTIME = times.modtime = newtstamp = tnow;
      ret = xmit_perform_rescan (display, dmpfname, newtstamp);
      unix_utime (flgfname, &times);
    }
    else
      ret = xmit_perform_reread (display, dmpfname);
  }

  if (ret)
  {
    xmit_sort ();
    next_mail = mail_top;

    /* At least "readholdtime" min to next scan */
    next_rescan = (long) unix_time (NULL) + (readholdtime / 100);
    last_rescan_tstamp = newtstamp;
  }

  next_time = longtimerset (15 * PER_SECOND);

  if (display)                  /* TJW 960526 */
  {
    xmit_window (mail_top);
    if (un_attended)
      do_status (M_READY_WAITING, NULL);
    list_next_event ();
  }

  sb_show ();
  return ret;
}


/* AW 981219 clear outbound queue */
void LOCALFUNC
xmit_clearqueue (void)
{
  MAILP p;

#ifdef CACHE
  cacheflush ();
#endif

  while (mail_top)
  {
    p = mail_top->ptrval.ptrnp.next;
    free (mail_top);
    mail_top = p;
  }
}


/* AW 981219 perform actual rescan of outbound                    */
/* if dmpfname is not null, dump contents of queue into that file */
/* return 2=rescan done                                           */
int LOCALFUNC
xmit_perform_rescan (int display, char *dmpfname, time_t tstamp)
{
  MAILP p;
  FILE *fp;
  DUMPMAIL dumpmail;
  struct utimbuf times;

  times.UT_ACTIME = times.modtime = tstamp;

  if (display)
  {
    if (un_attended)
      do_status (M_READY_RESCAN, NULL);
    else
      status_line (MSG_TXT (M_OUTBOUND_RESCAN));  /* Tell what we are doing */
  }

  status_line (">Rescanning Outbound");

  if (!rescanforced)
  {
    if ((fp = share_fopen (dmpfname, write_binary, DENY_RDWR)) != NULL)
    {
      xmit_clearqueue ();
      do_for_all_domains (NULL, xmit_find);
      for (p = mail_top; p; p = p->ptrval.ptrnp.next)
      {
        int listed;

        listed = nodefind (&(p->mail_addr), 0);
        xmit_setflags (p, listed, newnodedes.ModemType, newnodedes.NodeFlags, newnodedes.RealCost);

        dumpmail.mail = *p;
        dumpmail.mail.ptrval.value.listed = listed;
        dumpmail.mail.ptrval.value.ModemType = newnodedes.ModemType;
        dumpmail.mail.ptrval.value.NodeFlags = newnodedes.NodeFlags;
        dumpmail.mail.ptrval.value.RealCost = newnodedes.RealCost;

        if (p->mail_addr.Domain)
        {
          int i;

          for (i = 0; (i < DOMAINS) && (domain_name[i] != NULL); i++)
          {
            if (p->mail_addr.Domain == domain_name[i])
            {
              strncpy (dumpmail.Domain, domain_abbrev[i], 8);
              break;
            }
          }
        }
        else
          dumpmail.Domain[0] = '\0';

        fwrite (&dumpmail, sizeof (DUMPMAIL), 1, fp);
      }

      fclose (fp);
      unix_utime (dmpfname, &times);
    }
    else
    {
      status_line (">%s not updated, it is locked", dmpfname);
      return 0;
    }
  }
  else
  {
    xmit_clearqueue ();
    do_for_all_domains (NULL, xmit_find);
    for (p = mail_top; p; p = p->ptrval.ptrnp.next)
    {
      int listed;

      listed = nodefind (&(p->mail_addr), 0);
      xmit_setflags (p, listed, newnodedes.ModemType, newnodedes.NodeFlags, newnodedes.RealCost);
    }
  }

  return 2;
}

/* AW 981219 perform actual reread of outbound */
/* return 0=no action done, 1=dump read        */
int LOCALFUNC
xmit_perform_reread (int display, char *dmpfname)
{
  MAILP p;
  FILE *fp;
  DUMPMAIL dumpmail;

  if ((fp = share_fopen (dmpfname, read_binary, DENY_WRITE)) != NULL)
  {
    if (display)
    {
      if (un_attended)
        do_status (M_READY_READING, NULL);
      else
        status_line (MSG_TXT (M_OUTBOUND_READING));  /* Tell what we are doing */
    }

    status_line (">Reading Outbound");

    xmit_clearqueue ();

    do
    {
      if (fread (&dumpmail, sizeof (DUMPMAIL), 1, fp) == 1)
      {
        int listed = dumpmail.mail.ptrval.value.listed;
        byte ModemType = dumpmail.mail.ptrval.value.ModemType;
        word NodeFlags = dumpmail.mail.ptrval.value.NodeFlags;
        word RealCost = dumpmail.mail.ptrval.value.RealCost;

        dumpmail.mail.mail_addr.Domain = NULL;
        dumpmail.mail.ptrval.ptrnp.prev = dumpmail.mail.ptrval.ptrnp.next = NULL;

        if (dumpmail.Domain[0])
        {
          int i;

          for (i = 0; (i < DOMAINS) && (domain_name[i] != NULL); i++)
          {
            if (!strnicmp (dumpmail.Domain, domain_abbrev[i], 8))
            {
              dumpmail.mail.mail_addr.Domain = domain_name[i];
              break;
            }
          }
        }

        p = (MAILP) calloc (sizeof (MAIL), 1);
        *p = dumpmail.mail;
        if (mail_top)
          mail_top->ptrval.ptrnp.prev = p;
        p->ptrval.ptrnp.next = mail_top;
        mail_top = p;

        p->mailtypes &= ~(MAIL_UNKNOWN | MAIL_QSMALL | MAIL_WILLGO |
                          MAIL_CANTDIAL | MAIL_TOOBAD | MAIL_COST);

        nodefind (&(p->mail_addr), 0);  /* VRP 990916 */
        xmit_setflags (p, listed, ModemType, NodeFlags, RealCost);
      }
    }
    while (!feof (fp));

    fclose (fp);
  }
  else
    return 0;

  return 1;
}


MAILP LOCALFUNC
xmit_find (MAILP p, ADDRP address)
{
  char next_one[PATHLEN];
  char *HoldName;
  ADDR tmp;

#ifndef CACHE
  int j = 0;

#endif

  tmp = *address;

  HoldName = HoldAreaNameMunge (address);

  if (address->Point != 0)
  {
    sprintf (next_one, "%s%04hx%04hx.pnt" DIR_SEPS "*.*",
             HoldName, address->Net, address->Node);
    tmp.Point = 0;
  }
  else
  {
    sprintf (next_one, "%s*.*", HoldName);
    tmp.Net = 0;
    tmp.Node = 0;
    tmp.Point = 0;
  }

#ifdef CACHE
  dirinit (&dta_str, next_one);
  while (!dirread (&dta_str))
  {
#else
  j = 0;
  while (!dfind (&dta_str, next_one, j))
  {
    j = 1;
#endif
    /* We have a match. Was it a .FLO file or a .OUT file? */

    if (strncmp (&(dta_str.name[10]), "lo", 2) == 0)
    {
      /* FLO, DLO, CLO and HLO are the only ones! */
      if (strchr ("fdch", dta_str.name[9]) == NULL)
        continue;
    }
    else if (strncmp (&(dta_str.name[10]), "ut", 2) == 0)
    {
      /* OUT, DUT, CUT and HUT are the only ones! */
      if (strchr ("odch", dta_str.name[9]) == NULL)
        continue;
    }
    else if (strncmp (&(dta_str.name[9]), "req", 3) != 0)
      continue;

    /* We found a name, remember it */

    if (address->Point != 0)
    {
      if (sscanf (dta_str.name, "%08hx.", &(tmp.Point)) != 1)
        continue;
    }
    else if (sscanf (dta_str.name, "%04hx%04hx.", &(tmp.Net), &(tmp.Node)) != 2)
      continue;

    if (p == NULL)
    {
      p = mail_top = (MAILP) calloc (sizeof (MAIL), 1);
    }
    else
    {
      p->ptrval.ptrnp.next = (MAILP) calloc (sizeof (MAIL), 1);
      p->ptrval.ptrnp.next->ptrval.ptrnp.prev = p;
      p = p->ptrval.ptrnp.next;
    }

    p->numfiles = 0;

    if (xmit_install (p, &tmp))
    {
      /* No good */
      if (p->ptrval.ptrnp.prev != NULL)
      {
        p = p->ptrval.ptrnp.prev;
        free (p->ptrval.ptrnp.next);
        p->ptrval.ptrnp.next = NULL;
      }
      else
      {
        free (p);
        p = mail_top = NULL;
      }
    }
  }

#ifdef CACHE                    /* while (!done) */
  dirquit (&dta_str, 0);
#else
  if (j)
    dfind (&dta_str, NULL, 2);
#endif
  return (p);
}

int
xmit_next (ADDRP xaddr)
{
  int i, j;
  MAILP mp;

#ifdef CACHE
  cacheflush ();
#endif

  /* MR 110798 moved code from set_up_outbound to fix jumping outbound */

  mp = find_mail (&next_addr);
  if (mp == NULL)
    next_mail = mail_top;
  else
  {
    if (mp->ptrval.ptrnp.prev == NULL)
    {
      while (mp->ptrval.ptrnp.next)
        mp = mp->ptrval.ptrnp.next;  /* find last element in list */

      next_mail = mp;
    }
    else
      next_mail = mp->ptrval.ptrnp.prev;
  }

  /* MR 110798 stoped moving code from set_up_outbound */

  for (i = 0; i < 2; i++)
  {
    /* Set up the proper pointer */
    if ((next_mail == NULL) || (next_mail->ptrval.ptrnp.next == NULL))
    {
      if (next_rescan < (long) unix_time (NULL))
        xmit_reset (1, 0);
      next_mail = mail_top;
    }
    else
      next_mail = next_mail->ptrval.ptrnp.next;

    /* Loop through till we find something we can send */
    while (next_mail != NULL)
    {
      if (CanSendMail (next_mail))
      {
        if (bad_call (&(next_mail->mail_addr), 0))
          next_mail->mailtypes |= MAIL_TOOBAD;
        else
        {
          /* check for mail before calling */
          if ((j = any_mail (next_mail)) <= 0)
          {
            if (j == 0)
              xmit_delete ();
            else
              next_mail = next_mail->ptrval.ptrnp.next;

            continue;
          }

          *xaddr = next_mail->mail_addr;
          xmit_window (next_mail);
          return (1);
        }
      }

      next_mail = next_mail->ptrval.ptrnp.next;
    }
  }

  /* Oh well, we tried */
  next_mail = mail_top;         /* MR 970518 added */
  xmit_window (mail_top);
  return (0);
}

static void LOCALFUNC
xmit_delete_one (void)
{
  MAILP p;

  if (any_mail (next_mail) != 0)
  {
    status_line (MSG_TXT (M_STILL_HAVE_MAIL),
                 Full_Addr_Str (&(next_mail->mail_addr)));

    /* We still have something for him */

    next_mail->mailtypes &= ~MAIL_WILLGO;
    next_mail->mailtypes |= MAIL_TRIED;
    return;
  }

  if (next_mail != mail_top)
  {
    p = next_mail->ptrval.ptrnp.next;
    next_mail = next_mail->ptrval.ptrnp.prev;
    free (next_mail->ptrval.ptrnp.next);
    next_mail->ptrval.ptrnp.next = p;
    if (p != NULL)
      p->ptrval.ptrnp.prev = next_mail;
  }
  else
  {
    mail_top = mail_top->ptrval.ptrnp.next;
    free (next_mail);
    if (mail_top != NULL)
      mail_top->ptrval.ptrnp.prev = NULL;
    next_mail = mail_top;       /* MR 970518 NULL -> mail_top */
  }
}

void
xmit_delete (void)
{
  int i;

  if (next_mail == NULL && no_EMSI_Session)
    return;

#ifdef CACHE
  cacheflush ();
#endif

  if (no_EMSI_Session)
  {
    xmit_delete_one ();
  }
  else
  {
    for (i = (num_rakas - 1); i >= 0; i--)
    {
      next_mail = find_mail (&remote_akas[i]);
      if (next_mail != NULL)
        xmit_delete_one ();
    }
  }

  xmit_window (mail_top);
  no_EMSI_Session = TRUE;
  num_rakas = 0;
}

void LOCALFUNC
write_badcall (char *fname, int value)
{
  int temp;
  FILE *handle;

  temp = value;

  handle = fopen (fname, write_binary);
  if (handle != NULL)
  {
    fwrite ((void *) &temp, sizeof (int), 1, handle);

    fclose (handle);
  }
}

void LOCALFUNC
read_badcall (char *fname, int *value)
{
  FILE *handle;

  handle = fopen (fname, read_binary);
  if (handle != NULL)
  {
    fread ((void *) value, sizeof (int), 1, handle);

    fclose (handle);
  }
}

int
bad_call (ADDRP baddr, int rwd)
{
  int res, j;
  long i;                       /* TJW 961118 changed to long for DOS / OS/2 compatibility */

#ifdef CACHE
  DIRBUF bad_dta;

#else
  struct FILEINFO bad_dta;

#endif
  FILE *bad_wazoo;
  char *p, *HoldName;
  char fname[PATHLEN], fname1[PATHLEN];

#ifdef CACHE
  // cacheflush();    /* MR 970406 cause of CacheHold bug!? */
#endif

  HoldName = HoldAreaNameMunge (baddr);

  sprintf (fname, "%s%s.$$?", HoldName, Hex_Addr_Str (baddr));
  j = (int) strlen (fname) - 1; /* Point at ?          */
  res = -1;                     /* Initialize to fail  */

#ifdef CACHE
  dirinit (&bad_dta, fname);    /* This says findfirst */
  while (!dirread (&bad_dta))   /* as long as we match */
#else
  i = 0;                        /* This says findfirst */
  while (!dfind (&bad_dta, fname, (int) i))  /* as long as we match */
#endif
  {
    i = 1;                      /* switch to findnext  */
    if (isdigit (bad_dta.name[11]))  /* is there a digit?   */
    {
      fname[j] = bad_dta.name[11];  /* Yes, copy to fname  */
      res = fname[j] - '0';     /* Save it for testing */
      break;                    /* Get out of while    */
    }
  }

#ifdef CACHE
  dirquit (&bad_dta, 0);
#else
  if (i)
    dfind (&bad_dta, NULL, (int) 2);
#endif

  if (res == -1)                /* Successful search?  */
    fname[j] = '0';             /* No, base digit = 0  */

  if (rwd > 0)
  {
    /* First create a filename that is one higher than what we've got */

    strcpy (fname1, fname);
    fname1[j]++;
    if (fname1[j] > '9')
      fname1[j] = '9';

    if (res == -1)              /* Did we have a file? */
    {                           /* No, make one.       */
      i = rwd - 1;              /* zero based          */
      if (rwd == 2)             /* if No carrier       */
        write_badcall (fname, (int) i);
      else                      /* With carrier        */
        write_badcall (fname1, (int) i);
    }
    else
    {                           /* There was a file    */
      if (rwd == 2)
      {
        /* 2 = Unsuccessful, No carrier. Update contents of the file. */

        read_badcall (fname, &res);
        ++res;
        write_badcall (fname, res);
      }
      else
      {
        /* 1 = Unsuccessful, Carrier. Update file name to reflect failure. */

        if (res + 1 >= max_connects)
        {
          MAILP p;

          res = -1;
          write_badcall (fname, res);

          p = find_mail (baddr);
          if (p)
            p->mailtypes |= MAIL_TOOBAD;

        }

        rename (fname, fname1);
      }
    }
  }
  else if (rwd == 0)
  {
    /* 0 = We are reading a bad call status */
    /* Is it automatically ok (no .$$ file there) ? */

    if (res == -1)
      return (0);

    /* Were there too many connects with carrier? */

    if (res >= max_connects)    /* this says: max_connects = [0..9] ! */
      return (1);

    /* Ok, check for connects without carrier */

    res = 0;
    read_badcall (fname, &res);
    return (res >= max_noconnects);
  }
  else if (rwd < 0)
  {
    /* -1 = Cleanup of bad call status. This happens in two steps:
     *  a) delete 'netnode.$$?' in hold area;
     *  b) if a 'netnode.Z' file exists in hold area,
     *     1) delete all BADWAZOO.xxx files listed in the .Z file;
     *     2) delete the 'netnode.z' file. */

    if (res != -1)
    {
      unlink (fname);
#ifdef CACHE
      // cacheflush();    /* MR 970406 cause of CacheHold bug!? */
#endif
    }

    if (!mail_finished)
      return (0);

    sprintf (fname, "%s%s.z", HoldName, Hex_Addr_Str (baddr));
    if (dexists (fname))
    {
      bad_wazoo = fopen (fname, read_ascii);
      if (bad_wazoo == NULL)
        got_error (MSG_TXT (M_OPEN_MSG), fname);
      else
      {
        while (!feof (bad_wazoo))
        {
          e_input[0] = '\0';
          if (!fgets (e_input, 64, bad_wazoo))
            break;

          /* Point to BADWAZOO.xxx */
          p = strchr (e_input, ' ') + 1;

          /* Then just past it and terminate */
          p = strchr (p, ' ');
          *p = '\0';

          /* Back to where we were */
          p = strchr (e_input, ' ') + 1;

          /* Build file name and delete file */
          strcpy (fname1, CURRENT.sc_Inbound);
          strcat (fname1, p);
          unlink (fname1);
#ifdef CACHE
          cacheflush ();
#endif
        }

        fclose (bad_wazoo);
      }

      unlink (fname);
#ifdef CACHE
      // cacheflush();      /* MR 970406 cause of CacheHold bug!? */
#endif
    }
  }

  return (0);
}

int
set_up_outbound (int forcerescan)
{
  // MAILP mp;
  clock_t start;
  clock_t ticks;
  int ret;

  start = clock ();

  set_xy ("");

  ret = xmit_reset (1, forcerescan);

  /* and remember where we left off */
  if (hist.next_addr.Net != 0)
  {
    next_addr = hist.next_addr;
  }
  else
  {
    next_addr.Zone = next_addr.Net = next_addr.Node = next_addr.Point = 0;
    next_addr.Domain = NULL;
  }

  next_addr.Domain = NULL;

  ticks = clock () - start;
  rescanticks += ticks;
  rescancnt++;

#ifdef CACHE
  if (rescanstat)
  {
    clock_t average = rescanticks / rescancnt;

    /* jl 960729:  added CPS10 to avoid floating point arithmetik
     * with BC++/DOS (CLOCKS_PER_SEC is 18.2)
     * added casts to match format string */

#define CPS10 ((int) (CLOCKS_PER_SEC * 10))

    status_line (MSG_TXT (M_CACHEHOLD_STAT),
                 (int) (ticks * 10 / CPS10),
                 (int) (((1000 * ticks) / CPS10) % 100),
                 (long) (cachehold ? (cachemem + 512) / 1024 : 0),
                 (int) (average * 10 / CPS10),
                 (int) (((1000 * average) / CPS10) % 100));
  }
#endif

  next_time = longtimerset (15 * PER_SECOND);

  return ret;                   /* 0=didn't do anything, 1=reread, 2=rescan */
}

void
kill_bad (void)
{
#ifdef CACHE
  cacheflush ();
#endif
  do_for_all_domains (NULL, kill_one);
}

MAILP LOCALFUNC
kill_one (MAILP mailp, ADDRP address)
{
  char next_one[PATHLEN];
  char thisfile[PATHLEN];
  char *HoldName;
  char *p;

#ifndef CACHE
  int j;

#endif

  mailp = mailp;                /* unused, necessary 'cause we call back to kill_one() */

  HoldName = HoldAreaNameMunge (address);

  if (address->Point != 0)
  {
    sprintf (next_one, "%s%04hx%04hx.pnt" DIR_SEPS "*.$$?",
             HoldName, address->Net, address->Node);
  }
  else
  {
    sprintf (next_one, "%s*.$$?", HoldName);
  }

  strcpy (thisfile, next_one);
  p = strrchr (thisfile, '*');

#ifdef CACHE
  dirinit (&dta_str, next_one);
  while (!dirread (&dta_str))
  {
#else
  j = 0;
  while (!dfind (&dta_str, next_one, j))
  {
    j = 1;
#endif
    strcpy (p, dta_str.name);
    status_line (">Deleting %s", thisfile);
    unlink (thisfile);
  }                             /* while (!done) */

#ifdef CACHE
  dirquit (&dta_str, 0);
#else
  if (j)
    dfind (&dta_str, NULL, 2);
#endif

  return (mailp);
}

/* -------------------------------------------------------------------
 * Figure out if there's any mail for the specified node.
 * Used to determine if we need to actually make a call.
 *
 * Returns 0 for no mail at all
 *         1 for non-hold mail
 *        -1 for hold mail only
 *
 * I can't remember who was the first to demand this. I've been
 * pummelled for a while and there's almost certainly been a loss
 * of grey matter.
 * -------------------------------------------------------------------- */

static int LOCALFUNC
any_mail (MAILP node)
{
#ifdef CACHE

  DIRBUF dta;

#else

  struct FILEINFO dta;
  int j = 0;

#endif

  char next_one[PATHLEN];
  int ret = 0;

  sprintf (next_one, "%s%s.*",
           HoldAreaNameMunge (&(node->mail_addr)),
           Hex_Addr_Str (&(node->mail_addr)));
#ifdef CACHE
  dirinit (&dta, next_one);
  while (!dirread (&dta))
  {
#else
  while (!dfind (&dta, next_one, j))
  {
    j = 1;
#endif

#ifndef JACK_DECKER
    if (dta.name[9] == 'h')
    {
      ret = -1;
      continue;
    }
#endif
    if ((!strncmp (&(dta.name[10]), "lo", 2))
        || (!strncmp (&(dta.name[10]), "ut", 2)))
    {
      ret = 1;
      break;
    }
  }

#ifdef CACHE
  dirquit (&dta, 0);
#else
  if (j)
    dfind (&dta, NULL, 2);
#endif

  return (ret);
}

/* $Id: mailovly.c,v 1.15 1999/09/27 20:51:29 mr Exp $ */
