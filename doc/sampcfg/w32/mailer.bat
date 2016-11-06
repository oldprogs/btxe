@echo off
if NOT '%1' == '' goto Ok
echo "Usage:         mailer [tasknumber] <other options>"
echo For example: - 'mailer 1' starts Bink tast 1
echo              - 'mailer 1 debug' starts Bink task 1 in debug modus
goto end

:Ok

d:
cd\bink
@rem ************************************************
@rem * Simple Binkley batch file for Mail operation *
@rem * Written by Henk-Jan Kreuger                  *
@rem ************************************************
  
:Loop

  bt32.exe task=%1 %2 %3 %4 %5
  If ErrorLevel = 254 Goto end
  If ErrorLevel =  30 Goto Inmail
  If ErrorLevel =   1 Goto end
  Goto end

:Inmail
  @rem ErrorLevel 30 - Process incoming mail (via Binkley.evt exit)
  @rem              - unpack/toss mail then compress message areas
  @rem but you could better use a seperate batch file, and spawn to it
  cd fmail
  fmailw32 toss import
  cd ..\netmgr
  netmgr
  cd ..
  goto loop

:end

