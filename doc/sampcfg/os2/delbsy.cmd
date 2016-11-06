/* Cleanup outbound, flags directory and misc. other stuff */
/* 961129 by Thomas Waldmann                               */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

outbound = 'e:\out'
flags    = 'h:\bt\flags'

say "Cleaning outbound ..."

call SysFileTree outbound||'\*.BSY', 'bsy', 'FSO'
do i=1 to bsy.0
  say "Deleting" bsy.i "..."
  call SysFileDelete bsy.i
end

say "Cleaning flags directory ..."

call SysFileTree flags||'\*.BSY', 'bsy', 'FSO'
do i=1 to bsy.0
  say "Deleting" bsy.i "..."
  call SysFileDelete bsy.i
end

say "Cleaning miscellaneous ..."
call SysFileDelete "h:\bt\fmail\toss.sig"
call SysFileDelete "e:\semaphor\wrkrexit.sig"
