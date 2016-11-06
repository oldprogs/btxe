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
 * Filename    : $Source: E:/cvs/btxe/src/srif.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:42 $
 * State       : $State: Exp $
 *
 * Description : BinkleyTerm SRIF implementation
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"


int
invoke_SRIF (int janusflag, int (*callback) (char *), int (*calltime) (long))
{
  int srif_sent = 0;
  int good, i;
  char req[PATHLEN], command[PATHLEN], name[PATHLEN];
  char *s, resp[PATHLEN], sname[PATHLEN];
  ADDR foo;
  FILE *srif;
  AKA *aka;

  calltime = calltime;          /* MMP 960513 Keep compiler happy */

  /* calculating the req-name */
  for (aka = alias; aka != NULL; aka = aka->next)
  {
    memcpy (&foo, aka, sizeof (ADDR));

    /* For a point, massage the address to get the right .REQ filename */
    if (foo.Point != 0)
    {
      foo.Node = foo.Point;
      foo.Point = 0;
      foo.Net = (pvtnet > 0) ? (word) pvtnet : 0;
    }

    sprintf (req, request_template, CURRENT.sc_Inbound,
             Hex_Addr_Str (&foo), TaskNumber);

    if (!dexists (req))
      continue;                 /* on this aka is no request */

    sprintf (sname, "srif.%02x", TaskNumber);
    srif = fopen (sname, write_ascii);
    if (srif == NULL)
      goto done;
    fprintf (srif, "Sysop %s\n", remote_sysop);
    fprintf (srif, "AKA %u:%u/%u.%u\n", remote_addr.Zone, remote_addr.Net, remote_addr.Node, remote_addr.Point);

    /* include other akas as well */
    for (i = 0; i < num_rakas; i++)
    {
      if ((remote_akas[i].Zone != remote_addr.Zone) ||
          (remote_akas[i].Net != remote_addr.Net) ||
          (remote_akas[i].Node != remote_addr.Node) ||
          (remote_akas[i].Point != remote_addr.Point))
        fprintf (srif, "AKA %u:%u/%u.%u\n", remote_akas[i].Zone, remote_akas[i].Net, remote_akas[i].Node, remote_akas[i].Point);
    }

    fprintf (srif, "Baud %lu\n", cur_baud.rate_value);  /* TS/TJW 960620 */
    fprintf (srif, "Time %d\n", time_to_next (0));
    fprintf (srif, "RequestList %s\n", req);
    sprintf (resp, "%s%s.qlo",  /* TJW 960528 */
             HoldAreaNameMunge (&remote_addr),
             Hex_Addr_Str (&remote_addr));
    fprintf (srif, "ResponseList %s\n", resp);
    fprintf (srif, "RemoteStatus %sROTECTED\n", is_protected ? "P" : "UNP");
    fprintf (srif, "SystemStatus %sISTED\n", is_listed ? "L" : "UNL");
    fprintf (srif, "Site %s\n", remote_site);
    fprintf (srif, "Location %s\n", remote_location);
    fprintf (srif, "Phone %s\n", remote_phone);
    fprintf (srif, "Mailer %s\n", remote_mailer);
    fprintf (srif, "PORT %d\n", original_port);

    if (is_protected && strlen (password))
      fprintf (srif, "Password %s\n", password);

    fclose (srif);

    /* Spawn to SRIF ERP */
    sprintf (command, SRIF_commandline, sname);
    status_line (MSG_TXT (M_SRIF_INVOKE), command);
    close_up (share);
    b_spawn (command);
    come_back ();
    show_session (MSG_TXT (M_MCP_MAIL_XFER), &remote_addr);

    unlink (sname);             /* kill srif and request-file */
    unlink (req);

    srif = fopen (resp, read_ascii);
    if (srif != NULL)
    {
      while (!feof (srif))
      {
        req[0] = 0;
        if (fgets (req, 79, srif) == NULL)
          break;

        /* ; as the first char is a comment */
        if (req[0] == ';')
          continue;

        /* Get rid of the newline at the end */
        s = (char *) (req + strlen (req) - 1);
        while ((s >= req) && isspace (*s))
          *s-- = '\0';

        if (req[0] == '\0')
          continue;
        s = req;

        switch (req[0])
        {
        case SHOW_DELETE_AFTER:
        case DELETE_AFTER:
        case '+':
        case '=':
          s++;
          break;

        default:
          break;
        }

        strcpy (name, s);
        good = callback (name);

        if (good)
          srif_sent++;

        if (!janusflag)
        {
          switch (req[0])
          {
          case '=':
            if (!good)
              break;

          case SHOW_DELETE_AFTER:  /* fall-through ... */
          case DELETE_AFTER:
            unlink (name);
            break;
          }
        }
      }

      fclose (srif);
    }

    unlink (resp);
  }

done:
  return srif_sent;
}

/* $Id: srif.c,v 1.5 1999/03/22 03:47:42 mr Exp $ */
