#####################################################################
#                                                                   #
# makefile for compiling Binkley XE for OS/2 with Watcom C/C++ 10.x #
#                                                                   #
#####################################################################

# compile for which OS (os2 or dos) (see ..\dos\watcom.mak for DOS version!)
OS       = os2

# compile debug version (yes), non-debug version (no) or optimized version (opt)
DEB      = opt

# language (english, startrek, deutsch)
LANGUAGE = english


!include ..\os2\watcom10.mif

