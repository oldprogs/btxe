######################################################
# Makefile for Watcom 10.0 to create bink_pm.dll
######################################################

DST_DIR = ..\..\..\os2
CCOPT   = -bd -s -D__MIG_LIB__
LLOPT   =
CC      = WCC386
LL      = wlink

all:    $(DST_DIR)\bink_pm.dll

$(DST_DIR)\bink_pm.dll: bink_pm.c bink_pm.wat bink_pm.lnk
        $(CC) $(CCOPT) bink_pm.c
        $(LL) $(LLOPT) @bink_pm.lnk


