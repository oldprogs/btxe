/* Generate/Modify FlowFile (or REQ files) - written by Th. Waldmann     */ 
/* 4D address support added by Michael Reinsch                           */
/* NO domain support!                                                    */
/* ----------------------------------------------------------------------*/
/* FlowFile 2:2474/400 F   -> create/update 09aa0190.flo                 */
/* FlowFile 2:2474/400 C   -> create/update 09aa0190.clo                 */
/* FlowFile 2:2474/400.2 H -> create/update 09aa0190.pnt\00000002.hlo    */
/* FlowFile 1:1/2 C        -> create/update 00010002.clo (in Z1 outbound)*/
/* FlowFile 2:2/1 H file.txt -> add entry "file.txt" to 00020001.hlo     */
/* FlowFile 2:2/1 R file.txt -> add entry "file.txt" to 00020001.req     */
/*                                                                       */
/* If you invoke FlowFile.Cmd from another CMD file, you have to use:    */
/* Call FlowFile ...                                                     */

/* changes the values of the variables below according to your needs !!! */

defaultzone     = 2               /* your default outbound's zone number */
defaultoutbound = "e:\out\out"                  /* your default outbound */
rescanflag      = "h:\bt\flags\btrescan.flg"       /* global rescan flag */

/* end of values to change */

/* begin of program */

arg addr flavour line

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

addr    = strip(addr)
flavour = strip(flavour)
line    = strip(line)

call addflow addr, flavour, line
call touchflag rescanflag

exit

/* calculate binkley style outbound file name */

binkoutname: Procedure Expose defaultzone defaultoutbound
  parse arg addr, extension
  parse value addr with zone":"net"/"node"."point
  outbound = defaultoutbound
  if zone <> defaultzone then
   do
    outbound = outbound"."d2x(zone,3)
    rc=SysMkDir(outbound)
   end
  if point <> ' ' & point > 0 then
   do
    outbound = outbound"\"d2x(net,4)||d2x(node,4)".pnt"
    rc=SysMkDir(outbound)
    filename = outbound"\"d2x(point,8)"."extension
   end
  else
    filename = outbound"\"d2x(net,4)||d2x(node,4)"."extension
  return filename

/* test (and set if not already set) binkley outbound BSY flag */

testsetbusy: Procedure Expose defaultzone defaultoutbound 
  parse arg addr
  tmpfile  = SysTempFileName(binkoutname(addr, "???"))
  call touchflag tmpfile
  '@ren' tmpfile '*.BSY >NUL 2>NUL'
  renrc = rc
  if renrc<>0 then
    call SysFileDelete tmpfile
  return renrc

/* remove binkley outbound BSY flag */

delbusy: Procedure Expose defaultzone defaultoutbound 
  parse arg addr
  busyfile = binkoutname(addr, "BSY")
  call SysFileDelete busyfile
  return

/* create / update a flowfile */
  
addflow: Procedure Expose defaultzone defaultoutbound
  parse arg addr, flavour, line

  flavour=translate(flavour)
  
  if testsetbusy(addr)=0 then
    do
      if flavour='R' then
        flowfile = binkoutname(addr, "REQ")
      else
        flowfile = binkoutname(addr, flavour"LO")

      /* create flowfile if it does not exist already */
      /* if it does exist, change nothing             */
      call stream  flowfile, 'c', 'open write'
      if line <> "" then
        call lineout flowfile, line
      call stream  flowfile, 'c', 'close'
      call delbusy addr
    end
  else
    say 'System' addr 'is busy.'
  return

/* "touch" a file (create if not existing, update time stamp if existing) */

/* Attention - this is not a real "touch" yet! */
/* It writes a single byte to the beginning of the file !!!                                          */

touchflag: Procedure
  parse arg flag
  call stream  flag, 'c', 'open write'
  call charout flag, " ", 1    /* is there a better method to "touch" ? */
  call stream  flag, 'c', 'close'
  return
  
/* EOF */
