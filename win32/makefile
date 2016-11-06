# ---------------------------------------------------------------------------
# Makefile for Borland C++ Builder
# Written by Henk-Jan Kreuger
# Last modified: 01/06/1999

# Makefile version - do not edit
VERSION = BCB.01

#Specify version: Ntcomm.dll or WinFossil
!ifndef TARVER
TARVER = WF                          # {NT, WF}
!endif

# ---------------------------------------------------------------------------
!ifndef BCB
BCB = $(MAKEDIR)\..
!endif
# ---------------------------------------------------------------------------
OBJFILES_BT = \
             aglcrc.obj\
             asyn_w32.obj\
             b_banner.obj\
             b_bbs.obj\
             b_faxrcv.obj\
             b_flofil.obj\
             b_frproc.obj\
             b_help.obj\
             b_initva.obj\
             b_logs.obj\
             b_passwo.obj\
             b_proto.obj\
             b_rspfil.obj\
             b_sbinit.obj\
             b_script.obj\
             b_search.obj\
             b_sessio.obj\
             b_spawn.obj\
             b_term.obj\
             b_termov.obj\
             b_today.obj\
             b_wazoo.obj\
             b_whack.obj\
             b_wzsend.obj\
             banner.obj\
             binkdlg.obj\
             brec.obj\
             bsend.obj\
             bt.obj\
             btconfig.obj\
             cache.obj\
             callback.obj\
             callerid.obj\
             callwin.obj\
             chat.obj\
             common.obj\
             css.obj\
             data.obj\
             debug.obj\
             dosfuncs.obj\
             emsi.obj\
             evtparse.obj\
             evtsetup.obj\
             file_all.obj\
             file_w32.obj\
             freepoll.obj\
             ftsc.obj\
             heap.obj\
             history.obj\
             hydra.obj\
             includes.obj\
             janus.obj\
             keymap.obj\
             langload.obj\
             m7rec.obj\
             m7send.obj\
             mailer.obj\
             mailovly.obj\
             mailroot.obj\
             maxmcp.obj\
             maxprm.obj\
             mdm_proc.obj\
             misc.obj\
             nodeproc.obj\
             outbound.obj\
             pipe.obj\
             pktmsgs.obj\
             protcomm.obj\
             recvbark.obj\
             recvsync.obj\
             sbuf.obj\
             sched.obj\
             sendbark.obj\
             sendsync.obj\
             squish.obj\
             srif.obj\
             statetbl.obj\
             stats.obj\
             timer.obj\
             times.obj\
             version.obj\
             version7.obj\
             vfos_w32.obj\
             vt100.obj\
             xmrec.obj\
             xmsend.obj\
             yoohoo.obj\
             zmodem.obj\
#for BBS_INTERNAL
             script2.obj\
             bbs_ctra.obj\
             bbs_data.obj\
             bbs_io.obj
#end BBS_INTERNAL

OBJFILES_UT = \
             btutil.obj\
             common.obj\
             gethcomm.obj\
             getra.obj\
             getbgfax.obj\
             getmax3.obj\
             gettg.obj
# ---------------------------------------------------------------------------
!if ($(TARVER) == NT)
LIBFILES = ntcomm.lib
!else
LIBFILES =
!endif
!if ($(TARVER) == NT)
VERFLAG = NT
!else
VERFLAG = USE_WINFOSSIL
!endif
# ---------------------------------------------------------------------------
CFLAG1 = -3 -a- -c -H -Hs -O2 -tWC -w -w-aus -w-csu -w-sig
CFLAG2 = -DBBS_INTERNAL;CACHE;HEAPCHECK;HYDRADEV;$(VERFLAG) \
#;REL_CAND \
   -I$(BCB)\include;..\INCLUDE
LFLAGS = -L$(BCB)\lib;..\lib -ap -c -m -M -s -Sc:4000 -w
# ---------------------------------------------------------------------------
ALLOBJ = c0x32.obj
ALLLIB = $(LIBFILES) import32.lib cw32mt.lib
BTOBJ = $(ALLOBJ) $(OBJFILES_BT)
UTOBJ = $(ALLOBJ) $(OBJFILES_UT)
# ---------------------------------------------------------------------------

.autodepend

bt32.exe: $(OBJFILES_BT)
    ilink32 @&&!
    $(LFLAGS) +
    $(BTOBJ), +
    bt32.exe,, +
    $(ALLLIB)
!
btutil32.exe: $(OBJFILES_UT)
    ilink32 @&&!
    $(LFLAGS) +
    $(UTOBJ), +
    btutil32.exe,, +
    $(ALLLIB)
!
clean:
    del *.obj
    del *.il?
    del *.exe
    del *.map
    del *.tds
!
#-----------------------------------------------------------------------------
{..\src\}.cpp.obj:
    bcc32 $(CFLAG1) $(CFLAG2) $* 

{..\src\}.c.obj:
    bcc32 $(CFLAG1) $(CFLAG2) $**

{..\src\gethist\}.c.obj:
    bcc32 $(CFLAG1) $(CFLAG2) $**
#-----------------------------------------------------------------------------
