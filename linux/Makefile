#----------------------------------------------------------------------------#
# Makefile for Binkley/XE/Linux                                              #
#----------------------------------------------------------------------------#

# use alternative charset on the console (to get high-asciis right) [yes/no]

# USE_ALTCHAR  = no

# use fidoconfig if found (husky, see http://fidosoft.sar-gmbh.com)

USE_FIDOCONFIG = yes
PATH_FIDOCONFIG = -I../../fidoconfig -I../../smapi

# adjust these if neccessary

BINDIR  = /usr/local/bin/
MANDIR  = /usr/local/man/
CONFDIR = /etc/fido/

#----------------------------------------------------------------------------#

CC       = gcc
COPT     = -Wall -s
CDEFS    = -DHYDRADEV -DBBS_INTERNAL -DTRANSLATE -DPORTABLEBINFILES
INCLUDES = -I/usr/include -I/usr/local/include -I../include
LINK     = -lncurses

BTTGT    = btxe
BUTGT    = btutil
LNTGT    = binkley.lng

SCRCMPTGT = btscrcmp
SCRANSTGT = btan2scr

#----------------------------------------------------------------------------#

BUOBJ   =  \
btutil.o   \
common.o   \
gethcomm.o \
getra.o    \
getbgfax.o \
gettg.o

#removed: getmax3.o (too may system-specific stuff for now)

BTOBJ   =  \
aglcrc.o   \
b_banner.o \
b_bbs.o    \
b_faxrcv.o \
b_flofil.o \
b_frproc.o \
b_help.o   \
b_initva.o \
b_logs.o   \
b_passwo.o \
b_proto.o  \
b_rspfil.o \
b_sbinit.o \
b_script.o \
b_search.o \
b_sessio.o \
b_spawn.o  \
b_term.o   \
b_termov.o \
b_today.o  \
b_wazoo.o  \
b_whack.o  \
b_wzsend.o \
banner.o   \
binkdlg.o  \
brec.o     \
bsend.o    \
bt.o       \
btconfig.o \
cache.o    \
callback.o \
callerid.o \
callwin.o  \
chat.o     \
common.o   \
css.o      \
data.o     \
dosfuncs.o \
emsi.o     \
evtparse.o \
evtsetup.o \
file_all.o \
file_lnx.o \
fossil.o   \
freepoll.o \
ftsc.o     \
history.o  \
hydra.o    \
janus.o    \
keymap.o   \
langload.o \
m7rec.o    \
m7send.o   \
mailer.o   \
mailovly.o \
mailroot.o \
maxmcp.o   \
maxprm.o   \
mdm_proc.o \
misc.o     \
nodeproc.o \
outbound.o \
pktmsgs.o  \
protcomm.o \
recvbark.o \
recvsync.o \
sbuf.o     \
sched.o    \
sendbark.o \
sendsync.o \
squish.o   \
srif.o     \
statetbl.o \
stats.o    \
timer.o    \
version.o  \
version7.o \
vt100.o    \
xmrec.o    \
xmsend.o   \
yoohoo.o   \
zmodem.o   \
asyn_lnx.o \
vfos_lnx.o \
script2.o  \
bbs_ctra.o \
bbs_data.o \
bbs_io.o   \
times.o

#----------------------------------------------------------------------------#

ifeq ($(USE_FIDOCONFIG),yes)
BTOBJ := fcfgread.o $(BTOBJ)
CDEFS := $(CDEFS) -DUSE_FIDOCONFIG -DUNIX
INCLUDES := $(INCLUDES) $(PATH_FIDOCONFIG)
LINK := $(LINK) -lfidoconfig -lsmapilnx
endif

ifeq ($(USE_ALTCHAR),yes)
CDEFS := $(CDEFS) -DUSE_ALTCHARSET
endif

#----------------------------------------------------------------------------#

all: $(BTTGT) $(BUTGT) $(LNTGT)

%.o: ../src/%.c
	$(CC) $(COPT) $(CFLAGS) $(CDEFS) $(INCLUDES) -c ../src/$*.c

gethcomm.o: ../src/gethist/gethcomm.c
	$(CC) $(COPT) $(CFLAGS) $(INCLUDES) -c ../src/gethist/gethcomm.c

getra.o: ../src/gethist/getra.c
	$(CC) $(COPT) $(CFLAGS) $(INCLUDES) -c ../src/gethist/getra.c

getbgfax.o: ../src/gethist/getbgfax.c
	$(CC) $(COPT) $(CFLAGS) $(INCLUDES) -c ../src/gethist/getbgfax.c

gettg.o: ../src/gethist/gettg.c
	$(CC) $(COPT) $(CFLAGS) $(INCLUDES) -c ../src/gethist/gettg.c

$(BTTGT): $(BTOBJ)
	$(CC) -o $(BTTGT) $(BTOBJ) $(LINK)

$(BUTGT): $(BUOBJ)
	$(CC) -o $(BUTGT) $(BUOBJ)

$(LNTGT): $(BUTGT) ../src/language.txt
	./$(BUTGT) LNG key E ../src/language.txt $(LNTGT)

$(SCRCMPTGT): ../xprmntl/bbs/compile.c
	$(CC) $(COPT) $(CFLAGS) ../xprmntl/bbs/compile.c -D__EMX__ -o $(SCRCMPTGT)

$(SCRANSTGT): ../xprmntl/bbs/ansi2bs.c
	$(CC) $(COPT) $(CFLAGS) ../xprmntl/bbs/ansi2bs.c -D__EMX__ -o $(SCRANSTGT)

clean:
	rm -f *.o
	rm -f $(BUTGT)
	rm -f $(BTTGT)
	rm -f $(LNTGT)
	rm -f $(SCRCMPTGT)
	rm -f $(SCRANSTGT)

install: $(BUTGT) $(BTTGT)
	install $(BUTGT) $(BINDIR)
	install $(BTTGT) $(BINDIR)
	gzip btxe.1
	install -m 444 btxe.1.gz $(MANDIR)man1/
	gunzip btxe.1.gz
	ln -s -f $(MANDIR)man1/btxe.1.gz $(MANDIR)man1/btutil.1.gz

installlng:
	install -m 644 $(LNTGT) $(CONFDIR)

bbs: $(SCRCMPTGT) $(SCRANSTGT)

installbbs:
	install $(SCRCMPTGT) $(BINDIR)
	install $(SCRANSTGT) $(BINDIR)

