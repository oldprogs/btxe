
#
# makefile to create "bink_pm.dll" for use with VAC 3.00
#

.SUFFIXES: .c .obj

DEST   = ..\..\..\os2

CFLAGS = /Tl1 /Ss /Sp1 /Q /Ge- /W3 /G3 /Gs+
LFLAGS = "/pmtype:pm /packc /packd /optfunc /noe /noi /map /nologo /align:16"

######################################################################
# only important are the compiler switches
#
# /Ge- make a DLL (do not make an EXE)
# /G3  make for 386 CPU
# /Gs+ supress stack probes in function prologs
# /Ss  allow '//' for comments
#
######################################################################
# probably unknown linker switches are
#
# /packc /packd: pack code and data segments
# /optfunc     : default linkage is _Optlink (i.e. _pascal)
# /noe         : no extended libraries
# /noi         : no ignore case (i.e. be case sensitive) when exporting
#
######################################################################

.all: \
   $(DEST)\bink_pm.dll \
   $(DEST)\bink_pm.sym

.c.obj:
    icc.exe /I$(INCL) $(CFLAGS) /Fo"%|dpfF.obj" /C %s

$(DEST)\bink_pm.dll: \
    bink_pm.obj \
    bink_pm.def \
    ibmvacpp.mak
    icc.exe @<<
     /B$(LFLAGS)
     /Fe$(DEST)\bink_pm.dll
     bink_pm.def
     bink_pm.obj
<<

bink_pm.obj: \
    bink_pm.c \
    ibmvacpp.mak

$(DEST)\bink_pm.sym: bink_pm.map
    mapsym -a bink_pm.map
    @copy bink_pm.sym $(DEST)
    @del bink_pm.sym
    
    
