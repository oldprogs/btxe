@echo off
erase *.obj *.exe 2>nul

if %1 = opt goto opt

wpp386 cellstr.cpp  /d2 /bt=OS2 /wx
wpp386 colour.cpp   /d2 /bt=OS2 /wx
wpp386 scrnelem.cpp /d2 /bt=OS2 /wx
wpp386 scrntest.cpp /d2 /bt=OS2 /wx
wlink  system os2v2 pmcompatible option stack=32768 debug all option cache file {cellstr.obj colour.obj scrnelem.obj scrntest.obj} name scrntest.exe

goto ende
:opt

wpp386 cellstr.cpp  /ox /bt=OS2 /wx
wpp386 colour.cpp   /ox /bt=OS2 /wx
wpp386 scrnelem.cpp /ox /bt=OS2 /wx
wpp386 scrntest.cpp /ox /bt=OS2 /wx
wlink  system os2v2 pmcompatible option stack=32768 option cache file {cellstr.obj colour.obj scrnelem.obj scrntest.obj} name scrntest.exe

rem wcl scrntest.cpp /d2 /bt=DOS /wx /l=DOS
rem wcl scrntest.cpp /ox /bt=DOS /wx /l=DOS
rem wcl386 scrntest.cpp /d2 /bt=OS2 /wx /l=OS2V2
rem wcl386 scrntest.cpp /ox /bt=OS2 /wx /l=OS2V2

:ende

