[ If you're looking for the patch for the MaxComm.DLL included in the
  BTXE package, have a look at the end of this file. ]

=================================== OS2COMM ====================================
   From: Greg Matheson                       3:626/661.1     14 Aug 96  17:35:12
     To: Martin Noth                                         18 Aug 96  19:22:22
   Subj: Exit with DTR set high under BinkOS/2-XR2 ???                          
================================================================================
On Aug 11 17:59 96, Martin Noth of 2:241/1077.2 wrote:

 MN> Hi there,

 MN> * How can I get the port free when exiting from 
 MN> BinkOS/2-XR2 without dropping DTR?

Use the a patched  version of Maxcomm.dll from Maximus 3.1

I had the same problems when getting Bink to answer a PPP attempt.

;Maxcomm.PAT                                                            
;----------------------------------------------------                   
;    This patch alters the share mode in MaxComm.DLL from              
;    DenyReadWrite to DenyNone.                                        
;
;       Use this file by typing                                        
;       PATCH MAXCOMM.PAT /A                                           
;       at an OS/2 prompt in your Maximus directory, and in any other  
;    directory that contains MaxComm.DLL (eg, C:\OS2\DLL) - if you     
;    don't get every copy, the results, though predictable, are        
;    confusing   :-)                                                   
;
;
; Maxcomm.dll as supplied with Maximus 3.00 & 3.01.                    
;
FILE maxcomm.dll                                                       
;
VER 00000425 506A016812405050                                          
CHA 00000425 506A016842405050                                          
;
;                                                                      
;-----------------------------------                                    

Greg

--- Msgedsq/2 2.2e
 * Origin: From the Desktop of Homeport (3:626/661.1)
================================================================================


; Patch for MaxComm.DLL version 2.90.1   by A.G., 2:2411/525
;-----------------------------------------------------------
;    This patch alters the share mode in MaxComm.DLL from 
;    DenyReadWrite to DenyNone.
;
;       Use this file by typing
;       PATCH MAXCOMM.PAT /A
;       at an OS/2 prompt in your Binkley directory, and in any other
;    directory that contains MaxComm.DLL (eg, C:\OS2\DLL) - if you
;    don't get every copy, the results, though predictable, are
;    confusing   :-)
;
;
; Maxcomm.dll as supplied with Binkley XE.
;
FILE maxcomm.dll
;
VER 0000044B 506A016A1250509AFF
CHA 0000044B 506A016A4250509AFF
;
;
;-----------------------------------------------------------

