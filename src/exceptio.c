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
 * Filename    : $Source: E:/cvs/btxe/src/exceptio.c,v $
 * Revision    : $Revision: 1.6 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/23 22:28:45 $
 * State       : $State: Exp $
 * Orig. Author: Marc Fiammante, WalkStack From John Currier
 *
 * Description : OS/2 exception handler
 *
 * Note        :
 *   This code need VAC to be called with the following switches:
 *     For the compiler: /Ti
 *     For the Linker  : /B"/LINE /C /DEBUG" (this is the minimum).
 *
 *---------------------------------------------------------------------------*/

#ifdef OS_2

#ifdef EXCEPTIONS

#include "includes.h"
#include "exceptio.h"

#pragma pack(1)

BOOL InForceExit = FALSE;
ULONG Version[2];
BOOL fAlreadyTrapped = FALSE;

void WalkStack (HFILE, PULONG, PULONG, PCONTEXTRECORD, PULONG);
VOID ListModules (HFILE);
void GetSymbol (HFILE trap, CHAR * SymFileName, ULONG Object, ULONG TrapOffset);
APIRET GetLineNum (HFILE hTrap, CHAR * FileName, ULONG Object, ULONG TrapOffset);
BOOL TryToCommit (HFILE hTrap, PVOID address);
int Read32PmDebug (HFILE hTrap, int fh, int TrapSeg, int TrapOff, CHAR * FileName);

ULONG APIENTRY ExceptionHandler (PEXCEPTIONREPORTRECORD pERepRec,
                                 PEXCEPTIONREGISTRATIONRECORD pERegRec,
                                 PCONTEXTRECORD pCtxRec,
                                 PSZ Ausloeser);

// ========================================================================

APIRET
Append (PHFILE pF, PSZ pszName)
{
  ULONG ulPos = 0;
  APIRET rc;

  rc = DosOpen (pszName, pF, &ulPos, 0,
                FILE_NORMAL,
                OPEN_ACTION_CREATE_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_FLAGS_FAIL_ON_ERROR |
                OPEN_FLAGS_SEQUENTIAL |
                OPEN_SHARE_DENYREADWRITE |
                OPEN_ACCESS_READWRITE,
                NULL);
  if (rc == 0)
    DosSetFilePtr (*pF, 0, FILE_END, &ulPos);

  return (rc);
}

// ===========================================================================

static int _System
print (HFILE hf, PSZ szFormat,...)
{
  ULONG egal;
  va_list argptr;
  static CHAR message[512];

  va_start (argptr, szFormat);
  vsprintf (message, szFormat, argptr);
  va_end (argptr);
  message[511] = 0;
  DosWrite (hf, message, strlen (message), &egal);
  return 0;
}

// ===========================================================================

ULONG APIENTRY
GeneralHandler (PEXCEPTIONREPORTRECORD pERepRec,
                PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
#define SHOT    XCPT_ASYNC_PROCESS_TERMINATE
#define DIED    XCPT_PROCESS_TERMINATE
#define CTRL_C  XCPT_SIGNAL

  PTIB ptib;
  PPIB ppib;
  ULONG exnum = pERepRec->ExceptionNum;
  ULONG info1 = pERepRec->ExceptionInfo[1];


  if (InForceExit)
    return (XCPT_CONTINUE_SEARCH);

  if (pERepRec->fHandlerFlags & EH_NESTED_CALL)
    return (XCPT_CONTINUE_SEARCH);

  if (fAlreadyTrapped)
    return (XCPT_CONTINUE_SEARCH);

  if (pERepRec->ExceptionNum == DIED ||
      pERepRec->ExceptionNum == SHOT ||
      pERepRec->ExceptionNum == CTRL_C)
  {
    InForceExit = TRUE;
    return (XCPT_CONTINUE_SEARCH);
  }

  if (exnum == XCPT_GUARD_PAGE_VIOLATION)
  {
    if (TryToCommit (NULLHANDLE, (PVOID) info1))
      return (XCPT_CONTINUE_EXECUTION);

    DosUnsetExceptionHandler (pERegRec);

    if (DosGetInfoBlocks (&ptib, &ppib) == NO_ERROR)
      DosKillThread (ptib->tib_ptib2->tib2_ultid);
    return (XCPT_CONTINUE_SEARCH);
  }

  return (0xABCDABCD);
}

// ===========================================================================

ULONG APIENTRY
exHandler (PEXCEPTIONREPORTRECORD pERepRec,
           PEXCEPTIONREGISTRATIONRECORD pERegRec,
           PCONTEXTRECORD pCtxRec,
           PVOID p)
{
  ULONG result;

  p = p;
  result = GeneralHandler (pERepRec, pERegRec);
  if (result != 0xABCDABCD)
    return (result);
  else
    return (ExceptionHandler (pERepRec, pERegRec, pCtxRec, "binkley"));
}

// ===========================================================================

ULONG APIENTRY
ExceptionHandler (PEXCEPTIONREPORTRECORD pERepRec,
                  PEXCEPTIONREGISTRATIONRECORD pERegRec,
                  PCONTEXTRECORD pCtxRec,
                  PSZ Ausloeser)
{
  HFILE hTrap = HF_STDOUT;
  HMODULE hMod;
  APIRET rc;
  PTIB ptib;
  PPIB ppib;
  ULONG ObjNum;
  ULONG Offset;
  ULONG Nest;
  ULONG info0 = pERepRec->ExceptionInfo[0];
  ULONG info1 = pERepRec->ExceptionInfo[1];
  static CHAR TrapFile[20];
  static CHAR Buffer[CCHMAXPATH];
  static CHAR Name[CCHMAXPATH];
  static ULONG NumFH;
  static LONG ReqFH = 1;
  static CHAR t[100], a[100];

  // ------------------------------------------------------------------------
  fAlreadyTrapped = TRUE;
  // ------------------------------------------------------------------------
  DosEnterMustComplete (&Nest);

  rc = DosGetInfoBlocks (&ptib, &ppib);

  if (rc == NO_ERROR)
    sprintf (TrapFile, "bt32%04x.trp", ptib->tib_ptib2->tib2_ultid);
  else
    sprintf (TrapFile, "postmort.log");

  rc = DosSetRelMaxFH (&ReqFH, &NumFH);
  Append (&hTrap, TrapFile);
  if (hTrap == NULLHANDLE)
  {
    printf ("htrap NULL\n");
    hTrap = (HFILE) HF_STDOUT;
  }

  // ------------------------------------------------------------------------
  switch (pERepRec->ExceptionNum)
  {
  case XCPT_GUARD_PAGE_VIOLATION:
    sprintf (a, "guard page [type:");
    switch (info0)
    {
    case XCPT_READ_ACCESS:
      strcat (a, "read]");
      break;
    case XCPT_WRITE_ACCESS:
      strcat (a, "write]");
      break;
    default:
      strcat (a, "????]");
      break;
    }
    sprintf (t, "%s data:%08x", a, info1);
    break;

    // ------------------------------------------------------------------------
  case XCPT_ACCESS_VIOLATION:
    sprintf (a, "access violation [Type: ");
    switch (info0)
    {
    case XCPT_UNKNOWN_ACCESS:
      strcat (a, "unknown ");
      break;
    case XCPT_READ_ACCESS:
      strcat (a, "read from");
      break;
    case XCPT_WRITE_ACCESS:
      strcat (a, "write to");
      break;
    case XCPT_EXECUTE_ACCESS:
      strcat (a, "execution of");
      break;
    case XCPT_LIMIT_ACCESS:
      strcat (a, "limit");
      break;
    case XCPT_SPACE_ACCESS:
      sprintf (Buffer,
               "Space : Invalid Selector: %8.8p",
               info1 ? info1 + 1 : 0);
      strcat (a, Buffer);
      break;

    default:
      strcat (a, "????]");
      break;
    }
    sprintf (t, "%s Data:%08x]", a, info1);
    break;

  case XCPT_ASYNC_PROCESS_TERMINATE:
    sprintf (t, "Async process terminate [via thread %08x]", info0);
    break;

    // ------------------------------------------------------------------------
  case XCPT_UNABLE_TO_GROW_STACK:
    sprintf (t, "unable to grow stack");
    break;
  case XCPT_IN_PAGE_ERROR:
    sprintf (t, "in page error");
    break;
  case XCPT_INTEGER_DIVIDE_BY_ZERO:
    sprintf (t, "IDiv/0");
    break;
  case XCPT_FLOAT_DIVIDE_BY_ZERO:
    sprintf (t, "FDiv/0");
    break;
  case XCPT_FLOAT_INVALID_OPERATION:
    sprintf (t, "float inval");
    break;
  case XCPT_ILLEGAL_INSTRUCTION:
    sprintf (t, "invalid instruction");
    break;
  case XCPT_PRIVILEGED_INSTRUCTION:
    sprintf (t, "IOPL not enough");
    break;
  case XCPT_INTEGER_OVERFLOW:
    sprintf (t, "int overflow");
    break;
  case XCPT_FLOAT_OVERFLOW:
    sprintf (t, "float overflow");
    break;
  case XCPT_FLOAT_UNDERFLOW:
    sprintf (t, "float underflow");
    break;
  case XCPT_FLOAT_DENORMAL_OPERAND:
    sprintf (t, "float denormal operand");
    break;
  case XCPT_FLOAT_INEXACT_RESULT:
    sprintf (t, "inexakt float result");
    break;
  case XCPT_FLOAT_STACK_CHECK:
    sprintf (t, "float stack check");
    break;
  case XCPT_DATATYPE_MISALIGNMENT:
    sprintf (t, "data misalignment");
    break;
  case XCPT_BREAKPOINT:
    sprintf (t, "breakpoint");
    break;
  case XCPT_SINGLE_STEP:
    sprintf (t, "single step");
    break;
  case XCPT_INVALID_LOCK_SEQUENCE:
    sprintf (t, "inval. lock sequ");
    break;
  case XCPT_ARRAY_BOUNDS_EXCEEDED:
    sprintf (t, "array bounds exceeded");
    break;
  case XCPT_UNWIND:
    sprintf (t, "unwind");
    break;
  case XCPT_SIGNAL:
    sprintf (t, "signal exception");
    break;

    /* non-continue exeptions: */
  case XCPT_PROCESS_TERMINATE:
    sprintf (t, "process terminated");
    break;
  case XCPT_NONCONTINUABLE_EXCEPTION:
    sprintf (t, "exception #4711");
    break;
  case XCPT_INVALID_DISPOSITION:
    sprintf (t, "invalid disposition");
    break;
  case XCPT_BAD_STACK:
    sprintf (t, "bad stack");
    break;
  case XCPT_INVALID_UNWIND_TARGET:
    sprintf (t, "invalid unwind target");
    break;
  }

  // ------------------------------------------------------------------------

  print (hTrap, "\n-------------------------------------\n");

  print (hTrap, "%s %s\n", _strdate (a), _strtime (Buffer));
  print (hTrap, "exception   : %08x\n", pERepRec->ExceptionNum);
  print (hTrap, "address     : %08x\n", pERepRec->ExceptionAddress);
  print (hTrap, "source      : %s\n", Ausloeser);
  print (hTrap, "information : %08x, %08x\n", info0, info1);

  // ------------------------------------------------------------------------

  rc = DosQuerySysInfo (QSV_VERSION_MAJOR, QSV_VERSION_MINOR,
                        Version, sizeof (Version));

  if ((rc == 0) && (Version[0] >= 20) && (Version[1] >= 10))
  {
    /* version must be over 2.1 for DOSQUERYMODFROMEIP */
    print (hTrap, "OS/2 Version %d.%d\n", Version[0] / 10, Version[1]);
    rc = DOSQUERYMODFROMEIP (&hMod, &ObjNum, CCHMAXPATH,
                             Name, &Offset, pERepRec->ExceptionAddress);
    if (rc == 0)
    {
      print (hTrap, "code modul (internal name): %s\n", Name);
      rc = DosQueryModuleName (hMod, CCHMAXPATH, Name);
      print (hTrap, "code modul (file name)    : %s\n", Name);
      print (hTrap, "code object # %d at offset %x \n", ObjNum + 1, Offset);
      print (hTrap, "\n");

      print (hTrap, "      File     Line#  Public Symbol\n");
      print (hTrap, "  ÄÄÄÄÄÄÄÄÄÄÄÄ ÄÄÄÄÄ  ÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");
      rc = GetLineNum (hTrap, Name, ObjNum, Offset);
      /* if no codeview try with symbol files */
      if (rc != 0)
      {
        strcpy (Name + strlen (Name) - 3, "SYM");  /* Get Sym File name */
        GetSymbol (hTrap, Name, ObjNum, Offset);
      }
    }
    else
      print (hTrap, "invalid execution address\n");
  }

  // ------------------------------------------------------------------------

  print (hTrap, "-------------------------------------\n");
  print (hTrap, "plain text  : %s\n", t);
  print (hTrap, "-------------------------------------\n");

  // ------------------------------------------------------------------------
  // Termination Exception?
  // ------------------------------------------------------------------------

  if ((pERepRec->ExceptionNum != XCPT_UNWIND) &&
      (pERepRec->ExceptionNum != XCPT_PROCESS_TERMINATE) &&
      (pERepRec->ExceptionNum != XCPT_ASYNC_PROCESS_TERMINATE)
    )
  {
    goto LABEL_STOP_PROCESS;
  }

  // ======================================================================

  if (pERepRec->ExceptionNum == XCPT_ACCESS_VIOLATION &&
      pERepRec->ExceptionAddress != (PVOID) XCPT_DATA_UNKNOWN)
  {
    if ((pERepRec->ExceptionInfo[0] == XCPT_READ_ACCESS ||
         pERepRec->ExceptionInfo[0] == XCPT_WRITE_ACCESS) &&
        pERepRec->ExceptionInfo[1] != XCPT_DATA_UNKNOWN)
    {
      print (hTrap, "\npage fault\n");

      if (TryToCommit (hTrap, (PVOID) pERepRec->ExceptionInfo[1]))
      {
        if (hTrap != HF_STDOUT)
          DosClose (hTrap);
        DosExitMustComplete (&Nest);
        fAlreadyTrapped = FALSE;
        return (XCPT_CONTINUE_EXECUTION);
      }
    }
  }

  // ------------------------------------------------------------------------
  //  Thread-Section
  // ------------------------------------------------------------------------

  rc = DosGetInfoBlocks (&ptib, &ppib);
  if (rc == NO_ERROR)
  {
    rc = DosQueryModuleName (ppib->pib_hmte, CCHMAXPATH, Name);
    if (rc == NO_ERROR)
    {
      WalkStack (hTrap,
                 (PULONG) ptib->tib_pstack,
                 (PULONG) ptib->tib_pstacklimit,
                 (PCONTEXTRECORD) pCtxRec,
                 (PULONG) pERepRec->ExceptionAddress);
    }
  }

  // ------------------------------------------------------------------------

LABEL_STOP_PROCESS:
  if (hTrap != HF_STDOUT)
    DosClose (hTrap);

  UnLoadOS2DLLs ();

  printf ("exception  %8.8lX\n", pERepRec->ExceptionNum);
  printf ("creating   %s\n", TrapFile);
  printf ("source     %s\n", Ausloeser);

  DosExitMustComplete (&Nest);

  if ((pERepRec->fHandlerFlags & EH_NONCONTINUABLE) == 0)
    return (XCPT_CONTINUE_EXECUTION);

  printf ("exiting\n");

  ListModules (hTrap);

  DosError (FERR_ENABLEEXCEPTION | FERR_ENABLEHARDERR);
  DosSetPriority (PRTYS_PROCESS, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0);
  DosUnsetExceptionHandler (pERegRec);

  fAlreadyTrapped = FALSE;
  InForceExit = TRUE;
  return (XCPT_CONTINUE_SEARCH);
}

// ===========================================================================

BOOL
TryToCommit (HFILE hTrap, PVOID address)
{
  ULONG ulMemSize, flMemAttrs;
  APIRET rc;

  /* query the memory to find out why we faulted. */
  ulMemSize = 1;

  DosQueryMem ((PVOID) address, &ulMemSize, &flMemAttrs);

  /* If the memory is free or committed, */
  /* we have some other problem.         */
  /* If it is not free or not committed, commit it. */

  if (!(flMemAttrs & (PAG_FREE | PAG_COMMIT)))
  {
    if (hTrap != NULLHANDLE)
      print (hTrap, "\ntry to commit uncommitted memory\n");

    rc = DosSetMem ((PVOID) address, 4096, PAG_DEFAULT | PAG_COMMIT);
    if (rc == 0)
      return (TRUE);

    if (hTrap != NULLHANDLE)
      print (hTrap, "\nerror committing\n");
  }
  return (FALSE);
}

// ===========================================================================

VOID
ListModules (HFILE hTrap)
{
  APIRET rc;
  PVOID BaseAddress = (PVOID) 0x10000;
  ULONG RegionSize = 0x3FFFFFFF;
  ULONG AllocationFlags;
  ULONG ObjNum;
  ULONG Offset;
  HMODULE hMod;
  HMODULE LastModule = 0;
  static UCHAR Name[256];
  ULONG Size;

  print (hTrap, "ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
  print (hTrap, "³ list of currently accessed modules (DLLs) object addresses              ³");

  rc = DosQueryMem (BaseAddress, &RegionSize, &AllocationFlags);

  while (rc == NO_ERROR)
  {
    if ((AllocationFlags & PAG_EXECUTE) &&
        (AllocationFlags & PAG_BASE))
    {
      rc = DOSQUERYMODFROMEIP (&hMod, &ObjNum, CCHMAXPATH,
                               Name, &Offset, BaseAddress);
      //==========================================
      if (rc == 0)
      {
        if (hMod != LastModule)
        {
          memset (&Name, 0, sizeof (Name));
          rc = DosQueryModuleName (hMod, sizeof (Name), Name);
          print (hTrap, "\n³ module %048s handle %08d ³", Name, hMod);
          LastModule = hMod;
        }
        //==========================================

        DOS16SIZESEG (SELECTOROF (BaseAddress), &Size);
      }
    }
    if (AllocationFlags & PAG_FREE)
      RegionSize = 0x10000;
    RegionSize += 0x0FFF;
    RegionSize &= 0xFFFFF000;
    BaseAddress = (PVOID) (((PCHAR) BaseAddress) + RegionSize);
    RegionSize = ((PCHAR) 0x3FFFFFFF) - (PCHAR) BaseAddress;
    rc = DosQueryMem (BaseAddress, &RegionSize, &AllocationFlags);

    while ((rc == ERROR_INVALID_ADDRESS) ||
           (rc == ERROR_NO_OBJECT))
    {
      BaseAddress = (PVOID) (((PCHAR) BaseAddress) + 0x10000);

      if (BaseAddress > (PVOID) 0x3FFFFFFF)
        break;

      RegionSize = ((PCHAR) 0x3FFFFFFF) - (PCHAR) BaseAddress;
      rc = DosQueryMem (BaseAddress, &RegionSize, &AllocationFlags);
    }
    if (BaseAddress > (PVOID) 0x3FFFFFFF)
      break;
  }
  print (hTrap, "\nÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
}

// ===========================================================================

void
GetSymbol (HFILE hTrap, CHAR * SymFileName, ULONG Object, ULONG TrapOffset)
{
  static FILE *SymFile;
  static MAPDEF MapDef;
  static SEGDEF SegDef;
  static SYMDEF32 SymDef32;
  static SYMDEF16 SymDef16;
  static char Buffer[256];
  static int SegNum, SymNum, LastVal;
  static unsigned short int SegOffset, SymOffset, SymPtrOffset;

  SymFile = fopen (SymFileName, read_binary);
  if (SymFile == 0)
  {
    print (hTrap, "  could not open symbol file %s\n", SymFileName);
    return;
  }                             /* endif */

  fread (&MapDef, sizeof (MAPDEF), 1, SymFile);
  SegOffset = SEGDEFOFFSET (MapDef);

  for (SegNum = 0; SegNum < MapDef.cSegs; SegNum++)
  {
    /* printf("Scanning segment #%d Offset %4.4hX\n",SegNum+1,SegOffset); */
    if (fseek (SymFile, SegOffset, SEEK_SET))
    {
      print (hTrap, "seek error ");
      break;
    }

    fread (&SegDef, sizeof (SEGDEF), 1, SymFile);
    if (SegNum == Object)
    {
      Buffer[0] = 0x00;
      LastVal = 0;

      for (SymNum = 0; SymNum < SegDef.cSymbols; SymNum++)
      {
        SymPtrOffset = SYMDEFOFFSET (SegOffset, SegDef, SymNum);
        fseek (SymFile, SymPtrOffset, SEEK_SET);
        fread (&SymOffset, sizeof (unsigned short int), 1, SymFile);

        fseek (SymFile, SymOffset + SegOffset, SEEK_SET);
        if (SegDef.bFlags & 0x01)
        {
          fread (&SymDef32, sizeof (SYMDEF32), 1, SymFile);
          if (SymDef32.wSymVal > TrapOffset)
            print (hTrap, " between %s + %X ", Buffer, TrapOffset - LastVal);

          LastVal = SymDef32.wSymVal;
          Buffer[0] = SymDef32.achSymName[0];
          fread (&Buffer[1], 1, SymDef32.cbSymName, SymFile);
          Buffer[SymDef32.cbSymName] = 0x00;

          if (SymDef32.wSymVal > TrapOffset)
          {
            print (hTrap, "and %s - %X\n", Buffer, LastVal - TrapOffset);
            break;
          }

          /*printf("32 Bit Symbol <%s> Address %p\n",Buffer,SymDef32.wSymVal); */
        }
        else
        {
          fread (&SymDef16, sizeof (SYMDEF16), 1, SymFile);
          if (SymDef16.wSymVal > TrapOffset)
            print (hTrap, " between %s + %X ", Buffer, TrapOffset - LastVal);

          LastVal = SymDef16.wSymVal;
          Buffer[0] = SymDef16.achSymName[0];
          fread (&Buffer[1], 1, SymDef16.cbSymName, SymFile);
          Buffer[SymDef16.cbSymName] = 0x00;

          if (SymDef16.wSymVal > TrapOffset)
          {
            print (hTrap, "and %s - %X\n", Buffer, LastVal - TrapOffset);
            break;
          }

          /*printf("16 Bit Symbol <%s> Address %p\n",Buffer,SymDef16.wSymVal); */
        }
      }

      break;
    }

    SegOffset = NEXTSEGDEFOFFSET (SegDef);
  }

  fclose (SymFile);
}


// ===========================================================================
// ===========================================================================
/* Better New WalkStack From John Currier */

static void
WalkStack (HFILE hTrap,
           PULONG StackBottom,
           PULONG StackTop,
           PCONTEXTRECORD pCtxRec,
           PULONG ExceptionAddress)
{
  BOOL f32bit;
  BOOL fExceptionAddress = TRUE;  // Use Exception Addr 1st time thru

  PULONG RetAddr, LastEbp, Ebp;
  APIRET rc;
  ULONG Size, Attr;
  HMODULE hMod;
  ULONG ObjNum;
  ULONG Offset;
  USHORT Cs, Ip, Bp, Sp;
  USHORT count = 0;
  CHAR Name[CCHMAXPATH];

  StackTop = StackTop;

  // man muss doch nur auf dem stack nach cs:eip suchen,
  // um den stack-anfang zu bekommen?!?

  // Note: we can't handle stacks bigger than 64K for now...

  Sp = (USHORT) (((ULONG) StackBottom) >> 16);
  Bp = (USHORT) pCtxRec->ctx_RegEbp;
  Ebp = (PULONG) pCtxRec->ctx_RegEbp;
  RetAddr = ExceptionAddress;

  print (hTrap, "\ncall stack:\n");
  print (hTrap, "                                           source    line      nearest\n");
  print (hTrap, "      EBP      address    module  obj#      file     numbr  public symbol\n");
  print (hTrap, "    ÄÄÄÄÄÄÄÄ  ÄÄÄÄÄÄÄÄÄ  ÄÄÄÄÄÄÄÄ ÄÄÄÄ  ÄÄÄÄÄÄÄÄÄÄÄÄ ÄÄÄÄÄ  ÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");

  for (;;)
  {
    count++;
    print (hTrap, "%3d", count);
    Size = 10;
    rc = DosQueryMem ((PVOID) (Ebp + 2), &Size, &Attr);
    if (rc != NO_ERROR || !(Attr & PAG_COMMIT) || Size < 10)
    {
      print (hTrap, "invalid EBP: %8.8p\n", Ebp);
      break;
    }

    if ((ULONG) RetAddr == 0x00000053)
    {
      // versuch, thunking (und deren stackframe) zu fixen
      Ebp += 22;
      RetAddr = (PULONG) * (Ebp + 2);
    }

    // Get the (possibly) 16bit CS and IP
    if (fExceptionAddress)
    {
      Cs = (USHORT) (((ULONG) ExceptionAddress) >> 16);
      Ip = (USHORT) (ULONG) ExceptionAddress;
    }
    else
    {
      Cs = *(Ebp + 2);
      Ip = *(Ebp + 1);
    }

    // if the return address points to the stack then it's really just
    // a pointer to the return address (UGH!).

    if ((USHORT) ((ULONG) RetAddr >> 16) == Sp)
    {
      RetAddr = (PULONG) * (RetAddr);
    }
    if (Ip == 0 && *Ebp == 0)
    {
      // End of the stack so these are both shifted by 2 bytes:
      Cs = *(Ebp + 3);
      Ip = *(Ebp + 2);
    }

    // 16bit programs have on the stack:
    //   BP:IP:CS
    //   where CS may be thunked
    //
    //         in dump                 swapped
    //    BP        IP   CS          BP   CS   IP
    //   4677      53B5 F7D0        7746 D0F7 B553
    //
    // 32bit programs have:
    //   EBP:EIP
    // and you'd have something like this (with SP added) (not
    // accurate values)
    //
    //         in dump               swapped
    //      EBP       EIP         EBP       EIP
    //   4677 2900 53B5 F7D0   0029 7746 D0F7 B553
    //
    // So the basic difference is that 32bit programs have a 32bit
    // EBP and we can attempt to determine whether we have a 32bit
    // EBP by checking to see if its 'selector' is the same as SP.
    // Note that this technique limits us to checking stacks < 64K.
    //
    // Soooo, if IP (which maps into the same USHORT as the swapped
    // stack page in EBP) doesn't point to the stack (i.e. it could
    // be a 16bit IP) then see if CS is valid (as is or thunked).
    //
    // Note that there's the possibility of a 16bit return address
    // that has an offset that's the same as SP so we'll think it's
    // a 32bit return address and won't be able to successfully resolve
    // its details.

    f32bit = TRUE;

    if (!fExceptionAddress)
      if (Ip != Sp)
      {
        if (DOS16SIZESEG (Cs, &Size) == NO_ERROR)
        {
          f32bit = FALSE;
        }
        else if (DOS16SIZESEG ((Cs << 3) + 7, &Size) == NO_ERROR)
        {
          Cs = (Cs << 3) + 7;
          f32bit = FALSE;
        }
        if (!f32bit)
          RetAddr = (PULONG) (USHORT * _Seg16) MAKEULONG (Ip, Cs);
      }

    if (fExceptionAddress)
      print (hTrap, " trap  ->");
    else
      print (hTrap, " %8.8p", Ebp);

    if (f32bit)
      print (hTrap, "   %8.8p", RetAddr);
    else
      print (hTrap, " %04.04X:%04.04X", Cs, Ip);

    if (Version[0] >= 20 && Version[1] >= 10)
    {
      // Make a 'tick' sound to let the user know we're still alive
      DosBeep (2000, 2);

      rc = DosQueryMem ((PVOID) RetAddr, &Size, &Attr);
      if ((rc != NO_ERROR)
          || !(Attr & PAG_COMMIT)
          || (Size < 10))
      {
        print (hTrap, " invalid retaddr: %8.8p\n", RetAddr);
        print (hTrap, " DosQueryMem returned: rc=%08x, size=%08x, attr= %08x\n",
               rc, Size, Attr);
        break;                  /* avoid infinite loops */
      }
      else
      {
        rc = DOSQUERYMODFROMEIP (&hMod, &ObjNum, sizeof (Name),
                                 Name, &Offset, (PVOID) RetAddr);
        if (rc == NO_ERROR && ObjNum != -1)
        {
          static char szJunk[_MAX_FNAME];
          static char szName[_MAX_FNAME];

          DosQueryModuleName (hMod, sizeof (Name), Name);
          _splitpath (Name, szJunk, szJunk, szName, szJunk);
          print (hTrap, "  %-8s %04X", szName, ObjNum + 1);

          strcpy (Name + strlen (Name) - 3, "SYM");
          GetSymbol (hTrap, Name, ObjNum, (ULONG) Offset);
        }
        else
          print (hTrap, "  *unknown*");
      }
    }

    print (hTrap, "\n");

    Bp = (USHORT) * Ebp;
    if (Bp == 0 && (*Ebp + 1) == 0)
      break;

    if (!fExceptionAddress)
    {
      LastEbp = Ebp;
      Ebp = (PULONG) MAKEULONG (Bp, Sp);

      if (Ebp < LastEbp)
      {
        print (hTrap, "lost stack chain - new EBP below previous\n");
        break;
      }
    }
    else
      fExceptionAddress = FALSE;

    Size = 4;
    rc = DosQueryMem ((PVOID) Ebp, &Size, &Attr);
    if ((rc != NO_ERROR) || (Size < 4))
    {
      print (hTrap, "lost stack chain - invalid EBP: %8.8p\n", Ebp);
      break;
    }

    RetAddr = (PULONG) * (Ebp + 2);
  }

  print (hTrap, "end of call stack\n\n");
}

// ===========================================================================

#ifdef __WATCOMC__
   #define BYTE
#endif
#ifndef WORD
  #define WORD  short int
#endif

#include <exe.h>
#ifdef __WATCOMC__
   #define EXE386 1
#endif
#include <newexe.h>
#define  FOR_EXEHDR  1          /* avoid define conflicts between newexe.h and exe386.h */
#ifndef DWORD
  #define DWORD long int
#endif
#ifndef WORD
  #define WORD  short int
#endif
#include <exe386.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <share.h>
#include <io.h>

/* ------------------------------------------------------------------ */
/* Last 8 bytes of 16:16 file when CODEVIEW debugging info is present */

struct _eodbug
{
  unsigned short dbug;          /* 'NB' signature */
  unsigned short ver;           /* version        */
  unsigned long dfaBase;        /* size of codeview info */
}
eodbug;

#define         DBUGSIG         0x424E
#define         SSTMODULES      0x0101
#define         SSTPUBLICS      0x0102
#define         SSTTYPES        0x0103
#define         SSTSYMBOLS      0x0104
#define         SSTSRCLINES     0x0105
#define         SSTLIBRARIES    0x0106
#define         SSTSRCLINES2    0x0109
#define         SSTSRCLINES32   0x010B

struct _base
{
  unsigned short dbug;          /* 'NB' signature */
  unsigned short ver;           /* version        */
  unsigned long lfoDir;         /* file offset to dir entries */
}
base;

struct ssDir
{
  unsigned short sst;           /* SubSection Type */
  unsigned short modindex;      /* Module index number */
  unsigned long lfoStart;       /* Start of section */
  unsigned short cb;            /* Size of section */
};

struct ssDir32
{
  unsigned short sst;           /* SubSection Type */
  unsigned short modindex;      /* Module index number */
  unsigned long lfoStart;       /* Start of section */
  unsigned long cb;             /* Size of section */
};

struct ssModule
{
  unsigned short csBase;        /* code segment base */
  unsigned short csOff;         /* code segment offset */
  unsigned short csLen;         /* code segment length */
  unsigned short ovrNum;        /* overlay number */
  unsigned short indxSS;        /* Index into sstLib or 0 */
  unsigned short reserved;
  char csize;                   /* size of prefix string */
}
ssmod;

struct ssModule32
{
  unsigned short csBase;        /* code segment base */
  unsigned long csOff;          /* code segment offset */
  unsigned long csLen;          /* code segment length */
  unsigned long ovrNum;         /* overlay number */
  unsigned short indxSS;        /* Index into sstLib or 0 */
  unsigned long reserved;
  char csize;                   /* size of prefix string */
}
ssmod32;

struct ssPublic
{
  unsigned short offset;
  unsigned short segment;
  unsigned short type;
  char csize;
}
sspub;

struct ssPublic32
{
  unsigned long offset;
  unsigned short segment;
  unsigned short type;
  char csize;
}
sspub32;

typedef struct _SSLINEENTRY32
{
  unsigned short LineNum;
  unsigned short FileNum;
  unsigned long Offset;
}
SSLINEENTRY32;

typedef struct _FIRSTLINEENTRY32
{
  unsigned short LineNum;
  unsigned short FileNum;
  unsigned short numlines;
  unsigned short segnum;
}
FIRSTLINEENTRY32;

typedef struct _SSFILENUM32
{
  unsigned long first_displayable;  /* Not used */
  unsigned long number_displayable;  /* Not used */
  unsigned long file_count;     /* number of source files */
}
SSFILENUM32;

struct DbugRec
{                               /* debug info struct ure used in linked * list */
  struct DbugRec far *pnext;    /* next node *//* 013 */
  char far *SourceFile;         /* source file name *013 */
  unsigned short TypeOfProgram; /* dll or exe *014* */
  unsigned short LineNumber;    /* line number in source file */
  unsigned short OffSet;        /* offset into loaded module */
  unsigned short Selector;      /* code segment 014 */
  unsigned short OpCode;        /* Opcode replaced with BreakPt */
  unsigned long Count;          /* count over Break Point */
};

typedef struct DbugRec DBUG, far * DBUGPTR;  /* 013 */
char szNrPub[128];
char szNrLine[128];
struct new_seg *pseg;
struct o32_obj *pobj;           /* Flat .EXE object table entry */
struct ssDir *pDirTab;
struct ssDir32 *pDirTab32;
unsigned char *pEntTab;
unsigned long lfaBase;

/* ------------------------------------------------------------------ */

APIRET
GetLineNum (HFILE hTrap, CHAR * FileName, ULONG Object, ULONG TrapOffset)
{
  APIRET rc;
  FILE *ModuleFile;
  static struct exe_hdr old;
  static struct new_exe new;

#ifndef __WATCOMC__

  static struct e32_exe e32;

#endif

  strcpy (szNrPub, "   none found\n");
  strcpy (szNrLine, "   none found\n");

  ModuleFile = share_fopen (FileName, read_binary, SH_DENYNO);
  if (ModuleFile != NULL)
  {
    /* Read old Exe header */
    if (fread ((void *) &old, sizeof (char), 64, ModuleFile) != 64)
    {
      print (hTrap, "could not read old exe header %d\n", errno);
      fclose (ModuleFile);
      return 2;
    }

    /* Seek to new Exe header */
    if (fseek (ModuleFile, (long) E_LFANEW (old), SEEK_SET))
    {
      print (hTrap, "could not seek to new exe header %d\n", errno);
      fclose (ModuleFile);
      return 3;
    }

    if (fread ((void *) &new, sizeof (char), 64, ModuleFile) != 64)
    {
      print (hTrap, "could not read new exe header %d\n", errno);
      fclose (ModuleFile);
      return 4;
    }

    /* Check EXE signature */
    if (NE_MAGIC (new) == E32MAGIC)
    {
#ifndef __WATCOMC__
      fseek (ModuleFile, (long) E_LFANEW (old), SEEK_SET);

      if (fread ((void *) &e32, sizeof (e32), 1, ModuleFile) != 1)
      {
        print (hTrap, "could not read LX header %d\n", errno);
        fclose (ModuleFile);
        return 4;
      }

      /* Flat 32 executable */

      if (e32.e32_debuginfo == 0)
      {
        print (hTrap, "no debug info in exe file\n");
        fclose (ModuleFile);
        return 4;
      }

      if (fseek (ModuleFile, e32.e32_debuginfo + e32.e32_debuglen - 8, SEEK_SET) == -1L)
      {
        print (hTrap, "could not seek to debug info %d\n", errno);
        fclose (ModuleFile);
        return 4;
      }
#else

      fclose (ModuleFile);
      strcpy (FileName + strlen (FileName) - 3, "SYM");  /* Get Sym File name */
      ModuleFile = share_fopen (FileName, read_binary, SH_DENYNO);
      if (ModuleFile != NULL)
      {
        fseek (ModuleFile, (long) -8, SEEK_END);
#endif

        rc = Read32PmDebug (hTrap, fileno (ModuleFile), Object + 1, TrapOffset, FileName);
        if (rc == 0)
        {
          print (hTrap, "nearest public symbol : %s", szNrPub);
          print (hTrap, "nearest source line   : %s", szNrLine);
        }

        fclose (ModuleFile);

#ifdef __WATCOMC__
      }
#endif

      return rc;
    }
    else
    {                           /* Unknown executable */
      print (hTrap, "could not find exe signature");
      fclose (ModuleFile);
      return 11;
    }
  }                             /* Read new Exe header */
  else
  {
    print (hTrap, "could not open module file %d", errno);
    return 1;
  }                             /* endif */
}

char fname[128], ModName[80];
char ename[128], dummy[128];

int
Read32PmDebug (HFILE hTrap, int fh, int TrapSeg, int TrapOff, CHAR * FileName)
{
  static NrPublic, NrFile, numdir, namelen, numlines;
  static int ModIndex;
  static int bytesread, i, j;
  static int pOffset;
  static SSLINEENTRY32 LineEntry;
  static SSFILENUM32 FileInfo;
  static FIRSTLINEENTRY32 FirstLine;

  ModIndex = 0;

  if (read (fh, (void *) &eodbug, 8) == -1)
  {
    print (hTrap, "error %u reading debug info from %s\n", errno, FileName);
    return (19);
  }
  if (eodbug.dbug != DBUGSIG)
  {
    print (hTrap, "\nno codeview information stored (%08x)\n", eodbug.dbug);
    return (0);
  }

  if ((lfaBase = lseek (fh, -eodbug.dfaBase, SEEK_CUR)) == -1L)
  {
    print (hTrap, "error %u seeking base codeview data in %s\n", errno, FileName);
    return (20);
  }

  if (read (fh, (void *) &base, 8) == -1)
  {
    print (hTrap, "error %u reading base codeview data in %s\n", errno, FileName);
    return (21);
  }

  if (lseek (fh, base.lfoDir - 8 + 4, SEEK_CUR) == -1)
  {
    print (hTrap, "error %u seeking dir codeview data in %s\n", errno, FileName);
    return (22);
  }

  if (read (fh, (void *) &numdir, 4) == -1)
  {
    print (hTrap, "error %u reading dir codeview data in %s\n", errno, FileName);
    return (23);
  }

  /* Read dir table into buffer */
  if ((pDirTab32 = (struct ssDir32 *) calloc (numdir, sizeof (struct ssDir32))) == NULL)
  {
    print (hTrap, "out of memory!");
    return (-1);
  }

  if (read (fh, (void *) pDirTab32, numdir * sizeof (struct ssDir32)) == -1)
  {
    print (hTrap, "error %u reading codeview dir table from %s\n", errno, FileName);
    free (pDirTab32);
    return (24);
  }

  i = 0;
  while (i < numdir)
  {
    if (pDirTab32[i].sst != SSTMODULES)
    {
      i++;
      continue;
    }

    NrPublic = 0;
    NrFile = 0;
    /* point to subsection */
    lseek (fh, pDirTab32[i].lfoStart + lfaBase, SEEK_SET);
    read (fh, (void *) &ssmod32.csBase, sizeof (ssmod32));
    read (fh, (void *) ModName, (unsigned) ssmod32.csize);
    ModIndex = pDirTab32[i].modindex;
    ModName[ssmod32.csize] = '\0';
    i++;
    while (pDirTab32[i].modindex == ModIndex && i < numdir)
    {
      /* point to subsection */
      lseek (fh, pDirTab32[i].lfoStart + lfaBase, SEEK_SET);
      switch (pDirTab32[i].sst)
      {
      case SSTPUBLICS:
        bytesread = 0;
        while (bytesread < pDirTab32[i].cb)
        {
          bytesread += read (fh, (void *) &sspub32.offset, sizeof (sspub32));
          bytesread += read (fh, (void *) ename, (unsigned) sspub32.csize);
          ename[sspub32.csize] = '\0';
          if ((sspub32.segment == TrapSeg)
              && (sspub32.offset <= TrapOff)
              && (sspub32.offset >= NrPublic))
          {
            NrPublic = sspub32.offset;
            sprintf (szNrPub, "%04X:%08X%s %s  (%s)\n",
                     sspub32.segment, sspub32.offset,
                     (sspub32.type == 1) ? " abs" : " ", ename, ModName);
          }
        }
        break;

      case SSTSRCLINES32:
        if (TrapSeg != ssmod32.csBase)
          break;

        /* read first line */
        read (fh, (void *) &FirstLine, sizeof (FirstLine));

        if (FirstLine.LineNum != 0)
        {
          print (hTrap, "missing line table information\n");
          break;
        }                       /* endif */

        numlines = FirstLine.numlines;

        lseek (fh, 16, SEEK_CUR);

        {
          int x;
          char junk = 1;
          char name[CCHMAXPATH];

          /* since VAC includes the name of all header files into the EXE,
           * we have to read them (they are pascal strings) to the end. The
           * value "numlines" contains the number of filenames to read, but
           * better we check for null-length strings, too. After that
           * filenames the line number information indeed comes. (r.hoerner) */

          for (j = 0; junk && j < numlines; j++)
          {
            read (fh, (void *) &junk, 1);
            x = read (fh, name, junk);
            x = x;

            /*
             *  de-comment this and you'll get all header file names to the
             *  trapfile
             *
             *  if (x > 0)
             *  {
             *    name[x ] = 0;
             *    print(hTrap,"%s\n",name);
             *  }
             */
          }
        }

        lseek (fh, 4, SEEK_CUR);
        read (fh, (void *) &LineEntry, sizeof (LineEntry));
        numlines = LineEntry.LineNum;

        for (j = 0; j < numlines; j++)
        {
          int offset;

          read (fh, (void *) &LineEntry, sizeof (LineEntry));

          offset = ssmod32.csOff + LineEntry.Offset;

          if (LineEntry.LineNum == 0)
            if (offset != ssmod32.csOff)
              break;

          if (offset <= TrapOff)
          {
            NrFile = LineEntry.FileNum;
            pOffset = sprintf (szNrLine, "%04X,%08X  line #%hu",
                               ssmod32.csBase,
                               offset,
                               LineEntry.LineNum);

            if (offset == TrapOff)
            {
              i = numdir;
              break;
            }
          }
        }

        if (NrFile != 0)
        {
          read (fh, (void *) &FileInfo, sizeof (FileInfo));
          namelen = 0;
          for (j = 1; j <= FileInfo.file_count; j++)
          {
            namelen = 0;
            read (fh, (void *) &namelen, 1);
            read (fh, (void *) ename, namelen);
            if (j == NrFile)
              break;
          }
          ename[namelen] = '\0';
          pOffset = sprintf (szNrLine + pOffset, " (%s) (%s)\n", ename, ModName);
        }
        else
          strcat (szNrLine, "\n");  /* If no file found then just newline */

        break;
      }                         /* end switch */
      i++;
    }                           /* end while modindex */
  }                             /* End While i < numdir */
  free (pDirTab32);
  return (0);
}

#endif /* EXCEPTIONS */
#endif /* OS_2       */

// ===========================================================================
// ===========================================================================

/* $Id: exceptio.c,v 1.6 1999/03/23 22:28:45 mr Exp $ */
