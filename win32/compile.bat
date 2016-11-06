@echo off
make -DTARVER=NT bt32.exe
d:\bink\btutil32 lng patch bt32.exe d:\bink\binkley.lng
xcopy bt32.exe d:\bink /Y
