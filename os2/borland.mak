#   TE 971012
#
#   Makefile for Borland C 2.0 for OS/2 using Borland make.exe
#
#   Notes
#
#   - BCC/2 does not support CPU-specific optimization, so we always define
#     _CPU=386
#   - Optimization is turned off (safer this way ...)
#   - Creates async version only
#   - Use "make bt32debug.exe" to create an executable with turbo debugger
#     debug information.

# DEBFLAGS=-N -v -y
COMPILERFLAGS=-a- -v -k -X -C -w -H=headers.bor
OPTFLAGS=-O2 -Od
DEFFLAGS= -D_CPU=386 -DNEED_SET_PRIOR -DOS_2 -DHYDRADEV -DCSS \
  -DNEED_OS2COMMS -DUSELNGFILE \
# -DREL_CAND        \
# -DDEBUG

INCDIR=..\include
SRCDIR=..\src

ADDINCS= -I$(INCDIR) -I..\src
ADDLIBS= -L..\lib -L.

CFLAGS= $(DEFFLAGS) $(ADDINCS) $(COMPILERFLAGS) $(OPTFLAGS)

CC=bcc

#.SUFFIXES: .a .lib .o .obj .c

OBJ1=   aglcrc.obj    asyn_os2.obj  banner.obj    binkdlg.obj   brec.obj      \
        bsend.obj     btconfig.obj  b_banner.obj  b_bbs.obj     b_faxrcv.obj  \
        b_flofil.obj  b_frproc.obj  b_help.obj    b_initva.obj  \
        b_logs.obj    b_passwo.obj  b_proto.obj   b_rspfil.obj  b_sbinit.obj  \
        b_script.obj  b_search.obj  b_sessio.obj  b_spawn.obj   b_term.obj    \
        b_termov.obj  b_today.obj   b_wazoo.obj   b_whack.obj   b_wzsend.obj  \
        cache.obj     callback.obj  callerid.obj  capi.obj      cfosline.obj  \
        chat.obj      common.obj    css.obj       data.obj      dosfuncs.obj  \
        emsi.obj      evtparse.obj  evtsetup.obj  file_all.obj  fossil.obj    \
        freepoll.obj  ftsc.obj      history.obj   hydra.obj     janus.obj     \
        keymap.obj    langload.obj  m7rec.obj     m7send.obj    mailer.obj    \
        mailovly.obj  mailroot.obj  maxmcp.obj    mdm_proc.obj  misc.obj      \
        nodeproc.obj  os2_pm.obj    outbound.obj  pipe.obj      pktmsgs.obj   \
        recvbark.obj  recvsync.obj  sbuf.obj      sched.obj     sendbark.obj  \
        sendsync.obj  squish.obj    srif.obj      statetbl.obj  stats.obj     \
        timer.obj     version.obj   version7.obj  vfos_os2.obj  xmrec.obj     \
        xmsend.obj    yoohoo.obj    zmodem.obj    protcomm.obj  vt100.obj     \
        callwin.obj   maxprm.obj    file_os2.obj  times.obj

INC1=   $(INCDIR)\bink.h        $(INCDIR)\com.h         \
        $(INCDIR)\com_os2.h     $(INCDIR)\externs.h     \
        $(INCDIR)\includes.h    $(INCDIR)\keybd.h       \
        $(INCDIR)\keyfncs.h     $(INCDIR)\prototyp.h    \
        $(INCDIR)\sbuf.h        $(INCDIR)\sched.h       \
        $(INCDIR)\squish.h      $(INCDIR)\xfer.h        \
        $(INCDIR)\zmodem.h      $(INCDIR)\css.h



default:        all

all:            bt32.exe patche

includes.sem: $(INC1)
              @if exist *.obj del *.OBJ
              @if exist headers.bor del headers.bor
              @type NUL > includes.sem

.path.c = ..\src
.c.obj:

                @$(CC) $(CFLAGS) -D_MT -D__MT__ -c $*.C

bt32debug.exe:  includes.sem $(OBJ1) bt.obj btutil32.exe
                $(CC) -sm -v @&&|
-sDbinkley.def -ebt32debug.exe $(OBJ1) bt.obj
|

bt32.exe:       includes.sem $(OBJ1) bt.obj
                $(CC) -sm @&&|
-sDbinkley.def -ebt32.exe $(OBJ1) bt.obj
|

gethcomm.obj: $(SRCDIR)\gethist\gethcomm.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\gethcomm.c

getra.obj: $(SRCDIR)\gethist\getra.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\getra.c

getmax3.obj: $(SRCDIR)\gethist\getmax3.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\getmax3.c

getbgfax.obj: $(SRCDIR)\gethist\getbgfax.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\getbgfax.c

gettg.obj: $(SRCDIR)\gethist\gettg.c
    $(CC) $(CFLAGS) -c $(SRCDIR)\gethist\gettg.c

btutil32.exe:   includes.sem btutil.obj  common.obj gethcomm.obj getra.obj getmax3.obj getbgfax.obj gettg.obj
                $(CC) -sm @&&|
-ebtutil32.exe btutil.obj common.obj gethcomm.obj getra.obj getmax3.obj getbgfax.obj gettg.obj
|

#   generate language file from source

english.lng:    $(SRCDIR)\language.txt btutil32.exe
    btutil32 LNG key E $(SRCDIR)\language.txt english.lng

german.lng:     $(SRCDIR)\language.txt btutil32.exe
    btutil32 LNG key G $(SRCDIR)\language.txt german.lng

startrek.lng:   $(SRCDIR)\language.txt btutil32.exe
    btutil32 LNG key S $(SRCDIR)\language.txt startrek.lng

#   patch language file into bt32.exe

patche: bt32.exe btutil32.exe english.lng
    btutil32 LNG patch bt32.exe english.lng

patchg: bt32.exe btutil32.exe german.lng
    btutil32 LNG patch bt32.exe german.lng

patchs: bt32.exe btutil32.exe startrek.lng
    btutil32 LNG patch bt32.exe startrek.lng

clean:
    -del *.bak *.exe *.dll *.obj *.lib *.lng *.lst *.map *.sym *.zip includes.sem headers.bor
    -del ..\include\*.bak ..\src\*.bak ..\src\*~ ..\include\*~

