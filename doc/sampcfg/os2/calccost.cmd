/* Calculate costs with BT-XE ShortCostLog feature
 *
 * 961116 Freeware written by Thomas Waldmann
 * Thanks to D. Ahlgrimm for "Match" function.
 *
 * Usage:
 * calccost <costlog filename> <addr spec.> <date start> <date end>
 *
 * This will calculate statistics for systems matching <addr spec.> (may
 * contain * and ?) starting at <date start> (this day included) up to
 * <date end> (this day included, too).
 *
 * E.g.:
 *
 * calccost log\3\c.log * 960101 961231
 * -> calculates costs for line 3 to/from all systems for whole year 96
 * calccost log\3\c.log 2:2474/4* 961101 961130
 * -> ... to/from all systems 2:2474/4* for Nov. 96
 * calccost log\3\c.log 2:2474/4?? 961101 961130
 * -> ... to/from systems 2:2474/4?? for Nov. 96
 * calccost log\3\c.log 2:2474/400.* 961101 961130
 * -> ... to/from systems 2:2474/400.* for Nov. 96
 *
 * If you do not use default ShortCostLog layout, then you have to adapt
 * the "parse" call below ...
 */

version = '1.2'
 
parse arg costlog system datestart dateend .

say 'CalcCost.Cmd v'version' - Cost Calculator for BT-XE by Thomas Waldmann'
say

if costlog='' then
  do
    say 'No commandline parameters given, going into interactive mode...'
    say
    call charout ,'Filename of ShortCostLog     ? : '; parse pull costlog
    call charout ,'System (address with ? or *) ? : '; parse pull system
    call charout ,'Start date (e.g. 960101)     ? : '; parse pull datestart
    call charout ,'End date (e.g. 961231)       ? : '; parse pull dateend
  end
  
sum.cost     = 0
sum.rcost    = 0
sum.seconds  = 0
sum.calls    = 0
sum.inbytes  = 0
sum.outbytes = 0

if(stream(costlog, 'c', 'open read')\='READY:') then
  do
    say 'Cannot open' costlog '!'
    say 'Please ensure EXCLUSIVE access to this file - BT-XE must NOT access it!'
    exit
  end

do while lines(costlog)<>0
  line=strip(linein(costlog))
  if length(line)>0 then
    do
      _system=''
      _date='000000'
      parse value line with _date _time _taskno _system _connectrate _seconds _cost _rcost _inbytes _outbytes .
      if _date>=datestart & _date<=dateend then
        if Match(system,_system) then
          do
            sum.calls    = sum.calls    + 1
            sum.seconds  = sum.seconds  + _seconds
            sum.cost     = sum.cost     + _cost
            sum.rcost    = sum.rcost    + _rcost
            sum.inbytes  = sum.inbytes  + _inbytes
            sum.outbytes = sum.outbytes + _outbytes
          end
    end
end

call stream costlog, 'c', 'close'

say 'Statistics for   :' system 
say 'Time interval    :' datestart '..' dateend 
say 'Calls            :' sum.calls
say 'Seconds          :' sum.seconds 
say 'Inbound traffic  :' sum.inbytes
say 'Outbound traffic :' sum.outbytes
say 'Own costs        :' sum.cost
say 'Remote costs     :' sum.rcost
say

exit

/* -------------------------------------------------
   Liefert 1, wenn "spec" auf "name" passt. Sonst 0.
   "spec" darf * und ? enthalten.

   C-Version    : D.Ahlgrimm     03.1995
   REXX-Version : D.Ahlgrimm  21.06.1995

   05.09.1996  REXX-Code & Algorithmus optimiert (u.a. Grenzen genauer)
*/

Match: PROCEDURE
   PARSE ARG spec, name

   spec_lng=LENGTH(spec)+1
   name_lng=LENGTH(name)+1
   spec_pos=1
   name_pos=1
   DO WHILE spec_pos<spec_lng /* ber alle Zeichen in spec */
      spec_ptr=SUBSTR(spec, spec_pos, 1)
      name_ptr=SUBSTR(name, name_pos, 1)
      IF spec_ptr="*" THEN DO
         IF spec_pos+1=spec_lng THEN
            /* spec gleich '*', danach ist Ende -> name egal */
            RETURN(1)
         ss=SUBSTR(spec, spec_pos+1)
         /* as: Anzahl der '*' in spec */
         as=LENGTH(SPACE(TRANSLATE(ss, COPIES(" ", C2D("*"))"x",, " "), 0))
         DO i=0 to name_lng-name_pos-(LENGTH(ss)-as)
            /* 0 bis Gesamtl„nge - aktuelle Position - Anzahl... */
            /*  ...der nicht-'*'-Zeichen in spec */
            IF Match(ss, SUBSTR(name, name_pos+i))=1 THEN
               /* der Rest von spec (hinter dem '*') passt... */
               /* ...irgendwo auf den Rest von name */
               RETURN(1)
         END
         /* kein match der Reste gefunden */
         RETURN(0)
      END; ELSE DO
         IF (spec_ptr="?" & name_pos<>name_lng) | spec_ptr=name_ptr THEN DO
            spec_pos=spec_pos+1
            name_pos=name_pos+1
         END; ELSE
            /* spec gleich '?' und name ist zuende... */
            /* ...oder Zeichen passen nicht */
            RETURN(0)
      END
   END
   IF name_pos<>name_lng THEN
      /* spec zuende, name noch nicht vollst„ndig abgebaut */
      RETURN(0)
   /* spec und name sind beide abgebaut */
   RETURN(1)

