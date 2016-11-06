/* FindTabs - Usage: FindTabs <filespec>                               */
/* Recursively search for files matching <filespec> and show filenames */
/* of files containing TAB characters (09 char code)                   */
/* Written by Th. Waldmann                                             */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

arg fspec

tab = '09'x
verbose = 1      /* =1 -> show every line containing a TAB */

say 'Searching for TABS in' fspec
call SysFileTree fspec, files, 'FSO'
do i=1 to files.0
  file=files.i
  call SysFileSearch tab, file, lines, 'N'
  if lines.0>0 then
    do
      say file ':' lines.0
      if verbose then
        do j=1 to lines.0
          line=lines.j
          say line
        end
    end
  else
    call charout ,'.'
end
