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
 * Filename    : $Source: E:/cvs/btxe/src/emxmxcom.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/05 02:51:47 $
 * State       : $State: Exp $
 * Orig. Author: Andre Grueneberg
 *
 * Description : MAXCOMM.DLL interface for EMX
 *
 *---------------------------------------------------------------------------*/

#include <os2thunk.h>
#include "includes.h"

#ifdef __EMX__                  /* only needed for EMX */
#ifndef NEED_OS2COMMS

/* THUNK functions to the pascal 16bit functions in MAXCOMM.DLL */

USHORT COMOPEN ();
USHORT COMMAPI
ComOpen (PSZ PortName, LPHCOMM pHcomm, USHORT RxBufSize, USHORT TxBufSize)
{
  return ((USHORT)
          (_THUNK_PROLOG (4 + 4 + 2 + 2);
           _THUNK_FLAT (PortName);
           _THUNK_FLAT (pHcomm);
           _THUNK_SHORT (RxBufSize);
           _THUNK_SHORT (TxBufSize);
           _THUNK_CALLI (_emx_32to16 (&COMOPEN))));
}


USHORT COMHREGISTER ();
USHORT COMMAPI
ComHRegister (HCFILE hf, LPHCOMM * pHcomm, USHORT RxBufSize, USHORT TxBufSize)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4 + 2 + 2);
           _THUNK_SHORT (hf);
           _THUNK_FLAT (_emx_32to16 (&pHcomm));
           _THUNK_SHORT (RxBufSize);
           _THUNK_SHORT (TxBufSize);
           _THUNK_CALLI (_emx_32to16 (&COMHREGISTER))));
}


USHORT COMCLOSE ();
USHORT COMMAPI
ComClose (HCOMM hcomm)
{
  return ((USHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hcomm);
           _THUNK_CALLI (_emx_32to16 (&COMCLOSE))));
}


USHORT COMISONLINE ();
USHORT COMMAPI
ComIsOnline (HCOMM hcomm)
{
  return ((USHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hcomm);
           _THUNK_CALLI (_emx_32to16 (&COMISONLINE))));
}


USHORT COMWRITE ();
USHORT COMMAPI
ComWrite (HCOMM hc, PVOID pvBuf, USHORT cbBuf)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4 + 2);
           _THUNK_SHORT (hc);
           _THUNK_FLAT (pvBuf);
           _THUNK_SHORT (cbBuf);
           _THUNK_CALLI (_emx_32to16 (&COMWRITE))));
}


USHORT COMREAD ();
USHORT COMMAPI
ComRead (HCOMM hc, PVOID pvBuf, USHORT cbBuf, PUSHORT pcbBytesRead)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4 + 2 + 4);
           _THUNK_SHORT (hc);
           _THUNK_FLAT (pvBuf);
           _THUNK_SHORT (cbBuf);
           _THUNK_FLAT (pcbBytesRead);
           _THUNK_CALLI (_emx_32to16 (&COMREAD))));

}


USHORT COMGETC ();
SHORT COMMAPI
ComGetc (HCOMM hc)
{
  return ((SHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMGETC))));
}


USHORT COMPEEK ();
SHORT COMMAPI
ComPeek (HCOMM hc)
{
  return ((SHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMPEEK))));
}


USHORT COMPUTC ();
USHORT COMMAPI
ComPutc (HCOMM hc, SHORT c)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 2);
           _THUNK_SHORT (hc);
           _THUNK_SHORT (c);
           _THUNK_CALLI (_emx_32to16 (&COMPUTC))));
}


USHORT COMRXWAIT ();
USHORT COMMAPI
ComRxWait (HCOMM hc, LONG lTimeOut)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4);
           _THUNK_SHORT (hc);
           _THUNK_LONG (lTimeOut);
           _THUNK_CALLI (_emx_32to16 (&COMRXWAIT))));
}


USHORT COMTXWAIT ();
USHORT COMMAPI
ComTxWait (HCOMM hc, LONG lTimeOut)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4);
           _THUNK_SHORT (hc);
           _THUNK_LONG (lTimeOut);
           _THUNK_CALLI (_emx_32to16 (&COMTXWAIT))));
}


USHORT COMINCOUNT ();
USHORT COMMAPI
ComInCount (HCOMM hc)
{
  return ((USHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMINCOUNT))));
}


USHORT COMOUTCOUNT ();
USHORT COMMAPI
ComOutCount (HCOMM hc)
{
  return ((USHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMOUTCOUNT))));
}


USHORT COMOUTSPACE ();
USHORT COMMAPI
ComOutSpace (HCOMM hc)
{
  return ((USHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMOUTSPACE))));
}


USHORT COMPURGE ();
USHORT COMMAPI
ComPurge (HCOMM hc, SHORT fsWhich)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 2);
           _THUNK_SHORT (hc);
           _THUNK_SHORT (fsWhich);
           _THUNK_CALLI (_emx_32to16 (&COMPURGE))));
}


USHORT COMPAUSE ();
USHORT COMMAPI
ComPause (HCOMM hc)
{
  return ((USHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMPAUSE))));
}


USHORT COMRESUME ();
USHORT COMMAPI
ComResume (HCOMM hc)
{
  return ((USHORT)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMRESUME))));
}


USHORT COMGETFH ();
HCFILE COMMAPI
ComGetFH (HCOMM hc)
{
  return ((HCFILE)
          (_THUNK_PROLOG (2);
           _THUNK_SHORT (hc);
           _THUNK_CALLI (_emx_32to16 (&COMGETFH))));
}


USHORT COMWATCHDOG ();
USHORT COMMAPI
ComWatchDog (HCOMM hc, BOOL OnOff, USHORT Seconds)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 2 + 2);
           _THUNK_SHORT (hc);
           _THUNK_SHORT (OnOff);
           _THUNK_SHORT (Seconds);
           _THUNK_CALLI (_emx_32to16 (&COMWATCHDOG))));
}


USHORT COMGETDCB ();
USHORT COMMAPI
ComGetDCB (HCOMM hc, PDCBINFO pdbc)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4);
           _THUNK_SHORT (hc);
           _THUNK_FLAT (pdbc);
           _THUNK_CALLI (_emx_32to16 (&COMGETDCB))));

}


USHORT COMSETDCB ();
USHORT COMMAPI
ComSetDCB (HCOMM hc, PDCBINFO pdcb)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4);
           _THUNK_SHORT (hc);
           _THUNK_FLAT (pdcb);
           _THUNK_CALLI (_emx_32to16 (&COMSETDCB))));
}


USHORT COMSETBAUDRATE ();
USHORT COMMAPI
ComSetBaudRate (HCOMM hcomm,
                LONG bps,
                CHAR parity,
                USHORT databits,
                USHORT stopbits)
{
  return ((USHORT)
          (_THUNK_PROLOG (2 + 4 + 2 + 2 + 2);
           _THUNK_SHORT (hcomm);
           _THUNK_LONG (bps);
           _THUNK_CHAR (parity);
           _THUNK_SHORT (databits);
           _THUNK_SHORT (stopbits);
           _THUNK_CALLI (_emx_32to16 (&COMSETBAUDRATE))));
}


#endif /* NEED_OS2COMMS */
#endif /* __EMX__ */

/* $Id: emxmxcom.c,v 1.3 1999/03/05 02:51:47 mr Exp $ */
