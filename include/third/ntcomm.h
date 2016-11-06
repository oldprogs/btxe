/*
Copyright 1989, 1998 by Lanius Corporation.  All rights reserved.

COMMERCIAL DISTRIBUTION AND/OR USE IS PROHIBITED WITHOUT WRITTEN
CONSENT FROM LANIUS CORPORATION.

Noncommercial distribution or use is permitted under the
following terms:

- You may copy and distribute verbatim copies of the NTCOMM
documentation, source and executable code as you receive it, in any
medium, provided that you conspicuously and appropriately publish on
each copy a valid copyright notice "Copyright 1989, 1995 by Lanius
Corporation"; keep intact the notices on all files that refer to
this License Agreement and to the absence of any warranty;  PROVIDE
UNMODIFIED COPIES OF THE DOCUMENTATION AS PROVIDED WITH THE PROGRAM;
and give any other recipients of the NTCOMM program a copy of this
License Agreement along with the program.  You may charge a
distribution fee for the physical act of transferring a copy, but no
more than is necessary to recover your actual costs incurred in the
transfer.

- Mere aggregation of another unrelated program with this program
and documentation (or derivative works) on a volume of a storage or
distribution medium does not bring the other program under the scope
of these terms.

- You may not copy, sublicense, distribute or transfer NTCOMM and
its associated documentation except as expressly provided under this
License Agreement.  Any attempt otherwise to copy, sublicense,
distribute or transfer NTCOMM is void and your rights to use the
program under this License agreement shall be automatically
terminated.

However, parties who have received computer software programs from
you with this License Agreement will not have their licenses
terminated so long as such parties remain in full compliance, and
notify Lanius Corporation of their intention to comply with this
Agreement.

- You may not incorporate all or part of NTCOMM (including related
utilities) into a program which is not completely free for all
users.  If you wish to distribute NTCOMM in this manner, you must
obtain written permission from Lanius Corporation.

- This license may be revoked by Lanius Corporation without prior
notice.

WARRANTY

LANIUS CORPORATION PROVIDES ABSOLUTELY NO WARRANTY.  EXCEPT WHEN
OTHERWISE STATED IN WRITING, LANIUS CORPORATION AND/OR OTHER PARTIES
PROVIDE NTCOMM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF NTCOMM, AND THE
ACCURACY OF ITS ASSOCIATED DOCUMENTATION, IS WITH YOU.  SHOULD
NTCOMM OR ITS ASSOCIATED DOCUMENTATION PROVE DEFECTIVE, YOU ASSUME
THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT WILL LANIUS CORPORATION BE RESPONSIBLE IN ANY WAY FOR
THE BEHAVIOR OF MODIFIED VERSIONS OF NTCOMM.  IN NO EVENT WILL
LANIUS CORPORATION AND/OR ANY OTHER PARTY WHO MAY REDISTRIBUTE
NTCOMM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING
ANY LOST PROFITS, LOST MONIES, OR OTHER SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE
(INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED
INACCURATE OR LOSSES SUSTAINED BY THIRD PARTIES OR A FAILURE OF THE
PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) NTCOMM, EVEN IF LANIUS
CORPORATION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES, OR
FOR ANY CLAIM BY ANY OTHER PARTY.

If you have any questions about this license agreement, please feel
free to get in touch with us.

http://www.lanius.com
*/

#ifndef __NTCOMM_H_DEFINED
#define __NTCOMM_H_DEFINED

#include "third\pwin.h"
#include "third\comqueue.h"

#define COMMAPI __stdcall /* Standard NT API calling convention */
/*#define DEFAULT_COMM_MASK   (EV_ERR | EV_RLSD | EV_RXFLAG)*/
#define DEFAULT_COMM_MASK   (EV_ERR | EV_RLSD)

typedef struct
{
  HANDLE h;                 /* Handle of the physical com port file */

  COMQUEUE cqTx;            /* Transmit queue */
  COMQUEUE cqRx;            /* Receive queue */

  HANDLE hRx, hTx, hMn;     /* Handles for read and write threads */
  HANDLE hevTx, hevRx;      /* Semaphores for the tx/rx threads */
  HANDLE hevTxDone;         /* Pending transmit has completed */
  HANDLE hevRxWait, hevTxWait;  /* Waiting for input/output buf to clear */
  HANDLE hevRxPause, hevTxPause;  /* Stop transmitter for compause/resume */
  HANDLE hevRxDone;         /* Pending receive has completed */
  HANDLE hevMonDone;        /* Pending monitor has completed */

  BOOL fDCD;                /* Current status of DCD */
  volatile BOOL fDie;       /* True if we are trying to kill threads */
  DWORD dwCtrlC;            /* How many ^C's have we received from user? */
  volatile DWORD cThreads;  /* Number of active threads */

  COMMTIMEOUTS ct;          /* Timeout values */
} *HCOMM;

#define COMM_PURGE_RX 1
#define COMM_PURGE_TX 2
#define COMM_PURGE_ALL  (COMM_PURGE_RX | COMM_PURGE_TX)

#if ((defined __BORLANDC__) | (defined _MSC_VER)) // NS 03/02/99 - alse MSVC

#define ComOpenHandle _ComOpenHandle
#define ComOpen _ComOpen
#define ComClose _ComClose
#define ComIsOnline _ComIsOnline
#define ComWrite _ComWrite
#define ComRead _ComRead
#define ComGetc _ComGetc
#define ComPeek _ComPeek
#define ComPutc _ComPutc
#define ComRxWait _ComRxWait
#define ComTxWait _ComTxWait
#define ComInCount _ComInCount
#define ComOutCount _ComOutCount
#define ComOutSpace _ComOutSpace
#define ComPurge _ComPurge
#define ComGetHandle _ComGetHandle
#define ComGetDCB _ComGetDCB
#define ComSetDCB _ComSetDCB
#define ComSetBaudRate _ComSetBaudRate
#define ComPause _ComPause
#define ComResume _ComResume
#define ComWatchDog _ComWatchDog

#endif  /* __BORLANDC__ */

BOOL COMMAPI ComOpenHandle(HANDLE hfComm, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf);
BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf);
BOOL COMMAPI ComClose(HCOMM hc);
USHORT COMMAPI ComIsOnline(HCOMM hc);
BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount);
BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead);
int COMMAPI ComGetc(HCOMM hc);
int COMMAPI ComPeek(HCOMM hc);
BOOL COMMAPI ComPutc(HCOMM hc, int c);
BOOL COMMAPI ComRxWait(HCOMM hc, DWORD dwTimeOut);
BOOL COMMAPI ComTxWait(HCOMM hc, DWORD dwTimeOut);
DWORD COMMAPI ComInCount(HCOMM hc);
DWORD COMMAPI ComOutCount(HCOMM hc);
DWORD COMMAPI ComOutSpace(HCOMM hc);
BOOL COMMAPI ComPurge(HCOMM hc, DWORD fBuffer);
HANDLE COMMAPI ComGetHandle(HCOMM hc);
BOOL COMMAPI ComGetDCB(HCOMM hc, LPDCB pdcb);
USHORT COMMAPI ComSetDCB(HCOMM hc, LPDCB pdcb);
BOOL COMMAPI ComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE bDataBits, BYTE bStopBits);
BOOL COMMAPI ComPause(HCOMM hc);
BOOL COMMAPI ComResume(HCOMM hc);
BOOL COMMAPI ComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut);

#endif /* __NTCOMM_H_DEFINED */

