@echo off
set EXE=bt32.exe
if exist bt33.exe set EXE=bt33.exe
h:
cd \bt
del flag\btexit*.0%1 >NUL 2>NUL
call startf "" 10x6 %EXE% TASK=%1
