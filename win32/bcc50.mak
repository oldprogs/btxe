#
# Borland C++ IDE generated makefile
# Generated 13.02.97 at 13:46:07 
#
#.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32i  
TLINK32 = TLink32
TLIB    = TLib
BRC32   = Brc32
TASM32  = Tasm32
#
# IDE macros
#


#
# Options
#
IDE_LinkFLAGS32 =  -LC:\BC5\LIB
LinkerLocalOptsAtC32_btxe32dexe =  -c -Sc:4000
ResLocalOptsAtC32_btxe32dexe = 
BLocalOptsAtC32_btxe32dexe = 
CompInheritOptsAt_btxe32dexe = -IC:\BC5\INCLUDE -I..\include -O2 -OS -OI -5 -DCSS -DHYDRADEV
#-DREL_CAND
BCC32OPTS = -I..\include -O2 -5 -c
LinkerOptsAt_btxe32dexe = $(LinkerLocalOptsAtC32_btxe32dexe)
ResOptsAt_btxe32dexe = $(ResLocalOptsAtC32_btxe32dexe)
BOptsAt_btxe32dexe = $(BLocalOptsAtC32_btxe32dexe)

#
# Dependency List
#
Dep_btxe32 = \
   btxe32.exe

btxe32 : BccW32.cfg $(Dep_btxe32)
  echo MakeNode

Dep_btxe32dexe = \
   zsend.obj\
   zreceive.obj\
   sbuf.obj\
   yoohoo.obj\
   xmsend.obj\
   xmrec.obj\
   vfos_nt.obj\
   version7.obj\
   version.obj\
   times.obj\
   timer.obj\
   statetbl.obj\
   srif.obj\
   squish.obj\
   sendsync.obj\
   sendbark.obj\
   sched.obj\
   zmisc.obj\
   recvsync.obj\
   recvbark.obj\
   portio.obj\
   pipe.obj\
   outbound.obj\
   freepoll.obj\
   misc.obj\
   mdm_proc.obj\
   maxmcp.obj\
   mailroot.obj\
   mailovly.obj\
   mailer.obj\
   m7send.obj\
   m7rec.obj\
   langload.obj\
   keymap.obj\
   janus.obj\
   includes.obj\
   hydra.obj\
   heap.obj\
   ftsc.obj\
   nodeproc.obj\
   file_dos.obj\
   exceptio.obj\
   evtparse.obj\
   emsi.obj\
   dosfuncs.obj\
   debug.obj\
   data.obj\
   css.obj\
   common.obj\
   chat.obj\
   callback.obj\
   cache.obj\
   btconfig.obj\
   bt.obj\
   bsend.obj\
   brec.obj\
   binkdlg.obj\
   banner.obj\
   b_wzsend.obj\
   b_whack.obj\
   b_rspfil.obj\
   b_today.obj\
   b_termov.obj\
   b_term.obj\
   b_spawn.obj\
   b_sessio.obj\
   b_search.obj\
   b_sbinit.obj\
   b_wazoo.obj\
   aglcrc.obj\
   b_passwo.obj\
   b_logs.obj\
   b_initva.obj\
   b_help.obj\
   b_fuser.obj\
   b_frproc.obj\
   b_flofil.obj\
   b_faxrcv.obj\
   b_bbs.obj\
   b_banner.obj\
   async_nt.obj\
   b_proto.obj\
   fossil.obj\
   b_script.obj\
   pktmsgs.obj\
   callerid.obj\
   evtsetup.obj\
   history.obj\
   protcomm.obj\
   stats.obj\
   vt100.obj\
   ..\include\includes.h

Dep_btutil32dexe = \
   btutil.obj\
   misc.obj\
   ..\include\includes.h

btutil32.exe : $(Dep_btutil32dexe)
  $(TLINK32) @&&|
  $(IDE_LinkFLAGS32) $(LinkerOptsAt_btxe32dexe) +
c0x32.obj+
btutil.obj+
common.obj
$<,$*
import32.lib+
cw32.lib

|

btxe32.exe : $(Dep_btxe32dexe)
  $(TLINK32) @&&|
  $(IDE_LinkFLAGS32) $(LinkerOptsAt_btxe32dexe) +
c:\bc5\lib\c0x32.obj+
zsend.obj+
zreceive.obj+
sbuf.obj+
yoohoo.obj+
xmsend.obj+
xmrec.obj+
vfos_nt.obj+
version7.obj+
version.obj+
times.obj+
timer.obj+
statetbl.obj+
srif.obj+
squish.obj+
sendsync.obj+
sendbark.obj+
sched.obj+
zmisc.obj+
recvsync.obj+
recvbark.obj+
portio.obj+
pipe.obj+
outbound.obj+
freepoll.obj+
misc.obj+
mdm_proc.obj+
maxmcp.obj+
mailroot.obj+
mailovly.obj+
mailer.obj+
m7send.obj+
m7rec.obj+
langload.obj+
keymap.obj+
janus.obj+
includes.obj+
hydra.obj+
heap.obj+
ftsc.obj+
nodeproc.obj+
file_dos.obj+
exceptio.obj+
evtparse.obj+
emsi.obj+
dosfuncs.obj+
debug.obj+
data.obj+
css.obj+
common.obj+
chat.obj+
callback.obj+
cache.obj+
btconfig.obj+
bt.obj+
bsend.obj+
brec.obj+
binkdlg.obj+
banner.obj+
b_wzsend.obj+
b_whack.obj+
b_rspfil.obj+
b_today.obj+
b_termov.obj+
b_term.obj+
b_spawn.obj+
b_sessio.obj+
b_search.obj+
b_sbinit.obj+
b_wazoo.obj+
aglcrc.obj+
b_passwo.obj+
b_logs.obj+
b_initva.obj+
b_help.obj+
b_fuser.obj+
b_frproc.obj+
b_flofil.obj+
b_faxrcv.obj+
b_bbs.obj+
b_banner.obj+
async_nt.obj+
b_proto.obj+
fossil.obj+
b_script.obj+
pktmsgs.obj+
callerid.obj+
evtsetup.obj+
history.obj+
protcomm.obj+
stats.obj+
vt100.obj
$<,$*
import32.lib+
cw32.lib+
ntcomm.lib

|

zsend.obj :  ..\src\zsend.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\zsend.c
|

zreceive.obj :  ..\src\zreceive.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\zreceive.c
|

sbuf.obj :  ..\src\sbuf.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\sbuf.c
|

yoohoo.obj :  ..\src\yoohoo.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\yoohoo.c
|

xmsend.obj :  ..\src\xmsend.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\xmsend.c
|

xmrec.obj :  ..\src\xmrec.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\xmrec.c
|

vfos_nt.obj :  ..\src\vfos_nt.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\vfos_nt.c
|

version7.obj :  ..\src\version7.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\version7.c
|

version.obj :  ..\src\version.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\version.c
|

times.obj :  ..\src\times.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\times.c
|

timer.obj :  ..\src\timer.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\timer.c
|

statetbl.obj :  ..\src\statetbl.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\statetbl.c
|

srif.obj :  ..\src\srif.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\srif.c
|

squish.obj :  ..\src\squish.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\squish.c
|

sendsync.obj :  ..\src\sendsync.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\sendsync.c
|

sendbark.obj :  ..\src\sendbark.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\sendbark.c
|

sched.obj :  ..\src\sched.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\sched.c
|

zmisc.obj :  ..\src\zmisc.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\zmisc.c
|

recvsync.obj :  ..\src\recvsync.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\recvsync.c
|

recvbark.obj :  ..\src\recvbark.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\recvbark.c
|

portio.obj :  ..\src\portio.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\portio.c
|

pipe.obj :  ..\src\pipe.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\pipe.c
|

outbound.obj :  ..\src\outbound.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\outbound.c
|

freepoll.obj :  ..\src\freepoll.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\freepoll.c
|

misc.obj :  ..\src\misc.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\misc.c
|

mdm_proc.obj :  ..\src\mdm_proc.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\mdm_proc.c
|

maxmcp.obj :  ..\src\maxmcp.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\maxmcp.c
|

mailroot.obj :  ..\src\mailroot.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\mailroot.c
|

mailovly.obj :  ..\src\mailovly.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\mailovly.c
|

mailer.obj :  ..\src\mailer.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\mailer.c
|

m7send.obj :  ..\src\m7send.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\m7send.c
|

m7rec.obj :  ..\src\m7rec.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\m7rec.c
|

langload.obj :  ..\src\langload.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\langload.c
|

keymap.obj :  ..\src\keymap.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\keymap.c
|

janus.obj :  ..\src\janus.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\janus.c
|

includes.obj :  ..\src\includes.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\includes.c
|

hydra.obj :  ..\src\hydra.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -DDEBUG -o$@ ..\src\hydra.c
|

heap.obj :  ..\src\heap.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\heap.c
|

ftsc.obj :  ..\src\ftsc.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\ftsc.c
|

nodeproc.obj :  ..\src\nodeproc.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\nodeproc.c
|

file_dos.obj :  ..\src\file_dos.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\file_dos.c
|

exceptio.obj :  ..\src\exceptio.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\exceptio.c
|

evtparse.obj :  ..\src\evtparse.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\evtparse.c
|

emsi.obj :  ..\src\emsi.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\emsi.c
|

dosfuncs.obj :  ..\src\dosfuncs.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\dosfuncs.c
|

debug.obj :  ..\src\debug.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\debug.c
|

data.obj :  ..\src\data.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\data.c
|

css.obj :  ..\src\css.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\css.c
|

common.obj :  ..\src\common.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\common.c
|

chat.obj :  ..\src\chat.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\chat.c
|

callback.obj :  ..\src\callback.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\callback.c
|

cache.obj :  ..\src\cache.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\cache.c
|

btconfig.obj :  ..\src\btconfig.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\btconfig.c
|

bt.obj :  ..\src\bt.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\bt.c
|

bsend.obj :  ..\src\bsend.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\bsend.c
|

brec.obj :  ..\src\brec.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\brec.c
|

binkdlg.obj :  ..\src\binkdlg.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\binkdlg.c
|

banner.obj :  ..\src\banner.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\banner.c
|

b_wzsend.obj :  ..\src\b_wzsend.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_wzsend.c
|

b_whack.obj :  ..\src\b_whack.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_whack.c
|

b_rspfil.obj :  ..\src\b_rspfil.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_rspfil.c
|

b_today.obj :  ..\src\b_today.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_today.c
|

b_termov.obj :  ..\src\b_termov.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_termov.c
|

b_term.obj :  ..\src\b_term.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_term.c
|

b_spawn.obj :  ..\src\b_spawn.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_spawn.c
|

b_sessio.obj :  ..\src\b_sessio.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_sessio.c
|

b_search.obj :  ..\src\b_search.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_search.c
|

b_sbinit.obj :  ..\src\b_sbinit.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_sbinit.c
|

b_wazoo.obj :  ..\src\b_wazoo.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_wazoo.c
|

aglcrc.obj :  ..\src\aglcrc.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\aglcrc.c
|

b_passwo.obj :  ..\src\b_passwo.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_passwo.c
|

b_logs.obj :  ..\src\b_logs.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_logs.c
|

b_initva.obj :  ..\src\b_initva.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_initva.c
|

b_help.obj :  ..\src\b_help.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_help.c
|

b_fuser.obj :  ..\src\b_fuser.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_fuser.c
|

b_frproc.obj :  ..\src\b_frproc.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_frproc.c
|

b_flofil.obj :  ..\src\b_flofil.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_flofil.c
|

b_faxrcv.obj :  ..\src\b_faxrcv.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_faxrcv.c
|

b_bbs.obj :  ..\src\b_bbs.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_bbs.c
|

b_banner.obj :  ..\src\b_banner.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_banner.c
|

async_nt.obj :  ..\src\async_nt.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\async_nt.c
|

b_proto.obj :  ..\src\b_proto.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\b_proto.c
|

fossil.obj :    ..\src\fossil.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\fossil.c
|

pktmsgs.obj :    ..\src\pktmsgs.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\pktmsgs.c
|

callerid.obj :    ..\src\callerid.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\callerid.c
|

evtsetup.obj :    ..\src\evtsetup.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\evtsetup.c
|

history.obj :    ..\src\history.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\history.c
|

protcomm.obj :    ..\src\protcomm.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\protcomm.c
|

stats.obj :    ..\src\stats.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\stats.c
|

btutil.obj :    ..\src\btutil.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\btutil.c
|

vt100.obj :    ..\src\vt100.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_btxe32dexe) $(CompInheritOptsAt_btxe32dexe) -o$@ ..\src\vt100.c
|

b_script.obj :  ..\src\b_script.c
  bcc32 $(BCC32OPTS) ..\src\b_script.c

#Compiler configuration file
BccW32.cfg : 
   Copy &&|
-w
-R
-v
-vi
-H
-H=btxe32.csm
-WC
| $@


