#   TS 980222
#
#   makefile for BTXE project using IBM NMAKE and the emx 2.7.2.1 compiler
#   in `a.out' output mode with 0.9c runtime under os/2 by tom schlangen.
#
#   instead using macro magic to circumvent the 1024 char command line
#   limit of os/2, i prefered to use an object library for linking
#   purposes.
#
#   therefore, the file `emxaout.rsp' is a response file to the `ar'
#   librarian and contains a list of modules to put into the lib file.
#

CPU = 386

!IF "$(CPU)" == "86"
coptf= -m386 -D_CPU=386
!ENDIF

!IF "$(CPU)" == "186"
coptf= -m386 -D_CPU=386
!ENDIF

!IF "$(CPU)" == "286"
coptf= -m386 -D_CPU=386
!ENDIF

!IF "$(CPU)" == "386"
coptf= -m386 -D_CPU=386
!ENDIF

!IF "$(CPU)" == "486"
coptf= -m486 -D_CPU=486
!ENDIF

!IF "$(CPU)" == "586"
coptf= -m486 -malign-functions=2 -malign-loops=2 -malign-jumps=2 -D_CPU=586
!ENDIF

ADDINCS= -I..\include
ADDLIBS= -L..\lib -L. -lbinkley
OPTFLAGS= -s -O2 -Zcrtdll $(coptf)
DBGFLAGS= -g
MTHREADS= -Zmt
DEFFLAGS= -DNEED_SET_PRIOR -DOS_2 -DHYDRADEV\
 -DNEED_OS2COMMS\
#-DREL_CAND \
#-DDEBUG

INCDIR=..\include
SRCDIR=..\src
LNKFLAGS=

#   optimized, no debug, no snoop:
CFLAGS= -Wall -pedantic $(OPTFLAGS) $(DEFFLAGS) $(ADDINCS)

CC=gcc

#.SUFFIXES: .a .lib .o .obj .c

OBJ1=   aglcrc.o    asyn_os2.o  banner.o    binkdlg.o   brec.o      \
        bsend.o     btconfig.o  b_banner.o  b_bbs.o     b_faxrcv.o  \
        b_flofil.o  b_frproc.o  b_help.o    b_initva.o  \
        b_logs.o    b_passwo.o  b_proto.o   b_rspfil.o  b_sbinit.o  \
        b_script.o  b_search.o  b_sessio.o  b_spawn.o   b_term.o    \
        b_termov.o  b_today.o   b_wazoo.o   b_whack.o   b_wzsend.o  \
        cache.o     callback.o  callerid.o  callwin.o   capi.o      \
        cfosline.o  chat.o      common.o    css.o       data.o      \
        dosfuncs.o  emsi.o      evtparse.o  evtsetup.o  file_all.o  \
        fossil.o    freepoll.o  ftsc.o      history.o   hydra.o     \
        janus.o     keymap.o    langload.o  m7rec.o     m7send.o    \
        mailer.o    mailovly.o  mailroot.o  maxmcp.o    maxprm.o    \
        mdm_proc.o  misc.o      nodeproc.o  os2_pm.o    outbound.o  \
        pipe.o      pktmsgs.o   protcomm.o  recvbark.o  recvsync.o  \
        sbuf.o      sched.o     sendbark.o  sendsync.o  squish.o    \
        srif.o      statetbl.o  stats.o     timer.o     version.o   \
        version7.o  vfos_os2.o  vt100.o     xmrec.o     xmsend.o    \
        yoohoo.o    zmodem.o    file_os2.o  times.o


OBJ2=   gethcomm.o  getra.o     gettg.o     getmax3.o   getbgfax.o

INC1=   $(INCDIR)\bink.h        $(INCDIR)\com.h         \
        $(INCDIR)\com_os2.h     $(INCDIR)\externs.h     \
        $(INCDIR)\includes.h    $(INCDIR)\keybd.h       \
        $(INCDIR)\keyfncs.h     $(INCDIR)\prototyp.h    \
        $(INCDIR)\sbuf.h        $(INCDIR)\sched.h       \
        $(INCDIR)\squish.h      $(INCDIR)\xfer.h        \
        $(INCDIR)\zmodem.h      $(INCDIR)\css.h         \
        $(INCDIR)\callwin.h     $(INCDIR)\msgs.h

default:        all

all:            binkley.a bt32.exe patche toss.exe

$(OBJ1):        $(SRCDIR)\$*.c  $(INC1)
    $(CC) $(MTHREADS) $(CFLAGS) -c $(SRCDIR)\$*.c

gethcomm.o: $(SRCDIR)\gethist\gethcomm.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\gethcomm.c

getra.o: $(SRCDIR)\gethist\getra.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\getra.c

getmax3.o: $(SRCDIR)\gethist\getmax3.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\getmax3.c

getbgfax.o: $(SRCDIR)\gethist\getbgfax.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\getbgfax.c

gettg.o: $(SRCDIR)\gethist\gettg.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\gettg.c

binkley.a:      $(OBJ1) $(OBJ2)
    ar @emxaout.rsp

bt32.exe:       $(SRCDIR)\bt.c  binkley.a
    $(CC) $(MTHREADS) $(CFLAGS) -o bt32.exe $(SRCDIR)\bt.c $(LNKFLAGS) $(ADDLIBS)

btutil32.exe:   $(SRCDIR)\btutil.c  binkley.a
    $(CC) $(CFLAGS) -o btutil32.exe $(SRCDIR)\btutil.c $(LNKFLAGS) $(ADDLIBS)

toss.exe:       $(SRCDIR)\os2\toss\toss.c
    $(CC) $(CFLAGS) -o toss.exe $(SRCDIR)\os2\toss\toss.c

#   generate language file from source

english.lng:    $(SRCDIR)\language.txt btutil32.exe
    btutil32 LNG key E $(SRCDIR)\language.txt english.lng

german.lng:     $(SRCDIR)\language.txt btutil32.exe
    btutil32 LNG key G $(SRCDIR)\language.txt german.lng

startrek.lng:   $(SRCDIR)\language.txt btutil32.exe
    btutil32 LNG key S $(SRCDIR)\language.txt startrek.lng

sveridge.lng:   $(SRCDIR)\language.txt btutil32.exe
    btutil32 LNG key V $(SRCDIR)\language.txt sveridge.lng

#   patch language file into bt32.exe

patche: bt32.exe btutil32.exe english.lng
    btutil32 LNG patch bt32.exe english.lng

patchg: bt32.exe btutil32.exe german.lng
    btutil32 LNG patch bt32.exe german.lng

patchs: bt32.exe btutil32.exe startrek.lng
    btutil32 LNG patch bt32.exe startrek.lng

patchv: bt32.exe btutil32.exe sveridge.lng
    btutil32 LNG patch bt32.exe sveridge.lng

clean:
    -del *.a *.bak *.exe *.dll *.o *.obj *.lib *.lng *.lst *.map *.rar *.sym *.zip
    -del ..\os2_pm\*.sym
    -del ..\include\*.bak ..\src\*.bak

pack:
#   -zip -9 -r -u -g -o btsrcts \binksrc -x *.zip
    -rar a -md1024 -u -r btsrcts \binksrc
    -copy btsrcts.rar a:\

unpack:
#   -unzip -o -u a:\btsrcts.zip -d /
    -rar x -u -y btsrcts \

