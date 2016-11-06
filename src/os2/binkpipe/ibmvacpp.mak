#
# makefile for binkpipe using VAC++, 3.0
#

# to enable source level debugging, set DEBUG to /de

DEBUG   =
DST_DIR = ..\..\..\os2
INC_DIR = ..\..\..\include
CFLAGS  = /O+ /Ss /Q /Tx /Ti /W3 /Wtru /Gm /Gs+ /Sp1 /Ft- /I $(INC_DIR)
LFLAGS  = "$(DEBUG) /pmtype:pm /packd /packc /exepack /nologo /align:16"

.SUFFIXES: .LIB .def .dll .rc .res 

.all: \
    $(DST_DIR)\binkpipe.exe

.rc.res:
    rc.exe -i $(INC_DIR) -r %s %|dpfF.RES

.c.obj:
    icc.exe /Tl1 $(CFLAGS) /Fo"%|dpfF.obj" /C %s

$(DST_DIR)\binkpipe.exe: \
     binkpipe.obj \
     binkpipe.res \
     binkpipe.def \
     ibmvacpp.mak
     icc.exe @<<
     /B$(LFLAGS)
     /Fe$(DST_DIR)\binkpipe.exe
     binkpipe.def
     binkpipe.obj
<<
    rc.exe binkpipe.res $(DST_DIR)\binkpipe.exe

binkpipe.res: \
    binkpipe.rc \
    bt32.ico \
    binkpipe.dlg \
    $(INC_DIR)\binkpipe.h \
    ibmvacpp.mak

binkpipe.obj: \
    binkpipe.c \
    $(INC_DIR)\binkpipe.h \
    ibmvacpp.mak

bt32.ico :
    copy ..\..\..\os2\bt32.ico
