@echo off
rem %1 = speed of the computer-to-modem link rate in bps
rem %2 = caller's connect rate reported by the modem
rem %3 = the comm port in use (or in OS/2 and Win32, the port handle in use)
rem %4 = time to the next event in minutes
rem %5 = errorlevel exit in your ExtrnMail statement(default 99) 
rem %6 = extended info in the modem connect string (/ARQ, etc.)

if %5.==1. goto fax
goto end

:fax
cd fax
frec -task%TASK% -p%3 -sWAIT_OK
cd..

:end
