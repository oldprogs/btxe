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
 * Filename    : $Source: E:/cvs/btxe/include/prototyp.h,v $
 * Revision    : $Revision: 1.13 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:10 $
 * State       : $State: Exp $
 *
 * Description : Prototype definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

#ifdef NEED_WRITE_ANSI

void WRITE_ANSI (char);

#endif

#ifdef NEED_CPUTS

int _cdecl cputs (const char *);

#endif

#ifdef NEED_PUTCH

int _cdecl putch (int);

#endif

#ifdef NEED_CPRINTF

int _cdecl cprintf (const char *,...);

#endif

#ifdef NEED_KBHIT

int kbhit (void);

#endif

#ifdef NEED_MKTEMP

extern char *mktemp (char *);

#endif

#ifdef BINKLEY_SOUNDS

extern void Make_Sound (char *);

#endif

#ifdef DOS16

int _cdecl real_flush (int);

#endif

struct _key_fnc_hdr *KbMapSet (struct _key_fnc_hdr *KeyFncHdr);
void com_kick (void);
void b_sbinit (void);
void b_sbexit (void);
void sb_clean (void);
void sb_dirty (void);
void sb_init (void);
void sb_exit (void);
char *stripcolors (char *);
int sb_input_chars (REGIONP, short, short, char *, short);
int sb_input_fname (REGIONP, short, short, char *, short);  /* RDH */
REGIONP sb_new (short, short, short, short);
int sb_popup (short, short, short, short, int (*)(BINK_SAVEP, int), int);
int sb_putc (REGIONP, short);
int sb_putc_att (REGIONP, short, short);
int sb_putc_abs (REGIONP, short);
void sb_restore (BINK_SAVEP);
BINK_SAVEP sb_save (short, short, short, short);
void sb_show (void);
REGIONP sb_new_win (short, short, short, short,  /* sb_new     */
                    short, short,  /* sb_fill    */
                    short, short,  /* sb_box     */
                    char *);    /* sb_caption */

BINK_SAVEP sb_hide_popup (BINK_SAVEP);  /* r. hoerner */
void sb_show_popup (BINK_SAVEP, BINK_SAVEP);  /* r. hoerner */

unsigned short pascal KbRemap (unsigned short);
void sb_box (REGIONP, short, short);
void sb_caption (REGIONP, char *, unsigned short);
void sb_fill (REGIONP, short, short);
void sb_filla (REGIONP, short);
void sb_fillc (REGIONP, short);
void sb_clear (REGIONP win);
void sb_move (REGIONP, short, short);
void sb_move_abs (REGIONP, short, short);
void sb_puts (REGIONP, char *);
void sb_puts_abs (REGIONP, char *);
void sb_move_puts (REGIONP, short, short, char *);
void sb_move_puts_abs (REGIONP, short, short, char *);

#if 0                           /* unused */

unsigned char sb_ra (REGIONP, short, short);
unsigned char sb_rc (REGIONP, short, short);
unsigned int sb_rca (REGIONP, short, short);

#endif

void sb_scrl (REGIONP, short);
void sb_wa (REGIONP, short, short);
void sb_wa_abs (REGIONP, short, short);
void sb_wc (REGIONP, short, short);
void sb_wc_abs (REGIONP, short, short);
void sb_wca (REGIONP, short, short, short);
void sb_wca_abs (REGIONP, short, short, short);

int Do_Get (BINK_SAVEP, int);
int Do_Send (BINK_SAVEP, int);
int Do_Kill (BINK_SAVEP, int);
int Do_Poll (BINK_SAVEP, int);
int Do_Poll_Packet (BINK_SAVEP, int);
int Do_Zoom (BINK_SAVEP, int);
int Do_Event (BINK_SAVEP, int);
int Do_CallHistory (BINK_SAVEP, int);
int Do_Stats (BINK_SAVEP, int);
int Do_Ras (BINK_SAVEP, int);

int Upload (char *, int, char *);
int Download (char *, int, char *);

int set_prior (int);
int load_language (char *);     /* MMP 960420 Added exe argument */
void b_initvars (void);
void b_defaultvars (void);
void b_exitproc (void);
int got_ESC (void);
void BBSexit (void);
long timerset (unsigned int);
bool timeup (long);
bool longtimeup (long t);       /* r. hoerner */
long longtimerset (long t);     /* r. hoerner */
void dostime (int *, int *, int *, int *, int *);
void _cdecl time_release (void);
void fossil_gotoxy (int, int);
int fossil_wherex (void);
int fossil_wherey (void);
int get_key (void);
unsigned _cdecl dos_box_active (void);
unsigned _cdecl windows_active (void);
unsigned _cdecl dv_get_version (void);

#ifdef DOS16

void dos_break_off (void);
void fossil_ver (void);

#endif

void mtask_find (void);
void _cdecl windows_pause (void);
void _cdecl dv_pause (void);
void _cdecl msdos_pause (void);
void _cdecl os2_pause (void);
void _cdecl status_line (char *,...);
int time_to_next (int);
void write_sched (void);
void close_up (int);
void scr_printf (char *);
int SendBanner (char far *, int);
void opening_banner (void);
void mailer_banner (void);
void UUCPexit (int, int);
void come_back (void);
void fancy_str (char *);
int got_error (char *, char *);
int dfind (struct FILEINFO *, char *, int);
int n_frproc (char *, int (*)(char *), int (*)(long));
void Make_Response (char *, int);
int do_FLOfile (char *, int (*)(char *));
void fidouser (char *, ADDR *);
void opususer (char *, ADDR *);
void main_help (void);
void mailer_help (void);
int n_password (ADDR *, char *, char *, int *);
int n_getpassword (ADDR *);
int nodefind (ADDR *, int);
void do_extern (char *, int, char *);
int dexists (char *);
int do_script (char *);
void timer (int);

/* modem commands (mdm_proc.c) */

void modem_init (void);
void modem_answer (void);
void modem_busy (void);
void modem_reject (void);
void modem_hangup (void);
void modem_aftercall (void);
long modem_getcost (long);
int modem_response (int);       /* only used in b_script.c and mailroot.c */
void mdm_cmd_string (char *, int);  /* only used in b_script.c */
void mdm_cmd_char (int);        /* only used in b_script.c */
void send_break (int);          /* only used in b_script.c and b_termov.c */
char *get_response (long, int, int *);
int set_baud (unsigned long, int);
void program_baud (void);

int list_search (void);
int checklist (void);
int nodeproc (char *);
void gong (void);
void b_init (void);
MISSION_ERR b_session (int, CALLHIST *);  /* CFS 970411 - added CALLHIST *) */
void b_session_cleanup (int);   /* TJW 960805 *//* VRP 990826 */
void do_status (int, int *);    /* MR 971223 added int* */
void do_session (int);
void do_protocol (int);
int CallerSendSync (void);
int CalledRecvSync (void);

int Whack_CR (void);
void last_type (int, ADDR *);

MISSION_ERR FTSC_receiver (int);
MISSION_ERR FTSC_sender (int);

MISSION_ERR WaZOO (int);
MISSION_ERR send_WaZOO (short);

int YooHoo_Receiver (void);
int YooHoo_Sender (void);

void EMSI (int);
int EMSI_Sender (void);
int EMSI_Receiver (void);
int send_EMSI (void);
int send_Hello (int);

int get_Zmodem (char *, FILE *);
int Send_Zmodem (char *, char *, int, int);

MISSION_ERR Janus (void);

int respond_to_file_requests (int, int (*)(char *), int (*)(long));
void invent_pkt_name (char *);
void set_xy (char *);
char *receive_file (char *, char *, char);

int read_sched (void);
KWD_ERR parse_config (char *);

char *ctl_string (char *);
char *ctl_slash_string (int, char *);
int parse (char *, char far *[]);

void bottom_line (int);
void top_line (void);
void do_today (int doshowflag);
void boss_mail (int);
int unattended (void);
int get_number (char *);
int try_1_connect (char *, ADDRP);  /* SM 961110 */
void try_2_connect (char *, ADDRP);  /* SM 961110 */
void change_prompt (void);
int do_mail (ADDR *, int);
void ansi_map (unsigned int);
char *zalloc (void);
int xfermdm7 (char *);
int recvmdm7 (char *);
MAINTYPE main (int, char **);
int opusfind (ADDR *, int);
int ver7find (ADDR *, int);
int TBBSLookup (ADDR *, int);
char *skip_blanks (char *);
char *skip_to_blank (char *);
char *skip_to_word (char *);
KWD_ERR parse_event (char *);
void SEA_sendreq (void);
int try_sealink (void);
int SEA_recvreq (void);
void clear_filetransfer (void);
long throughput (int, int, unsigned long);
char *numdisp (long, int);
void show_os_memfree (void);
int is_arcmail (char *, int);
void send_can (void);
int bad_call (ADDR *, int);
void receive_exit (void);
void xmit_sameplace (void);

void find_event (void);
void errl_exit_shell (int, char *, char *);  /* MR 961123 */
void errl_exit (int);
void errl_shell (int);
long random_time (int);
int handle_inbound_mail (int);
int xmit_reset (int, int);
int xmit_next (ADDR *);
void kill_bad (void);

/* AG 990119 removed: void put_up_time (void); */
void adios (int);
void big_pause (int);
long zfree (char *);
void show_debug_name (char *);
char *check_netfile (char *);
void unique_name (char *);
void write_sched (void);

/* AW 980111 removed: void transfer_message (unsigned int, char *, ... );  / * 8/1/97 r.hoerner */
/* removed: 1/9/97 r.hoerner : void z_message(char *);  */
/* AW 980111 removed: void Z_transfer_msg_init(int, char *, char *, long, long); / * r.hoerner */
/* AW 980111 removed: void Z_message_transfer_new(int, long);                    / * r.hoerner */
/* AW 980111 removed: void Z_message_transfer(int, long, long);                  / * r.hoerner */
/* AW 980111 moved local zmodem prototypes to zmodem.c:
   int Z_GetByte (int);
   void Z_PutString (unsigned char *);
   void Z_SendHexHeader (unsigned int, unsigned char *);
   int Z_GetHeader (unsigned char *);
   int Z_GetZDL (void);
   void Z_PutLongIntoHeader (long);
   void z_log (char *);
   void show_loc (unsigned long, unsigned int);
   void Z_UncorkTransmitter (void);
 */

char *add_backslash (char *);
char *delete_backslash (char *);
char *store_TZ (void);
void vfossil_init (void);
void vfossil_close (void);
void vfossil_cursor (int);
MAILP find_mail (ADDR *);
void xmit_window (MAILP);
void xmit_delete (void);
char *HoldAreaNameMunge (ADDR *);
int LoadScanList (int, int);
void swapper (char far *, char far *, int, char far *, unsigned);
void b_spawn (char *);
int do_dial_strings (int);
void exit_DTR (void);
void delete_old_file (char *);
long cost_of_call (int, long, long, CALLHIST * Data);
void update_files (int, char *, long, long, int *);
void screen_clear (void);
void clear_eol (void);
void log_product (int, int, int);
int b_term (void);
int set_up_outbound (int);
int is_user (char *);
int next_minute (void);
void can_Janus (unsigned long, char *);
int Modem7_Receive_File (char *);
int Modem7_Send_File (char *);
int Batch_Receive (char *);
int Batch_Send (char *);
int Xmodem_Send_File (char *, char *);
int Xmodem_Receive_File (char *, char *);
int Internal_Xmodem_Receive_File (char *, char *);
int SEAlink_Send_File (char *, char *);
int Internal_SEAlink_Send_File (char *, char *);
int Telink_Send_File (char *, char *);
int Internal_Telink_Send_File (char *, char *);
unsigned short crc_block (unsigned char *, int);
void Data_Check (XMDATAP, int);
void wait_for_key (int);
void Get_Block (XMARGSP);
int Xmodem_Send (int, int, long);
void Send_Block (XMARGSP);
int calc_window (void);
int Receive_Resync (long *);
void Xmodem_Error (char *, long);
int Header_in_data (unsigned char *);
void Find_Char (int);
int Xmodem_Receive (int, char *);
void Send_Response (int, int *, int *, int *, long, int, long);
void Set_Up_Telink (XMDATAP, TLDATAP, long *);
void End_Telink (int, TLDATAP, XMDATAP, char *, int);
void Wait_Clear (int);
void Send_Resync (long, int);
int state_machine (STATEP, void *, int);
int Bark_Request (char *, char *, char *, char *);
void Build_Bark_Packet (BARKARGSP);
int Receive_Bark_Packet (BARKARGSP);
int Batch_Xmodem_Receive_File (char *, char *);
int Internal_Batch_Xmodem_Receive_File (char *, char *);
long Set_Up_Restart (XMARGSP);
void Finish_Xmodem_Receive (XMARGSP);
void Get_Telink_Info (XMARGSP);
int Read_Block (XMARGSP);
void Check_ACKNAK (XMARGSP);
void show_block (long, char *, XMARGSP);
int check_failed (char *, char *, char *, char *);
void add_abort (char *, char *, char *, char *, char *);
void remove_abort (char *, char *);
void Build_Header_Block (XMARGSP, char);
void show_sending_blocks (XMARGSP);
char *show_num (XMARGSP, long);
long remaining (long, long);    /* remaining time to transfer x byte */
void estimate_time (int, long, long);
void elapse_time (int show, int where);
int parse_address (char *, ADDR *);
int find_address (char *, ADDR *);
char *Hex_Addr_Str (ADDR *);
char *Full_Addr_Str (ADDR *);
char *add_domain (char *);
char *find_domain (char *);
int flag_file (int, ADDR *, int);
int term_overlay (short);
void write_stats (void);
void list_next_event (void);
int machine (STATEP, void *, int);
int share_open (char *, int, int);
FILE *share_fopen (char *, char *, int);
void forcexitcheck (void);
int forcerescancheck (void);
int show_alive (void);
void no_longer_alive (void);    /* MR 961123 */
int faxreceive (int, CALLHIST *);  /* AW 970531 - added CALLHIST */
int SquishScan (char *);

/* TJW 961010 IPC interface */
#ifdef OS_2                     /* else these are #defined empty to save space */

void IPC_Ping (void);
void IPC_Open (void);
void IPC_Close (void);

#endif

void IPC_SetStatus (char *);
void show_session (char *, ADDRP);  /* TJW 970128 show session in MCP / task list / IPC */

/* MMP 960419 - b_logs.c */
/* AW 980118 not needed globally *
 * char *status_log_path(void);  *
 * char *cost_log_path(void);    */

void timesync (void);

void openlogs (int newlineflag);  /* TJW 960527 */
void closelogs (void);          /* TJW 960527 */
void movelogs (void);
void binkexit (int);

#ifdef CACHE

/* MMP 960513 - cache.c */
void dirinit (DIRBUF *, char *);
int dirread (DIRBUF *);
void dirquit (DIRBUF *, int);
int cachedump (char *);
void cacheflush (void);
int cachestat (char *, struct stat *, DIRECTORY **, DIRENTRY **, int, long);

#else

int cachestat (char *, struct stat *, int, long);

#endif

long netsize (MAILP, char *, unsigned short AttachType);  /* always, even if no #cache (r.hoerner) */

int freepoll (ADDRP, FPP, time_t);  /* CE 960423 */

int invoke_SRIF (int, int (*)(char *), int (*)(long));  /* mtt 960429 */
void change_call_to_normal (ADDRP addr);  /* CE 960506 */

void b_sbinit (void);

void heapchk (int);

void timedisplay (void);        /* TJW 960624 */

long totalsize (ADDRP addr, unsigned short AttachType);  /* get size of all mail for addr */

void chat_init (void);          /* TJW 960704 chat functions */
void chat_exit (void);
void chat_put_local (char *);
void chat_put_remote (char *);
void chat_put (REGIONP, char *);
void chat_prepare (void);

void redraw_callwin (void);

BINK_EVENTP find_costevent (long);  /* TJW 960721 */

void fossil_announce (int);     /*  TS 960725   */

void read_in_log (void);        /* TJW 960724 */

void log_screen (char *, char *);  /* TJW 960724 */

void calc_cost (int, long, long, long *, long *, long *, word *, char **, word);

char mail_stat (MAILP);         /* TJW 960731 */

void global_rescan (void);      /* TJW 960731 */

long custom_date (struct tm *); /* TJW 960804 */

void cleanup_req (char *);      /* TJW 960808 */

int eval (char *);              /* TJW 960830 */

void append_pkt (REGIONP, unsigned int, char *, char *);

BOOL keyabort (void);

#ifdef ANSI_TIME_T

int ansi_stat (const char *fname, struct stat *buffer);
int ansi_fstat (int handle, struct stat *buffer);
time_t ansi_time (time_t * timeptr);
time_t ansi_mktime (struct tm *time);
struct tm *ansi_localtime (const time_t * timeval);
struct tm *ansi_gmtime (const time_t * timeval);
char *ansi_ctime (const time_t * timeval);
int ansi_utime (char *fname, struct utimbuf *buf);

#endif

char *strstrci (char *string, char *substring);  /* CFS 961020 */
void set_banner (void);         /* CFS 961020 */
void parse_stringrep (char *par, int);  /* CFS 961020 */
void perform_rep (char *, int); /* CFS 970328 */

char *Get_CallerID (char *);    /* MR 970311 */
int rejected_call (char *);     /* MR 970324 */
void callback (ADDRP CBAddress);  /* CFS 961105 */

/* Semaphore operations (file system semaphores) */
int sem_tas (char *dir, char *base_fname, char *ext_fname);  /* TJW 961028 */
int sem_clr (char *dir, char *base_fname, char *ext_fname);  /* TJW 961028 */

void show_cs_port (long sbaud, bool mode);  /*  TS 961122   */

#ifdef DOS16

int _cdecl getbiosscreenrows (void);  /*  TS 961214   *//* MR 961224 */
int _cdecl get_cshape (void);   /*  TS 961215   *//*     added */
void _cdecl set_cshape (int i); /*  TS 961215   *//*    _cdecl */
void _cdecl hide_cursor (int i);  /*  TS 961215   */
void _cdecl clear_screen (void);  /* r.hoerner 15 jan 97 */
void _cdecl clear_to_eol (void);  /* r.hoerner 15 jan 97 */
unsigned _cdecl dos_largest_free_block (void);  /*  TS 970524   */

#endif

void log_emsi (char, char *, int);  /* R. Hoerner */
int QueryNodeFlag (int, unsigned long, ADDRP);  /* R. Hoerner */
char *PasswordFromFile (ADDRP); /* r. hoerner */
int ExecSession (unsigned char *);  /* r. hoerner */
int GetAltPhone (char *, ADDRP);
void GetAltFidoFlag (word *, ADDRP);
void GetAltModemflag (word *, ADDRP);
int GetHiddenInfo (int, int, struct _newnode *);
int GetAltOnLineTime (short *, short *, ADDRP);  /* VRP 990820 */
int IsOnLine (short, short);    /* VRP 990820 */

int mail_addsend (ADDRP, char *, char *);  /* VRP 990826 */

int SameAddress (ADDRP, ADDRP);
int invalid_address (ADDRP);
void show_nodes_name (char *);
int update_clock (short);
void show_our_capabilities (char *);
void display_life (void);

/* --------------------------------------------------------------------- */

#ifdef OS_2

VOID LoadOS2DLLs (VOID);
VOID UnLoadOS2DLLs (VOID);
VOID SetWindowTitle (char *);
BOOL IsPipeRunning (HWND *);
VOID SendToDlg (PSZ, ULONG, ULONG, ULONG);  /* binkpipe */

# ifdef __WATCOMC__

APIRET BtQueryProcAddr (HMODULE hm, ULONG ulOrd, PCSZ procname, PFN * pProcAdr);

# else
#  define BtQueryProcAddr DosQueryProcAddr
# endif

#else

# ifdef _WIN32                  // HJK 98/06/13

VOID SetWindowTitle (char *);

# endif

void SendToDlg (char *, long, long, long);

#endif

/* --------------------------------------------------------------------- */
/* Common routines of JANUS.C and HYDRA.C                                */
/* --------------------------------------------------------------------- */
/* removed 9/1/97 r.hoerner: void j_message (unsigned int, char *,...);  */

void j_status (char *,...);
void j_msgend (short);
void xfer_summary (char *, char *, long *, short);
void update_status (long *, long *, long, long *, short);
long through (long *, long *);

/* --------------------------------------------------------------------- */

#if defined(OS_2) | defined(_WIN32)

void snoop_open (PSZ);
void snoop_close (void);
void SnoopWrite (HSNOOP, PSZ);

#endif

/* --------------------------------------------------------------------- */

#ifdef SMP_API

BOOL32 smp_init (void);
BOOL32 query_cpu (double *idle, double *busy, double *intr);

#endif

/* --------------------------------------------------------------------- */

void ctrl_modemwin (int);       /* MR 970215 */
void print2modemwin (short, short);  /* MR 970216 */

char *fname8p3 (char *longpath);  /* TJW 970309 */

NODESTRUC *QueryNodeStruct (ADDRP, int);  /* MR 970321 */
void copy_address (ADDRP, ADDRP);  /* r. hoerner */

/* --------------------------------------------------------------------- */

#ifdef OS_2

int CAPIInit (char *, char *);
void CAPIStop (void);
int CFOSMonitorStart (void);
int CFOSResetHardware (void);
word CFOSAddOrRemoveChannel (int what);

#endif

int CommonProtocolKeyCheck (word Key);
int SaveCallInfo (CALLHIST data);
unsigned short get_filetype (char *file);

time_t age_seconds (time_t);
char *strntcpy (char *dst, char *src, size_t size);

void termwin_init (void);
void termwin_exit (void);

#ifdef BBS_INTERNAL

void bbsinternal (int);

#endif

#ifdef __EMX__

int _ungetch (int c);

#endif

void do_random_system (void);
RANDOM_SYSTEM *QueryRandomGroup (char *name, int create);

int islocalregion (word Zone, word Net);

/* TE011097, moved from vt100.c */
void vt_install (void (*)(), void (*)(), REGIONP);
void vt_init (int, int, int, int, int);
void vt_pinit (REGIONP, int, int);
void vt_set (int, int, FILE *, int, int, int, int);
void vt_out (int);
void vt_send (int ch);

void checkpath (void);
void forcebttaskcheck (void);

// TJW: some special "time" functions without TZ time shifts
int unix_utime_noshift (char *fname, struct utimbuf *buf);
int unix_stat_noshift (const char *fname, struct stat *buffer);
time_t unix_mktime_noshift (struct tm *time);

// time_t unix_rshifttime(time_t t);

void get_PrivKey (word * ppriv, dword * pkeys, char *PrivKey);
KWD_ERR cfg_MaxPrm (int, char *);
KWD_ERR cfg_CopyString (int keyword, char **p, char *c);

long get_bink_timezone (void);
int ModemType_matches (byte, byte);
MDM_TRNS *find_mdm_trans (byte);
char *get_modem_flags (char *);

#ifdef USE_FIDOCONFIG

int read_fidoconfig ();
char *PasswordFromFidoconfig (ADDRP);
void setpathsfromfidoconfig ();

#endif

/* $Id: prototyp.h,v 1.13 1999/09/27 20:51:10 mr Exp $ */
