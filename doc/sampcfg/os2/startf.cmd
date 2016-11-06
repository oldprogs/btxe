/* STARTF - START with some specific Font */

Parse Upper arg arg1 .

Parse arg '"'title'"' font runcmd

Call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
Call SysLoadFuncs

If Pos('X', translate(font))>0 Then
  Do
    Parse Upper Value font With fh 'X' fw
    fontsize=d2c(fw*256+fh)
  End
Else
  Signal Help

tag.0      = 2                      /* 2 languages are configured */
tag.1.lang = 'DEUTSCH'
tag.1.tag  = 'Schrif~tartgr”áe...'
tag.2.lang = 'ENGLISH'              /* language */
tag.2.tag  = '~Font Size...'        /* entry to change in this language */

found=0
do i=1 to tag.0
  result=SysIni('USER', 'Shield', tag.i.tag)  /* search correct language */
  if result\='ERROR:' then
    do
      found=1
      leave
    end
end

if found=1 then
  do
    say 'Using Language' tag.i.lang'.'
    savesize=SysIni('USER', 'Shield', tag.i.tag) /* save user ini font size  */
    call SysIni 'USER', 'Shield', tag.i.tag, fontsize   /* modify font size  */
    if title\="" then
      '@START "'title'" /WIN /N' runcmd                 /* run!              */
    else
      '@START           /WIN /N' runcmd                 /* run!              */
    call SysIni 'USER', 'Shield', tag.i.tag, savesize   /* restore font size */
  end
else
  do
    say 'Could not determine OS/2 language - aborting ...'
    say 'To fix that, you must select the font settings page in the system menu,'
    say 'then select and save a font.'
    call SysSleep 5
    exit 1
  end
Exit 0

Help:
  Say 'STARTF - START with selected font'
  Say 'Freeware by Thomas Waldmann, using SETWIN of Michael Thompson.'
  Say
  Say 'Usage: startf "title" font'
  Say
  Say '  title    = Window title (may be empty if program does this setting)'
  Say '  font     = Font for window (8x8, 10x6, ...)'
  Say
  call charout ,'Press any key to exit ...'
  Pull Dummy
  Exit 1
