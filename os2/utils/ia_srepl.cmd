/* IA_SREPL.Cmd - Inter-Active Search & Replace tool
 *
 * Usage:
 * Input:
 * Output:
 *
 * Freeware by Thomas Waldmann, 2:2474/400
 */

version = '1.0'

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

call SysCls

say 'IA_SRepl.Cmd v'version '- Inter-Active Search and Replace Tool.'
say 'Freeware written by Thomas Waldmann, 2:2474/400.'
say

call charout , 'Enter File specification for recursive search: '
parse pull filespec
call charout , 'Enter Search String: '
parse pull from
call charout , 'Enter Replacement String: '
parse pull to

call SysFileTree filespec, 'file', 'FSO'

do i=1 to file.0
  infile = file.i||".tmp"
  outfile = file.i
  '@del' infile '>NUL 2>NUL'
  '@move' file.i substr(infile,3) '>NUL 2>NUL'
  call stream infile, 'c', 'open read'
  call stream outfile, 'c', 'open write'
  lineno = 0
  do while lines(infile)
    lineno = lineno + 1
    line=linein(infile)
    lastpos = 0
    p = pos(from, line)
    do while p>lastpos
      say outfile ': #'lineno 'change this line (Y[es], N[o], C[omment]) ?'
      say copies(' ',p-1)||copies('v',length(from))
      say line
      part1= substr(line,1,p-1)
      part2= substr(line,p+length(from))
      say part1||to||part2
      say copies(' ',p-1)||copies('^',length(to))
      pull answer
      if answer='Y' then
        do
          line=part1||to||part2
          lastpos = p + length(to)
        end
      else if answer='C' then
        do
          call charout ,'Enter Comment: '
          parse pull comment
          line=line '/*' comment '*/'
          lastpos = p
        end
      else
        lastpos = p
      p = pos(from, line)
    end
    call lineout outfile, line
  end  
  call stream infile, 'c', 'close'
  call stream outfile, 'c', 'close'
  '@del' infile '>NUL 2>NUL'
end

/* EOF */
