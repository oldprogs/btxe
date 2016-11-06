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
 * Filename    : $Source: E:/cvs/btxe/src/fossil.c,v $
 * Revision    : $Revision: 1.6 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:29 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm FOSSIL version 5 module
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#ifdef OS_2
// HJK - 99/01/24
//void
//fossil_ver (void)
//{
//}

#ifdef __EMX__
/* emx does not known _ungetch() */

static BOOL32 unget = FALSE;
static int unget_c = 0;

int
_ungetch (int c)
{
  unget = TRUE;
  unget_c = c;
  return 0;
}
#endif

/* get_key () - P. Fitzsimmons */
int
get_key (void)
{
  int c;

#ifdef __EMX__
  if (unget)
  {
    unget = FALSE;
    return unget_c;
  }
#endif
  c = getch ();

  if (!c || c == 0xE0)          /* OS/2 returns 0xE0 if one of the *
                                 * grey edit keys are hit          */
    c = getch () << 8;
  return (c);
}
#endif

#ifdef  DOS16                   /*  TS960928    */
int
get_key (void)
{
  int c = getch ();

  if (!c || c == 0xE0)
    c = getch () << 8;
  return (c);
}
#endif

#ifdef DOS16

#define sendbyte SENDBYTE
#define ComInit Cominit
#define mdm_disable MDM_DISABLE
#define mdm_enable MDM_ENABLE
#define modem_in MODEM_IN
#define modem_status MODEM_STATUS
#define clear_inbound CLEAR_INBOUND
#define sendchars SENDCHARS
#define clear_outbound CLEAR_OUTBOUND
#define buffer_byte BUFFER_BYTE
#define unbuffer_bytes UNBUFFER_BYTES
#define peekbyte PEEKBYTE

int Port = 0;

int FailSafeTimer = 0;

static void LOCALFUNC fill_buffer (void);
static USHORT ComOutSpace (void);

/*  TS 960722   fossil announcement */
#define ID_STR_LEN  80
void
fossil_announce (int fs)
{
  char trnstr[256] = "";
  char outstr[ID_STR_LEN + 1] = "";
  char id_str[ID_STR_LEN + 1] = "";
  char *fos_ans = NULL;
  char *fos_sig = NULL;
  char *fos_ver = NULL;
  char *p;

  /*  first we make a copy of the string already in fossil_info, since     */
  /*  we have to mangle with the string destructively                      */
  strncpy (id_str, fossil_info.id_string, ID_STR_LEN);
  id_str[ID_STR_LEN] = '\0';    /*  terminate the string for safety's sake */

  /*  we convert the string to upper case                                  */
  p = id_str;
  for (p = id_str; *p != '\0'; p++)
    *p = (char) toupper ((int) *p);

  /*  we have to copy the msg string first, since it will get destroyed    */
  strcpy (trnstr, MSG_TXT (M_FOS_TRANS));

  /*  we now parse the search/answer string pairs                          */
  if (NULL != (p = strtok (trnstr, " ,:")))
    while (1)
    {
      fos_sig = strstr (id_str, p);  /* get answer string */
      fos_ans = strtok (NULL, " ,:");
      if (fos_sig != NULL)      /* we found a signature */
        break;
      else
        p = strtok (NULL, " ,:");  /* get next search string */
      if (p == NULL)
        break;                  /* no more strings to parse */
    }

  /*  we terminate the signature                                           */
  p = fos_sig;
  while (isalnum (*p) || *p == '/' || *p == '!')
    p++;
  *p = '\0';

  /*  find the version and terminate it                                    */
  if (fos_sig != NULL)
  {
    p++;
    while (*p != '.')
      p++;
    fos_ver = p;
    fos_ver--;
    p++;
    while (isalnum (*p))
      p++;
    *p = '\0';
  }
  else
    fos_ver = NULL;

  /*  compose the string to display/log                                    */
  sprintf (outstr, "%s %s V%s (%i/%i)",
           MSG_TXT (M_FOSSIL_TYPE),
           fos_sig == NULL ? "?" : fos_ans,
           fos_ver == NULL ? "?" : fos_ver,
           fossil_info.ibufr,
           fossil_info.obufr);
  if (fs)
    status_line (outstr);
  else
  {
    p = strcat (outstr, "\r\n");
    p++;                        /* we skip the leading blank */
    scr_printf (p);
  }
}

void
fossil_ver ()
{
  union REGS r;
  struct SREGS s;

  char far *q;

  q = (char far *) &fossil_info;

  r.x.cx = sizeof (struct finfo);

  segread (&s);
  s.es = s.ds = FP_SEG (q);

  r.x.di = r.x.si = FP_OFF (q);

  r.x.dx = Port;
  r.x.ax = 0x1b00;

  int86x (0x14, &r, &r, &s);
}

void
com_kick ()
{
  XON_DISABLE ();               /* Uncork the transmitter */
  XON_ENABLE ();
}

unsigned int
Com_ (char request, byte parm1)
{
  union REGS r;

  r.h.ah = request;
  r.h.al = parm1;
  r.x.dx = Port;

  return int86 (0x14, &r, &r);
}

unsigned
ComInit (int WhichPort, int failsafe)
{
  union REGS r;
  struct SREGS s;
  char far *ptr = &ctrlc_ctr;
  int ret;

  fossil_fetch_pointer = fossil_buffer;
  fossil_count = 0;

  r.h.ah = 4;
  r.x.dx = (Port = WhichPort);
  r.x.bx = 0x4f50;

  segread (&s);

  s.es = s.ds = FP_SEG (ptr);
  r.x.cx = FP_OFF (ptr);

  ret = int86x (0x14, &r, &r, &s);

  if (ret == 0x1954)
  {
    FailSafeTimer = failsafe;
    r.x.ax = 0x0f01;
    r.x.dx = Port;
    int86 (0x14, &r, &r);
  }

  return (ret);
}

int
com_getc (int t)
{
  long t1;

  if (!CHAR_AVAIL ())
  {
    t1 = timerset ((unsigned int) (t * 100));
    while (!CHAR_AVAIL ())
    {
      if (timeup (t1))
      {
        return (EOF);
      }

      /*
       * This should work because we only do TIMED_READ when we have
       * carrier
       */
      if (!CARRIER)
      {
        return (EOF);
      }

      time_release ();
    }
  }

  return ((unsigned int) (MODEM_IN ()) & 0x00ff);
}

int
modem_in (void)
{
  unsigned char c;

  while (fossil_count == 0)
  {
    fill_buffer ();
    if (fossil_count == 0)
      time_release ();
  }

  --fossil_count;
  c = (unsigned char) *fossil_fetch_pointer++;
  return ((int) c);
}

int
peekbyte (void)
{
  unsigned char c;

  if (fossil_count == 0)
  {
    fill_buffer ();
    if (fossil_count == 0)
      return (-1);
  }

  c = (unsigned char) *fossil_fetch_pointer;
  return ((int) c);
}

static void LOCALFUNC
fill_buffer (void)
{
  union REGS r;
  int ret;

  if (fossil_count != 0)
    return;

  fossil_fetch_pointer = fossil_buffer;

  r.x.ax = 0x0c00;
  r.x.dx = Port;

  ret = int86 (0x14, &r, &r);

  if (ret != -1)
  {
    fossil_count = 1;
    *fossil_fetch_pointer = (char) ret;
    r.x.ax = 0x0200;
    int86 (0x14, &r, &r);
  }
}

void
clear_inbound (void)
{
  union REGS r;

  fossil_fetch_pointer = fossil_buffer;
  fossil_count = 0;

  r.x.ax = 0x0a00;
  r.x.dx = Port;

  int86 (0x14, &r, &r);
}

void
clear_outbound (void)
{
  union REGS r;

  out_send_pointer = out_buffer;
  out_count = 0;

  r.x.ax = 0x0900;
  r.x.dx = Port;

  int86 (0x14, &r, &r);
}

int
modem_status (void)
{
  union REGS r;
  int ret;

  r.x.ax = 0x0300;
  r.x.dx = Port;

  ret = int86 (0x14, &r, &r);

  if (fossil_count != 0)
    ret |= 256;

  return (ret);
}

void
do_break (int on_off)
{
  union REGS r;

  r.h.al = (char) on_off;
  r.h.ah = 0x1a;
  r.x.dx = Port;

  int86 (0x14, &r, &r);
}

void
mdm_enable (unsigned long mask)
{
  union REGS r;

  CLEAR_INBOUND ();

#ifdef EXTENDED_BAUDRATES
  if (extend_rates)
  {
    r.h.cl = (byte) mask;
    if (comm_bits == 3)
      r.h.ch = 3;
    else
      r.h.ch = 2;
    if (stop_bits == 4)
      r.h.bl = 1;
    else
      r.h.bl = 0;

    switch (parity)
    {
    case 0x08:                 /* ODD_PARITY */
      r.h.bh = 1;
      break;

    case 0x18:                 /* EVEN_PARITY */
      r.h.bh = 2;
      break;

    default:
      r.h.bh = 0;
      break;
    }

    r.h.ah = 0x1e;
    r.h.al = 0;
    r.x.dx = Port;

    int86 (0x14, &r, &r);
    return;
  }
#endif

  r.h.al = (byte) (mask | comm_bits | parity | stop_bits);
  r.h.ah = 0;
  r.x.dx = Port;

  int86 (0x14, &r, &r);
}

void
mdm_disable (void)
{
  union REGS r;

  fossil_fetch_pointer = fossil_buffer;
  fossil_count = 0;

  r.x.ax = 0x0500;
  r.x.dx = Port;

  int86 (0x14, &r, &r);
}

void
unbuffer_bytes (void)
{
  if (out_count == 0)
    return;

  SENDCHARS ((char far *) out_buffer, out_count, 1);

  out_send_pointer = out_buffer;
  out_count = 0;
}

void
buffer_byte (unsigned char c)
{
  if (out_count == 128)
    unbuffer_bytes ();

  out_count++;
  *out_send_pointer++ = c;
}

void
sendbyte (unsigned char c)
{
  union REGS r;
  int done = 0;

  if (out_count != 0)
    unbuffer_bytes ();

  while (done == 0)
  {
    r.h.ah = 0x0b;
    r.h.al = c;
    r.x.dx = Port;

    done = int86 (0x14, &r, &r);
    if (done == 0)
    {
      peekbyte ();              /* Waiting for output, try to get input */
      time_release ();
    }
  }
}

void
sendchars (char far * str, unsigned len, int dcd)
{
  union REGS r1, r2;
  struct SREGS s;
  long timer = 0L;
  int sent;

  r1.x.ax = 0x1900;
  r1.x.dx = Port;

  segread (&s);

  s.es = s.ds = FP_SEG (str);

  while (len != 0)
  {
    r1.x.cx = len;
    r1.x.si = r1.x.di = FP_OFF (str);
    sent = int86x (0x14, &r1, &r2, &s);
    if (sent == (int) len)
      return;

    len -= sent;
    str += sent;

    if (FailSafeTimer)
    {
      if (timer == 0L)
        timer = timerset (FailSafeTimer);
      else
      {
        if (timeup (timer))
        {
          modem_hangup ();
          return;
        }
      }
    }

    peekbyte ();                /* Waiting for output, try to get input */
    time_release ();
    if (dcd && !CARRIER)
      return;
  }
}

/* Currently only used by HYDRA */

/*
 * This is a lot like ComWrite, but will return the sent-character
 * count if either a timeout occurs or carrier is lost. The timer
 * is specified by passing in a time_t for end-of-time. If zero is
 * passed for the timer, we'll just check for carrier loss.
 *
 */

USHORT
ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer)
{
  USHORT cbRemaining = cbBuf;
  USHORT cbToSend;
  BOOL fTimedOut;

  fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  while (!fTimedOut && CARRIER && cbRemaining)
  {
    cbToSend = ComOutSpace ();
    if ((--cbToSend) > 0)
    {
      if (cbToSend > cbRemaining)
        cbToSend = cbRemaining;
      SENDCHARS ((char far *) lpBuf, cbToSend, 1);
      lpBuf += cbToSend;
      cbRemaining -= cbToSend;
    }
    else
      time_release ();
    fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  }
  return (cbBuf - cbRemaining);
}

USHORT
ComTXRemain (void)
{
  union REGS r1, r2;
  struct SREGS s;
  char far *q;

  q = (char far *) &fossil_info;

  r1.x.ax = 0x1b00;
  r1.x.cx = sizeof (FOSINFO);
  r1.x.dx = Port;
  segread (&s);
  s.es = s.ds = FP_SEG (q);
  r1.x.di = FP_OFF (q);
  int86x (0x14, &r1, &r2, &s);

  return (fossil_info.obufr - fossil_info.ofree);
}

static USHORT
ComOutSpace (void)
{
  union REGS r1, r2;
  struct SREGS s;
  char far *q;

  q = (char far *) &fossil_info;

  r1.x.ax = 0x1b00;
  r1.x.cx = sizeof (FOSINFO);
  r1.x.dx = Port;
  segread (&s);
  s.es = s.ds = FP_SEG (q);
  r1.x.di = FP_OFF (q);
  int86x (0x14, &r1, &r2, &s);

  return (fossil_info.ofree);
}


void
fossil_gotoxy (int col, int row)
{
  union REGS r;

  r.h.dh = (char) row;
  r.h.dl = (char) col;
  r.x.ax = 0x1100;

  int86 (0x14, &r, &r);
}

int
fossil_whereami (void)
{
  union REGS r;

  r.x.ax = 0x1200;
  int86 (0x14, &r, &r);
  return r.x.dx;
}

int
fossil_wherey (void)
{
  union REGS r;

  r.x.ax = 0x1200;
  int86 (0x14, &r, &r);
  return (int) r.h.dh;
}

int
fossil_wherex (void)
{
  union REGS r;

  r.x.ax = 0x1200;
  int86 (0x14, &r, &r);
  return (int) r.h.dl;
}

#endif

/* $Id: fossil.c,v 1.6 1999/03/22 03:47:29 mr Exp $ */
