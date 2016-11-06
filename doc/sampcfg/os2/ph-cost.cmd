/* This REXX-script's purpose is to evaluate Binkley XE's COSTLOG-file.

   The very special about this script:
   It can handle any 'Shortcostlog'-string not containing the 'newline' ($<) statement.

   This version lists every connection of a certain period and calculates
   the resulting phone-costs.

   References of environment-variables are evaluated the way binkley does while interpreting
   it's config file. So if you have defined
    COSTLOG=D:\FIDO\LOGS\COST%Task%.log
   make sure that the environment-variable 'task' ist set up before running this script.

   You may set Task to '?' ord '*' to list up all rour tasks' costs.

   */

version="1.21"

currency='DM'           /* Your Currency-Unit (Dollars) */
currency_100='Pf'       /* Unit of 1/100      (cent)    */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs
rc=setlocal()

/* Parse Program parameters */
parse upper arg binkleycfg rest
if (binkleycfg='') then do
 call syntaxhelp
 call Halt 1
END

netmail=0; fromdate=''; todate='';

do while \(rest='')
   parse var rest aword rest
   select
       when left(aword,2)='-F' then fromdate=delstr(aword,1,2)
       when left(aword,2)='-T' then Todate  =delstr(aword,1,2)
       when left(aword,2)='-N' then netmail=1
       when pos('=',aword)>1 then do
              i=pos('=',aword)
              call value left(aword,i-1), delstr(aword,1,i) , 'OS2ENVIRONMENT'
            end;
       otherwise
            call syntaxhelp
            call halt 1
   end
end


/* read the shortcostlog-template from  Binkley.cfg and convert it for parsing*/
Call Gettemplate


Call Checktemplate

/* getting the costlog-path out of Binkley.cfg */
Call Getcostfiles

/* convert the dates (supplied by parameters) into 'yyyymmdd'-format */

if fromdate='' then fromdate='1.1.1900' /* since 1900 if not given */
parse value translate(fromdate,'..','/-') with fromday '.' frommonth '.' fromyear
if \datatype(fromyear,'N') then fromyear=right(date('N'),4)
fromdate=right('19'||fromyear,4)||right(frommonth,2,0)||right(fromday,2,0)

if todate='' then todate=date('E') /*till today if no to-date given*/
parse value translate(todate,'..','/-') with today '.' tomonth '.' toyear
if \datatype(toyear,'N') then toyear=right(date('N'),4)
todate=right('19'||toyear,4)||right(tomonth,2,0)||right(today,2,0)


/* Get all costlog files matching the definition of the 'COSTLOG'-Statement. */
if netmail then do
  cr=x2c('0D')          /* End-Of-Line string */
  call writemsgheader   /* Write PAcket-Header */
end; else do
  cr=x2c('0D 0A')       /* End-Of-Line string */
  pkt='STDOUT'          /* Open standard-output */
  rc=stream(pkt,'c','open write')
end

call charout pkt,cr
call charout pkt,'Phonecost-Calalculation'||cr
call charout pkt,'from 'fromday'.'frommonth'.'fromyear' to 'today'.'tomonth'.'toyear ':'||cr
call charout pkt,'---------------------------------------------'||cr

fee_total=0

/* Process every costlog file matching the definition */
do costfilenr=1 to costlogfile.0

   costlogfile=costlogfile.costfilenr
   /* Open the Costlog file.
      since access to this file is not possible while binkley is running
      it's copied by 'TYPE' and the resulting copy is read instead */
   newcostlogfile=systempfilename(filespec('drive',costlogfile)||filespec('path',costlogfile)||'costtmp.???')
   '@TYPE 'costlogfile' >'newcostlogfile

   call charout pkt,'*** 'costlogfile||cr

   rc=Stream(newcostlogfile,'c','OPEN READ')
   if \(rc='READY:') then do
    Say 'Error: cannot open 'newcostlogfile': 'rc
    call halt 3
   End;


   /* ignore first line, here usually only column descriptions are found.
    if this ist not true for your costlog-files comment out the following line */
   line=linein(newcostlogfile)

   /* while there are unread lines in the active costlog-file loop*/
   do while stream(newcostlogfile,'S')='READY'

     /* read in line and parse it */
     call GetConnection linein(newcostlogfile)

     /* If year not specified by Shortcost.log, assume today's year */
     if \shortcostlog_mask.year then connection.year=right(date('N'),2)

     /* Translate date of the read in entry to 'yyyymmdd'-format */
     thedate=right('19'||connection.year,4)||right(connection.month,2,'0')||right(connection.day,2,'0')

     if (thedate>=fromdate) & (thedate<=todate) then do
        /* it's within the specified time range */
        line=right(Connection.day,2)'.'right(Connection.month,2,'0')'.'right(Connection.year,2,'0')

        /* if time of call is supplied by Costlog add to Output */
        if shortcostlog_mask.hour & shortcostlog_mask.minute then do
           line=line right(connection.hour,2)':'right(connection.minute,2,'0')
           if shortcostlog_mask.second then line=line||':'right(connection.second,2,'0')
        end;

        /* if duration of call is supplied by Costlog add to Output */
        if shortcostlog_mask.seconds then line=line right(connection.seconds,4)'s'

        /* if supplied by Costlog, find tariff matching to tariffindex
            (in Germany the tariff-index refers to different zones of distance,each with unique fees/sec) */
        if shortcostlog_mask.tariff then
          select
             when connection.tariff=0  then Tarif='frei'
             when connection.tariff=1  then Tarif='City'
             when connection.tariff=2  then Tarif='R50'
             when connection.tariff=3  then Tarif='R200'
             when connection.tariff=4  then Tarif='fern'
             when connection.tariff=5  then Tarif='Vis-a vis'
             when connection.tariff=6  then Tarif='Euro-City'
             when connection.tariff=7  then Tarif='Euro1'
             when connection.tariff=8  then Tarif='Euro2'
             when connection.tariff=9  then Tarif='Welt1'
             when connection.tariff=10 then Tarif='Welt2'
             when connection.tariff=11 then Tarif='Welt3'
             when connection.tariff=12 then Tarif='Welt4'
             otherwise Tarif='unbekannt' /*unknown*/
          end
          else tarif=''
        line=line||center(tarif,9)' -> 'connection.cost 'Pf.'

        /* write out connection's data */
        call charout pkt,line||cr

        /* add fee */
        fee_total=fee_total + connection.cost

     end;
   end;
   /* close costfile and delete the copied file */
   rc=Stream(newcostlogfile,'c','Close')
   rc=sysfiledelete(newcostlogfile)
end
call charout pkt,'--------------------------------------------'||cr
call charout pkt,'You have spent' fee_total%100','right(fee_total//100,2,'0') currency'.'||cr
if netmail then call charout pkt,x2c('00 00 00')
rc=stream(pkt,'c','close')
call halt 0

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
Halt: PROCEDURE
 parse arg errorlevel
 rc=endlocal()
 exit errorlevel
End

Syntaxhelp: PROCEDURE
  Say ''
  Say 'Phonecost        (c) 1997 by Jrgen Appel 2:2426/4001.39'
  Say 'SYNTAX:'
  Say ' Phonecost Binkley.cfg -fdd.mm.yy -tdd.mm.yy -N [environment settings]'
  Say '               ³            ³          ³      ³'
  Say '               ³            ³          ³ Use this switch to make'
  Say '               ³            ³          ³ Phonecost create a Netmail'
  Say '               ³            ³          ³ report'
  Say '               ³            ³          ÀÄend of period to evaluate'
  say '               ³            ³             (today assumed if not given)'
  Say '               ³            ÀÄÄÄÄÄÄÄÄÄÄÄÄbegin of evaluation-period'
  Say '               ³                          (first entry if not given)'
  Say '               ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄBinkley''s-config-file'
  Say ''
  say ' You may set up environment variables in the commandline, too:'
  say ' simply append them to your other parameters. '
  say ' '
  say ' PHONECOST D:\Com\Binkley\Binkley.cfg -f18.1.97 -N Task=1 '
  say '  This Command adds up your phoning costs since 18.1.97 and outputs '
  say '  a netmail report. It will set the environment variable TASK to 1 so'
  say '  all occurences of %TASK% in your binkley.cfg will be replaced by "1".'
  say ''
Return

Checktemplate: Procedure Expose Shortcostlog_mask.

 if \(shortcostlog_mask.month  & shortcostlog_mask.day & shortcostlog_mask.cost) then do
 say 'Error!'
 say ''
 say 'At least the following information has to be written to your Cost.log-file:'
 say '    Date of call ($d, $m)'
 say '    cost of call ($c) '
 say ''
 call halt 1
 end
return


WC_Pos: PROCEDURE
/* Works exactly as the internal function POS, but supports following Wildcards:
        ?   any (single) char
        *   multiple chars (even zero)
*/
 parse arg template, string, from
 if from='' then from=0; else from=from-1;
 template=strip(template,'B','*')


 string=delstr(string,1,from)

 stern=pos('*',template)
 fragezeichen=pos('?',template)

      if stern=0        then wildcard=fragezeichen
 else if fragezeichen=0 then wildcard=stern
 else wildcard=min(stern,fragezeichen)

 if wildcard>0 then do

    templeft=left(template,wildcard-1)
    if \(templeft='') then do
       found=pos(templeft,string)
       if found=0 then return 0
       string=delstr(string,1,found+wildcard-2)
    end; else found=-1

    wc=substr(template,wildcard,1)
    if wc='?' then string=delstr(string,1,1)

    template=substr(template,wildcard+1,length(template)-wildcard)
    i=WC_Pos(template,string)
    if (wc='?') & \(i=1) & \(found=-1) then i=0

    if i=0 then return 0
    if found=-1 then found=i
    return (found+from)
 end
 else
 if template=string then return 1+from
                    else return (pos(template,string)+from)







/* ---------------------------------------------------------------------------------------- */







/* ***************************************************************************************** */
Gettemplate: PROCEDURE Expose template binkleycfg Shortcostlog_template Shortcostlog_mask.
/* This procedure reads in the shortcostlog-line out of Binkley' cfg-file.
   So a parsing-template for latter evaliation of the costlog-file will be created.
   It's stored in a variable named 'template'.
   Die Variable 'binkleycfg' muá den Filenamen der Binkley-Konfigurationsdatei beinhalten.
   Shortcostlog_template contains the template as it appears in Binkley.cfg (for whatever
   purposes you might need it)
   Shortcostlog_mask tells you, which information ist specified by your Shortcostlog.
   If an Item is defined, it is set to 1, if not, it's 0.
   */
  rc=FileSearch('Shortcostlog', binkleycfg, 'Shortcostlog.')

  if rc>0 then do
    call syntaxhelp
    say 'Error: Cannot open 'binkleycfg'.';
    call halt 1
  End;
  i=1;

  do while (i<=shortcostlog.0) & (i>0)
   shortcostlog.i=strip(shortcostlog.i)
   if \(translate(left(shortcostlog.i,12))='SHORTCOSTLOG') then i=i+1; /*ignore comments*/
   else do
        parse value Shortcostlog.i with . template
        template=Processpercents(strip(template))
        Shortcostlog_template=template
        i=0;
   end;
  end
  if i>0 then do
    call syntaxhelp
    say 'Error: No Shortcostlog statement found in 'binkleycfg '!'
    call halt 2
  end
  /* delete numbers following the $-char */
  last=0
  do until last=0
    last=pos('$',template,last+1)
    lang=0
    do while Datatype(substr(template,last+1,lang+1),'N')
       lang=lang+1;
    end
    template=delstr(template,last+1,lang)
  end

  Shortcostlog_mask.tasknumber              = pos('$#',template)>0
  Shortcostlog_mask.nodeaddress             = pos('$A',template)>0
  Shortcostlog_mask.baudrate                = pos('$B',template)>0
  Shortcostlog_mask.seconds                 = pos('$s',template)>0
  Shortcostlog_mask.cost                    = pos('$c',template)>0
  Shortcostlog_mask.tariff                  = pos('$z',template)>0

  Shortcostlog_mask.Day                     = pos('$d',template)>0
  Shortcostlog_mask.Month                   = pos('$m',template)>0
  Shortcostlog_mask.Year                    = pos('$y',template)>0
  Shortcostlog_mask.Abb_Month               = pos('$b',template)>0

  Shortcostlog_mask.Hour                    = pos('$H',template)>0
  Shortcostlog_mask.Minute                  = pos('$M',template)>0
  Shortcostlog_mask.Second                  = pos('$S',template)>0

  Shortcostlog_mask.total.number            = pos('$T',template)>0
  Shortcostlog_mask.total.size              = pos('$t',template)>0
  Shortcostlog_mask.total.cps               = pos('$U',template)>0
  Shortcostlog_mask.total.errors            = pos('$E',template)>0
  Shortcostlog_mask.total.efficiency        = pos('$u',template)>0
  Shortcostlog_mask.total.biggest.size      = pos('$C',template)>0
  Shortcostlog_mask.total.biggest.seconds   = pos('$D',template)>0
  Shortcostlog_mask.total.biggest.cps       = pos('$Y',template)>0


  Shortcostlog_mask.inbound.number          = pos('$I',template)>0
  Shortcostlog_mask.inbound.size            = pos('$i',template)>0
  Shortcostlog_mask.inbound.cps             = pos('$J',template)>0
  Shortcostlog_mask.inbound.errors          = pos('$f',template)>0
  Shortcostlog_mask.inbound.efficiency      = pos('$j',template)>0
  Shortcostlog_mask.inbound.biggest.size    = pos('$V',template)>0
  Shortcostlog_mask.inbound.biggest.seconds = pos('$W',template)>0
  Shortcostlog_mask.inbound.biggest.cps     = pos('$X',template)>0

  Shortcostlog_mask.outbound.Number         = pos('$O',template)>0
  Shortcostlog_mask.outbound.size           = pos('$o',template)>0
  Shortcostlog_mask.outbound.cps            = pos('$P',template)>0
  Shortcostlog_mask.outbound.errors         = pos('$g',template)>0
  Shortcostlog_mask.outbound.Efficiency     = pos('$p',template)>0
  Shortcostlog_mask.outbound.biggest.size   = pos('$v',template)>0
  Shortcostlog_mask.outbound.biggest.seconds= pos('$w',template)>0
  Shortcostlog_mask.outbound.biggest.CPS    = pos('$x',template)>0


  Call Replace '"' "'"
  Call Replace '$#' '"Connection.tasknumber"'
  Call Replace '$A' '"Connection.nodeaddress"'
  Call Replace '$B' '"Connection.baudrate"'
  Call Replace '$s' '"Connection.seconds"'
  Call Replace '$c' '"Connection.cost"'
  Call Replace '$z' '"Connection.tariff"'

  Call Replace '$d' '"Connection.Day"'
  Call Replace '$m' '"Connection.Month"'
  Call Replace '$y' '"Connection.Year"'
  Call Replace '$b' '"Connection.Abb_Month"'

  Call Replace '$H' '"Connection.Hour"'
  Call Replace '$M' '"Connection.Minute"'
  Call Replace '$S' '"Connection.Second"'

  Call Replace '$T' '"Connection.total.number"'
  Call Replace '$t' '"Connection.total.size"'
  Call Replace '$U' '"Connection.total.cps"'
  Call Replace '$E' '"Connection.total.errors"'
  Call Replace '$u' '"Connection.total.efficiency"'
  Call Replace '$C' '"Connection.total.biggest.size"'
  Call Replace '$D' '"Connection.total.biggest.seconds"'
  Call Replace '$Y' '"Connection.total.biggest.cps"'


  Call Replace '$I' '"Connection.inbound.number"'
  Call Replace '$i' '"Connection.inbound.size"'
  Call Replace '$J' '"Connection.inbound.cps"'
  Call Replace '$f' '"Connection.inbound.errors"'
  Call Replace '$j' '"Connection.inbound.efficiency"'
  Call Replace '$V' '"Connection.inbound.biggest.size"'
  Call Replace '$W' '"Connection.inbound.biggest.seconds"'
  Call Replace '$X' '"Connection.inbound.biggest.cps"'

  Call Replace '$O' '"Connection.outbound.Number"'
  Call Replace '$o' '"Connection.outbound.size"'
  Call Replace '$P' '"Connection.outbound.cps"'
  Call Replace '$g' '"Connection.outbound.errors"'
  Call Replace '$p' '"Connection.outbound.Efficiency"'
  Call Replace '$v' '"Connection.outbound.biggest.size"'
  Call Replace '$w' '"Connection.outbound.biggest.seconds"'
  Call Replace '$x' '"Connection.outbound.biggest.CPS"'

  Call Replace '$$' '$'



  last=1
  do until last=0
    last=pos(' ',template,last)
    if last>0 then template=delstr(template,last,1);
  end;
  last=1
  do until last=0
    last=pos('""',template,last)
    if last>0 then do
     template=delstr(template,last,2);
     template=insert(' ',template,last-1);
    end
  end;

  template=strip(template,'B','"') '.'
RETURN

 replace: PROCEDURE expose template
 parse arg alt neu
 last=1
 do until last=0
   last=pos(alt,template,last)
   if last>0 then do
    template=delstr(template,last,length(alt));
    template=insert(neu,template,last-1);
   end
 end;
 return last>0


/* ***************************************************************************************** */
Getcostfiles: PROCEDURE Expose costlogfile. binkleycfg
/* This procedure reads in the Costlog-line from Binkley.cfg.
   So the path of the Costlog-Files is calculated and stored in the 'costfiles'-Stem.
   'binkleycfg' has to contain the name of Binkley's configuration file. */
  rc=FileSearch('costlog', binkleycfg, 'costlog.')
  if rc>0 then do
    call syntaxhelp
    say 'Error: Cannot open 'binkleycfg'.';
    call halt 1
  End;
  i=1; costfiles=''

  drop costlogfile.
  Costlogfile.0=0
  do while (i<=costlog.0)
   costlog.i=strip(costlog.i)
   if translate(left(costlog.i,7))='COSTLOG' then
   do
        parse value costlog.i with . costfiles .
        costfiles=Processpercents(strip(costfiles))

        if costfiles='' then do
          call syntaxhelp
          say 'Error: No Costlog-Path defined in 'binkleycfg '!'
          call halt 2;
        end

        rc=sysfiletree(costfiles,'File.','F')
        do k=1 to file.0
           parse UPPER VAR File.k . . . . name .
           ok=1
           if k>1 then do l=1 to k-1
              ok=ok & \(name=file.l)
            end
           if ok then do
              j=costlogfile.0+1
              costlogfile.0=j
              costlogfile.j=name
           end
        end;
        drop file.
   end;
   i=i+1
  end
  if costlogfile.0=0 then do
   Call Syntaxhelp
   Say 'Error: No costlogfile file.'
   call halt 2
  End;
RETURN

Getconnection: Procedure expose template Connection.
/* This prozedue parses a string (=line from Costlog -file)
   with the template defined in binkley.cfg.

   After execution of this procedure the data is
   stored in a stem named 'Connection.' as follows:

     $#     Connection.tasknumber
     $A     Connection.nodeaddress
     $B     Connection.baudrate
     $s     Connection.seconds
     $c     Connection.cost
     $z     Connection.tariff

     $d     Connection.Day
     $m     Connection.Month
     $y     Connection.Year
     $b     Connection.Abb_Month

     $H     Connection.Hour
     $M     Connection.Minute
     $S     Connection.Second

     $T     Connection.total.number
     $t     Connection.total.size
     $U     Connection.total.cps
     $E     Connection.total.errors
     $u     Connection.total.efficiency
     $C     Connection.total.biggest.size
     $D     Connection.total.biggest.seconds
     $Y     Connection.total.biggest.cps


     $I     Connection.inbound.number
     $i     Connection.inbound.size
     $J     Connection.inbound.cps
     $f     Connection.inbound.errors
     $j     Connection.inbound.efficiency
     $V     Connection.inbound.biggest.size
     $W     Connection.inbound.biggest.seconds
     $X     Connection.inbound.biggest.cps

     $O     Connection.outbound.Number
     $o     Connection.outbound.size
     $P     Connection.outbound.cps
     $g     Connection.outbound.errors
     $p     Connection.outbound.Efficiency
     $v     Connection.outbound.biggest.size
     $w     Connection.outbound.biggest.seconds
     $x     Connection.outbound.biggest.CPS

     Variables referencing to information not given in the log-file line
     remain undefined.
    */
  parse arg line
  drop Connection.
  interpret "parse value line with" template
Return

ProcessPercents: PROCEDURE
/* This procedure evaluates the percent characters (%) the way Binkley does.
   The only parameter that has to be supplied is the string to be processed.
   If the string starts wit a "%" it's treated as a comment and no further
        processing is done.
   Otherwise every environment-Identifier enclosed by % (like %Task%) is
   replaced by it's value.
   A double percent (%%) is converted by a single "%".          */
 Parse arg string

 string=strip(string)

last1=0
do until last1=0
 last1=pos('%',string,last1+1)
 if last1>0 then do /* could be beginning of an "environment" identifer */
   last2=pos('%',string,last1+1)
   if last2=0        then return string
                        /* no corresponding second percent: return */ else
   if last2=last1+1 then string=delstr(string,last2,1)
                        /* replace double percent by single quote */ else
   do
                        /* replace environment identifier by value */
    identifier=substr(string,last1+1,last2-last1-1)
    string=delstr(string,last1,last2-last1+1)
    env=value(identifier,,'OS2ENVIRONMENT')
    if env='' then do
     say'Error: Environment variable %'identifier'% undefined.'
     call halt 3
    end;
    string=insert(env,string,last1-1)
   end
 end
end
return string


/* ***************************************************************************************** */
GetInbound: PROCEDURE Expose Binkleycfg Inboundpath
/* This prozedure reads in the Nefile Protinbound and Knowninbound-line(s) from Binkley.cfg.
   So the inbound-path is calculated and stored in the 'Inboundpath'-Variable.
   'binkleycfg' has to contain the name of Binkley's configuration file. */

/* first look for protected inbound */
rc=FileSearch('Protinbound', binkleycfg, 'Inbound.')
if rc>0 then do
  call syntaxhelp
  say 'Error: Cannot open 'binkleycfg'.';
  call halt 1
End;
i=1; Inboundpath=''
do while (i<=Inbound.0)
 inbound.i=strip(inbound.i)
 if translate(left(inbound.i,1))='P' then
 do
      parse value inbound.i with . Inboundpath .
      call Checkinbound
 end;
 i=i+1;
end
if \(inboundpath='') then return;
/* no protected inbound defined: look for Known inbound */
rc=FileSearch('Knowninbound', binkleycfg, 'Inbound.')
if rc>0 then do
  call syntaxhelp
  say 'Error: Cannot open 'binkleycfg'.';
  call halt 1
End;
i=1; Inboundpath=''
do while (i<=Inbound.0)
 inbound.i=strip(inbound.i)
 if translate(left(inbound.i,1))='K' then
 do
      parse value inbound.i with . Inboundpath .
      call Checkinbound
 end;
 i=i+1
end
if \(inboundpath='') then return;
/* no known inbound defined: look for Unsecure inbound */
rc=FileSearch('Netfile', binkleycfg, 'Inbound.')
if rc>0 then do
  call syntaxhelp
  say 'Error: Cannot open 'binkleycfg'.';
  call halt 1
End;
i=1; Inboundpath=''
do while (i<=Inbound.0)
 inbound.i=strip(inbound.i)
 if translate(left(inbound.i,1))='N' then
 do
      parse value inbound.i with . Inboundpath .
      call Checkinbound
 end;
 i=i+1;
end
if \(inboundpath='') then return;
/* no inbound defined at all */
  call syntaxhelp
  say 'Error: No Inbound-Directory defined in 'binkleycfg '!'
  call halt 2
RETURN
Checkinbound: PROCEDURE expose Inboundpath
/*Checks whether the Inbound Directory really exists and
  replaces wildcards by real name*/
inboundpath=Processpercents(strip(strip(inboundpath),'T','\'))
rc=sysfiletree(inboundpath,'File.','D')
parse value File.1 with . . . . Inboundpath .
return

WriteMsgheader: PROCEDURE Expose pkt cr binkleycfg
/* Creates a .PKT-Packet in your 'most protected Inbound dir' and opens
   it for writing. PKT is set to the filename of this Packet.
   Destination address of the created repord is the first Address-entry in
   binkley.cfg. Therefore binkleycfg has to be set up properly before entering
   this procedure. */

  /* getting the 'most protected Inbound' path from Binkley.cfg */
  Call GetInbound
  rc=FileSearch('Address', binkleycfg, 'Addrs.')
  if rc>0 then do
    call syntaxhelp
    say 'Error: Cannot open 'binkleycfg'.';
    call halt 2
  End;
  i=1; addr=''
  do while (i<=addrs.0)
   addrs.i=strip(addrs.i)
   if translate(left(addrs.i,1))='A' then
   do
     parse value addrs.i with . addr .
     addr=processpercents(addr)
        i=900; /* that's near enogh to infinity ;-) */
   end;
  end
  parse value addr with zone':'net'/'node'.'point
  if point='' then point=0
  xzone =reverse(x2c(right(d2x(zone),4,'0') )) /*convert to a binary-word */
  xnode =reverse(x2c(right(d2x(node),4,'0') ))
  xnet  =reverse(x2c(right(d2x(net),4,'0') ))
  xpoint=reverse(x2c(right(d2x(point),4,'0') ))
  pkt=systempfilename(inboundpath'\'||date('B')||'??.PKT')
  Say 'PHONECOST: Writing report to packet 'pkt
  rc=stream(pkt,'c','open write')
  parse value date('E') with day'/'month'/'year
  if year < 70 then
    year=reverse(x2c(d2x(year+2000)))
  else
    year=reverse(x2c(d2x(year+1900)))
  month=reverse(x2c(right(d2x(month-1),4,'0') ))
  day=reverse(x2c(right(d2x(day),4,'0') ))

  parse value time('N') with hour':'minute':'sec
  hour=reverse(x2c(right(d2x(hour),4,'0') ))
  minute=reverse(x2c(right(d2x(minute),4,'0') ))
  sec=reverse(x2c(right(d2x(sec),4,'0') ))

  /* Packet - Header ( refers to FSC-0039 ) */
  call charout pkt,xnode /* Orgnode */
  call charout pkt,xnode /* Dstnode */
  call charout pkt,year||month||day||hour||minute||sec /* Date packet generated */
  call charout pkt,x2c('00 00') /* Baudrate ?? */
  call charout pkt,x2c('02 00') /* PAcket - Version */
  call charout pkt,xnet /*orgnet */
  call charout pkt,xnet /*dstnet */
  call charout pkt,x2c('7A') /* Productcode lo (=Squish) */
  call charout pkt,x2c('01') /* Product-revision (hi) */
  call charout pkt,x2c('00 00 00 00 00 00 00 00') /*No Password */
  call charout pkt,xzone /*orgzone */
  call charout pkt,xzone /*dstzone */
  call charout pkt,x2c('00 00') /* Spare Change ?? */
  call charout pkt,x2c('00 01') /* CW , byte swapped */
  call charout pkt,x2c('00') /* Productcode hi (=Squish) */
  call charout pkt,x2c('0B') /* Product-revision (lo) */
  call charout pkt,x2c('01 00') /* CW , byte swapped */
  call charout pkt,xzone /*orgzone */
  call charout pkt,xzone /*dstzone */
  call charout pkt,xpoint /*orgpoint */
  call charout pkt,xpoint /*dstpoint */
  call charout pkt,'XPKT'

  /* Message (refers to FTS-0001) */
  call charout pkt,x2c('02 00') /*Message-sign*/
  call charout pkt,xnode /* orignode */
  call charout pkt,xnode /* destnode */
  call charout pkt,xnet  /* orignet */
  call charout pkt,xnet  /* destnet */
  call charout pkt,x2c('0500') /* Attribute */
  call charout pkt,x2c('0000') /* Cost */
  datetime=right(delstr(date(n),7,2),9,'0')  time(n)||d2c(0)
  call charout pkt,datetime /*datetime */
  call charout pkt,'Sysop'||d2c(0) /* Reciever */
  call charout pkt,'PhoneCost'||d2c(0) /*Sender */
  call charout pkt,'Phonecosts'||d2c(0) /*Subject */
  if point<>0 then do
    call charout pkt,d2c(1)||'TOPT '||point||cr
    call charout pkt,d2c(1)||'FMPT '||point||cr
  end
  call charout pkt,d2c(1)||'INTL 'zone':'net'/'node' 'zone':'net'/'node||cr
  call charout pkt,d2c(1)||'PID: Phonecost'||cr
  call charout pkt,d2c(1)||'MSGID: 'zone':'net'/'node'.'point' '||d2x(random(65535))||d2x(random(65535))||cr
  call charout pkt,d2c(1)||'CHRS: IBMPC 2'||cr
RETURN



Filesearch:
/* works exactly as Sysfilesearch, but considers binkley-config-style sections defined by
   environment variables ([COMMON] , [%Task%=1] e.c.).
   also it does not search comments  */

Parse arg __1, __2, __stem, __3
rc=__filesearch(__1, __2, __stem, __3)
RETURN rc

__FileSearch: PROCEDURE Expose (__stem)

parse arg template, file, stem, option

rc=stream(file,'c','open read')
if \(rc='READY:') then return 3
ignorecase=(pos('C',translate(option))=0)
if ignorecase then template=translate(template)

call value __stem'0',0
sectionok=1
lnr=0
do while stream(file,'State')='READY'
   line=strip(linein(file))
   lnr=lnr+1

   first=left(line,1)
   if \(first='%') & \(first=';') then do /* if it's no 'line' comment */
     i=pos(' ;',line)                     /* eliminate rest-of-line comments */
     do while i>0
        if instr(line,i+2,1)=';' then i=pos(' ;',line,i+3)
        else do
            line=left(line,i-1)
            i=0
        end
     end

     if \(line='') then
      if (first='[') then do  /* Line with section definition */
         i=pos(']',line)
         equation=translate(processpercents(substr(line,2,i-2)))
         i=pos('=',equation)
         if i>0 then do
            if substr(equation,i,2)='==' then equation=delstr(equation,i,1)
            term1=left(equation,i-1)
            term2=delstr(equation,1,i)
            sectionok= ( (WC_pos(term1,term2)=1) | (WC_pos(term2,term1)=1) )
         end; else if equation='COMMON' then sectionok=1
      end;
      else if sectionok then do

           if ignorecase then tline=translate(line); else tline=line

           if pos(template,tline)>0 then do
              i=value(__stem'0')+1
              call value __stem'0',i

              if pos('N',option)>0 then line=lnr line

              call value __stem||i, line
           end
      end
   end
end
rc=stream(file,'c','close')
return 0;

