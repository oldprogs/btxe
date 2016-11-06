####################################################################
#                                                                  #
#  Make file for BinkleyTerm-OS/2 Version 2.60 XE                  #
#                                                                  #
#  Compiler: CSet/2 2.01                                           #
#                                                                  #
#  use MAKE all                                                    #
#  or  MAKE bt32.exe                                               #
#  or  MAKE btutil32.exe                                           #
#                                                                  #
#  This file is used to build the 32-bit version of the            #
#  software, for use on OS/2 2.x and Warp.                         #
#                                                                  #
####################################################################


SRC_DIR    = ..\src
INCL_DIR   = ..\include
LIB_DIR    = ..\lib

LINKLIBS    = os2386.lib
BINKLIBS    = $(LIB_DIR)\maxcomm.lib $(LIB_DIR)\snserver.lib $(LIB_DIR)\mcp32.lib

!IF "$(CPU)" == ""
CPU=486
!ENDIF

!IF "$(CPU)" == "86"
coptf= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "186"
coptf= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "286"
coptf= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "386"
coptf= /G3 /D_CPU=386
!ENDIF

!IF "$(CPU)" == "486"
coptf= /G4 /D_CPU=486
!ENDIF

!IF "$(CPU)" == "586"
coptf= /G5 /D_CPU=586
!ENDIF

# for BT version with snoop and MCP (normal / debug):
OPTFLAGS = /DNEED_SET_PRIOR /DOS_2 /DHYDRADEV /DPROMETER /DOS2SETTITLE /DMAX_MCP /DSnoop
# /DNEED_OS2COMMS /DCSS

DEBFLAGS = /Ti /Tm+ /Tx+ /O-
WARNFLAGS = /W3 /Wpar /Wpro /Wrea /Wret /Wtru /Wund /Wuse
# WARNFLAGS  = WARNFLAGS = /W3 /Wpar /Wpro /Wrea /Wret /Wtru /Wund /Wuse /Kbcepr 
# LISTFLAGS = /L+ /La+
LISTFLAGS =
CFLAGS_N = /O+ /G3 /Sm /Sp1 /Ss /C /Tl30 /Q $(WARNFLAGS) $(OPTFLAGS) /I $(INCL_DIR) $(LISTFLAGS) 
CFLAGS_D = $(DEBFLAGS) $(CFLAGS_N)

LFLAGS_N = /map /noi /nol /pmtype:vio /align:16 /base:65536 /stack:32768 /exepack
LFLAGS_D = $(LFLAGS_N)

CFLAGS   = $(CFLAGS_N)
LFLAGS   = $(LFLAGS_N)

CC       = ICC
LINK     = LINK386

USEPCH  = /Si
MAKEPCH = /Fi

DEF_H1   = $(INCL_DIR)\com.h $(INCL_DIR)\com_os2.h $(INCL_DIR)\xfer.h
DEF_H2   = $(INCL_DIR)\bink.h $(INCL_DIR)\zmodem.h $(INCL_DIR)\keybd.h
DEF_H3   = $(INCL_DIR)\sbuf.h $(INCL_DIR)\sched.h $(INCL_DIR)\externs.h
DEF_H4   = $(INCL_DIR)\includes.h $(INCL_DIR)\prototyp.h $(INCL_DIR)\keyfncs.h
DEF_H5   = $(INCL_DIR)\squish.h $(INCL_DIR)\debug.h 

DEF_H    = $(DEF_H1) $(DEF_H2) $(DEF_H3) $(DEF_H4) $(DEF_H5)

cobj0=banner.obj   callback.obj
cobj1=bt.obj       asyn_os2.obj ftsc.obj     b_script.obj sbuf.obj
cobj2=btconfig.obj brec.obj     b_search.obj b_today.obj
cobj3=b_termov.obj bsend.obj    b_sessio.obj b_faxrcv.obj
cobj4=b_banner.obj b_spawn.obj
cobj5=b_bbs.obj    mailer.obj   b_term.obj   chat.obj
cobj6=b_flofil.obj mailroot.obj b_wazoo.obj  mailovly.obj squish.obj
cobj7=b_frproc.obj mdm_proc.obj b_whack.obj  sched.obj
cobj8=misc.obj     b_wzsend.obj timer.obj    css.obj
cobj9=b_help.obj   nodeproc.obj sendbark.obj vfos_os2.obj times.obj
cobja=b_initva.obj m7rec.obj    data.obj     m7send.obj   emsi.obj
cobjb=b_passwo.obj yoohoo.obj   version7.obj maxmcp.obj   file_os2.obj
cobjc=b_proto.obj  dosfuncs.obj zmisc.obj    prometer.obj debug.obj
cobjd=b_rspfil.obj evtparse.obj zreceive.obj freepoll.obj srif.obj
cobje=b_sbinit.obj fossil.obj   zsend.obj    version.obj  heap.obj
cobjf=keymap.obj   janus.obj    outbound.obj sendsync.obj file_all.obj
cobjg=statetbl.obj xmrec.obj    xmsend.obj   langload.obj cache.obj
cobjh=recvbark.obj recvsync.obj hydra.obj    aglcrc.obj   b_logs.obj
cobjs1=btutil.obj  common.obj

btobjs = $(cobj0) $(cobj1) $(cobj2) $(cobj3) $(cobj4) $(cobj5) $(cobj6) $(cobj7) $(cobj8) $(cobj9) $(cobja) $(cobjb) $(cobjc) $(cobjd) $(cobje) $(cobjf) $(cobjg) $(cobjh)
btobjsc = $(btobjs) common.obj

btpch = includes.obj

all: btutil32.exe bt32.exe english.lng startrek.lng deutsch.lng patcheng

clean:
        !del *.obj *.pch *.map *.exe *.lng

patcheng :
        btutil32 lng patch bt32.exe english.lng

patchstt :
        btutil32 lng patch bt32.exe startrek.lng


bt32.exe:  $(btpch) $(btobjsc)
        $(LINK) @<<file.lrf
 $(btobjsc: = +^
)
bt32.exe$(LFLAGS)
bt32.map
$(LINKLIBS) $(BINKLIBS)
nul.def
<<

bt32dbg.exe:  $(btpch) $(btobjs)
        $(LINK) @<<file.lrf
 /debug $(btobjs: = +^
)
bt32dbg.exe$(LFLAGS)
bt32dbg.map
$(LINKLIBS) $(BINKLIBS)
nul.def
<<

btutil32.exe: $(cobjs1)
        $(LINK) $(cobjs1),btutil32.exe$(LFLAGS),nul.map,$(LINKLIBS),nul.def

english.lng: btutil32.exe $(SRC_DIR)\language.txt
 btutil32 lng key E $(SRC_DIR)\language.txt english.lng

startrek.lng: btutil32.exe $(SRC_DIR)\language.txt
 btutil32 lng key S $(SRC_DIR)\language.txt startrek.lng

deutsch.lng: btutil32.exe $(SRC_DIR)\language.txt
 btutil32 lng key G $(SRC_DIR)\language.txt deutsch.lng

$(btpch): $(SRC_DIR)\$*.c $(DEF_H)
        $(CC) $(CFLAGS) $(MAKEPCH) $(SRC_DIR)\$*.c

$(btobjs): $(SRC_DIR)\$*.c $(btpch)
        $(CC) $(CFLAGS) $(USEPCH) $(SRC_DIR)\$*.c

common.obj: $(SRC_DIR)\common.c $(DEF_H)
        $(CC) $(CFLAGS) $(SRC_DIR)\common.c

btutil.obj: $(SRC_DIR)\btutil.c $(DEF_H)
        $(CC) $(CFLAGS) $(SRC_DIR)\btutil.c

