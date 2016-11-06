/*
 * little REXX script to create DIFFs for BT-XE development
 * written by Michael Reinsch (2:2474/141, mr@cc86.org)
 *
 * requires RXROBI.DLL
 *
 * please contact the author directly if you have questions / comments or
 * if you have found a bug and do NOT post them into the developer echo
 *
 * known 'misfeatures' (probably won't be fixed):
 *  - only two chars allowed for developer name (due to 8.3)
 *
 * history: v1.0 970806 MR first version of makediff
 *          v1.1 970811 MR added unwanted chars check of created diff
 *          v1.2 970811 MR minor bugfixes
 *          v1.3 971016 MR some minor enhancements
 *          v1.4 971202 MR added parameter 'ignore' (see comment above search)
 *          v1.5 971224 MR added name of diff in doc template
 *          v1.6 971228 MR introduced little bug in v1.5, fixed.
 *          v1.7 980611 MR changes.log will be added to doku template
 *
 */

/* config - !modify this part! */

devname.long  = 'Michael Reinsch'              /* your complete name         */
devname.shrt  = 'MR'                           /* only two chars here!       */
gnudiff       = 'e:\utils\diff.exe'            /* where is the diff utility? */
rar201        = 'e:\utils\rar.exe'             /* where is rar 2.01?         */
tic.inbound   = 'e:\outbound\out'              /* no leading backslash!      */
tic.mynode    = '2:2474/141'                   /* your fido address          */
tic.seenby    = ''                             /* if you want to add seenbys */
tic.password  = '********'                     /* your password              */

/* end of config */

diffgenver    = '1.7'

/*
 * do NOT allow 'final' (i.e. do NOT pack and hatch diff) if one of those
 * chars were found - except if user explicitly allows it via 'final ignore'
 */
 
search.0      = 2
search.1.char = '09'x
search.1.desc = 'TAB'
search.2.char = '1a'x
search.2.desc = 'EOF'

arg arg1 arg2 arg3

call RxFuncAdd 'RxCRC32', 'RXROBI', 'RXCRC32'
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

say
say '   BT-XE Diff Generator v'diffgenver '  written by Michael Reinsch'
say

if arg1 = '' | arg1 = '?' | arg1 = '/?' | arg1 ='-?' then do
  say ' error: wrong or no parameter!'
  say
  call Help
  exit
end

diff.path = CheckNames(arg1, arg2)
call ComposeNames diff.path

say ' creating' diff.nme'.dif based on' diff.org' updating to' diff.new
'@'gnudiff '-c -p -r -N -X' '\'diff.org'\makediff.exc' '\'diff.org '\'diff.new '>'diff.nme'.dif'

if rc = 1 then do
  rc1 = CheckDiff(diff.nme'.dif') & \(arg3 = 'IGNORE')
  rc2 = CreateDoc(diff.nme, diff.beta, diff.path)
end
else if rc = 0 then do
  say ' no differences, aborting'
  exit
end
else do
  say ' error: gnudiff returned error code' rc', aborting'
  exit
end

if arg2 = 'FINAL' & \(rc1 | rc2)  then do
  say ' packing' diff.nme'.dif and' diff.nme'.doc:'
  say
  '@'rar201 'm -std -m5' diff.nme'.rar' diff.nme'.dif' diff.nme'.doc'
  call Hatch diff.nme'.rar', 'BINKLEY.DEV', 'Diff Beta-'betaname.org 'to Beta-'betaname.new 'created by' devname.long',' Date('N')
  say
end

say ' done.'

return



/* functs */

Help: procedure
  say ' makediff <btxedir> [final [ignore]]'
  say '           |^^^^^^   |^^^^  |^^^^^'
  say '           |         |      \-- ignore if unwanted chars were found'
  say '           |         \--------- to also pack and hatch diff and doc'
  say '           \------------------- _NO_ leading or trailing backslashes!'
  say
  say ' <btxedir> contains the modified source tree of BT-XE; the directory'
  say ' containing the reference version will be constructed automaticly.'
  say ' Please note that both directories should be in root directory!'
  say
  say ' naming convention: XGx_ORG - original beta               e.g. XGR_ORG'
  say '                    XGx_PRE - next not yet released beta  e.g. XGS_PRE'
  say '                    XGx_yyz - beta modified by yy, no. z  e.g. XGR_MR1'
return


CheckNames: procedure
  parse arg arg1, arg2
  if Length(arg1) \= 7 | Left(arg1,1) \= 'X' | Substr(arg1,4,1) \= '_' then do
    say ' error: <btxedir> must match naming convention!'
    say
    call Help
    exit
  end
  if Right(arg1,4) = '_PRE' then
    if arg2 = 'FINAL' then do
      new = Left(arg1,3)'_ORG'
      '@ren' arg1 new '>nul'
      if rc = 0 then
        arg1 = new
    end
return arg1


ComposeNames: procedure expose diff. betaname. devname.
  parse arg inname
  diff.new = inname
  if Right(diff.new,4) = '_PRE'  |  Right(diff.new,4) = '_ORG' then do
    prev_ver     = CalcPrevVer(Substr(diff.new,3,1))
    if prev_ver = 0 then
      diff.org   = Left(diff.new,1) || CalcPrevVer(Substr(diff.new,2,1))'Z_ORG'
    else
      diff.org   = Left(diff.new,2) || CalcPrevVer(Substr(diff.new,3,1))'_ORG'
    betaname.org = Left(diff.org,3)
    betaname.new = Left(diff.new,3)
    diff.nme     = Left(diff.org,3)'_'Left(diff.new,3)
    diff.beta    = 1
  end
  else do
    prev_ver = CalcPrevVer(Right(diff.new,1))
    if prev_ver = 0 then do
      diff.org     = Left(diff.new,4)'ORG'
      betaname.org = Left(diff.org,3)
      betaname.new = Left(diff.new,3)'+'devname.shrt || Right(diff.new,1)
    end
    else do
      diff.org     = Left(diff.new,6) || prev_ver
      betaname.org = Left(diff.org,3)'+'devname.shrt || prev_ver
      betaname.new = Left(diff.new,3)'+'devname.shrt || Right(diff.new,1)
    end
    diff.nme  = diff.new
    diff.beta = 0
  end
return


CheckDiff: procedure expose diff. search.
  parse arg diffname
  say ' searching' diffname 'for unwanted chars'
  didfind = 0
  do i = 1 to search.0
    numfinds = 0
    call SysFileSearch search.i.char, diffname, 'lines', ''
    if lines.0 > 0 then do
      do j = 1 to lines.0
        if (Left(lines.j,4) \= '*** ') & (Left(lines.j,4) \= '--- ') then
          numfinds = numfinds + 1
      end
      if numfinds > 0 then do
        say
        say ' Warning! Found' numfinds 'unwanted' search.i.desc 'char(s) in diff!'
        say
        didfind = 1
      end
    end
  end
return didfind


CreateDoc: procedure expose devname. betaname.
  parse arg basename, type, dir
  docname = basename'.doc'
  difname = basename'.dif'
  chglist = dir'\changes.log'
  created = 0
  if \FileExist(docname) then do
    created = 1
    say ' no doc found, creating template' docname
    call LineOut docname, difname':'
    if type = 1 then
      call LineOut docname, ' - created by    :' devname.long '(merging diffs)'
    else
      call LineOut docname, ' - created by    :' devname.long
    call LineOut docname,   ' - based on      : Beta-'betaname.org
    call LineOut docname,   ' - updates to    : Beta-'betaname.new
    if type = 1 then
      call LineOut docname, ' - included diffs: see below'
    else
      call LineOut docname, ' - included diffs: [none, else list all included diffs here]'
    call LineOut docname, ''
    call LineOut docname, 'List of Changes:'
    if FileExist(chglist) then do
      created = 0
      call LineOut docname, ''
      do while Lines(chglist)
        cp = LineIn(chglist)
        call LineOut docname, cp
      end
      call LineOut chglist
    end
    else do
      if type = 1 then
        call LineOut docname, ' [list changes here or add' chglist '(will be included here)]'
      else
        call LineOut docname, ' [list your changes here]'
      call LineOut docname, ''
      call LineOut docname, ''
    end
    call LineOut docname, '[EOF]'
    call LineOut docname
  end
return created


Hatch: procedure expose tic. diffgenver
  parse arg fname, farea, desc
  say
  say ' hatching file' fname' into area' farea
  say ' desc:' desc
  say ' hatch (y/N) ?'
  if Yes() then do
    call MakeTic fname, farea, desc
    '@copy' fname tic.inbound '>nul'
    if rc = 0 then
      call SysFileDelete fname
  end
return


MakeTic: procedure expose tic. diffgenver
  parse arg fname, farea, desc
  ticfile=tic.inbound||'\mtmr'||D2X(Random(65535),4)||'.tic'
  call LineOut ticfile, 'Area' farea
  call LineOut ticfile, 'Origin' tic.mynode
  call LineOut ticfile, 'From' tic.mynode
  call LineOut ticfile, 'File' FileSpec('name',fname)
  call LineOut ticfile, 'Desc' desc
  call LineOut ticfile, 'CRC' RxCRC32(fname)
  call LineOut ticfile, 'Size' Stream(fname,'c','query size')
  call LineOut ticfile, 'Created by BT-XE Diff Generator v'diffgenver
  call LineOut ticfile, 'Seenby' tic.mynode
  if tic.seenby \= '' then
    call lineOut ticfile, 'Seenby' tic.seenby
  call LineOut ticfile, 'Pw' tic.password
  call LineOut ticfile
return


CalcPrevVer: procedure
  arg cur_ver
  if      cur_ver = 0 then                prev_ver = 0
  else if cur_ver > 0 & cur_ver < 10 then prev_ver = cur_ver - 1
  else if cur_ver = 'A' then              prev_ver = 9
  else                                    prev_ver = D2C(C2D(cur_ver)-1)
return prev_ver


FileExist: procedure
  parse arg fname
  if Stream(fname, 'c', 'query exists') \= '' then
    rc = 1
  else
    rc = 0
return rc


Yes:
  ret = 0
  parse upper value(SysGetKey('NOECHO')) with key
  if key = 'Y' then
    ret = 1
return ret

