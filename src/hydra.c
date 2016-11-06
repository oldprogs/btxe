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
 * Filename    : $Source: E:/cvs/btxe/src/hydra.c,v $
 * Revision    : $Revision: 1.11 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:46 $
 * State       : $State: Exp $
 * Orig. Author: Arjen Lentz, contributed by Michael Buenter
 *
 * Description : HydraCom Version 1.08 (+ rev. upto 18 dec 93)
 *
 * Note        :
 *
 *   HydraCom was written by Arjen G. Lentz, LENTZ SOFTWARE-DEVELOPMENT
 *   COPYRIGHT (C) 1991-1993; ALL RIGHTS RESERVED
 *
 *   The HYDRA protocol was designed by Arjen G. Lentz,
 *   LENTZ SOFTWARE-DEVELOPMENT and Joaquim H. Homrighausen
 *   COPYRIGHT (C) 1991-1993; ALL RIGHTS RESERVED
 *
 *   Revision history:
 *     06 Sep 1991 - (AGL) First tryout
 *     .. ... .... - Internal development
 *     11 Jan 1993 - HydraCom version 1.00, Hydra revision 001 (01 Dec 1992)
 *     13 Mar 1993 - HydraCom version 1.03, Hydra revision 001 (01 Dec 1992)
 *     ..
 *     Changes made by Michael Buenter:
 *     03 Dec 1993 - adapted hydra for BT-EE and BT 2.58, many changes
 *     ..
 *     Updated source fixes by Arjen Lentz:
 *     04 Sep 1993 - HydraCom version 1.08, Hydra revision 001 (01 Dec 1992)
 *     23 Dec 1993 - updated to post-1.08 revisions upto 18 Dec 1993
 *
 *   For complete details of the Hydra and HydraCom licensing restrictions,
 *   please refer to the license agreements which are published in their
 *   entirety in HYDRACOM.C and LICENSE.DOC, and also contained in the
 *   documentation file HYDRACOM.DOC
 *
 *   Use of this file is subject to the restrictions contained in the Hydra
 *   and HydraCom licensing agreements. If you do not find the text of this
 *   agreement in any of the aforementioned files, or if you do not have these
 *   files, you should immediately contact LENTZ SOFTWARE-DEVELOPMENT and/or
 *   Joaquim Homrighausen at one of the addresses listed below. In no event
 *   should you proceed to use this file without having accepted the terms of
 *   the Hydra and HydraCom licensing agreements, or such other agreement as
 *   you are able to reach with LENTZ SOFTWARE-DEVELOMENT and Joaquim
 *   Homrighausen.
 *
 *   Hydra protocol design and HydraCom driver:     Hydra protocol design:
 *     Arjen G. Lentz                                 Joaquim H. Homrighausen
 *     LENTZ SOFTWARE-DEVELOPMENT                     389, route d'Arlon
 *     Langegracht 7B                                 L-8011 Strassen
 *     3811 BT  Amersfoort                            Luxembourg
 *     The Netherlands
 *     FidoNet 2:283/512, AINEX-BBS +31-33-633916     FidoNet 2:270/17
 *     arjen_lentz@f512.n283.z2.fidonet.org           joho@ae.lu
 *
 *   Please feel free to contact us at any time to share your comments about
 *   our software and/or licensing policies.
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"
#include "aglcrc.h"

static int LOCALFUNC xfer_init (char *fname, long fsize, long ftime);
static int LOCALFUNC xfer_okay (void);

#define inteli(x) (x)
#define intell(x) (x)

extern void chatautostart (void);

/* HYDRA's memory ---------------------------------------------------------- */
static BOOL originator;         /* are we the orig side?     */
static int batchesdone;         /* No. HYDRA batches done    */
static BOOL hdxlink;            /* hdx link & not orig side  */
static ULONG options;           /* INIT options hydra_init() */
static word timeout;            /* general timeout in secs   */
static char abortstr[] =
{
  24, 24, 24, 24, 24, 24, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0
};

#ifdef HYDRADEV

static char *hdxmsg = "Fallback to one-way xfer";

#endif

static char *pktprefix = "";
static char *autostr = "hydra\r";

/* TJW 960522: made tables static */

static word crc16tab[CRC_TABSIZE];  /* CRC-16 table                    */
static ULONG *crc32tab;         /* CRC-32 table                        */

static char *GenericError = "!%s";
static byte *txbuf, *rxbuf;     /* packet buffers                      */
static ULONG txoptions, rxoptions;  /* HYDRA options (INIT seq)        */
static char txpktprefix[H_PKTPREFIX + 1];  /* pkt prefix str they want */
static long txwindow, rxwindow; /* window size (0=streaming)           */
static h_timer braindead;       /* braindead timer                     */
static byte *txbufin;           /* read data from disk here            */
static byte txlastc;            /* last byte put in txbuf              */
static byte rxdle;              /* count of received H_DLEs            */
static byte rxpktformat;        /* format of pkt receiving             */
static byte *rxbufptr;          /* current position in rxbuf           */
static byte *rxbufmax;          /* highwatermark of rxbuf              */
static char txfname[PATHLEN + 1], rxfname[PATHLEN + 1];  /* fname of cur file */
static char rxpathname[PATHLEN + 1];  /* pathname of currrent file     */
static long txftime, rxftime;   /* file timestamp (UNIX)               */
static long txfsize, rxfsize;   /* file length                         */
static FILE *txfd = NULL;       /* Handle of file being sent           */
static FILE *rxfd = NULL;       /* Handle of file being recv           */
static word rxpktlen;           /* length of last packet               */
static word rxblklen;           /* len of last good data blk           */
static int txstate, rxstate;    /* xmit/recv states                    */
static long txpos, rxpos;       /* current position in files           */
static word txblklen;           /* length of last block sent           */
static word txmaxblklen;        /* max block length allowed            */
static long txlastack;          /* last dataack received               */
static int txstart, rxstart;    /* flags                               */
static long txoffset, rxoffset; /* offset in file we begun             */
static h_timer txtimer, rxtimer;  /* retry timers                      */
static word txretries, rxretries;  /* retry counters                   */
static long rxlastsync;         /* filepos last sync retry             */
static long txsyncid, rxsyncid; /* id of last resync                   */
static word txgoodneeded;       /* to send before larger blk           */
static word txgoodbytes;        /* no. sent at this blk size           */

struct _h_flags
{
  char *str;
  ULONG val;
};

static struct _h_flags h_flags[] =
{
  {"XON", HOPT_XONXOFF},
  {"TLN", HOPT_TELENET},
  {"CTL", HOPT_CTLCHRS},
  {"HIC", HOPT_HIGHCTL},
  {"HI8", HOPT_HIGHBIT},
  {"BRK", HOPT_CANBRK},
  {"ASC", HOPT_CANASC},
  {"UUE", HOPT_CANUUE},
  {"C32", HOPT_CRC32},
  {"DEV", HOPT_DEVICE},
  {"FPT", HOPT_FPT},
  {NULL, 0x0L}
};


/* ----------------------------------------------------------------------- */

static char *LOCALFUNC
h_revdate (long revstamp)
{
  static char buf[12];
  struct tm *t;
  time_t tt;

  /* note, this is a kludge that assumes that the C compiler's time_t
   * is represented as "unix time".  It should really be replaced by
   * a routine to break the unix time into y, m + d */
  tt = (time_t) revstamp;
  t = unix_localtime (&tt);
  sprintf (buf, "%02d %s %d",
           t->tm_mday, mtext[t->tm_mon],
           t->tm_year + ((t->tm_year < 70) ? 2000 : 1900));

  return (buf);
}                               /*h_revdate() */

#ifdef HYDRADEV

/* ------------------------------------------------------------------------- */

static void
hydra_msgdev (byte * data, word len)
{                               /* text is already NUL terminated by calling func hydra_devrecv() */
  ULONG remote_moh;

  len = len;
  status_line ("*HMSGDEV: %s", data);
  if (strncmp ((char *) data, "Remote has ", 11) == 0)  /* AG 990116 */
  {
    remote_moh = atoi ((char *) &(data[11]));
    if (remote_moh >= RxStats.tot_moh)
      RxStats.tot_moh = remote_moh;
    status_line (MSG_TXT (M_REMOTE_MOH), RxStats.tot_moh);
  }
}                               /*hydra_msgdev() */

static void
hydra_con (byte * data, word len)
{                               /* text is already NUL terminated by calling func hydra_devrecv() */
  len = len;
  chat_put_remote ((char *) data);
}                               /*hydra_con() */

/* ------------------------------------------------------------------------- */

static int devtxstate;          /* dev xmit state            */
static h_timer devtxtimer;      /* dev xmit retry timer      */
static word devtxretries;       /* dev xmit retry counter    */
static long devtxid, devrxid;   /* id of last devdata pkt    */
static char devtxdev[H_FLAGLEN + 1];  /* xmit device ident flag    */
static byte *devtxbuf;          /* ptr to usersupplied dbuf  */
static word devtxlen;           /* len of data in xmit buf   */

struct _h_dev
{
  char *dev;
  void (*func) (byte * data, word len);
};

static struct _h_dev h_dev[] =
{
  {"MSG", hydra_msgdev},        /* internal protocol msg     */
  {"CON", hydra_con},           /* text to console (chat)    */
  {"PRN", NULL},                /* data to printer           */
  {"ERR", NULL},                /* text to error output      */
  {NULL, NULL}
};

/* ------------------------------------------------------------------------- */

BOOL
hydra_devfree (void)
{
  if (devtxstate || !(txoptions & HOPT_DEVICE) || txstate >= HTX_END)
    return (FALSE);             /* busy or not allowed       */
  else
    return (TRUE);              /* allowed to send a new pkt */
}                               /*hydra_devfree() */

/* ------------------------------------------------------------------------- */

BOOL
hydra_devsend (char *dev, byte * data, word len)
{
  if (!dev || !data || !len || !hydra_devfree ())
    return (FALSE);

  strncpy (devtxdev, dev, H_FLAGLEN);
  devtxdev[H_FLAGLEN] = '\0';
  strupr (devtxdev);
  devtxbuf = data;
  devtxlen = (len > H_MAXBLKLEN) ? H_MAXBLKLEN : len;

  devtxid++;
  devtxtimer = h_timer_reset ();
  devtxretries = 0;
  devtxstate = HTD_DATA;

  /* special for chat, only prolong life if our side keeps typing! */
  if (chattimer > 0L && !strcmp (devtxdev, "CON") && txstate == HTX_REND)
    braindead = h_timer_set (H_BRAINDEAD);

  return (TRUE);
}                               /*hydra_devsend() */

/* ------------------------------------------------------------------------- */

BOOL
hydra_devfunc (char *dev, void (*func) (byte * data, word len))
{
  register int i;

  for (i = 0; h_dev[i].dev; i++)
  {
    if (!strnicmp (dev, h_dev[i].dev, H_FLAGLEN))
    {
      h_dev[i].func = func;
      return (TRUE);
    }
  }

  return (FALSE);
}                               /*hydra_devfunc() */

/* ------------------------------------------------------------------------- */

static void
hydra_devrecv (void)
{
  register char *p = (char *) rxbuf;
  register int i;
  word len = rxpktlen;

  p += (int) sizeof (long);     /* skip the id long  */
  len -= (int) sizeof (long);

  for (i = 0; h_dev[i].dev; i++)
  {                             /* walk through devs */
    if (!strncmp (p, h_dev[i].dev, H_FLAGLEN))
    {
      if (h_dev[i].func)
      {
        len -= ((int) strlen (p)) + 1;  /* sub devstr len         */
        p += ((int) strlen (p)) + 1;  /* skip devtag              */
        p[len] = '\0';          /* NUL terminate                  */
        (*h_dev[i].func) ((byte *) p, len);  /* call output func  */
      }
      break;
    }
  }
}                               /*hydra_devrecv() */

#endif

#if defined(OS_2) | defined(_WIN32) | defined(__unix__)

static void LOCALFUNC
MakeShortName (char *Name, char *ShortName)
{
  int r1 = 0, r2 = 0, r3 = 0;

  if (strchr (Name, '.') == NULL)
  {
    strncpy (ShortName, Name, 8);
    ShortName[8] = 0;
  }
  else
  {
    while (r1 < 8 && Name[r1] != '.' && Name[r1] != '\0')
      ShortName[r2++] = Name[r1++];

    ShortName[r2++] = '.';

    r1 = strlen (Name) - 1;

    ShortName[r2] = '\0';

    while (r1 > 0 && Name[r1] != '.' && Name[r1] != '\0' && r3 < 3)
    {
      r1--;
      r3++;
    }

    r1++;

    strcat (ShortName, &Name[r1]);
  }

  strlwr (ShortName);
}

#endif

/* ------------------------------------------------------------------------- */

static int LOCALFUNC
xfer_init (char *fname, long fsize, long ftime)
{
  int i;
  char namebuf[PATHLEN];

  Resume_WaZOO = 0;

  strcpy (rxpathname, remote_capabilities ? CURRENT.sc_Inbound : download_path);
  strcat (rxpathname, fname);

  /* ------------------------------------------------------------------ */
  /* Save info on WaZOO transfer in case of abort                       */
  /* ------------------------------------------------------------------ */
  if (remote_capabilities)
  {
    strcpy (Resume_name, fname);
    sprintf (Resume_info, "%ld %lo", fsize, ftime);
  }

  /* ------------------------------------------------------------------ */
  /* Check if this is a failed WaZOO transfer which should be resumed   */
  /* ------------------------------------------------------------------ */
  if (remote_capabilities && dexists (Abortlog_name))
  {
    Resume_WaZOO = (byte) check_failed (Abortlog_name, fname, Resume_info, namebuf);
  }

  if (Resume_WaZOO)
  {
    strcpy (rxpathname, CURRENT.sc_Inbound);
    strcat (rxpathname, namebuf);
  }
  else
  {
    if (dexists (rxpathname))
    {
      struct stat f;

      unix_stat_noshift (rxpathname, &f);
      if (fsize == f.st_size && ftime == f.st_mtime)
        return (FALSE);         /* already have file */
      else
      {
        i = strlen (rxpathname) - 1;
        if ((!overwrite) || (is_arcmail (rxpathname, i)))
        {
          unique_name (rxpathname);
        }
        else
        {
          unlink (rxpathname);
        }
      }
    }                           /* if exist */
  }                             /* Resume_WaZOO */

  return (TRUE);
}                               /*xfer_init() */

/* ------------------------------------------------------------------------- */

static int LOCALFUNC
xfer_okay (void)
{
  char new_pathname[PATHLEN];   /* TJW 960526 static -> dynamic */
  char *p;

  remove_abort (Abortlog_name, Resume_name);
  strcpy (new_pathname, CURRENT.sc_Inbound);
  p = new_pathname + ((int) strlen (new_pathname));  /* start of fname */
  strcat (new_pathname, Resume_name);  /* add real fname */
  unique_name (new_pathname);   /* make it unique */
  if (rename (rxpathname, new_pathname))  /* rename temp to real */
    status_line (MSG_TXT (M_COULD_NOT_RENAME), rxpathname, new_pathname);
  strcpy (rxpathname, new_pathname);
  return (stricmp (p, Resume_name));  /* dup rename? */
}

/* ------------------------------------------------------------------------- */

static void LOCALFUNC
put_flags (char *buf, struct _h_flags flags[], long val)
{
  register char *p;
  register int i;

  p = buf;
  for (i = 0; flags[i].val; i++)
  {
    if (val & flags[i].val)
    {
      if (p > buf)
        *p++ = ',';
      strcpy (p, flags[i].str);
      p += H_FLAGLEN;
    }
  }
  *p = '\0';
}                               /*put_flags() */

/* ------------------------------------------------------------------------- */

static ULONG LOCALFUNC
get_flags (char *buf, struct _h_flags flags[])
{
  register ULONG val;
  register char *p;
  register int i;

  val = 0x0L;
  for (p = strtok (buf, ","); p; p = strtok (NULL, ","))
  {
    for (i = 0; flags[i].val; i++)
    {
      if (!strcmp (p, flags[i].str))
      {
        val |= flags[i].val;
        break;
      }
    }
  }
  return (val);
}                               /*get_flags() */

/* ------------------------------------------------------------------------- */
/* CRC-16/32 code now separate source  / AGL:10mar93                         */
/* ------------------------------------------------------------------------- */

INLINEFUNC byte *LOCALFUNC
put_binbyte (register byte * p, register byte c)
{
  register byte n;

  n = c;
  if (txoptions & HOPT_HIGHCTL)
    n &= 0x7f;

  if (n == H_DLE ||
      ((txoptions & HOPT_XONXOFF) && (n == XON || n == XOFF)) ||
      ((txoptions & HOPT_TELENET) && n == '\r' && txlastc == '@') ||
      ((txoptions & HOPT_CTLCHRS) && (n < 32 || n == 127)))
  {
    *p++ = H_DLE;
    c ^= 0x40;
  }

  *p++ = c;
  txlastc = n;

  return (p);
}                               /*put_binbyte() */

/* ------------------------------------------------------------------------- */

static void LOCALFUNC
txpkt (register word len, int type)
{
  register byte *in, *out;

#ifdef HYDRA_UUE

  register word c, n;

#endif

  BOOL crc32 = FALSE;
  byte format;
  static char hexdigit[] = "0123456789abcdef";

  txbufin[len++] = (byte) type;

  switch (type)
  {
  case HPKT_START:
  case HPKT_INIT:
  case HPKT_INITACK:
  case HPKT_END:
  case HPKT_IDLE:
    format = HCHR_HEXPKT;
    break;

  default:
    /* COULD do smart format selection depending on data and options! */
    if (txoptions & HOPT_HIGHBIT)
    {
      if ((txoptions & HOPT_CTLCHRS) && (txoptions & HOPT_CANUUE))
        format = HCHR_UUEPKT;
      else if (txoptions & HOPT_CANASC)
        format = HCHR_ASCPKT;
      else
        format = HCHR_HEXPKT;
    }
    else
      format = HCHR_BINPKT;
    break;
  }

  if (format != HCHR_HEXPKT && (txoptions & HOPT_CRC32))
    crc32 = TRUE;

#ifdef DEBUG
  if (debugging_log)
  {
    char *s1, *s2, *s3, *s4;

    status_line (">-> PKT (format='%c'  type='%c'  crc=%d  len=%d)",
                 (char) format, (char) type, crc32 ? 32 : 16, (int) len - 1);

    switch (type)
    {
    case HPKT_START:
      status_line (">   <autostr>START");
      break;

    case HPKT_INIT:
      s1 = ((char *) txbufin) + ((int) strlen ((char *) txbufin)) + 1;
      s2 = s1 + ((int) strlen (s1)) + 1;
      s3 = s2 + ((int) strlen (s2)) + 1;
      s4 = s3 + ((int) strlen (s3)) + 1;
      status_line (">   INIT (appinfo='%s'  can='%s'  want='%s'  options='%s'  pktprefix='%s')",
                   (char *) txbufin, s1, s2, s3, s4);
      break;

    case HPKT_INITACK:
      status_line (">   INITACK");
      break;

    case HPKT_FINFO:
      status_line (">   FINFO (%s)", txbufin);
      break;

    case HPKT_FINFOACK:
      if (rxfd != NULL)
      {
        if (rxpos > 0L)
          s1 = "RES";
        else
          s1 = "BOF";
      }
      else if (rxpos == -1L)
        s1 = "HAVE";
      else if (rxpos == -2L)
        s1 = "SKIP";
      else
        s1 = "EOB";
      status_line (">   FINFOACK (pos=%ld %s  rxstate=%d  rxfd=%d)",
                   rxpos, s1, (int) rxstate, fileno (rxfd));
      break;

    case HPKT_DATA:
      status_line (">   DATA (ofs=%ld  len=%d)",
                   intell (h_long1 (txbufin)), (int) len - 5);
      break;

    case HPKT_DATAACK:
      status_line (">   DATAACK (ofs=%ld)",
                   intell (h_long1 (txbufin)));
      break;

    case HPKT_RPOS:
      status_line (">   RPOS (pos=%ld%s  blklen=%ld  syncid=%ld)",
                   rxpos, rxpos < 0L ? " SKIP" : "",
                   intell (h_long2 (txbufin)), rxsyncid);
      break;

    case HPKT_EOF:
      status_line (">   EOF (ofs=%ld%s)",
                   txpos, txpos < 0L ? " SKIP" : "");
      break;

    case HPKT_EOFACK:
      status_line (">   EOFACK");
      break;

    case HPKT_IDLE:
      status_line (">   IDLE");
      break;

    case HPKT_END:
      status_line (">   END");
      break;

#ifdef HYDRADEV
    case HPKT_DEVDATA:
      status_line (">   DEVDATA (id=%ld  dev='%s'  len=%hu)",
                   devtxid, devtxdev, devtxlen);
      break;

    case HPKT_DEVDACK:
      status_line (">   DEVDACK (id=%ld)",
                   intell (h_long1 (rxbuf)));
      break;
#endif

    default:                   /* This couldn't possibly happen! ;-) */
      break;
    }
  }
#endif

  if (crc32)
  {
    ULONG crc = CRC32POST (crc32block (crc32tab, CRC32INIT, txbufin, len));  /*AGL:10mar93 */

    txbufin[len++] = (byte) crc;
    txbufin[len++] = (byte) (crc >> 8);
    txbufin[len++] = (byte) (crc >> 16);
    txbufin[len++] = (byte) (crc >> 24);
  }
  else
  {
    word crc = CRC16POST (crc16block (crc16tab, CRC16INIT, txbufin, len));  /*AGL:10mar93 */

    txbufin[len++] = (byte) crc;
    txbufin[len++] = (byte) (crc >> 8);
  }

  in = txbufin;
  out = txbuf;
  txlastc = 0;
  *out++ = H_DLE;
  *out++ = format;

  switch (format)
  {
  case HCHR_HEXPKT:
    for (; len > 0; len--, in++)
    {
      if (*in & 0x80)
      {
        *out++ = '\\';
        *out++ = hexdigit[((*in) >> 4) & 0x0f];
        *out++ = hexdigit[(*in) & 0x0f];
      }
      else if (*in < 32 || *in == 127)
      {
        *out++ = H_DLE;
        *out++ = (byte) ((*in) ^ 0x40);
      }
      else if (*in == '\\')
      {
        *out++ = '\\';
        *out++ = '\\';
      }
      else
        *out++ = *in;
    }
    break;

  case HCHR_BINPKT:
    for (; len > 0; len--)
      out = put_binbyte (out, *in++);
    break;

  case HCHR_ASCPKT:
#ifdef HYDRA_UUE
    for (n = c = 0; len > 0; len--)
    {
      c |= (word) ((*in++) << n);
      out = put_binbyte (out, (byte) (c & 0x7f));
      c >>= 7;
      if (++n >= 7)
      {
        out = put_binbyte (out, (byte) (c & 0x7f));
        n = c = 0;
      }
    }
    if (n > 0)
      out = put_binbyte (out, (byte) (c & 0x7f));
#endif
    break;

  case HCHR_UUEPKT:
#ifdef HYDRA_UUE
    for (; len >= 3; in += 3, len -= 3)
    {
      *out++ = (byte) h_uuenc (in[0] >> 2);
      *out++ = (byte) h_uuenc (((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0x0f));
      *out++ = (byte) h_uuenc (((in[1] << 2) & 0x3c) | ((in[2] >> 6) & 0x03));
      *out++ = (byte) h_uuenc (in[2] & 0x3f);
    }
    if (len > 0)
    {
      *out++ = (byte) h_uuenc (in[0] >> 2);
      *out++ = (byte) h_uuenc (((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0x0f));
      if (len == 2)
        *out++ = (byte) h_uuenc ((in[1] << 2) & 0x3c);
    }
#endif
    break;
  }

  *out++ = H_DLE;
  *out++ = HCHR_PKTEND;

  if (type != HPKT_DATA && format != HCHR_BINPKT)
  {
    *out++ = '\r';
    *out++ = '\n';
  }

  for (in = (byte *) txpktprefix; *in; in++)
  {
    switch (*in)
    {
    case 221:                  /* transmit break signal for one second */
      do_break (TRUE);
      {
        h_timer t = h_timer_set (2);

        while (!h_timer_expired (t, h_timer_get ()))
          time_release ();
      }
      do_break (FALSE);
      break;

    case 222:
      {
        h_timer t = h_timer_set (2);

        while (!h_timer_expired (t, h_timer_get ()))
          time_release ();
      }
      break;

    case 223:
      SENDBYTE (0);
      break;

    default:
      SENDBYTE (*in);
      break;
    }
  }

  ComTXBlockTimeout ((BYTE *) txbuf, (USHORT) (out - txbuf), braindead);
}                               /*txpkt() */

/* ------------------------------------------------------------------------- */

static int LOCALFUNC
rxpkt (void)
{
  register byte *p, *q = rxbuf;
  register word n, i;
  short c;
  h_timer tnow = h_timer_get ();  /*AGL:16jul93 */

  if (keyabort ())
    return (H_SYSABORT);

  if (!CARRIER)
    return (H_CARRIER);

  if (h_timer_running (braindead) && h_timer_expired (braindead, tnow))
  {
#ifdef DEBUG
    if (debugging_log)
      status_line (" <- BrainDead (timer=%08lx  time=%08lx)", braindead, tnow);
#endif
    return (H_BRAINTIME);
  }

  if (h_timer_running (txtimer) && h_timer_expired (txtimer, tnow))
  {
#ifdef DEBUG
    if (debugging_log)
      status_line (" <- TxTimer (timer=%08lx  time=%08lx)", txtimer, tnow);
#endif
    return (H_TXTIME);
  }

#ifdef HYDRADEV
  if (h_timer_running (devtxtimer) && h_timer_expired (devtxtimer, tnow))
  {
#ifdef DEBUG
    if (debugging_log)
      status_line (" <- DevTxTimer (timer=%08lx  time=%08lx)", devtxtimer, unix_time (NULL));
#endif
    return (H_DEVTXTIME);
  }
#endif

  p = rxbufptr;

  while (CHAR_AVAIL ())
  {
    c = MODEM_IN ();
    if (rxoptions & HOPT_HIGHBIT)
      c &= 0x7f;

    n = c;
    if (rxoptions & HOPT_HIGHCTL)
      n &= 0x7f;

    if (n != H_DLE &&
        (((rxoptions & HOPT_XONXOFF) && (n == XON || n == XOFF)) ||
         ((rxoptions & HOPT_CTLCHRS) && (n < 32 || n == 127))))
      continue;

    if (rxdle || c == H_DLE)
    {
      switch (c)
      {
      case H_DLE:
        if (++rxdle >= 5)
          return (H_CANCEL);
        break;

      case HCHR_PKTEND:
        rxbufptr = p;

        if (p == NULL)
          break;

        switch (rxpktformat)
        {
        case HCHR_BINPKT:
          q = rxbufptr;
          break;

        case HCHR_HEXPKT:
          for (p = q = rxbuf; p < rxbufptr; p++)
          {
            if (*p == '\\' && *++p != '\\')
            {
              i = *p;
              n = *++p;
              if ((i -= '0') > 9)
                i -= ('a' - ':');
              if ((n -= '0') > 9)
                n -= ('a' - ':');
              if ((i & ~0x0f) || (n & ~0x0f))
              {
                c = H_NOPKT;
                break;
              }
              *q++ = (byte) ((i << 4) | n);
            }
            else
              *q++ = *p;
          }
          if (p > rxbufptr)
            c = H_NOPKT;
          break;

        case HCHR_ASCPKT:
#ifdef HYDRA_UUE
          n = i = 0;
          for (p = q = rxbuf; p < rxbufptr; p++)
          {
            i |= ((*p & 0x7f) << n);
            if ((n += 7) >= 8)
            {
              *q++ = (byte) (i & 0xff);
              i >>= 8;
              n -= 8;
            }
          }
#else
          c = H_NOPKT;
#endif
          break;

        case HCHR_UUEPKT:
#ifdef HYDRA_UUE
          n = (int) (rxbufptr - rxbuf);
          for (p = q = rxbuf; n >= 4; n -= 4, p += 4)
          {
            if (p[0] <= ' ' || p[0] >= 'a' ||
                p[1] <= ' ' || p[1] >= 'a' ||
                p[2] <= ' ' || p[2] >= 'a' ||
                p[3] <= ' ' || p[3] >= 'a')
            {
              c = H_NOPKT;
              break;
            }

            *q++ = (byte) ((h_uudec (p[0]) << 2) | (h_uudec (p[1]) >> 4));
            *q++ = (byte) ((h_uudec (p[1]) << 4) | (h_uudec (p[2]) >> 2));
            *q++ = (byte) ((h_uudec (p[2]) << 6) | h_uudec (p[3]));
          }

          if (n >= 2)
          {
            if (p[0] <= ' ' || p[0] >= 'a' ||
                p[1] <= ' ' || p[1] >= 'a')
            {
              c = H_NOPKT;
              break;
            }

            *q++ = (byte) ((h_uudec (p[0]) << 2) | (h_uudec (p[1]) >> 4));

            if (n == 3)
            {
              if (p[2] <= ' ' || p[2] >= 'a')
              {
                c = H_NOPKT;
                break;
              }

              *q++ = (byte) ((h_uudec (p[1]) << 4) | (h_uudec (p[2]) >> 2));
            }
          }
#else
          c = H_NOPKT;
#endif
          break;

        default:               /* This'd mean internal fluke */
#ifdef DEBUG
          if (debugging_log)
          {
            status_line (" <- <PKTEND> (pktformat='%c' dec=%d hex=%02x) ??",
                  (char) rxpktformat, (int) rxpktformat, (int) rxpktformat);
          }
#endif
          c = H_NOPKT;
          break;
        }

        rxbufptr = NULL;

        if (c == H_NOPKT)
          break;

        rxpktlen = (word) (q - rxbuf);
        if (rxpktformat != HCHR_HEXPKT && (rxoptions & HOPT_CRC32))
        {
          if (rxpktlen < 5)
          {
            c = H_NOPKT;
            break;
          }

          n = h_crc32test (crc32block (crc32tab, CRC32INIT, rxbuf, rxpktlen));  /*AGL:10mar93 */
          rxpktlen -= (word) sizeof (long);  /* remove CRC-32 */
        }
        else
        {
          if (rxpktlen < 3)
          {
            c = H_NOPKT;
            break;
          }

          n = h_crc16test (crc16block (crc16tab, CRC16INIT, rxbuf, rxpktlen));  /*AGL:10mar93 */
          rxpktlen -= (word) sizeof (word);  /* remove CRC-16 */
        }

        rxpktlen--;             /* remove type  */

        if (n)
        {
#ifdef DEBUG
          if (debugging_log)
          {
            char *s1, *s2, *s3, *s4;

            status_line ("><- PKT (format='%c'  type='%c'  len=%hd)",
                         (char) rxpktformat, rxbuf[rxpktlen], rxpktlen);

            switch (rxbuf[rxpktlen])
            {
            case HPKT_START:
              status_line ("<   START");
              break;

            case HPKT_INIT:
              s1 = ((char *) rxbuf) + ((int) strlen ((char *) rxbuf)) + 1;
              s2 = s1 + ((int) strlen (s1)) + 1;
              s3 = s2 + ((int) strlen (s2)) + 1;
              s4 = s3 + ((int) strlen (s3)) + 1;
              status_line ("<   INIT (appinfo='%s'  can='%s'  want='%s'  options='%s'  pktprefix='%s')",
                           (char *) rxbuf, s1, s2, s3, s4);
              break;

            case HPKT_INITACK:
              status_line ("<   INITACK");
              break;

            case HPKT_FINFO:
              status_line ("<   FINFO ('%s'  rxstate=%d)", rxbuf, (int) rxstate);
              break;

            case HPKT_FINFOACK:
              status_line ("<   FINFOACK (pos=%ld  txstate=%d  txfd=%d)",
                    intell (h_long1 (rxbuf)), (int) txstate, fileno (txfd));
              break;

            case HPKT_DATA:
              status_line ("<   DATA (rxstate=%d  pos=%ld  len=%hu)",
                           (int) rxstate, intell (h_long1 (rxbuf)),
                           (word) (rxpktlen - ((int) sizeof (long))));

              break;

            case HPKT_DATAACK:
              status_line ("<   DATAACK (rxstate=%d  pos=%ld)",
                           (int) rxstate, intell (h_long1 (rxbuf)));
              break;

            case HPKT_RPOS:
              status_line ("<   RPOS (pos=%ld%s  blklen=%hu->%ld  syncid=%ld%s  txstate=%d  txfd=%d)",
                           intell (h_long1 (rxbuf)),
                           intell (h_long1 (rxbuf)) < 0L ? " SKIP" : "",
                           txblklen, intell (h_long2 (rxbuf)),
                           intell (h_long3 (rxbuf)),
                         intell (h_long3 (rxbuf)) == rxsyncid ? " DUP" : "",
                           (int) txstate, fileno (txfd));
              break;

            case HPKT_EOF:
              status_line ("<   EOF (rxstate=%d  pos=%ld%s)",
                           (int) rxstate, intell (h_long1 (rxbuf)),
                           intell (h_long1 (rxbuf)) < 0L ? " SKIP" : "");
              break;

            case HPKT_EOFACK:
              status_line ("<   EOFACK (txstate=%d)", (int) txstate);
              break;

            case HPKT_IDLE:
              status_line ("<   IDLE");
              break;

            case HPKT_END:
              status_line ("<   END");
              break;

#ifdef HYDRADEV
            case HPKT_DEVDATA:
              s1 = ((char *) rxbuf) + ((int) sizeof (long));

              status_line ("<   DEVDATA (id=%ld  dev=%s  len=%u",
                           intell (h_long1 (rxbuf)), s1,
                           (int) rxpktlen - (((int) sizeof (long)) + ((int) strlen (s1)) + 1));

              break;

            case HPKT_DEVDACK:
              status_line ("<   DEVDACK (devtxstate=%d  id=%ld)",
                           (int) devtxstate, intell (h_long1 (rxbuf)));
              break;
#endif
            default:
              status_line ("<   Unkown pkttype %d (txstate=%d  rxstate=%d)",
                       (int) rxbuf[rxpktlen], (int) txstate, (int) rxstate);
              break;
            }
          }
#endif
          return ((int) rxbuf[rxpktlen]);
        }                       /*goodpkt */

#ifdef DEBUG
        if (debugging_log)
          status_line (">Bad CRC (format='%c'  type='%c'  len=%d)",
                       (char) rxpktformat, rxbuf[rxpktlen], (int) rxpktlen);
#endif
        break;

      case HCHR_BINPKT:
      case HCHR_HEXPKT:
#ifdef HYDRA_UUE
      case HCHR_ASCPKT:
      case HCHR_UUEPKT:
#endif
#ifdef DEBUG
        if (debugging_log)
          status_line (" <- <PKTSTART> (pktformat='%c')", (char) c);
#endif
        rxpktformat = (byte) c;
        p = rxbufptr = rxbuf;
        rxdle = 0;
        break;

      default:
        if (p)
        {
          if (p < rxbufmax)
            *p++ = (byte) (c ^ 0x40);
          else
          {
#ifdef DEBUG
            if (debugging_log)
              status_line (" <- Pkt too long - discarded");
#endif
            p = NULL;
          }
        }

        rxdle = 0;
        break;
      }                         /* case */
    }
    else if (p)
    {
      /* TJW 960416 illegal memory access with  */
      /* too small buffer of orig. 2.60 version */
      if (p < rxbufmax)
        *p++ = (byte) c;
      else
      {
#ifdef DEBUG
        if (debugging_log)
          status_line (" <- Pkt too long - discarded");
#endif
        p = NULL;
      }
    }
  }

  rxbufptr = p;

  time_release ();
  return (H_NOPKT);
}                               /*rxpkt() */

/* ------------------------------------------------------------------------- */

static void LOCALFUNC
hydra_badxfer (void)
{
  if (rxfd != NULL)
  {
    fclose (rxfd);
    rxfd = NULL;

    if (remote_capabilities)
    {
      if (!Resume_WaZOO && ReqSkipFile != 1)
        add_abort (Abortlog_name, Resume_name, rxpathname, CURRENT.sc_Inbound, Resume_info);
    }
    else
      unlink (rxpathname);
  }
}                               /*hydra_badxfer() */

/* ------------------------------------------------------------------------- */

void
hydra_init (ULONG want_options)
{
  char *HoldName;

  status_line (">hydra_init(%08x)", want_options);

  txbuf = bufp->hydra._txbuf;   /* H_BUFLEN bytes size */
  txbufin = bufp->hydra._txbufin;  /* txbuf + ((H_MAXBLKLEN+H_OVERHEAD+5)*2) */
  rxbuf = bufp->hydra._rxbuf;   /* H_BUFLEN bytes size */
  rxbufmax = bufp->hydra._rxbuf + sizeof (bufp->hydra._rxbuf);  /* rxbuf + H_MAXPKTLEN */

  /* TJW 960522 thrown out dynamic allocation of crc16tab and crc32tab */
  /* tables are static now, init done once in hydracrcinit()           */

  batchesdone = 0;
  mail_finished = 1;

  originator = remote_capabilities ? (isOriginator ? TRUE : FALSE) : TRUE;

  HoldName = HoldAreaNameMunge (&called_addr);

  sprintf (Abortlog_name, "%s%s.z",  /* TJW 961007 was ".%s%s.Z\0" */
           HoldName, Hex_Addr_Str (&remote_addr));

  if (originator)
    hdxlink = FALSE;
  else
    hdxlink = !((janus_baud >= cur_baud.rate_value) || (janus_OK));

  options = (want_options & HCAN_OPTIONS) & ~HUNN_OPTIONS;

  timeout = (word) (40960L / cur_baud.rate_value);
  if (timeout < H_MINTIMER)
    timeout = H_MINTIMER;
  else if (timeout > H_MAXTIMER)
    timeout = H_MAXTIMER;

  if (cur_baud.rate_value >= 2400)
    txmaxblklen = H_MAXBLKLEN;  /* as in HydraCom 1.09d */
  else
  {
    txmaxblklen = (short) ((cur_baud.rate_value / 300) * 128);
    if (txmaxblklen < 256)
      txmaxblklen = 256;
  }

  rxblklen = txblklen = (cur_baud.rate_value < 2400U) ? 256 : 512;

  txgoodbytes = 0;
  txgoodneeded = txmaxblklen;   /*AGL:23feb93 */

  txstate = HTX_DONE;

  set_prior (PRIO_HYDRA);
  XON_DISABLE ();

  sb_show ();
}                               /*hydra_init() */

/* ------------------------------------------------------------------------- */

void
hydra_deinit (void)
{
  status_line (">hydra_deinit()");
  set_prior (PRIO_MODEM);
}                               /*hydra_deinit() */

/* ------------------------------------------------------------------------- */

int
hydra (char *txpathname, char *txalias)
{
  int res = 0;
  int pkttype;
  char *p, *q;
  int i, j;
  struct stat f;
  NODESTRUC *np;
  char junk[34];

  /* ----------------------------------------------------------------- */
  status_line (">hydra(%s,%s)", txpathname ? txpathname : "(null)",
               txalias ? txalias : "(null)");
  /* ----------------------------------------------------------------- */

  if (txstate == HTX_DONE)
  {
    txstate = HTX_START;
    txoptions = HTXI_OPTIONS;
    txpktprefix[0] = '\0';

    rxstate = HRX_INIT;
    rxoptions = HRXI_OPTIONS;
    rxfd = NULL;
    rxdle = 0;
    rxbufptr = NULL;
    rxtimer = h_timer_reset ();

#ifdef HYDRADEV
    devtxid = devrxid = 0L;
    devtxtimer = h_timer_reset ();
    devtxstate = HTD_DONE;
#endif

    braindead = h_timer_set (H_BRAINDEAD);
  }
  else
    txstate = HTX_FINFO;

  txtimer = h_timer_reset ();
  txretries = 0;

  /* ----------------------------------------------------------------- */
  if (txpathname)
  {
    unix_stat_noshift (txpathname, &f);
    txfsize = f.st_size;
    txftime = f.st_mtime;

    txfd = share_fopen (txpathname, read_binary, DENY_WRITE);
    if (txfd == NULL)
    {
      got_error (MSG_TXT (M_OPEN_MSG), txpathname);
      return (XFER_SKIP);
    }

    if (isatty (fileno (txfd)))
    {
      errno = 1;
      got_error (MSG_TXT (M_DEVICE_MSG), txpathname);
      fclose (txfd);
      txfd = NULL;
      return (XFER_SKIP);
    }

    f_strupr (txpathname);

    for (p = txpathname, q = txfname; *p; p++)
    {
      if (*q = *p, *p == '\\' || *p == ':' || *p == '/')
        q = txfname;
      else
        q++;
    }
    *q = '\0';

    if (txalias)
      strupr (txalias);

    txstart = 0;
    txsyncid = 0L;
  }
  else
  {
    txfd = NULL;
    strcpy (txfname, "");
  }

  /* ----------------------------------------------------------------- */
  do
  {
#ifdef HYDRADEV
    /* -------------------------------------------------------------- */
    switch (devtxstate)
    {
      /* ------------------------------------------------------- */
    case HTD_DATA:
      if (txstate > HTX_RINIT)
      {
        h_long1 (txbufin) = intell (devtxid);
        p = ((char *) txbufin) + ((int) sizeof (long));

        strcpy (p, devtxdev);
        p += H_FLAGLEN + 1;
        memcpy (p, devtxbuf, devtxlen);
        txpkt (((word) sizeof (long)) + H_FLAGLEN + 1 + devtxlen, HPKT_DEVDATA);

        devtxtimer = h_timer_set ((!rxstate && txstate == HTX_REND) ? timeout / 2 : timeout);  /*AGL:10mar93 */
        devtxstate = HTD_DACK;
      }
      break;

      /* ------------------------------------------------------- */
    default:
      break;

      /* ------------------------------------------------------- */
    }
#endif
    /* -------------------------------------------------------------- */
    if (ReqSkipFile)
      rxpos = -2L;
    switch (txstate)
    {
      /* ------------------------------------------------------- */
    case HTX_START:
      SENDCHARS (autostr, (int) strlen (autostr), 1);
      txpkt (0, HPKT_START);
      txtimer = h_timer_set (H_START);
      txstate = HTX_SWAIT;
      break;

      /* ------------------------------------------------------- */
    case HTX_INIT:
      p = (char *) txbufin;
      sprintf (p, "%08lx%s,%s %s", H_REVSTAMP, PRDCT_PRTY, PRDCT_VRSN, HC_OS);
      p += ((int) strlen (p)) + 1;  /* our app info & HYDRA rev. */
      put_flags (p, h_flags, HCAN_OPTIONS);  /* what we CAN  */
      p += ((int) strlen (p)) + 1;
      put_flags (p, h_flags, options);  /* what we WANT */
      p += ((int) strlen (p)) + 1;
      sprintf (p, "%08lx%08lx", /* TxRx windows */
               hydra_txwindow, hydra_rxwindow);
      p += ((int) strlen (p)) + 1;
      strcpy (p, pktprefix);    /* pkt prefix string we want */
      p += ((int) strlen (p)) + 1;

      txoptions = HTXI_OPTIONS;
      txpkt ((word) (((byte *) p) - txbufin), HPKT_INIT);
      txoptions = rxoptions;
      txtimer = h_timer_set (timeout / 2);
      txstate = HTX_INITACK;
      break;

      /* ------------------------------------------------------- */
    case HTX_FINFO:
      if (txfd != NULL)
      {
        if (!txretries)
        {
          if (txalias)
            strcpy (TxStats.fname, fname8p3 (txalias));
          else
            strcpy (TxStats.fname, fname8p3 (txfname));
          TxStats.FileLen = txfsize;
          css_upd ();
          strlwr (txfname);
        }
        sprintf ((char *) txbufin, "%08lx%08lx%08lx%08lx%08lx",
                 txftime, txfsize, 0L, 0L, 0L);
#if defined(OS_2) | defined(_WIN32) | defined(__unix__)
        MakeShortName (txalias ? txalias : txfname,
                       (char *) &txbufin[strlen ((char *) txbufin)]);
        strlwr ((char *) txbufin);
        strcpy ((char *) &txbufin[strlen ((char *) txbufin) + 1],
                txalias ? txalias : txfname);
        /* TJW 960924 long filename support */
#else
        strcat ((char *) txbufin, txalias ? txalias : txfname);
        strlwr (txbufin);
#endif
      }
      else
      {
        if (!txretries)
        {
#ifdef DEBUG
          status_line ("+HSEND: End of batch");
#endif
        }

        strcpy ((char *) txbufin, txfname);
      }

#if defined(OS_2) | defined(_WIN32) | defined(__unix__)
      {                         /* TJW 960924 short + long filename support */
        int len = strlen ((char *) txbufin) + 1;

        if (len > 1)
          len += strlen ((char *) txbufin + len) + 1;
        txpkt ((word) len, HPKT_FINFO);
      }
#else
      txpkt (((word) strlen ((char *) txbufin)) + 1, HPKT_FINFO);
#endif

      txtimer = h_timer_set (txretries ? timeout / 2 : timeout);
      txstate = HTX_FINFOACK;
      break;

      /* ------------------------------------------------------- */
    case HTX_XDATA:
      if (ComTXRemain () > txmaxblklen)
        break;

      if (txpos < 0L)
        i = -1;                 /* Skip */
      else
      {
        h_long1 (txbufin) = intell (txpos);
        i = fread (txbufin + ((int) sizeof (long)), sizeof (char), txblklen, txfd);

        if (ferror (txfd))
        {
          got_error (MSG_TXT (M_READ_MSG), txfname);
          fclose (txfd);
          txfd = NULL;
          txpos = -2L;          /* Skip */
        }
      }

      if (i > 0)
      {
        txpos += i;

        txpkt (((word) sizeof (long)) + i, HPKT_DATA);

        if (txblklen < txmaxblklen &&
            (txgoodbytes += (word) i) >= txgoodneeded)
        {
          txblklen <<= 1;
          if (txblklen >= txmaxblklen)
          {
            txblklen = txmaxblklen;
            txgoodneeded = 0;
          }
          txgoodbytes = 0;
        }

        if (txwindow && (txpos >= (txlastack + txwindow)))
        {
          txtimer = h_timer_set (txretries ? timeout / 2 : timeout);
          txstate = HTX_DATAACK;
        }

        TxStats.FilePos = txpos;
        css_upd ();
        break;
      }
      /* fallthrough to HTX_EOF */

      /* ------------------------------------------------------- */
    case HTX_EOF:
      h_long1 (txbufin) = intell (txpos);
      txpkt ((word) sizeof (long), HPKT_EOF);

      txtimer = h_timer_set (txretries ? timeout / 2 : timeout);
      txstate = HTX_EOFACK;

      break;

      /* ------------------------------------------------------- */
    case HTX_END:
      txpkt (0, HPKT_END);
      txpkt (0, HPKT_END);
      txtimer = h_timer_set (timeout / 2);
      txstate = HTX_ENDACK;
      break;

      /* ------------------------------------------------------- */
    default:
      break;

    }

    pkttype = rxpkt ();

    switch (pkttype)
    {
      /* ------------------------------------------------- */
    case H_CARRIER:
    case H_CANCEL:
    case H_SYSABORT:
    case H_BRAINTIME:
      switch (pkttype)
      {
      case H_CARRIER:
        status_line (GenericError, &(MSG_TXT (M_NO_CARRIER)[1]));
        break;

      case H_CANCEL:
        status_line (MSG_TXT (M_SESSION_ABORT));
        break;

      case H_SYSABORT:
        status_line (GenericError, &(MSG_TXT (M_KBD_MSG)[1]));
        break;

      case H_BRAINTIME:
        status_line (MSG_TXT (M_OTHER_DIED));
        break;
      }

      txstate = HTX_DONE;
      res = XFER_ABORT;
      RxStats.cur_errors++;
      TxStats.cur_errors++;
      break;

      /* ------------------------------------------------- */
    case H_TXTIME:
      if (txstate == HTX_XWAIT || txstate == HTX_REND)
      {
        txpkt (0, HPKT_IDLE);
        txtimer = h_timer_set (H_IDLE);
        break;
      }

      if (++txretries > H_RETRIES)
      {
        status_line (MSG_TXT (M_FUBAR_MSG));
        txstate = HTX_DONE;
        res = XFER_ABORT;
        TxStats.cur_errors++;
        break;
      }

      status_line ("!%s", MSG_TXT (M_TIMEOUT));
      css_upd ();
      TxStats.cur_errors++;
#ifdef DEBUG
      status_line ("-HSEND: Timeout - Retry %u", txretries);
#endif
      txtimer = h_timer_reset ();

      switch (txstate)
      {
      case HTX_SWAIT:
        txstate = HTX_START;
        break;

      case HTX_INITACK:
        txstate = HTX_INIT;
        break;

      case HTX_FINFOACK:
        txstate = HTX_FINFO;
        break;

      case HTX_DATAACK:
        txstate = HTX_XDATA;
        break;

      case HTX_EOFACK:
        txstate = HTX_EOF;
        break;

      case HTX_ENDACK:
        txstate = HTX_END;
        break;
      }
      break;

      /* ------------------------------------------------- */
#ifdef HYDRADEV
    case H_DEVTXTIME:
      if (++devtxretries > H_RETRIES)
      {
        status_line (MSG_TXT (M_FUBAR_MSG));
        txstate = HTX_DONE;
        res = XFER_ABORT;
        TxStats.cur_errors++;
        break;
      }

      status_line ("!%s", MSG_TXT (M_TIMEOUT));
      css_upd ();
      TxStats.cur_errors++;
#ifdef DEBUG
      status_line ("-HDEVTX: Timeout - Retry %u", devtxretries);
#endif
      devtxtimer = h_timer_reset ();
      devtxstate = HTD_DATA;
      break;
#endif

      /* ------------------------------------------------- */
    case HPKT_START:
      if (txstate == HTX_START || txstate == HTX_SWAIT)
      {
        txtimer = h_timer_reset ();
        txretries = 0;
        txstate = HTX_INIT;
        braindead = h_timer_set (H_BRAINDEAD);
      }
      break;

      /* ------------------------------------------------- */
    case HPKT_INIT:
      if (rxstate == HRX_INIT)
      {
        p = (char *) rxbuf;
        p += ((int) strlen (p)) + 1;
        q = p + ((int) strlen (p)) + 1;
        rxoptions = options | HUNN_OPTIONS;
        rxoptions |= get_flags (q, h_flags);
        rxoptions &= get_flags (p, h_flags);
        rxoptions &= HCAN_OPTIONS;

        if (rxoptions < (options & HNEC_OPTIONS))
        {
          status_line ("!HYDRA: Incompatible on this link");
          txstate = HTX_DONE;
          res = XFER_ABORT;
          RxStats.cur_errors++;
          TxStats.cur_errors++;
          break;
        }

        p = q + ((int) strlen (q)) + 1;
        rxwindow = txwindow = 0L;
        sscanf (p, "%08lx%08lx", &rxwindow, &txwindow);

        if (rxwindow < 0L)
          rxwindow = 0L;

        if (hydra_rxwindow &&
            (!rxwindow || hydra_rxwindow < rxwindow))
          rxwindow = hydra_rxwindow;

        if (txwindow < 0L)
          txwindow = 0L;

        if (hydra_txwindow &&
            (!txwindow || hydra_txwindow < txwindow))
          txwindow = hydra_txwindow;

        p += ((int) strlen (p)) + 1;
        strncpy (txpktprefix, p, H_PKTPREFIX);
        txpktprefix[H_PKTPREFIX] = '\0';

        if (!batchesdone)
        {
          long revstamp;

          p = (char *) rxbuf;
          sscanf (p, "%08lx", &revstamp);
          status_line (">HYDRA: Other's HydraRev=%s",
                       h_revdate (revstamp));
          p += 8;
          if ((q = strchr (p, ',')) != NULL)
            *q = ' ';
          if ((q = strchr (p, ',')) != NULL)
            *q = '/';
          status_line (">HYDRA: Other's App.Info '%s'", p);
          put_flags ((char *) rxbuf, h_flags, rxoptions);
          status_line (">HYDRA: Using link options '%s'", rxbuf);
          if (txwindow || rxwindow)
            status_line (">HYDRA: Window tx=%ld rx=%ld", txwindow, rxwindow);
        }

        chattimer = (rxoptions & HOPT_DEVICE) ? 0L : -2L;
        chatautostart ();       /* Don't try moving this to chat.c! */
        txoptions = rxoptions;
        rxstate = HRX_FINFO;

        /* AG 990119 in xHydra sessions only send in 2nd batch */
        if ((rxoptions & HOPT_DEVICE) && !(do_RH1hydra && !batchesdone))
        {
          if (HydraMessage)     /* mtt 970117 */
            hydra_devsend ("CON", (byte *) HydraMessage, (word) strlen (HydraMessage));

          /* AG 990117 */
          sprintf (junk, "Remote has %ld bytes for us", TxStats.tot_moh);
          hydra_devsend ("MSG", (byte *) & junk, (word) strlen (junk));
        }
      }

      txpkt (0, HPKT_INITACK);
      break;

      /* ------------------------------------------------- */
    case HPKT_INITACK:
      if (txstate == HTX_INIT || txstate == HTX_INITACK)
      {
        braindead = h_timer_set (H_BRAINDEAD);
        txtimer = h_timer_reset ();
        txretries = 0;
        txstate = HTX_RINIT;
      }
      break;

      /* ------------------------------------------------- */
    case HPKT_FINFO:
      if (rxstate == HRX_FINFO)
      {
        braindead = h_timer_set (H_BRAINDEAD);
        if (!rxbuf[0])
        {
#ifdef DEBUG
          status_line ("*HRECV: End of batch");
#endif
          rxpos = 0L;
          rxstate = HRX_DONE;
          batchesdone++;

          for (j = 0; j < num_rakas; j++)
            if ((np = QueryNodeStruct (&remote_akas[j], 0)) != NULL)
              if (np->SyncHydraSessions)
                SyncHydraSession = 1;
        }
        else
        {
          long diskfree;        /* 64Byte units free on disk */
          long dummy;

#if defined(OS_2) | defined(_WIN32) | defined(__unix__)
          long plen = rxpktlen - sizeof (long);
          long slen;

#endif
          rxfsize = rxftime = 0L;
          rxfname[0] = '\0';
          sscanf ((char *) rxbuf, "%08lx%08lx%08lx%08lx%08lx%s",
                  &rxftime, &rxfsize, &dummy, &dummy, &dummy, rxfname);
          strlwr (rxfname);

          i = strlen (rxfname) - 1;
          if ((i > 2) && (rxfname[i - 2] == 'r') &&
              (rxfname[i - 1] == 'e') && (rxfname[i] == 'q'))
          {
            sprintf (&rxfname[i - 1], "%02x", TaskNumber);
          }

#if defined(OS_2) | defined(_WIN32) | defined(__unix__)
          slen = strlen ((char *) rxbuf) + 1;  /* TJW 960924 long filename support */
          if (plen > slen)      /* is there anything MORE ? */
            if (strlen (((char *) rxbuf) + slen) > strlen (rxfname))
              strcpy (rxfname, ((char *) rxbuf) + slen);
#endif

          if (!xfer_init (rxfname, rxfsize, rxftime))  /* Already have file */
          {
            status_line (MSG_TXT (M_ALREADY_HAVE), rxpathname);
            rxpos = -1L;
          }
          else
          {
            diskfree = zfree (rxpathname);
            if (rxfsize / 64 + 160L > diskfree)
            {
              status_line (MSG_TXT (M_OUT_OF_DISK_SPACE));
              RxStats.cur_errors++;
              rxpos = -2L;
            }
            else
            {
              if (dexists (rxpathname))  /* Resuming? */
              {
                rxfd = fopen (rxpathname, read_binary_plus);
                if (rxfd == NULL)
                {
                  got_error (MSG_TXT (M_OPEN_MSG), rxpathname);
                  RxStats.cur_errors++;
                  rxpos = -2L;
                }
              }
              else
              {
                rxfd = fopen (rxpathname, write_binary_plus);
                if (rxfd == NULL)
                {
                  got_error (MSG_TXT (M_OPEN_MSG), rxpathname);
                  RxStats.cur_errors++;
                  rxpos = -2L;
                }
              }

              if (rxfd != NULL)
              {
                p = check_netfile (rxfname);
                status_line ("#%s %s %s", MSG_TXT (M_RECEIVING), (p) ? p : " ", rxfname);
                strcpy (RxStats.fname, fname8p3 (rxfname));
                RxStats.FileLen = rxfsize;
                css_upd ();

                if (fseek (rxfd, 0L, SEEK_END))
                {
                  got_error (MSG_TXT (M_SEEK_MSG), rxpathname);
                  RxStats.cur_errors++;
                  hydra_badxfer ();
                  rxpos = -2L;
                }
                else
                {
                  diskfree = zfree (rxpathname);  /*AGL:07jul93 */
                  rxoffset = rxpos = ftell (rxfd);
                  if (rxpos < 0L)
                  {
                    got_error (MSG_TXT (M_SEEK_MSG), rxfname);
                    RxStats.cur_errors++;
                    hydra_badxfer ();
                    rxpos = -2L;
                  }
                  else
                  {
                    if ((rxfsize - rxoffset) / 64 + 160L > diskfree)
                    {           /*AGL:07jul93 */
                      status_line (MSG_TXT (M_OUT_OF_DISK_SPACE));
                      RxStats.cur_errors++;
                      hydra_badxfer ();
                      rxpos = -2L;
                    }
                    else
                    {
                      rxstart = 0;
                      rxtimer = h_timer_reset ();
                      rxretries = 0;
                      rxlastsync = 0L;
                      rxsyncid = 0L;
                      RxStats.FilePos = rxpos;
                      css_upd ();
                      if (rxpos > 0L)
                      {
                        status_line (MSG_TXT (M_SYNCHRONIZING), rxpos);
                      }
                      rxstate = HRX_DATA;
                    }
                  }
                }
              }
            }
          }
        }
      }
      else if (rxstate == HRX_DONE)
        rxpos = (!rxbuf[0]) ? 0L : -2L;

      h_long1 (txbufin) = intell (rxpos);
      txpkt ((word) sizeof (long), HPKT_FINFOACK);

      if (!rxstart)
      {                         /* start rx throughput timer! */
        rxstart++;
        throughput (0, 0, 0L);
      }

      break;

      /* ------------------------------------------------- */
    case HPKT_FINFOACK:
      if (txstate == HTX_FINFO || txstate == HTX_FINFOACK)
      {
        braindead = h_timer_set (H_BRAINDEAD);
        txretries = 0;
        if (!txfname[0])
        {
          txtimer = h_timer_set (H_IDLE);
          txstate = HTX_REND;
        }
        else if (txfd != NULL)
        {
          long txstartpos;

          txtimer = h_timer_reset ();
          txstartpos = intell (h_long1 (rxbuf));
          if (txstartpos >= 0L)
          {
            txstart = 0;
            txpos = txstartpos;
            txoffset = txpos;
            txlastack = txpos;
            TxStats.FilePos = txpos;
            css_upd ();

            if (txpos > 0L)
            {
              status_line (MSG_TXT (M_SYNCHRONIZING), txpos);
              if (fseek (txfd, txpos, SEEK_SET))
              {
                got_error (MSG_TXT (M_SEEK_MSG), txfname);
                TxStats.cur_errors++;
                fclose (txfd);
                txfd = NULL;
                txpos = -2L;
                txstate = HTX_EOF;
                break;
              }
            }

            if (!txstart)
            {                   /* start Tx throughput timer! */
              txstart++;
              throughput (0, 1, 0L);
            }
            txstate = HTX_XDATA;
          }
          else
          {
            fclose (txfd);
            txfd = NULL;

            if (txpos == -1L)
            {
              status_line (MSG_TXT (M_REMOTE_REFUSED), txfname);
              return (XFER_OK);
            }
            else
            {                   /* (txpos < -1L) file NOT sent */
              status_line ("+HSEND: Skipping %s", txfname);
              return (XFER_SKIP);
            }
          }
        }
      }
      break;

      /* ------------------------------------------------- */
    case HPKT_DATA:
      if (rxstate == HRX_DATA)
      {
        if (intell (h_long1 (rxbuf)) != rxpos ||
            intell (h_long1 (rxbuf)) < 0L)
        {
          if (intell (h_long1 (rxbuf)) <= rxlastsync)
          {
            rxtimer = h_timer_reset ();
            rxretries = 0;
          }

          rxlastsync = intell (h_long1 (rxbuf));

          if (!h_timer_running (rxtimer) ||
              h_timer_expired (rxtimer, h_timer_get ()))
          {
            if (rxretries > 4)
            {
              if (txstate < HTX_REND &&
                  !originator && !hdxlink)
              {
                hdxlink = TRUE;
                rxretries = 0;
              }
            }

            if (++rxretries > H_RETRIES)
            {
              status_line (MSG_TXT (M_FUBAR_MSG));
              RxStats.cur_errors++;
              txstate = HTX_DONE;
              res = XFER_ABORT;
              break;
            }

            if (rxretries == 1 || rxretries == 4)  /*AGL:14may93 */
              rxsyncid++;

            rxblklen /= (word) 2;
            i = rxblklen;
            if (i <= 64)
              i = 64;
            else if (i <= 128)
              i = 128;
            else if (i <= 256)
              i = 256;
            else if (i <= 512)
              i = 512;
            else
              i = 1024;

            if (!ReqSkipFile)
              status_line (MSG_TXT (M_J_BAD_PACKET), rxpos);
            RxStats.cur_errors++;

#ifdef DEBUG
            status_line ("-HRECV: Bad pkt at %ld - Retry %u (newblklen=%u)",
                         rxpos, rxretries, i);
#endif

            h_long1 (txbufin) = intell (rxpos);
            h_long2 (txbufin) = intell ((long) i);
            h_long3 (txbufin) = intell (rxsyncid);
            txpkt (3 * ((int) sizeof (long)), HPKT_RPOS);

            rxtimer = h_timer_set (timeout);
          }
        }
        else
        {
          braindead = h_timer_set (H_BRAINDEAD);
          rxpktlen -= (word) sizeof (long);

          rxblklen = rxpktlen;
          if (fwrite (rxbuf + ((int) sizeof (long)), sizeof (char), rxpktlen, rxfd) != rxpktlen)
          {
            got_error (MSG_TXT (M_WRITE_MSG), rxfname);
            RxStats.cur_errors++;
            hydra_badxfer ();
            rxpos = -2L;
            rxretries = 1;
            rxsyncid++;
            h_long1 (txbufin) = intell (rxpos);
            h_long2 (txbufin) = intell (0L);
            h_long3 (txbufin) = intell (rxsyncid);
            txpkt (3 * ((int) sizeof (long)), HPKT_RPOS);

            rxtimer = h_timer_set (timeout);
            break;
          }

          rxretries = 0;
          rxtimer = h_timer_reset ();
          rxlastsync = rxpos;
          rxpos += rxpktlen;
          if (rxwindow)
          {
            h_long1 (txbufin) = intell (rxpos);
            txpkt ((word) sizeof (long), HPKT_DATAACK);
          }

          RxStats.FilePos = rxpos;
          css_upd ();
        }                       /*badpkt */
      }                         /*rxstate==HRX_DATA */
      break;

      /* ------------------------------------------------- */
    case HPKT_DATAACK:
      if (txstate == HTX_XDATA || txstate == HTX_DATAACK ||
          txstate == HTX_XWAIT ||
          txstate == HTX_EOF || txstate == HTX_EOFACK)
      {
        if (txwindow && intell (h_long1 (rxbuf)) > txlastack)
        {
          txlastack = intell (h_long1 (rxbuf));
          if (txstate == HTX_DATAACK &&
              (txpos < (txlastack + txwindow)))
          {
            txstate = HTX_XDATA;
            txretries = 0;
            txtimer = h_timer_reset ();
          }
        }
      }
      break;

      /* ------------------------------------------------- */
    case HPKT_RPOS:
      if (txstate == HTX_XDATA || txstate == HTX_DATAACK ||
          txstate == HTX_XWAIT ||
          txstate == HTX_EOF || txstate == HTX_EOFACK)
      {
        if (intell (h_long3 (rxbuf)) != txsyncid)
        {
          txsyncid = intell (h_long3 (rxbuf));
          txretries = 1;
        }
        else
        {                       /*AGL:14may93 */
          if (++txretries > H_RETRIES)
          {
            status_line (MSG_TXT (M_FUBAR_MSG));
            txstate = HTX_DONE;
            res = XFER_ABORT;
            TxStats.cur_errors++;
            break;
          }
          if (txretries != 4)
            break;              /*AGL:14may93 */
        }

        txtimer = h_timer_reset ();
        txpos = intell (h_long1 (rxbuf));
        if (txpos < 0L)
        {
          if (txfd != NULL)
          {
            status_line ("+HSEND: Skipping %s", txfname);
            fclose (txfd);
            txfd = NULL;
            txstate = HTX_EOF;
          }
          txpos = -2L;
          break;
        }

        if (txblklen > (word) intell (h_long2 (rxbuf)))
          txblklen = (word) intell (h_long2 (rxbuf));
        else
          txblklen >>= 1;

        if (txblklen <= 64)
          txblklen = 64;
        else if (txblklen <= 128)
          txblklen = 128;
        else if (txblklen <= 256)
          txblklen = 256;
        else if (txblklen <= 512)
          txblklen = 512;
        else
          txblklen = 1024;

        txgoodbytes = 0;
        txgoodneeded += (word) (txmaxblklen * 2);  /*AGL:23feb93 */
        if (txgoodneeded > txmaxblklen * 8)  /*AGL:23feb93 */
          txgoodneeded = (word) (txmaxblklen * 8);  /*AGL:23feb93 */

        TxStats.FilePos = txpos;
        css_upd ();

        status_line (MSG_TXT (M_SYNCHRONIZING), txpos);
        if (fseek (txfd, txpos, SEEK_SET))
        {
          got_error (MSG_TXT (M_SEEK_MSG), txfname);
          TxStats.cur_errors++;
          fclose (txfd);
          txfd = NULL;
          txpos = -2L;
          txstate = HTX_EOF;
          break;
        }

        if (txstate != HTX_XWAIT)
          txstate = HTX_XDATA;
      }
      break;

      /* ------------------------------------------------- */
    case HPKT_EOF:
      if (rxstate == HRX_DATA)
      {
        if (intell (h_long1 (rxbuf)) < 0L)
        {
          hydra_badxfer ();
          RxStats.cur_errors++;
          status_line ("+HRECV: Skipping %s", rxfname);
          rxstate = HRX_FINFO;
          braindead = h_timer_set (H_BRAINDEAD);
          ReqSkipFile = 0;
        }
        else if (intell (h_long1 (rxbuf)) != rxpos)
        {
          if (intell (h_long1 (rxbuf)) <= rxlastsync)
          {
            rxtimer = h_timer_reset ();
            rxretries = 0;
          }

          rxlastsync = intell (h_long1 (rxbuf));

          if (!h_timer_running (rxtimer) ||
              h_timer_expired (rxtimer, h_timer_get ()))
          {
            if (++rxretries > H_RETRIES)
            {
              RxStats.cur_errors++;
              status_line (MSG_TXT (M_FUBAR_MSG));
              txstate = HTX_DONE;
              res = XFER_ABORT;
              break;
            }

            if (rxretries == 1 || rxretries == 4)  /*AGL:14may93 */
              rxsyncid++;

            rxblklen /= (word) 2;
            i = rxblklen;
            if (i <= 64)
              i = 64;
            else if (i <= 128)
              i = 128;
            else if (i <= 256)
              i = 256;
            else if (i <= 512)
              i = 512;
            else
              i = 1024;

            RxStats.cur_errors++;
            status_line ("-HRECV: Bad EOF at %ld - Retry %u (newblklen=%u)",
                         rxpos, rxretries, i);
            h_long1 (txbufin) = intell (rxpos);
            h_long2 (txbufin) = intell ((long) i);
            h_long3 (txbufin) = intell (rxsyncid);
            txpkt (3 * ((int) sizeof (long)), HPKT_RPOS);

            rxtimer = h_timer_set (timeout);
          }
        }
        else
        {
          long rxtime;

          fclose (rxfd);
          rxfd = NULL;

          if (Resume_WaZOO)     /* resumed transfer? */
          {
            if (xfer_okay ())
              status_line (MSG_TXT (M_RENAME_MSG), rxpathname);
          }

          if (rxftime > 0)      /* utime doesn't like negative numbers */
          {
            struct utimbuf utimes;

            utimes.UT_ACTIME = utimes.modtime = rxftime;
            unix_utime_noshift (rxpathname, (UTIMBUF *) & utimes);
          }

          /* stop rx throughput timer! */
          rxtime = throughput (1, 0, rxfsize) / PER_SECOND;
          rxfsize = rxpos - rxoffset;
          status_line ("%s-H%s %s", MSG_TXT (M_FILE_RECEIVED), (txoptions & HOPT_CRC32) ? "/32" : " ", rxpathname);
          update_files (0, rxpathname, rxfsize, rxtime, &(RxStats.cur_errors));

          RxStats.cur_mxfrd += rxfsize;
          RxStats.cur_fxfrd++;
          RxStats.FilePos = RxStats.FileLen = -1L;  /* invalidate! */
          css_upd ();

          rxstate = HRX_FINFO;
          braindead = h_timer_set (H_BRAINDEAD);
        }                       /*skip/badeof/eof */
      }                         /*rxstate==HRX_DATA */

      if (rxstate == HRX_FINFO)
        txpkt (0, HPKT_EOFACK);

      break;

      /* ------------------------------------------------- */
    case HPKT_EOFACK:
      if (txstate == HTX_EOF || txstate == HTX_EOFACK)
      {
        braindead = h_timer_set (H_BRAINDEAD);
        if (txfd != NULL)
        {
          long txtime;

          fclose (txfd);
          txfsize = txpos - txoffset;
          /* stop tx throughput timer! */
          txtime = throughput (1, 1, txfsize) / PER_SECOND;
          status_line ("%s-H%s %s", MSG_TXT (M_FILE_SENT), (txoptions & HOPT_CRC32) ? "/32" : " ", txpathname);
          update_files (1, txpathname, txfsize, txtime, &(TxStats.cur_errors));

          TxStats.cur_mxfrd += txfsize;
          TxStats.cur_fxfrd++;
          TxStats.FileLen = TxStats.FilePos = -1L;  /* invalidate! */
          css_upd ();

          return (XFER_OK);
        }
        else
          return (XFER_SKIP);
      }
      break;

      /* ------------------------------------------------- */
    case HPKT_IDLE:
      if (txstate == HTX_XWAIT)
      {
        hdxlink = FALSE;
        txtimer = h_timer_reset ();
        txretries = 0;
        txstate = HTX_XDATA;
      }
      else if (txstate >= HTX_FINFO && txstate < HTX_REND)
        braindead = h_timer_set (H_BRAINDEAD);
      break;

      /* ------------------------------------------------- */
    case HPKT_END:
      /* special for chat, other side wants to quit */
      if (chattimer > 0L && txstate == HTX_REND)
      {
        chattimer = -3L;
        break;
      }

      if (txstate == HTX_END || txstate == HTX_ENDACK)
      {
        txpkt (0, HPKT_END);
        txpkt (0, HPKT_END);
        txpkt (0, HPKT_END);
#ifdef DEBUG
        status_line ("+HYDRA: Completed");
#endif
        txstate = HTX_DONE;
        res = XFER_OK;
      }
      break;

#ifdef HYDRADEV
      /* ------------------------------------------------- */
    case HPKT_DEVDATA:
      if (devrxid != intell (h_long1 (rxbuf)))
      {
        hydra_devrecv ();
        devrxid = intell (h_long1 (rxbuf));
      }
      h_long1 (txbufin) = h_long1 (rxbuf);  /*AGL:10feb93 */
      txpkt ((word) sizeof (long), HPKT_DEVDACK);

      break;

      /* ------------------------------------------------- */
    case HPKT_DEVDACK:
      if (devtxstate && (devtxid == intell (h_long1 (rxbuf))))
      {
        devtxtimer = h_timer_reset ();
        devtxstate = HTD_DONE;
      }
      break;
#endif

      /* ------------------------------------------------- */
    default:                   /* unknown packet types: IGNORE, no error! */
      break;
    }                           /*(pkttype) */

    switch (txstate)
    {
      /* ------------------------------------------------- */
    case HTX_START:
    case HTX_SWAIT:
      if (rxstate == HRX_FINFO)
      {
        txtimer = h_timer_reset ();
        txretries = 0;
        txstate = HTX_INIT;
      }
      break;

      /* ------------------------------------------------- */
    case HTX_RINIT:
      if (rxstate == HRX_FINFO)
      {
        txtimer = h_timer_reset ();
        txretries = 0;
        txstate = HTX_FINFO;
      }
      break;

      /* ------------------------------------------------- */
    case HTX_XDATA:
      if (rxstate && hdxlink)
      {
        status_line (MSG_TXT (M_GOING_ONE_WAY));
#ifdef HYDRADEV
        hydra_devsend ("MSG", (byte *) hdxmsg, (int) strlen (hdxmsg));
#endif
        txtimer = h_timer_set (H_IDLE);
        txstate = HTX_XWAIT;
      }
      break;

      /* ------------------------------------------------- */
    case HTX_XWAIT:
      if (!rxstate)
      {
        txtimer = h_timer_reset ();
        txretries = 0;
        txstate = HTX_XDATA;
      }
      break;

      /* ------------------------------------------------- */
    case HTX_REND:
#ifdef HYDRADEV
      if (!rxstate && !devtxstate)
#else
      if (!rxstate)
#endif
      {
        /* special for chat, braindead will protect */
        if (chattimer > 0L)
          break;
        if (chattimer == 0L)
          chattimer = -3L;

        txtimer = h_timer_reset ();
        txretries = 0;
        txstate = HTX_END;
      }
      break;

      /* ------------------------------------------------- */
    default:                   /* any other state - nothing to do */
      break;

    }                           /*switch(txstate) */
  }
  while (txstate);

  if (txfd != NULL)
  {
    fclose (txfd);
    txfd = NULL;
  }

  hydra_badxfer ();

  if (res == XFER_ABORT)
  {
    CLEAR_OUTBOUND ();

    if (remote_capabilities)
      modem_hangup ();

    if (CARRIER)
    {
      braindead = h_timer_set (10);  /* wait max. 10s after abort */
      ComTXBlockTimeout ((BYTE *) abortstr, (int) strlen (abortstr), braindead);
      while (!OUT_EMPTY () && CARRIER && !h_timer_expired (braindead, h_timer_get ()))
        time_release ();

      CLEAR_OUTBOUND ();
    }

    CLEAR_INBOUND ();
    mail_finished = 0;
  }
  else
  {
    braindead = h_timer_set (10);  /* wait max. 10s after abort */
    while (!OUT_EMPTY () && CARRIER && !h_timer_expired (braindead, h_timer_get ()))
      time_release ();
  }

  // CEH 980803
  // if there are still errors counted in ?xStats.cur_errors (when a file has not
  // been received completely, so that update_file has not been called):

  TxStats.tot_errors += TxStats.cur_errors;
  RxStats.tot_errors += RxStats.cur_errors;
  hist.err_out += TxStats.cur_errors;
  hist.err_in += RxStats.cur_errors;
  TxStats.cur_errors = 0;
  RxStats.cur_errors = 0;

  return (res);
}                               /*hydra() */

void
hydracrcinit (void)
{
  crc16init (crc16tab, CRC16POLY);
  crc32tab = cr3tab;
}

/* $Id: hydra.c,v 1.11 1999/03/23 22:28:46 mr Exp $ */
