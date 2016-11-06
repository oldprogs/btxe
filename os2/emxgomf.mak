#   TS 980222 / SMS 980705
#
#   makefile for BTXE project using GNU MAKE and the emx 2.7.2.1 compiler
#   in `omf' output mode with 0.9c runtime under os/2 by tom schlangen.
#   Migration from nmake to gnu make and 'a.out' to 'omf' by Sascha Silbe.
#
#   instead using macro magic to circumvent the 1024 char command line
#   limit of os/2, i prefered to use an object library for linking
#   purposes.
#
#   therefore, the file `emxomf.rsp' is a response file to the `ar'
#   librarian and contains a list of modules to put into the lib file.
#

CPU = 486
USEMAXCOMM = 0

ifeq "$(CPU)" "86"
coptf= -m386 -D_CPU=386
endif

ifeq "$(CPU)" "186"
coptf= -m386 -D_CPU=386
endif

ifeq "$(CPU)" "286"
coptf= -m386 -D_CPU=386
endif

ifeq "$(CPU)" "386"
coptf= -m386 -D_CPU=386
endif

ifeq "$(CPU)" "486"
coptf= -m486 -D_CPU=486
endif

ifeq "$(CPU)" "586"
coptf= -m486 -malign-functions=2 -malign-loops=2 -malign-jumps=2 -D_CPU=586
endif

ADDINCS= -I../include
ifeq "$(USEMAXCOMM)" "1"
ADDLIBS= -L../lib -L. -lmaxcomm -lmcp32 -lbinkley
else
ADDLIBS= -L../lib -L. -lmcp32 -lbinkley
endif
OPTFLAGS= -s -O2 -Zcrtdll -Zomf $(coptf)
DBGFLAGS= -g
MTHREADS= -Zmt

# available flags:
# DEBUG			- generate line information etc. for use with 
#			  debugger
# CACHE			- Outbound-Cache
# NEED_SET_PRIOR	- set priority
# NEED_OS2COMMS		- OS/2 only: use internal async module instead of 
#			  maxcomm.dll
# USE_WINFOSSIL		- Win32 only: use WinFossil instead of ntcomm.dll
# EXCEPTIONS		- install exceptions handler
# HEAPCHECK		- install heapcheck handler
# BBS_INTERNAL		- internal BBS
# REL_CAND		- release candidate (use commandline parameter
#			  'notreleasedyet'
# OVERLAYS		- DOS only: use overlays
# SNOOP			- named pipes
# MAX_MCP		- Maximus MCP
# HYDRADEV		- Hydra

DEFFLAGS1= -DNEED_SET_PRIOR -DOS_2 -DHYDRADEV -DSNOOP -DMAX_MCP -DCACHE
#-DREL_CAND \
#-DDEBUG
ifeq "$(USEMAXCOMM)" "1"
DEFFLAGS= $(DEFFLAGS1)
else
DEFFLAGS= $(DEFFLAGS1) -DNEED_OS2COMMS
endif

INCDIR=../include
SRCDIR=../src
LNKFLAGS=

#   optimized, no debug, no snoop:
CFLAGS= -Wall -pedantic $(OPTFLAGS) $(DEFFLAGS) $(ADDINCS)

CC=gcc

#.SUFFIXES: .a .lib .o .obj .c

OBJ1=   aglcrc.obj   asyn_os2.obj banner.obj   binkdlg.obj  brec.obj     \
        bsend.obj    btconfig.obj b_banner.obj b_bbs.obj    b_faxrcv.obj \
        b_flofil.obj b_frproc.obj b_help.obj   b_initva.obj b_logs.obj   \
        b_passwo.obj b_proto.obj  b_rspfil.obj b_sbinit.obj b_script.obj \
        b_search.obj b_sessio.obj b_spawn.obj  b_term.obj   b_termov.obj \
        b_today.obj  b_wazoo.obj  b_whack.obj  b_wzsend.obj cache.obj    \
        callback.obj callerid.obj callwin.obj  capi.obj     cfosline.obj \
        chat.obj     common.obj   css.obj      data.obj     dosfuncs.obj \
        emsi.obj     emxmxcom.obj evtparse.obj evtsetup.obj file_all.obj \
        fossil.obj   freepoll.obj ftsc.obj     history.obj  hydra.obj    \
        janus.obj    keymap.obj   langload.obj m7rec.obj    m7send.obj   \
        mailer.obj   mailovly.obj mailroot.obj maxmcp.obj   maxprm.obj   \
        mdm_proc.obj misc.obj     nodeproc.obj os2_pm.obj   outbound.obj \
        pipe.obj     pktmsgs.obj  protcomm.obj recvbark.obj recvsync.obj \
        sbuf.obj     sched.obj    sendbark.obj sendsync.obj squish.obj   \
        srif.obj     statetbl.obj stats.obj    timer.obj    version.obj  \
        version7.obj vfos_os2.obj vt100.obj    xmrec.obj    xmsend.obj   \
        yoohoo.obj   zmodem.obj   file_os2.obj times.obj


OBJ2=   gethcomm.obj getra.obj    gettg.obj    getmax3.obj  getbgfax.obj

INC1=   $(INCDIR)/bink.h        $(INCDIR)/com.h         \
        $(INCDIR)/com_os2.h     $(INCDIR)/externs.h     \
        $(INCDIR)/includes.h    $(INCDIR)/keybd.h       \
        $(INCDIR)/keyfncs.h     $(INCDIR)/prototyp.h    \
        $(INCDIR)/sbuf.h        $(INCDIR)/sched.h       \
        $(INCDIR)/squish.h      $(INCDIR)/xfer.h        \
        $(INCDIR)/zmodem.h      $(INCDIR)/css.h         \
        $(INCDIR)/callwin.h     $(INCDIR)/msgs.h

default:        all

all:            binkley.lib bt32.exe patche toss.exe

#$(OBJ1):        $(SRCDIR)/$(wildcard *.c)  $(INC1)
#	$(CC) $(MTHREADS) $(CFLAGS) -c $(SRCDIR)/$(wildcard *.c)

$(OBJ1):        %.obj: $(SRCDIR)/%.c
	$(CC) $(MTHREADS) $(CFLAGS) -c $<


gethcomm.obj: $(SRCDIR)/gethist/gethcomm.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/gethist/gethcomm.c

getra.obj: $(SRCDIR)/gethist/getra.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/gethist/getra.c

getmax3.obj: $(SRCDIR)/gethist/getmax3.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/gethist/getmax3.c

getbgfax.obj: $(SRCDIR)/gethist/getbgfax.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/gethist/getbgfax.c

gettg.obj: $(SRCDIR)/gethist/gettg.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/gethist/gettg.c

binkley.lib:      $(OBJ1) $(OBJ2)
	emxomfar @emxomf.rsp

bt32.exe:       $(SRCDIR)/bt.c  binkley.lib
	$(CC) $(MTHREADS) $(CFLAGS) -o bt32.exe $(SRCDIR)/bt.c $(LNKFLAGS) $(ADDLIBS) binkley.def

btutil32.exe:   $(SRCDIR)/btutil.c  binkley.lib
	$(CC) $(CFLAGS) -o btutil32.exe $(SRCDIR)/btutil.c $(LNKFLAGS) $(ADDLIBS) binkley.def

toss.exe:       $(SRCDIR)/os2/toss/toss.c
	$(CC) $(CFLAGS) -o toss.exe $(SRCDIR)/os2/toss/toss.c binkley.def

#   generate language file from source

english.lng:    $(SRCDIR)/language.txt btutil32.exe
	./btutil32.exe LNG key E $(SRCDIR)/language.txt english.lng

german.lng:     $(SRCDIR)/language.txt btutil32.exe
	./btutil32.exe LNG key G $(SRCDIR)/language.txt german.lng

startrek.lng:   $(SRCDIR)/language.txt btutil32.exe
	./btutil32.exe LNG key S $(SRCDIR)/language.txt startrek.lng

sveridge.lng:   $(SRCDIR)/language.txt btutil32.exe
	./btutil32.exe LNG key V $(SRCDIR)/language.txt sveridge.lng

#   patch language file into bt32.exe

patche: bt32.exe btutil32.exe english.lng
	./btutil32.exe LNG patch bt32.exe english.lng

patchg: bt32.exe btutil32.exe german.lng
	./btutil32.exe LNG patch bt32.exe german.lng

patchs: bt32.exe btutil32.exe startrek.lng
	./btutil32.exe LNG patch bt32.exe startrek.lng

patchv: bt32.exe btutil32.exe sveridge.lng
	./btutil32.exe LNG patch bt32.exe sveridge.lng

clean:
	-del *.a *.bak *.exe *.dll *.o *.obj *.lib *.lng *.lst *.map *.rar *.sym *.zip
	-del ..\\os2_pm\\*.sym
	-del ..\\include\\*.bak ..\\src\\*.bak

pack:
#	-zip -9 -r -u -g -o btsrcts /binksrc -x *.zip
	-rar a -md1024 -u -r btsrcts \\binksrc
	-copy btsrcts.rar a:\\

unpack:
#	-unzip -o -u a:/btsrcts.zip -d /
	-rar x -u -y btsrcts \\

