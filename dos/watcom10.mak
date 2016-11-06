####################################################################
#                                                                  #
# makefile for compiling Binkley XE for DOS with Watcom C/C++ 10.x #
#                                                                  #
####################################################################

# compile for which OS (os2 or dos) (see ..\os2\watcom.mak for OS2 version!)
OS       = dos

# compile debug version (yes), non-debug version (no) or optimized version (opt)
DEB      = opt

# compile overlayed version (yes) or non-overlayed version (no)
OVL      = yes

# language (english, startrek, deutsch)
LANGUAGE = english


!include ..\os2\watcom10.mif

