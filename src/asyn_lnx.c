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
 * Filename    : $Source: E:/cvs/btxe/src/asyn_lnx.c,v $
 * Revision    : $Revision: 1.9 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/08/29 19:55:48 $
 * State       : $State: Exp $
 * Orig. Author: Ben Stuyts and Louis Lagendijk
 *
 * Description : Com-Module and other stuff for Linux
 *
 *---------------------------------------------------------------------------*/

/* Com-Module and other stuff for Linux */

#include "includes.h"

/*** Some buffering functions first ***/

#define BUFFERSIZE 0x0fff

typedef struct
{
  unsigned char buf[BUFFERSIZE + 1];
  int buf_rp;
  int buf_wp;
  int buf_used;
}
BufferType;

#define FREEINBUFFER(a) (((BUFFERSIZE+1-1+a.buf_rp-a.buf_wp)&BUFFERSIZE))
#define USEDINBUFFER(a) (((BUFFERSIZE+1  +a.buf_wp-a.buf_rp)&BUFFERSIZE))

static BufferType txBuf;
static BufferType rxBuf;

void
initBuffer (BufferType * buf)
{
  buf->buf_rp = 0;
  buf->buf_wp = 0;
}

unsigned char
getBuffer (BufferType * buf)
{
  unsigned char result;

  if (buf->buf_rp != buf->buf_wp)
  {
    result = buf->buf[buf->buf_rp];
    buf->buf_rp = ((buf->buf_rp + 1) & BUFFERSIZE);
    return (result);
  }
  else
  {
    status_line (">getBuffer: empty");
    return (0);
  }
}

unsigned char
peekBuffer (BufferType * buf)
{
  if (buf->buf_rp != buf->buf_wp)
  {
    return (buf->buf[buf->buf_rp]);
  }
  else
  {
    status_line (">peekBuffer: empty");
    return (0);
  }
}

void
putBuffer (BufferType * buf, unsigned char ch)
{
  if (((buf->buf_wp + 1) & BUFFERSIZE) != (buf->buf_rp))
  {
    buf->buf[buf->buf_wp] = ch;
    buf->buf_wp = (buf->buf_wp + 1) & BUFFERSIZE;
  }
  else
  {
    status_line (">putBuffer: full");
  }
}

/*** Com-Handling ***/

static int modemdf = -1;
static int portisopen = 0;
static char lockfilename[128];


unsigned
Cominit (int port, int failsafe)
{
  int fd, i, j, pid;
  struct termios tios;
  char buf[128];

  for (i = 0, j = -1; port_device[i]; i++)
    if (port_device[i] == '/')
      j = i;
  j++;
  strcpy (lockfilename, "/var/lock/");
  strcat (lockfilename, "LCK..");
  for (i = strlen (lockfilename); port_device[j]; j++, i++)
    lockfilename[i] = port_device[j];
  lockfilename[i] = 0;

  if (!portisopen)
  {

  retry_open:

    fd = open (lockfilename, O_CREAT | O_EXCL | O_RDWR, 0644);
    if (fd < 0)
    {
      if (errno == EEXIST)
      {

        fd = open (lockfilename, O_RDONLY);
        if (fd < 0)
        {
          printf ("ERR: could not open existing lockfile\n");
          return (0);
        }

        i = read (fd, buf, 127);
        if (i == 4)
        {
          pid = (int) buf;
        }
        else
        {
          buf[i] = 0;
          sscanf (buf, "%d", &pid);
        }                       /* endif */

        if (!pid)
        {
          printf ("ERR: could not read existing lockfile\n");
          close (fd);
          return (0);
        }
        close (fd);

        if (kill (pid, 0) == -1)
        {
          if (errno == ESRCH)
          {
            if (unlink (lockfilename) == -1)
            {
              printf ("ERR: could not remove lockfile\n");
              return (0);
            }
          }
          else
          {
            printf ("ERR: could not kill locking process\n");
            return (0);
          }
        }
        else
        {
          printf ("ERR: locking proccess still alive\n");
          return (0);
        }
      }
      else
      {
        printf ("ERR: could not create lockfile\n");
        return (0);
      }

      goto retry_open;
    }

    sprintf (buf, "%010ld\n", (long) getpid ());
    if (write (fd, buf, strlen (buf)) != strlen (buf))
    {
      printf ("ERR: could not write lockfile\n");
      close (fd);
      return (0);
    }
    close (fd);

    portisopen = 1;
    modemdf = open (port_device, O_RDWR | O_NDELAY, 0);
    if (modemdf < 0)
    {
      printf ("ERR: could not open [%s]\n", port_device);
      MDM_DISABLE ();
      return (0);
    }

  }

  fcntl (modemdf, F_SETFL, FASYNC);

  tcgetattr (modemdf, &tios);
  tios.c_iflag = 0;
  tios.c_oflag = 0;
  tios.c_lflag = 0;
  tios.c_cflag = B0 | CS8 | CREAD | CLOCAL | CRTSCTS;

  if (tcsetattr (modemdf, TCSANOW, &tios) < 0)
  {
    perror ("ERR: btxe: Cominit: could not tcsetattr\n");
    MDM_DISABLE ();
    return (0);
  }

  initBuffer (&txBuf);
  initBuffer (&rxBuf);
  CLEAR_INBOUND ();

  return (0x1954);

}

USHORT
com_online (void)
{
  int tmp = 0;

  if (ioctl (modemdf, TIOCMGET, &tmp) < 0)
    status_line (">ERR: com_online");
  return ((tmp & TIOCM_CD) != 0);
}

BOOLEAN
CHAR_AVAIL (void)
{
  int i;
  int numchar = 0;
  static char tmpbuf[BUFFERSIZE + 1];

  if (rxBuf.buf_rp != rxBuf.buf_wp)
    return (TRUE);
  if (ioctl (modemdf, FIONREAD, &numchar) < 0)
    status_line (">ERR: CHAR_AVAIL [1]");
  if (numchar > 0)
  {
    numchar = read (modemdf, tmpbuf, FREEINBUFFER (rxBuf));
    if (numchar == 0)
      status_line (">ERR: CHAR_AVAIL [2]");
    for (i = 0; i < numchar; i++)
      putBuffer (&rxBuf, tmpbuf[i]);
  }
  return (rxBuf.buf_rp != rxBuf.buf_wp);
}

void
SENDBYTE (unsigned char this)
{
  if (write (modemdf, &this, 1) == -1)
    status_line (">ERR: SENDBYTE");
}

void
SENDCHARS (char *this, size_t n, BOOLEAN carcheck)
{
  int numsent = 0;

  while ((!carcheck || CARRIER) && (n > 0))
  {
    numsent = write (modemdf, this, n);
    if (numsent == -1)
    {
      status_line (">ERR: SENDCHARS");
    }
    else
    {
      n -= numsent;
      this += numsent;
    }
  }
}

BOOLEAN
OUT_EMPTY (void)
{
  int out;

  if (ioctl (modemdf, TIOCOUTQ, &out) < 0)
    status_line (">ERR: OUT_EMPTY");
  return (out == 0);
}

void
CLEAR_OUTBOUND (void)
{
  if (tcflush (modemdf, TCOFLUSH) < 0)
    status_line (">ERR: CLEAR_OUTBOUND");
  initBuffer (&txBuf);
}

void
CLEAR_INBOUND (void)
{
  if (tcflush (modemdf, TCIFLUSH) < 0)
    status_line (">ERR: CLEAR_INBOUND");
  initBuffer (&rxBuf);
}

int
MODEM_IN (void)
{
  unsigned char c = 0;

  if (CHAR_AVAIL ())
  {
    c = getBuffer (&rxBuf);
    return (c);
  }
  else
    return (-1);
}

int
com_getc (int t)
{
  unsigned char c = 0;
  long t1;

  if (!CHAR_AVAIL ())
  {
    t1 = timerset ((unsigned int) (t * PER_SECOND));
    while (!CHAR_AVAIL ())
    {
      if (timeup (t1))
        return (EOF);
      if (!CARRIER)
        return (EOF);
      time_release ();
    }
  }
  c = getBuffer (&rxBuf);
  return (c);
}

int
PEEKBYTE (void)
{
  int c;

  if (CHAR_AVAIL ())
  {
    c = rxBuf.buf[rxBuf.buf_rp];
    return (c);
  }
  else
    return (-1);
}

void
LOWER_DTR (void)
{
  struct termios tty;

  tcgetattr (modemdf, &tty);
  cfsetospeed (&tty, B0);
  cfsetispeed (&tty, B0);
  tcsetattr (modemdf, TCSANOW, &tty);
}

void
RAISE_DTR (void)
{
  struct termios tty;

  LOWER_DTR ();
  tcgetattr (modemdf, &tty);
  cfsetospeed (&tty, max_baud.rate_mask);
  cfsetispeed (&tty, max_baud.rate_mask);
  tcsetattr (modemdf, TCSANOW, &tty);
}

void
XON_ENABLE (void)
{
  struct termios tty;

  tcgetattr (modemdf, &tty);
  tty.c_iflag |= (IXON);
  tcsetattr (modemdf, TCSANOW, &tty);
}

void
IN_XON_ENABLE (void)
{
  struct termios tty;

  tcgetattr (modemdf, &tty);
  tty.c_iflag |= (IXOFF);
  tcsetattr (modemdf, TCSANOW, &tty);
}

void
XON_DISABLE (void)
{
  struct termios tty;

  tcgetattr (modemdf, &tty);
  tty.c_iflag &= ~(IXOFF | IXON);
  tcsetattr (modemdf, TCSANOW, &tty);
}

void
MDM_ENABLE (unsigned long rate)
{
  struct termios tios;

  if (lock_baud && (baud != max_baud.rate_value))
    rate = max_baud.rate_mask;
  if (tcgetattr (modemdf, &tios))
    status_line (">ERR: MDM_ENABLE tcgetattr");
  if (cfsetospeed (&tios, rate))
    status_line (">ERR: MDM_ENABLE cfsetospeed %i)", rate);
  if (tcsetattr (modemdf, TCSANOW, &tios))
    status_line (">ERR: MDM_ENABLE tcsetattr");
}

void
MDM_DISABLE (void)
{
  initBuffer (&txBuf);
  initBuffer (&rxBuf);
  if (portisopen)
  {
    if (close (modemdf) == -1)
      status_line (">ERR: MDM_DISABLE close");
    modemdf = -1;
    portisopen = 0;
    if (unlink (lockfilename) == -1)
      status_line (">ERR: MDM_DISABLE lockfile");
  }
}

void
com_break (int on)
{
  /* this is not what it should do (turn break on */
  /* and off, but is better than nothing          */
  if (on)
    tcsendbreak (modemdf, 3);
}

void
BUFFER_BYTE (unsigned char this)
{
  if (!FREEINBUFFER (txBuf))
    UNBUFFER_BYTES ();
  putBuffer (&txBuf, this);
}

void
UNBUFFER_BYTES (void)
{
  static char tmpbuf[BUFFERSIZE + 1];
  int i = 0;

  while (USEDINBUFFER (txBuf))
    tmpbuf[i++] = getBuffer (&txBuf);
  SENDCHARS (tmpbuf, i, TRUE);
}

/*** Hydra receiver stuff ***/

USHORT
ComTXBlockTimeout (BYTE * lpBuf, USHORT cbBuf, ULONG ulTimer)
{
  int sent = 0;
  int fTimedOut;

  if ((modemdf == -1) || (cbBuf == 0) || (ulTimer == 0) || (NoModem))
    return (0);

  fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  while (!fTimedOut && CARRIER && cbBuf)
  {
    sent = write (modemdf, lpBuf, cbBuf);
    if (sent > 0)
    {
      cbBuf -= sent;
      lpBuf += sent;
    }
    if (cbBuf)
      time_release ();
    fTimedOut = (ulTimer != 0) ? (ulTimer < (ULONG) unix_time (NULL)) : FALSE;
  }
  return (sent);
}

USHORT
ComTXRemain (void)
{
  if (modemdf == -1)
    return (0);
  return (USEDINBUFFER (txBuf));
}

void
com_kick (void)
{
  XON_DISABLE ();
  XON_ENABLE ();
}

void
linux_pause ()
{
  fd_set rfds;
  struct timeval tv;

  if (!portisopen)
  {
    sleep (1);
  }
  else
  {
    FD_ZERO (&rfds);
    FD_SET (modemdf, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 42;

    if ((OUT_EMPTY ()) && (!USEDINBUFFER (txBuf)))
    {
      tv.tv_usec = 500;
      select (modemdf + 1, &rfds, NULL, NULL, &tv);
    }
    else
    {
      tv.tv_usec = 42;
      select (1, &rfds, &rfds, NULL, &tv);
    }
  }
}

/*** keyboard-handling ***/

BOOLEAN
KEYPRESS (void)
{
  int c;

  if (ioctl (0, FIONREAD, &c) < 0)
    status_line ("!asyn_lnx.c: KEYPRESS: Error");
  return (c);
}

// the next two functions are inspired by TTGetKey() from ansi.c in MsgEd
// by Paul Edwards (and modifications by Tobias Ernst)

int
getonechar (int mintime)
{
  int c;
  struct termios tios;

  tcgetattr (0, &tios);
  if (mintime == 1)
  {
    tios.c_cc[VMIN] = 1;
    tios.c_cc[VTIME] = 0;
  }
  else
  {
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 2;
  }
  tcsetattr (0, 0, &tios);
  c = getchar ();
  tcgetattr (0, &tios);
  tios.c_cc[VMIN] = 0;
  tios.c_cc[VTIME] = 0;
  tcsetattr (0, 0, &tios);
  return (c);
}

int
get_key ()
{
  int c;

  c = getonechar (1);

  if (c == 0x1b)
  {

    c = getonechar (2);
    switch (c)
    {
    case EOF:
      clearerr (stdin);         // no break here

    case 0x1b:
      c = getchar ();
      switch (c)
      {
      case '[':
        c = getonechar (2);
        switch (c)
        {
        case 'A':
          c = CUPAR;
          break;                // these are ATL + ... (but not used, so map

        case 'B':
          c = CDNAR;
          break;                // them to CTRL + ... (if alt does not work))

        case 'C':
          c = CRTAR;
          break;
        case 'D':
          c = CLFAR;
          break;
        case 'F':
          c = END;
          break;                // these are not used with ALT

        case 'G':
          c = HOME;
          break;
        case 'I':
          c = PGUP;
          break;
          // case 'L': c=INS;  break;
        case 'M':
          c = ALTF1;
          break;
        case 'N':
          c = ALTF2;
          break;
        case 'O':
          c = ALTF3;
          break;
        case 'P':
          c = ALTF4;
          break;
        case 'Q':
          c = ALTF5;
          break;
        case 'R':
          c = ALTF6;
          break;
        case 'S':
          c = ALTF7;
          break;
        case 'T':
          c = ALTF8;
          break;
        case 'U':
          c = ALTF9;
          break;
        case 'V':
          c = ALTF10;
          break;
        case '[':
          c = getonechar (2);
          switch (c)
          {
          case 'A':
            c = ALTF1;
            break;
          case 'B':
            c = ALTF2;
            break;
          case 'C':
            c = ALTF3;
            break;
          case 'D':
            c = ALTF4;
            break;
          case 'E':
            c = ALTF5;
            break;
          default:
            c = 0;
            break;
          }
          break;
        case '1':
          c = getonechar (2);
          switch (c)
          {
          case '~':
            c = HOME;
            break;
          case 53:
            if (getonechar (2) == '~')
              c = ALTF5;
            else
              c = 0;
            break;
          case 55:
            if (getonechar (2) == '~')
              c = ALTF6;
            else
              c = 0;
            break;
          case 56:
            if (getonechar (2) == '~')
              c = ALTF7;
            else
              c = 0;
            break;
          case 57:
            if (getonechar (2) == '~')
              c = ALTF8;
            else
              c = 0;
            break;
          default:
            c = 0;
          }
          break;
        case '2':
          c = getonechar (2);
          switch (c)
          {
            // case '~': c=INS; break;
          case 48:
            if (getonechar (2) == '~')
              c = ALTF9;
            else
              c = 0;
            break;
          case 49:
            if (getonechar (2) == '~')
              c = ALTF10;
            else
              c = 0;
            break;
          default:
            c = 0;
            break;
          }
          break;
        case '3':
          if (getonechar (2) == '~')
            c = 0;              /* DEL; */
          else
            c = 0;
          break;
        case '4':
          if (getonechar (2) == '~')
            c = END;
          else
            c = 0;
          break;
        case '5':
          switch (getonechar (2))
          {
          case '~':
            c = PGUP;
            break;
          case '^':
            c = CPGUP;
            break;
          default:
            c = 0;
            break;
          }
          break;
        case '6':
          switch (getonechar (2))
          {
          case '~':
            c = PGDN;
            break;
          case '^':
            c = CPGDN;
            break;
          default:
            c = 0;
            break;
          }
          break;
        case EOF:
          clearerr (stdin);
          c = 0;
          break;
        default:
          c = 0;
          break;
        }
        break;
      default:
        c = 0x1b;
        break;
      }
      break;
    case ':':
      c = PF1;
      break;
    case ';':
      c = PF2;
      break;
    case '<':
      c = PF3;
      break;
    case '=':
      c = PF4;
      break;
    case '>':
      c = PF5;
      break;
    case '?':
      c = PF6;
      break;
    case '@':
      c = PF7;
      break;
    case 'A':
      c = PF8;
      break;
    case 'B':
      c = PF9;
      break;
    case 'C':
      c = PF10;
      break;
    case 71:
      c = HOME;
      break;
    case 72:
      c = UPAR;
      break;
    case 73:
      c = PGUP;
      break;
    case 75:
      c = LFAR;
      break;
    case 77:
      c = RTAR;
      break;
    case 81:
      c = PGDN;
      break;
    case 80:
      c = DNAR;
      break;
      // case   83: c=DEL;  break;
    case 'a':
      c = ALTA;
      break;
    case 'b':
      c = ALTB;
      break;
    case 'c':
      c = ALTC;
      break;
    case 'd':
      c = ALTD;
      break;
    case 'e':
      c = ALTE;
      break;
    case 'f':
      c = ALTF;
      break;
    case 'g':
      c = ALTG;
      break;
    case 'h':
      c = ALTH;
      break;
    case 'i':
      c = ALTI;
      break;
    case 'j':
      c = ALTJ;
      break;
    case 'k':
      c = ALTK;
      break;
    case 'l':
      c = ALTL;
      break;
    case 'm':
      c = ALTM;
      break;
    case 'n':
      c = ALTN;
      break;
    case 'o':
      c = ALTO;
      break;
    case 'p':
      c = ALTP;
      break;
    case 'q':
      c = ALTQ;
      break;
    case 'r':
      c = ALTR;
      break;
    case 's':
      c = ALTS;
      break;
    case 't':
      c = ALTT;
      break;
    case 'u':
      c = ALTU;
      break;
    case 'v':
      c = ALTV;
      break;
    case 'w':
      c = ALTW;
      break;
    case 'x':
      c = ALTX;
      break;
    case 'y':
      c = ALTY;
      break;
    case 'z':
      c = ALTZ;
      break;
    case 'O':
      c = getonechar (2);
      switch (c)
      {
      case 33:
        c = ALTA;
        break;
      case 64:
        c = ALTB;
        break;
      case 35:
        c = ALTC;
        break;
      case 36:
        c = ALTD;
        break;
      case 37:
        c = ALTE;
        break;
      case 94:
        c = ALTF;
        break;
      case 38:
        c = ALTG;
        break;
      case 42:
        c = ALTH;
        break;
      case 40:
        c = ALTI;
        break;
      case 41:
        c = ALTJ;
        break;
      case 95:
        c = ALTK;
        break;
      case 43:
        c = ALTL;
        break;
      case 91:
        c = ALTM;
        break;
      case 132:
        c = ALTN;
        break;
      case 93:
        c = ALTO;
        break;
      case 125:
        c = ALTP;
        break;
      case 92:
        c = ALTQ;
        break;
      case 34:
        c = ALTR;
        break;
      case 124:
        c = ALTS;
        break;
      case 59:
        c = ALTT;
        break;
      case 58:
        c = ALTU;
        break;
      case 39:
        c = ALTV;
        break;
      case 44:
        c = ALTW;
        break;
      case 60:
        c = ALTX;
        break;
      case 46:
        c = ALTY;
        break;
      case 62:
        c = ALTZ;
        break;

      case 48:
        c = ALT0;
        break;
      case 49:
        c = ALT1;
        break;
      case 50:
        c = ALT2;
        break;
      case 51:
        c = ALT3;
        break;
      case 52:
        c = ALT4;
        break;
      case 53:
        c = ALT5;
        break;
      case 54:
        c = ALT6;
        break;
      case 55:
        c = ALT7;
        break;
      case 56:
        c = ALT8;
        break;
      case 57:
        c = ALT9;
        break;

      case 75:
        c = ALTF1;
        break;
      case 76:
        c = ALTF2;
        break;
      case 79:
        c = ALTF3;
        break;
      case 78:
        c = ALTF4;
        break;
      case 69:
        c = ALTF5;
        break;
      case 70:
        c = ALTF6;
        break;
      case 71:
        c = ALTF7;
        break;
      case 72:
        c = ALTF8;
        break;
      case 73:
        c = ALTF9;
        break;
      case 74:
        c = ALTF10;
        break;

      case 80:
        c = PF1;
        break;
      case 81:
        c = PF2;
        break;
      case 82:
        c = PF3;
        break;
      case 83:
        c = PF4;
        break;
      case 84:
        c = PF5;
        break;

        // case 110: c=DEL;  break;
        // case 112: c=INS;  break;
      case 113:
        c = END;
        break;
      case 115:
        c = PGDN;
        break;
      case 119:
        c = HOME;
        break;
      case 121:
        c = PGUP;
        break;

      case 98:
        c = CDNAR;
        break;
      case 97:
        c = CUPAR;
        break;
      case 99:
        c = CRTAR;
        break;
      case 100:
        c = CLFAR;
        break;

      default:
        c = 0x1b;
        break;
      }
      break;
    case '[':
      c = getonechar (2);
      switch (c)
      {
      case 'A':
        c = UPAR;
        break;
      case 'B':
        c = DNAR;
        break;
      case 'C':
        c = RTAR;
        break;
      case 'D':
        c = LFAR;
        break;
      case 'F':
        c = END;
        break;
      case 'G':
        c = HOME;
        break;
      case 'I':
        c = PGUP;
        break;
        // case 'L': c=INS;  break;
      case 'M':
        c = PF1;
        break;
      case 'N':
        c = PF2;
        break;
      case 'O':
        c = PF3;
        break;
      case 'P':
        c = PF4;
        break;
      case 'Q':
        c = PF5;
        break;
      case 'R':
        c = PF6;
        break;
      case 'S':
        c = PF7;
        break;
      case 'T':
        c = PF8;
        break;
      case 'U':
        c = PF9;
        break;
      case 'V':
        c = PF10;
        break;
      case '[':
        c = getonechar (2);
        switch (c)
        {
        case 'A':
          c = PF1;
          break;
        case 'B':
          c = PF2;
          break;
        case 'C':
          c = PF3;
          break;
        case 'D':
          c = PF4;
          break;
        case 'E':
          c = PF5;
          break;
        default:
          c = 0;
          break;
        }
        break;
      case '1':
        c = getonechar (2);
        switch (c)
        {
        case '~':
          c = HOME;
          break;
        case 53:
          if (getonechar (2) == '~')
            c = PF5;
          else
            c = 0;
          break;
        case 55:
          if (getonechar (2) == '~')
            c = PF6;
          else
            c = 0;
          break;
        case 56:
          if (getonechar (2) == '~')
            c = PF7;
          else
            c = 0;
          break;
        case 57:
          if (getonechar (2) == '~')
            c = PF8;
          else
            c = 0;
          break;
        default:
          c = 0;
        }
        break;
      case '2':
        c = getonechar (2);
        switch (c)
        {
          // case '~': c=INS; break;
        case 48:
          if (getonechar (2) == '~')
            c = PF9;
          else
            c = 0;
          break;
        case 49:
          if (getonechar (2) == '~')
            c = PF10;
          else
            c = 0;
          break;
        default:
          c = 0;
          break;
        }
        break;
      case '3':
        if (getonechar (2) == '~')
          c = 0;                /* DEL; */
        else
          c = 0;
        break;
      case '4':
        if (getonechar (2) == '~')
          c = END;
        else
          c = 0;
        break;
      case '5':
        switch (getonechar (2))
        {
        case '~':
          c = PGUP;
          break;
        case '^':
          c = CPGUP;
          break;
        default:
          c = 0;
          break;
        }
        break;
      case '6':
        switch (getonechar (2))
        {
        case '~':
          c = PGDN;
          break;
        case '^':
          c = CPGDN;
          break;
        default:
          c = 0;
          break;
        }
        break;
      case EOF:
        clearerr (stdin);
        c = 0;
        break;
      default:
        c = 0;
        break;
      }
      break;
    default:
      if (c < 0)
        c = 0x1b;
      else
        c = 0;
      break;
    }
  }
  else if (c == 127)
  {
    c = BKS;
  }
  else if (c > 127)
  {
    switch (tolower (c - 128))
    {
    case 'a':
      c = ALTA;
      break;
    case 'b':
      c = ALTB;
      break;
    case 'c':
      c = ALTC;
      break;
    case 'd':
      c = ALTD;
      break;
    case 'e':
      c = ALTE;
      break;
    case 'f':
      c = ALTF;
      break;
    case 'g':
      c = ALTG;
      break;
    case 'h':
      c = ALTH;
      break;
    case 'i':
      c = ALTI;
      break;
    case 'j':
      c = ALTJ;
      break;
    case 'k':
      c = ALTK;
      break;
    case 'l':
      c = ALTL;
      break;
    case 'm':
      c = ALTM;
      break;
    case 'n':
      c = ALTN;
      break;
    case 'o':
      c = ALTO;
      break;
    case 'p':
      c = ALTP;
      break;
    case 'q':
      c = ALTQ;
      break;
    case 'r':
      c = ALTR;
      break;
    case 's':
      c = ALTS;
      break;
    case 't':
      c = ALTT;
      break;
    case 'u':
      c = ALTU;
      break;
    case 'v':
      c = ALTV;
      break;
    case 'w':
      c = ALTW;
      break;
    case 'x':
      c = ALTX;
      break;
    case 'y':
      c = ALTY;
      break;
    case 'z':
      c = ALTZ;
      break;
    case '0':
      c = ALT0;
      break;
    case '1':
      c = ALT1;
      break;
    case '2':
      c = ALT2;
      break;
    case '3':
      c = ALT3;
      break;
    case '4':
      c = ALT4;
      break;
    case '5':
      c = ALT5;
      break;
    case '6':
      c = ALT6;
      break;
    case '7':
      c = ALT7;
      break;
    case '8':
      c = ALT8;
      break;
    case '9':
      c = ALT9;
      break;
    default:
      c = 0;
    }
  }
  else if (c < 0)
    c = 0;
  return (c);
}



/*** string functions ***/

char *
strupr (char *this)
{
  int i;

  for (i = 0; this[i]; i++)
    this[i] = toupper (this[i]);
  return (this);
}

char *
strlwr (char *this)
{
  int i;

  for (i = 0; this[i]; i++)
    this[i] = tolower (this[i]);
  return (this);
}

int
memicmp (const void *__s1, const void *__s2, size_t __n)
{
  // this is not the same, but near enought for the moment
  return (strncasecmp (__s1, __s2, __n));
}

char *
ultoa (unsigned long n, char *buf, int base)
{
  int i, j;

  if (buf == NULL)
  {
    status_line ("!ultoa: null-buffer passed");
    return (NULL);
  }
  else
  {
    if (!n)
    {
      buf[0] = '0';
      buf[1] = 0;
      return (buf);
    }
    j = 1;
    i = 0;
    while (n >= j)
    {
      j *= base;
      i++;
    }
    buf[i--] = 0;
    while (n)
    {
      buf[i--] = '0' + (n % base);
      n /= base;
    }
    return (buf);
  }
}

/* $Id: asyn_lnx.c,v 1.9 1999/08/29 19:55:48 ceh Exp $ */
