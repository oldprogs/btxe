@echo off
h:
cd \Max
set PATH=%MAX%;%MAX%\offtag;%PATH%
echo max -s%1 -b%2 -p%3 -n%TASK% -t%4 -e0 -lh:\bt\log\%TASK%\binkadd.log >>h:\bt\log\%TASK%\spawn.log
max      -s%1 -b%2 -p%3 -n%TASK% -t%4 -e0 -lh:\bt\log\%TASK%\binkadd.log
if errorlevel 3 goto lastuser
goto end
:lastuser
btutil gethist max3 /h:%BT%\task\callhist.all /m:%MAX%\max.prm /t:%TASK%
:end
