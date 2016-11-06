#===================================================================
# Makefile for watcom 10.0 to create binkpipe.exe
#===================================================================
.SILENT
.EXTENSIONS  :
.EXTENSIONS  : .exe .obj .c .res .rc
#===================================================================

INC_DIR = ..\..\..\include
OS2_DIR = ..\..\..\os2

CFLAGS   = -bt=OS2V2 -mf -D_Optlink= -i=$(INC_DIR) -3r -w3 -s -bm
CC       = WCC386 $(CFLAGS)

LFLAGS   =
LINK     = WLINK  $(LFLAGS)

#===================================================================
!  define OLDINC $(%INCLUDE)

.BEFORE
        set include=$(OLDINC);$(INC_DIR)

.AFTER
        set include=$(OLDINC)

#===================================================================
.c.obj:
        $(CC) $*.c

.rc.res:
        rc -r $*.rc -i $(INC_DIR)

#-------------------------------------------------------------------
all: $(OS2_DIR)\binkpipe.exe binkpipe.res .symbolic
      @%null

#===================================================================
binkpipe.res: binkpipe.rc bt32.ico $(INC_DIR)\binkpipe.h
    rc -r binkpipe.rc binkpipe.res -i $(INC_DIR)

binkpipe.obj: binkpipe.c $(INC_DIR)\binkpipe.h binkpipe.wat

#-------------------------------------------------------------------
$(OS2_DIR)\binkpipe.exe: binkpipe.obj binkpipe.lnk binkpipe.wat binkpipe.res
    $(LINK) @binkpipe.lnk
    rc -p -x binkpipe.res $(OS2_DIR)\binkpipe.exe

