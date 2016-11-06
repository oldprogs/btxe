#############################################################################
#                                                                           #
# Make file for Screen Classes                                              #
#                                                                           #
# Compiler: VisualAge C++ 3.0                                               #
#                                                                           #
# Usage: nmake -f IBMVACPP.MAK                                              #
#                                                                           #
#############################################################################

DEBUG      = 0

SRC_DIR    = .
INCL_DIR   = .
LIB_DIR    = .

LINKLIBS    = OS2386.LIB

# for performance analyzer: /Gh
WC_EXTRA  = /Wcnd /Weff /Wini /Word /Wrea /Wtru /Wund
C_EXTRA   = /Weff /Wrea /Wtru /Wund
CFLAGS_X  = /GM+ /GT+ /W3 /Wuse /Wpar /Wdcl /Wcnv /Wpro /Wret /Sm /Sp1 /C /Tl1 /Q

!IF $(DEBUG) == 0
CFLAGS  = /O+         $(CFLAGS_X) /I $(INCL_DIR)
!ELSE
CFLAGS  = /O- /Ti /Tm $(CFLAGS_X) /I $(INCL_DIR)
!ENDIF

# /Ol    invoke intermediate code linker
# /Oinnn enable inline function code up to nnn intermediate code instr
# /W3    all errors and warnings
# /Sm    Ignore migration keywords (!?)
# /Sp1   pack aggregate members on alignment 1
# /O+    optimize generated code (do NOT use for debugging!)
# /C     compile only
# /G5    Pentium optimization
# /GT+   Erzeuge tiled memory (die COM-Buffer..)
# /GM+   Multithread Libraries (HydraWrite!)
# /Gs+   suppress stack probes in function prologues
# /Tln   keep compiler n mins in memory
# /Ti    generate debugging information
# /Tx    generate full exception register dump
# /Tm    debug memory management
# /Q     suppress the logo
# /DOS_2 #define OS_2
# /I     set include dir(s)

# for performance analyzer: /NOE  cppopa3.obj
!IF $(DEBUG) == 0
LFLAGS =         /Q /B"                /packc /packd /map /noi /nol /pmtype:vio /align:16 /exepack"
!ELSE
LFLAGS = /Ti /Tm /Q /B"/line /c /debug /packc /packd /map /noi /nol /pmtype:vio /align:16 /exepack" 
!ENDIF
# /Q       no logo
# /B"..."  pass options to linker
# /packc   pack code segments together
# /packs   pack data segments together

CC       = ICC
LINK     = ICC

scrnobjs= cellstr.obj colour.obj scrnelem.obj scrntest.obj

all: \
    screen.exe

clean:
    @del *.obj *.pch *.map *.exe *.dll *.lng *.sym
    
screen.exe:  $(scrnobjs)
    $(LINK) @<<
    $(LFLAGS)
    /FeSCREEN.EXE
    $(scrnobjs)
    $(LINKLIBS)
<<
#   cppopa3.obj
#   =========== only needed for VAC++ profiler

$(scrnobjs): $(SRC_DIR)\$*.cpp
    @echo compiling $*.cpp
    $(CC) $(CFLAGS) $(SRC_DIR)\$*.cpp


