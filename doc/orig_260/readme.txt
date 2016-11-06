Using the contents of this .zip file you can support BinkleyTerm 
builds for DOS, OS/2 (32-bit) and Win32 (NT or Windows 95).

The standard compiler for DOS is Microsoft Visual C++ 2.x.
The standard compiler for Win32 is Microsoft Visual C++ 4.0.
The standard compiler for OS/2 is IBM C/Set2 C++.

Create the tree like this:

x (directory name, suggest \binkley)
x\src
x\include
x\lib
x\ (directory name, suggest dos)
x\ (directory name, suggest win32)
x\ (directory name, suggest os2)

extract src.zip into src
extract include.zip into include
extract lib.zip into lib
put binkley.mak into dos
put bt.def into dos
put makefile.* into os2
put w32bink.mak into win32

to build:

go into appropriate directory and invoke nmake using the makefile you want.
for dos or win32, you use the .mak file. for os2, you need to use the .snp
file if you want snoop support and the .nsp if you don't.

note:

the above assumes that the PATH, INCLUDE and LIB directories are already
set up correctly for your compiler.

the x\lib directory is presently only used by the OS/2 version, which links
to the MAXCOMM.DLL and SNSERVER.DLL (depending on "flavor").


