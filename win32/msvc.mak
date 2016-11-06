# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "BT32.MAK" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/Bt32.exe $(OUTDIR)/Bt32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /G4 /Zp2 /MT /W3 /GX /YX /O2 /I "..\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_WIN32" /D "CACHE" /D "CO_NOPT" /D "HYDRADEV" /D "HEAPCHECK" /D _CPU=486 /D "NT" /FR /c
CPP_PROJ=/nologo /G4 /Zp2 /MT /W3 /GX /YX /O2 /I "..\include" /D "NDEBUG" /D\
 "WIN32" /D "_CONSOLE" /D "_WIN32" /D "CACHE" /D "CO_NOPT" /D "HYDRADEV" /D\
 "HEAPCHECK" /D _CPU=486 /D "NT" /FR$(INTDIR)/ /Fp$(OUTDIR)/"BT32.pch" /F\
o$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"Bt32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/SQUISH.SBR \
	$(INTDIR)/B_SBINIT.SBR \
	$(INTDIR)/B_SEARCH.SBR \
	$(INTDIR)/NODEPROC.SBR \
	$(INTDIR)/BREC.SBR \
	$(INTDIR)/BINK_ASM.SBR \
	$(INTDIR)/B_FRPROC.SBR \
	$(INTDIR)/VERSION7.SBR \
	$(INTDIR)/MAILROOT.SBR \
	$(INTDIR)/B_TODAY.SBR \
	$(INTDIR)/M7REC.SBR \
	$(INTDIR)/CACHE.SBR \
	$(INTDIR)/TIMER.SBR \
	$(INTDIR)/CFOSLINE.SBR \
	$(INTDIR)/BBS_DATA.SBR \
	$(INTDIR)/HEAP.SBR \
	$(INTDIR)/B_WZSEND.SBR \
	$(INTDIR)/PROTCOMM.SBR \
	$(INTDIR)/JANUS.SBR \
	$(INTDIR)/CSS.SBR \
	$(INTDIR)/MAILER.SBR \
	$(INTDIR)/AGLCRC.SBR \
	$(INTDIR)/MISC.SBR \
	$(INTDIR)/B_TERMOV.SBR \
	$(INTDIR)/RECVSYNC.SBR \
	$(INTDIR)/CALLBACK.SBR \
	$(INTDIR)/B_FUSER.SBR \
	$(INTDIR)/VFOS_W32.SBR \
	$(INTDIR)/STATS.SBR \
	$(INTDIR)/EMSI.SBR \
	$(INTDIR)/ASYN_W32.SBR \
	$(INTDIR)/KEYMAP.SBR \
	$(INTDIR)/SCHED.SBR \
	$(INTDIR)/BBS_IO.SBR \
	$(INTDIR)/B_TERM.SBR \
	$(INTDIR)/B_LOGS.SBR \
	$(INTDIR)/STATETBL.SBR \
	$(INTDIR)/FOSSIL.SBR \
	$(INTDIR)/BBS_CTRA.SBR \
	$(INTDIR)/OUTBOUND.SBR \
	$(INTDIR)/CALLERID.SBR \
	$(INTDIR)/B_INITVA.SBR \
	$(INTDIR)/TIMES.SBR \
	$(INTDIR)/DATA.SBR \
	$(INTDIR)/B_FLOFIL.SBR \
	$(INTDIR)/M7SEND.SBR \
	$(INTDIR)/B_SESSIO.SBR \
	$(INTDIR)/BTCONFIG.SBR \
	$(INTDIR)/PKTMSGS.SBR \
	$(INTDIR)/B_BBS.SBR \
	$(INTDIR)/ZMODEM.SBR \
	$(INTDIR)/BT.SBR \
	$(INTDIR)/HISTORY.SBR \
	$(INTDIR)/FTSC.SBR \
	$(INTDIR)/BINKDLG.SBR \
	$(INTDIR)/B_WAZOO.SBR \
	$(INTDIR)/BANNER.SBR \
	$(INTDIR)/SENDBARK.SBR \
	$(INTDIR)/EVTPARSE.SBR \
	$(INTDIR)/B_FAXRCV.SBR \
	$(INTDIR)/B_HELP.SBR \
	$(INTDIR)/SCRIPT2.SBR \
	$(INTDIR)/B_PROTO.SBR \
	$(INTDIR)/EVTSETUP.SBR \
	$(INTDIR)/B_SPAWN.SBR \
	$(INTDIR)/XMSEND.SBR \
	$(INTDIR)/CAPI.SBR \
	$(INTDIR)/B_WHACK.SBR \
	$(INTDIR)/VERSION.SBR \
	$(INTDIR)/MAXMCP.SBR \
	$(INTDIR)/XMREC.SBR \
	$(INTDIR)/INCLUDES.SBR \
	$(INTDIR)/CPULOAD.SBR \
	$(INTDIR)/COMMON.SBR \
	$(INTDIR)/SENDSYNC.SBR \
	$(INTDIR)/MAILOVLY.SBR \
	$(INTDIR)/B_PASSWO.SBR \
	$(INTDIR)/MDM_PROC.SBR \
	$(INTDIR)/FREEPOLL.SBR \
	$(INTDIR)/FILE_ALL.SBR \
	$(INTDIR)/FILE_W32.SBR \
	$(INTDIR)/B_SCRIPT.SBR \
	$(INTDIR)/LANGLOAD.SBR \
	$(INTDIR)/SBUF.SBR \
	$(INTDIR)/BSEND.SBR \
	$(INTDIR)/SRIF.SBR \
	$(INTDIR)/EXCEPTIO.SBR \
	$(INTDIR)/DOSFUNCS.SBR \
	$(INTDIR)/CALLWIN.SBR \
	$(INTDIR)/DEBUG.SBR \
	$(INTDIR)/YOOHOO.SBR \
	$(INTDIR)/VT100.SBR \
	$(INTDIR)/B_RSPFIL.SBR \
	$(INTDIR)/RECVBARK.SBR \
	$(INTDIR)/B_BANNER.SBR \
	$(INTDIR)/PIPE.SBR \
	$(INTDIR)/MAXPRM.SBR \
	$(INTDIR)/HYDRA.SBR \
	$(INTDIR)/CHAT.SBR

$(OUTDIR)/Bt32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib libc.lib ntcommms.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib winmm.lib libc.lib ntcommms.lib /NOLOGO\
 /SUBSYSTEM:console /INCREMENTAL:no /PDB:$(OUTDIR)/"BT32.pdb" /MACHINE:I386\
 /OUT:$(OUTDIR)/"Bt32.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/SQUISH.OBJ \
	$(INTDIR)/B_SBINIT.OBJ \
	$(INTDIR)/B_SEARCH.OBJ \
	$(INTDIR)/NODEPROC.OBJ \
	$(INTDIR)/BREC.OBJ \
	$(INTDIR)/BINK_ASM.OBJ \
	$(INTDIR)/B_FRPROC.OBJ \
	$(INTDIR)/VERSION7.OBJ \
	$(INTDIR)/MAILROOT.OBJ \
	$(INTDIR)/B_TODAY.OBJ \
	$(INTDIR)/M7REC.OBJ \
	$(INTDIR)/CACHE.OBJ \
	$(INTDIR)/TIMER.OBJ \
	$(INTDIR)/CFOSLINE.OBJ \
	$(INTDIR)/BBS_DATA.OBJ \
	$(INTDIR)/HEAP.OBJ \
	$(INTDIR)/B_WZSEND.OBJ \
	$(INTDIR)/PROTCOMM.OBJ \
	$(INTDIR)/JANUS.OBJ \
	$(INTDIR)/CSS.OBJ \
	$(INTDIR)/MAILER.OBJ \
	$(INTDIR)/AGLCRC.OBJ \
	$(INTDIR)/MISC.OBJ \
	$(INTDIR)/B_TERMOV.OBJ \
	$(INTDIR)/RECVSYNC.OBJ \
	$(INTDIR)/CALLBACK.OBJ \
	$(INTDIR)/B_FUSER.OBJ \
	$(INTDIR)/VFOS_W32.OBJ \
	$(INTDIR)/STATS.OBJ \
	$(INTDIR)/EMSI.OBJ \
	$(INTDIR)/ASYN_W32.OBJ \
	$(INTDIR)/KEYMAP.OBJ \
	$(INTDIR)/SCHED.OBJ \
	$(INTDIR)/BBS_IO.OBJ \
	$(INTDIR)/B_TERM.OBJ \
	$(INTDIR)/B_LOGS.OBJ \
	$(INTDIR)/STATETBL.OBJ \
	$(INTDIR)/FOSSIL.OBJ \
	$(INTDIR)/BBS_CTRA.OBJ \
	$(INTDIR)/OUTBOUND.OBJ \
	$(INTDIR)/CALLERID.OBJ \
	$(INTDIR)/B_INITVA.OBJ \
	$(INTDIR)/TIMES.OBJ \
	$(INTDIR)/DATA.OBJ \
	$(INTDIR)/B_FLOFIL.OBJ \
	$(INTDIR)/M7SEND.OBJ \
	$(INTDIR)/B_SESSIO.OBJ \
	$(INTDIR)/BTCONFIG.OBJ \
	$(INTDIR)/PKTMSGS.OBJ \
	$(INTDIR)/B_BBS.OBJ \
	$(INTDIR)/ZMODEM.OBJ \
	$(INTDIR)/BT.OBJ \
	$(INTDIR)/HISTORY.OBJ \
	$(INTDIR)/FTSC.OBJ \
	$(INTDIR)/BINKDLG.OBJ \
	$(INTDIR)/B_WAZOO.OBJ \
	$(INTDIR)/BANNER.OBJ \
	$(INTDIR)/SENDBARK.OBJ \
	$(INTDIR)/EVTPARSE.OBJ \
	$(INTDIR)/B_FAXRCV.OBJ \
	$(INTDIR)/B_HELP.OBJ \
	$(INTDIR)/SCRIPT2.OBJ \
	$(INTDIR)/B_PROTO.OBJ \
	$(INTDIR)/EVTSETUP.OBJ \
	$(INTDIR)/B_SPAWN.OBJ \
	$(INTDIR)/XMSEND.OBJ \
	$(INTDIR)/CAPI.OBJ \
	$(INTDIR)/B_WHACK.OBJ \
	$(INTDIR)/VERSION.OBJ \
	$(INTDIR)/MAXMCP.OBJ \
	$(INTDIR)/XMREC.OBJ \
	$(INTDIR)/INCLUDES.OBJ \
	$(INTDIR)/CPULOAD.OBJ \
	$(INTDIR)/COMMON.OBJ \
	$(INTDIR)/SENDSYNC.OBJ \
	$(INTDIR)/MAILOVLY.OBJ \
	$(INTDIR)/B_PASSWO.OBJ \
	$(INTDIR)/MDM_PROC.OBJ \
	$(INTDIR)/FREEPOLL.OBJ \
	$(INTDIR)/FILE_ALL.OBJ \
	$(INTDIR)/FILE_W32.OBJ \
	$(INTDIR)/B_SCRIPT.OBJ \
	$(INTDIR)/LANGLOAD.OBJ \
	$(INTDIR)/SBUF.OBJ \
	$(INTDIR)/BSEND.OBJ \
	$(INTDIR)/SRIF.OBJ \
	$(INTDIR)/EXCEPTIO.OBJ \
	$(INTDIR)/DOSFUNCS.OBJ \
	$(INTDIR)/CALLWIN.OBJ \
	$(INTDIR)/DEBUG.OBJ \
	$(INTDIR)/YOOHOO.OBJ \
	$(INTDIR)/VT100.OBJ \
	$(INTDIR)/B_RSPFIL.OBJ \
	$(INTDIR)/RECVBARK.OBJ \
	$(INTDIR)/B_BANNER.OBJ \
	$(INTDIR)/PIPE.OBJ \
	$(INTDIR)/MAXPRM.OBJ \
#	\msvc\MSVC20\LIB\NTCOMM.LIB \
	$(INTDIR)/HYDRA.OBJ \
	$(INTDIR)/CHAT.OBJ

$(OUTDIR)/Bt32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/Bt32.exe $(OUTDIR)/Bt32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /G4 /Zp2 /MT /W3 /GX /Zi /YX /Od /I "..\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_WIN32" /D "CACHE" /D "CO_NOPT" /D "HYDRADEV" /D "HEAPCHECK" /D _CPU=486 /D "NT" /FR /c
CPP_PROJ=/nologo /G4 /Zp2 /MT /W3 /GX /Zi /YX /Od /I "..\include" /D "_DEBUG"\
 /D "WIN32" /D "_CONSOLE" /D "_WIN32" /D "CACHE" /D "CO_NOPT" /D "HYDRADEV" /D\
 "HEAPCHECK" /D _CPU=486 /D "NT" /FR$(INTDIR)/ /Fp$(OUTDIR)/"Bt32.pch" /F\
o$(INTDIR)/\
 /Fd$(OUTDIR)/"Bt32.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"Bt32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/SQUISH.SBR \
	$(INTDIR)/B_SBINIT.SBR \
	$(INTDIR)/B_SEARCH.SBR \
	$(INTDIR)/NODEPROC.SBR \
	$(INTDIR)/BREC.SBR \
	$(INTDIR)/BINK_ASM.SBR \
	$(INTDIR)/B_FRPROC.SBR \
	$(INTDIR)/VERSION7.SBR \
	$(INTDIR)/MAILROOT.SBR \
	$(INTDIR)/B_TODAY.SBR \
	$(INTDIR)/M7REC.SBR \
	$(INTDIR)/CACHE.SBR \
	$(INTDIR)/TIMER.SBR \
	$(INTDIR)/CFOSLINE.SBR \
	$(INTDIR)/BBS_DATA.SBR \
	$(INTDIR)/HEAP.SBR \
	$(INTDIR)/B_WZSEND.SBR \
	$(INTDIR)/PROTCOMM.SBR \
	$(INTDIR)/JANUS.SBR \
	$(INTDIR)/CSS.SBR \
	$(INTDIR)/MAILER.SBR \
	$(INTDIR)/AGLCRC.SBR \
	$(INTDIR)/MISC.SBR \
	$(INTDIR)/B_TERMOV.SBR \
	$(INTDIR)/RECVSYNC.SBR \
	$(INTDIR)/CALLBACK.SBR \
	$(INTDIR)/B_FUSER.SBR \
	$(INTDIR)/VFOS_W32.SBR \
	$(INTDIR)/STATS.SBR \
	$(INTDIR)/EMSI.SBR \
	$(INTDIR)/ASYN_W32.SBR \
	$(INTDIR)/KEYMAP.SBR \
	$(INTDIR)/SCHED.SBR \
	$(INTDIR)/BBS_IO.SBR \
	$(INTDIR)/B_TERM.SBR \
	$(INTDIR)/B_LOGS.SBR \
	$(INTDIR)/STATETBL.SBR \
	$(INTDIR)/FOSSIL.SBR \
	$(INTDIR)/BBS_CTRA.SBR \
	$(INTDIR)/OUTBOUND.SBR \
	$(INTDIR)/CALLERID.SBR \
	$(INTDIR)/B_INITVA.SBR \
	$(INTDIR)/TIMES.SBR \
	$(INTDIR)/DATA.SBR \
	$(INTDIR)/B_FLOFIL.SBR \
	$(INTDIR)/M7SEND.SBR \
	$(INTDIR)/B_SESSIO.SBR \
	$(INTDIR)/BTCONFIG.SBR \
	$(INTDIR)/PKTMSGS.SBR \
	$(INTDIR)/B_BBS.SBR \
	$(INTDIR)/ZMODEM.SBR \
	$(INTDIR)/BT.SBR \
	$(INTDIR)/HISTORY.SBR \
	$(INTDIR)/FTSC.SBR \
	$(INTDIR)/BINKDLG.SBR \
	$(INTDIR)/B_WAZOO.SBR \
	$(INTDIR)/BANNER.SBR \
	$(INTDIR)/SENDBARK.SBR \
	$(INTDIR)/EVTPARSE.SBR \
	$(INTDIR)/B_FAXRCV.SBR \
	$(INTDIR)/B_HELP.SBR \
	$(INTDIR)/SCRIPT2.SBR \
	$(INTDIR)/B_PROTO.SBR \
	$(INTDIR)/EVTSETUP.SBR \
	$(INTDIR)/B_SPAWN.SBR \
	$(INTDIR)/XMSEND.SBR \
	$(INTDIR)/CAPI.SBR \
	$(INTDIR)/B_WHACK.SBR \
	$(INTDIR)/VERSION.SBR \
	$(INTDIR)/MAXMCP.SBR \
	$(INTDIR)/XMREC.SBR \
	$(INTDIR)/INCLUDES.SBR \
	$(INTDIR)/CPULOAD.SBR \
	$(INTDIR)/COMMON.SBR \
	$(INTDIR)/SENDSYNC.SBR \
	$(INTDIR)/MAILOVLY.SBR \
	$(INTDIR)/B_PASSWO.SBR \
	$(INTDIR)/MDM_PROC.SBR \
	$(INTDIR)/FREEPOLL.SBR \
	$(INTDIR)/FILE_ALL.SBR \
	$(INTDIR)/FILE_W32.SBR \
	$(INTDIR)/B_SCRIPT.SBR \
	$(INTDIR)/LANGLOAD.SBR \
	$(INTDIR)/SBUF.SBR \
	$(INTDIR)/BSEND.SBR \
	$(INTDIR)/SRIF.SBR \
	$(INTDIR)/EXCEPTIO.SBR \
	$(INTDIR)/DOSFUNCS.SBR \
	$(INTDIR)/CALLWIN.SBR \
	$(INTDIR)/DEBUG.SBR \
	$(INTDIR)/YOOHOO.SBR \
	$(INTDIR)/VT100.SBR \
	$(INTDIR)/B_RSPFIL.SBR \
	$(INTDIR)/RECVBARK.SBR \
	$(INTDIR)/B_BANNER.SBR \
	$(INTDIR)/PIPE.SBR \
	$(INTDIR)/MAXPRM.SBR \
	$(INTDIR)/HYDRA.SBR \
	$(INTDIR)/CHAT.SBR

$(OUTDIR)/Bt32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib libc.lib ntcomm.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=kernel32.lib user32.lib winmm.lib libc.lib ntcommms.lib /NOLOGO\
 /SUBSYSTEM:console /INCREMENTAL:yes /PDB:$(OUTDIR)/"Bt32.pdb" /DEBUG\
 /MACHINE:I386 /OUT:$(OUTDIR)/"Bt32.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/SQUISH.OBJ \
	$(INTDIR)/B_SBINIT.OBJ \
	$(INTDIR)/B_SEARCH.OBJ \
	$(INTDIR)/NODEPROC.OBJ \
	$(INTDIR)/BREC.OBJ \
	$(INTDIR)/BINK_ASM.OBJ \
	$(INTDIR)/B_FRPROC.OBJ \
	$(INTDIR)/VERSION7.OBJ \
	$(INTDIR)/MAILROOT.OBJ \
	$(INTDIR)/B_TODAY.OBJ \
	$(INTDIR)/M7REC.OBJ \
	$(INTDIR)/CACHE.OBJ \
	$(INTDIR)/TIMER.OBJ \
	$(INTDIR)/CFOSLINE.OBJ \
	$(INTDIR)/BBS_DATA.OBJ \
	$(INTDIR)/HEAP.OBJ \
	$(INTDIR)/B_WZSEND.OBJ \
	$(INTDIR)/PROTCOMM.OBJ \
	$(INTDIR)/JANUS.OBJ \
	$(INTDIR)/CSS.OBJ \
	$(INTDIR)/MAILER.OBJ \
	$(INTDIR)/AGLCRC.OBJ \
	$(INTDIR)/MISC.OBJ \
	$(INTDIR)/B_TERMOV.OBJ \
	$(INTDIR)/RECVSYNC.OBJ \
	$(INTDIR)/CALLBACK.OBJ \
	$(INTDIR)/B_FUSER.OBJ \
	$(INTDIR)/VFOS_W32.OBJ \
	$(INTDIR)/STATS.OBJ \
	$(INTDIR)/EMSI.OBJ \
	$(INTDIR)/ASYN_W32.OBJ \
	$(INTDIR)/KEYMAP.OBJ \
	$(INTDIR)/SCHED.OBJ \
	$(INTDIR)/BBS_IO.OBJ \
	$(INTDIR)/B_TERM.OBJ \
	$(INTDIR)/B_LOGS.OBJ \
	$(INTDIR)/STATETBL.OBJ \
	$(INTDIR)/FOSSIL.OBJ \
	$(INTDIR)/BBS_CTRA.OBJ \
	$(INTDIR)/OUTBOUND.OBJ \
	$(INTDIR)/CALLERID.OBJ \
	$(INTDIR)/B_INITVA.OBJ \
	$(INTDIR)/TIMES.OBJ \
	$(INTDIR)/DATA.OBJ \
	$(INTDIR)/B_FLOFIL.OBJ \
	$(INTDIR)/M7SEND.OBJ \
	$(INTDIR)/B_SESSIO.OBJ \
	$(INTDIR)/BTCONFIG.OBJ \
	$(INTDIR)/PKTMSGS.OBJ \
	$(INTDIR)/B_BBS.OBJ \
	$(INTDIR)/ZMODEM.OBJ \
	$(INTDIR)/BT.OBJ \
	$(INTDIR)/HISTORY.OBJ \
	$(INTDIR)/FTSC.OBJ \
	$(INTDIR)/BINKDLG.OBJ \
	$(INTDIR)/B_WAZOO.OBJ \
	$(INTDIR)/BANNER.OBJ \
	$(INTDIR)/SENDBARK.OBJ \
	$(INTDIR)/EVTPARSE.OBJ \
	$(INTDIR)/B_FAXRCV.OBJ \
	$(INTDIR)/B_HELP.OBJ \
	$(INTDIR)/SCRIPT2.OBJ \
	$(INTDIR)/B_PROTO.OBJ \
	$(INTDIR)/EVTSETUP.OBJ \
	$(INTDIR)/B_SPAWN.OBJ \
	$(INTDIR)/XMSEND.OBJ \
	$(INTDIR)/CAPI.OBJ \
	$(INTDIR)/B_WHACK.OBJ \
	$(INTDIR)/VERSION.OBJ \
	$(INTDIR)/MAXMCP.OBJ \
	$(INTDIR)/XMREC.OBJ \
	$(INTDIR)/INCLUDES.OBJ \
	$(INTDIR)/CPULOAD.OBJ \
	$(INTDIR)/COMMON.OBJ \
	$(INTDIR)/SENDSYNC.OBJ \
	$(INTDIR)/MAILOVLY.OBJ \
	$(INTDIR)/B_PASSWO.OBJ \
	$(INTDIR)/MDM_PROC.OBJ \
	$(INTDIR)/FREEPOLL.OBJ \
	$(INTDIR)/FILE_ALL.OBJ \
	$(INTDIR)/FILE_W32.OBJ \
	$(INTDIR)/B_SCRIPT.OBJ \
	$(INTDIR)/LANGLOAD.OBJ \
	$(INTDIR)/SBUF.OBJ \
	$(INTDIR)/BSEND.OBJ \
	$(INTDIR)/SRIF.OBJ \
	$(INTDIR)/EXCEPTIO.OBJ \
	$(INTDIR)/DOSFUNCS.OBJ \
	$(INTDIR)/CALLWIN.OBJ \
	$(INTDIR)/DEBUG.OBJ \
	$(INTDIR)/YOOHOO.OBJ \
	$(INTDIR)/VT100.OBJ \
	$(INTDIR)/B_RSPFIL.OBJ \
	$(INTDIR)/RECVBARK.OBJ \
	$(INTDIR)/B_BANNER.OBJ \
	$(INTDIR)/PIPE.OBJ \
	$(INTDIR)/MAXPRM.OBJ \
#	\msvc\MSVC20\LIB\NTCOMM.LIB \
	$(INTDIR)/HYDRA.OBJ \
	$(INTDIR)/CHAT.OBJ

$(OUTDIR)/Bt32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=..\SRC\SQUISH.C

$(INTDIR)/SQUISH.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_SBINIT.C

$(INTDIR)/B_SBINIT.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_SEARCH.C

$(INTDIR)/B_SEARCH.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\NODEPROC.C

$(INTDIR)/NODEPROC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BREC.C

$(INTDIR)/BREC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BINK_ASM.C

$(INTDIR)/BINK_ASM.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_FRPROC.C

$(INTDIR)/B_FRPROC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\VERSION7.C

$(INTDIR)/VERSION7.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\MAILROOT.C

$(INTDIR)/MAILROOT.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_TODAY.C

$(INTDIR)/B_TODAY.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\M7REC.C

$(INTDIR)/M7REC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CACHE.C

$(INTDIR)/CACHE.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\TIMER.C

$(INTDIR)/TIMER.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CFOSLINE.C

$(INTDIR)/CFOSLINE.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BBS_DATA.C

$(INTDIR)/BBS_DATA.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\HEAP.C

$(INTDIR)/HEAP.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_WZSEND.C

$(INTDIR)/B_WZSEND.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\PROTCOMM.C

$(INTDIR)/PROTCOMM.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\JANUS.C

$(INTDIR)/JANUS.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CSS.C

$(INTDIR)/CSS.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\MAILER.C

$(INTDIR)/MAILER.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\AGLCRC.C

$(INTDIR)/AGLCRC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\MISC.C

$(INTDIR)/MISC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_TERMOV.C

$(INTDIR)/B_TERMOV.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\RECVSYNC.C

$(INTDIR)/RECVSYNC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CALLBACK.C

$(INTDIR)/CALLBACK.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_FUSER.C

$(INTDIR)/B_FUSER.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\VFOS_W32.C

$(INTDIR)/VFOS_W32.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\STATS.C

$(INTDIR)/STATS.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\EMSI.C

$(INTDIR)/EMSI.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\ASYN_W32.C

$(INTDIR)/ASYN_W32.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\KEYMAP.C

$(INTDIR)/KEYMAP.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\SCHED.C

$(INTDIR)/SCHED.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BBS_IO.C

$(INTDIR)/BBS_IO.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_TERM.C

$(INTDIR)/B_TERM.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_LOGS.C

$(INTDIR)/B_LOGS.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\STATETBL.C

$(INTDIR)/STATETBL.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\FOSSIL.C

$(INTDIR)/FOSSIL.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BBS_CTRA.C

$(INTDIR)/BBS_CTRA.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\OUTBOUND.C

$(INTDIR)/OUTBOUND.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CALLERID.C

$(INTDIR)/CALLERID.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_INITVA.C

$(INTDIR)/B_INITVA.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\TIMES.C

$(INTDIR)/TIMES.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\DATA.C

$(INTDIR)/DATA.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_FLOFIL.C

$(INTDIR)/B_FLOFIL.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\M7SEND.C

$(INTDIR)/M7SEND.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_SESSIO.C

$(INTDIR)/B_SESSIO.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BTCONFIG.C

$(INTDIR)/BTCONFIG.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\PKTMSGS.C

$(INTDIR)/PKTMSGS.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_BBS.C

$(INTDIR)/B_BBS.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\ZMODEM.C

$(INTDIR)/ZMODEM.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BT.C

$(INTDIR)/BT.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\HISTORY.C

$(INTDIR)/HISTORY.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\FTSC.C

$(INTDIR)/FTSC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BINKDLG.C

$(INTDIR)/BINKDLG.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_WAZOO.C

$(INTDIR)/B_WAZOO.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BANNER.C

$(INTDIR)/BANNER.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\SENDBARK.C

$(INTDIR)/SENDBARK.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\EVTPARSE.C

$(INTDIR)/EVTPARSE.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_FAXRCV.C

$(INTDIR)/B_FAXRCV.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_HELP.C

$(INTDIR)/B_HELP.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\SCRIPT2.C

$(INTDIR)/SCRIPT2.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_PROTO.C

$(INTDIR)/B_PROTO.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\EVTSETUP.C

$(INTDIR)/EVTSETUP.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_SPAWN.C

$(INTDIR)/B_SPAWN.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\XMSEND.C

$(INTDIR)/XMSEND.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CAPI.C

$(INTDIR)/CAPI.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_WHACK.C

$(INTDIR)/B_WHACK.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\VERSION.C

$(INTDIR)/VERSION.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\MAXMCP.C

$(INTDIR)/MAXMCP.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\XMREC.C

$(INTDIR)/XMREC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\INCLUDES.C

$(INTDIR)/INCLUDES.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CPULOAD.C

$(INTDIR)/CPULOAD.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\COMMON.C

$(INTDIR)/COMMON.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\SENDSYNC.C

$(INTDIR)/SENDSYNC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\MAILOVLY.C

$(INTDIR)/MAILOVLY.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_PASSWO.C

$(INTDIR)/B_PASSWO.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\MDM_PROC.C

$(INTDIR)/MDM_PROC.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\FREEPOLL.C

$(INTDIR)/FREEPOLL.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\FILE_ALL.C

$(INTDIR)/FILE_ALL.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\FILE_W32.C

$(INTDIR)/FILE_ALL.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_SCRIPT.C

$(INTDIR)/B_SCRIPT.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\LANGLOAD.C

$(INTDIR)/LANGLOAD.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\SBUF.C

$(INTDIR)/SBUF.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\BSEND.C

$(INTDIR)/BSEND.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\SRIF.C

$(INTDIR)/SRIF.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\EXCEPTIO.C

$(INTDIR)/EXCEPTIO.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\DOSFUNCS.C

$(INTDIR)/DOSFUNCS.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CALLWIN.C

$(INTDIR)/CALLWIN.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\DEBUG.C

$(INTDIR)/DEBUG.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\YOOHOO.C

$(INTDIR)/YOOHOO.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\VT100.C

$(INTDIR)/VT100.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_RSPFIL.C

$(INTDIR)/B_RSPFIL.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\RECVBARK.C

$(INTDIR)/RECVBARK.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\B_BANNER.C

$(INTDIR)/B_BANNER.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\PIPE.C

$(INTDIR)/PIPE.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\MAXPRM.C

$(INTDIR)/MAXPRM.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\msvc\MSVC20\LIB\NTCOMM.LIB
# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\HYDRA.C

$(INTDIR)/HYDRA.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=..\SRC\CHAT.C

$(INTDIR)/CHAT.OBJ :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
# End Group
# End Project
################################################################################

