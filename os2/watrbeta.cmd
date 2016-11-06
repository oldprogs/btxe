@echo off
setlocal
set btxever=XH5
md temp         1>nul 2>nul
md temp\async   1>nul 2>nul
md temp\maxcomm 1>nul 2>nul
copy ..\src\language.txt temp 1>nul 2>nul
copy ..\doc\bug_rprt.doc temp 1>nul 2>nul
copy ..\doc\join.us      temp 1>nul 2>nul
copy ..\doc\team.lst     temp 1>nul 2>nul
copy ..\doc\xe_user.doc  temp 1>nul 2>nul
copy ..\doc\xe_hist.doc  temp 1>nul 2>nul
copy ..\doc\tz_watc.doc  temp 1>nul 2>nul
echo ** BinkleyTerm 2.60  eXtended Edition **>temp\file_id.diz
echo *** OS/2 binary distribution archive ***>>temp\file_id.diz
echo This  archive contains the OS/2 binaries>>temp\file_id.diz
echo of Beta-%btxever%.>>temp\file_id.diz
echo ****************************************>>temp\file_id.diz
wmake /h /f watcom10.mak clean
wmake /h /f watcom10.mak bt OS=os2 BIT=32 CPU=386 DEB=opt MCD=yes %1 %2 %3 %4 %5
copy e_32_386\bt32_386.* temp\maxcomm 1>nul 2>nul
wmake /h /f watcom10.mak clean OS=os2 BIT=32 CPU=386 DEB=opt MCD=yes %1 %2 %3 %4 %5
wmake /h /f watcom10.mak all OS=os2 BIT=32 CPU=386 DEB=opt MCD=no %1 %2 %3 %4 %5
copy e_32_386\bt32_386.* temp\async 1>nul 2>nul
copy e_32_386\bink_pm.*  temp 1>nul 2>nul
copy e_32_386\binkpipe.* temp 1>nul 2>nul
copy e_32_386\btutil32.* temp 1>nul 2>nul
copy e_32_386\toss.*     temp 1>nul 2>nul
wmake /h /f watcom10.mak clean OS=os2 BIT=32 CPU=386 DEB=opt MCD=no %1 %2 %3 %4 %5
cd temp
..\..\..\rar.exe m -std -m5 -r \bos2w%btxever%.rar *
cd ..
rd temp
echo bos2w%btxever%.rar ** BinkleyTerm 2.60  eXtended Edition **>>\files.bbs
echo                                *** OS/2 binary distribution archive ***>>\files.bbs
echo                                This  archive contains the OS/2 binaries>>\files.bbs
echo                                of Beta-%btxever%.>>\files.bbs
echo                                ****************************************>>\files.bbs
