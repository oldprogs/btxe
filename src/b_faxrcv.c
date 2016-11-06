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
 * Filename    : $Source: E:/cvs/btxe/src/b_faxrcv.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:33 $
 * State       : $State: Exp $
 * Orig. Author: Michael Buenter
 *
 * Description : BinkleyTerm FAX file reception module
 *
 * Note        : Original UNIX sources Henry Minsky 11/02/90 hqm@ai.mit.edu
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"
#include "faxproto.h"

/*--------------------------------------------------------------------------*/
/* Local routines                                                           */
/*--------------------------------------------------------------------------*/

static int LOCALFUNC get_fax_file (int);
static int LOCALFUNC read_g3_stream (FILE *, int);
static void LOCALFUNC get_faxline (char *, int, unsigned int);
static void LOCALFUNC init_swaptable (void);
static void LOCALFUNC init_modem_response (void);
static void LOCALFUNC get_modem_result_code (void);
static void LOCALFUNC parse_text_response (char *);

static int LOCALFUNC faxmodem_receive_page (int);

/*--------------------------------------------------------------------------*/
/* Private data                                                             */
/*--------------------------------------------------------------------------*/

static int gEnd_of_document;
static unsigned char swaptable[256];
static int swaptableinit = FALSE;
static struct faxmodem_response response;

static unsigned long faxsize = 0L;

/*--------------------------------------------------------------------------*/
/* FAX RECEIVE Routines                                                     */
/*--------------------------------------------------------------------------*/

/* receive fax files into basefilename */

int
faxreceive (int zmode, CALLHIST * data)
{
  int result;
  int page;

  if (!fax_in)
    return (0);

  happy_compiler = zmode;

  if (!swaptableinit)
    init_swaptable ();

  unix_time (&etm);

  if (fax_baud != -1)
  {
    baud = fax_baud;
    cur_baud = pbtypes[baud];
    MDM_ENABLE (cur_baud.rate_mask);
    show_cs_port (cur_baud.rate_value, FALSE);
  }

  do_status (M_FAX_RECV, NULL);

  init_modem_response ();
  gEnd_of_document = FALSE;
  response.fcon = TRUE;         /* we already connected */

  result = 0;

  for (page = 0; gEnd_of_document == FALSE; page++)
  {
    result = get_fax_file (page);
    status_line (">FAX get_fax_file returns = %d", result);
    switch ((int) result)
    {
    case PAGE_GOOD:
      data->h.f.pages++;
      data->h.f.bytes += faxsize;
      continue;

    case PAGE_HANGUP:
      status_line (MSG_TXT (M_FAX_RECEIVED_PAGES), page);
      result = 1;
      gEnd_of_document = TRUE;
      strcpy (data->h.f.status, "success");
      break;

    default:
      status_line (MSG_TXT (M_FAX_TRANSMISSION_ERROR));
      result = page;
      gEnd_of_document = TRUE;
      strcpy (data->h.f.status, "error");
      break;
    }
  }

  set_baud (max_baud.rate_value, 0);
  show_cs_port (max_baud.rate_value, FALSE);  /*  TS 961122   */
  strntcpy (data->h.f.remoteid, response.remote_id, sizeof (data->h.f.remoteid));
  data->h.f.speed = 2400L * response.T30.br;
  sprintf (data->h.f.filename, "page%02x??.fax", (TaskNumber & 0xff));
  data->task = TaskNumber;
  data->outgoing = 0;
  data->calltype = CT_FAX;
  data->subclass = 0;

  return result;
}

/* This executes the +FDR receive page command, and looks for
 * the proper CONNECT response, or, if the document is finished,
 * looks for the FHNG/FHS code.
 *
 * returns:
 *  PAGE_GOOD                no error conditions occured during reception
 *  PAGE_HANGUP              normal end of transmission
 *  PAGE_ERROR               something's wrong
 */

static int LOCALFUNC
get_fax_file (int page)
{
  char buf[PATHLEN], j[100];
  int result;
  FILE *fp = NULL;
  int opage = page;

  status_line (">FAX [get_fax_file]");

  do
  {
    sprintf (buf, "%spage%02x%02x.fax", fax_in, (TaskNumber & 0xff), opage++);
  }
  while (dexists (buf) && (opage < 256));

  if (opage == 256)
  {
    status_line (MSG_TXT (M_FAX_CREATE_FAILED), "");
    return (PAGE_ERROR);
  }

  if ((result = faxmodem_receive_page (page)) == 0)
  {
    /* filename to create for this page of document */
    if ((fp = fopen (buf, write_binary)) == NULL)
    {
      status_line (MSG_TXT (M_FAX_CREATE_FAILED), buf);
      return (PAGE_ERROR);
    }

    if (!page)
      status_line (MSG_TXT (M_FAX_CONNECT_WITH), response.remote_id);

    sprintf (j, " FAX Rcv %s; page %02x", buf, page);
    status_line (j);

    sprintf (j, " vr%d br%d wd%d ln%d df%d ec%d bf%d st%d",
             response.T30.vr, response.T30.br, response.T30.wd,
             response.T30.ln, response.T30.df, response.T30.ec,
             response.T30.bf, response.T30.st);
    status_line (j);

    result = read_g3_stream (fp, page);
  }

  if (fp != NULL)
  {
    fclose (fp);
    if (faxsize <= 256L)
      unlink (buf);
    else
      status_line (MSG_TXT (M_FAX_FILE_RECEIVED), buf, faxsize);
  }

  return (result);
}

/* Reads a data stream from the faxmodem, unstuffing DLE characters.
 * Returns the +FET value (2 if no more pages) or 4 if hangup.
 */

static int LOCALFUNC
read_g3_stream (FILE * fp, int page)
{
  register short c;
  unsigned char *secbuf, *p;
  long ltimer = 0L;             /* MB 94-01-01 */
  int pseudo_carrier;           /* MB 94-01-01 */

  status_line (">FAX [read_g3_stream]");

  happy_compiler = page;        /* Make compiler happy      */
  response.post_page_response_code = -1;  /* reset page codes         */
  response.post_page_message_code = -1;

  CLEAR_INBOUND ();             /* flush echoes or return codes */

  if ((secbuf = (unsigned char *) calloc (1, 1024)) == NULL)
    goto fax_error;

  p = secbuf;

  /* fax_status (ultoa (faxsize, e_input_buf, 10)); */

  pseudo_carrier = !(CARRIER);  /* test if modem sets DCD */
  if (pseudo_carrier)
    status_line (">FAX modem doesn't assert DCD [read_g3_stream]");

  status_line (">FAX DC2  [read_g3_stream]");

  /* Send DC2 to start phase C data stream */

  SENDBYTE ((unsigned char) DC2);

  while (pseudo_carrier || CARRIER)  /* data only when carrier high */
  {
    if (!CHAR_AVAIL ())         /* if nothing ready, */
    {
      if (pseudo_carrier)       /* MB 94-01-01 */
      {                         /* process timeout if modem does not   */
        /* set DCD, this is only a kludge, but */
        /* it could prevent an endless loop    */
        if (!ltimer)
          ltimer = timerset (15 * PER_SECOND);  /* 15 secs timeout */
        else if (timeup (ltimer))
          goto fax_error;       /* Houston, we lost the downlink   */
      }
      time_release ();
      continue;                 /* process timeouts */
    }
    else
      ltimer = 0L;              /* reset no char waiting timer */

    c = (char) (MODEM_IN () & 0xff);  /* get a character  */

    if (c == DLE)               /* DLE handling     */
    {
      long ltimer2 = 0L;

      while (!CHAR_AVAIL ())
      {
        if (!ltimer2)
          ltimer2 = timerset (4 * PER_SECOND);
        else if (timeup (ltimer2))
        {
          faxsize = 0L;
          goto fax_error;       /* give up */
        }
      }

      c = (char) TIMED_READ (0);

      if (c == ETX)             /* end of stream */
        goto end_page;

      /* DLE DLE gives DLE. We don't know what to do if it
         isn't ETX (above) or DLE. So we'll just always treat
         DLE (not ETX) as (not ETX).

         Fall out of here into storage. */
    }

    *p++ = swaptable[(unsigned char) c];
    faxsize++;

    if (!(faxsize % 1024))
    {
      /* fax_status (ultoa (faxsize, e_input_buf, 10)); */
      if (fwrite (secbuf, 1, 1024, fp) != 1024)
      {
        goto fax_error;         /* hoppala */
      }
      p = secbuf;
      time_release ();
    }
  }

end_page:

  if (faxsize % 1024)
  {
    if (fwrite (secbuf, 1, (size_t) (faxsize % 1024), fp) != (size_t) (faxsize % 1024))
      goto fax_error;           /* hoppala */
    /* fax_status (ultoa (faxsize, e_input_buf, 10)); */
  }

  free (secbuf);

  status_line (">FAX Waiting for +FET/+FHNG  [read_g3_stream]");
  c = 0;
  while (response.post_page_message_code == -1)  /* wait for +FET */
  {
    get_modem_result_code ();
    c++;
    if ((!response.post_page_response_code) || (c > 5) || (response.error))
      return (PAGE_ERROR);
    if (response.hangup_code != -1)
      return (PAGE_HANGUP);
  }
  return (PAGE_GOOD);

fax_error:

  if (secbuf != NULL)
    free (secbuf);
  status_line (MSG_TXT (M_FAX_RECEIVE_PAGE_ERROR));
  get_modem_result_code ();
  return (PAGE_ERROR);
}

/*--------------------------------------------------------------------------*/
/* Class 2 Faxmodem Protocol Functions                                      */
/*                                                                          */
/* Taken from EIA Standards Proposal No. 2388: Proposed New Standard        */
/* "Asynchronous Facsimile DCE Control Standard" (if approved,              */
/* to be published as EIA/TIA-592)                                          */
/*--------------------------------------------------------------------------*/

/* reads a line of characters, terminated by a newline */

static void LOCALFUNC
get_faxline (char *p, int nbytes, unsigned int wtime)
{
  short c;                      /* current modem character   */
  int count = 1;                /* character count (+null)   */
  long t;
  char *resp;

  t = timerset (wtime);

  resp = p;

  while ((count < nbytes)       /* until we have n bytes,    */
         && (!timeup (t)))      /* or out of time            */
  {
    if (!CHAR_AVAIL ())         /* if nothing ready yet,     */
    {
      time_release ();
      continue;                 /* just process timeouts     */
    }

    c = (char) (MODEM_IN () & 0xff);  /* get a character           */

    if (c == '\n')
      continue;

    if (c == '\r')
    {
      if (count > 1)
        break;                  /* get out                   */
      else
        continue;               /* otherwise just keep going */
    }

    *p++ = (char) c;            /* store the character       */
    ++count;                    /* increment the counter     */
  }

  *p = '\0';                    /* terminate the new string  */

  if (debugging_log && (count > 1) && strnicmp (resp, "AT", 2))
    status_line (">FAX %s", resp);  /* pop it on screen  */
}

static void LOCALFUNC
init_swaptable (void)
{
  int i, j;

  for (i = 0; i < 256; i++)
  {
    /* swap the low order 4 bits with the high order */
    /* no. swaps "abcdefgh" to "hgfedcba" /r.hoerner 14 jan 97 */

    j = (((i & 0x01) << 7) |
         ((i & 0x02) << 5) |
         ((i & 0x04) << 3) |
         ((i & 0x08) << 1) |
         ((i & 0x10) >> 1) |
         ((i & 0x20) >> 3) |
         ((i & 0x40) >> 5) |
         ((i & 0x80) >> 7));
    swaptable[i] = (unsigned char) j;
  }
  swaptableinit = TRUE;
}

/****************************************************************
 * Initialize a faxmodem_response struct
 */

static void LOCALFUNC
init_modem_response (void)
{
  response.remote_id[0] = '\0';
  response.fcon = FALSE;
  response.connect = FALSE;
  response.ok = FALSE;
  response.error = FALSE;
  response.hangup_code = -1;
  response.post_page_response_code = -1;
  response.post_page_message_code = -1;
  response.T30.ec = response.T30.bf = 0;
}

/* This function parses numeric responses from the faxmodem.
 * It fills in any relevant slots of the faxmodem_response structure.
 */

static void LOCALFUNC
get_modem_result_code (void)
{
  char buf[256];
  long t;

  status_line (">FAX [get_modem_result_code]");

  t = timerset (4 * PER_SECOND);

  while (!timeup (t))
  {
    buf[0] = '\0';
    get_faxline (buf, 255, 100);
    if (buf[0])
    {
      parse_text_response (buf);
      return;
    }
  }
  return;
}

/*

   static void LOCALFUNC
   fax_status (char *str)
   {
   status_line (str);
   }

 */

static void LOCALFUNC
parse_text_response (char *str)
{
  /* Look for +FCON, +FDCS, +FDIS, +FHNG, +FHS, +FPTS, +FK, +FTSI */

  if (!strnicmp ("+FCO", str, 4))
  {
    response.fcon = TRUE;
    /* fax_status ("+FCO      "); */
    return;
  }

  if (!strnicmp (str, "OK", 2))
  {
    response.ok = TRUE;
    return;
  }

  if (!strnicmp (str, "CONNECT", 7))
  {
    response.connect = TRUE;
    return;
  }

  if (!strnicmp (str, "NO CARRIER", 10) || !strnicmp (str, "ERROR", 5))
  {
    response.error = TRUE;
    response.hangup_code = 0;
    return;
  }

  if (!strnicmp (str, "+FDCS", 5))
  {
    sscanf (str + 6, "%d,%d,%d,%d,%d,%d,%d,%d",
            &response.T30.vr, &response.T30.br, &response.T30.wd,
            &response.T30.ln, &response.T30.df, &response.T30.ec,
            &response.T30.bf, &response.T30.st);
    /* fax_status ("+FDCS     "); */
    return;
  }

  if (!strnicmp (str, "+FHNG", 5))
  {
    sscanf (str + 6, "%d", &response.hangup_code);
    /* fax_status ("+FHNG     "); */
    return;
  }

  if (!strnicmp (str, "+FPTS", 5))
  {
    sscanf (str + 6, "%d", &response.post_page_response_code);
    /* fax_status ("+FPTS     "); */
    return;
  }

  if (!strnicmp (str, "+FTSI", 5))
  {
    strcpy (response.remote_id, str + 6);
    /* fax_status ("+FTSI     "); */
    return;
  }

  if (!strnicmp (str, "+FET", 4))
  {
    sscanf (str + 5, "%d", &response.post_page_message_code);
    /* fax_status ("+FET      "); */
    return;
  }

  if (!strnicmp (str, "+FHS", 4))  /* Class 2.0 */
  {
    sscanf (str + 5, "%d", &response.hangup_code);
    /* fax_status ("+FHS      "); */
    return;
  }

  if (!strnicmp (str, "+FCS", 4))  /* Class 2.0 */
  {
    sscanf (str + 5, "%d,%d,%d,%d,%d,%d,%d,%d",
            &response.T30.vr, &response.T30.br, &response.T30.wd,
            &response.T30.ln, &response.T30.df, &response.T30.ec,
            &response.T30.bf, &response.T30.st);
    /* fax_status ("+FCS      "); */
    return;
  }

  if (!strnicmp (str, "+FPS", 4))  /* Class 2.0 */
  {
    sscanf (str + 5, "%d", &response.post_page_response_code);
    /* fax_status ("+FPS      "); */
    return;
  }

  if (!strnicmp (str, "+FTI", 4))  /* Class 2.0 */
  {
    strcpy (response.remote_id, str + 5);
    /* fax_status ("+FTI      "); */
    return;
  }

}

/****************************************************************
 * Action Commands
 */

/* Receive a page
 * after receiving OK,
 * send +FDR
 * This is somewhat ugly, because the FDR command can return
 * a couple of possible results;
 * If the connection is valid, it returns something like
 *  +FCFR
 *  +FDCS: <params>
 *  CONNECT
 *
 * If, on the other hand, the other machine has hung up, it returns
 * +FHNG: <code>  or
 * +FHS: <code>
 *
 * and if the connection was never made at all, it returns ERROR (actually numeric
 * code 4)
 *
 * faxmodem_receive_page returns values:
 * PAGE_GOOD     page reception OK, data coming
 * PAGE_HANGUP   normal hangup
 * PAGE_ERROR    page error
 */

static int LOCALFUNC
faxmodem_receive_page (int page)
{
  long t;
  char buf[100];

  happy_compiler = page;        /* CE 960417 */

  faxsize = 0L;
  response.connect = response.ok = FALSE;

  /* We wait until a string "OK" is seen
   * or a "+FHNG"
   * or a "ERROR" or "NO CARRIER"
   * or until 10 seconds for a response.
   */

  t = timerset (10 * PER_SECOND);

  status_line (">FAX Waiting for OK  [faxmodem_receive_page]");

  while (!timeup (t) && (!response.ok))
  {
    get_faxline (buf, 100, 100);
    status_line ("> Response from peer: %s", buf);
    parse_text_response (buf);

    if (response.hangup_code != -1)
      return (PAGE_HANGUP);

    if (response.error)
      return (PAGE_ERROR);
  }

  if (!response.ok)
    return (PAGE_ERROR);

  SENDCHARS ("AT+FDR\r", 7, 1);
  status_line (">FAX AT+FDR  [faxmodem_receive_page]");

  /* We wait until either a string "CONNECT" is seen
     * or a "+FHNG"
     * or until 10 seconds for a response.
   */

  t = timerset (10 * PER_SECOND);

  status_line (">FAX Waiting for CONNECT  [faxmodem_receive_page]");

  while (!timeup (t))
  {
    get_faxline (buf, 100, 100);
    status_line ("> Response from peer: %s", buf);
    parse_text_response (buf);

    if (response.connect == TRUE)
      return (PAGE_GOOD);

    if (response.hangup_code != -1)
      return (PAGE_HANGUP);

    if (response.error)
      return (PAGE_ERROR);
  }

  return (PAGE_ERROR);
}

/* $Id: b_faxrcv.c,v 1.5 1999/03/23 22:28:33 mr Exp $ */
