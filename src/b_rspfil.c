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
 * Filename    : $Source: E:/cvs/btxe/src/b_rspfil.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:35 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm File Request Failure Message Module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

char far *rspverbs[] =
{
  "text",
  "date",
  "time",
  "bink",
  "mynode",
  "system",
  "sysop",
  "yrnode",
  "request",
  "status",
  "abort",
  "exit",
  "line",
  NULL
};

typedef struct
{                               /* MB 93-12-12  PKTRSP */
  short ver;
  short orig_node;
  short dest_node;
  short orig_net;
  short dest_net;
  short attr;
  short cost;
}
PACKED, *PACKED_PTR;

#define ATTRIB     129

void Make_Response (char *, int);  /* Build a response      */

void
Make_Response (char *data, int failure)
{
  FILE *Template, *Response;
  char *dummy_text;
  char *text;
  char *failure_text[10];
  char resp_filename[PATHLEN];
  char *p, *q, *s;
  struct tm *tp;
  time_t ltime;
  int i;
  struct _pkthdr45 pkt_hdr;
  PACKED p_msg;
  ADDR real_addr;
  AKA *aka;

  if (alias == NULL)
    return;

  aka = alias;

  for (i = 0; (i < assumed) && (aka != NULL); i++)
    aka = aka->next;

  if (aka == NULL)
    aka = alias;

  if (Netmail_Session == 2)
  {
    text = strchr (data, ' ');
    if (text != NULL)
      *text = '\0';
  }

  Template = Response = (FILE *) NULL;
  dummy_text = NULL;

  for (i = 0; i < 9; i++)
    failure_text[i] = NULL;

  if (((text = calloc (1, 256)) == NULL)
      || ((dummy_text = calloc (1, 256)) == NULL))
    goto resp_failed;

  unix_time (&ltime);
  tp = unix_localtime (&ltime);

  if ((Template = share_fopen (CURRENT.rq_Template, read_ascii, DENY_WRITE)) == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), CURRENT.rq_Template);
    goto resp_failed;
  }

  if (pktrsp)                   /* MB 93-12-12  PKTRSP */
  {
    invent_pkt_name (resp_filename);  /* prepare unique response filename */
    sprintf (dummy_text, "%02x", TaskNumber);
    strncpy (resp_filename, dummy_text, 2);
    if (failure == 10)
    {
      sprintf (dummy_text, "%s%s", PROT.sc_Inbound, resp_filename);
      assumed = 0;
      aka = alias;
    }
    else
      sprintf (dummy_text, "%s%s", flag_dir, resp_filename);
    strcpy (resp_filename, dummy_text);
  }
  else
  {
    if (aka->Point != 0)
      sprintf (resp_filename, "%08hx.rsp", aka->Point);
    else
      sprintf (resp_filename, "%s.rsp", Hex_Addr_Str ((ADDR *) aka));
  }

  if ((Response = fopen (resp_filename, write_binary)) == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), resp_filename);
    goto resp_failed;
  }

  if (pktrsp)                   /* MB 93-12-12  PKTRSP */
  {
    real_addr = remote_addr;
    if ((pvtnet >= 0) && (remote_addr.Net == (word) pvtnet) &&
        (boss_addr.Zone == remote_addr.Zone) &&
        (remote_addr.Point == 0))
    {
      real_addr.Point = remote_addr.Node;
      real_addr.Net = boss_addr.Net;
      real_addr.Node = boss_addr.Node;
    }
    /* set up pkt and msg headers */

    memset (&pkt_hdr, 0, sizeof (pkt_hdr));

    pkt_hdr.orig_zone = aka->Zone;
    pkt_hdr.orig_net = p_msg.orig_net = aka->Net;
    pkt_hdr.orig_node = p_msg.orig_node = aka->Node;
    pkt_hdr.orig_point = aka->Point;

    pkt_hdr.dest_zone = remote_addr.Zone;
    pkt_hdr.dest_net = remote_addr.Net;
    pkt_hdr.dest_node = remote_addr.Node;  /* TJW 960507 added this ;-) */
    pkt_hdr.dest_point = remote_addr.Point;

    if (remote_password)        /* TJW 960507 also put in password */
      strncpy ((char *) pkt_hdr.password, remote_password, 8);

    pkt_hdr.ver = p_msg.ver = PKTVER;
    pkt_hdr.subver = 2;

    pkt_hdr.product = PRDCT_CODE;
    pkt_hdr.serial = 0;

    p_msg.dest_net = real_addr.Net;
    p_msg.dest_node = real_addr.Node;

    p_msg.attr = ATTRIB;

    /* write pkt header */

    fwrite ((char *) &pkt_hdr, sizeof (struct _pkthdr), 1, Response);

    /* write msg header */

    fwrite ((char *) &p_msg, sizeof (PACKED), 1, Response);

    /* write date */
    /* TJW 960507 vv          v */
    sprintf (dummy_text, "%02d %3.3s %02d  %02d:%02d:%02d",
             tp->tm_mday, mtext[tp->tm_mon], tp->tm_year % 100,
             tp->tm_hour, tp->tm_min, tp->tm_sec);
    fwrite (dummy_text, strlen (dummy_text) + 1, 1, Response);

    /* write sysop */

    fwrite ("Sysop", 6, 1, Response);

    /* write your name */

    fwrite (sysop, strlen (sysop) + 1, 1, Response);

    /* write subject */

    fwrite ("Response to File Request", 25, 1, Response);

    /* write extended flags */

    if (aka->Zone != remote_addr.Zone)
    {
      sprintf (dummy_text, "\001INTL %d:%d/%d %d:%d/%d\r",
               real_addr.Zone, real_addr.Net, real_addr.Node,
               aka->Zone, aka->Net, aka->Node);
      fwrite (dummy_text, strlen (dummy_text), 1, Response);
    }
    if (aka->Point > 0)
    {
      sprintf (dummy_text, "\001FMPT %d\r", aka->Point);
      fwrite (dummy_text, strlen (dummy_text), 1, Response);
    }
    if (real_addr.Point > 0)
    {
      sprintf (dummy_text, "\001TOPT %d\r", real_addr.Point);
      fwrite (dummy_text, strlen (dummy_text), 1, Response);
    }
    fwrite ("\r\n\r\n", 4, 1, Response);
  }

  while (!feof (Template))
  {

  read_line:

    e_input[0] = '\0';
    if (fgets (text, 254, Template) == NULL)
      break;

    if (text[0] == '%' && text[1] == ';')
      continue;                 /* Comment at start, no output  */

    p = text;
    q = e_input;

    while (*p)
    {

      if (*p == '\n')           /* All done if newline seen    */
        break;

      if (*p != '%')            /* Copy until(unless) we see % */
      {
        *q++ = *p++;
        continue;
      }

      if (*++p == ';')          /* If followed by ; just skip  */
        break;

      if ((i = parse (p, rspverbs)) == -1)  /* Check against arg list */
      {
        *q++ = '%';             /* No match, use the % literal */
        continue;
      }

      switch (i)
      {

      case 1:                  /* "text"       */

        if (((i = atoi (p = skip_blanks (&p[4]))) < 1) || (i > 9))
          goto read_line;

        if (!*(p = skip_to_blank (p)) || !*++p)
          goto read_line;

        if (failure_text[--i] != NULL)
        {
          free (failure_text[i]);
          failure_text[i] = NULL;
        }

        failure_text[i] = s = calloc (1, 1 + strlen (p));
        if (s == NULL)
          goto read_line;
        while ((*p) && (*p != '\n'))
          *s++ = *p++;
        *s++ = '\0';

        goto read_line;

      case 2:                  /* "date"       */
        p += 4;
        sprintf (dummy_text, "%2d-%3s-%02d", tp->tm_mday, mtext[tp->tm_mon], tp->tm_year % 100);
      scopy:
        s = dummy_text;
      ccopy:
        while (*s)
          *q++ = *s++;

        break;

      case 3:                  /* "time"       */
        p += 4;
        sprintf (dummy_text, "%2d:%02d", tp->tm_hour, tp->tm_min);
        goto scopy;

      case 4:                  /* "bink"       */
        p += 4;
        s = ANNOUNCE;
        goto ccopy;

      case 5:                  /* "mynode"     */
        p += 6;
        sprintf (dummy_text, "%s", Full_Addr_Str ((ADDR *) aka));
        goto scopy;

      case 6:                  /* "system"     */
        p += 6;
        s = system_name;
        goto ccopy;

      case 7:                  /* "sysop"      */
        p += 5;
        s = sysop;
        goto ccopy;

      case 8:                  /* "yrnode"     */
        p += 6;
        sprintf (dummy_text, "%s", Full_Addr_Str (&remote_addr));
        goto scopy;

      case 9:                  /* "request"    */
        p += 7;
        s = data;
        goto ccopy;

      case 10:                 /* "status"     */
        p += 6;
        strcpy (dummy_text, p);
        if (failure_text[failure - 1] == NULL)
          text[0] = '\0';
        else
          strcpy (text, failure_text[failure - 1]);
        strcat (text, dummy_text);
        p = text;
        break;

      case 11:                 /* "abort"      */
        if (*(p = skip_blanks (&p[5])))  /* If there's an argument, */
        {
          if (failure != atoi (p))  /* See if it matches failure   */
            goto read_line;     /* No, keep going.             */
        }

        fclose (Response);      /* Abort things: Close file,   */
        Response = NULL;        /* Keep track of closed file   */
        unlink (resp_filename); /* Then delete it              */
        goto resp_failed;       /* And take the failure exit   */

      case 12:                 /* "exit"       */
        if (*(p = skip_blanks (&p[4])))  /* If there's an argument, */
        {
          if (failure != atoi (p))  /* See if it matches failure   */
            goto read_line;     /* No, keep going.             */
        }
        goto resp_done;         /* A match, close the file     */

      case 13:                 /* "line"       */
        if ((!(*(p = skip_blanks (&p[4]))))
            || (failure != atoi (p)))  /* If argument doesn't match,  */
          goto read_line;       /* throw out line, keep going. */

        if (*(p = skip_to_blank (p)))  /* If there's any text,        */
        {
          strcpy (text, ++p);   /* Copy rest of line down      */
          p = text;             /* Move pointer to front       */
        }
        break;

      }                         /* End switch   */
    }                           /* End while *p */

    *q++ = '\r';
    *q++ = '\n';
    *q = '\0';

    i = (int) (q - e_input);
    fwrite (e_input, (unsigned int) i, 1, Response);
  }                             /* End while !feof (Template) */

resp_done:

  strcpy (data, resp_filename);

  fclose (Template);
  Template = NULL;

  /* write the final null *//* MB 93-12-12  PKTRSP */
  if (pktrsp)
    fwrite ("\r\n\r\n\0\0\0", 7, 1, Response);

  fclose (Response);
  Response = NULL;
  goto cleanup;

resp_failed:

  *data = '\0';

  if (Response != NULL)
    fclose (Response);

  if (Template != NULL)
    fclose (Template);

cleanup:

  for (i = 0; i < 9; i++)
  {
    if (failure_text[i] != NULL)
    {
      free (failure_text[i]);
      failure_text[i] = NULL;
    }
  }

  if (text != NULL)
    free (text);

  if (dummy_text != NULL)
    free (dummy_text);

  return;
}

/* $Id: b_rspfil.c,v 1.5 1999/03/23 22:28:35 mr Exp $ */
