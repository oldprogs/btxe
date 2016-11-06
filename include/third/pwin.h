/*
Copyright 1989, 1998 by Lanius Corporation.  All rights reserved.

COMMERCIAL DISTRIBUTION AND/OR USE IS PROHIBITED WITHOUT WRITTEN
CONSENT FROM LANIUS CORPORATION.

Noncommercial distribution or use is permitted under the
following terms:

- You may copy and distribute verbatim copies of the NTCOMM
documentation, source and executable code as you receive it, in any
medium, provided that you conspicuously and appropriately publish on
each copy a valid copyright notice "Copyright 1989, 1995 by Lanius
Corporation"; keep intact the notices on all files that refer to
this License Agreement and to the absence of any warranty;  PROVIDE
UNMODIFIED COPIES OF THE DOCUMENTATION AS PROVIDED WITH THE PROGRAM;
and give any other recipients of the NTCOMM program a copy of this
License Agreement along with the program.  You may charge a
distribution fee for the physical act of transferring a copy, but no
more than is necessary to recover your actual costs incurred in the
transfer.

- Mere aggregation of another unrelated program with this program
and documentation (or derivative works) on a volume of a storage or
distribution medium does not bring the other program under the scope
of these terms.

- You may not copy, sublicense, distribute or transfer NTCOMM and
its associated documentation except as expressly provided under this
License Agreement.  Any attempt otherwise to copy, sublicense,
distribute or transfer NTCOMM is void and your rights to use the
program under this License agreement shall be automatically
terminated.

However, parties who have received computer software programs from
you with this License Agreement will not have their licenses
terminated so long as such parties remain in full compliance, and
notify Lanius Corporation of their intention to comply with this
Agreement.

- You may not incorporate all or part of NTCOMM (including related
utilities) into a program which is not completely free for all
users.  If you wish to distribute NTCOMM in this manner, you must
obtain written permission from Lanius Corporation.

- This license may be revoked by Lanius Corporation without prior
notice.

WARRANTY

LANIUS CORPORATION PROVIDES ABSOLUTELY NO WARRANTY.  EXCEPT WHEN
OTHERWISE STATED IN WRITING, LANIUS CORPORATION AND/OR OTHER PARTIES
PROVIDE NTCOMM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF NTCOMM, AND THE
ACCURACY OF ITS ASSOCIATED DOCUMENTATION, IS WITH YOU.  SHOULD
NTCOMM OR ITS ASSOCIATED DOCUMENTATION PROVE DEFECTIVE, YOU ASSUME
THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT WILL LANIUS CORPORATION BE RESPONSIBLE IN ANY WAY FOR
THE BEHAVIOR OF MODIFIED VERSIONS OF NTCOMM.  IN NO EVENT WILL
LANIUS CORPORATION AND/OR ANY OTHER PARTY WHO MAY REDISTRIBUTE
NTCOMM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING
ANY LOST PROFITS, LOST MONIES, OR OTHER SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE
(INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED
INACCURATE OR LOSSES SUSTAINED BY THIRD PARTIES OR A FAILURE OF THE
PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) NTCOMM, EVEN IF LANIUS
CORPORATION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES, OR
FOR ANY CLAIM BY ANY OTHER PARTY.

If you have any questions about this license agreement, please feel
free to get in touch with us.

http://www.lanius.com
*/

#ifndef __PWIN_H_DEFINED
#define __PWIN_H_DEFINED

#ifdef NT
  #define WIN32_LEAN_AND_MEAN
  #define _INC_DDEMLH
/*  #define NOGDI*/
  #define NOUSER
  #define NOMM
  #define NOREG
  #define NONETWK
  #define NOCDERR
  #define NOCOMMDLG
  #define NODDE
  #define NODLGS
  #define NODRIVINIT
  #define NOLZEXPAND
  #define NONB30
  #define NOOLE
  #define NORPC
  #define NOSHELLAPI
  #define NOPERF
  #define NOSOCK
  #define NOSPOOL
  #define NOSERVICE

  #ifndef YESWINERROR
    #define NOWINERROR
  #endif

  #define NOATOM
  #define NOCLIPBOARD
  #define NOCOLOR
  #define NOSOUND
  #define NOPROFILER

  #include <windows.h>

  /* !@#$!@$# windows header tries to redefine "pascal" and gets
   * it wrong.  (It tries to change it to "__stdcall"!)
   */

  #undef pascal
  #define pascal __pascal
  
  #undef cdecl
  #define cdecl __cdecl

  #pragma pack()
#endif

#endif /* __PWIN_H_DEFINED */

