@echo off
if %1.==. goto help

e:\utils\gnu\diff\patch.exe -p2 <%1
goto end

:help
echo Usage: dopatch diffname
goto end

:end

