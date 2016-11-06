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
 * Filename    : $Source: E:/cvs/btxe/src/xmrec.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:57 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Xmodem Receiver State Machine.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static long x_no_sealink;

void Find_Char (int);
int Header_in_data (unsigned char *);
void Send_ACK (XMARGSP);
void Send_NAK (XMARGSP);
long Open_Xmodem_File (XMARGSP);

int XRInit (XMARGSP);
int XREnd (XMARGSP);
int XRRecInit (XMARGSP);
int XRBrecInit (XMARGSP);
int XRRecStart (XMARGSP);
int XRWaitFirst (XMARGSP);
int XRWaitBlock (XMARGSP);
int XRRestart (XMARGSP);
int XRSetOvrdr (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
XSTATES, *XSTATEP;

XSTATES Xmodem_Receiver[] =
{
  {"XRInit", XRInit},
  {"XREnd", XREnd},
  {"XR0", XRRecInit},
  {"XR0B", XRBrecInit},
  {"XR1", XRRecStart},
  {"XR2", XRWaitFirst},
  {"XR3", XRWaitBlock},
  {"XR4", XRRestart},
  {"XR5", XRSetOvrdr}
};

long
Open_Xmodem_File (XMARGSP args)
{
  char *s1, *s2;

  if (args->file_pointer == NULL)
  {
    args->temp_name = calloc (1, 80);

    if (args->path != NULL)
      strcpy (args->temp_name, args->path);

    s1 = &(args->temp_name[strlen (args->temp_name)]);
    strcpy (s1, "BTXXXXXX");

    s2 = mktemp (args->temp_name);
    if (s2 != NULL)
      args->file_pointer = fopen (s2, write_binary);

    if (args->file_pointer == NULL)
    {
      status_line (MSG_TXT (M_TEMP_NOT_OPEN), s2);
      return (-1L);
    }
  }

  throughput (0, 0, 0L);
  return (0L);
}

long
Set_Up_Restart (XMARGSP args)
{
  char foo[100];
  char foo1[50];
  struct stat st;

  args->sub_results = 0;

  /* Look for file in directory */
  if (args->path != NULL)
    strcpy (foo, args->path);

  if ((args->received_name != NULL) &&
      (strlen (args->received_name) > 0) &&
      args->options.Resync)
  {
    strcat (foo, args->received_name);

    if (unix_stat_noshift (foo, &st) == 0)
    {
      if ((st.st_size == args->filelen) && (st.st_mtime == (time_t) (args->save_filetime.oneword.timedate)))
      {
        args->file_pointer = fopen (foo, read_binary_plus);
        if (args->file_pointer != NULL)
        {
          throughput (0, 0, 0L);
          fseek (args->file_pointer, 0L, SEEK_END);
          args->sub_results = DID_RESYNC;
          args->temp_name = calloc (1, 80);
          strcpy (args->temp_name, foo);
          args->prev_bytes = args->filelen;
          status_line (MSG_TXT (M_ALREADY_HAVE), foo);
          status_line (MSG_TXT (M_SYNCHRONIZING_EOF));
          return (args->total_blocks + 1L);
        }
      }
    }

    /* Look for file in .Z file */
    if (dexists (Abortlog_name))
    {
      sprintf (Resume_info, "%ld %lo", args->filelen, args->save_filetime.oneword.timedate);
      if (check_failed (Abortlog_name, args->received_name, Resume_info, foo1))
      {
        foo[0] = '\0';
        /* Here it looks like it was a failed WaZOO session */
        if (args->path != NULL)
          strcpy (foo, args->path);
        strcat (foo, foo1);

        args->file_pointer = fopen (foo, read_binary_plus);
        if (args->file_pointer != NULL)
        {
          unix_stat (foo, &st);
          throughput (0, 0, 0L);
          args->temp_name = calloc (1, 80);
          strcpy (args->temp_name, foo);
          args->prev_bytes = (st.st_size / 128L) * 128L;
          fseek (args->file_pointer, args->prev_bytes, SEEK_SET);
          status_line (MSG_TXT (M_SYNCHRONIZING), args->prev_bytes);
          return (args->prev_bytes / 128L + 1L);
        }
      }
    }
  }

  return (Open_Xmodem_File (args));
}

void
Finish_Xmodem_Receive (XMARGSP args)
{
  struct stat st;
  char new_name[PATHLEN];
  struct utimbuf times;
  int i, j, k;

  /* Set the file's time and date stamp */
  if ((args->save_header == SOH) || (args->save_header == SYN))
  {
    fclose (args->file_pointer);
    times.modtime = times.UT_ACTIME =
      (time_t) args->save_filetime.oneword.timedate;
    unix_utime_noshift (args->temp_name, (UTIMBUF *) & times);
  }
  else
  {
    strcpy (args->received_name, "");
    if (args->file_pointer)
      fclose (args->file_pointer);
  }

  if (args->result == SUCCESS)
  {
    long lFileTime;
    long lFileSize;

    /* Get the file information */
    unix_stat (args->temp_name, &st);

    lFileSize = st.st_size - args->prev_bytes;
    lFileTime = throughput (1, 0, (unsigned long) lFileSize) / PER_SECOND;

    if (args->sub_results & DID_RESYNC)
    {
      status_line ("%s: %s", MSG_TXT (M_FILE_RECEIVED), args->temp_name);
      update_files (0, args->temp_name, lFileSize, lFileTime, &(RxStats.cur_errors));
    }
    else
    {
      new_name[0] = '\0';

      if (args->path != NULL)
        strcpy (new_name, args->path);

      if ((args->filename == NULL) || (strlen (args->filename) == 0))
      {
        if (strlen (args->received_name) > 0)
          strcat (new_name, args->received_name);
        else
          strcat (new_name, "BAD_FILE.000");
      }
      else
        strcat (new_name, args->filename);

      i = (int) strlen (args->temp_name) - 1;
      j = (int) strlen (new_name) - 1;

      if (args->temp_name[i] == '.')
        args->temp_name[i] = '\0';

      if (new_name[j] == '.')
      {
        new_name[j] = '\0';
        --j;
      }

      i = 0;
      k = is_arcmail (new_name, j);

      status_line ("%s: %s", MSG_TXT (M_FILE_RECEIVED), new_name);
      update_files (0, new_name, lFileSize, lFileTime, &(RxStats.cur_errors));
      RxStats.cur_fxfrd++;
      RxStats.cur_mxfrd += lFileSize;
      RxStats.FilePos = RxStats.FileLen = -1L;
      css_upd ();

      if ((!overwrite) || k)
      {
        while (rename (args->temp_name, new_name))
        {
          if (isdigit (new_name[j]))
            new_name[j]++;
          else
            new_name[j] = '0';

          if (!isdigit (new_name[j]))
            return;

          i = 1;
        }
        CLEAR_IOERR ();
      }
      else
      {
        unlink (new_name);
        while (rename (args->temp_name, new_name))
        {
          if (!i)
            status_line (MSG_TXT (M_ORIGINAL_NAME_BAD), new_name);

          if (isdigit (new_name[j]))
            new_name[j]++;
          else
            new_name[j] = '0';

          if (!isdigit (new_name[j]))
            return;

          i = 1;
        }

        CLEAR_IOERR ();
      }
      if (i)
        status_line (MSG_TXT (M_RENAME_MSG), new_name);
    }

    remove_abort (Abortlog_name, args->received_name);
  }
  else
  {
    if ((args->received_name != NULL) && (strlen (args->received_name) > 0) && (args->save_header != 0))
    {
      sprintf (Resume_info, "%ld %lo", args->filelen, args->save_filetime.oneword.timedate);
      add_abort (Abortlog_name, args->received_name, args->temp_name, args->path, Resume_info);
    }
    else
    {
      /* File aborted, so remove all traces of it */
      if (args->temp_name != NULL)
        unlink (args->temp_name);
    }
  }

  if (args->temp_name != NULL)
    free (args->temp_name);
}

void
Get_Telink_Info (XMARGSP args)
{
  char *p1;
  char junkbuff[100];
  TLDATAP t;
  unsigned int i, j;
  struct tm tmstruc;
  time_t curr_time;

  /* Figure out how many blocks we will get */
  t = (TLDATAP) & (args->header);
  args->total_blocks = (t->filelength + 127) / 128;
  t->nullbyte = '\0';
  p1 = strchr (t->filename, ' ');

  if (p1 != NULL)
    *p1 = '\0';

  /* Rename .REQ files */

  i = strlen (t->filename) - 4;
  if ((i > 0) && (stricmp (&(t->filename)[i], ".req") == 0))
  {
    (t->filename)[i] = '\0';
    status_line (MSG_TXT (M_REC_REQ_AS), t->filename, t->filename, TaskNumber);
    sprintf (&(t->filename)[i], ".r%02x", TaskNumber);
  }

  strcpy (args->received_name, t->filename);
  args->save_header = args->header;
  if (args->save_header == SYN)
  {
    i = t->filetime.twowords.time;
    j = t->filetime.twowords.date;

    curr_time = unix_time (NULL);
    tmstruc = *unix_localtime (&curr_time);  /* Structure assignment */

    tmstruc.tm_year = (j >> 9) + 80;
    tmstruc.tm_mon = ((j >> 5) & 0x0f) - 1;
    tmstruc.tm_mday = j & 0x1f;

    tmstruc.tm_hour = i >> 11;
    tmstruc.tm_min = (i >> 5) & 0x3f;
    tmstruc.tm_sec = i & 0x1f;

    args->save_filetime.oneword.timedate = unix_mktime_noshift (&tmstruc);
  }
  else
    args->save_filetime.oneword.timedate = t->filetime.oneword.timedate;

  args->filelen = t->filelength;
  strncpy (args->sending_program, t->sendingprog, 15);

  /* "Rcv %ld blks of %s from %s (%ld bytes)." */

  sprintf (junkbuff, MSG_TXT (M_RECEIVE_MSG),
           args->total_blocks,
           args->received_name,
           args->sending_program,
           args->filelen);

  strcpy (RxStats.fname, fname8p3 (args->received_name));
  RxStats.FilePos = 0;
  RxStats.FileLen = t->filelength;
  css_upd ();
}

int
Read_Block (XMARGSP args)
{
  unsigned char *p;             /* Pointers to XMODEM data */
  int i;                        /* Counter */
  int j;                        /* Counter start */
  unsigned char c;              /* character being processed */
  int in_char;
  char junkbuff[128];
  long head_timer;
  struct _pkthdr *packet;       /* FTS-0001 packet type */
  struct _pkthdr45 *pkt0045;    /* FSC-0045 packet type */
  struct _pkthdr39 *pkt0039;    /* FSC-0039 packet type */
  unsigned int cwtest;          /* Used to verify FSC-0039 type */

  if (got_ESC ())
  {
    status_line (MSG_TXT (M_KBD_MSG));
    return (KBD_ERR);
  }

  /* Set up to point into the XMODEM data structure */
  p = (unsigned char *) &(args->header);

  /* Get the first character that is waiting */
  *p = (unsigned char) TIMED_READ (8);

  head_timer = timerset (PER_MINUTE);
  j = 1;
  while (!timeup (head_timer))
  {
    /* Now key off of the header character */
    switch (*p)
    {
    case EOT:                  /* End of file */
      /* Is this a valid EOT */
      if (args->total_blocks <= args->WriteBLK)
        return (EOT_BLOCK);
      else
      {
        status_line (MSG_TXT (M_UNEXPECTED_EOF), args->total_blocks);
        return (BAD_BLOCK);
      }

    case SYN:                  /* Telink block */
      /* For Telink, read all of the data except the checksum */
      for (i = 1; i < sizeof (TLDATA) - 2; i++)
      {
        /* If we go more than 5 second, then we have a short block */
        if ((in_char = TIMED_READ (5)) < 0)
          return (BAD_BLOCK);

        *(++p) = (unsigned char) (in_char & 0xff);
      }

      /* if the block number or its complement are wrong, return error */
      if ((args->block_num != 0) || (args->block_num_comp != 0xff))
      {
        RxStats.cur_errors++;
        return (BAD_BLOCK);
      }

      /* Now calculate the checksum - Telink block always checksum mode */
      Data_Check ((XMDATAP) & (args->header), CHECKSUM);

      /* See if we can receive the checksum byte */
      if ((in_char = TIMED_READ (10)) < 0)
      {
        RxStats.cur_errors++;
        Xmodem_Error (MSG_TXT (M_TIMEOUT), 0L);
        return (BAD_BLOCK);
      }

      /* Was it right */
      c = (unsigned char) (in_char & 0xff);
      if (c != args->data_check[0])
      {
        RxStats.cur_errors++;
        Xmodem_Error (MSG_TXT (M_CHECKSUM), 0L);
        return (BAD_BLOCK);
      }
      /* Everything looks good, it must be a legal TELINK block */

      Get_Telink_Info (args);
      return (TELINK_BLOCK);

    case SOH:                  /* Normal data block */
      args->datalen = 128;
      /* Read in all of the data for an XMODEM block except the checksum */
      p += (j - 1);
      for (i = j; i < sizeof (XMDATA) - 2; i++)
      {
        /* If we go more than 5 seconds, then it is a short block */
        if ((in_char = TIMED_READ (5)) < 0)
          return (BAD_BLOCK);

        *(++p) = (unsigned char) (in_char & 0xff);
      }

      /* The block number is 0 to 255 inclusive */
      c = (unsigned char) (args->blocknum & 0xff);

      /* Properly calculate the CRC or checksum */
      Data_Check ((XMDATAP) & (args->header), args->options.do_CRC ? CRC : CHECKSUM);

      /* Can we get the checksum byte */
      if ((in_char = TIMED_READ (10)) < 0)
      {
        RxStats.cur_errors++;
        Xmodem_Error (MSG_TXT (M_TIMEOUT), args->WriteBLK);
        return (BAD_BLOCK);
      }

      /* Is it the right value */
      c = (unsigned char) (in_char & 0xff);
      if (c != args->data_check[0])
      {
        status_line (">Xmodem Receive: Bad %s", (args->options.do_CRC) ? "CRC" : "checksum");
        Xmodem_Error (MSG_TXT (M_CRC_MSG), args->WriteBLK);
        RxStats.cur_errors++;
        if (args->options.do_CRC)
          TIMED_READ (5);
        return (BAD_BLOCK);
      }

      /* If we are in CRC mode, do the second byte */
      if (args->options.do_CRC)
      {
        /* Can we get the character */
        if ((in_char = TIMED_READ (10)) < 0)
        {
          status_line (">Xmodem Receive: Timeout waiting for CRC byte 2");
          RxStats.cur_errors++;
          Xmodem_Error (MSG_TXT (M_TIMEOUT), args->WriteBLK);
          return (BAD_BLOCK);
        }

        /* Is it right */
        c = (unsigned char) (in_char & 0xff);
        if (c != args->data_check[1])
        {
          Xmodem_Error (MSG_TXT (M_CRC_MSG), args->WriteBLK);
          RxStats.cur_errors++;
          return (BAD_BLOCK);
        }
      }

      /* Do we have a valid data block */
      if (args->block_num_comp != (unsigned char) ((~(args->block_num)) & 0xff))
      {
        if (!(args->options.SEAlink))
        {
          Xmodem_Error (MSG_TXT (M_JUNK_BLOCK), args->WriteBLK);
          RxStats.cur_errors++;
          return (BAD_BLOCK);
        }

        p = (unsigned char *) &(args->header);
        j = Header_in_data (p);
        if (j)
          continue;

        j = 1;
        Find_Char (SOH);
        *p = (unsigned char) TIMED_READ (0);
      }

      if ((args->WriteBLK == 1) && (args->header == SOH) && (args->block_num == 0))
      {
        Get_Telink_Info (args);
        return (SEALINK_BLOCK);
      }

      if (first_block)
      {
        packet = (struct _pkthdr *) args->data;
        pkt0045 = (struct _pkthdr45 *) packet;
        pkt0039 = (struct _pkthdr39 *) packet;

        if (!remote_capabilities)
        {
          remote_addr.Net = packet->orig_net;
          remote_addr.Node = packet->orig_node;
          if (packet->rate == 2)
          {
            /* This is a FSC-0045 (type 2.2) packet! */
            remote_addr.Zone = packet->orig_zone;
            remote_addr.Point = (unsigned) pkt0045->orig_point;

            strncpy (junkbuff, (char *) (pkt0045->orig_domain), 8);
            junkbuff[8] = '\0';
            remote_addr.Domain = find_domain (junkbuff);
          }
          else
          {
            remote_addr.Domain = NULL;
            cwtest = (((pkt0039->CapValid) & 0x7f00) >> 8) +
              (((pkt0039->CapValid) & 0x007f) << 8);
            if (cwtest == (unsigned int) ((pkt0039->CapWord) & 0x7f7f))
            {
              /* This is a FSC-0039 packet! */
              remote_addr.Zone = pkt0039->orig_zone;
              remote_addr.Point = pkt0039->orig_point;
            }
            else
            {
              remote_addr.Zone = packet->orig_zone;
              remote_addr.Point = 0;
            }
          }

          /* Here we have extracted the Zone, Net, Node, Point and Domain from
           * the packet -- regardless of type. Now see if we need to map to a
           * fake net or to mung the address because it's someone else's
           * point. */

          if ((remote_addr.Point > 0) &&
              (pvtnet >= 0) &&
              ((remote_addr.Zone == alias[assumed].Zone) ||
               (remote_addr.Zone == 0)) &&
              (remote_addr.Net == boss_addr.Net) &&
              (remote_addr.Node == boss_addr.Node))
          {
            remote_addr.Net = pvtnet;
            remote_addr.Node = remote_addr.Point;
            remote_addr.Point = 0;
          }
          else if ((pvtnet >= 0) && (remote_addr.Point > 0))
          {
            remote_addr.Point = 0;
            remote_addr.Node = (unsigned short) -1;
          }
        }

        if (who_is_he)
        {
          if (!remote_addr.Zone && !remote_addr.Net && !remote_addr.Node)
          {
            modem_hangup ();    /* Bad trip, cut it off */
            return (CARRIER_ERR);  /* Get out of here!     */
          }

          if (nodefind (&remote_addr, 1))
          {
            if (!remote_addr.Zone)
              remote_addr.Zone = found_zone;

            sprintf (junkbuff, "%s: %s (%s)",
                     MSG_TXT (M_REMOTE_SYSTEM),
                     newnodedes.SystemName,
                     Full_Addr_Str (&remote_addr));
          }
          else
          {
            sprintf (junkbuff, "%s: %s (%s)",
                     MSG_TXT (M_REMOTE_SYSTEM),
                     MSG_TXT (M_UNKNOWN_MAILER),
                     Full_Addr_Str (&remote_addr));
          }

          last_type (2, &remote_addr);
          status_line (junkbuff);
          show_nodes_name (NULL);
        }

        if (args->sending_program[0] != '\0')
          status_line ("%s %s", MSG_TXT (M_REMOTE_USES), args->sending_program);
        else
          log_product (packet->product, 0, packet->serial);

        who_is_he = 0;
        first_block = 0;
      }

      if (args->WriteBLK == args->total_blocks)
        args->datalen = (int) (args->filelen - ((args->WriteBLK - 1) * 128));

      /* If we got this far, it is a valid data block */
      args->recblock = args->block_num;
      return (XMODEM_BLOCK);

    default:                   /* Bad block */
      if ((args->blocknum <= 1) || (PEEKBYTE () < 0))
        return (BAD_BLOCK);

      /* Garbage header, return bad */
      *p = (unsigned char) TIMED_READ (0);
    }
  }

  return (BAD_BLOCK);
}

int
XRInit (XMARGSP args)
{
  char *HoldName;

  x_no_sealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);

  args->tries = 0;
  args->goodfile = 1;
  XON_DISABLE ();
  HoldName = HoldAreaNameMunge (&called_addr);
  sprintf (Abortlog_name, "%s%s.z", HoldName, Hex_Addr_Str (&remote_addr));
  args->sending_program[0] = '\0';
  return ((int) args->control);
}

int
XREnd (XMARGSP args)
{
  args->result = (int) args->control;
  Finish_Xmodem_Receive (args);
  return ((int) args->control);
}

int
XRRecInit (XMARGSP args)
{
  x_no_sealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);
  args->options.SEAlink = 0;
  args->options.SLO = 0;
  args->options.Resync = 0;
  args->options.MacFlow = 0;
  args->options.do_CRC = 1;
  args->blocknum = 0;
  args->WriteBLK = 1;
  args->curr_byte = 0L;
  args->tries = 0;
  return (XR1);
}

int
XRBrecInit (XMARGSP args)
{
  x_no_sealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);
  args->options.SEAlink = 0;
  args->options.SLO = 0;
  args->options.Resync = 0;
  args->options.MacFlow = 0;
  args->options.do_CRC = 1;
  args->blocknum = 0;
  args->WriteBLK = 1;
  args->curr_byte = 0L;
  args->tries = 0;
  return (XR2);
}

int
XRRecStart (XMARGSP args)
{
  Send_NAK (args);
  return (XR2);
}

int
XRWaitFirst (XMARGSP args)
{
  long XR2Timer;

  XR2Timer = timerset (8 * PER_SECOND);
  if (args->tries >= 10)
  {
    args->goodfile = 0;
    return (TIME_ERR);
  }

  if (args->tries == 5)
  {
    args->options.do_CRC = 0;
    ++(args->tries);
    return (XR1);
  }

  while (CARRIER)
  {
    switch (Read_Block (args))
    {
    case EOT_BLOCK:
      args->WriteBLK = 0;
      Send_ACK (args);
      return (SUCCESS_EOT);

    case TELINK_BLOCK:
      if (Open_Xmodem_File (args) == -1L)
        return (OPEN_ERR);
      Send_ACK (args);
      args->tries = 0;
      return (XR3);

    case SEALINK_BLOCK:
      args->options.SEAlink = x_no_sealink ? 0 : 1;
      if (args->options.SEAlink && !no_resync)
        args->options.Resync = (((SEADATAP) (&(args->header)))->Resync) != 0;
      return (XR4);

    case XMODEM_BLOCK:
      if (args->recblock == 1)
      {
        if (Open_Xmodem_File (args) == -1L)
          return (OPEN_ERR);
        fwrite (args->data, sizeof (unsigned char), args->datalen, args->file_pointer);

        ++(args->WriteBLK);
        args->curr_byte = 128L;
        ++(args->blocknum);
        Send_ACK (args);
        args->tries = 0;
        return (XR3);
      }
      /* Fallthrough on wrong block */

    case BAD_BLOCK:
      ++(args->tries);
      RxStats.cur_errors++;
      return (XR1);

    case CARRIER_ERR:
    case KBD_ERR:
      RxStats.cur_errors++;
      return (CARRIER_ERR);
    }

    if (timeup (XR2Timer))
    {
      ++(args->tries);
      return (XR1);
    }
  }

  return (CARRIER_ERR);
}

int
XRWaitBlock (XMARGSP args)
{
  int blocknum_copy;

  if (args->tries >= 10)
  {
    args->goodfile = 0;
    return (TIME_ERR);
  }

  while (CARRIER)
  {
    switch (Read_Block (args))
    {
    case EOT_BLOCK:
      args->options.SLO = 0;
      Send_ACK (args);
      return (SUCCESS);

    case XMODEM_BLOCK:
      blocknum_copy = (int) args->blocknum & 0xff;
      if (args->recblock == ((blocknum_copy - 1) & 0xff))
      {
        --(args->blocknum);
        Send_ACK (args);
        return (XR3);
      }

      if (args->recblock == blocknum_copy)
      {
        fwrite (args->data, sizeof (unsigned char), args->datalen, args->file_pointer);

        ++(args->WriteBLK);
        args->curr_byte += 128L;
        Send_ACK (args);
        args->tries = 0;
        return (XR3);
      }

      if (args->recblock < blocknum_copy)
        args->recblock += 256;

      if ((args->recblock > blocknum_copy) && (args->recblock <= ((blocknum_copy + 127) & 0xff)))
      {
        if (args->tries != 0)
        {
          /* We have sent at least one nak, now only send them
           * every so often to allow buffers to drain */
          if ((args->recblock - blocknum_copy) % 16)
            return (XR3);

          /* If it is a multiple of 16, then check that it is
           * higher than 32 */
          if ((args->recblock - blocknum_copy) / 16 < 2)
            return (XR3);
        }
      }

      /* fallthrough on bad block */

    case BAD_BLOCK:
      Send_NAK (args);
      ++(args->tries);
      return (XR3);

    case CARRIER_ERR:
    case KBD_ERR:
      RxStats.cur_errors++;
      return (CARRIER_ERR);
    }
  }

  return (CARRIER_ERR);
}

int
XRRestart (XMARGSP args)
{
  long c;

  c = Set_Up_Restart (args);
  if (c == -1L)
    return (OPEN_ERR);

  if ((!c) || (!(args->options.Resync)))
  {
    Send_ACK (args);
    args->tries = 0;
  }
  else
  {
    args->WriteBLK = c;
    args->curr_byte = (c - 1) * 128L;
    args->blocknum = (unsigned char) ((args->WriteBLK) & 0xff);
    Send_NAK (args);
  }

  return (XR5);
}

int
XRSetOvrdr (XMARGSP args)
{
  if (!no_overdrive)
    args->options.SLO = (((SEADATAP) (&(args->header)))->SLO) != 0;

  if (args->options.SLO)
    show_block ((long) (args->WriteBLK - 1), NULL, args);

  /* was: " *Overdrive*" */
  return (XR3);
}

int
Internal_Xmodem_Receive_File (char *path, char *filename)
{
  XMARGS xmfile;
  int res;

  // locate_y = wherey ();
  // locate_x = wherex ();
  memset (&xmfile, 0, sizeof (XMARGS));
  xmfile.path = path;
  xmfile.filename = filename;
  xmfile.total_blocks = -1L;
  xmfile.sent_ACK = 0;
  res = state_machine ((STATEP) Xmodem_Receiver, &xmfile, XR0);

  RxStats.tot_errors += RxStats.cur_errors;
  hist.err_in += RxStats.cur_errors;
  RxStats.cur_errors = 0;
  return res;
}

int
Xmodem_Receive_File (char *path, char *filename)
{
  int res;

  res = Internal_Xmodem_Receive_File (path, filename);
  status_line (">XModem_Receive_File(%s) returns %d",
               filename == NULL ? "(null)" : filename, res);
  if (res == SUCCESS)
    return (FILE_RECV_OK);
  else
    return (res == SUCCESS_EOT ? NOTHING_RECVED : CANNOT_RECV);
}

int
Internal_Batch_Xmodem_Receive_File (char *path, char *filename)
{
  XMARGS xmfile;
  int res;

  // locate_y = wherey ();
  // locate_x = wherex ();
  memset (&xmfile, 0, sizeof (XMARGS));
  xmfile.path = path;
  xmfile.filename = filename;
  xmfile.total_blocks = -1L;
  xmfile.sent_ACK = 0;
  res = state_machine ((STATEP) Xmodem_Receiver, &xmfile, XR0B);

  RxStats.tot_errors += RxStats.cur_errors;
  hist.err_in += RxStats.cur_errors;
  RxStats.cur_errors = 0;
  return res;
}


int SAInit (XMARGSP);
int SAEnd (XMARGSP);
int SAClearLine (XMARGSP);
int SASendACK (XMARGSP);
int SASEAlink (XMARGSP);
int SAIncBlk (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
ASTATES, *ASTATEP;

ASTATES ACK_States[] =
{
  {"SAInit", SAInit},
  {"SAEnd", SAEnd},
  {"SA0", SAClearLine},
  {"SA1", SASendACK},
  {"SA2", SASEAlink},
  {"SA3", SAIncBlk}
};

int
SAInit (XMARGSP args)
{
  return ((int) args->control);
}

int
SAEnd (XMARGSP args)
{
  return ((int) args->control);
}

int
SAClearLine (XMARGSP args)
{
  long SA0Timer;

  SA0Timer = timerset (30 * PER_SECOND);
  if (args->options.SLO)
    return (SA3);

  if (args->options.SEAlink)
    return (SA1);

  while (CARRIER && !timeup (SA0Timer))
  {
    if (PEEKBYTE () >= 0)
    {
      TIMED_READ (0);
      time_release ();
      continue;
    }

    return (SA1);
  }

  return (TIME_ERR);
}

int
SASendACK (XMARGSP args)
{
  SENDBYTE (ACK);
  args->sent_ACK = 1;
  return (SA2);
}

int
SASEAlink (XMARGSP args)
{
  if (!(args->options.SEAlink))
    return (SA3);

  SENDBYTE (args->blocknum);
  SENDBYTE ((unsigned char) ~(args->blocknum));
  return (SA3);
}

void
show_block (long b, char *error, XMARGSP args)
{
  char junk[16];
  long k;

  ultoa (((unsigned long) b), junk, 10);

  RxStats.FilePos = args->curr_byte;
  css_upd ();

  k = args->filelen - args->curr_byte;
  if (k < 0L)
    k = 0L;

  if (error)
    status_line ("!%s", error);
}

int
SAIncBlk (XMARGSP args)
{
  ++(args->blocknum);
  if ((args->options.SLO) &&
      (((args->WriteBLK > 0) && (!((args->WriteBLK - 1) & 0x001F)) &&
        (args->WriteBLK < args->total_blocks)) ||
       (args->WriteBLK >= args->total_blocks)))
  {
    show_block ((long) (args->WriteBLK - 1), NULL, args);
    /* was: " *Overdrive*" */
  }
  else if ((!(args->options.SLO)) && (args->WriteBLK > 0))
    show_block ((long) (args->WriteBLK - 1), NULL, args);

  return (SUCCESS);
}

void
Send_ACK (XMARGSP args)
{
  state_machine ((STATEP) ACK_States, args, SA0);
}

void Send_Resync_Packet (XMARGSP);

int SNInit (XMARGSP);
int SNEnd (XMARGSP);
int SNClearLine (XMARGSP);
int SNSendNAK (XMARGSP);
int SNSEAlink (XMARGSP);
int SNAckResync (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
NSTATES, *NSTATEP;

NSTATES NAK_States[] =
{
  {"SNInit", SNInit},
  {"SNEnd", SNEnd},
  {"SN0", SNClearLine},
  {"SN1", SNSendNAK},
  {"SN2", SNSEAlink},
  {"SN3", SNAckResync}
};

int
SNInit (XMARGSP args)
{
  return ((int) args->control);
}

int
SNEnd (XMARGSP args)
{
  return ((int) args->control);
}

int
SNClearLine (XMARGSP args)
{
  long SN0Timer;

  SN0Timer = timerset (30 * PER_SECOND);
  if (args->options.Resync)
  {
    Send_Resync_Packet (args);
    return (SN3);
  }

  if (args->options.SEAlink)
    return (SN1);

  while (CARRIER && !timeup (SN0Timer))
  {
    if (PEEKBYTE () >= 0)
    {
      TIMED_READ (0);
      time_release ();
      continue;
    }

    return (SN1);
  }

  return (TIME_ERR);
}

int
SNSendNAK (XMARGSP args)
{
  if (args->options.do_CRC && (args->sent_ACK == 0))
    SENDBYTE (WANTCRC);
  else
    SENDBYTE (NAK);

  return (SN2);
}

int
SNSEAlink (XMARGSP args)
{
  if (!(args->options.SEAlink))
    return (SUCCESS);

  SENDBYTE (args->blocknum);
  SENDBYTE ((unsigned char) ~(args->blocknum));
  return (SUCCESS);
}

int
SNAckResync (XMARGSP args)
{
  long SN3Timer;
  int c;

  SN3Timer = timerset (30 * PER_SECOND);

  while (CARRIER && !timeup (SN3Timer))
  {
    if ((unsigned int) (c = TIMED_READ (10)) == 0xffff)
    {
      Send_Resync_Packet (args);
      continue;
    }

    if (c == ACK)
    {
      big_pause (1);
      c = PEEKBYTE ();
      if ((c == SOH) || (c == EOT))
        return (SUCCESS);
    }
  }

  if (!CARRIER)
    return (CARRIER_ERR);
  else
    return (TIME_ERR);
}

void
Send_NAK (XMARGSP args)
{
  state_machine ((STATEP) NAK_States, args, SN0);
}

void
Send_Resync_Packet (XMARGSP args)
{
  unsigned char resyncit[30];
  unsigned short nak_crc;

  SENDBYTE (SYN);
  sprintf ((char *) resyncit, "%ld", args->WriteBLK);
  SENDCHARS ((char *) resyncit, strlen ((char *) resyncit), 1);
  nak_crc = crc_block ((unsigned char *) resyncit, (int) strlen ((char *) resyncit));
  SENDBYTE (ETX);
  SENDBYTE ((unsigned char) (nak_crc & 0xff));
  CLEAR_INBOUND ();
  SENDBYTE ((unsigned char) (nak_crc >> 8));
}

void
Xmodem_Error (char *s, long block_number)
{
  char j[50];
  char k[50];

  sprintf (j, "%s %s %ld", s, MSG_TXT (M_ON_BLOCK), block_number);
  sprintf (k, "%-49.49s", j);
  status_line ("!Xmodem Error: %s", k);
}

void
Find_Char (int c)
{
  long t1;
  long t2;

  t1 = timerset (30 * PER_SECOND);
  t2 = timerset (PER_SECOND);
  while (!timeup (t1) && !timeup (t2))
  {
    if (!CARRIER)
      break;

    if (PEEKBYTE () == (c & 0xff))
      break;
    else if (PEEKBYTE () >= 0)
    {
      TIMED_READ (0);
      t2 = timerset (PER_SECOND);
    }
  }
}

int
Header_in_data (unsigned char *p)
{
  int i;
  int j;
  char *p1;

  p1 = (char *) p;
  ++p1;
  j = sizeof (XMDATA) - 2;
  for (i = 1; i < j; i++, p1++)
  {
    if (*p1 == SOH)
    {
      memcpy (p, p1, (unsigned int) (j - i));
      return (j - i);
    }
  }

  return (0);
}

/* $Id: xmrec.c,v 1.7 1999/03/23 22:28:57 mr Exp $ */
