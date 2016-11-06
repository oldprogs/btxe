@echo off

rem %1 = speed of the computer-to-modem link rate in bps
rem %2 = caller's connect rate reported by the modem
rem %3 = the comm port in use (or in OS/2 and Win32, the port handle in use)
rem %4 = time to the next event in minutes
rem %5 = errorlevel exit in your ExtrnMail statement(default 99)
rem      1 = Fax on Line 1
rem      5 = PPP on Line 1
rem %6 = extended info in the modem connect string (/ARQ, etc.)

echo DTE:%1 Connect rate:%2 COM Handle:%3 Time:%4 EL:%5 Extended Info:%6

if %5.==1. GOTO FAX1
if %5.==5. GOTO PPP1
goto END

rem FAX ========================================================================
:FAX1

echo Fax Receive ...
cd\Bink\Bgfax
Bgfax32 /fax D:\Bink\Fax h%3 z
Start /r D:\Bink\Bgfax\Autoprn.bat
cd\Bink

goto END
rem ============================================================================

rem PPP ========================================================================
:PPP1

echo PPP ...

goto END
rem ============================================================================

:END
cd\Bink
