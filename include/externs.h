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
 * Filename    : $Source: E:/cvs/btxe/include/externs.h,v $
 * Revision    : $Revision: 1.16 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:08 $
 * State       : $State: Exp $
 *
 * Description : BinkleyTerm Data
 *
 *---------------------------------------------------------------------------*/

extern unsigned int BINK_MAJVERSION;
extern unsigned int BINK_MINVERSION;

extern char *ANNOUNCE;
extern char *xfer_id;

#ifdef _WIN32

extern char ntwin[];

#endif

extern short SB_COLS_M_1;

extern struct _key_fnc_hdr TrmnlKeyFncHdr;
extern struct _key_fnc_hdr UnattendedKeyFncHdr;

extern struct _lang_hdr LangHdr;
extern char **msgtxt;
extern char *mtext[];

extern struct baud_str btypes[];

#ifdef EXTENDED_BAUDRATES

extern int extend_rates;
extern struct baud_str exttypes[];

#endif

extern struct baud_str max_baud;  /* Maximum baud rate */
extern struct baud_str *pbtypes;

// extern int locate_x, locate_y;
extern int port_ptr, original_port;

extern int TaskNumber;          /* unique user-specified id   */
extern int share;               /* if 1, don't uninit FOSSIL  */
extern int no_sharing;          /* if 1, no File Sharing calls */
extern int no_size;             /* If 1, don't compute sizes  */
extern int first_block;         /* 1 = first block/Matrix packet */

extern int serial;              /* serial no, -1 = unregistd. */
extern int fstblklen;
extern int net_params;          /* if 1, net params complete */
extern int un_attended;
extern int command_line_un;
extern int command_line_term;   /* MR 970307 */
extern int overwrite;
extern int redo_dynam;
extern int do_screen_blank;
extern int no_time_display;     /* HJK - 99/01/24 */
extern int screen_blank;
extern int blank_on_key;
extern long blank_time;
extern unsigned int BBStimeout;
extern int loglevel;
extern int debugging_log;
extern int gong_allowed;
extern int MAXport;
extern short pvtnet;
extern int poll_tries;

extern int ring_tries;
extern int ring_wait;

extern int buftmo;
extern unsigned int comm_bits;
extern unsigned int parity;
extern unsigned int stop_bits;
extern unsigned int carrier_mask;
extern unsigned int handshake_mask;

extern int baud;                /* Index to baud table */
extern struct baud_str cur_baud;  /* Current baud rate   */
extern int fax_baud;            /* Index to fax baud   */

extern struct modem_commands modem;
extern int lines_aftercall;
extern int lines_afterconnect;

#ifdef OS_2
#ifdef NEED_OS2COMMS

extern BOOL32 share_port;       /* open port in shared mode */

#endif
#endif

#ifdef _WIN32
#ifdef NT

extern bool share_port;

#endif
#endif

extern char *emsistr[];

extern unsigned int matrix_mask;
extern char *def_system_name, *system_name;
extern char *def_sysop, *sysop;
extern char *def_location, *location;
extern char *mymaxbaud;
extern char *myflags;
extern char *myphone;
extern char *netmail;
extern char *net_info;
extern char *hold_area;
extern char *flag_dir;
extern char *fax_in;
extern char *domain_area;
extern AKA *alias;
extern AKREGION *regions;
extern int assumed;
extern ADDR boss_addr;
extern ADDR my_addr;

extern struct secure CURRENT;

extern struct secure DEFAULT;
extern struct secure KNOWN;
extern struct secure PROT;
extern struct secure SPECIAL;

extern struct req_accum freq_accum;

extern char *domain_name[];
extern char *domain_abbrev[];
extern char *domain_nodelist[];

extern DOMAINKLUDGE *domainkludge;  /* Pointer to domain kludges */
extern int num_domain_kludge;   /* Number of domain kludges  */
extern int size_domain_kludge;  /* Current size of kludge    */

extern byte *Txbuf;             /* Pointer to transmit buffer */
extern byte *Secbuf;            /* Pointer to receive buffer */
extern byte *popbuf;            /* Pointer to popup buffer   */

extern char *native_protocols;
extern char *extern_protocols;
extern char *protocols[];
extern int extern_index;
extern struct prototable protos[];

extern char *ext_mail_string[]; /* String for UUCP, etc. */
extern int lev_ext_mail[];      /* Errorlevel for ext mail */
extern int num_ext_mail;        /* Number of ext mails   */
extern int BBSopt;              /* MR 961122 char* -> int; why waste space ?? */
extern char *BBSbanner;
extern char *BBSreader;
extern char *BOSSphone;
extern char *BOSSpwd;
extern char *download_path;
extern char *BBSnote;
extern char *MAILnote;
extern int EXTopt;              /* MR 961122 char* -> int; why waste space ?? */

#ifdef BINKLEY_SOUNDS

extern char *fnBBSSound;
extern char *fnEXTSound;
extern char *fnMailSound;
extern char *fnFileSound;
extern char *fnStartSound;
extern char *fnFAXSound;

#endif

extern void (_cdecl * mtask_idle) (void);
extern char *BINKpath;
extern char capturename[];
extern char *log_name;
extern char *readlog_name;      /* TJW 960724 */
extern char *config_name;
extern char *swapdir;

// TJW980104: see comment in b_initva.c
// extern char *saved_TZ;

/* MMP 960419 - begin */
extern char *MailFlag;
extern char *NetMailFlag;       /* VRP 990918 */
extern char *ArcMailFlag;       /* VRP 990918 */
extern char *TicFlag;           /* VRP 990918 */

extern time_t diff_trx;
extern long max_diff_trx;
extern int locallogs;

/* MMP 960419 - end */
extern char *shortcostlog;      /* MMP 960422 */
extern long maxbsyage;          /* MMP 960428 */
extern char *spawnnook;         /* MMP 960428 */
extern long connectrate;        /* MMP 960428 */

#ifdef CACHE

extern int cachehold;           /* MMP 960507 */
extern long cachemem;           /* MMP 960513 */
extern int rescanstat;          /* MMP 960513 */

#endif

extern long rescancnt;          /* MMP 960513 */
extern long rescanticks;        /* MMP 960513 */

extern long readflagtime;       /* CEH 990604 */

extern char *scan_list[];
extern int set_loaded;
extern byte user_exits[6];
extern char junk[];
extern int happy_compiler;

extern REGIONP wholewin;
extern REGIONP settingswin;
extern REGIONP historywin;
extern REGIONP holdwin;
extern REGIONP callwin;
extern REGIONP filewin;
extern BOXTYPE box[];

extern int scrllines;

extern char cIPRepChar;
extern PN_TRNS *pn;
extern MDM_TRNS *mm;

extern int resp_count;
extern int max_resp_count;
extern struct resp_str *mdm_resps;
extern int def_resp_count;
extern struct resp_str def_mdm_resps[];

extern FILE *cost_log;
extern char *cost_log_name;
extern int cost_unit;
extern int eurocost;
extern int UseRingingCost;      /* CFS 961225 */
extern int UseModemRing;
extern long cfscallstart;       /* Last non-connect string to/from modem */

extern FILE *status_log;
extern short boxtype;

extern int no_collide;
extern int cursor_col;
extern int cursor_row;
extern char *keys[];
extern char *shells[];
extern char *errlvlshell[];

extern int got_arcmail;
extern int got_packet;
extern int got_mail;
extern int got_fax;
extern int got_tic;             /* VRP 990918 */
extern int mail_finished;
extern int sent_mail;
extern ADDR remote_addr;
extern unsigned short remote_capabilities;
extern unsigned short my_capabilities;
extern ADDR remote_akas[];      /* List of alias addresses of remote  */
extern int num_rakas;           /* How many he's using (EMSI)         */
extern int remote_pickup;

extern ADDR next_addr;
extern int Netmail_Session;
extern char *remote_password;
extern char *BBSesc;
extern char *noBBS;
extern int pickup_ok;
extern int no_pickup;
extern int pickup_all;
extern int no_WaZOO;
extern int no_WaZOO_Session;
extern int no_EMSI;
extern int no_EMSI_Session;
extern int made_request;        /* WaZOO file request flag */
extern int isOriginator;
extern HISTORY start_hist;

extern word ExtMailMask;
extern char *ExtMailAgent;

extern char *request_template;

extern char ext_flags[];

extern char *prodcode[];
extern struct _lang_hdr PrdctHdr;
extern char **PrdctTbl;

extern struct _lang_hdr AnsiHdr;
extern char *AnsiTbl;

#ifdef HAVE_HYDRA

extern ULONG hydra_options;
extern long hydra_txwindow;
extern long hydra_rxwindow;

#endif

extern int no_hydra;
extern int no_janus;
extern int no_dirzap;
extern int no_zapzed;
extern int no_zipzed;
extern int no_dietifna;
extern int on_our_nickel;

extern int direct_zap;

extern int should_show_alive;   /* RDH */

#if (defined USE_WINFOSSIL) || (defined DOS16)  /* MR 970329 */

extern char fossil_buffer[];
extern char out_buffer[];
extern char *fossil_fetch_pointer;
extern char *out_send_pointer;
extern int fossil_count;
extern int out_count;

#endif

#ifdef DOS16                    /* MR 970329 */

extern FOSINFO fossil_info;

#endif

extern char ctrlc_ctr;

extern int no_requests;         /* r. hoerner: char->int! */
extern int who_is_he;

extern int curmudgeon;          /* 1 = Randy Bush            */
extern int small_window;
extern int no_overdrive;
extern int no_resync;
extern int no_sealink;
extern int immed_update;

extern unsigned long janus_baud;

extern ADDR called_addr;
extern int mail_only;
extern int caller;
extern int more_mail;

extern int doing_poll;

extern char *BBSwelcome;
extern char *aftermail;
extern int quickaftermail;      /* MMP 960428 */

extern int find_counter;

#ifdef CACHE

extern DIRBUF dta_str;          /* MMP 960508 */

#else

extern struct FILEINFO dta_str;

#endif

extern int slowmodem;
extern PN_TRNS *pn_head;
extern P_ANUMS anum_head;
extern MDM_TRNS *mm_head;

#ifdef DOS16                    /* MR 970329 */

extern unsigned have_windows;
extern unsigned have_dv;

#endif

extern unsigned winslice;
extern unsigned long lock_baud;
extern int ARQ_lock;            /* If 1, baudrate is locked  */
extern int ARQs;                /* Number of ARQ strings     */
extern char *ARQ[16];           /* Pointers to ARQ strings   */

extern int cMNP;                /* Number of MNP strings     */
extern char *pchMNP[16];        /* Pointers to MNP strings   */

extern char *e_input;           /* dynamic allocation since jan 14 97 */

extern char *IDUNNO_msg;
extern char *local_CEOL;

extern char *wkday[];
extern int _days[];

extern char *nodelist_name;     /* Name of our nodelist      */
extern char *nodelist_base;     /* Name of current nodelist  */
extern char *fri_areadat;
extern char *fri_areaidx;       /* AW980217 name of area.idx   Max 3.0 */
extern char *fri_uclass;        /* AW980217 name of access.dat Max 3.0 */
extern struct _max_classname *fri_max_classname;
extern int fri_areatype;        /* TE 9605.. Maximus or Proboard? */

extern int newnodelist;
extern int version7;
extern int sysopndx;

extern int autobaud;

extern unsigned short found_zone;  /* zone we found             */
extern unsigned short found_net;  /* zone we found             */
extern struct _newnode newnodedes;  /* structure in new list     */
extern char far *node_index;

extern HISTORY hist;
extern BINK_EVENT far *e_ptrs;
extern unsigned char keeperr1, keeperr2;  /* CFS 970309 */
extern int requests_ok;
extern int num_events;
extern int cur_event;
extern int next_event;
extern int got_sched;
extern int noforce;
extern int max_connects;
extern int max_noconnects;
extern int server_mode;
extern int forcexit;
extern char *packer;
extern char *cleanup;
extern char *answerback;
extern char *CIDline;

#ifdef OS_2

extern byte UseCAPICID;
extern char **AddCIDTo;
extern USHORT AddCIDToNumber;
extern char CIDFromCAPI[80];
extern byte UpdateCID;

#endif

#ifdef SMP_API

extern BOOL32 smp;

#endif

extern time_t etm;
extern long file_length;

extern char *XFER_str;

extern char *script_path;       /* Where the script files live */

extern char Rxhdr[];            /* Received header        */
extern char Txhdr[];            /* Transmitted header     */

// extern short Tx_y,Rx_y,tx_line; /* lines for display */

extern long Rxpos;              /* Received file position                       */
extern int Txfcs32;             /* TRUE means send binary frames with 32 bit FCS */
extern int Crc32t;              /* Display flag indicating 32 bit CRC being sent */
extern int Crc32;               /* Display flag indicating 32 bit CRC being rcvd */

extern int Rxtimeout;           /* Tenths of seconds to wait for something      */
extern int Rxframeind;          /* ZBIN ZBIN32,ZHEX type of frame received      */

extern char *Filename;          /* Name of the file being up/downloaded         */

extern word z_size;
extern int fsent;               /* global counter of sent files             */
extern int zmodem_state;        /* zmodem state counter                     */
extern byte Resume_WaZOO;       /* Flags resumption of aborted WaZOO xfer   */
extern char Resume_name[];      /* "Real" name of file being received       */
extern char Resume_info[];      /* File size&time info for rx file          */
extern char Abortlog_name[];    /* Path of WaZOO aborted xfer log           */

extern nfunc nodefunc;          /* Nodelist function to use */
extern ufunc userfunc;          /* Userlist function to use */

extern SB_COLORS colors;

extern int vfossil_installed;

#ifdef DOS16                    // HJK 99/01/19 - Only used in DOS version

extern struct vfossil_hooks vfossil_funcs;

#endif

extern char far *blanks;

extern char *logptr;

extern MAILP mail_top;
extern MAILP next_mail;
extern long next_rescan;
extern time_t last_rescan_tstamp;
extern long waitfor_line;

extern J_TYPESP j_top;
extern J_TYPESP j_next;
extern int janus_OK;
extern char saved_response[];
extern char mdm_reliable[];

extern unsigned long far cr3tab[];

extern unsigned short far crctab[];
extern short SB_ROWS;
extern short SB_COLS;
extern char stat_str[];
extern char script_line[];
extern int need_update;
extern char *BadChars;

#if defined(OS_2) | defined(_WIN32)

extern char *pipename;
extern HSNOOP hsnoop;
extern char *MCPpipename;

#endif

extern int pktrsp;              /* MB 93-12-12  PKTRSP */

extern int freqlongnames;       /* HJK - 99/01/22 */

extern BUF *bufp;               /* TJW 960416 see also buffer.h */

extern long reinittime;         /* mtt 96-04-16 */
extern long readholdtime;       /* mtt 96-04-16 */

extern char SRIF_commandline[]; /* mtt 96-04-28 */
extern char remote_sysop[];
extern char remote_phone[];
extern char remote_site[];
extern char remote_location[];
extern char remote_mailer[];
extern int is_protected;
extern int is_listed;
extern char password[];         /* mtt 96-05-19 */

extern char *version_type;      /* TJW 960421 */
extern char *version_base;      /*  MR 970326 */
extern char *version_modified;  /*  MR 970326 */
extern char *version_opts;      /*  MR 970716 */

extern char *rc_version_type;   /* TJW 970531 */
extern char *rc_version_base;   /* TJW 970531 */
extern char *rc_version_modified;  /* TJW 970531 */

extern char compiledatetime[];  /* MR 980505 */
extern char compilerinfo[];     /* TS/TJW 961013 */

extern char fpnumber[];         /* caller-id for freepoll CE 960421 */

extern char *ModemReject;       /* CE 960505 */
extern int chg_call;            /* CE 960506 */
extern char *modem_aftercallout;  /* CE 960507 */
extern int lines_aftercallout;  /* CE 960507 */
extern int chg_mail;            /* CE 960526 */

#ifdef MAKEDIR

extern int makedir;             /* TJW 960429 */

#endif

extern int do_RH1hydra;         /* TW 960528 */
extern int do_chat;             /* TJW 960705 */

extern char *protpreference;    /* TJW 960720 */
extern char actprotpreference[];  /* TJW 961128 */

extern int costtimecorr_conn,   /* TJW 960721 */
  costtimecorr_sess;            /* TJW 960805 */

extern char *ipc_dir;           /* TJW 960723 */

extern int EMSIbanner;          /* TJW 960728 */

extern long costcps;            /* TJW 960802 */

extern int nohydrachat;         /* TJW 960804 */

extern char *task_dir;          /* TJW 960830 */

extern long chattimer;
extern long next_time;

extern char *title_format;      /* TJW 961013 */

extern struct SBBSBanner **BBSBannerCID;
extern unsigned definedbanners;
extern char *CurrentCID;
extern char *BBSbanner;
extern char *InUseBBSBanner;
extern struct SStringRep **SReplacements;
extern unsigned definedreplacements;
extern int nologhundredths;

#ifdef DOS16

extern int noansitrash;         /*  TS961214    */
extern int cshape;              /*  TS961215    */

#endif

extern int SIOMode;             /* CFS 9612 */

extern PKTREP PKTStats;         /* CFS 970209 */
extern int DoStats;

extern char *local_inbound;     /* CEH 990612 */

extern char *HydraMessage;      /* mtt 970115 */

/* r. hoerner included those ----------- */
extern PHONE *phone1;                 /**/

extern BINK_SAVEP popped_up;          /**/
extern NODESTRUC *knownnode;          /**/

extern char *port_device;             /**/
extern char lastfound_SysopName[];    /**/

extern char *emsilog   ;              /**/
extern char *passwordfile;            /**/

extern char *bink_pipe;               /**/
extern long assumebaud;               /**/

extern long ReDialTime;               /**/
extern long s_startpos;               /**/

extern long r_startpos;               /**/
extern int exact_mdm_type;            /**/

extern int  debug_emsi ;              /**/
extern int no_delay;                  /**/

extern int  noMNPFilter;              /**/
extern int binkdir;                   /**/

extern int  hidedomains;              /**/
extern int rescanforced;              /**/

extern int  nofancystrings;           /**/
extern int inzoomwindow;              /**/

extern int  MaxRingWait;              /**/
extern int show_password;             /**/

extern int  noWildcards;              /**/
extern int DelBadCall;                /**/

extern char *eventfile;               /**/
extern int initialized;               /**/

extern short session_method;          /**/
extern int cFosCost;                  /**/

extern char HAS[],HAS_NOT[];          /**/
extern int DrawClock;                 /**/

extern int  clock_minute;             /**/
extern char clock_char;               /**/

extern char clock_color;              /**/
extern int smart_bidi;                /**/

extern int  smart_bidi_out;           /**/
/*--------------------------------------*/

extern ONLINE_TIME *online_time1;  /* VRP 990820 */
extern HIDDEN *hidden1;         /* VRP 990830 */
extern short weekdays[7];       /* VRP 990829 */
extern int SendHoldIfCaller;    /* VRP 990824 */
extern int RestorePoll;         /* VRP 990826 */
extern char RestorePollFlavour[2];  /* VRP 990826 */
extern int SaveExtSession;      /* VRP 990920 */


#ifdef OS_2

extern byte MonitorCFOS;
extern int ReqChannelChange;    /* CFS 970320 */
extern byte ReqHardwareReset;   /* CFS 970320 */

#endif

extern char *addBINKpath;       /* MR 970212 */
extern int show_modemwin;       /* MR 970215 */

extern int no_tranx;            /* JNK 970312 */

extern byte ReqSkipFile;        /* CFS 970323 */

#ifdef BBS_INTERNAL             /* AW 970531 made #ifdef instead of #if (watcom complains) */

void bbsinternal (int locmode);

#endif

extern char HistoryFileName[PATHLEN];  /* CFS 970411 - AG 990119 */
extern long realspeed;

extern char RemCommandBuf[];
extern char LocCommandBuf[];

extern byte autochatopen;
extern char *ChatLogDir;
extern FILE *ChatLogHandle;

extern RANDOM_SYSTEM **RandomSystemInfo;
extern word RandomGroups;

extern int HistMode;
extern LSTATS *lstats;

extern long debugflags;         /* TJW970715 temporary debug flags */

extern char *TZ;                /* TJW970715 ptr to TZ environment */
extern long tzutc_timer;        /* AW 980301 for timezone calculations */

extern time_t scdcompiletime;   /* TJW971031 */

extern int check_path;          /* CE 971218 */
extern int check_path_errorlevel;  /* CE 971218 */
extern int check_path_time;     /* CE 971218 */
extern int AKA_matching;        /* CE 971230 */
extern int SyncHydraSession;    /* CE 980118 */
extern int NoModem;             /* CE 989119 */
extern int bttask;              /* CE 980227 */

extern long rejectignoresize;   /* JH 980817 */

/* $Id: externs.h,v 1.16 1999/09/27 20:51:08 mr Exp $ */
