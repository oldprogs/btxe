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
 * Filename    : $Source: E:/cvs/btxe/src/xmsend.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/20 04:08:29 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Xmodem Sender State Machine.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

static long x_no_sealink;

void
Build_Header_Block (XMARGSP args, char type)
{
  SEADATAP ttmp;
  struct FILEINFO dta;

  dfind (&dta, args->filename, 0);
  args->save_header = type;
  ttmp = (SEADATAP) & (args->header);
  memset (ttmp, 0, sizeof (XMDATA));
  ttmp->header = type;
  ttmp->block_num = 0;
  ttmp->block_num_comp = 0xff;
  ttmp->filelength = args->filelen;
  strncpy (ttmp->sendingprog, xfer_id, 14);

  if (type == SYN)
  {
    memset (ttmp->filename, ' ', 16);
    ttmp->timedate = dta.time;  //UTC???
    /* This is the CRC bit in the TeLink header */
    ttmp->Resync = 1;
  }
  else
  {
    ttmp->timedate = args->save_filetime.oneword.timedate;
    ttmp->SLO = (unsigned char) (((cur_baud.rate_value >= 9600L) && !no_overdrive) ? 1 : 0);
    ttmp->Resync = (unsigned char) (no_resync ? 0 : 1);
    ttmp->MACFLOW = 1;
  }

  if (args->temp_name != NULL)
    strncpy (ttmp->filename, args->temp_name, strlen (args->temp_name));
  else
    strncpy (ttmp->filename, (char *) (dta.name), strlen (dta.name));

  dfind (&dta, NULL, 2);
}

void XSSetVars (XMARGSP);
int XSInit (XMARGSP);
int XSEnd (XMARGSP);
int XSXmtStart (XMARGSP);
int XSXmTeStrt (XMARGSP);
int XSCheckACK (XMARGSP);
int XSSendBlk (XMARGSP);
int XSWaitEnd (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
XSTATES, *XSTATEP;

XSTATES Xmodem_Sender[] =
{
  {"XSInit", XSInit},
  {"XSEnd", XSEnd},
  {"XS0", XSXmtStart},
  {"XS0T", XSXmTeStrt},
  {"XS1", XSCheckACK},
  {"XS2", XSSendBlk},
  {"XS3", XSWaitEnd}
};

int
XSInit (XMARGSP args)
{
  struct stat st;
  char junkbuff[100];

  x_no_sealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);

  /* Get the file information */
  if (unix_stat_noshift (args->filename, &st))
    return (OPEN_ERR);          /* Print error message */

  args->file_pointer = share_fopen (args->filename, read_binary, DENY_WRITE);
  if (args->file_pointer == NULL)
    return (OPEN_ERR);          /* Print error message */

  /* Get important information out of it */
  args->filelen = st.st_size;
  args->LastBlk = (st.st_size + 127) / 128;
  args->save_filetime.oneword.timedate = st.st_mtime;
  args->prev_bytes = 0L;
  args->tot_errs = 0;

  sprintf (junkbuff, MSG_TXT (M_SEND_MSG),
           args->LastBlk,
           args->filename,
           args->filelen);

  strcpy (TxStats.fname, fname8p3 (args->filename));
  TxStats.FilePos = 0;
  TxStats.FileLen = args->filelen;
  css_upd ();

  /* Start the throughput calculations */
  throughput (0, 1, 0L);
  return ((int) args->control);
}

int
XSEnd (XMARGSP args)
{
  args->result = (int) args->control;

  /* Close file */
  if (args->file_pointer)
    fclose (args->file_pointer);

  if (args->tot_errs > 3)
    status_line (MSG_TXT (M_CORRECTED_ERRORS), args->tot_errs, args->LastBlk);

  /* Log that we sent it */
  if (args->result == SUCCESS)
  {
    long lTime, lSize;

    lSize = args->filelen - args->prev_bytes;
    lTime = throughput (1, 1, (unsigned long) lSize) / PER_SECOND;
    status_line ("%s-X: %s", MSG_TXT (M_FILE_SENT), args->filename);
    update_files (1, args->filename, lSize, lTime, &(TxStats.cur_errors));
    TxStats.cur_fxfrd++;
    TxStats.cur_mxfrd += lSize;
    TxStats.FilePos = TxStats.FileLen = -1L;
    css_upd ();
  }

  return (args->result);
}

void
XSSetVars (XMARGSP args)
{
  x_no_sealink = QueryNodeFlag (no_sealink, NOSEA, &remote_addr);

  if (x_no_sealink)
  {
    args->options.SLO = 0;
    args->options.Resync = 0;
  }
  else
  {
    args->options.SLO = ((cur_baud.rate_value >= 9600L) && !no_overdrive) ? 1 : 0;
    args->options.Resync = (~no_resync) & 1;
  }

  args->options.SEAlink = 0;
  args->SendBLK = 1;
  args->curr_byte = 0L;
  args->NextBLK = 1;
  args->ACKST = 0;
  args->ACKBLK = -1L;
  args->Window = 1;
  args->ACKsRcvd = 0;
  args->NumNAK = 0;
  args->T1 = timerset (30 * PER_SECOND);
}

int
XSXmtStart (XMARGSP args)
{
  XSSetVars (args);
  Build_Header_Block (args, SOH);
  return (XS1);
}

int
XSXmTeStrt (XMARGSP args)
{
  XSSetVars (args);
  Build_Header_Block (args, SYN);
  return (XS1);
}

int
XSCheckACK (XMARGSP args)
{
  Check_ACKNAK (args);
  return (XS2);
}

int
XSSendBlk (XMARGSP args)
{
  if (!CARRIER)
  {
    TxStats.cur_errors++;
    return (CARRIER_ERR);
  }

  if (got_ESC ())
  {
    status_line (MSG_TXT (M_KBD_MSG));
    return (KBD_ERR);
  }

  if ((args->NumNAK > 4) && (args->SendBLK == 0))
  {
    if (args->save_header == SOH)
      return (XS0T);
    else
    {
      args->NumNAK = 0;
      ++(args->ACKBLK);
      ++(args->SendBLK);
      return (XS2);
    }
  }

  if (args->NumNAK > 10)
  {
    /* Too Many Errors */
    TxStats.cur_errors++;
    return (SEND_RETRY_ERR);
  }

  if (timeup (args->T1))
  {
    /* Fatal Timeout */
    return (SEND_TIMEOUT);
  }

  if (args->SendBLK > (args->LastBlk + 1))
    return (XS3);

  if (args->SendBLK > (args->ACKBLK + args->Window))
  {
    time_release ();
    return (XS1);
  }

  if (args->SendBLK == (args->LastBlk + 1))
  {
    SENDBYTE (EOT);
    ++(args->SendBLK);
    args->T1 = timerset (30 * PER_SECOND);
    show_sending_blocks (args);
    time_release ();
    return (XS1);
  }

  /* Increment the block count before sending because we read the next
   * block immediately after sending this block.  On error free connects
   * we have a big net win because we never do a seek, and while we are
   * sending one block, we read the next.  If we do get errors, then we
   * have to seek back to the previous block, and that will be a bother.
   * With today's phone lines and modems, we'll assume error free is more
   * often than not, and take our chances. */

  if (args->options.SLO && args->options.SEAlink)
    args->ACKBLK = args->SendBLK;

  ++(args->SendBLK);
  args->curr_byte += 128L;
  Send_Block (args);
  args->T1 = timerset (PER_MINUTE);
  return (XS1);
}

int
XSWaitEnd (XMARGSP args)
{
  show_sending_blocks (args);

  if (args->ACKBLK < (args->LastBlk + 1))
  {
    time_release ();
    return (XS1);
  }

  if (!CARRIER)
  {
    TxStats.cur_errors++;
    return (CARRIER_ERR);
  }

  return (SUCCESS);
}

int
Internal_SEAlink_Send_File (char *filename, char *sendname)
{
  XMARGS xm;
  int res;

  xm.filename = filename;
  xm.temp_name = sendname;
  res = state_machine ((STATEP) Xmodem_Sender, &xm, XS0);
  TxStats.tot_errors += TxStats.cur_errors;
  hist.err_out += TxStats.cur_errors;
  TxStats.cur_errors = 0;
  return res;
}

int
SEAlink_Send_File (char *filename, char *sendname)
{
  int res;

  res = Internal_SEAlink_Send_File (filename, sendname);
  status_line (">SEAlink_Send_File(%s) returns %d",
               filename == NULL ? "(null)" : filename, res);
  return (res == SUCCESS ? FILE_SENT_OK : CANNOT_SEND);
}

int
Xmodem_Send_File (char *filename, char *sendname)
{
  int res;

  res = SEAlink_Send_File (filename, sendname);

  TxStats.tot_errors += TxStats.cur_errors;
  hist.err_out += TxStats.cur_errors;
  TxStats.cur_errors = 0;

  return (res);
}

int
Internal_Telink_Send_File (char *filename, char *sendname)
{
  XMARGS xm;
  int res;

  xm.filename = filename;
  xm.temp_name = sendname;
  res = state_machine ((STATEP) Xmodem_Sender, &xm, XS0T);
  TxStats.tot_errors += TxStats.cur_errors;
  hist.err_out += TxStats.cur_errors;
  TxStats.cur_errors = 0;
  return res;
}

int
Telink_Send_File (char *filename, char *sendname)
{
  int res;

  res = Internal_Telink_Send_File (filename, sendname);
  status_line (">Telink_Send_File(%s) returns %d",
               filename == NULL ? "(null)" : filename, res);
  return (res == SUCCESS ? FILE_SENT_OK : CANNOT_SEND);
}

void
Get_Block (XMARGSP args)
{
  XMDATAP xtmp;

  if (args->SendBLK == 0)
  {
    Build_Header_Block (args, args->save_header);
    args->NextBLK = -1L;
    return;
  }

  xtmp = (XMDATAP) & (args->header);

  /* Set up buffer as all ^Zs for EOF */
  memset (xtmp, SUB, sizeof (XMDATA));

  /* Now set up the header stuff */
  xtmp->header = SOH;
  xtmp->block_num = (unsigned char) (args->SendBLK & 0xff);
  xtmp->block_num_comp = (unsigned char) ~xtmp->block_num;

  if (args->NextBLK != args->SendBLK)
    fseek (args->file_pointer, (args->SendBLK - 1) * 128, SEEK_SET);

  args->NextBLK = args->SendBLK + 1;

  /* Can we read any data? *//* TJW: fread is unsigned -> not <= 0, but == 0 */
  if (fread ((char *) xtmp->data_bytes, 1, 128, args->file_pointer) == 0)
    return;

  /* Looks good */
  return;
}

void
Send_Block (XMARGSP args)
{
  if (args->header == SYN)
    Data_Check ((XMDATAP) & (args->header), CHECKSUM);
  else
    Data_Check ((XMDATAP) & (args->header), args->options.do_CRC ? CRC : CHECKSUM);

  if ((!(args->options.do_CRC)) || (args->header == SYN))
    SENDCHARS ((char *) &(args->header), sizeof (XMDATA) - 1, 1);
  else
    SENDCHARS ((char *) &(args->header), sizeof (XMDATA), 1);

  UNBUFFER_BYTES ();
  show_sending_blocks (args);
  Get_Block (args);
}

void
show_sending_blocks (XMARGSP args)
{
  char *TmpPtr = (char *) &happy_compiler;
  long k;

  k = args->filelen - args->curr_byte;
  if (k < 0L)
    k = 0L;

  if (args->options.SLO)        /* SeaLinkOverdrive */
  {
    if ((!((args->SendBLK - 1) & 0x1f))
        || ((args->SendBLK - 1) > args->LastBlk))
    {
      long eff;

      TxStats.FilePos = args->curr_byte;
      css_upd ();
      eff = (args->save_header == SOH) ? 94L : 70L;  /* efficiency!!! */
    }
  }
  else
  {
    long eff;

    TxStats.FilePos = args->curr_byte;
    css_upd ();
    eff = (args->save_header == SOH) ? 94L : 70L;
  }

  happy_compiler = *(int *) TmpPtr;  /* Makes the compiler happy! */
}

int ACInit (XMARGSP);
int ACEnd (XMARGSP);
int ACChkRcvd (XMARGSP);
int ACSLCheck (XMARGSP);
int ACSLVerify (XMARGSP);
int ACSLACKNAK (XMARGSP);
int ACXMCheck (XMARGSP);
int ACSLOCheck (XMARGSP);
int ACSL1Check (XMARGSP);
int ACACKNAK (XMARGSP);
int ACXMACK (XMARGSP);
int ACXMNAK (XMARGSP);
int ACRESYNC (XMARGSP);

typedef struct
{
  char *state_name;
  int (*state_func) (XMARGSP);
}
ASTATES, *ASTATEP;

ASTATES ACKNAK_Check[] =
{
  {"ACInit", ACInit},
  {"ACEnd", ACEnd},
  {"AC0", ACChkRcvd},
  {"AC1", ACSLCheck},
  {"AC2", ACSLVerify},
  {"AC3", ACSLACKNAK},
  {"AC4", ACXMCheck},
  {"AC5", ACSLOCheck},
  {"AC6", ACSL1Check},
  {"AC7", ACACKNAK},
  {"AC8", ACXMACK},
  {"AC9", ACXMNAK},
  {"AC10", ACRESYNC}
};

int
ACInit (XMARGSP args)
{
  args->result = 0;
  return ((int) args->control);
}

int
ACEnd (XMARGSP args)
{
  args->result = (int) args->control;
  return (args->result);
}

int
ACChkRcvd (XMARGSP args)
{
  if (PEEKBYTE () >= 0)
  {
    args->CHR = TIMED_READ (0);
    return (AC1);
  }

  return (SUCCESS);
}

int
ACSLCheck (XMARGSP args)
{
  if (args->ACKST > 2)
    return (AC2);

  return (AC6);
}

int
ACSLVerify (XMARGSP args)
{
  if (args->ARBLK8 == (unsigned char) ((~args->CHR) & 0xff))
  {
    args->ARBLK = args->SendBLK - ((args->SendBLK - args->ARBLK8) & 0xff);
    return (AC3);
  }

  args->options.SEAlink = 0;
  args->Window = 1;
  args->ACKST = 0;
  return (AC6);
}

int
ACSLACKNAK (XMARGSP args)
{
  if ((args->ARBLK < 0)
      || (args->ARBLK > args->SendBLK)
      || (args->ARBLK <= (args->SendBLK - 128)))
  {
    return (AC0);
  }

  if (args->ACKST == 3)
  {
    args->options.SEAlink = (~x_no_sealink) & 1;
    args->Window = calc_window ();
    args->ACKBLK = args->ARBLK;
    ++(args->ACKsRcvd);
    args->ACKST = 0;
    return (AC5);
  }

  args->SendBLK = args->ARBLK;
  args->curr_byte = (args->SendBLK - 1) * 128L;
  if (args->curr_byte < 0L)
    args->curr_byte = 0L;

  if (args->SendBLK > 0)
  {
    TxStats.cur_errors++;
    ++(args->tot_errs);
  }

  Get_Block (args);
  args->ACKST = 0;

  return (AC4);
}

int
ACXMCheck (XMARGSP args)
{
  if (args->NumNAK < 4)
  {
    args->options.SEAlink = (~x_no_sealink) & 1;
    args->Window = calc_window ();
  }
  else
  {
    args->options.SEAlink = 0;
    args->Window = 1;
  }

  return (SUCCESS);
}

int
ACSLOCheck (XMARGSP args)
{
  if ((args->options.SLO == 0) || (args->ACKsRcvd < 10))
    return (SUCCESS);

  args->options.SLO = 0;
  return (SUCCESS);
}

int
ACSL1Check (XMARGSP args)
{
  if ((args->ACKST == 1) || (args->ACKST == 2))
  {
    args->ARBLK8 = (unsigned char) args->CHR;
    args->ACKST += 2;
    return (AC6);
  }

  if ((args->options.SEAlink == 0) || (args->ACKST == 0))
    return (AC7);

  return (AC0);
}

int
ACACKNAK (XMARGSP args)
{
  long mac_timer;

  switch (args->CHR)
  {
  case ACK:
    args->ACKST = 1;
    args->NumNAK = 0;
    return (AC8);

  case WANTCRC:
    args->options.do_CRC = 1;
    /* Fallthrough */

  case NAK:
    args->ACKST = 2;
    ++(args->NumNAK);
    CLEAR_OUTBOUND ();
    timer (6);
    return (AC9);

  case SYN:
    CLEAR_OUTBOUND ();
    if (!no_resync)
    {
      args->result = Receive_Resync (&(args->resync_block));
      args->ACKST = 0;
      return (AC10);
    }
    else
      return (AC0);

  case DC3:                    /* ^S */
    if (args->options.SEAlink && (args->ACKST == 0))
    {
      mac_timer = timerset (10 * PER_SECOND);
      while (CARRIER && !timeup (mac_timer))
      {
        if (TIMED_READ (0) == DC1)
          break;

        time_release ();
      }

      return (AC0);
    }

    /* Otherwise, fallthrough */

  default:
    break;
  }

  return (AC0);
}

int
ACXMACK (XMARGSP args)
{
  if (!args->options.SEAlink)
    ++(args->ACKBLK);

  return (AC0);
}

int
ACXMNAK (XMARGSP args)
{
  if (!args->options.SEAlink)
  {
    args->SendBLK = args->ACKBLK + 1;
    args->curr_byte = (args->SendBLK - 1) * 128L;
    if (args->curr_byte < 0L)
      args->curr_byte = 0L;

    if (args->SendBLK > 0)
    {
      TxStats.cur_errors++;
      ++(args->tot_errs);
    }

    Get_Block (args);
  }

  return (AC0);
}

int
ACRESYNC (XMARGSP args)
{
  CLEAR_OUTBOUND ();
  if (args->result != SUCCESS)
  {
    SENDBYTE (NAK);
    return (SUCCESS);
  }

  if (args->SendBLK == 1)
  {
    args->prev_bytes = (args->resync_block - 1) * 128;
    if (args->prev_bytes > args->filelen)
      args->prev_bytes = args->filelen;
    status_line (MSG_TXT (M_SYNCHRONIZING), args->prev_bytes);
  }
  else
  {
    ++(args->tot_errs);
    TxStats.cur_errors++;
  }

  args->options.SEAlink = 1;
  args->Window = calc_window ();
  args->SendBLK = args->resync_block;
  args->curr_byte = (args->SendBLK - 1) * 128L;
  if (args->curr_byte < 0L)
    args->curr_byte = 0L;

  Get_Block (args);
  args->ACKBLK = args->SendBLK - 1;
  SENDBYTE (ACK);
  return (SUCCESS);
}

void
Check_ACKNAK (XMARGSP args)
{
  state_machine ((STATEP) ACKNAK_Check, args, AC0);
}

int
Receive_Resync (long *resync_block)
{
  unsigned char resyncit[30];
  unsigned char *p;
  unsigned char a, b;
  unsigned short nak_crc, his_crc;

  p = resyncit;

  while ((*p = (unsigned char) TIMED_READ (1)) != ETX)
  {
    if ((*p < '0') || (*p > '9'))
    {
      status_line (">SEAlink Send: Resync bad byte '%02x'", *p);
      TxStats.cur_errors++;
      return (RESYNC_ERR);
    }
    ++p;
  }

  *p = '\0';
  nak_crc = crc_block ((unsigned char *) resyncit, (int) strlen ((char *) resyncit));
  a = (unsigned char) TIMED_READ (1);
  b = (unsigned char) TIMED_READ (1);
  his_crc = (b << 8) | a;

  if (nak_crc != his_crc)
  {
    status_line (">SEAlink Send: Resync bad crc %04hx/%04hx", nak_crc, his_crc);
    TxStats.cur_errors++;
    return (CRC_ERR);
  }

  *resync_block = atol ((char *) resyncit);

  status_line (">SEAlink Send: Resync to %ld", *resync_block);
  return (SUCCESS);
}

int
calc_window ()
{
  long window;

  window = cur_baud.rate_value / 400L;
  if (window <= 0L)
    window = 2L;
  if (small_window)
    window = (window > 6L) ? 6L : window;
  else
    window = (window > 2000L) ? 2000L : window;

  return (int) window;
}

/* $Id: xmsend.c,v 1.5 1999/03/20 04:08:29 mr Exp $ */
