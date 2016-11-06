/*---------------------------------------------------------------------------*
 *
 *                               BinkleyTerm
 *
 *              (C) Copyright 1987-96, Bit Bucket Software Co.
 *     For license and contact information see /doc/orig_260/license.260.
 *
 *           This version was modified by the BinkleyTerm XE Team.
 *        For contact information see /doc/team.lst and /doc/join.us.
 *  For a complete list of changes see /doc/xe_user.doc and /doc/xe_hist.doc.
 *
 * Filename    : $Source: E:/cvs/btxe/include/keyfncs.h,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:11 $
 * State       : $State: Exp $
 *
 * Description : Function definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* Unattended mode command keys */

#define F_UN_BSE            0xfe00  /* Base of unattended mode fns  */
#define F_UN_ANSWER         (F_UN_BSE+0x00)  /* Send answer string to modem  */
#define F_UN_REINITMODEM    (F_UN_BSE+0x01)  /* User wants to reinit modem   */
#define F_UN_CALLRIGHTNOW   (F_UN_BSE+0x02)  /* Call next eligible node now  */
#define F_UN_POLLBOSS       (F_UN_BSE+0x03)  /* Poll the bossnode            */
#define F_UN_POLLNODE       (F_UN_BSE+0x04)  /* Poll a specified node        */
#define F_UN_POLLPKT        (F_UN_BSE+0x05)  /* Queue a poll to a node       */
#define F_UN_RESCANOUTBOUND (F_UN_BSE+0x06)  /* TJW 960421 rescan outbound   */
#define F_UN_TOGGLEDEBUG    (F_UN_BSE+0x07)  /* toggle debug mode            */
#define F_UN_TOGGLEMODEMW   (F_UN_BSE+0x08)  /* toggle modem window          */

#define F_UN_TERMINALMODE   (F_UN_BSE+0x10)  /* Exit mailer, enter terminal  */
#define F_UN_MSGEDITOR      (F_UN_BSE+0x11)  /* Spawn user's editor          */
#define F_UN_EXITBINK       (F_UN_BSE+0x12)  /* Exit BinkleyTerm             */

#define F_UN_FUNKEY1        (F_UN_BSE+0x21)  /* Exit with errorlevel 10      */
#define F_UN_FUNKEY2        (F_UN_BSE+0x22)  /* Exit with errorlevel 20      */
#define F_UN_FUNKEY3        (F_UN_BSE+0x23)  /* Exit with errorlevel 30      */
#define F_UN_FUNKEY4        (F_UN_BSE+0x24)  /* Exit with errorlevel 40      */
#define F_UN_FUNKEY5        (F_UN_BSE+0x25)  /* Exit with errorlevel 50      */
#define F_UN_FUNKEY6        (F_UN_BSE+0x26)  /* Exit with errorlevel 60      */
#define F_UN_FUNKEY7        (F_UN_BSE+0x27)  /* Exit with errorlevel 70      */
#define F_UN_FUNKEY8        (F_UN_BSE+0x28)  /* Exit with errorlevel 80      */
#define F_UN_FUNKEY9        (F_UN_BSE+0x29)  /* Exit with errorlevel 90      */
#define F_UN_FUNKEY10       (F_UN_BSE+0x2a)  /* Exit with errorlevel 100     */

#define F_UN_SHELL          (F_UN_BSE+0x30)  /* Shell to OS command line     */
#define F_UN_SHELL1         (F_UN_BSE+0x31)  /* Spawn user-command 1         */
#define F_UN_SHELL2         (F_UN_BSE+0x32)  /* Spawn user-command 2         */
#define F_UN_SHELL3         (F_UN_BSE+0x33)  /* Spawn user-command 3         */
#define F_UN_SHELL4         (F_UN_BSE+0x34)  /* Spawn user-command 4         */
#define F_UN_SHELL5         (F_UN_BSE+0x35)  /* Spawn user-command 5         */
#define F_UN_SHELL6         (F_UN_BSE+0x36)  /* Spawn user-command 6         */
#define F_UN_SHELL7         (F_UN_BSE+0x37)  /* Spawn user-command 7         */
#define F_UN_SHELL8         (F_UN_BSE+0x38)  /* Spawn user-command 8         */
#define F_UN_SHELL9         (F_UN_BSE+0x39)  /* Spawn user-command 9         */

#define F_UN_GETFILE        (F_UN_BSE+0x40)  /* Get a file from somebody     */
#define F_UN_SENDFILE       (F_UN_BSE+0x41)  /* Send a file to somebody      */
#define F_UN_KILLNODESMAIL  (F_UN_BSE+0x42)  /* Kill all mail to somebody    */

#define F_UN_ZOOM           (F_UN_BSE+0x43)  /* Zoom Outbound                */

#define F_UN_HELPSCREEN     (F_UN_BSE+0x50)  /* Bring up mailer helpscreen   */
#define F_UN_BLANKSCREEN    (F_UN_BSE+0x51)  /* Force the screen to blank    */
#define F_UN_REPAINTSCREEN  (F_UN_BSE+0x52)  /* Repaint the hosed screen     */

#define F_UN_ENTERCHAT      (F_UN_BSE+0x60)  /* Enter Chat mode              */
#define F_UN_RESTARTEVENTS  (F_UN_BSE+0x61)  /* Restart nonforced events     */
#define F_UN_QUITTHISEVENT  (F_UN_BSE+0x62)  /* End this event now           */
#define F_UN_CLEARHIST      (F_UN_BSE+0x63)  /* Reset Today-At-A-Glance      */

#define F_ZOOM_KILLNODE     (F_UN_BSE+0x64)  /* kill                         */
#define F_ZOOM_ADDRESS      (F_UN_BSE+0x65)  /* readdress                    */
#define F_ZOOM_CRASH        (F_UN_BSE+0x66)  /* crash                        */
#define F_ZOOM_DIRECT       (F_UN_BSE+0x67)  /* direct                       */
#define F_ZOOM_GET          (F_UN_BSE+0x68)  /* get file                     */
#define F_ZOOM_HOLD         (F_UN_BSE+0x69)  /* hold                         */
#define F_ZOOM_KILLTRIES    (F_UN_BSE+0x6a)  /* kill dial tries              */
#define F_ZOOM_NORMAL       (F_UN_BSE+0x6b)  /* normal                       */
#define F_ZOOM_POLL         (F_UN_BSE+0x6c)  /* poll                         */
#define F_ZOOM_KILLREQ      (F_UN_BSE+0x6d)  /* kill request                 */
#define F_ZOOM_SEND         (F_UN_BSE+0x6e)  /* send file                    */
#define F_ZOOM_STOPMAIL     (F_UN_BSE+0x6f)  /* stop mail                    */

#define F_ZOOM_NODEINFO     (F_UN_BSE+0x01)  /* get nodelistinfo in zoomwin  */
#define F_ZOOM_NODEEDIT     (F_UN_BSE+0x11)  /* edit nodes flags in zoomwin  */

#define F_PEND_DNAR         (F_UN_BSE+0x70)  /* Scroll Pending down 1 line   */
#define F_PEND_UPAR         (F_UN_BSE+0x71)  /* Scroll Pending up 1 line     */
#define F_PEND_PGDN         (F_UN_BSE+0x72)  /* Scroll Pending down 4 lines  */
#define F_PEND_PGUP         (F_UN_BSE+0x73)  /* Scroll Pending up 4 lines    */
#define F_PEND_HOME         (F_UN_BSE+0x74)  /* Scroll Pending to top        */
#define F_PEND_END          (F_UN_BSE+0x75)  /* Scroll Pending to bottom     */

/* Recent Activity window scrolling */

#define F_CALLWIN_DNAR      (F_UN_BSE+0x76)  /* Scroll RA down 1 line        */
#define F_CALLWIN_UPAR      (F_UN_BSE+0x77)  /* Scroll RA up 1 line          */
#define F_CALLWIN_PGDN      (F_UN_BSE+0x78)  /* Scroll RA down 9 lines       */
#define F_CALLWIN_PGUP      (F_UN_BSE+0x79)  /* Scroll RA up 9 lines         */
#define F_CALLWIN_TOP       (F_UN_BSE+0x7a)  /* Scroll RA to top             */
#define F_CALLWIN_END       (F_UN_BSE+0x7b)  /* Scroll RA to end             */

/* Event behaviour setup */
#define F_UN_EVENTOVRD      (F_UN_BSE+0x7c)  /* Event behaviour setup        */

/* Call history window */
#define F_UN_CALLHIST       (F_UN_BSE+0x7d)  /* Call history window          */

/* Stats screen */
#define F_UN_STATS          (F_UN_BSE+0x7e)  /* Stats screen */

/* internal BBS local mode */
#define F_UN_BBSLOCALMODE   (F_UN_BSE+0x7f)  /* Enter bbs in local mode      */

/* Recent activity / history window search */
#define F_UN_RAS            (F_UN_BSE+0x80)  /* Recent activity search       */

/* Terminal mode command keys */

#define F_TRM_BS            0xfd00  /* Base for all term mode fns   */
#define F_TERM_SETBAUD      (F_TRM_BS+0x00)  /* Set baudrate on this port    */
#define F_TERM_COMMCONFIG   (F_TRM_BS+0x01)  /* Set data and stop bits       */
#define F_TERM_DOBREAK      (F_TRM_BS+0x02)  /* Toggle break on/off          */
#define F_TERM_CHANGEPORT   (F_TRM_BS+0x03)  /* Change the port we're using  */

#define F_TERM_REINITMODEM  (F_TRM_BS+0x10)  /* Send init string to modem    */
#define F_TERM_HANGUP       (F_TRM_BS+0x11)  /* Drop DTR to hang up modem    */

#define F_TERM_DIALOUT      (F_TRM_BS+0x20)  /* Dial a specified number      */
#define F_TERM_DIALGROUP    (F_TRM_BS+0x21)  /* Dial a hunt group            */
#define F_TERM_POLLBOSS     (F_TRM_BS+0x22)  /* Poll the bossnode            */
#define F_TERM_POLLNODE     (F_TRM_BS+0x23)  /* Poll a specified node        */

#define F_TERM_DOWNLOAD     (F_TRM_BS+0x30)  /* Download a file from remote  */
#define F_TERM_UPLOAD       (F_TRM_BS+0x31)  /* Upload a file to remote      */
#define F_TERM_CAPTUREFILE  (F_TRM_BS+0x32)  /* Open/close text capture file */
#define F_TERM_GATEWAYMODE  (F_TRM_BS+0x33)  /* Don't map keyboard           */

#define F_TERM_MACRO1       (F_TRM_BS+0x41)  /* Transmit user string 1       */
#define F_TERM_MACRO2       (F_TRM_BS+0x42)  /* Transmit user string 2       */
#define F_TERM_MACRO3       (F_TRM_BS+0x43)  /* Transmit user string 3       */
#define F_TERM_MACRO4       (F_TRM_BS+0x44)  /* Transmit user string 4       */
#define F_TERM_MACRO5       (F_TRM_BS+0x45)  /* Transmit user string 5       */
#define F_TERM_MACRO6       (F_TRM_BS+0x46)  /* Transmit user string 6       */
#define F_TERM_MACRO7       (F_TRM_BS+0x47)  /* Transmit user string 7       */
#define F_TERM_MACRO8       (F_TRM_BS+0x48)  /* Transmit user string 8       */

#define F_TERM_MACRO9       (F_TRM_BS+0x49)  /* Transmit user string 9       */

#define F_TERM_CLEARSCREEN  (F_TRM_BS+0x50)  /* Erase the messy screen       */
#define F_TERM_HELPSCREEN   (F_TRM_BS+0x51)  /* Bring up terminal helpscreen */
#define F_TERM_VERSION      (F_TRM_BS+0x52)  /* Display Bink version         */

#define F_TERM_MAILERMODE   (F_TRM_BS+0x60)  /* Exit terminal, enter mailer  */
#define F_TERM_EXITBINK     (F_TRM_BS+0x61)  /* Exit BinkleyTerm             */
#define F_TERM_SHELL        (F_TRM_BS+0x62)  /* Shell to OS command line     */

/* $Id: keyfncs.h,v 1.2 1999/02/27 01:15:11 mr Exp $ */
