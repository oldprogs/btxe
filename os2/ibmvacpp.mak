#############################################################################
#                                                                           #
# Make file for BinkleyTerm-OS/2 Version 2.60 XE                            #
#                                                                           #
# Compiler: VisualAge C++ 3.0                                               #
#                                                                           #
# This file is used to build the 32-bit version of the software for         #
# usage on OS/2 2.x, Warp v3 and Warp v4 systems.                           #
#                                                                           #
# Usage: nmake -f IBMVACPP.MAK ["CPU=<string>"] [optional target]           #
#                                                                           #
# If no target is specified, "all" will be generated.                       #
#                                                                           #
# Defining "CPU=<string>" (<string> may be `86', `186', `286', `386' and    #
# will result in a 386 32Bit-EXE to be built or '486' or '586' what will    #
# result in 486 or Pentium optimized EXEs to be built.) on the nmake        #
# command line will generate code optimized for the specified CPU type.     #
#                                                                           #
#############################################################################

DEBUG      = 0
USEMAXCOMM = 1

SRC_DIR    = ..\src
INCL_DIR   = ..\include
LIB_DIR    = ..\lib

LINKLIBS    = os2386.lib

!IF $(USEMAXCOMM) == 1
BINKLIBS    = $(LIB_DIR)\mcp32.lib $(LIB_DIR)\maxcomm.lib
!ELSE
BINKLIBS    = $(LIB_DIR)\mcp32.lib
!ENDIF

!IF "$(CPU)" == ""
CCPUF= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "86"
CCPUF= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "186"
CCPUF= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "286"
CCPUF= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "386"
CCPUF= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "486"
CCPUF= /G4 /D_CPU=486
!ENDIF

!IF "$(CPU)" == "586"
CCPUF= /G5 /D_CPU=586
!ENDIF

# options to compile into Binkley
#
# /DLOAD_PM_DLLS will lead to load the required PM-DLLs with DosLoadModule
#
#
!IF $(USEMAXCOMM) == 1
OPTFLAGS = /DOS_2 /DNEED_SET_PRIOR /DHYDRADEV /DCACHE                 /DCFSPROBLEM /DSMP_API /DEXCEPTIONS
# /DREL_CAND
# /DBBS_INTERNAL
!ELSE
OPTFLAGS = /DOS_2 /DNEED_SET_PRIOR /DHYDRADEV /DCACHE /DNEED_OS2COMMS /DCFSPROBLEM /DSMP_API /DEXCEPTIONS
# /DREL_CAND
!ENDIF
#          /DDEBUG \
  

# for performance analyzer: /Gh
WC_EXTRA  = /Wcnd /Weff /Wini /Word /Wrea /Wtru /Wund
C_EXTRA   = /Weff /Wrea /Wtru /Wund
CFLAGS_X  = /GM+ /GT+ /W3 /Wuse /Wpar /Wdcl /Wcnv /Wpro /Wret /Sm /Sp1 /Ss /C /Tl1 /Q
!IF $(DEBUG) == 0
CFLAGS  = /O+         $(CCPUF) $(CFLAGS_X) $(OPTFLAGS) /I $(INCL_DIR) /DCO_OPT
!ELSE
CFLAGS  = /O- /Ti /Tm $(CCPUF) $(CFLAGS_X) $(OPTFLAGS) /I $(INCL_DIR) /DCO_NOPT
!ENDIF
# /Ol    invoke intermediate code linker
# /Oinnn enable inline function code up to nnn intermediate code instr
# /W3    all errors and warnings
# /Sm    Ignore migration keywords (!?)
# /Sp1   pack aggregate members on alignment 1
# /Ss    allow // for comments in C-source
# /O+    optimize generated code (do NOT use for debugging!)
# /C     compile only
# /G5    Pentium optimization
# /GT+   Erzeuge tiled memory (die COM-Buffer..)
# /GM+   Multithread Libraries (HydraWrite!)
# /Gs+   suppress stack probes in function prologues
# /Tln   keep compiler n mins in memory
# /Ti    generate debugging information
# /Tx    generate full exception register dump
# /Tm    debug memory management
# /Q     suppress the logo
# /DOS_2 #define OS_2
# /I     set include dir(s)

# for performance analyzer: /NOE  cppopa3.obj
!IF $(DEBUG) == 0
LFLAGS =         /Q /B"                /packc /packd /map /noi /nol /pmtype:vio /align:16 /exepack"
!ELSE
LFLAGS = /Ti /Tm /Q /B"/line /c /debug /packc /packd /map /noi /nol /pmtype:vio /align:16 /exepack" 
!ENDIF
# /Q       no logo
# /B"..."  pass options to linker
# /packc   pack code segments together
# /packs   pack data segments together

CC       = ICC
LINK     = ICC

USEPCH = /Si
MAKEPCH = /Fi

H1   = $(INCL_DIR)\com.h      $(INCL_DIR)\com_os2.h  $(INCL_DIR)\xfer.h
H2   = $(INCL_DIR)\bink.h     $(INCL_DIR)\zmodem.h   $(INCL_DIR)\keybd.h
H3   = $(INCL_DIR)\sbuf.h     $(INCL_DIR)\sched.h    $(INCL_DIR)\externs.h
H4   = $(INCL_DIR)\includes.h $(INCL_DIR)\prototyp.h $(INCL_DIR)\keyfncs.h
H5   = $(INCL_DIR)\squish.h   $(INCL_DIR)\debug.h    $(INCL_DIR)\css.h
H6   = $(INCL_DIR)\binkpipe.h $(INCL_DIR)\msgs.h     $(INCL_DIR)\history.h
H7   = $(INCL_DIR)\defines.h  $(INCL_DIR)\hydra.h    $(INCL_DIR)\janus.h
H8   = $(INCL_DIR)\fidomail.h $(INCL_DIR)\version7.h

DEF_H    = $(H1) $(H2) $(H3) $(H4) $(H5) $(H6) $(H7) $(H8)


# Neu, noch testen:

#H1   = $(INCL_DIR)\aglcrc.h   $(INCL_DIR)\async.h    $(INCL_DIR)\asyos2v1.h
#H2   = $(INCL_DIR)\banner.h   $(INCL_DIR)\bink.h     $(INCL_DIR)\binkpipe.h
#H3   = $(INCL_DIR)\box.h      $(INCL_DIR)\buffer.h   $(INCL_DIR)\cache.h
#H4   = $(INCL_DIR)\capi11.h   $(INCL_DIR)\com.h      $(INCL_DIR)\com_dos.h
#H5   = $(INCL_DIR)\com_nt.h   $(INCL_DIR)\com_os2.h  $(INCL_DIR)\com_os2p.h
#H6   = $(INCL_DIR)\css.h      $(INCL_DIR)\debug.h    $(INCL_DIR)\defines.h
#H7   = $(INCL_DIR)\exceptio.h $(INCL_DIR)\externs.h  $(INCL_DIR)\faxproto.h
#H8   = $(INCL_DIR)\fidomail.h $(INCL_DIR)\history.h  $(INCL_DIR)\hydra.h
#H9   = $(INCL_DIR)\includes.h $(INCL_DIR)\janus.h    $(INCL_DIR)\keybd.h
#H10  = $(INCL_DIR)\keyfncs.h  $(INCL_DIR)\language.h $(INCL_DIR)\lngmagic.h
#H11  = $(INCL_DIR)\maxcomm.h  $(INCL_DIR)\maxmcp.h   $(INCL_DIR)\msgs.h
#H12  = $(INCL_DIR)\pktmsgs.h  $(INCL_DIR)\prototyp.h
#H13  = $(INCL_DIR)\sbuf.h     $(INCL_DIR)\sched.h    $(INCL_DIR)\squish.h
#H14  = $(INCL_DIR)\timer.h    $(INCL_DIR)\version7.h $(INCL_DIR)\vfossil.h
#H15  = $(INCL_DIR)\video.h    $(INCL_DIR)\wnfossil.h $(INCL_DIR)\xfer.h
#H16  = $(INCL_DIR)\zmodem.h

#DEF_H= $(H1) $(H2) $(H3) $(H4) $(H5) $(H6) $(H7) $(H8) \
#       $(H9) $(H10) $(H11) $(H12) $(H13) $(H14) $(H15) $(H16)

cobj1=bt.obj       asyn_os2.obj ftsc.obj     b_script.obj sbuf.obj
cobj2=btconfig.obj brec.obj     b_search.obj b_today.obj  data.obj
cobj3=b_termov.obj bsend.obj    b_sessio.obj b_faxrcv.obj binkdlg.obj
cobj4=b_banner.obj b_spawn.obj  banner.obj   stats.obj    vt100.obj
cobj5=b_bbs.obj    mailer.obj   b_term.obj   chat.obj     callback.obj
cobj6=b_flofil.obj mailroot.obj b_wazoo.obj  mailovly.obj squish.obj
cobj7=b_frproc.obj mdm_proc.obj b_whack.obj  sched.obj    pktmsgs.obj
cobj8=misc.obj     b_wzsend.obj timer.obj    css.obj
cobj9=b_help.obj   nodeproc.obj sendbark.obj vfos_os2.obj os2_pm.obj
cobja=b_initva.obj m7rec.obj    m7send.obj   emsi.obj     pipe.obj
cobjb=b_passwo.obj yoohoo.obj   version7.obj maxmcp.obj   exceptio.obj
cobjc=b_proto.obj  dosfuncs.obj zmodem.obj   debug.obj    cfosline.obj
cobjd=b_rspfil.obj evtparse.obj freepoll.obj srif.obj     callwin.obj
cobje=b_sbinit.obj fossil.obj   heap.obj     common.obj   maxprm.obj
cobjf=keymap.obj   janus.obj    outbound.obj sendsync.obj file_all.obj
cobjg=statetbl.obj xmrec.obj    xmsend.obj   langload.obj cache.obj
cobjh=recvbark.obj recvsync.obj hydra.obj    aglcrc.obj   b_logs.obj
cobji=evtsetup.obj capi.obj     callerid.obj protcomm.obj history.obj
cobjj=cpuload.obj  file_os2.obj times.obj
# script2.obj  bbs_ctra.obj bbs_data.obj bbs_io.obj

cobjs1=btutil.obj  common.obj
cobjs2=gethcomm.obj getra.obj gettg.obj getmax3.obj getbgfax.obj

btobjs = $(cobj1) $(cobj2) $(cobj3) $(cobj4) $(cobj5) $(cobj6) $(cobj7) $(cobj8) $(cobj9) $(cobja) $(cobjb) $(cobjc) $(cobjd) $(cobje) $(cobjf) $(cobjg) $(cobjh) $(cobji) $(cobjj) 
btobjsc = $(btobjs) version.obj

btpch = includes.obj

all: \
    version.obj \
    btutil32.exe bt32.exe english.lng startrek.lng deutsch.lng swedish.lng patcheng \
    binkpipe.exe \
    toss.exe \
    bink_pm.dll \
    bt32.sym \
    cleanup

distrib:
    @rar a -ep bos2_bin $(SRC_DIR)\language.txt
    @rar a -ep bos2_bin dll\*.dll
    @rar a     bos2_bin bt32.exe bt32.sym bt32.map
    @rar a     bos2_bin btutil32.exe
    @rar a     bos2_bin bink_pm.dll bink_pm.sym
    @rar a -ep bos2_bin ..\doc\xe_user.doc ..\doc\xe_hist.doc
    @rar a     bos2_bin toss.exe
    @rar a     bos2_bin binkpipe.exe
    
clean:
    @del *.obj *.pch *.map *.exe *.dll *.lng *.sym bos2_bin.rar
    
cleanup:
    @del csetc.pch

patcheng:
    btutil32 lng patch bt32.exe english.lng

patchger:
    btutil32 lng patch bt32.exe deutsch.lng

patchstt:
    btutil32 lng patch bt32.exe startrek.lng

bt32.exe:  $(btpch) $(btobjsc)
    $(LINK) @<<
    $(LFLAGS)
    /Febt32.exe
    binkley.def
    $(btobjsc)
    $(LINKLIBS)
    $(BINKLIBS)
<<
#   cppopa3.obj
#   =========== only needed for VAC++ profiler

btutil32.exe: $(cobjs1) $(cobjs2) $(btpch)
    $(LINK) @<<
    $(LFLAGS)
    /Febtutil32.exe
    $(cobjs1)
    $(cobjs2)
    $(LINKLIBS)
<<

v7debug.exe: v7debug.obj
    $(LINK) @<<
    $(LFLAGS)
    /Fev7debug.exe
    v7debug.obj
<<

binkpipe.exe:
    @cd ..\src\os2\binkpipe
    @nmake /f ibmvacpp.mak
    @cd ..\..\..\os2

bink_pm.dll:
    @cd ..\src\os2\bink_pm
    @nmake /f ibmvacpp.mak
    @cd ..\..\..\os2

toss.exe: $(SRC_DIR)\os2\toss\toss.c
    $(CC) /O+ /G5 $(SRC_DIR)\os2\toss\toss.c /Fetoss.exe
          
english.lng: btutil32.exe $(SRC_DIR)\language.txt
    btutil32 lng key E $(SRC_DIR)\language.txt english.lng

startrek.lng: btutil32.exe $(SRC_DIR)\language.txt
    btutil32 lng key S $(SRC_DIR)\language.txt startrek.lng

deutsch.lng: btutil32.exe $(SRC_DIR)\language.txt
    btutil32 lng key G $(SRC_DIR)\language.txt deutsch.lng

swedish.lng: btutil32.exe $(SRC_DIR)\language.txt
    btutil32 lng key V $(SRC_DIR)\language.txt swedish.lng

$(btpch): $(SRC_DIR)\$*.c $(DEF_H)
    @echo compiling $*.c
    $(CC) $(CFLAGS) $(MAKEPCH) $(SRC_DIR)\$*.c

$(btobjs): $(SRC_DIR)\$*.c $(btpch)
    @echo compiling $*.c
    $(CC) $(CFLAGS) $(USEPCH) $(SRC_DIR)\$*.c

btutil.obj: $(SRC_DIR)\btutil.c $(DEF_H)
    $(CC) $(CFLAGS) $(SRC_DIR)\btutil.c

gethcomm.obj: $(SRC_DIR)\gethist\gethcomm.c
    $(CC) $(CFLAGS) $(SRC_DIR)\gethist\gethcomm.c

getra.obj: $(SRC_DIR)\gethist\getra.c
    $(CC) $(CFLAGS) $(SRC_DIR)\gethist\getra.c

getmax3.obj: $(SRC_DIR)\gethist\getmax3.c
    $(CC) $(CFLAGS) $(SRC_DIR)\gethist\getmax3.c

getbgfax.obj: $(SRC_DIR)\gethist\getbgfax.c
    $(CC) $(CFLAGS) $(SRC_DIR)\gethist\getbgfax.c

gettg.obj: $(SRC_DIR)\gethist\gettg.c
    $(CC) $(CFLAGS) $(SRC_DIR)\gethist\gettg.c

version.obj: $(SRC_DIR)\version.c $(btobjs)
    $(CC) $(CFLAGS) $(SRC_DIR)\version.c

v7debug.obj: $(SRC_DIR)\v7debug.c $(DEF_H)
    $(CC) $(CFLAGS) $(SRC_DIR)\v7debug.c

bt32.sym: bt32.map
    @echo creating sym file
    mapsym -a bt32.map

