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
 * Filename    : $Source: E:/cvs/btxe/src/zmodem.c,v $
 * Revision    : $Revision: 1.8 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:58 $
 * State       : $State: Exp $
 * Orig. Author: Wynn Wagner III
 *
 * Description : Zmodem protocol module.
 *
 * Note        :
 *   This module is based largely on a similar module in OPUS-CBCS V1.03b.
 *   The original work is (C) Copyright 1986, Wynn Wagner III. The original
 *   authors have graciously allowed us to use their code in this work.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/* ------------------------------------------------------------------------ */
/* Local routines                                                           */
/* ------------------------------------------------------------------------ */

// zreceive
static int LOCALFUNC RZ_ReceiveData (byte *, int);
static int LOCALFUNC RZ_32ReceiveData (byte *, int);
static int LOCALFUNC RZ_InitReceiver (void);
static int LOCALFUNC RZ_ReceiveBatch (FILE *);
static int LOCALFUNC RZ_ReceiveFile (FILE *);
static int LOCALFUNC RZ_GetHeader (void);
static int LOCALFUNC RZ_SaveToDisk (long *);
static void LOCALFUNC RZ_AckBibi (void);

// zsend
static void LOCALFUNC ZS_SendBinaryHeader (unsigned short, byte *);
static void LOCALFUNC ZS_SendData (byte *, int, unsigned short);
static void LOCALFUNC ZS_SendByte (byte);
static int LOCALFUNC ZS_GetReceiverInfo (void);
static int LOCALFUNC ZS_SendFile (int, int);
static int LOCALFUNC ZS_SendFileData (int);
static int LOCALFUNC ZS_SyncWithReceiver (int);
static void LOCALFUNC ZS_EndSend (void);

// zmisc
static int LOCALFUNC _Z_GetBinaryHeader (unsigned char *);
static int LOCALFUNC _Z_32GetBinaryHeader (unsigned char *);
static int LOCALFUNC _Z_GetHexHeader (unsigned char *);
static int LOCALFUNC _Z_GetHex (void);
static int LOCALFUNC _Z_TimedRead (void);
static long LOCALFUNC _Z_PullLongFromHeader (unsigned char *);
static int LOCALFUNC Z_GetByte (int);
static void LOCALFUNC Z_PutString (unsigned char *);
static void LOCALFUNC Z_SendHexHeader (unsigned int, unsigned char *);
static int LOCALFUNC Z_GetHeader (unsigned char *);
static int LOCALFUNC Z_GetZDL (void);
static void LOCALFUNC Z_PutLongIntoHeader (long);
static void LOCALFUNC Z_UncorkTransmitter (void);

/* ------------------------------------------------------------------------ */
/* Private declarations                                                     */
/* ------------------------------------------------------------------------ */

// zreceive
static long DiskAvail;
static long filetime;
static char realname[64];

// zsend
static FILE *Infile;            /* Handle of file being sent */
static long Strtpos;            /* Starting byte position of download */
static long LastZRpos;          /* Last error location       */
static long ZRPosCount;         /* ZRPOS repeat count        */
static long Txpos;              /* Transmitted file position */
static int Rxbuflen;            /* Receiver's max buffer length */
static int Rxflags;             /* Receiver's flags          */

// zmisc
static int Rxtype;              /* Type of header received   */
static char hex[] = "0123456789abcdef";

/* ------------------------------------------------------------------------ */
/* Private data                                                             */
/* ------------------------------------------------------------------------ */

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32

static char Attn[ZATTNLEN + 1]; /* String rx sends to tx on err            */
static FILE *Outfile;           /* Handle of file being received           */
static int Tryzhdrtype;         /* Hdr type to send for Last rx close      */
static char isBinary;           /* Current file is binary mode             */
static char EOFseen;            /* indicates cpm eof (^Z) was received     */
static char Zconv;              /* ZMODEM file conversion request          */
static int RxCount;             /* Count of data bytes received            */
static char Upload_path[PATHLEN];  /* Dest. path of file being received    */
static long Filestart;          /* File offset we started this xfer from   */
static int FileSkipInProgress = 0;

/* ------------------------------------------------------------------------ */
/* GET ZMODEM                                                               */
/* Receive a batch of files.                                                */
/* returns TRUE (1) for good xfer, FALSE (0) for bad                        */
/* can be called from f_upload or to get mail from a WaZOO Opus             */
/* ------------------------------------------------------------------------ */

int
get_Zmodem (char *rcvpath, FILE * xferinfo)
{
  char namebuf[PATHLEN];
  int i;
  char *p;
  char *HoldName;
  long t;

#ifdef DEBUG
  show_debug_name ("get_Zmodem");
#endif

  filetime = 0;

  if (direct_zap)
    XON_DISABLE ();
  else
    IN_XON_ENABLE ();

  Outfile = NULL;
  z_size = 0;
  Rxtimeout = 250;              /* 100; */
  Tryzhdrtype = ZRINIT;

  strcpy (namebuf, rcvpath);
  Filename = namebuf;

  strcpy (Upload_path, rcvpath);
  p = Upload_path + strlen (Upload_path) - 1;
  while (p >= Upload_path && *p != DIR_SEPC)
    --p;
  *(++p) = '\0';

  HoldName = HoldAreaNameMunge (&called_addr);

  sprintf (Abortlog_name, "%s%s.z",
           HoldName, Hex_Addr_Str (&remote_addr));

  DiskAvail = zfree (Upload_path);

  i = RZ_InitReceiver ();
  switch (i)
  {
  case ZFILE:
    i = RZ_ReceiveBatch (xferinfo);
    if (i != OK)
      break;
  case ZCOMPL:
    if (direct_zap)             /* HJK - 98/08/07 - CARRIER problems */
      XON_DISABLE ();           /* Make sure xmitter is unstuck */

    return FILE_RECV_OK;
  }

  CLEAR_OUTBOUND ();
  XON_DISABLE ();               /* Make sure xmitter is unstuck */
  send_can ();                  /* transmit at least 10 cans    */
  t = timerset (200);           /* wait no more than 2 seconds  */

  while (!timeup (t) && !OUT_EMPTY () && CARRIER)
    time_release ();            /* Give up slice while waiting  */

  if (!direct_zap)
    XON_ENABLE ();              /* Turn XON/XOFF back on...     */

  if (Outfile)
    fclose (Outfile);

  return NOTHING_RECVED;
}                               /* get_Zmodem */

/* ------------------------------------------------------------------------ */
/* RZ RECEIVE DATA                                                          */
/* Receive array buf of max length with ending ZDLE sequence                */
/* and CRC.  Returns the ending character or error code.                    */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
RZ_ReceiveData (register byte * buf, register int length)
{
  register int c;
  register word crc;
  char *endpos;
  int d;

#ifdef DEBUG
  show_debug_name ("RZ_ReceiveData");
#endif

  if (Rxframeind == ZBIN32)
    return RZ_32ReceiveData (buf, length);

  crc = RxCount = 0;
  buf[0] = buf[1] = 0;
  endpos = (char *) buf + length;

  while ((char *) buf <= endpos)
  {
    if ((c = Z_GetZDL ()) & ~0xFF)
    {
    CRCfoo:
      switch (c)
      {
      case GOTCRCE:
      case GOTCRCG:
      case GOTCRCQ:
      case GOTCRCW:            /* C R C s                           */
        crc = Z_UpdateCRC (((d = c) & 0xFF), crc);
        if ((c = Z_GetZDL ()) & ~0xFF)
          goto CRCfoo;

        crc = Z_UpdateCRC (c, crc);
        if ((c = Z_GetZDL ()) & ~0xFF)
          goto CRCfoo;

        crc = Z_UpdateCRC (c, crc);
        if (crc & 0xFFFF)
        {
          status_line ("!%s", MSG_TXT (M_CRC_MSG));
          RxStats.cur_errors++;
          return ERROR;
        }

        RxCount = length - (int) (endpos - (char *) buf);
        return d;

      case GOTCAN:             /* Cancel                            */
        status_line (MSG_TXT (M_CAN_MSG));
        RxStats.cur_errors++;
        return ZCAN;

      case TIMEOUT:            /* Timeout                           */
        status_line ("!%s", MSG_TXT (M_TIMEOUT));
        RxStats.cur_errors++;
        return c;

      case RCDO:               /* No carrier                        */
        // status_line (MSG_TXT (M_NO_CARRIER));
        CLEAR_INBOUND ();
        RxStats.cur_errors++;
        return c;

      default:                 /* Something bizarre                 */
        status_line ("!%s", MSG_TXT (M_DEBRIS));
        CLEAR_INBOUND ();
        RxStats.cur_errors++;
        return c;
      }
    }

    *buf++ = (unsigned char) c;
    crc = Z_UpdateCRC (c, crc);
  }

  status_line ("!%s", MSG_TXT (M_LONG_PACKET));
  RxStats.cur_errors++;
  return ERROR;
}                               /* RZ_ReceiveData */

/* ------------------------------------------------------------------------ */
/* RZ RECEIVE DATA with 32 bit CRC                                          */
/* Receive array buf of max length with ending ZDLE sequence                */
/* and CRC.  Returns the ending character or error code.                    */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
RZ_32ReceiveData (register byte * buf, register int length)
{
  register int c;
  unsigned long crc;
  char *endpos;
  int d;

#ifdef DEBUG
  show_debug_name ("RZ_32ReceiveData");
#endif

  crc = 0xFFFFFFFFUL;
  RxCount = 0;
  buf[0] = buf[1] = 0;
  endpos = (char *) buf + length;

  while ((char *) buf <= endpos)
  {
    if ((c = Z_GetZDL ()) & ~0xFF)
    {
    CRCfoo:
      switch (c)
      {
      case GOTCRCE:
      case GOTCRCG:
      case GOTCRCQ:
      case GOTCRCW:            /* C R C s                           */
        d = c;
        c &= 0377;
        crc = Z_32UpdateCRC (c, crc);
        if ((c = Z_GetZDL ()) & ~0xFF)
          goto CRCfoo;

        crc = Z_32UpdateCRC (c, crc);
        if ((c = Z_GetZDL ()) & ~0xFF)
          goto CRCfoo;

        crc = Z_32UpdateCRC (c, crc);
        if ((c = Z_GetZDL ()) & ~0xFF)
          goto CRCfoo;

        crc = Z_32UpdateCRC (c, crc);
        if ((c = Z_GetZDL ()) & ~0xFF)
          goto CRCfoo;

        crc = Z_32UpdateCRC (c, crc);
        if (crc != 0xDEBB20E3UL)
        {
          status_line ("!%s", MSG_TXT (M_CRC_MSG));
          return ERROR;
        }

        RxCount = length - (int) (endpos - (char *) buf);
        return d;

      case GOTCAN:             /* Cancel                            */
        status_line (MSG_TXT (M_CAN_MSG));
        RxStats.cur_errors++;
        return ZCAN;

      case TIMEOUT:            /* Timeout                           */
        status_line ("!%s", MSG_TXT (M_TIMEOUT));
        RxStats.cur_errors++;
        return c;

      case RCDO:               /* No carrier                        */
        // status_line (MSG_TXT (M_NO_CARRIER));
        CLEAR_INBOUND ();
        RxStats.cur_errors++;
        return c;

      default:                 /* Something bizarre                 */
        status_line ("!%s", MSG_TXT (M_DEBRIS));
        CLEAR_INBOUND ();
        RxStats.cur_errors++;
        return c;
      }
    }

    *buf++ = (unsigned char) c;
    crc = Z_32UpdateCRC (c, crc);
  }

  status_line ("!%s", MSG_TXT (M_LONG_PACKET));
  RxStats.cur_errors++;
  return ERROR;
}                               /* RZ_ReceiveData */

/* ------------------------------------------------------------------------ */
/* RZ INIT RECEIVER                                                         */
/* Initialize for Zmodem receive attempt, try to activate Zmodem sender     */
/* Handles ZSINIT, ZFREECNT, and ZCOMMAND frames                            */
/*                                                                          */
/* Return codes:                                                            */
/*    ZFILE .... Zmodem filename received                                   */
/*    ZCOMPL ... transaction finished                                       */
/*    ERROR .... any other condition                                        */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
RZ_InitReceiver ()
{
  register int n;
  int errors = 0;
  char *sptr;

#ifdef DEBUG
  show_debug_name ("RZ_InitReceiver");
#endif

  for (n = 12; --n >= 0;)
  {
    /* ------------------------------------------------------------ */
    /* Set buffer length (0=unlimited, don't wait).                 */
    /* Also set capability flags                                    */
    /* ------------------------------------------------------------ */

    Z_PutLongIntoHeader (0L);
    Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO;
    Z_SendHexHeader (Tryzhdrtype, (byte *) Txhdr);
    if (Tryzhdrtype == ZSKIP)
      Tryzhdrtype = ZRINIT;

  AGAIN:

    switch (Z_GetHeader ((byte *) Rxhdr))
    {
    case ZFILE:

      Zconv = Rxhdr[ZF0];
      Tryzhdrtype = ZRINIT;
      if (RZ_ReceiveData (Txbuf, WAZOOMAX) == GOTCRCW)
        return ZFILE;
      Z_SendHexHeader (ZNAK, (byte *) Txhdr);
      if (--n < 0)
      {
        sptr = "ZFILE";
        goto Err;
      }
      goto AGAIN;

    case ZSINIT:
      if (RZ_ReceiveData ((byte *) Attn, ZATTNLEN) == GOTCRCW)
      {
        Z_PutLongIntoHeader (1L);
        Z_SendHexHeader (ZACK, (byte *) Txhdr);
      }
      else
        Z_SendHexHeader (ZNAK, (byte *) Txhdr);

      if (--n < 0)
      {
        sptr = "ZSINIT";
        goto Err;
      }
      goto AGAIN;

    case ZFREECNT:
      Z_PutLongIntoHeader (DiskAvail >= 0L && DiskAvail <= 0x01ffffffL ?
                           DiskAvail * 64L : 0x7fffffffL);
      Z_SendHexHeader (ZACK, (byte *) Txhdr);
      goto AGAIN;

    case ZCOMMAND:
      /* --------------------------------------- */
      /* Paranoia is good for you...             */
      /* Ignore command from remote, but lie and */
      /* say we did the command ok.              */
      /* --------------------------------------- */
      if (RZ_ReceiveData (Txbuf, WAZOOMAX) == GOTCRCW)
      {
        status_line (MSG_TXT (M_Z_IGNORING), Txbuf);
        Z_PutLongIntoHeader (0L);

        do
          Z_SendHexHeader (ZCOMPL, (byte *) Txhdr);
        while (++errors < 10 && Z_GetHeader ((byte *) Rxhdr) != ZFIN);

        RZ_AckBibi ();
        return ZCOMPL;
      }
      else
        Z_SendHexHeader (ZNAK, (byte *) Txhdr);

      if (--n < 0)
      {
        sptr = "CMD";
        goto Err;
      }
      goto AGAIN;

    case ZCOMPL:
      if (--n < 0)
      {
        sptr = "COMPL";
        goto Err;
      }
      goto AGAIN;

    case ZFIN:
      RZ_AckBibi ();
      return ZCOMPL;

    case ZCAN:
      sptr = MSG_TXT (M_CAN_MSG);
      goto Err;

    case RCDO:
      sptr = &(MSG_TXT (M_NO_CARRIER)[1]);
      CLEAR_INBOUND ();
      goto Err;
    }                           /* switch */
  }                             /* for */

  sptr = MSG_TXT (M_TIMEOUT);

Err:
  sprintf (e_input, MSG_TXT (M_Z_INITRECV), sptr);
  status_line (e_input);
  RxStats.cur_errors++;

  return ERROR;
}                               /* RZ_InitReceiver */

/* ------------------------------------------------------------------------ */
/* RZ_ReceiveBatch                                                          */
/* Receive a batch of files using ZMODEM protocol                           */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
RZ_ReceiveBatch (FILE * xferinfo)
{
  register int c;
  char namebuf[PATHLEN];
  long sanity_timeout;

#ifdef DEBUG
  show_debug_name ("RZ_ReceiveBatch");
#endif

  sanity_timeout = timerset (2 * PER_MINUTE);  /* two minutes to get going */

  while (!timeup (sanity_timeout))
  {
    c = RZ_ReceiveFile (xferinfo);
    status_line (">Returned from RZ_ReceiveFile");
    switch (c)
    {
    case ZEOF:
      if (Resume_WaZOO)
      {
        remove_abort (Abortlog_name, Resume_name);
        strcpy (namebuf, Upload_path);
        strcat (namebuf, Resume_name);
        unique_name (namebuf);
        rename (Filename, namebuf);
      }
      sanity_timeout = timerset (2 * PER_MINUTE);  /* reset timeout */
      /* fallthrough */

    case ZFERR:
    case ZSKIP:
      switch (RZ_InitReceiver ())
      {
      case ZCOMPL:
        return OK;

      case ZFILE:
        break;

      default:
        RxStats.cur_errors++;
        return ERROR;
      }
      break;

    default:
      fclose (Outfile);
      Outfile = NULL;
      if (remote_capabilities && !FileSkipInProgress)
      {
        if (!Resume_WaZOO)
          add_abort (Abortlog_name, Resume_name, Filename, Upload_path, Resume_info);
      }
      else if (FileSkipInProgress != 1)
        unlink (Filename);
      FileSkipInProgress = 0;
      return c;
    }
  }

  RxStats.cur_errors++;
  return ERROR;
}                               /* RZ_ReceiveBatch */


void LOCALFUNC
doerrorcountersfornextfile (int receiver)  /* CEH 980803 */
{
  if (receiver)
  {
    hist.err_in += RxStats.cur_errors;
    RxStats.tot_errors += RxStats.cur_errors;
    RxStats.cur_errors = 0;
  }
  else
  {
    hist.err_out += TxStats.cur_errors;
    TxStats.tot_errors += TxStats.cur_errors;
    TxStats.cur_errors = 0;
  }
}


/* ------------------------------------------------------------------------ */
/* RZ RECEIVE FILE                                                          */
/* Receive one file; assumes file name frame is preloaded in Txbuf          */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
RZ_ReceiveFile (FILE * xferinfo)
{
  register int c;
  int n;
  long rxbytes;
  char *sptr;
  struct utimbuf utimes;
  long lTime, lSize;

#ifdef DEBUG
  show_debug_name ("RZ_ReceiveFile");
#endif

  EOFseen = FALSE;
  c = RZ_GetHeader ();
  if (c == ERROR || c == ZSKIP)
  {
    doerrorcountersfornextfile (1);
    return (Tryzhdrtype = ZSKIP);
  }
  else if (c == ZFERR)
  {
    doerrorcountersfornextfile (1);
    return (Tryzhdrtype = ZFERR);
  }

  n = 10;
  rxbytes = Filestart;

  for (;;)
  {
    Z_PutLongIntoHeader (rxbytes);
    Z_SendHexHeader (ZRPOS, (byte *) Txhdr);

  NxtHdr:
    if (ReqSkipFile)
    {
      status_line (":Got request to skip file, sending ZSkip");
      Z_SendHexHeader (ZSKIP, (byte *) Txhdr);
      FileSkipInProgress = ReqSkipFile;
      ReqSkipFile = 0;
    }
    switch (c = Z_GetHeader ((byte *) Rxhdr))
    {
    case ZDATA:                /* Data Packet                             */
      if (Rxpos != rxbytes)
      {
        if (!FileSkipInProgress)
        {
          if (--n < 0)
          {
            sptr = MSG_TXT (M_FUBAR_MSG);
            goto Err;
          }
          RxStats.cur_errors++;
          status_line ("!%s; %ld/%ld (SK: %d)", MSG_TXT (M_BAD_POS), rxbytes, Rxpos, FileSkipInProgress);
          Z_PutString ((byte *) Attn);
          continue;
        }
        else                    /* That stupid remote ignored my ZSKIP - send him another one! */
          Z_SendHexHeader (ZSKIP, (byte *) Txhdr);
      }

    MoreData:
      if (ReqSkipFile)
      {
        status_line (":Got request to skip file, sending ZSkip");
        Z_SendHexHeader (ZSKIP, (byte *) Txhdr);
        FileSkipInProgress = ReqSkipFile;
        ReqSkipFile = 0;
      }

      switch (c = RZ_ReceiveData (Txbuf, WAZOOMAX))
      {
      case ZCAN:               /* Cancel */
        sptr = MSG_TXT (M_CAN_MSG);
        goto Err;

      case RCDO:
        sptr = &(MSG_TXT (M_NO_CARRIER)[1]);
        CLEAR_INBOUND ();
        goto Err;

      case ERROR:              /* CRC error             */
        if (FileSkipInProgress) /* Who minds in data was corrupt... */
          continue;

        if (--n < 0)
        {
          sptr = MSG_TXT (M_FUBAR_MSG);
          goto Err;
        }

        status_line (MSG_TXT (M_OFFSET_RETRIES), rxbytes, n);
        Z_PutString ((byte *) Attn);
        RxStats.cur_errors++;
        continue;

      case TIMEOUT:
        if (FileSkipInProgress) /* Who minds in data was corrupt... */
          continue;

        if (--n < 0)
        {
          sptr = MSG_TXT (M_TIMEOUT);
          goto Err;
        }

        RxStats.cur_errors++;
        status_line (MSG_TXT (M_OFFSET_RETRIES), rxbytes, n);
        continue;

      case GOTCRCW:            /* End of frame          */
        n = 10;
        if (RZ_SaveToDisk (&rxbytes) == ERROR)
        {
          doerrorcountersfornextfile (1);
          return ERROR;
        }
        Z_PutLongIntoHeader (rxbytes);
        Z_SendHexHeader (ZACK, (byte *) Txhdr);
        goto NxtHdr;

      case GOTCRCQ:            /* Zack expected         */
        n = 10;
        if (RZ_SaveToDisk (&rxbytes) == ERROR)
        {
          doerrorcountersfornextfile (1);
          return ERROR;
        }
        Z_PutLongIntoHeader (rxbytes);
        Z_SendHexHeader (ZACK, (byte *) Txhdr);
        goto MoreData;

      case GOTCRCG:            /* Non-stop              */
        n = 10;
        if (RZ_SaveToDisk (&rxbytes) == ERROR)
        {
          doerrorcountersfornextfile (1);
          return ERROR;
        }
        goto MoreData;

      case GOTCRCE:            /* Header to follow      */
        n = 10;
        if (RZ_SaveToDisk (&rxbytes) == ERROR)
        {
          doerrorcountersfornextfile (1);
          return ERROR;
        }
        goto NxtHdr;
      }
      /* Default falls through */

    case ZNAK:
    case TIMEOUT:              /* Packet was probably garbled */
      if (--n < 0)
      {
        sptr = MSG_TXT (M_JUNK_BLOCK);
        goto Err;
      }

      RxStats.cur_errors++;
      status_line (MSG_TXT (M_OFFSET_RETRIES), rxbytes, n);
      continue;

    case ZFILE:                /* Sender didn't see our ZRPOS yet */
      RZ_ReceiveData (Txbuf, WAZOOMAX);
      continue;

    case ZEOF:                 /* End of the file */
      /* Ignore EOF if it's at wrong place; force a timeout because the eof
       * might have gone out before we sent our ZRPOS */
      status_line (">Got ZEOF");
      if (Rxpos != rxbytes && !FileSkipInProgress)
        goto NxtHdr;

      lSize = rxbytes - Filestart;
      lTime = throughput (1, 0, lSize) / PER_SECOND;

      fclose (Outfile);

      RxStats.cur_mxfrd += rxbytes;
      RxStats.cur_fxfrd++;
      RxStats.FilePos = RxStats.FileLen = -1L;
      css_upd ();

      status_line ("%s-Z%s %s", MSG_TXT (M_FILE_RECEIVED), Crc32 ? "/32" : "", realname);
      update_files (0, realname, lSize, lTime, &(RxStats.cur_errors));

      if (filetime > 0)         /* utime doesn't like negative numbers */
      {
        utimes.UT_ACTIME = utimes.modtime = filetime;
        unix_utime_noshift (Filename, (UTIMBUF *) & utimes);
      }

      Outfile = NULL;
      if (xferinfo != NULL)
        fprintf (xferinfo, "%s\n", Filename);

      doerrorcountersfornextfile (1);
      return c;

    case ERROR:                /* Too much garbage in header search error */
      if (FileSkipInProgress)
        continue;

      if (--n < 0)
      {
        sptr = MSG_TXT (M_JUNK_BLOCK);
        goto Err;
      }

      RxStats.cur_errors++;
      status_line (MSG_TXT (M_OFFSET_RETRIES_SK), rxbytes, n, FileSkipInProgress);
      Z_PutString ((byte *) Attn);
      continue;

    case ZSKIP:
      status_line ("!Got ZSKIP");
      doerrorcountersfornextfile (1);
      return c;

    default:
      sptr = MSG_TXT (M_I_DONT_KNOW);
      CLEAR_INBOUND ();
      goto Err;
    }
  }

Err:
  sprintf (e_input, MSG_TXT (M_Z_RZ), sptr);
  status_line (e_input);
  RxStats.cur_errors++;
  doerrorcountersfornextfile (1);
  return ERROR;
}                               /* RZ_ReceiveFile */

/* ------------------------------------------------------------------------ */
/* RZ GET HEADER                                                            */
/* Process incoming file information header                                 */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
RZ_GetHeader ()
{
  register byte *p;
  struct stat f;
  int i;
  char *ourname;
  char *theirname;
  long filesize;
  char *fileinfo;

#ifdef DEBUG
  show_debug_name ("RZ_GetHeader");
#endif

  /* ------------------------------------------------------------------ */
  /* Setup the transfer mode                                            */
  /* ------------------------------------------------------------------ */
  i = RXBINARY;                 /* RXBINARY is #defined as FALSE */

  isBinary = (char) ((!i && (Zconv == ZCNL) ? 0 : 1));
  Resume_WaZOO = 0;

  /* ------------------------------------------------------------------ */
  /* Extract and verify filesize, if given.                             */
  /* Reject file if not at least 10K free                               */
  /* ------------------------------------------------------------------ */
  filesize = 0L;
  filetime = 0L;
  fileinfo = (char *) Txbuf + 1 + strlen ((char *) Txbuf);
  if (*fileinfo)
    sscanf (fileinfo, "%ld %lo", &filesize, &filetime);

  if (filesize / 64 + 160 > DiskAvail)
  {
    status_line (MSG_TXT (M_OUT_OF_DISK_SPACE));
    /* return ERROR; */
    RxStats.cur_errors++;
    return ZFERR;
  }

  /* ------------------------------------------------------------------ */
  /* Get and/or fix filename for uploaded file                          */
  /* ------------------------------------------------------------------ */
  p = (byte *) (Filename + strlen (Filename) - 1);  /* Find end of upload path */
  while ((char *) p >= Filename && *p != DIR_SEPC)
    p--;
  ourname = (char *) ++p;

  p = Txbuf + strlen ((char *) Txbuf) - 1;  /* Find transmitted simple
                                               * filename */
  while (p >= Txbuf && *p != '\\' && *p != '/' && *p != ':')
    p--;
  theirname = (char *) ++p;

  /* ------------------------------------------------------------------ */
  /* Rename .REQ file if this is a netmail session                      */
  /* ------------------------------------------------------------------ */
  if (remote_capabilities)
  {
    i = strlen (theirname) - 4;
    if ((i > 0) && (stricmp (&theirname[i], ".req") == 0))
    {
      theirname[i] = '\0';
      status_line (MSG_TXT (M_REC_REQ_AS), theirname, theirname, TaskNumber);
      sprintf (&theirname[i], ".r%02x", TaskNumber);
    }
  }

  strcpy (ourname, theirname);  /* Start w/ our path & their name */
  strcpy (realname, Filename);

  /* ------------------------------------------------------------------ */
  /* Save info on WaZOO transfer in case of abort                       */
  /* ------------------------------------------------------------------ */
  if (remote_capabilities)
  {
    strcpy (Resume_name, theirname);
    sprintf (Resume_info, "%ld %lo", filesize, filetime);
  }

  /* ------------------------------------------------------------------ */
  /* Check if this is a failed WaZOO transfer which should be resumed   */
  /* ------------------------------------------------------------------ */
  if (remote_capabilities && dexists (Abortlog_name))
    Resume_WaZOO = (byte) check_failed (Abortlog_name, theirname, Resume_info, ourname);

  /* ------------------------------------------------------------------- */
  /* Reset file-skipping variables, we don't want to skip the wrong file */
  /* ------------------------------------------------------------------- */
  ReqSkipFile = 0;
  FileSkipInProgress = 0;

  /* ------------------------------------------------------------------ */
  /* Open either the old or a new file, as appropriate                  */
  /* ------------------------------------------------------------------ */
  if (Resume_WaZOO)
  {
    if (dexists (Filename))
      p = (byte *) read_binary_plus;
    else
      p = (byte *) write_binary;
  }
  else
  {
    strcpy (ourname, theirname);

    /* --------------------------------------------------------------- */
    /* If the file already exists:                                     */
    /* 1) And the new file has the same time and size, return ZSKIP    */
    /* 2) And OVERWRITE is turned on, delete the old copy              */
    /* 3) Else create a unique file name in which to store new data    */
    /* --------------------------------------------------------------- */
    if (unix_stat_noshift (Filename, &f) != -1)
    {                           /* If file already exists...      */
      if (filesize == f.st_size && (time_t) filetime == f.st_mtime)
      {
        status_line (MSG_TXT (M_ALREADY_HAVE), Filename);
        return ZSKIP;
      }

      i = strlen (Filename) - 1;
      if ((!overwrite) || (is_arcmail (Filename, i)))
      {
        unique_name (Filename);
        status_line (MSG_TXT (M_RENAME_MSG), Filename);
      }
      else
        unlink (Filename);
    }

    p = (byte *) write_binary;
  }

  Outfile = fopen (Filename, (char *) p);
  if (Outfile == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), Filename);
    return ERROR;
  }

  if (isatty (fileno (Outfile)))
  {
    errno = 1;
    got_error (MSG_TXT (M_DEVICE_MSG), Filename);
    fclose (Outfile);
    RxStats.cur_errors++;
    return (ERROR);
  }

  Filestart = (Resume_WaZOO) ? filelength (fileno (Outfile)) : 0L;
  if (Resume_WaZOO)
    status_line (MSG_TXT (M_SYNCHRONIZING), Filestart);
  fseek (Outfile, Filestart, SEEK_SET);

  if (remote_capabilities)
    p = (byte *) check_netfile (theirname);
  else
    p = NULL;

  file_length = filesize;

  strcpy (RxStats.fname, fname8p3 (realname));
  RxStats.FilePos = Filestart;
  RxStats.FileLen = filesize;
  css_upd ();
  throughput (0, 0, 0L);

  return OK;
}                               /* RZ_GetHeader */

/* ------------------------------------------------------------------------ */
/* RZ SAVE TO DISK                                                          */
/* Writes the received file data to the output file.                        */
/* If in ASCII mode, stops writing at first ^Z, and converts all            */
/*   solo CR's or LF's to CR/LF pairs.                                      */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
RZ_SaveToDisk (long *rxbytes)
{
  static byte lastsent;
  register byte *p;
  register unsigned int count;
  int i;

#ifdef DEBUG
  show_debug_name ("RZ_SaveToDisk");
#endif

  show_alive ();
  count = RxCount;

  if (got_ESC ())
  {
    send_can ();                /* Cancel file */
    while ((i = Z_GetByte (20)) != TIMEOUT && i != RCDO)  /* Wait for line to clear */
      CLEAR_INBOUND ();
    send_can ();                /* and Cancel Batch */
    status_line (MSG_TXT (M_KBD_MSG));
    RxStats.cur_errors++;
    return ERROR;
  }

  if (count != z_size)
    z_size = count;

  if (isBinary)
  {
    if (fwrite (Txbuf, 1, count, Outfile) != count)
      goto oops;
  }
  else
  {
    if (EOFseen)
      return OK;

    for (p = Txbuf; count > 0; count--)
    {
      if (*p == CPMEOF)
      {
        EOFseen = TRUE;
        return OK;
      }

      if (*p == '\n')
      {
        if (lastsent != '\r' && putc ('\r', Outfile) == EOF)
          goto oops;
      }
      else
      {
        if (lastsent == '\r' && putc ('\n', Outfile) == EOF)
          goto oops;
      }

      if (putc ((lastsent = *p++), Outfile) == EOF)
        goto oops;
    }
  }

  *rxbytes += RxCount;

  RxStats.FilePos = *rxbytes;
  RxStats.FileLen = file_length;
  css_upd ();

  return OK;

oops:
  got_error (MSG_TXT (M_WRITE_MSG), Filename);
  RxStats.cur_errors++;
  return ERROR;

}                               /* RZ_SaveToDisk */

/* ------------------------------------------------------------------------ */
/* RZ ACK BIBI                                                              */
/* Ack a ZFIN packet, let byegones be byegones                              */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
RZ_AckBibi ()
{
  register int n;

#ifdef DEBUG
  show_debug_name ("RZ_AckBiBi");
#endif

  CLEAR_OUTBOUND ();
  CLEAR_INBOUND ();

  Z_PutLongIntoHeader (0L);
  for (n = 4; --n;)
  {
    Z_SendHexHeader (ZFIN, (byte *) Txhdr);
    switch (Z_GetByte (100))
    {
    case 'O':
      Z_GetByte (1);            /* Discard 2nd 'O' */
      /* Fall-through */

    case TIMEOUT:
    case RCDO:
      return;
    }
  }
}                               /* RZ_AckBibi */


// zsend.c =================================================================

/* ------------------------------------------------------------------------ */
/* SEND ZMODEM (send a file)                                                */
/*   returns FILE_SENT_OK (1) for good xfer, NOTHING_SENT (0) for bad       */
/*   sends one file per call; 'batch' flags start and end of batch          */
/* ------------------------------------------------------------------------ */
/* r. hoerner: according to zmodem.h "OK" equals to "FALSE" (0).            *
 *             i therefore removed all return(OK) by return(FALSE) to avoid *
 *             misunderstandings                                            *
 *             renamed "fsent" to "batch" for the very same reason          *
 *             changed ALL protocols to return resultcodes of the same TYPE *
 *             and NAME. This makes the upper layer much easier to          *
 *             understand what happend.                                     *
 * ------------------------------------------------------------------------ */

int
Send_Zmodem (char *fname, char *alias, int batch, int wazoo)
{
  register byte *p;
  register byte *q;
  struct stat f;
  time_t filetime;
  int rc = FILE_SENT_OK;

#ifdef DEBUG
  show_debug_name ("send_Zmodem");
#endif

  if (direct_zap)
    XON_DISABLE ();
  else
    IN_XON_ENABLE ();

  z_size = 0;
  Infile = NULL;

  switch (batch)
  {
  case ZMODEM_INIT:
    // r.hoerner (debug)
    // status_line("!send_Zmodem: ZMODEM_INIT (%s)",fname?fname:"(null)");
    Z_PutString ((byte *) "rz\r");
    Z_PutLongIntoHeader (0L);
    Z_SendHexHeader (ZRQINIT, (byte *) Txhdr);
    /* Fall through */

  case ZMODEM_END:
    Rxtimeout = 200;
    if (ZS_GetReceiverInfo () == ERROR)
    {
      XON_DISABLE ();
      CLEAR_OUTBOUND ();
      CLEAR_INBOUND ();
      if (!direct_zap)
        XON_ENABLE ();
      return CANNOT_SEND;       /* MR 970513 was: NOTHING_SENT */
    }
  }

  Rxtimeout = (int) (614400L / (long) cur_baud.rate_value);

  if (Rxtimeout < 400)
    Rxtimeout = 400;

  if (fname == NULL)
    goto Done;

  /* ------------------------------------------------------------------ */
  /* Prepare the file for transmission.  Just ignore file open errors   */
  /* because there may be other files that can be sent.                 */
  /* ------------------------------------------------------------------ */

  Filename = fname;
  CLEAR_IOERR ();
  Infile = share_fopen (Filename, read_binary, DENY_WRITE);
  if (Infile == NULL)
  {
    got_error (MSG_TXT (M_OPEN_MSG), Filename);
    rc = NOTHING_SENT;
    goto Done;
  }

  if (isatty (fileno (Infile)))
  {
    errno = 1;
    got_error (MSG_TXT (M_DEVICE_MSG), Filename);
    rc = NOTHING_SENT;
    goto Done;
  }

  /* ------------------------------------------------------------------ */
  /* Send the file                                                      */
  /* Display outbound filename, size, and ETA for sysop                 */
  /* ------------------------------------------------------------------ */

  unix_stat_noshift (Filename, &f);
  file_length = f.st_size;
  strcpy (TxStats.fname, fname8p3 (Filename));
  TxStats.FilePos = 0;
  TxStats.FileLen = file_length;
  css_upd ();

  /* ------------------------------------------------------------------ */
  /* Get outgoing file name; no directory path, lower case              */
  /* ------------------------------------------------------------------ */
#ifndef NEW_PATH_STUFF
  for (p = (byte *) ((alias != NULL) ? alias : Filename), q = Txbuf; *p;)
  {
    if ((*p == '/') || (*p == '\\') || (*p == ':'))
      q = Txbuf;
    else
      *q++ = (char) tolower (*p);

    p++;
  }

  *q++ = '\0';
  p = q;
#else
  p = ZMdmFlNmCndtn (Txbuf, ((alias != NULL) ? alias : Filename), NULL, 0);
  p += strlen (p);
  q = ++p;
#endif

  /* ------------------------------------------------------------------ */
  /* Zero out remainder of file header packet                           */
  /* ------------------------------------------------------------------ */
  while (q < (Txbuf + KSIZE))
    *q++ = '\0';

  /* ------------------------------------------------------------------ */
  /* Store filesize, time last modified, and file mode in header packet */
  /* ------------------------------------------------------------------ */
  filetime = f.st_mtime;
  sprintf ((char *) p, "%lu %lo %o", f.st_size, filetime, f.st_mode);

  /* ------------------------------------------------------------------ */
  /* Transmit the filename block and { the download                     */
  /* ------------------------------------------------------------------ */
  throughput (0, 1, 0L);

  /* ------------------------------------------------------------------ */
  /* Check the results                                                  */
  /* ------------------------------------------------------------------ */
  switch (ZS_SendFile (1 + strlen ((char *) p) + (int) (p - Txbuf), wazoo))
  {
  case ERROR:                  /* Something tragic happened */
    rc = CANNOT_SEND;
    break;

  case OK:                     /* File was sent */
    CLEAR_IOERR ();
    break;

  case ZSKIP:
    status_line (MSG_TXT (M_REMOTE_REFUSED), Filename);
    rc = FILE_SKIPPED;          /* Success but don't truncate! */
    break;

  default:
    /* Ignore the problem, get next file, trust other   */
    /* error handling mechanisms to deal with problems  */
    break;
  }

Done:

  if (Infile)
    fclose (Infile);
  Infile = NULL;

  if (batch < ZMODEM_INIT)      /* ZMODEM_ENDBATCH or ZMODEM_END */
    ZS_EndSend ();

  XON_DISABLE ();
  if (!direct_zap)
    XON_ENABLE ();

  // HJK 980805 - Last file was counted twice in stats
  if (batch != ZMODEM_ENDBATCH)
  {
    TxStats.cur_mxfrd += file_length;
    TxStats.cur_fxfrd++;
    TxStats.FilePos = TxStats.FileLen = -1L;
    css_upd ();
  }

  if (batch == -4711)           /* r.hoerner: debug and no warning ,) */
  {
    char junk[32];

    switch (batch)
    {
    case ZMODEM_INIT:
      sprintf (junk, "ZMODEM_INIT");
      break;

    case ZMODEM_END:
      sprintf (junk, "ZMODEM_END");
      break;

    case ZMODEM_ENDBATCH:
      sprintf (junk, "ZMODEM_ENDBATCH");
      break;

    default:
      sprintf (junk, "%d", batch);
    }

    status_line ("!SendZModem(%s,%s)=%d",
                 fname == NULL ? "(null)" : fname,
                 junk, rc);
  }

  return rc;
}                               /* send_Zmodem */

/* ------------------------------------------------------------------------ */
/* ZS SEND BINARY HEADER                                                    */
/* Send ZMODEM binary header hdr of type type                               */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
ZS_SendBinaryHeader (unsigned short type, register byte * hdr)
{
  int n;

#ifdef DEBUG
  show_debug_name ("ZS_SendBinaryHeader");
#endif

  BUFFER_BYTE (ZPAD);
  BUFFER_BYTE (ZDLE);

  if ((Crc32t = Txfcs32) != 0)
  {
    /* crc32 */
    unsigned long crc;

    BUFFER_BYTE (ZBIN32);
    ZS_SendByte ((byte) type);

    crc = 0xFFFFFFFFUL;
    crc = Z_32UpdateCRC (type, crc);

    for (n = 4; --n >= 0;)
    {
      ZS_SendByte (*hdr);
      crc = Z_32UpdateCRC (((unsigned short) (*hdr++)), crc);
    }

    crc = ~crc;
    for (n = 4; --n >= 0;)
    {
      ZS_SendByte ((byte) crc);
      crc >>= 8;
    }
  }
  else
  {
    /* crc16 */
    register unsigned short crc;

    BUFFER_BYTE (ZBIN);
    ZS_SendByte ((byte) type);

    crc = Z_UpdateCRC (type, 0);

    for (n = 4; --n >= 0;)
    {
      ZS_SendByte (*hdr);
      crc = Z_UpdateCRC (((unsigned short) (*hdr++)), crc);
    }

    ZS_SendByte ((byte) (crc >> 8));
    ZS_SendByte ((byte) crc);
  }

  UNBUFFER_BYTES ();

  if (type != ZDATA)
  {
    while (CARRIER && !OUT_EMPTY ())
      time_release ();
    if (!CARRIER)
      CLEAR_OUTBOUND ();
  }
}                               /* ZS_SendBinaryHeader */

/* ------------------------------------------------------------------------ */
/* ZS SEND DATA                                                             */
/* Send binary array buf with ending ZDLE sequence frameend                 */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
ZS_SendData (register byte * buf, int length, unsigned short frameend)
{
#ifdef DEBUG
  show_debug_name ("ZS_SendData");
#endif

  if (Crc32t)
  {
    /* crc32 */
    unsigned long crc;

    crc = 0xFFFFFFFFUL;
    for (; --length >= 0; ++buf)
    {
      ZS_SendByte (*buf);
      crc = Z_32UpdateCRC (((unsigned short) (*buf)), crc);
    }

    BUFFER_BYTE (ZDLE);
    BUFFER_BYTE ((unsigned char) frameend);
    crc = Z_32UpdateCRC (frameend, crc);
    crc = ~crc;

    for (length = 4; --length >= 0;)
    {
      ZS_SendByte ((byte) crc);
      crc >>= 8;
    }
  }
  else
  {
    /* crc16 */
    register unsigned short crc;

    crc = 0;
    for (; --length >= 0;)
    {
      ZS_SendByte (*buf);
      crc = Z_UpdateCRC (((unsigned short) (*buf++)), crc);
    }

    BUFFER_BYTE (ZDLE);
    BUFFER_BYTE ((unsigned char) frameend);
    crc = Z_UpdateCRC (frameend, crc);
    ZS_SendByte ((byte) (crc >> 8));
    ZS_SendByte ((byte) crc);
  }

  UNBUFFER_BYTES ();

  if (frameend == ZCRCW)
  {
    if (!direct_zap)
      SENDBYTE (XON);
    while (CARRIER && !OUT_EMPTY ())
      time_release ();
    if (!CARRIER)
      CLEAR_OUTBOUND ();
  }
}                               /* ZS_SendData */


/* ------------------------------------------------------------------------ */
/* ZS SEND BYTE                                                             */
/* Send character c with ZMODEM escape sequence encoding.                   */
/* Escape XON, XOFF. Escape CR following @ (Telenet net escape)             */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
ZS_SendByte (register byte c)
{
  static byte lastsent;

  switch (c)
  {
  case 015:                    /* 0x0d */
  case 0215:                   /* 0x8d */
    if ((lastsent & 0x7F) != '@')
      goto SendIt;
    /* else fall through */

  case 020:                    /* 0x10 */
  case 021:                    /* 0x11 */
  case 023:                    /* 0x13 */
  case 0220:                   /* 0x90 */
  case 0221:                   /* 0x91 */
  case 0223:                   /* 0x93 */
  case ZDLE | 0x80:
    if (direct_zap)
      goto SendIt;

  case ZDLE:                   /* Quoted characters */
    BUFFER_BYTE (ZDLE);
    c ^= 0x40;
    /* Then fall through */

  default:                     /* Normal character output */

  SendIt:
    BUFFER_BYTE (lastsent = c);

  }
}                               /* ZS_SendByte */

/* ------------------------------------------------------------------------ */
/* ZS GET RECEIVER INFO                                                     */
/* Get the receiver's init parameters                                       */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
ZS_GetReceiverInfo ()
{
  int n;
  int res;

#ifdef DEBUG
  show_debug_name ("ZS_GetReceiverInfo");
#endif

  for (n = 10; --n >= 0;)
  {
    res = Z_GetHeader ((byte *) Rxhdr);
    switch (res)
    {
    case ZCHALLENGE:           /* Echo receiver's challenge number     */
      Z_PutLongIntoHeader (Rxpos);
      Z_SendHexHeader (ZACK, (byte *) Txhdr);
      continue;

    case ZCOMMAND:             /* They didn't see our ZRQINIT          */
      Z_PutLongIntoHeader (0L);
      Z_SendHexHeader (ZRQINIT, (byte *) Txhdr);
      continue;

    case ZRINIT:
      Rxflags = 0377 & Rxhdr[ZF0];
      Rxbuflen = ((word) Rxhdr[ZP1] << 8) | Rxhdr[ZP0];
      Txfcs32 = Rxflags & CANFC32;
      return OK;

    case ZCAN:
      status_line (MSG_TXT (M_CAN_MSG));
      TxStats.cur_errors++;
      return ERROR;

    case RCDO:
      // status_line (MSG_TXT (M_NO_CARRIER));
      TxStats.cur_errors++;
      return ERROR;

    case TIMEOUT:
      status_line ("!%s", MSG_TXT (M_TIMEOUT));
      TxStats.cur_errors++;
      return ERROR;

    case ZRQINIT:
      if (Rxhdr[ZF0] == ZCOMMAND)
        continue;
      /* else fall through */

    default:
      Z_SendHexHeader (ZNAK, (byte *) Txhdr);
      continue;
    }
  }

  return ERROR;
}                               /* ZS_GetReceiverInfo */

/* ------------------------------------------------------------------------ */
/* ZS SEND FILE                                                             */
/* Send ZFILE frame and begin sending ZDATA frame                           */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
ZS_SendFile (int blen, int wazoo)
{
  register int c;
  long t;

#ifdef DEBUG
  show_debug_name ("ZS_SendFile");
#endif

  for (;;)
  {
    if (got_ESC ())
    {
      CLEAR_OUTBOUND ();
      XON_DISABLE ();           /* Make sure xmitter is unstuck */
      send_can ();              /* transmit at least 10 cans    */
      t = timerset (200);       /* wait no more than 2 seconds  */

      while (!timeup (t) && !OUT_EMPTY () && CARRIER)
        time_release ();        /* Give up slice while waiting  */

      if (!direct_zap)
        XON_ENABLE ();          /* Turn XON/XOFF back on...     */

      status_line (MSG_TXT (M_KBD_MSG));
      TxStats.cur_errors++;
      doerrorcountersfornextfile (0);
      return ERROR;
    }
    else if (!CARRIER)
    {
      TxStats.cur_errors++;
      doerrorcountersfornextfile (0);
      return ERROR;
    }

    Txhdr[ZF0] = LZCONV;        /* Default file conversion mode */
    Txhdr[ZF1] = LZMANAG;       /* Default file management mode */
    Txhdr[ZF2] = LZTRANS;       /* Default file transport mode  */
    Txhdr[ZF3] = 0;
    ZS_SendBinaryHeader (ZFILE, (byte *) Txhdr);
    ZS_SendData (Txbuf, blen, ZCRCW);

  Again:

    switch (c = Z_GetHeader ((byte *) Rxhdr))
    {
    case ZRINIT:
      while ((c = Z_GetByte (50)) > 0)
        if (c == ZPAD)
          goto Again;
      /* if we run out, Fall thru to */

    default:
      continue;

    case ZCAN:
    case RCDO:
    case TIMEOUT:
    case ZFIN:
    case ZFERR:
    case ZABORT:
      TxStats.cur_errors++;
      doerrorcountersfornextfile (0);
      return ERROR;

    case ZSKIP:                /* Other system wants to skip this file    */
      doerrorcountersfornextfile (0);
      return c;

    case ZRPOS:                /* Resend from this position...            */
      fseek (Infile, Rxpos, SEEK_SET);
      if (Rxpos != 0L)
      {
        TxStats.cur_errors++;
        status_line (MSG_TXT (M_SYNCHRONIZING), Rxpos);
        CLEAR_OUTBOUND ();      /* Get rid of queued data */
        XON_DISABLE ();         /* End XON/XOFF restraint */
        if (!direct_zap)
        {
          SENDBYTE (XON);       /* Send XON to remote     */
          // TJW970714 remark: BTR doesn't send XON !?
          XON_ENABLE ();        /* Start XON/XOFF again   */
        }
      }

      LastZRpos = Strtpos = Txpos = Rxpos;
      ZRPosCount = 10;
      CLEAR_INBOUND ();
      doerrorcountersfornextfile (0);
      return ZS_SendFileData (wazoo);
    }
  }

#ifndef __IBMC__
#ifndef __WATCOMC__             /* MR  961107 watcom is already happy...   */
#ifndef __TURBOC__              /* so is BC 3.1                  ...   */
  return ERROR;                 /* TJW 960606 just to make compiler happy, */
#endif /* should never happen */
#endif
#endif
}                               /* ZS_SendFile */

/* ------------------------------------------------------------------------ */
/* ZS SEND FILE DATA                                                        */
/* Send the data in the file                                                */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
ZS_SendFileData (int wazoo)
{
  register int c, e;
  unsigned long ulrate;
  word newcnt;
  word blklen;
  word maxblklen;
  word goodblks = 0;
  word goodneeded = 1;
  long lSize, lTime;
  long t;

#ifdef DEBUG
  show_debug_name ("ZS_SendFileData");
#endif

  ulrate = cur_baud.rate_value;
  maxblklen = (ulrate >= 9600) ? WAZOOMAX :
    ((ulrate < 300) ? 128 : (int) ulrate / 300 * 256);

  if (maxblklen > WAZOOMAX)
    maxblklen = WAZOOMAX;
  if (!wazoo && maxblklen > KSIZE)
    maxblklen = KSIZE;
  if (Rxbuflen && maxblklen > (unsigned) Rxbuflen)
    maxblklen = Rxbuflen;
  if (wazoo && (remote_capabilities & ZED_ZIPPER))
    maxblklen = KSIZE;

  blklen = (fstblklen != 0) ? fstblklen : maxblklen;
  goodneeded = (fstblklen != 0) ? 8 : 1;

SomeMore:

  if (CHAR_AVAIL ())
  {
  WaitAck:

    switch (c = ZS_SyncWithReceiver (1))
    {
    case ZSKIP:                /* Skip this file                          */
      return c;

    case ZACK:
      break;

    case ZRPOS:                /* Resume at this position                 */
      TxStats.cur_errors++;
      blklen = ((blklen >> 2) > 64) ? blklen >> 2 : 64;
      goodblks = 0;
      goodneeded = ((goodneeded << 1) > 16) ? 16 : goodneeded << 1;
      break;

    case ZRINIT:               /* Receive init                            */
      goto file_sent;

    case TIMEOUT:              /* Timed out on message from other side    */
      TxStats.cur_errors++;
      break;

    default:
      TxStats.cur_errors++;
      status_line (MSG_TXT (M_CAN_MSG));
      fclose (Infile);
      Infile = NULL;
      return ERROR;
    }

    /* Noise probably got us here. Odds of surviving are not good. But we
     * have to get unstuck in any event. */

    Z_UncorkTransmitter ();     /* Get our side free if need be      */

    if (!direct_zap)
      SENDBYTE (XON);           /* Send an XON to release other side */
    // TJW970714 remark: BTR doesn't send XON

    while (CHAR_AVAIL ())
    {
      switch (MODEM_IN ())
      {
      case CAN:
      case RCDO:
      case ZPAD:
        goto WaitAck;
      }
    }
  }

  newcnt = Rxbuflen;
  Z_PutLongIntoHeader (Txpos);
  ZS_SendBinaryHeader (ZDATA, (byte *) Txhdr);

  do
  {
    if (got_ESC ())
    {
      CLEAR_OUTBOUND ();
      XON_DISABLE ();           /* Make sure xmitter is unstuck */
      send_can ();              /* transmit at least 10 cans    */
      t = timerset (200);       /* wait no more than 2 seconds  */

      while (!timeup (t) && !OUT_EMPTY () && CARRIER)
        time_release ();        /* Give up slice while waiting  */

      if (!direct_zap)
        XON_ENABLE ();          /* Turn XON/XOFF back on...     */

      status_line (MSG_TXT (M_KBD_MSG));
      goto oops;
    }

    if (!CARRIER)
      goto oops;

    if ((unsigned) (c = fread (Txbuf, 1, blklen, Infile)) != z_size)
      z_size = c;

    if ((unsigned) c < blklen)
    {
#ifdef BTPE_DEBUG
      status_line (">We are going to ZCRCE with %d bytes, blklen is %d",
                   c, blklen);
#endif
      e = ZCRCE;
    }
    else
    {
      newcnt -= c;
      if (Rxbuflen && ((newcnt == 0) || (newcnt > WAZOOMAX)))
        e = ZCRCW;
      else
        e = ZCRCG;
    }

#ifdef SPIRIT_FIX
    /* There is a modem, the Spirit II, which has a bug when
       talking to a Rockwell modem, in that it will send every
       block twice.  Pausing for 3/10 second will work around
       that bug. */
    if (Txpos == 0)
    {
      status_line (">pausing on block 0");
      timer (3);
    }
#endif

    ZS_SendData (Txbuf, c, (unsigned short) e);

    Txpos += c;

    TxStats.FilePos = Txpos;
    TxStats.FileLen = file_length;
    css_upd ();

    if (blklen < maxblklen && ++goodblks > goodneeded)
    {
      blklen = ((word) (blklen << 1) < maxblklen) ? blklen << 1 : maxblklen;
      goodblks = 0;
    }

    if (e == ZCRCW)
      goto WaitAck;

    while (CHAR_AVAIL ())
    {
      switch (MODEM_IN ())
      {
      case CAN:
      case RCDO:               /* Interruption detected;               */
      case ZPAD:               /* stop sending and process complaint   */
        TxStats.cur_errors++;
        status_line ("!%s", MSG_TXT (M_TROUBLE));
        CLEAR_OUTBOUND ();
        ZS_SendData (Txbuf, 0, ZCRCE);
        goto WaitAck;
      }
    }
  }
  while (e == ZCRCG);

  for (;;)
  {
    Z_PutLongIntoHeader (Txpos);
    ZS_SendBinaryHeader (ZEOF, (byte *) Txhdr);

    switch (ZS_SyncWithReceiver (7))
    {
    case ZACK:
      continue;

    case ZRPOS:                /* Resume at this position...              */
      TxStats.cur_errors++;
      goto SomeMore;

    case ZRINIT:               /* Receive init                            */
    file_sent:
      lSize = Txpos - Strtpos;
      lTime = throughput (1, 1, lSize) / PER_SECOND;
      status_line ("%s-Z%s %s", MSG_TXT (M_FILE_SENT), Crc32t ? "/32" : "", Filename);
      update_files (1, Filename, lSize, lTime, &(TxStats.cur_errors));
      return OK;

    case ZSKIP:                /* Request to skip the current file        */
      status_line (MSG_TXT (M_SKIP_MSG));
      CLEAR_IOERR ();
      fclose (Infile);
      Infile = NULL;
      return c;

    default:
    oops:
      TxStats.cur_errors++;
      status_line (MSG_TXT (M_CAN_MSG));
      fclose (Infile);
      Infile = NULL;
      return ERROR;
    }
  }

#ifndef __IBMC__
#ifndef __WATCOMC__             /* MR  961107 watcom is already happy...   */
#ifndef __TURBOC__
  return ERROR;                 /* TJW 960606 just to make compiler happy, */
#endif
#endif /* should never happen */
#endif
}                               /* ZS_SendFileData */

/* ------------------------------------------------------------------------ */
/* ZS SYNC WITH RECEIVER                                                    */
/* Respond to receiver's complaint, get back in sync with receiver          */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
ZS_SyncWithReceiver (int num_errs)
{
  register int c;
  char j[50];

#ifdef DEBUG
  show_debug_name ("ZS_SyncWithReceiver");
#endif

  for (;;)
  {
    c = Z_GetHeader ((byte *) Rxhdr);
    CLEAR_INBOUND ();
    switch (c)
    {
    case TIMEOUT:
      status_line ("!%s", MSG_TXT (M_TIMEOUT));
      if ((num_errs--) >= 0)
      {
        TxStats.cur_errors++;
        break;
      }
      /* else fall through */

    case ZCAN:
    case ZABORT:
    case ZFIN:
    case RCDO:
      TxStats.cur_errors++;
      status_line (MSG_TXT (M_ERROR));
      return ERROR;

    case ZRPOS:
      TxStats.cur_errors++;
#ifdef BTPE_DEBUG
      status_line (">received ZRPOS of %ld", Rxpos);
#endif

      if (Rxpos == LastZRpos)   /* Same as last time?    */
      {
        if (!(--ZRPosCount))    /* Yup, 10 times yet?    */
        {
#ifdef BTPE_DEBUG
          status_line (">over 10 repositions");
#endif
          return ERROR;         /* Too many, get out     */
        }
      }
      else
        ZRPosCount = 10;        /* Reset repeat count    */

      LastZRpos = Rxpos;        /* Keep track of this    */
      rewind (Infile);          /* In case file EOF seen */
      fseek (Infile, Rxpos, SEEK_SET);
      Txpos = Rxpos;
      TxStats.FilePos = Txpos;
      css_upd ();
      sprintf (j, MSG_TXT (M_RESENDING_FROM), Txpos);
      status_line ("!%s", j);
      return c;

    case ZSKIP:
      status_line (MSG_TXT (M_SKIP_MSG));  /* fall through */

    case ZRINIT:
      CLEAR_IOERR ();
      fclose (Infile);
      Infile = NULL;
      return c;

    case ZACK:
      return c;

    default:
      status_line ("!%s", MSG_TXT (M_I_DONT_KNOW));
      ZS_SendBinaryHeader (ZNAK, (byte *) Txhdr);
      continue;
    }
  }

#ifndef __IBMC__
#ifndef __WATCOMC__             /* MR  961107 watcom is already happy...   */
#ifndef __TURBOC__
  return ERROR;                 /* TJW 960606 just to make compiler happy, */
#endif /* should never happen */
#endif
#endif
}                               /* ZS_SyncWithReceiver */

/* ------------------------------------------------------------------------ */
/* ZS END SEND                                                              */
/* Say BIBI to the receiver, try to do it cleanly                           */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
ZS_EndSend ()
{

#ifdef DEBUG
  show_debug_name ("ZS_EndSend");
#endif

  CLEAR_OUTBOUND ();
  CLEAR_INBOUND ();

  for (;;)
  {
    Z_PutLongIntoHeader (0L);
    ZS_SendBinaryHeader (ZFIN, (byte *) Txhdr);

    switch (Z_GetHeader ((byte *) Rxhdr))
    {
    case ZFIN:
      SENDBYTE ('O');
      SENDBYTE ('O');

      while (CARRIER && !OUT_EMPTY ())
        time_release ();

      if (!CARRIER)
        CLEAR_OUTBOUND ();
      /* fallthrough... */

    case ZCAN:
    case RCDO:
    case TIMEOUT:
      return;
    }
  }
}                               /* ZS_EndSend */


// zmisc.c =================================================================

/* Send a byte as two hex digits */
#define Z_PUTHEX(i,c) {i=(c);SENDBYTE(hex[((i)&0xF0)>>4]);SENDBYTE(hex[(i)&0xF]);}

/* ------------------------------------------------------------------------ */
/* Z GET BYTE                                                               */
/* Get a byte from the modem;                                               */
/* return TIMEOUT if no read within timeout tenths,                         */
/* return RCDO if carrier lost                                              */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
Z_GetByte (int tenths)
{
  long timeout;

  if (PEEKBYTE () >= 0)
    return (MODEM_IN ());

  timeout = timerset (tenths * 10);

  do
  {
    if (PEEKBYTE () >= 0)
      return MODEM_IN ();

    if (!CARRIER)
      return RCDO;

    if (got_ESC ())
      return -1;

    time_release ();
  }
  while (!timeup (timeout));

  return TIMEOUT;
}

/* ------------------------------------------------------------------------ */
/* Z PUT STRING                                                             */
/* Send a string to the modem, processing for \336 (sleep 1 sec)            */
/* and \335 (break signal, ignored)                                         */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
Z_PutString (register unsigned char *s)
{
  register unsigned c;

  while (*s)
  {
    switch (c = *s++)
    {
    case (unsigned int) '\336':
      big_pause (2);
      break;

    case (unsigned int) '\335':  /* Should send a break on this */
      break;

    default:
      SENDBYTE ((unsigned char) c);
    }
  }

  Z_UncorkTransmitter ();
}                               /* Z_PutString */

/* ------------------------------------------------------------------------ */
/* Z SEND HEX HEADER                                                        */
/* Send ZMODEM HEX header hdr of type type                                  */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
Z_SendHexHeader (unsigned int type, register unsigned char *hdr)
{
  register int n;
  register int i;
  register word crc;

  Z_UncorkTransmitter ();       /* Get our transmitter going */

#ifdef DEBUG
  show_debug_name ("Z_SendHexHeader");
#endif

  SENDBYTE (ZPAD);
  SENDBYTE (ZPAD);
  SENDBYTE (ZDLE);
  SENDBYTE (ZHEX);

  Z_PUTHEX (i, type);

  Crc32t = 0;
  crc = Z_UpdateCRC (type, 0);

  for (n = 4; --n >= 0;)
  {
    Z_PUTHEX (i, (*hdr));
    crc = Z_UpdateCRC (((unsigned short) (*hdr++)), crc);
  }
  Z_PUTHEX (i, (crc >> 8));
  Z_PUTHEX (i, crc);

  /* Make it printable on remote machine */
  SENDBYTE ('\r');
  SENDBYTE ('\n');

  /* Uncork the remote in case a fake XOFF has stopped data flow */
  if (type != ZFIN && type != ZACK)
    SENDBYTE (021);

  if (!CARRIER)
    CLEAR_OUTBOUND ();
}                               /* Z_SendHexHeader */

/* ------------------------------------------------------------------------ */
/* Z UNCORK TRANSMITTER                                                     */
/* Wait a reasonable amount of time for transmitter buffer to clear.        */
/*   When it does, or when time runs out, turn XON/XOFF off then on.        */
/*   This should release a transmitter stuck by line errors.                */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
Z_UncorkTransmitter (void)
{
  long t;

#ifdef DEBUG
  show_debug_name ("Z_UncorkTransmitter");
#endif

  if (!OUT_EMPTY () && CARRIER)
  {
    t = timerset (5 * Rxtimeout);  /* Wait for silence */

    while (!timeup (t) && !OUT_EMPTY () && CARRIER)
      time_release ();          /* Give up slice while waiting  */
  }

  com_kick ();
}

/* ------------------------------------------------------------------------ */
/* Z GET HEADER                                                             */
/* Read a ZMODEM header to hdr, either binary or hex.                       */
/*   On success, set Zmodem to 1 and return type of header.                 */
/*   Otherwise return negative on error                                     */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
Z_GetHeader (byte * hdr)
{
  register int c;
  unsigned long n;
  int cancount;

#ifdef DEBUG
  show_debug_name ("Z_GetHeader");
#endif

  n = (unsigned long) cur_baud.rate_value;  /* Max characters before
                                             * start of frame */
  cancount = 5;

Again:
  if (got_ESC ())
  {
    send_can ();
    status_line (MSG_TXT (M_KBD_MSG));
    return ZCAN;
  }

  Rxframeind = Rxtype = 0;

  switch (c = _Z_TimedRead ())
  {
  case ZPAD:
  case ZPAD | 0200:            /* This is what we want. */
    break;

  case RCDO:
  case TIMEOUT:
    goto Done;

  case CAN:
  GotCan:
    if (--cancount <= 0)
    {
      c = ZCAN;
      goto Done;
    }

    switch (c = Z_GetByte (1))
    {
    case TIMEOUT:
      goto Again;

    case ZCRCW:
      c = ERROR;
      /* fallthrough... */

    case RCDO:
      goto Done;

    case CAN:
      if (--cancount <= 0)
      {
        c = ZCAN;
        goto Done;
      }

      goto Again;
    }
    /* fallthrough... */

  default:
  Agn2:

    if (--n == 0)               /* TJW 960617 was: <=, but n is unsigned -> == */
    {
      status_line (MSG_TXT (M_FUBAR_MSG));
      return ERROR;
    }

    if (c != CAN)
      cancount = 5;
    goto Again;
  }

  cancount = 5;

Splat:
  switch (c = _Z_TimedRead ())
  {
  case ZDLE:                   /* This is what we want. */
    break;

  case ZPAD:
    goto Splat;

  case RCDO:
  case TIMEOUT:
    goto Done;

  default:
    goto Agn2;
  }

  switch (c = _Z_TimedRead ())
  {
  case ZBIN:
    Rxframeind = ZBIN;
    Crc32 = 0;
    c = _Z_GetBinaryHeader (hdr);
    break;

  case ZBIN32:
    Crc32 = Rxframeind = ZBIN32;
    c = _Z_32GetBinaryHeader (hdr);
    break;

  case ZHEX:
    Rxframeind = ZHEX;
    Crc32 = 0;
    c = _Z_GetHexHeader (hdr);
    break;

  case CAN:
    goto GotCan;

  case RCDO:
  case TIMEOUT:
    goto Done;

  default:
    goto Agn2;
  }

  Rxpos = _Z_PullLongFromHeader (hdr);

Done:
  return c;
}                               /* Z_GetHeader */

/* ------------------------------------------------------------------------ */
/* Z GET BINARY HEADER                                                      */
/* Receive a binary style header (type and position)                        */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
_Z_GetBinaryHeader (register unsigned char *hdr)
{
  register int c;
  register word crc;
  register int n;

#ifdef DEBUG
  show_debug_name ("Z_GetBinaryHeader");
#endif

  if ((c = Z_GetZDL ()) & ~0xFF)
    return c;
  Rxtype = c;
  crc = Z_UpdateCRC (c, 0);

  for (n = 4; --n >= 0;)
  {
    if ((c = Z_GetZDL ()) & ~0xFF)
      return c;
    crc = Z_UpdateCRC (c, crc);
    *hdr++ = (unsigned char) (c & 0xff);
  }

  if ((c = Z_GetZDL ()) & ~0xFF)
    return c;

  crc = Z_UpdateCRC (c, crc);
  if ((c = Z_GetZDL ()) & ~0xFF)
    return c;

  crc = Z_UpdateCRC (c, crc);
  if (crc & 0xFFFF)
  {
    status_line ("!%s", MSG_TXT (M_CRC_MSG));
    return ERROR;
  }

  return Rxtype;
}                               /* _Z_GetBinaryHeader */

/* ------------------------------------------------------------------------ */
/* Z GET BINARY HEADER with 32 bit CRC                                      */
/* Receive a binary style header (type and position)                        */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
_Z_32GetBinaryHeader (register unsigned char *hdr)
{
  register int c;
  register unsigned long crc;
  register int n;

#ifdef DEBUG
  show_debug_name ("Z_32GetBinaryHeader");
#endif

  if ((c = Z_GetZDL ()) & ~0xFF)
    return c;
  Rxtype = c;
  crc = 0xFFFFFFFFUL;
  crc = Z_32UpdateCRC (c, crc);

  for (n = 4; --n >= 0;)
  {
    if ((c = Z_GetZDL ()) & ~0xFF)
      return c;
    crc = Z_32UpdateCRC (c, crc);
    *hdr++ = (unsigned char) (c & 0xff);
  }

  for (n = 4; --n >= 0;)
  {
    if ((c = Z_GetZDL ()) & ~0xFF)
      return c;

    crc = Z_32UpdateCRC (c, crc);
  }

  if (crc != 0xDEBB20E3UL)
  {
    status_line ("!%s", MSG_TXT (M_CRC_MSG));
    return ERROR;
  }

  return Rxtype;
}                               /* _Z_32GetBinaryHeader */

/* ------------------------------------------------------------------------ */
/* Z GET HEX HEADER                                                         */
/* Receive a hex style header (type and position)                           */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
_Z_GetHexHeader (register unsigned char *hdr)
{
  register int c;
  register word crc;
  register int n;

#ifdef DEBUG
  show_debug_name ("Z_GetHexHeader");
#endif

  if ((c = _Z_GetHex ()) < 0)
    return c;
  Rxtype = c;
  crc = Z_UpdateCRC (c, 0);

  for (n = 4; --n >= 0;)
  {
    if ((c = _Z_GetHex ()) < 0)
      return c;
    crc = Z_UpdateCRC (c, crc);
    *hdr++ = (unsigned char) c;
  }

  if ((c = _Z_GetHex ()) < 0)
    return c;

  crc = Z_UpdateCRC (c, crc);
  if ((c = _Z_GetHex ()) < 0)
    return c;

  crc = Z_UpdateCRC (c, crc);
  if (crc & 0xFFFF)
  {
    status_line ("!%s", MSG_TXT (M_CRC_MSG));
    return ERROR;
  }

  if (Z_GetByte (1) == '\r')
    Z_GetByte (1);              /* Throw away possible cr/lf */

  return Rxtype;
}

/* ------------------------------------------------------------------------ */
/* Z GET HEX                                                                */
/* Decode two lower case hex digits into an 8 bit byte value                */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
_Z_GetHex (void)
{
  register int c, n;

#ifdef DEBUG
  show_debug_name ("Z_GetHex");
#endif

  if ((n = _Z_TimedRead ()) < 0)
    return n;
  n -= '0';

  if (n > 9)
    n -= ('a' - ':');

  if (n & ~0xF)
    return ERROR;

  if ((c = _Z_TimedRead ()) < 0)
    return c;

  c -= '0';
  if (c > 9)
    c -= ('a' - ':');

  if (c & ~0xF)
    return ERROR;

  return ((n << 4) | c);
}

/* ------------------------------------------------------------------------ */
/* Z GET ZDL                                                                */
/* Read a byte, checking for ZMODEM escape encoding                         */
/* including CAN*5 which represents a quick abort                           */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
Z_GetZDL (void)
{
  register int c;

  if ((c = Z_GetByte (Rxtimeout)) != ZDLE)
    return c;

  switch (c = Z_GetByte (Rxtimeout))
  {
  case CAN:
    return ((c = Z_GetByte (Rxtimeout)) < 0) ? c :
      ((c == CAN) && ((c = Z_GetByte (Rxtimeout)) < 0)) ? c :
      ((c == CAN) && ((c = Z_GetByte (Rxtimeout)) < 0)) ? c : (GOTCAN);

  case ZCRCE:
  case ZCRCG:
  case ZCRCQ:
  case ZCRCW:
    return (c | GOTOR);

  case ZRUB0:
    return 0x7F;

  case ZRUB1:
    return 0xFF;

  default:
    return (c < 0) ? c :
      ((c & 0x60) == 0x40) ? (c ^ 0x40) : ERROR;
  }
}                               /* Z_GetZDL */

/* ------------------------------------------------------------------------ */
/* Z TIMED READ                                                             */
/* Read a character from the modem line with timeout.                       */
/*  Eat parity, XON and XOFF characters.                                    */
/* ------------------------------------------------------------------------ */

static int LOCALFUNC
_Z_TimedRead (void)
{
  register int c;

#ifdef DEBUG
  show_debug_name ("Z_TimedRead");
#endif

  for (;;)
  {
    if ((c = Z_GetByte (Rxtimeout)) < 0)
      return c;

    switch (c &= 0x7F)
    {
    case XON:
    case XOFF:
      continue;

    default:
      if (!(c & 0x60))
        continue;
      /* Else fall through */

    case '\r':
    case '\n':
    case ZDLE:
      return c;
    }
  }

#ifndef __IBMC__                /* VAC 3.00 is happy */
#ifndef __WATCOMC__             /* WATCOM 10.0 is happy */
#ifndef __TURBOC__              /* Borland C 3.1 is happy */
  return 0;                     /* TJW 960604 just to make compiler happy */
#endif
#endif
#endif
}                               /* _Z_TimedRead */

/* ------------------------------------------------------------------------ */
/* Z LONG TO HEADER                                                         */
/* Store long integer pos in Txhdr                                          */
/* ------------------------------------------------------------------------ */

static void LOCALFUNC
Z_PutLongIntoHeader (long pos)
{
#ifndef GENERIC
  *((long *) Txhdr) = pos;
#else
  Txhdr[ZP0] = pos;
  Txhdr[ZP1] = pos >> 8;
  Txhdr[ZP2] = pos >> 16;
  Txhdr[ZP3] = pos >> 24;
#endif
}                               /* Z_PutLongIntoHeader */

/* ------------------------------------------------------------------------ */
/* Z PULL LONG FROM HEADER                                                  */
/* Recover a long integer from a header                                     */
/* ------------------------------------------------------------------------ */

static long LOCALFUNC
_Z_PullLongFromHeader (unsigned char *hdr)
{
#ifndef GENERIC
  return (*((long *) hdr));     /*PLF Fri  05-05-1989  06:42:41 */
#else
  long l;

  l = hdr[ZP3];
  l = (l << 8) | hdr[ZP2];
  l = (l << 8) | hdr[ZP1];
  l = (l << 8) | hdr[ZP0];
  return l;
#endif
}                               /* _Z_PullLongFromHeader */

/* $Id: zmodem.c,v 1.8 1999/03/23 22:28:58 mr Exp $ */
