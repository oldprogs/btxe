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
 * Filename    : $Source: E:/cvs/btxe/include/bink.h,v $
 * Revision    : $Revision: 1.15 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:07 $
 * State       : $State: Exp $
 *
 * Description : Major definitions used in BinkleyTerm
 *
 *---------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------ */
/* LEGIBLE SECTION.  Definitions to make "C" look like a real language.     */
/* ------------------------------------------------------------------------ */

#ifndef BINK_DEFINED
#define BINK_DEFINED

#ifndef max
#define max(a,b)     ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b)     ((a)<=(b)?(a):(b))
#endif

/* updcrc macro derived from article Copyright (C) 1986 Stephen Satchell.
 *  NOTE: First argument must be in range 0 to 255.
 *        Second argument is referenced twice.
 *
 * Programmers may incorporate any or all code into their programs,
 * giving proper credit within the source. Publication of the
 * source routines is permitted so long as proper credit is given
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
 * Omen Technology. */

#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255) ^ cp] ^ (crc << 8))

/* We need it the other way around for BinkleyTerm */
#define xcrc(crc,cp) ( crctab[((crc >> 8) & 255) ^ cp] ^ (crc << 8))

typedef unsigned bit;
typedef unsigned short word;
typedef unsigned char byte;

#define fallthrough

#define LOCAL        static
#define empty_str    ""

/* The following changes by Greylock Software, to facilitate the MEWEL
 * version of Bink. */

#define PRDCT_NM(m)     PrdctTbl[m]
#define POLL_POPUP

struct _lang_hdr
{
  short ElemCnt;
  short PoolSize;
};

struct _key_fnc
{
  short FncIdx;
  short ScanCode;
};

struct _key_fnc_hdr
{
  short KeyFncCnt;
  short *KeyFncAry;
  struct _key_fnc *KeyFncTbl;
};

typedef struct pnums
{
  char num[20];
  char pre[20];
  char suf[20];
  short len;
  struct pnums *next;
}
PN_TRNS;

typedef struct anums
{
  char *num;
  char *alt;
  struct anums *next;
}
ANUMS, *P_ANUMS;

typedef struct mnums
{
  byte mdm;
  char pre[50];
  char suf[50];
  char ascii[32];
  struct mnums *next;
}
MDM_TRNS;

/* Used to parse result codes. Filled in by user in BTCONFIG or
 * by default in B_INITVA. */

struct resp_str
{
  char *resp;
  unsigned disp;
};

/* structure of our config statements */
/*
   THIS STRUC IS OBSOLETE AND REMOVED! R.Hoerner jan 1997
   struct parse_list
   {
   char p_length;
   char *p_string;
   };

 */

/* Values for try_?_connect, definition in resp_str.disp. */

#define USER_BREAK     -9       /* user pressed ESC                    */
#define WAS_BUSY       -2       /* return "BUSY"                       */
#define WAS_INCOMING   -1       /* either "incoming call" or "cables"  */

/* modem results */

#define FAILURE     0           /* nothing happened, break             */
#define IGNORE      1           /* nothing important happend, continue */
#define CONNECTED   2           /* CONNECT seen                        */
#define INCOMING    3           /* NO DIALTONE seen when dialing: incoming call */
#define DIALRETRY   4           /* BUSY seen, try alternate number     */
#define RINGING     5           /* RING seen                           */
#define RING        6           /* RINGING seen,                       */
#define COSTRESET   7           /* A string that resets the cost       */
#define FAX        16           /* Fax starts at 16                    */
#define ZYXFAX     17           /* Fax or'ed with 1.                   */
#define FAX_RES1   18
#define FAX_RES2   20
#define FAX_RES3   24           /* last fax response!                  */


struct modem_commands           /* MR 970325 */
{
  int no_okcheck;               /* MR 970720 disable OK check       */
  char *init_setup;
  char *init[5];                /* Used to init modem (mailer)      */
  int init_cnt;                 /* MR 970310 multiple init commands */
  char *terminit[5];            /* Used to init modem (terminal)    */
  int terminit_cnt;             /* MR 970310 multiple init commands */
  char *answer_setup[5];        /* Send before answer is send       */
  int answer_setup_cnt;         /* MR 970503                        */
  char *answer;                 /* Answer a call                    */
  char *reject;                 /* reject incomming call            */
  char *busy;                   /* Take modem offhook               */
  char *hangup;                 /* Hang Up, do not reset modem!     */
  char *aftercall;              /* Sent to modem after call         */
  char *aftercallout;           /* Sent to modem after reject ???   */
  char *afterconnect;           /* Sent to modem after connect      */
  char *dial_setup;
  char *predial;                /* Pre-dial setup                   */
  char *postdial;               /* Post-dial cleanup                */
  char *normprefix;
  char *normsuffix;
};


struct prototable
{
  char first_char;
  short entry;
};

typedef struct j_types
{
  char j_match[30];
  struct j_types *next;
}
J_TYPES, *J_TYPESP;

/* mailtypes bit field definitions */

#define MAIL_CRASH     0x0001
#define MAIL_HOLD      0x0002
#define MAIL_DIRECT    0x0004
#define MAIL_NORMAL    0x0008
#define MAIL_REQUEST   0x0010
#define MAIL_WILLGO    0x0020
#define MAIL_TRIED     0x0040
#define MAIL_TOOBAD    0x0080
#define MAIL_UNKNOWN   0x0100
#define MAIL_COST      0x0200
#define MAIL_QSMALL    0x0400
#define MAIL_CANTDIAL  0x0800
#define MAIL_RES1000   0x1000
#define MAIL_RES2000   0x2000
#define MAIL_RES4000   0x4000
#define MAIL_RES8000   0x8000

typedef struct finfo
{
  int info_size;
  char curr_fossil;
  char curr_rev;
  char far *id_string;
  unsigned int ibufr;           /* TJW 961231 int -> unsigned int */
  unsigned int ifree;           /* ""    I hope this is correct!  */
  unsigned int obufr;           /* ""                             */
  unsigned int ofree;           /* ""                             */
  byte swidth;
  byte sheight;
  char baud;
}
FOSINFO;

typedef short (*PFI3) (short, short, short, short);

typedef struct
{
  unsigned short background;
  unsigned short settings;
  unsigned short history;
  unsigned short hold;
  unsigned short call;
  unsigned short file;
  unsigned short calling;
  unsigned short popup;
  unsigned short headers;       /* TJW 960428 2 more colors - like EE ! */
  unsigned short frames;        /* TJW 960428 2 more colors - like EE ! */
  unsigned short mw_sent;       /* MR 970310 again 2 more colors for modem win */
  unsigned short mw_rcvd;       /* MR 970310 again 2 more colors for modem win */
}
SB_COLORS;

/* ------------------------------------------------------------------------ */
/* Sealink and Telink header structure                                      */
/* ------------------------------------------------------------------------ */

#define HEADER_NAMESIZE  17

struct zero_block
{
  long size;                    /* file length                    */
  long time;                    /* file date/time stamp           */
  char name[HEADER_NAMESIZE];   /* original file name             */
  char moi[15];                 /* sending program name           */
  char noacks;                  /* for SLO                        */
};

/* ------------------------------------------------------------------------ */
/* NodeList.Sys                                                             */
/*                                                                          */
/*    NET > 0 and NODE > 0    Normal node                                   */
/*                                                                          */
/*    NET > 0 and NODE <= 0   Host node                                     */
/*                               Net host........node== 0                   */
/*                               Regional host...node==-1                   */
/*                               Country host....node==-2                   */
/*                                                                          */
/*    NET == -1      Nodelist.Sys revision                                  */
/*                                                                          */
/*    NET == -2      Nodelist statement                                     */
/*                                                                          */
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* NODE                                                                     */
/* Please note the NewStyle structure (below).  Time is running out for the */
/* existing `_node' structure!  Opus currently uses the old style node      */
/* structure, but not for long.                                             */
/* ------------------------------------------------------------------------ */

struct _node
{
  short number;                 /* node number                              */
  short net;                    /* net number                               */
  short cost;                   /* cost of a message to this node           */
  short rate;                   /* baud rate                                */
  char name[20];                /* node name                                */
  char phone[40];               /* phone number                             */
  char city[40];                /* city and state                           */
};

/* ------------------------------------------------------------------------ */
/* THE NEWSTYLE NODE LIST IS NOW BEING USED BY OPUS 1.10                    */
/* ------------------------------------------------------------------------ */

struct _newnode
{
  word NetNumber;
  word NodeNumber;
  word Cost;                    /* cost to user for a message */
  char SystemName[34];          /* node name */
  char PhoneNumber[40];         /* phone number */
  char MiscInfo[30];            /* city and state */
  char Password[8];             /* WARNING: not necessarily null-terminated */
  word RealCost;                /* phone company's charge */
  word HubNode;                 /* this node's hub, or point number if a point */
  byte BaudRate;                /* baud rate divided by 300 */
  byte ModemType;               /* RESERVED for modem type */
  word NodeFlags;               /* set of flags (see below) */
  short online_start;           /* online start hour AW990321 */
  short online_end;             /* online end hour AW990321 */
};

/* ------------------------------------------------------------------------ */
/* nodex.ndx                                                                */
/*                                                                          */
/* Version 7 Nodelist Index structure.  This is a 512-byte record, which    */
/* is defined by three structures:  Record 0 is the Control Record, then    */
/* some number of Leaf Node (LNode) Records, then the Index Node (INode)    */
/* Records.  This defines an unbalanced binary tree.                        */
/*                                                                          */
/* This description is based on Scott Samet's CBTREE.PAS program.           */
/*                                                                          */
/* ------------------------------------------------------------------------ */

struct _ndx
{
  union
  {
    struct _CtlBlk
    {
      word CtlBlkSize;          /* Blocksize of Index Blocks   */
      long CtlRoot;             /* Block number of Root        */
      long CtlHiBlk;            /* Block number of last block  */
      long CtlLoLeaf;           /* Block number of first leaf  */
      long CtlHiLeaf;           /* Block number of last leaf   */
      long CtlFree;             /* Head of freelist            */
      word CtlLvls;             /* Number of index levels      */
      word CtlParity;           /* XOR of above fields         */
    }
    CtlBlk;

    struct _INodeBlk
    {
      long IndxFirst;           /* Pointer to next lower level */
      long IndxBLink;           /* Pointer to previous link    */
      long IndxFLink;           /* Pointer to next link        */
      short IndxCnt;            /* Count of Items in block     */
      word IndxStr;             /* Offset in block of 1st str  */

      /* If IndxFirst is NOT -1, this is INode:          */
      struct _IndxRef
      {
        word IndxOfs;           /* Offset of string into block */
        word IndxLen;           /* Length of string            */
        long IndxData;          /* Record number of string     */
        long IndxPtr;           /* Block number of lower index */
      }
#ifndef PORTABLEBINFILES
      IndxRef[20];
#else
      IndxRef[42];              /* make sure there is enough space */
#endif
    }
    INodeBlk;

    struct _LNodeBlk
    {
      /* IndxFirst is -1 in LNodes   */
      long IndxFirst;           /* Pointer to next lower level */
      long IndxBLink;           /* Pointer to previous link    */
      long IndxFLink;           /* Pointer to next link        */
      short IndxCnt;            /* Count of Items in block     */
      word IndxStr;             /* Offset in block of 1st str  */
      struct _LeafRef
      {
        word KeyOfs;            /* Offset of string into block */
        word KeyLen;            /* Length of string            */
        long KeyVal;            /* Pointer to data block       */
      }
#ifndef PORTABLEBINFILES
      LeafRef[30];
#else
      LeafRef[62];              /* make sure there is enough space */
#endif
    }
    LNodeBlk;

    char RawNdx[512];

  }
  ndx;
};

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* OPUS 1.20 Version 7 Nodelist structure. Copyright 1991 Wynn Wagner III   */
/* and Doug Boone. Used by permission.                                      */
/*                                                                          */
/* ------------------------------------------------------------------------ */

struct _vers7
{
  short Zone;
  short Net;
  short Node;
  short HubNode;                /* If a point, this is point number. */
  word CallCost;                /* phone company's charge */
  word MsgFee;                  /* Amount charged to user for a message */
  word NodeFlags;               /* set of flags (see below) */
  byte ModemType;               /* RESERVED for modem type */
  byte Phone_len;
  byte Password_len;
  byte Bname_len;
  byte Sname_len;
  byte Cname_len;
  byte pack_len;
  byte BaudRate;                /* baud rate divided by 300 */
};

/* ------------------------------------------------------------------------ */
/* Values for the `NodeFlags' field                                         */
/* ------------------------------------------------------------------------ */

#define B_hub    0x0001         /* node is a net hub      0000 0000 0000 0001 */
#define B_host   0x0002         /* node is a net host     0000 0000 0000 0010 */
#define B_region 0x0004         /* node is region coord   0000 0000 0000 0100 */
#define B_zone   0x0008         /* is a zone gateway      0000 0000 0000 1000 */
#define B_CM     0x0010         /* runs continuous mail   0000 0000 0001 0000 */
#define B_res1   0x0020         /* reserved by Opus       0000 0000 0010 0000 */
#define B_res2   0x0040         /* reserved by Opus       0000 0000 0100 0000 */
#define B_res3   0x0080         /* reserved by Opus       0000 0000 1000 0000 */
#define B_res4   0x0100         /* reserved by Opus       0000 0001 0000 0000 */
#define B_res5   0x0200         /* reserved for non-Opus  0000 0010 0000 0000 */
#define B_res6   0x0400         /* reserved for non-Opus  0000 0100 0000 0000 */
#define B_res7   0x0800         /* reserved for non-Opus  0000 1000 0000 0000 */
#define B_point  0x1000         /* node is a point        0001 0000 0000 0000 */
#define B_res9   0x2000         /* reserved for non-Opus  0010 0000 0000 0000 */
#define B_resa   0x4000         /* reserved for non-Opus  0100 0000 0000 0000 */
#define B_resb   0x8000         /* reserved for non-Opus  1000 0000 0000 0000 */


/* nodex.dtp (v7+) */

typedef unsigned long dword;

#define DWORD_DEFINED

typedef struct _DTPCtl
{
  word size;                    /* Size of this control structure */
  byte version;                 /* Version of DTP file */
  byte AllFixSize;              /* sizeof(_DTPAllLnk) */
  byte AddFixSize;              /* sizeof(_DTPNodeLnk) */
}
DTPCTL;

typedef struct _DTPAllLnk
{
  word Region;                  /* Region, 0 if none */
  word Hub;                     /* Hub, 0 if none */
  dword SOfs;                   /* DAT offset of next Same Sysop entry, 0xffffffff if none */
  dword POfs;                   /* DAT offset of next Same Phone entry, 0xffffffff if none */
  dword FeOfs;                  /* DAT offset of next "Equal Fido Level" entry, 0xffffffff if none */
  byte Sn;                      /* Number (0 based) of Sysop entry (ADR order) */
  byte Pn;                      /* Number (0 based) of Phone entry (ADR order) */
}
DTPALLLNK;

typedef struct _DTPNodeLnk
{
  word ndowns;                  /* number of system in lower level ("downlinks") */
  dword FlOfs;                  /* DAT offset of "Lower Fido Level" - first "downlink" */
}
DTPNODELNK;

typedef struct
{
  DTPCTL control;
  DTPNODELNK TopLink;
}
DTPHEADER;

/* ------------------------------------------------------------------------ */
/* Nodelist.Idx                                                             */
/* (File is terminated by EOF)                                              */
/* ------------------------------------------------------------------------ */

struct _ndi
{
  short node;                   /* node number  */
  short net;                    /* net number   */
};

/* ------------------------------------------------------------------------ */
/* QuickBBS 2.00 QNL_IDX.BBS                                                */
/* (File is terminated by EOF)                                              */
/* ------------------------------------------------------------------------ */

struct QuickNodeIdxRecord
{
  unsigned short QI_Zone;
  unsigned short QI_Net;
  unsigned short QI_Node;
  byte QI_NodeType;
};

/* ------------------------------------------------------------------------ */
/* QuickBBS 2.00 QNL_DAT.BBS                                                */
/* (File is terminated by EOF)                                              */
/* ------------------------------------------------------------------------ */

struct QuickNodeListRecord
{
  byte QL_NodeType;
  unsigned short QL_Zone;
  unsigned short QL_Net;
  unsigned short QL_Node;
  char QL_Name[21];             /* Pascal! 1 byte count, up to 20 chars */
  char QL_City[41];             /* 1 + 40 */
  char QL_Phone[41];            /* 1 + 40 */
  char QL_Password[9];          /* 1 + 8 */
  word QL_Flags;                /* Same as flags in new nodelist structure */
  word QL_BaudRate;
  word QL_Cost;
};

/* SEAdog NETLIST.DOG format */

struct netls
{
  short netnum;
  char netname[14];
  char netcity[40];
  short havehost;
  short nethost;
  short havegate;
  short netgate;
  long nodeptr;
  short numnodes;
};

/* SEAdog NODELIST.DOG format */

struct nodels
{
  short nodenum;
  char nodename[14];
  char nodecity[40];
  char nodephone[40];
  short havehub;
  short nodehub;
  short nodecost;
  short nodebaud;
};

/* Things most nodelists don't contain, but Binkley likes to have */

struct extrastuff
{
  char password[8];
  unsigned short flags1;
  byte modem;
  char extra[5];                /* for future expansion */
};

/* Format of a XMODEM block */

typedef struct
{
  unsigned char header;
  unsigned char block_num;
  unsigned char block_num_comp;
  unsigned char data_bytes[128];
  unsigned char data_check[2];
}
XMDATA, *XMDATAP;

/* Format of Telink block 0 */

typedef struct
{
  unsigned char header;
  unsigned char block_num;
  unsigned char block_num_comp;
  long filelength;

  union
  {
    struct
    {
      unsigned short time;
      unsigned short date;
    }
    twowords;

    struct
    {
      unsigned long timedate;
    }
    oneword;
  }
  filetime;

  char filename[16];
  char nullbyte;
  char sendingprog[15];
  char noacks;
  unsigned char crcmode;
  char fill[86];
  unsigned char data_check[2];
}
TLDATA, *TLDATAP;

/* Format of SEAlink block 0 */

typedef struct
{
  unsigned char header;
  unsigned char block_num;
  unsigned char block_num_comp;
  long filelength;
  unsigned long timedate;
  char filename[17];
  char sendingprog[15];
  char SLO;
  char Resync;
  char MACFLOW;
  char fill[85];
  unsigned char data_check[2];
}
SEADATA, *SEADATAP;

typedef struct
{
  unsigned int SEAlink:1;       /* Can do SEAlink */
  unsigned int SLO:1;           /* Can do SEAlink with Overdrive */
  unsigned int Resync:1;        /* Can do SEAlink with Resync */
  unsigned int MacFlow:1;       /* Can do SEAlink with Macintosh flow control */
  unsigned int do_CRC:1;        /* Should do CRC instead of checksum */
  unsigned int TeLink:1;        /* We saw a TeLink header */
}
TRANS, *TRANSP;

typedef struct
{
  char *state_name;
  short (*state_func) (void *);
}
STATES, *STATEP;

typedef struct
{
  long control;                 /* We will always have a long on top */
  long filler;                  /* Just to be sure. Min size, 8 bytes */
}
STATEBASE, *STATEBASEP;

typedef struct
{
  long control;                 /* These must always start with a long! */
  TRANS options;                /* Transfer options */
  short result;                 /* Result from last operation */
  short sub_results;            /* Extra result codes */
  long T1;                      /* General purpose timer */
  long T2;                      /* General purpose timer */
  short Window;                 /* SEAlink window size */
  long SendBLK;                 /* Current block to be sent */
  long NextBLK;                 /* Next block we will try to send */
  long ACKBLK;                  /* Block that was last ACK'd */
  long LastBlk;                 /* Last block in file */
  long ARBLK;                   /* Used in ACK Check calculations */
  long WriteBLK;                /* Block number to write to file */
  long filelen;                 /* Length of file being sent */
  long curr_byte;               /* Current byte offset of sending or receiving */
  long prev_bytes;              /* Bytes that we are resyncing over */
  long total_blocks;            /* Total number of blocks in file to be received */
  long resync_block;            /* Block number we received to resync to */
  short NumNAK;                 /* Number of NAK's received this block */
  short ACKsRcvd;               /* Number of ACK's received since file start */
  short ACKST;                  /* Current state of the ack/nak state variable */
  short tries;                  /* Number of tries thus far */
  short goodfile;               /* 0 if file was bad, 1 if file was good */
  short datalen;                /* Length of data in this block */
  short recblock;               /* Block number received */
  short sent_ACK;               /* Whether or not we sent an ACK already */
  short tot_errs;               /* Total number of errors */
  unsigned char ARBLK8;         /* 8 bit value of ARBLK */
  unsigned char blocknum;       /* 8 bit value of SendBLK */
  unsigned char check;          /* checksum value */
  unsigned char save_header;    /* Received header from first block */
  short CHR;                    /* General purpose receive character */

  union                         /* File date and time in Telink or SEAlink format */
  {
    struct
    {
      unsigned short time;
      unsigned short date;
    }
    twowords;

    struct
    {
      unsigned long timedate;
    }
    oneword;
  }
  save_filetime;

  char received_name[20];       /* Received filename from Telink or SEAlink */
  char m7name[12];              /* Filename in Modem7 format */
  char sending_program[16];     /* Sending program name */
  char *filename;               /* The filename to be sent or received */
  char *path;                   /* Just the path to the file to be sent/received */
  char *fptr;                   /* Pointer into character fields */
  char *temp_name;              /* Temporary name for receiving */
  FILE *file_pointer;           /* The pointer for read/write/seek operations */

  unsigned char header;
  unsigned char block_num;
  unsigned char block_num_comp;
  unsigned char data[128];
  unsigned char data_check[2];
}
XMARGS, *XMARGSP;

#define DID_RESYNC 1

typedef struct
{
  long control;                 /* These must always start with a long! */
  short tries;
  short barklen;
  short barkok;
  long T1;
  short nfiles;
  char *inbound;
  char *filename;
  char barkpacket[128];
  char *barkpw;
  char *barktime;
}
BARKARGS, *BARKARGSP;

/* ------------------------------------------------------------------------ */
/* FIDONET ADDRESS STRUCTURE                                                */
/* ------------------------------------------------------------------------ */

typedef struct _ADDRESS
{
  word Zone;
  word Net;
  word Node;
  word Point;
  char *Domain;
}
ADDR, *ADDRP;

typedef struct _DOMAINKLUDGE
{
  word zone;
  word domain;
}
DOMAINKLUDGE;

typedef struct mail
{
  ADDR mail_addr;               /* Addressee */
  unsigned short numfiles;      /* Total number of packets and files      */
  unsigned short mailtypes;     /* Bitmask of types we have for addressee */
  unsigned long mailsize;       /* Total amount of mail for addressee     */
  unsigned long req_size;       /* Amount of mail that is requests        */
  unsigned long callsize;       /* Amount of mail that isn't on hold      */
  unsigned long oldest;         /* time_t of oldest mail                  */

  union
  {
    struct
    {
      struct mail *next;
      struct mail *prev;
    }
    ptrnp;

    struct
    {                           /* these values are stored in btrescan.dmp */
      byte listed;              /* if ever the struct of btrescan.dmp      */
      byte ModemType;           /* changes, move these values into         */
      word NodeFlags;           /* struct dumpmail!                        */
      word RealCost;
      word Rsvd;
    }
    value;
  }
  ptrval;
}
MAIL, *MAILP;

typedef struct dumpmail         /* TJW 960721 struct dumped to btrescan.dmp */
{
  MAIL mail;
  char Domain[8];
}
DUMPMAIL, *DUMPMAILP;

typedef int (*nfunc) (ADDR *, int);
typedef void (*ufunc) (char *, ADDR *);

#define MAX_EXTERN  8

#define ALIAS_CNT   100         /* CE 960413 (was 25) */
#define DOMAINS     50          /* r. hoerner 1/6/97 */

/* ------------------------------------------------------------------------ */
/* Matrix mask                                                              */
/* Undefined bits are reserved by Opus                                      */
/* ------------------------------------------------------------------------ */

#define NO_TRAFFIC  0x0001
#define LOCAL_ONLY  0x0002
#define OPUS_ONLY   0x0004

#define NO_EXITS    0x2000
#define MAIL_ONLY   0x4000
#define TAKE_REQ    0x8000

/* ------------------------------------------------------------------------ */
/* Message packet header                                                    */
/* ------------------------------------------------------------------------ */

#define PKTVER  2

/* ------------------------------------------ */
/* POSSIBLE VALUES FOR `product' (below)      */
/*                                            */
/* NOTE: These product codes are assigned by  */
/* the FidoNet<tm> Technical Stardards Com-   */
/* mittee.  If you are writing a program that */
/* builds packets, you will need a product    */
/* code.  Please use ZERO until you get your  */
/* own.  For more information on codes, write */
/* to FTSC at 115/333.                        */
/* ------------------------------------------ */

#define isFIDO      0
#define isANXCLO    4
#define isOPUS      5
#define isIGOR      65
#define isTIMS      66
#define isBITBRAIN  0x1b
#define isDBRIDGE   0x1d
#define isMILQUE    0xac

#ifndef PRDCT_CODE
#define PRDCT_CODE      isBITBRAIN
#endif

#ifndef PRDCT_MAJOR
#define PRDCT_MAJOR     2
#endif

#ifndef PRDCT_MINOR
#define PRDCT_MINOR     60
#endif

#ifndef PRDCT_PRFX
#define PRDCT_PRFX      "BINKLEY"
#endif

#ifndef PRDCT_PRFXlower
#define PRDCT_PRFXlower "binkley"
#endif

#ifndef PRDCT_PRTY
#define PRDCT_PRTY      "BinkleyTerm"
#endif

#ifndef PRD_SH_PRTY
# if defined OS_2
#   define PRD_SH_PRTY  "BT/2"
# elif defined _WIN32
#   define PRD_SH_PRTY  "BT/Win32"
# elif defined __linux__
#   define PRD_SH_PRTY  "BT/Linux"
# elif defined __unix__
#   define PRD_SH_PRTY  "BT/Unix"
# else
#   define PRD_SH_PRTY  "BT"
# endif
#endif

#ifndef PRDCT_SHRT
# define PRDCT_SHRT     "Bink"
#endif

#ifndef PRDCT_VRSN
# define PRDCT_VRSN     "2.60XE"  /* TJW 960413 */
#endif

struct _pkthdr
{
  short orig_node;              /* originating node               */
  short dest_node;              /* destination node               */
  short year;                   /* 0..99  when packet was created */
  short month;                  /* 0..11  when packet was created */
  short day;                    /* 1..31  when packet was created */
  short hour;                   /* 0..23  when packet was created */
  short minute;                 /* 0..59  when packet was created */
  short second;                 /* 0..59  when packet was created */
  short rate;                   /* destination's baud rate        */
  short ver;                    /* packet version, must be 2      */
  short orig_net;               /* originating network number     */
  short dest_net;               /* destination network number     */
  char product;                 /* product type                   */
  char serial;                  /* serial number (some systems)   */

  byte password[8];             /* session/pickup password        */
  short orig_zone;              /* originating zone               */
  short dest_zone;              /* Destination zone               */
  byte B_fill2[16];
  long B_fill3;
};

struct _pkthdr45                /* FSC-0045 (2.2) packet type     */
{
  short orig_node;              /* originating node               */
  short dest_node;              /* destination node               */
  short orig_point;             /* originating point              */
  short dest_point;             /* destination point              */
  byte b_fill1[8];              /* Unused, must be zero           */
  short subver;                 /* packet subversion, must be 2   */
  short ver;                    /* packet version, must be 2      */
  short orig_net;               /* originating network number     */
  short dest_net;               /* destination network number     */
  char product;                 /* product type                   */
  char serial;                  /* serial number (some systems)   */

  byte password[8];             /* session/pickup password        */
  short orig_zone;              /* originating zone               */
  short dest_zone;              /* Destination zone               */
  byte orig_domain[8];          /* originating domain             */
  byte dest_domain[8];          /* destination domain             */
  long B_fill3;
};

struct _pkthdr39                /* FSC-0039 packet type           */
{
  short orig_node;              /* originating node               */
  short dest_node;              /* destination node               */
  short year;                   /* 0..99  when packet was created */
  short month;                  /* 0..11  when packet was created */
  short day;                    /* 1..31  when packet was created */
  short hour;                   /* 0..23  when packet was created */
  short minute;                 /* 0..59  when packet was created */
  short second;                 /* 0..59  when packet was created */
  short rate;                   /* destination's baud rate        */
  short ver;                    /* packet version, must be 2      */
  short orig_net;               /* originating network number     */
  short dest_net;               /* destination network number     */
  byte product_low;             /* FTSC product type (low byte)   */
  byte prod_rev_low;            /* product rev (low byte)         */

  byte password[8];             /* session/pickup password        */
  short zone_ignore[2];         /* Zone info from other software  */
  byte B_fill1[2];
  short CapValid;               /* CapWord with bytes swapped.    */
  byte product_hi;              /* FTSC product type (high byte)  */
  byte prod_rev_hi;             /* product rev (hi byte)          */
  short CapWord;                /* Capability word                */
  short orig_zone;              /* originating zone               */
  short dest_zone;              /* Destination zone               */
  short orig_point;             /* originating point              */
  short dest_point;             /* destination point              */
  long ProdData;                /* Product-specific data          */
};

/* ------------------------------------------------------------------------ */
/* WaZOO                                                                    */
/* ------------------------------------------------------------------------ */

#ifndef ACK
#define ACK    0x06
#endif

#ifndef NAK
#define NAK    0x15
#endif

#ifndef ENQ
#define ENQ    0x05
#endif

#ifndef YOOHOO
#define YOOHOO 0x00f1
#endif

#ifndef TSYNC
#define TSYNC  0x00ae
#endif

struct _Hello
{
  word signal;                  /* always 'o'     (0x6f)                   */
  word hello_version;           /* currently 1    (0x01)                   */
  word product;                 /* product code                            */
  word product_maj;             /* major revision of the product           */
  word product_min;             /* minor revision of the product           */
  char my_name[60];             /* Other end's name                        */
  char sysop[20];               /* sysop's name                            */
  word my_zone;                 /* 0== not supported                       */
  word my_net;                  /* out primary net number                  */
  word my_node;                 /* our primary node number                 */
  word my_point;                /* 0== not supported                       */
  byte my_password[8];          /* ALL 8 CHARACTERS ARE SIGNIFICANT !!!!!  */
  byte reserved2[8];            /* reserved by Opus                        */
  word capabilities;            /* see below                               */
  byte reserved3[12];           /* available to non-Opus                   */
  /* systems by prior "approval" of 124/108. */
};                              /* size 128 bytes */

/* ------------------------------------------------------------------------ */
/* YOOHOO<tm> CAPABILITY VALUES                                             */
/* ------------------------------------------------------------------------ */

#define Y_DIETIFNA  0x0001
#define FTB_USER    0x0002
#define ZED_ZIPPER  0x0004
#define ZED_ZAPPER  0x0008
#define DOES_IANUS  0x0010
#define DOES_HYDRA  0x0020
#define Bit_6       0x0040
#define DIR_ZAPPER  0x0080      /* TJW970714 only used internally! */
#define Bit_8       0x0100
#define Bit_9       0x0200
#define Bit_a       0x0400
#define Bit_b       0x0800
#define Bit_c       0x1000
#define Bit_d       0x2000
#define DO_DOMAIN   0x4000
#define WZ_FREQ     0x8000

/* ------------------------------------------------------------------------ */
/* EMSI                                                                     */
/* ------------------------------------------------------------------------ */

/* Indexes into the emsistr array for use by EMSI parsing */

#define EMSI_REQ   0
#define EMSI_DAT   1
#define EMSI_HBT   2
#define EMSI_NAK   3
#define EMSI_ACK   4
#define EMSI_INQ   5
#define EMSI_CLI   6

#define EMSI_LEN   14           /* longest emsi string */
#define EMSI_MAX   6            /* highest emsi string */

/* ------------------------------------------------------------------------ */
/* TJW 960505 PRIORITY CONSTANTS for set_prior(priority)                    */
/* ------------------------------------------------------------------------ */

#define PRIO_NORMAL  1          /* used for non critical actions and when spawning external programs */
#define PRIO_REGULAR 2          /* used when waiting for call */
#define PRIO_JANUS   3          /* used when transfering data with janus */
#define PRIO_MODEM   4          /* used when communicating with the modem */
#define PRIO_HYDRA   5          /* used when transfering data with hydra */

/* ------------------------------------ */
/* TE 960518 TYPE OF FILE REQUEST INDEX */
/* ------------------------------------ */

#define FRI_MAXIMUS  0
#define FRI_PROBOARD 1

#define E_INPUT_LEN 512         /* TJW 960708 size of array e_input[] */

/* ----------------------------- */
/* CFS 961020 STRING REPLACEMENT */
/* ----------------------------- */

struct SStringRep
{
  char *change;
  char *replacement;
  int frommodem;
};

#ifdef _WIN32                   /* AS: 970624, avoid compiler warning. (CALLBACK is defined as __stdcall in windef.h) */
#undef CALLBACK
#endif

/* R. Hoerner, possible flags for NODESTRUC */

#define NOEMSI   0x00001UL      /* "NoEMSI <nodenumber>"     OUTBOUND CALLS    */
#define NOWAZOO  0x00002UL      /* "NoWazoo <nodenumber>"    OUTBOUND CALLS    */
#define NOHYDRA  0x00004UL      /* "NoHydra <nodenumber>"    IN/OUTBOUND CALLS */
#define NOJANUS  0x00008UL      /* "NoJanus <nodenumber>"    IN/OUTBOUND CALLS */
#define NOREQ    0x00010UL      /* "NoReqests <nodenumber>"  INBOUND     CALLS */
#define NODIET   0x00020UL      /* "NoDietIFNA <nodenumber>" OUTBOUND CALLS    */
#define FREEREQ  0x00040UL      /* "ReqOnUs <nodenumber>"    IN/OUTBOUND CALLS */
#define NOZED    0x00080UL      /* "NoZedZap <nodenumber>"   IN/OUTBOUND CALLS */
#define NOSEA    0x00100UL      /* "NoSeaLink <nodenumber>"  IN/OUTBOUND CALLS */
#define NOPICK   0x00200UL      /* "NoPickup <nodenumber>"   IN/OUTBOUND CALLS */
#define NOHYCHAT 0x00400UL      /* "NoHydraChat <nodenumber>"IN/OUTBOUND CALLS */
#define DO_BELL  0x00800UL      /* "Gong <nodenumber>"        node x calls: bimmeling */
#define NOJOKERS 0x01000UL      /* "NoWildCards <nodenumber>" node may not freq "*.*" */
#define FREEPOLL 0x02000UL      /* "FreePoll <nodenumber>"   IN/OUTBOUND CALLS */
#define CALLBACK 0x04000UL      /* "CallBack <nodenumber>"   INBOUND CALLS */
#define TIMESYNC 0x08000UL      /* "TimeSync <nodenumber>"   IN/OUTBOUND CALLS */
#define NOZIP    0x10000UL      /* "NoZedZip <nodenumber>"   IN/OUTBOUND CALLS */

#define AT_NETMAIL   1
#define AT_ECHOMAIL  2
#define AT_OTHER     4

/* MR 970311 moved some parts to NODESTRUC, access via NODESTRUC */

typedef struct _FP              /* CE 15.04.96 Configline for FreePoll */
{
  long Size;                    /* limit in KB */
  time_t Delta_T;               /* max delta time for freepoll override */
  time_t MinDelta_T;            /* MR 970810 min delta time for override */
  short AttachType;             /* Bit-mapped struct - CFS 970504 */
  int and_op;                   /* =1 -> AND op, =0 -> OR op */
  struct _FP *next;
}
FP, *FPP;

typedef struct _CIDLST
{
  char *CallerID;
  struct _CIDLST *next;
}
CIDLST, *CIDLSTP;

/* ---------------------------------------------------------------------- */
/* Address structure for linked list of remote nodes (unlimited!)         */
/* ---------------------------------------------------------------------- */

typedef struct _NODESTRUC
{
  ADDR addr;                    /* MR 970314 */
  CIDLSTP CallerIDList;         /* caller ID list */
  long Flags;
  FPP FPoll;
  char *ExtraDir;               /* *MUST* be backslash-terminated */
  char *ExtraDirIn;             /* *MUST* be backslash-terminated */
  int ExtraDirHold;
  int ExtraDirNotify;
  int SyncHydraSessions;
  long tzutc;                   /* AW 980208 diff in seconds from UTC */
  struct _NODESTRUC *next;
}
NODESTRUC, *PNODESTRUC;

/* ---------------------------------------------------------------------- */
/* Address structure for linked list of AKAS (unlimited AKAs!)            */
/* ---------------------------------------------------------------------- */

typedef struct _AKA
{
  word Zone;
  word Net;
  word Node;
  word Point;
  char *Domain;
  struct _AKA *next;
}
AKA, *PAKA;

/* ---------------------------------------------------------------------- */
/* Structure for linked list of regions                                   */
/* ---------------------------------------------------------------------- */

typedef struct _AKREGION
{
  word Zone;
  char *Region;
  struct _AKREGION *next;
}
AKREGION, *AKPREGION;

/* ---------------------------------------------------------------------- */
/* structure for linked list of overrides                                 */
/* ---------------------------------------------------------------------- */

typedef struct _phones
{
  word Zone;
  word Net;
  word Node;
  word Point;
  char *Domain;
  char *num;
  word modem;
  word fido;
  struct _phones *next;
}
PHONE, *PPHONE;

/* ---------------------------------------------------------------------- */
/* structure for linked list of online times - VRP 990820                 */
/* ---------------------------------------------------------------------- */

typedef struct _online_time
{
  word Zone;
  word Net;
  word Node;
  word Point;
  char *Domain;
  short start;
  short end;
  short days;
  struct _online_time *next;
}
ONLINE_TIME, *PONLINE_TIME;

/* ---------------------------------------------------------------------- */
/* structure for linked list of hidden lines - VRP 990820                 */
/* ---------------------------------------------------------------------- */

typedef struct _hidden_line
{
  char *num;
  word modem;
  word fido;
  short start;
  short end;
  short days;
  struct _hidden_line *next;
}
HIDDEN_LINE, *PHIDDEN_LINE;

typedef struct _hidden
{
  word Zone;
  word Net;
  word Node;
  word Point;
  char *Domain;
  struct _hidden_line *current;
  struct _hidden_line *hidden_line1;
  struct _hidden *next;
}
HIDDEN, *PHIDDEN;

/* ---------------------------------------------------------------------- */
/* structure for reports to PKT - CFS 970209                              */
/* ---------------------------------------------------------------------- */

typedef struct _pktrep
{
  ADDR from, to;
  int toecho;
  char echo[65];
  char password[9];
}
PKTREP, *PPKTREP;

/* ---------------------------------------------------------------------- */
/* structure for long stats reports                                       */
/* ---------------------------------------------------------------------- */

typedef struct _lstats
{
  char *name;
  PKTREP pktr;
  int alllines;                 /* 0=only this task, other=all tasks */
  int coverage;                 /* This week, this year, etc */
  long days;                    /* Number of days to cover, if coverage==DAYS_FIXED; */
  struct _lstats *next;
}
LSTATS, *PLSTATS;

/* error values for config / event file parsers */

typedef enum _keyword_errors
{
  kwd_ok,
  kwd_bad,
  kwd_unknown,
  kwd_nomem,
  kwd_other_err
}
KWD_ERR;

/* error values for mailer sessions */

typedef enum _mailsession_errors
{
  mission_ok,
  mission_error,
  mission_aborted,
  mission_timeout
}
MISSION_ERR;

typedef struct
{
  char id[40];
  char **sysop;
  word sysopcount;
  char **system;
  word systemcount;
  char **location;
  word locationcount;
}
RANDOM_SYSTEM;


/* AW980220 array struct of priv<->classname relations for */
/* Maximus 2.x/3.x file request index                      */

struct _max_classname
{
  word priv;
  char *classname;
};

#endif /* BINK_DEFINED */

/* $Id: bink.h,v 1.15 1999/09/27 20:51:07 mr Exp $ */
