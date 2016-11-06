/* Send all files to <ToAddr> with <Flavour> that match to <FileSpec> and
 * (optionally) delete/kill or truncate the files after sending
 *
 * SendTo <ToAddr> <Flavour> <Filespec> [KILL]
 *
 * e.g.:
 * SendTo 2:2474/400 C C:\TW\*
 * SendTo 2:2474/400 H C:\TW\* KILL
 * SendTo 2:2474/400 H C:\TW\* TRUNC
 *
 * written by Thomas Waldmann 2:2474/400
 */

mode = "FSO" /* "FSO" : search files recursively below start directory */
             /* "FO"  : search files only in given directory           */
             
arg ToAddr Flavour Filespec KillOpt

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

if KillOpt="KILL" then
  FlowChar="^"
else
if KillOpt="TRUNC" then
  FlowChar="#"
else
  FlowChar=""
  
call SysFileTree FileSpec, "file", mode
do i=1 to file.0
  say "Sending" file.i "to" ToAddr", Flavour" Flavour KillOpt "..."
  call FlowFile ToAddr Flavour FlowChar||file.i
end

