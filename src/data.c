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
 * Filename    : $Source: E:/cvs/btxe/src/data.c,v $
 * Revision    : $Revision: 1.18 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:23 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm Data
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#ifdef __BORLANDC__
extern unsigned _stklen = 32768U;

#endif

unsigned int BINK_MAJVERSION = PRDCT_MAJOR;
unsigned int BINK_MINVERSION = PRDCT_MINOR;

/* TJW 960415 ALPHA/BETA version numbering to version.c ! */

char *ANNOUNCE;
char *xfer_id = ""
#if defined OS_2
"BinkOS2"
#elif defined _WIN32
"BinkW32"
#else
PRD_SH_PRTY
#endif
" " PRDCT_VRSN;

#ifdef _WIN32

char ntwin[256];

#endif

short SB_COLS_M_1;              /* TJW 960723 removed = 0; */
short SB_ROWS;
short SB_COLS;

/* TJW 960428 2 more! */
/* MR 970310 again 2 more! */
SB_COLORS colors =
{
  7, 7, 7, 7, 7, 7, 112, 7, 7, 7, 7, 7
};

struct _key_fnc_hdr TrmnlKeyFncHdr;
struct _key_fnc_hdr UnattendedKeyFncHdr;

char **msgtxt;
struct _lang_hdr LangHdr =
{
  0, 0
};

char *mtext[] =
{
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char *wkday[] =
{
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};


struct baud_str btypes[] =
{
#ifdef BAUD_150
  {150, BAUD_150},
#endif
  {300, BAUD_300},
#ifdef BAUD_600
  {600, BAUD_600},
#endif
  {1200, BAUD_1200},
  {2400, BAUD_2400},
  {4800, BAUD_4800},
  {9600, BAUD_9600},
  {19200, BAUD_19200},
  {38400UL, BAUD_38400},
#ifdef BAUD_57600
  {57600UL, BAUD_57600},
#endif
#ifdef BAUD_115200
  {115200UL, BAUD_115200},
#endif
  {0, 0}
};

#ifdef EXTENDED_BAUDRATES

int extend_rates = 0;

struct baud_str exttypes[] =
{
#ifdef BAUD_EXT_150
  {150, BAUD_EXT_150},
#endif
#ifdef BAUD_EXT_300
  {300, BAUD_EXT_300},
#endif
#ifdef BAUD_EXT_600
  {600, BAUD_EXT_600},
#endif
#ifdef BAUD_EXT_1200
  {1200, BAUD_EXT_1200},
#endif
#ifdef BAUD_EXT_2400
  {2400, BAUD_EXT_2400},
#endif
#ifdef BAUD_EXT_4800
  {4800, BAUD_EXT_4800},
#endif
#ifdef BAUD_EXT_9600
  {9600, BAUD_EXT_9600},
#endif
#ifdef BAUD_EXT_19200
  {19200, BAUD_EXT_19200},
#endif
#ifdef BAUD_EXT_38400
  {38400UL, BAUD_EXT_38400},
#endif
#ifdef BAUD_EXT_57600
  {57600UL, BAUD_EXT_57600},
#endif
#ifdef BAUD_EXT_76800
  {76800UL, BAUD_EXT_76800},
#endif
#ifdef BAUD_EXT_115200
  {115200UL, BAUD_EXT_115200},
#endif
  {0, 0}
};

#endif /* EXTENDED_BAUDRATES */

struct baud_str max_baud =
{
  2400, BAUD_2400
};                              /* Max baud value */

struct baud_str *pbtypes = btypes;

// int locate_x, locate_y;

int port_ptr, original_port;

int TaskNumber = 0;             /* unique user-specified id   */
int share = SHARE_DEFAULT;      /* if 1, don't uninit FOSSIL  */
int no_sharing = 0;             /* if 1, no File Sharing calls */
int no_size = 0;
int first_block;                /* 1 = first block/Matrix packet */

int serial = -1;                /* -1 is unregistered         */
int fstblklen = 0;              /* Start Zmodem packet size   */
int net_params = 0;
int un_attended = 0;
int command_line_un = 0;        /* MR 970307 moved '= 0' from b_initva */
int command_line_term = 0;      /* MR 970307 added */
int overwrite = 0;
int redo_dynam = 0;
int do_screen_blank = 0;
int no_time_display = 0;
int screen_blank = 0;
int blank_on_key = 1;
long blank_time = PER_MINUTE;
unsigned int BBStimeout = 20 * PER_SECOND;
int loglevel = 6;
int debugging_log = 0;
int gong_allowed = 0;

#ifndef __unix__

int MAXport = 2;

#else

int MAXport = 0;

#endif

int poll_tries = 500;           /* How many tries in a poll? */

int ring_tries = 4;             /* How many rings before fail   */
int ring_wait = 1;              /* Incoming rings before answer */

int buftmo = PER_MINUTE;        /* 60 second timeout         */
unsigned int comm_bits = BITS_8;  /* Default data bits = 8     */
unsigned int parity = NO_PARITY;  /* Default parity = NONE     */
unsigned int stop_bits = STOP_1;  /* Default stop bits = 1     */
unsigned int carrier_mask = 0x80;  /* Default carrier bit = 80H */
unsigned int handshake_mask = USE_XON | USE_CTS;  /* Force CTS/RTS flow control */

int baud;                       /* Index to baud table       */
struct baud_str cur_baud;       /* Current baud rate         */
int fax_baud = -1;              /* Index of baud for faxes   */

struct modem_commands modem;    /* MR 970326 */

int lines_aftercall = 0;
int lines_afterconnect = 0;
int lines_aftercallout = 0;     /* CE 960507 */

char *CIDline = NULL;           /* String that prefixes CID (example: /ID=) */

#ifdef OS_2

BOOL32 share_port = FALSE;      /* open port in shared mode  */

#endif

#ifdef _WIN32

bool share_port = FALSE;

#endif

char *emsistr[] =
{
  "**EMSI_REQA77E",
  "**EMSI_DAT",
  "**EMSI_HBTEAEE",
  "**EMSI_NAKEEC3",
  "**EMSI_ACKA490",
  "**EMSI_INQC816",
  "**EMSI_CLIFA8C"
};

unsigned int matrix_mask = TAKE_REQ;  /* Netmail session abilities */

/* Default system info fields */
char *def_system_name = NULL;   /* Name of system to send    */
char *def_sysop = NULL;         /* Name of sysop for yoohoo  */
char *def_location = NULL;      /* Name of location for EMSI */

/* Current (=used for current connection) system info fields */
char *system_name = NULL;       /* Name of system to send    */
char *sysop = NULL;             /* Name of sysop for yoohoo  */
char *location = NULL;          /* Name of location for EMSI */

char *myphone = NULL;           /* Own phonenumber for EMSI  */
char *myflags = NULL;           /* Nodelistflags for EMSI    */
char *mymaxbaud = NULL;         /* Max. baudrate for EMSI    */
char *netmail = NULL;           /* Net Mail directory        */
char *net_info = NULL;          /* Location of nodelist, etc */
char *hold_area = NULL;         /* Path to outbound dir      */
char *flag_dir = NULL;          /* Where to find INMAIL.$$$  */
char *fax_in = NULL;            /* Where to store inbound fax */
char *domain_area = NULL;       /* Where to put domain name  */

/* CEH 19990514: this is no longer needed */
/* char *domain_loc = NULL; */
/* Where to put domain name  */

AKA *alias = NULL;              /* List of alias addresses   */
AKREGION *regions = NULL;       /* List of our regions       */
int assumed = 0;                /* Alias we're now assuming  */

ADDR boss_addr;                 /* Bossnode address */
ADDR my_addr;                   /* Our address      */
ADDR remote_addr;
ADDR remote_akas[ALIAS_CNT + 1];  /* List of alias addresses of remote */
ADDR next_addr;
ADDR called_addr;

struct secure CURRENT;          /* Current path, quotas, etc */
struct secure DEFAULT;          /* Default paths, etc        */
struct secure KNOWN;            /* Sec for KNOWN address     */
struct secure PROT;             /* Sec for PW-PR address     */
struct secure SPECIAL;          /* Sec for special Inbound   */

struct req_accum freq_accum;    /* Accumulator for freq stats */

char *domain_name[DOMAINS];     /* Known domain names        */
char *domain_abbrev[DOMAINS];   /* Known domain name abbrev  */
char *domain_nodelist[DOMAINS]; /* Known domain nodelists    */

DOMAINKLUDGE *domainkludge;     /* Pointer to domain kludges */
int num_domain_kludge = 0;      /* Number of domain kludges  */
int size_domain_kludge = 0;     /* Current size of kludge    */

BUF *bufp;                      /* TJW 960416 pointer to buffer union */

byte *Txbuf;                    /* Pointer to xmit buffer    */

char *native_protocols = "XZTSH";  /* Protocols we support        */

                                  /* TJW 960513: added Hydra key */
char *extern_protocols = NULL;  /* Protocols we can call     */
char *protocols[MAX_EXTERN];    /* external file protocol    */
int extern_index = 0;           /* external proto index      */
struct prototable protos[MAX_EXTERN];  /* Table for execution */

char *ext_mail_string[16];      /* String for UUCP, etc.     */
int lev_ext_mail[16];           /* Errorlevel for extmail    */
int num_ext_mail;               /* Number of ext mails       */

int BBSopt = OPT_NONE;          /* MR 961122 char* -> int BBS command options */
char *BBSbanner = NULL;         /* Default BBS banner        */
char *InUseBBSBanner;           /* Banner to be used for this call */
struct SBBSBanner **BBSBannerCID = NULL;  /* Banners for different CID */

char *BBSreader = NULL;         /* Message reader            */
char *BOSSphone = NULL;         /* Phone number for boss     */
char *BOSSpwd = NULL;           /* Password for boss         */
char *download_path = NULL;     /* Default download path     */
char *BBSnote = NULL;           /* Tell user BBS loading     */
char *MAILnote = NULL;          /* Say same for ext mail     */
int EXTopt = OPT_BATCH;         /* MR 961122 char* -> int External mailer options  */
struct SStringRep **SReplacements = NULL;  /* Strings to be replaced before logging */
unsigned definedbanners = 0;
unsigned definedreplacements = 0;
char *CurrentCID = NULL;        /* Current caller-ID, if any *//* MR 970311 */

#if defined(OS_2) && !defined(__EMX__)  /* CFS 970310 */

byte UseCAPICID = 0;            /* Get CID info directly from CAPI */
char **AddCIDTo = NULL;         /* Add /ID=number to modem strings starting with this */
USHORT AddCIDToNumber = 0;
char CIDFromCAPI[80];           /* Last CID received from CAPI */
byte UpdateCID = 1;

#endif

#ifdef SMP_API

BOOL32 smp = FALSE;

#endif

int nologhundredths = 0;        /* If 1, 1/100s of second won't be logged - CFS961121 */

#ifdef BINKLEY_SOUNDS

char *fnBBSSound = NULL;
char *fnEXTSound = NULL;
char *fnMailSound = NULL;
char *fnFileSound = NULL;
char *fnStartSound = NULL;
char *fnFAXSound = NULL;

#endif

void (_cdecl * mtask_idle) (void);
char *BINKpath = "";
char capturename[PATHLEN];
char *log_name = NULL;
char *readlog_name = NULL;      /* TJW 960724 */
char *config_name = "binkley.cfg";
char *swapdir = NULL;

/* MMP 960419 - begin */

char *MailFlag = NULL;
char *NetMailFlag = NULL;       /* VRP 990918 */
char *ArcMailFlag = NULL;       /* VRP 990918 */
char *TicFlag = NULL;           /* VRP 990918 */
time_t diff_trx;                /* Set in Decode_EMSI_Header() */
long max_diff_trx;
int locallogs = 0;

/* MMP 960419 - end */

char *shortcostlog = NULL;      /* MMP 960422 */
long maxbsyage = 0;             /* MMP 960428 */
char *spawnnook = NULL;         /* MMP 960428 */
long connectrate = 0;           /* MMP 960428 */

#ifdef CACHE                    /* MR 970329 */

int cachehold = 0;              /* MMP 960507 */
long cachemem = 0;              /* MMP 960513 */
int rescanstat = 0;             /* MMP 960513 */

#endif

long rescancnt = 0;             /* MMP 960513 */
long rescanticks = 0;           /* MMP 960513 */

long readflagtime = 15;         /* CEH 990604 */

#if defined(OS_2) | defined(_WIN32)

char *pipename = NULL;
HSNOOP hsnoop = NULLHANDLE;

#endif

char *scan_list[10];
int set_loaded = 0;

byte user_exits[6];

int happy_compiler;

REGIONP wholewin = NULL;
REGIONP settingswin = NULL;
REGIONP historywin = NULL;
REGIONP holdwin = NULL;
REGIONP callwin = NULL;
REGIONP filewin = NULL;

int scrllines = 0;

/* TE 971014 replacement character found in the nodelist instead of the
 * IP point */
char cIPRepChar = '\0';

PN_TRNS *pn = NULL;
MDM_TRNS *mm = NULL;

int resp_count = 0;
int max_resp_count = 0;
struct resp_str *mdm_resps = NULL;

/* Default modem response strings. Used when the user
 * doesn't take advantage of the new config verbs. */

struct resp_str def_mdm_resps[] =
{
  {"RINGING", RING},
//  {"RING RESPONSE", COSTRESET},
  {"RING", RINGING},            /* if RING has to be moved from pos 2 */
                                /* to another pos, don't forget to    */
                                /* adjust btconfig.c, SameRing!!!     */
  {"CONNECT", CONNECTED},
//  {"RRING",         COSTRESET},
  {"BUSY", DIALRETRY},          /* Was FAILURE before dial retry */
  {"VOICE", FAILURE},
  {"ERROR", FAILURE},
  {"OK", IGNORE},               /* TJW 960616 FAILURE -> IGNORE */
  {"NO CARRIER", FAILURE},
  {"NO DIAL", INCOMING},        /* "NO DIAL TONE" or "NO DIALTONE" */
//  {"DIALING",       COSTRESET},
  {"NO ANSWER", FAILURE},
//  {"DIAL TONE",     COSTRESET},
  {"+FCO", FAX},
  {"+FCON", FAX}
};

int def_resp_count = sizeof def_mdm_resps / sizeof def_mdm_resps[0];

FILE *cost_log = NULL;
char *cost_log_name = NULL;
int cost_unit = 1;
int eurocost = 0;
int UseRingingCost = 0;         /* CFS 961225 (yeah, coding in Christmas) */
int UseModemRing = 0;
long cfscallstart;

FILE *status_log = NULL;
short boxtype = 0;

int no_collide = 0;             /* No collision detect */
int cursor_col = 79;
int cursor_row = 22;
char *keys[9];
char *shells[9];
char *errlvlshell[255];

int Netmail_Session;
HISTORY start_hist;
int isOriginator = 0;           /* Global logical "I'm the caller"   */
int got_arcmail;
int got_packet;
int got_mail;
int got_fax;
int got_tic;                    /* VRP 990918 */
int mail_finished;
int sent_mail;
int made_request;               /* WaZOO file request flag */

unsigned short remote_capabilities;
unsigned short my_capabilities;

int num_rakas = 0;              /* How many he's using (EMSI)        */
int remote_pickup = 0;          /* Does the remote want his mail ?   */


char *remote_password = NULL;
char *BBSesc = NULL;            /* TJW 961028 "\rPress <Escape> to enter BBS.\r"; */
char *noBBS = NULL;             /* TJW 961028 "\r\rProcessing Mail. Please hang up.\r\r"; */
int pickup_ok = 0;              /* event flag 'P' */
int no_pickup = 0;
int pickup_all = 0;
int no_WaZOO = 0;
int no_WaZOO_Session = 0;
int no_EMSI = 0;
int no_EMSI_Session = 0;

word ExtMailMask = 0;
char *ExtMailAgent = NULL;

char *request_template = "%s%s.r%02x";

char ext_flags[] = "odch";      /* Change to whatever, and */

struct _lang_hdr PrdctHdr;
char **PrdctTbl;                /* The New Order */

struct _lang_hdr AnsiHdr;
char *AnsiTbl;

#ifdef HAVE_HYDRA

ULONG hydra_options = 0x0L;
long hydra_txwindow = 0L;
long hydra_rxwindow = 0L;

#endif

int no_hydra = 0;
int no_janus = 0;
int no_zapzed = 0;
int no_dirzap = 0;
int no_zipzed = 0;
int no_dietifna = 0;
int on_our_nickel = 0;

int direct_zap;

int should_show_alive = 0;      /* RDH *** 9604 */

#if (defined USE_WINFOSSIL) || (defined DOS16)  /* MR 970329 */

char fossil_buffer[128];
char out_buffer[128];
char *fossil_fetch_pointer = fossil_buffer;
char *out_send_pointer = out_buffer;
int fossil_count = 0;
int out_count = 0;

#endif

#ifdef DOS16

FOSINFO fossil_info =
{
  0, 0, 0, 0L, 0, 0, 0, 0, 0, 0, 0
};

#endif

char ctrlc_ctr = 0;

int no_requests = 0;
int who_is_he = 0;

int curmudgeon = 0;             /* 1 = Randy Bush           */
int small_window = 0;
int no_overdrive = 0;
int no_resync = 0;
int no_sealink = 0;
int immed_update = 0;

unsigned long janus_baud = 0;

int mail_only = 0;
int caller;
int more_mail;
int doing_poll = 0;

char *BBSwelcome = NULL;        /* was static data (r. hoerner) */
char *aftermail = NULL;
int quickaftermail = 0;         /* MMP 960428 */

#ifdef CACHE

DIRBUF dta_str;                 /* MMP 960513 Was struct FILEINFO */

#else

struct FILEINFO dta_str;

#endif

int slowmodem = 0;

PN_TRNS *pn_head = NULL;
ANUMS *anum_head = NULL;
MDM_TRNS *mm_head = NULL;

#ifdef DOS16                    /* MR 970329 */

unsigned have_windows = 0;
unsigned have_dv = 0;

#endif

unsigned winslice = 0;
unsigned long lock_baud = 0;

/* Ideally we'd merge the ARQ and MNP stuff and have a new
 * config verb that defined the strings and one that said lock
 * on ARQ */

int ARQ_lock = 0;               /* If 1, baudrate is locked  */
int ARQs = 0;                   /* Number of ARQ strings     */
char *ARQ[16];                  /* Pointers to ARQ strings   */

int cMNP = 0;                   /* Number of MNP strings     */
char *pchMNP[16];               /* Pointers to MNP strings   */

char *e_input;                  /* no more static: allocated on heap /r.hoerner/ */

char *local_CEOL = "\033[K";

char *nodelist_name = NULL;     /* Name of our nodelist      */
char *nodelist_base = NULL;     /* Name of current nodelist  */
char *fri_areadat = NULL;       /* TJW 960522 filename of frq idx area file */
char *fri_areaidx = NULL;       /* AW 980217 name of area idx file Max 3.0  */
char *fri_uclass = NULL;        /* AW 980217 name of access.dat Max 3.0     */

/* TE 960518: 0=Maximus, 1=Proboard, FRI_MAXIMUS, FRI_PROBOARD in bink.h */
int fri_areatype = 0;

struct _max_classname *fri_max_classname = NULL;

int newnodelist = 0;            /* if 1, use new nodelist.   */
int version7 = 1;               /* Support new V7 nodelist   */
int sysopndx = 0;               /* =0 : use nodex.sdx, =2 : use sysop.ndx */

int autobaud = 0;               /* Use highest baudrate when calling out */

unsigned short found_zone = 0;  /* What zone found node is in */
unsigned short found_net = 0;   /* What net found node is in */
struct _newnode newnodedes;     /* structure in new list     */

char far *node_index = (char far *) NULL;  /* pointer to node array     */

HISTORY hist;
BINK_EVENT far *e_ptrs = NULL;
unsigned char keeperr1, keeperr2;  /* CFS 970309 - backup copy of mail errorlevels */
int requests_ok = 1;
int num_events = 0;
int cur_event = -1;
int next_event = 0;
int got_sched = 0;
int noforce = 0;
int max_connects = 3;
int max_noconnects = 10000;
int server_mode = 0;
int forcexit = 0;               /* force exit errorlevel */
char *packer = NULL;
char *cleanup = NULL;
char *answerback = NULL;

time_t etm = (time_t) 0L;
long file_length = 0L;

// short Tx_y;    /* Hydra/Janus Line number of file transmission status display */
// short Rx_y;    /* Hydra/Janus Line number of file reception status display    */
// short tx_line; /* unidirectional protocols: line numer of same kind */

// MR 970329 not used (static in hydra.c and janus.c)
// char txfname[PATHLEN+1]; /* from hydra.c. Now used also by janus.c */
// char rxfname[PATHLEN+1]; /* from hydra.c. Now used also by janus.c */

char *script_path = NULL;       /* Where the script files live */

char Rxhdr[4];                  /* Received header           */
char Txhdr[4];                  /* Transmitted header        */

long Rxpos;                     /* Received file position                            */
int Txfcs32;                    /* TRUE means send binary frames with 32 bit FCS     */
int Crc32t;                     /* Display flag indicating 32 bit CRC being sent     */
int Crc32;                      /* Display flag indicating 32 bit CRC being received */

// MR 970329 not used:
// int Znulls;           /* # of nulls to send at beginning of ZDATA hdr      */

int Rxtimeout;                  /* Tenths of seconds to wait for something */
int Rxframeind;                 /* ZBIN ZBIN32,ZHEX type of frame received */
char *Filename;                 /* Name of the file being up/downloaded    */
word z_size = 0;
int fsent;                      /* global counter of sent files (WaZoo)    */
int zmodem_state;               /* zmodem state counter                    */
byte Resume_WaZOO;              /* Flags resumption of aborted WaZOO xfer  */
char Resume_name[13];           /* "Real" name of file being received      */
char Resume_info[48];           /* File size&time info for rx file         */
char Abortlog_name[PATHLEN];    /* Path of WaZOO aborted xfer log          */

nfunc nodefunc = ver7find;      /* Nodelist function to use */
ufunc userfunc = opususer;      /* Userlist function to use */

#ifdef DOS16

struct vfossil_hooks vfossil_funcs;

#endif

int vfossil_installed = 0;
VIOMODEINFO vfos_mode;
char far *blanks;

char *logptr = NULL;

MAILP mail_top;
MAILP next_mail;
long next_rescan = 0L;
time_t last_rescan_tstamp = (time_t) 0;
long waitfor_line = 0L;         /* Used to limit collision logic */

J_TYPESP j_top;
J_TYPESP j_next;
int janus_OK;
char saved_response[128];
char mdm_reliable[128];

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*     The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*     The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

unsigned long far cr3tab[] =
{                               /* CRC polynomial 0xedb88320 */
  0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL,
  0x706af48fUL, 0xe963a535UL, 0x9e6495a3UL, 0x0edb8832UL, 0x79dcb8a4UL,
  0xe0d5e91eUL, 0x97d2d988UL, 0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL,
  0x90bf1d91UL, 0x1db71064UL, 0x6ab020f2UL, 0xf3b97148UL, 0x84be41deUL,
  0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL, 0x136c9856UL,
  0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL,
  0xfa0f3d63UL, 0x8d080df5UL, 0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL,
  0xa2677172UL, 0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL,
  0x35b5a8faUL, 0x42b2986cUL, 0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL,
  0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL, 0x26d930acUL, 0x51de003aUL,
  0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL, 0xcfba9599UL,
  0xb8bda50fUL, 0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL,
  0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL, 0x76dc4190UL,
  0x01db7106UL, 0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL,
  0x9fbfe4a5UL, 0xe8b8d433UL, 0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL,
  0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL, 0x91646c97UL, 0xe6635c01UL,
  0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL, 0x6c0695edUL,
  0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL, 0x65b0d9c6UL, 0x12b7e950UL,
  0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL,
  0xfbd44c65UL, 0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL,
  0x4adfa541UL, 0x3dd895d7UL, 0xa4d1c46dUL, 0xd3d6f4fbUL, 0x4369e96aUL,
  0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL, 0x44042d73UL, 0x33031de5UL,
  0xaa0a4c5fUL, 0xdd0d7cc9UL, 0x5005713cUL, 0x270241aaUL, 0xbe0b1010UL,
  0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
  0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL,
  0x2eb40d81UL, 0xb7bd5c3bUL, 0xc0ba6cadUL, 0xedb88320UL, 0x9abfb3b6UL,
  0x03b6e20cUL, 0x74b1d29aUL, 0xead54739UL, 0x9dd277afUL, 0x04db2615UL,
  0x73dc1683UL, 0xe3630b12UL, 0x94643b84UL, 0x0d6d6a3eUL, 0x7a6a5aa8UL,
  0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL, 0xf00f9344UL,
  0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL,
  0x196c3671UL, 0x6e6b06e7UL, 0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL,
  0x67dd4accUL, 0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL,
  0xd6d6a3e8UL, 0xa1d1937eUL, 0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL,
  0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL, 0xd80d2bdaUL, 0xaf0a1b4cUL,
  0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL, 0x316e8eefUL,
  0x4669be79UL, 0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL,
  0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL, 0xc5ba3bbeUL,
  0xb2bd0b28UL, 0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL,
  0x2cd99e8bUL, 0x5bdeae1dUL, 0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL,
  0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL, 0x72076785UL, 0x05005713UL,
  0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL, 0x92d28e9bUL,
  0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL, 0x86d3d2d4UL, 0xf1d4e242UL,
  0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL,
  0x18b74777UL, 0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL,
  0x8f659effUL, 0xf862ae69UL, 0x616bffd3UL, 0x166ccf45UL, 0xa00ae278UL,
  0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL, 0xa7672661UL, 0xd06016f7UL,
  0x4969474dUL, 0x3e6e77dbUL, 0xaed16a4aUL, 0xd9d65adcUL, 0x40df0b66UL,
  0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
  0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL,
  0xcdd70693UL, 0x54de5729UL, 0x23d967bfUL, 0xb3667a2eUL, 0xc4614ab8UL,
  0x5d681b02UL, 0x2a6f2b94UL, 0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL,
  0x2d02ef8dUL
};

/* -rev 04-16-87  (abbreviated)
 *  The CRC-16 routines used by XMODEM, YMODEM, and ZMODEM
 *  are also in this file, a fast table driven macro version */

/* crctab calculated by Mark G. Mendel, Network Systems Corporation */
unsigned short far crctab[256] =
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
  0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
  0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
  0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
  0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
  0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
  0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
  0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
  0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
  0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
  0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
  0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
  0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

char stat_str[] = "%c %8s %     ";
char script_line[] = "Script Line %03d: %     ";
int need_update = 0;

char *BadChars = "\026\021\b\377\376\375\374";

int pktrsp = 0;                 /* MB 93-12-12  PKTRSP */

int freqlongnames = 0;          /* HJK - 99/01/22 */

long reinittime = 10L * (long) PER_MINUTE;  /* mtt 96-04-16, TS 970124  */
long readholdtime = PER_MINUTE; /* mtt 96-04-16 */

char SRIF_commandline[PATHLEN]; /* mtt 96-04-27 vvv SRIF stuff */
char remote_sysop[80];
char remote_site[80];
char remote_location[80];
char remote_phone[80];
char remote_mailer[80];
char password[18];              /* 16 Bytes PW + 1 Byte \0 + 1 Byte alignment */
int is_protected;
int is_listed;                  /* mtt 96-05-19 ^^^ SRIF stuff */

char fpnumber[128] = "";        /* caller-id for freepoll   CE 960421 */
char *ModemReject = NULL;       /* CE 960505 */
int chg_call = 0;               /* CE 960506 */
int chg_mail = (int) 'f';       /* CE 960526 */
int check_path = 0;             /* CE 971218 don't check drives for there existens */
int check_path_errorlevel = 253;  /* CE 971218 default errorlevel for CheckPath */
int check_path_time = 30;       /* CE 971218 default time for next CheckPath */

#ifdef MAKEDIR

int makedir = 0;                /* TJW 960429 */

#endif

int do_RH1hydra;                /* TJW 960528 RH1hydra / xHydra */
int do_chat = 0;                /* TJW 960706 */

char *protpreference = NULL;    /* TJW 960719 Protocol preference for EMSI  */
char actprotpreference[20];     /* TJW 961128 ProtPreference current session  */

int costtimecorr_conn = 5;      /* TJW 960721 time correction for cost calc */
int costtimecorr_sess = 5;      /* TJW 960805 time correction for cost calc */

char *ipc_dir = NULL;           /* TJW 960723 where to put IPC##.BBS files  */

int EMSIbanner = 0;             /* TJW 960728 send EMSI banner string ?     */

long costcps = 1024L;           /* TJW 960802 assumed CPS for cost calculation */

int nohydrachat = 0;            /* TJW 960804 disable hydra chat */

char *task_dir = NULL;          /* TJW 960830 where to put BINKLEY.?## files  */

#ifdef DOS16

int noansitrash = 0;            /*  TS 961214   */
int cshape;                     /*  TS 961215 holds cursor shape    */

#endif

XFERSTATS TxStats, RxStats;     /* TS/TJW 960903 for new xfer stats window */
GENSTATS GenStats;              /* TJW 970310 for new xfer stats window    */

long chattimer;
long next_time = 0L;            /* timer variable for forcerescancheck() */

char *title_format = NULL;      /* TJW 961013 window title format */

int SIOMode = 0;                /* CFS 9612 */

PKTREP PKTStats;                /* CFS 970209 */
int DoStats = 0;

char *local_inbound = NULL;     /* CEH 990212 */

char *HydraMessage = NULL;      /* mtt 970115 */

/* ------------------------------------------------------------------------- */
/* r. hoerner included this stuff                                            */
/* ------------------------------------------------------------------------- */
BINK_SAVEP popped_up = NULL;    /* status_line()!                            */
NODESTRUC *knownnode = NULL;    /* list of nodes with special flags set      */
char *port_device = NULL;       /* replaces/addon to port_ptr                */
int exact_mdm_type = 1;         /* "0" = "bittype" nodelist flags            */
int debug_emsi = 0;             /* "1" = log emsi string                     */
char *emsilog = NULL;           /* path+filename to emsi logfile             */
char *passwordfile = NULL;      /* path+filename to password file            */
long assumebaud = 300;          /* if baudrate unknown                       */
long ReDialTime = 0;            /* redial time (sec) after busy              */
int no_delay = 0;               /* don't delay errormsgs if not wanted       */
int noMNPFilter = 0;            /* NO filtering at all                       */
int binkdir = 0;                /* make missing binkley directories          */
int hidedomains = 1;            /* don't display domains in outbound window  */
int rescanforced = 0;           /* rescan even if btrescan.bsy exists        */
int nofancystrings = 0;         /* no fancification anymore                  */
int inzoomwindow = 0;           /* for "status_line"                         */
int MaxRingWait = 75;           /* Max seconds before abort outdialing       */
int show_password = 0;          /* show password in info-display (alt-i)     */
int noWildcards = 0;            /* Allow "*" and "?" in fReq files           */
int DelBadCall = 0;             /* Allow deletion of netnode.$$? if called   */
char lastfound_SysopName[50];   /* version7.c,set_vars(). For display only   */
PHONE *phone1 = NULL;           /* phonenumber override                      */
char *bink_pipe = NULL;         /* Binkleys internal pipe: name of remote    */
char *eventfile = "binkley.evt";  /* filename of event file for task x       */
int initialized = 0;            /* ok to work with "status_line"??           */

short session_method;           /* current session method                    */
int cFosCost = 0;               /* take actual cost from cFos-Info (ATi1)    */
char HAS[] = "*";               /* for use with debugging protocols          */
char HAS_NOT[] = " ";           /* saves a lot of space                      */
int DrawClock = 0;              /* screensaver clock                         */

char *addBINKpath = "";         /* MR 970213 */
int clock_minute = -1;          /* update clock 1 time per minute            */
char clock_char = 0;            /* alternate char  to draw the clock         */
char clock_color = 2;           /* alternate color to draw the clock         */
int smart_bidi = 0;             /* allow zModem instead of Hydra if one-way  */
int smart_bidi_out = 0;         /* allow smart_bidi on outgoing calls        */

/* ------------------------------------------------------------------------- */

ONLINE_TIME *online_time1 = NULL;  /* VRP 990820 */
HIDDEN *hidden1 = NULL;         /* VRP 990830 */
short weekdays[7] =
{
  DAY_SUNDAY, DAY_MONDAY, DAY_TUESDAY, DAY_WEDNESDAY, DAY_THURSDAY,
  DAY_FRIDAY, DAY_SATURDAY
};                              /* VRP 990829 */

int SendHoldIfCaller = 0;       /* VRP 990824 */

int RestorePoll = 0;            /* VRP 990826 */
char RestorePollFlavour[2] = "\0\0";  /* VRP 990826 */
int SaveExtSession = 0;         /* VRP 990920 */

int show_modemwin = 0;          /* MR 970215 */

#ifdef OS_2

byte MonitorCFOS = 0;           /* CFS 970311 */
int ReqChannelChange = 0;       /* CFS 970320 */
byte ReqHardwareReset = 0;      /* CFS 970320 */

#endif

byte ReqSkipFile = 0;           /* CFS 970323 */

int no_tranx = 0;               /* JNK 070312 */

char HistoryFileName[PATHLEN];  /* CFS 970411 - AG 990119 corrected length */
long realspeed = 0;             /* CFS 970412 - speed reported by CONNECT */

byte autochatopen = 0;

/* ------------------------------------------------------------------------- */
/* Chat command processor stuff                                              */
/* ------------------------------------------------------------------------- */
char RemCommandBuf[SBCOM_BUFLENGTH];
char LocCommandBuf[SBCOM_BUFLENGTH];

char *ChatLogDir = NULL;
FILE *ChatLogHandle = NULL;

/* ------------------------------------------------------------------------- */
/* Random system information (the result of a CFS' idealess weekend)         */
/* ------------------------------------------------------------------------- */
RANDOM_SYSTEM **RandomSystemInfo = NULL;
word RandomGroups = 0;

long debugflags = 0L;           // temporary debug bit flags

int HistMode = HIST_LONG;
LSTATS *lstats = NULL;

time_t scdcompiletime;          // TJW 971031 compile time of scd file

char *TZ = NULL;                // TJW 971031 ptr to TZ environment

int AKA_matching = 0;           /* CE 971230 - AKA matching on incoming calls */
long tzutc_timer = 0;           /* AW 980301 timezone calculation */
int SyncHydraSession = 0;       /* CE 980119 - synchronize Hydra sessions */
int NoModem = 0;                /* CE 980119 - Modem is enabled */
int bttask = 0;                 /* CE 980227 - kill active session, if 1 */
long rejectignoresize = -1;     // JH 980817 - ignore rejects when more mail on hold

/* $Id: data.c,v 1.18 1999/09/27 20:51:23 mr Exp $ */
