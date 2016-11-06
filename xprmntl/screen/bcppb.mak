# ---------------------------------------------------------------------------
# Makefile for Boland C++ Builder for Windows
# ---------------------------------------------------------------------------
VERSION = BCB.01
# ---------------------------------------------------------------------------
!ifndef BCB
BCB = $(MAKEDIR)\..
!endif
# ---------------------------------------------------------------------------
PROJECT = Screen.exe
OBJFILES = \
           cellstr.obj\
           colour.obj\
           scrnelem.obj\
           scrntest.obj
LIBFILES = import32.lib
# ---------------------------------------------------------------------------
CFLAG1 = -3 -a- -c -w -w-aus -w-csu -w-sig
CFLAG2 = -I$(BCB)\include;$(BCB)\include\vcl -H=$(BCB)\lib\vcld.csm 
LFLAGS = -L$(BCB)\lib\obj;$(BCB)\lib -ap -c -Tpe -x -w
LINKER = ilink32
# ---------------------------------------------------------------------------
ALLOBJ = c0x32.obj $(OBJFILES)
ALLLIB = $(LIBFILES) vcl.lib cw32mt.lib 
# ---------------------------------------------------------------------------
.autodepend

$(PROJECT): $(OBJFILES)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB)
!

.cpp.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $* 

.c.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $**

clean:
    @del *.obj
    @del *.il?
    @del *.tds
    @del *.exe
#-----------------------------------------------------------------------------
