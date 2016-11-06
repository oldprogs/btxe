@echo off
if NOT '%1' == '' goto Ok
echo "Usage:         mailerex [tasknumber] <other options>"
echo For example: - 'mailerex 1' starts Bink tast 1
echo              - 'mailerex 1 debug' starts Bink task 1 in debug modus
goto end

:Ok

d:
cd\bink
@rem ***********************************************
@rem * Binkley batch file for Mail operation       *
@rem * Yukon Mail System = 1:3409/1                *
@rem * Inspired by Ken Wilson - SysOp Ned's Opus   *
@rem * Written by Steven Horn - February 18, 1995  *
@rem * Modified many times since then and last re- *
@rem * written August 18,1998                      *
@rem * Some modifications done by Henk-Jan Kreuger *
@rem ***********************************************
  
:Loop

  bt32 task=%1
  If ErrorLevel = 254 Goto end
  If ErrorLevel =  70 Goto DoDiff
  If ErrorLevel =  50 Goto Fido_Ftp
  If ErrorLevel =  30 Goto Inmail
  If ErrorLevel =  20 Goto Inmail
  If ErrorLevel =  17 Goto FTPcall
  If ErrorLevel =  16 Goto Mailcall  
  If ErrorLevel =  14 Goto Sysfix
  If ErrorLevel =  13 Goto SendNews
  If ErrorLevel =  10 Goto DoPoll
  If ErrorLevel =   1 Goto end

:Inmail
  @rem ErrorLevel 30 - Process incoming mail (via Binkley.evt exit)
  @rem              - unpack/toss mail then compress message areas
  d:
  cd\fidomail\msg\inbound 
  c:\utils\gus d:\fidomail\msg\inbound /m
  c:\utils\pktsort -cc:\utils\pktsort.cfg
  cd\bink
  sq386 in out squash -fd:\fidomail\msg\areas.lst link
  wimm -fd:\fidomail\msg\areas.lst
  del d:\fidomail\msg\areas.lst
  Netmgr
  If exist d:\fidomail\msg\inbound\nodediff.a?? goto Process
  goto loop

:Process
  If exist d:\fidomail\msg\inbound\nodediff.* goto DoDiff
  goto loop

:DoDiff
  arce d:\fidomail\msg\inbound\nodediff.* d:\bink\nodelist
  if exist d:\bink\nodelist\nodediff.0?? goto Continue
  if exist d:\bink\nodelist\nodediff.1?? goto Continue
  if exist d:\bink\nodelist\nodediff.2?? goto Continue
  if exist d:\bink\nodelist\nodediff.3?? goto Continue
  goto Error

:Continue
  del d:\fidomail\reqfiles\nodediff.a*
  cd\bink\nodelist
  pkzip -a d:\bin\nodediff nodediff.*
  if exist d:\bin\nodediff.zip del d:\fidomail\reqfiles\nodediff.zip
  pkunzip -n d:\fidomail\reqfiles\nodelist.zip d:\bink\nodelist
  if not exist d:\fidomail\reqfiles\nodelist.* goto Error
  move /y d:\fidomail\reqfiles\nodelist.zip d:\temp
  xlaxdiff /c
  if errorlevel 1 goto Error
  qnode
  if errorlevel 1 goto Error
  pkzip -a d:\fidomail\reqfiles\nodelist nodelist.*
  del nodelist.*
  copy d:\fidomail\msg\inbound\nodediff.a* d:\fidomail\reqfiles
  move /y d:\fidomail\msg\inbound\nodediff.a* d:\fidomail\hold
  del d:\fidomail\msg\inbound\*.tic
  cd\bink
  if exist d:\fidomail\reqfiles\nodediff.a* xr send d:\fidomail\reqfiles\nodediff.a* 3409/3 /t diff.txt /k
  if exist d:\fidomail\reqfiles\nodelist.zip del d:\temp\nodelist.zip
  move /y d:\bin\nodediff.zip d:\fidomail\reqfiles
  if exist d:\bink\nodelist\net3409.* move /y d:\bink\nodelist\net3409.* d:\bink\nodestub
  copy d:\bink\nodelist\snet3409 d:\bink\nodelist\net3409
  goto loop

:Error
  echo Error in nodediff processing... process aborted! >> SQUISH.LOG
  ren d:\fidomail\msg\inbound\nodediff.* nodedif!.*
  goto loop

:FTPcall
  @rem Error Level 17 - Send out FTP mail
  cd\bink
  sq386 out squash -fd:\fidomail\msg\areas.lst
  del d:\fidomail\msg\areas.lst
  if exist d:\fidomail\msg\outbound\008a0092.?lo goto Fido_Ftp
  goto loop

:Mailcall
  @rem Error Level 16 - Poll for echomail
  cd\bink
  sq386 out squash -fd:\fidomail\msg\areas.lst 
  del d:\fidomail\msg\areas.lst
  if exist d:\fidomail\msg\outbound\008a0092.?lo goto Fido_Ftp
  goto loop

:Sysfix
  @rem Error Level 14 - Maintain message bases
  msgnum -r -q d:\fidomail\msg\net
  sqpack d:\fidomail\msg\squish\*.sqd >> d:\logs\squish.log
  sq386 squash
  call d:\bat\makefido.bat
  goto Fido_Ftp
  
:SendNews
  @rem Error Level 13 - Process Fidonews
  If exist d:\fidomail\msg\inbound\fnewsf*.zip move /y d:\fidomail\temp\fnewsf*.zip d:\fidomail\fidonews
  If exist d:\fidomail\msg\inbound\fnewsf*.zip move /y d:\fidomail\msg\inbound\fnewsf*.zip d:\fidomail\temp
  If exist d:\fidomail\temp\fnewsf*.zip xr send d:\fidomail\temp\fnewsf*.zip 3409/3 /t fnews.txt /k
  Copy d:\fidomail\temp\fnewsf*.zip d:\fidomail\reqfiles
  Move /y d:\fidomail\hold\nodediff.a* d:\fidomail\reqfiles
  del d:\fidomail\msg\inbound\*.tic
  goto loop 

:DoPoll
  @rem Error Level 10 - Process Outbound mail
  cd\bink
  sq386 out -fd:\fidomail\msg\areas.lst squash 
  del d:\fidomail\msg\areas.lst
  goto Fido_Ftp
  
:Fido_Ftp
  @rem Error Level 50 - Process Fidonet Ftp mail
  call n3409.bat
  @rem goto loop
  goto inmail

:end

