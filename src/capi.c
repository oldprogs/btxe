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
 * Filename    : $Source: E:/cvs/btxe/src/capi.c,v $
 * Revision    : $Revision: 1.7 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 01:49:23 $
 * State       : $State: Exp $
 * Orig. Author: C.F.S.
 *
 * Description : CAPI-CID module
 *
 *---------------------------------------------------------------------------*/

#if defined(OS_2) && !defined(__EMX__)
#pragma stack16 (0)

#include "includes.h"
#include "third/capi11.h"

static HMODULE capiDLL = (HMODULE) 0;

HFILE IsdnHandle;
int ApplId;
int cancel;
static HEV EventSemaphore = 0;
int Manufacturer = 0;
bool needs_dossleep = FALSE;

void CAPIGoodBye (void);

char *MessageLost = "CAPI message lost";
char *BadID = "CAPI error: Invalid application ID";

#define  MAN_UNKNOWN 0
#define  MAN_DIEHL   1
#define  MAN_TELES   2
#define  MAN_NCP     3
#define  MAN_NCPA    4

void
ConvertToDecent (unsigned char *number)
{
  int i, j;
  int l = number[0];            /* Length in [0], "nice" Pascal */

  for (i = 1; i <= l; i++)
  {
    if (number[i] & 0x80)
      break;
  }

  j = 0;
  for (i++; i <= l; i++, j++)
    number[j] = number[i];

  number[j] = '\0';
}

void
#if defined(__IBMC__) || defined(__IBMCPP__)
ProcessCAPIMessage (void *msgrx)
#else
ProcessCAPIMessage (API_MSG _Far16 * msgrx)
#endif
{
  API_MSG msg =
  {
    {0, 0, 0, 0}
  };
  API_MSG *apimsg = msgrx;
  unsigned short cmx;
  int plci, i, j;
  unsigned char phone[40];

  msg.header.Appl_id = ApplId;
  msg.header.Number = 0;

  if (apimsg == NULL)           /* Null message */
    return;

  cmx = apimsg->header.Command;

  switch (cmx)
  {
  case CONNECT_I:
    plci = apimsg->info.connect_ind.PLCI;
    i = apimsg->info.connect_ind.Structs[0];
    for (j = 0; j <= i; j++)
      phone[j] = apimsg->info.connect_ind.Structs[j];
    phone[j] = 0;
    ConvertToDecent (phone);
    if (UpdateCID)
      strcpy (CIDFromCAPI, (char *) phone);
    msg.header.Length = 11;
    msg.header.Command = CONNECT_I | RESPONSE;
    msg.info.connect_res.PLCI = plci;
    msg.info.connect_res.Reject = 1;
    API_PUT_MESSAGE (ApplId, (API_MSG * _Seg16) & msg);
    return;

  case DISCONNECT_I:
    plci = apimsg->info.disconnect_ind.PLCI;
    msg.header.Length = 10;
    msg.header.Command = DISCONNECT_I | CONFIRM;
    msg.info.disconnect_res.PLCI = plci;
    API_PUT_MESSAGE (ApplId, (API_MSG * _Seg16) & msg);
    return;

    /* Should not happen but just in case we are ready to process them */
  case INFO_I:
    plci = apimsg->info.info_ind.PLCI;
    msg.header.Length = 10;
    msg.header.Command = INFO_I | CONFIRM;
    msg.info.info_con.PLCI = plci;
    API_PUT_MESSAGE (ApplId, (API_MSG * _Seg16) & msg);
    return;
  }

  return;
}

typedef void _Far16 _Pascal fnCapiSignal (void);

static void _Far16 _Pascal
SignalRoutine (void)
{
  DosPostEventSem (EventSemaphore);
  if (needs_dossleep)
    DosSleep (1);
  return;
}

void
Listen (PVOID x)
{
#ifdef __BORLANDC__
  /* TE 140498: This probably is not correct, but it does not compile with
   * Borland the other way round. You'd rather not use the CAPI
   * feature with a Borland compiled BT ... */
  API_MSG _Seg16 *msg;

#else
#if defined(__IBMC__) || defined(__IBMCPP__)

  API_MSG *_Seg16 msg;

#else

  API_MSG _Far16 *msg;

#endif
#endif

#if !defined(__IBMC__) && !defined(__IBMCPP__)

  void _Far16 _Pascal (*pfnCapiSignal) (void);

#endif

  int rcs, rca, i;
  unsigned long scount = 0;

  x = x;

#if !defined(__IBMC__) && !defined(__IBMCPP__)
  pfnCapiSignal = &SignalRoutine;
#endif

  printf ("CAPI message process thread successfully started.\n");
  if (Manufacturer == MAN_NCPA)
  {
    do
    {
      DosSleep (5);
      rca = API_GET_MESSAGE (ApplId, (API_MSG * _Seg16 * _Seg16) & msg);
      if (rca == 0)
        ProcessCAPIMessage (msg);
      if (rca == 0x1002)
        status_line (":%s (%d)", BadID, ApplId);
      if (rca == 0x1007)
        status_line (":%s", MessageLost);
    }
    while (!cancel);

    _endthread ();
  }

  DosResetEventSem (EventSemaphore, &scount);

#if defined(__IBMC__) || defined(__IBMCPP__)
  API_SET_SIGNAL (ApplId, SignalRoutine);
#else
  API_SET_SIGNAL (ApplId, pfnCapiSignal);
#endif

  if (Manufacturer == MAN_DIEHL)
  {
    do
    {
      rcs = DosWaitEventSem (EventSemaphore, 10000);
      if (rcs == 0)
        DosResetEventSem (EventSemaphore, &scount);

      for (i = 0; i < scount; i++)
      {
        rca = API_GET_MESSAGE (ApplId, (API_MSG * _Seg16 * _Seg16) & msg);
        if (rca == 0)
          ProcessCAPIMessage (msg);
        if (rca == 0x1002)
          status_line (":%s (%d)", BadID, ApplId);
        if (rca == 0x1007)
          status_line (":%s", MessageLost);
      };

    }
    while (!cancel);

    _endthread ();
  }

  do
  {
    rcs = DosWaitEventSem (EventSemaphore, 10000);
    if (rcs == 0)
      DosResetEventSem (EventSemaphore, &scount);

    do
    {
      rca = API_GET_MESSAGE (ApplId, (API_MSG * _Seg16 * _Seg16) & msg);
      if (rca == 0)
        ProcessCAPIMessage (msg);
      if (rca == 0x1002)
        status_line (":%s (%d)", BadID, ApplId);
      if (rca == 0x1007)
        status_line (":%s", MessageLost);
    }
    while (rca == 0);
  }
  while (!cancel);

  _endthread ();
}

int
CAPISetup (char *eaz, char *services)
{
  int rc;
  char *MessageBuffer;
  int NumberMessages, Layer3, L3Size, L3Packets;
  char hb[100];
  unsigned short EazMask = 0, ServiceMask = 0;
  API_MSG msg =
  {
    {0, 0, 0, 0}
  };

  printf ("CAPI: ");
  if (!API_INSTALLED ())
  {
    printf ("Not found. Cannot continue\n");
    return -1;
  }

  printf ("is installed.\n");
  printf ("Manufacturer: ");
  API_GET_MANUFACTURER (hb);
  printf ("%s\n", hb);
  Manufacturer = MAN_UNKNOWN;

  if (strstrci (hb, "Diehl") == hb)
    Manufacturer = MAN_DIEHL;
  if (strstrci (hb, "TELES") == hb)
    Manufacturer = MAN_TELES;
  if (strstrci (hb, "High So") == hb)
    Manufacturer = MAN_NCP;
  if (strstrci (hb, "NCP") == hb)
    Manufacturer = MAN_NCPA;

  API_GET_VERSION (hb);
  printf ("Version: %s\n", hb);
  if (strstr (hb, "Release 2.9") != NULL && Manufacturer == MAN_TELES)
    needs_dossleep = TRUE;
  API_GET_SERIAL_NUMBER (hb);
  printf ("Serial #: %s\n", hb);

  /*  ------------------------- Variable setup ---------------------------- */
  MessageBuffer = (char *) malloc (65000L);
  if (MessageBuffer == NULL)
  {
    printf ("Not enough memory for message buffer.\n");
    return -1;
  }

  NumberMessages = 40;
  Layer3 = 2;
  L3Size = 2048;
  if (Manufacturer == MAN_DIEHL)
    L3Packets = (63000 - (180 * NumberMessages)) / L3Size;
  else
    L3Packets = (32000 - (180 * NumberMessages)) / L3Size;

  /*  ---------------------- Registration with CAPI ---------------------- */
  ApplId = 0;
  ApplId = API_REGISTER (MessageBuffer, NumberMessages, Layer3, L3Packets, L3Size);
  if (!ApplId)
  {
    printf ("CAPI registration failed.\n");
    return -1;
  }

  printf ("Registration with CAPI successful (ID: %d)\n", ApplId);

  rc = DosCreateEventSem ((PSZ) NULL, &EventSemaphore, 0, FALSE);
  if (rc)
  {
    printf ("\nError calling DosCreateEventSem: %d\n", rc);
    CAPIGoodBye ();
    return -1;
  }

  cancel = 0;
  _beginthread (Listen, NULL, 32768, NULL);

  DosSleep (1000);              /* Sleep a bit until CAPI is ready */

  msg.header.Length = 17;
  msg.header.Command = LISTEN_R;
  msg.header.Appl_id = ApplId;

  msg.info.listen_req.Controller = 0;
  msg.info.listen_req.Info_Mask = 0x0000;  /* Don't disturb with infos */
  msg.info.listen_req.Si_Mask = 0x000F;  /* Mask */

  while (isdigit (*eaz))
    EazMask |= (0x01 << (*eaz++ & 0x0F));

  while (*services)
  {
    switch (toupper (*services++))
    {
    case 'V':
      ServiceMask |= 1;
      break;
    case 'T':
      ServiceMask |= 2;
      break;
    case 'A':
      ServiceMask |= 4;
      break;
    case '2':
      ServiceMask |= 8;
      break;
    case '4':
      ServiceMask |= 16;
      break;
    case 'I':
      ServiceMask |= 32;
      break;
    case 'D':
      ServiceMask |= 128;
      break;
    case '5':
      ServiceMask |= 256;
      break;
    case 'L':
      ServiceMask |= 512;
      break;
    case 'M':
      ServiceMask |= 1024;
      break;
    case 'R':
      ServiceMask |= 8192;
      break;
    case 'G':
      ServiceMask |= 16384;
      break;
    case 'X':
      ServiceMask |= 32768;
      break;
    }
  }

  printf ("EAZ Mask: %hu, Service Mask %hu\n", EazMask, ServiceMask);
  msg.info.listen_req.Eaz_Mask = EazMask;
  msg.info.listen_req.Si_Mask = ServiceMask;
  rc = API_PUT_MESSAGE (ApplId, (API_MSG *) & msg);
  return 0;
}

void
CAPIGoodBye (void)
{
  int rc = API_RELEASE (ApplId);

  cancel = 1;
  if (!rc)
    printf ("Capi released successfully.\n");
  else
    printf ("Could not release CAPI (error: %d)!\n", rc);

  DosSleep (100);
}


USHORT
CapiLoadDLL (void)
{
  APIRET rc = 0;

  rc = DosLoadModule (NULL, 0l, (PCSZ) "CAPI", &capiDLL);
  if (rc == 0)
  {
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_REGISTER",
                           (PFN *) & API_REGISTER);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_RELEASE",
                           (PFN *) & API_RELEASE);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_GET_MESSAGE",
                           (PFN *) & API_GET_MESSAGE);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_PUT_MESSAGE",
                           (PFN *) & API_PUT_MESSAGE);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_SET_SIGNAL",
                           (PFN *) & API_SET_SIGNAL);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_GET_MANUFACTURER",
                           (PFN *) & API_GET_MANUFACTURER);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_GET_VERSION",
                           (PFN *) & API_GET_VERSION);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_INSTALLED",
                           (PFN *) & API_INSTALLED);
    rc += BtQueryProcAddr (capiDLL, 0L, (PCSZ) "API_GET_SERIAL_NUMBER",
                           (PFN *) & API_GET_SERIAL_NUMBER);
  }

  return (USHORT) rc;
}

void
CapiUnLoadDLL (void)
{
  if (capiDLL)
    DosFreeModule (capiDLL);
}

USHORT
CapiInstalled (void)
{
  return (API_INSTALLED ());
}

int
CAPIInit (char *EAZs, char *Services)
{
  int x;

  printf ("----------------------------------------------\n");

  if (CapiLoadDLL ())
  {
    printf ("Link with CAPI failed, CAPI.DLL not found?\n");
    return -1;
  }

  x = CAPISetup (EAZs, Services);
  printf ("----------------------------------------------\n");
  return x;
}

void
CAPIStop (void)
{
  UseCAPICID = 0;
  CAPIGoodBye ();
  CapiUnLoadDLL ();
}

#endif /* if defined(OS_2) && !defined(__EMX__) */

/* $Id: capi.c,v 1.7 1999/03/22 01:49:23 mr Exp $ */
