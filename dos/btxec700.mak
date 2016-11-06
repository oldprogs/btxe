#############################################################################
#                                                                           #
# MakeFile for BTXE/DOS by tom schlangen                        TS 980712   #
# Based on the original Bink/DOS makefile                                   #
# Suited for compilers MSC V7.0, MSC V8.00 (aka Visual C/C++), and the      #
# assemblers MS MASM 5.1 or Borland TASM 2.01                               #
#                                                                           #
#############################################################################
#                                                                           #
# Usage: nmake -f BTXEC700.MAK ["CPU=<string>"] [optional target]           #
#                                                                           #
# If no target is specified, the default will generate a non-overlayed,     #
# (or `BTBIG', if you prefer) DOS executable, patched with the english      #
# language messages and optimized for no specific processor.                #
#                                                                           #
# Defining "CPU=<string>" (<string> may be `86', `186', `286', `386')       #
# on the nmake command line will generate code optimized for the            #
# correspondingly name processor. Using MSC700, `186' generates the same    #
# code as `286', and all processors above `286' are mapped to `286'.        #
#                                                                           #
#############################################################################

#############################################################################
#                                                                           #
# Definitions                                                               #
#                                                                           #
#############################################################################
PROJ = BINKLEY
PROJFILE = BINKLEY.MAK

SRC_DIR  = ..\src
INCL_DIR = ..\include

#############################################################################
#                                                                           #
# Define `masm' or `tasm' according to the assembler you use                #
#                                                                           #
#############################################################################
ASM = tasm
CC  = cl

LINKER = link
LRF  = echo > NUL

#############################################################################
#                                                                           #
# Recommended optimizing command line switches:                             #
#   for MSC 4.0, instead of -Oslern, use -Ot                                #
#   for MSC 5.0,                     use -Os                                #
#   for MSC 5.1,                     use -Oxrn                              #
#   for MSC 6.0a,                    use -Olerg                             #
#   for MSC 7.0a,                    use -Os                                #
#   for MSC 8.0,                     use -Os                                #
#                                                                           #
#############################################################################

!IF "$(DEBUG)" != ""
cdebug = -Gt -Gt32 -Od -Os -Zi 
!ELSE
cdebug = -Gs -Gt32 -Os 
!ENDIF

!IF "$(WARNINGS)" != ""
cwarn = -W4
!ELSE
cwarn = -W3
!ENDIF

coptf= -G3
cpuid= -D_CPU=386
ccflgs = -c -AL -Gx- /DHYDRADEV /DCSS /DCHAT /DREL_CAND /nologo
cclcl = -Zp -I $(INCL_DIR)

CFLAGS = $(ccflgs) $(coptf) $(cdebug) $(cwarn) $(cpuid) $(cclcl)

#############################################################################
#                                                                           #
# Objects needed by the mailer executable                                   #
#                                                                           #
#############################################################################
COBJS = aglcrc.obj   b_banner.obj b_bbs.obj    b_faxrcv.obj \
        b_flofil.obj b_frproc.obj b_help.obj   b_initva.obj b_logs.obj   \
        file_all.obj b_passwo.obj b_proto.obj  b_rspfil.obj b_sbinit.obj \
        b_script.obj b_search.obj b_sessio.obj b_term.obj   b_termov.obj \
        b_today.obj  b_wazoo.obj  b_whack.obj  b_wzsend.obj banner.obj   \
        bink_asm.obj brec.obj     bsend.obj    bt.obj       btconfig.obj \
        cache.obj    callback.obj callerid.obj chat.obj     css.obj      \
        data.obj     dosfuncs.obj emsi.obj     evtparse.obj evtsetup.obj \
        file_dos.obj fossil.obj   freepoll.obj ftsc.obj     history.obj  \
        hydra.obj    janus.obj    keymap.obj   langload.obj m7rec.obj    \
        m7send.obj   mailer.obj   mailovly.obj mailroot.obj maxmcp.obj   \
        mdm_proc.obj misc.obj     nodeproc.obj outbound.obj pktmsgs.obj  \
        protcomm.obj recvbark.obj recvsync.obj sbuf.obj     sched.obj    \
        sendbark.obj sendsync.obj squish.obj   srif.obj     statetbl.obj \
        stats.obj    timer.obj    version7.obj vfos_dos.obj vt100.obj    \
        xmrec.obj    xmsend.obj   yoohoo.obj   zmodem.obj   callwin.obj  \
        maxprm.obj   times.obj
   
NOOVLOBJS = b_spwnno.obj    vers_no.obj
OVLOBJS = b_spwnov.obj      vers_ov.obj

#############################################################################
#                                                                           #
# Objects common/shared between several executables                         #
#                                                                           #
#############################################################################
COBJCOMMON = common.obj

#############################################################################
#                                                                           #
# Objects needed by the BTUTIL executable                                   #
#                                                                           #
#############################################################################
COBJ2 = btutil.obj gethcomm.obj getra.obj gettg.obj getmax3.obj getbgfax.obj

#############################################################################
#                                                                           #
# Objects assembled by the MASM or TASM assemblers                          #
#                                                                           #
#############################################################################
MOBJS = spawn.obj

#############################################################################
#                                                                           #
# Includes                                                                  #
#                                                                           #
#############################################################################
INCL_FILES = $(INCL_DIR)\includes.h $(INCL_DIR)\bink.h                   \
             $(INCL_DIR)\defines.h $(INCL_DIR)\com.h                     \
             $(INCL_DIR)\com_dos.h $(INCL_DIR)\xfer.h                    \
             $(INCL_DIR)\msgs.h $(INCL_DIR)\zmodem.h $(INCL_DIR)\keybd.h \
             $(INCL_DIR)\keyfncs.h $(INCL_DIR)\sbuf.h                    \
             $(INCL_DIR)\sched.h $(INCL_DIR)\timer.h                     \
             $(INCL_DIR)\vfossil.h $(INCL_DIR)\externs.h                 \
             $(INCL_DIR)\prototyp.h $(INCL_DIR)\squish.h

#############################################################################
#                                                                           #
# Dependency lists                                                          #
#                                                                           #
#############################################################################
bt:     bt.exe btutil.exe english.lng german.lng startrek.lng       \
        sveridge.lng patche

#   generate language file from source

english.lng:    $(SRC_DIR)\language.txt btutil.exe
    btutil LNG key E $(SRC_DIR)\language.txt english.lng

german.lng:     $(SRC_DIR)\language.txt btutil.exe
    btutil LNG key G $(SRC_DIR)\language.txt german.lng

startrek.lng:   $(SRC_DIR)\language.txt btutil.exe
    btutil LNG key S $(SRC_DIR)\language.txt startrek.lng

sveridge.lng:   $(SRC_DIR)\language.txt btutil.exe
    btutil LNG key V $(SRC_DIR)\language.txt sveridge.lng

#   patch language file into bt.exe

patche: bt.exe btutil.exe english.lng
    btutil LNG patch bt.exe english.lng

patchg: bt.exe btutil.exe german.lng
    btutil LNG patch bt.exe german.lng

patchs: bt.exe btutil.exe startrek.lng
    btutil LNG patch bt.exe startrek.lng

patchs: bt.exe btutil.exe sveridge.lng
    btutil LNG patch bt.exe sveridge.lng

#############################################################################
#                                                                           #
# Rulez for the mailer executables                                          #
#                                                                           #
#############################################################################
bt.exe: $(COBJS) $(OVLOBJS) $(MOBJS) $(COBJCOMMON)
        $(LRF) @<<$(PROJ).lrf
/INFO/FAR/PACKC/PACKF/DYNAMIC:391/SEG:256 $(OVLOBJS) + $(COBJS: = +^
) +
$(MOBJS: = +^
) +
$(COBJCOMMON: = +^
)
$*/STACK:24576 /NOI
$*/MAP:FULL/LI,,BTXEC700
<<
        $(LINKER) @$(PROJ).lrf

bt-dbg.exe: $(COBJS) $(NOOVLOBJS) $(MOBJS)
    $(LRF) @<<$(PROJ).lrf
/CODEVIEW/NOE/INFO/FAR/DYNAMIC:300/SEG:160 $(NOOVLOBJS) + $(COBJS: = +^
) +
$(MOBJS: = +^
) +
$(COBJCOMMON: = +^
)
$*/STACK:32767 /NOI
$*/MAP/LI;
<<
    $(LINKER) @$(PROJ).lrf

btutil.exe: $(COBJ2) $(COBJCOMMON)
    link /EXEPACK $(COBJ2) $(COBJCOMMON), $*/stack:3000,$*/map/li;

#############################################################################
#                                                                           #
# Rulez for the history importer modules.                                   #
#                                                                           #
#############################################################################
gethcomm.obj:   $(SRC_DIR)\gethist\gethcomm.c
    $(CC) $(CFLAGS) -I $(SRC_DIR)\gethist $(SRC_DIR)\gethist\gethcomm.c

getra.obj:      $(SRC_DIR)\gethist\getra.c
    $(CC) $(CFLAGS) -I $(SRC_DIR)\gethist $(SRC_DIR)\gethist\getra.c

getmax3.obj:    $(SRC_DIR)\gethist\getmax3.c
    $(CC) $(CFLAGS) -I $(SRC_DIR)\gethist $(SRC_DIR)\gethist\getmax3.c

getbgfax.obj:   $(SRC_DIR)\gethist\getbgfax.c
    $(CC) $(CFLAGS) -I $(SRC_DIR)\gethist $(SRC_DIR)\gethist\getbgfax.c

gettg.obj:      $(SRC_DIR)\gethist\gettg.c
    $(CC) $(CFLAGS) -I $(SRC_DIR)\gethist $(SRC_DIR)\gethist\gettg.c


#############################################################################
#                                                                           #
# Rulez for .C -> .OBJ                                                      #
#                                                                           #
#############################################################################
$(COBJS): $(SRC_DIR)\$*.c includes.obj
    $(CC) $(CFLAGS) /Yuincludes.h $(SRC_DIR)\$*.c

$(COBJCOMMON): $(SRC_DIR)\$*.c includes.obj
    $(CC) $(CFLAGS) /Yuincludes.h $(SRC_DIR)\$*.c

b_spwnno.obj: $(SRC_DIR)\b_spawn.c includes.obj
    $(CC) $(CFLAGS) -Fob_spwnno.obj /Yuincludes.h $(SRC_DIR)\b_spawn.c

b_spwnov.obj: $(SRC_DIR)\b_spawn.c includes.obj
    $(CC) -DOVERLAYS $(CFLAGS) -Fob_spwnov.obj /Yuincludes.h $(SRC_DIR)\b_spawn.c

vers_no.obj: $(SRC_DIR)\version.c includes.obj
    $(CC) $(CFLAGS) -Fovers_no.obj /Yuincludes.h $(SRC_DIR)\version.c

vers_ov.obj: $(SRC_DIR)\version.c includes.obj
    $(CC) -DOVERLAYS $(CFLAGS) -Fovers_ov.obj /Yuincludes.h $(SRC_DIR)\version.c

#############################################################################
#                                                                           #
# Rulez for .ASM -> .OBJ                                                    #
#                                                                           #
# use additional switch -Zi for debugging                                   #
#                                                                           #
#############################################################################
$(MOBJS): $(SRC_DIR)\$*.asm
    $(ASM) -mx -DMODL=LARGE $(SRC_DIR)\$*;
    
#############################################################################
#                                                                           #
# Rulez for some special cases                                              #
#                                                                           #
#############################################################################

btutil.obj: $(SRC_DIR)\$*.c includes.obj
    $(CC) $(CFLAGS) /Yuincludes.h $(SRC_DIR)\btutil.c

langload.obj: $(INCL_DIR)\msgs.h

includes.obj: $(INCL_FILES)
    $(CC) $(CFLAGS) /Ycincludes.h $(SRC_DIR)\includes.c

