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
 * Filename    : $Source: E:/cvs/btxe/src/btconfig.c,v $
 * Revision    : $Revision: 1.20 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/30 09:38:00 $
 * State       : $State: Exp $
 * Orig. Author: Vince Perriello
 *
 * Description : BinkleyTerm Configuration File Parser
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#ifdef OS_2                     /* SM 961105 */
#ifdef PORTACCESS
#include "portio.h"
#endif
#endif

void killblanks (char *s);      /* MR 970113 see misc.c */

static KWD_ERR LOCALFUNC parse_keyword (int, char *);  /* TS 970328  */
KWD_ERR cfg_CopyString (int, char **, char *);
static KWD_ERR cfg_CopySlashString (int, char **, char *);
static KWD_ERR cfg_macro (char *);
static void cfg_dial_string (char *);
static KWD_ERR cfg_alt_num (char *);
static KWD_ERR cfg_mdm_response (char *c, int);
static void cfg_mdm_trans (char *);
static void cfg_domain (char *);
static KWD_ERR cfg_domain_kludge (char *);
static KWD_ERR cfg_override (char *);
static KWD_ERR cfg_hidden (char *);  /* VRP 990830 */
static KWD_ERR cfg_insert_AKA (char *);  /* r. hoerner */
static KWD_ERR cfg_set_nodeflag (int *, unsigned long, char *);
static KWD_ERR cfg_set_callerid (char *, ADDRP);  /* MR 970414  */
static KWD_ERR cfg_condpoll (char *);  /* MR 970327  */
static void cfg_Janus (char *);
static void bad_line (int, char *);  /* MR 970321  */
static word LOCALFUNC SetFidoFlag (char *, ADDRP);
static word LOCALFUNC SetModemFlag (char *);
static KWD_ERR LOCALFUNC SetOnLineTime (char *);  /* VRP 990820 */
static char *cfgfgets (char *, FILE *);  /* MMP 960419 / TJW 960928 */
static KWD_ERR cfg_set_chatlogdir (char *c);
static KWD_ERR cfg_set_nodeextradir (char *c);
static KWD_ERR cfg_set_nodeinbounddir (char *c);
static KWD_ERR cfg_set_SyncHydraSessions (char *c);
static KWD_ERR cfg_set_randomsystem (char *c);
static KWD_ERR cfg_set_randomsysop (char *c);
static KWD_ERR cfg_set_randomlocation (char *c);
static KWD_ERR cfg_set_nodetimezone (char *c);
static KWD_ERR cfg_parse_pktstats (char *c, PKTREP * PKTStats);
static KWD_ERR cfg_parse_online_time (char *, short *, short *, short *);  /* VRP 990830 */
static KWD_ERR cfg_statsdef (char *c);
static void LOCALFUNC cfg_insert_region (word Zone, char *Region);


#ifdef BINKLEY_SOUNDS

static char *cfg_sound (char *);

#endif

#define CFGLINEBUFSIZE 3072

#ifdef __IBMC__
#pragma pack(1)
#endif

static char far *config_lines[] =
{
  "SameRing",
  "NewNodeList",                /* obsolete! HJK - 99/02/28 */
  "QuickNodeList",              /* obsolete! HJK - 99/02/28 */
  "Answerback",
  "Macro",
  "Shell",
  "Dial",
  "Event",
  "Zone",                       /* obsolete! TJW970202 */
  "MaxReq",
  "LogLevel",
  "Baud",
  "MaxPort",
  "Port",
  "Carrier",
  "StatusLog",
  "Reader",
  "BossPhone",
  "BossPwd",
  "Protocol",
  "System",
  "Sysop",
  "Boss",
  "Point",                      /* obsolete! TJW970202 */
  "Aka",                        /* obsolete! TJW970202 */
  "Hold",
  "Downloads",
  "NetFile",
  "Init",
  "Busy",
  "Prefix",
  "NodeList",
  "Avail",
  "OKFile",
  "About",
  "MailNote",
  "Banner",
  "Unattended",
  "Overwrite",
  "NoOKCheck",                  /* MR 970720  40 */
  "ReqOnUs",
  "LockBaud",
  "TimeOut",
  "NoSLO",
  "SlowModem",
  "SmallWindow",
  "NoPickup",
  "NoRequests",
  "NetMail",
  "Suffix",
  "SysopNDX",
  "AutoBaud",
  "Gong",
  "NoCollide",
  "NoDirZap",
  "DebugFlags",
  "Outbound",
  "ExtrnMail",
  "BBSNote",
  "BBS",
  "ScriptPath",
  "BoxType",
  "Inbound",
  "Include",
  "CaptureFile",
  "CursorCol",
  "CursorRow",
  "DoingMail",
  "EnterBBS",
  "PrivateNet",
  "Packer",
  "Cleanup",
  "AfterMail",
  "Colors",
  "JanusBaud",
  "ReqTemplate",
  "KnownAvail",
  "KnownReqList",
  "KnownAbout",
  "KnownInbound",
  "KnownReqLim",
  "KnownReqTpl",
  "ProtAvail",
  "ProtReqList",
  "ProtAbout",
  "ProtInbound",
  "ProtReqLim",
  "ProtReqTpl",
  "NodeTimeZone",               /* MR 980104 */
  "NoZones",                    /* obsolete! TJW970202 */
  "Answer",
  "PollTries",
  "SwapDir",
  "Address",
  "CurMudgeon",
  "NoWaZOO",
  "ScreenBlank",
  "Mark_Kromm",
  "Server",
  "ModemTrans",
  "PreDial",
  "PreInit",
  "DTRHigh",
  "Debug",
  "NoZedZap",
  "NoResync",
  "NoSEAlink",
  "FTS-0001",
  "LineUpdate",
  "JanusOK",
  "TermInit",
  "DomainKludge",
  "Snoop",
  "Flags",
  "NoHydraChat",                /* TJW 960804 */
  "MaxBytes",
  "KnownMaxBytes",
  "ProtMaxBytes",
  "StartBlkLen",
  "MaxTime",
  "KnownMaxTime",
  "ProtMaxTime",
  "RingTries",
  "NoSharing",
  "Version7",
  "BlankWait",
  "Serial",
  "NoDietIfna",
  "Version6",                   /* obsolete! HJK - 99/02/28 */
  "WinSlice",
  "NoSize",
  "ForcExit",
  "Extern",
  "NoEMSI",
  "MyLocation",
  "MyPhone",
  "MyListFlags",
  "MyMaxbaud",
  "PickUpAll",
  "FileSec",
  "KnownSec",
  "ProtSec",
  "MaxAreas",
  "Domain",
  "MailSound",
  "FileSound",
  "BBSSound",
  "EXTSound",
  "StartSound",
  "FaxSound",
  "FaxInDir",
  "ModemFailure",
  "ModemIgnore",
  "ModemConnect",
  "ModemRinging",
  "ModemIncoming",
  "ModemFax",
  "Costlog",
  "CostUnit",
  "EuroCost",
  "NoFilter",
  "BiDiOK",
  "BiDiBaud",
  "NoHydra",
  "NoJanus",
  "ExtBaudRates",
  "AfterCall",
  "PktRsp",
  "ExtSession",
  "FaxBaud",
  "ErrLevelShell",
  "AltNumber",
  "ModemRetry",
  "RecentActivityLines",
  "RingWait",
  "FreqLongNames",              /* HJK - 99/01/22 */
                                /* TJW 960414 single config file for n tasks */
  "[",                          /* section begin with expression             */

  "TaskPath",                   /* TJW 235 178 */
  "ShowAlive",                  /* RDH *** 9604 179 */
  "ReadHoldTime",               /* mtt 96-04-16 180 */
  "ReInitTime",                 /* mtt 96-04-16 181 */
  "ConditionalPoll",            /* CE/TJW 960425 182 */
  "Reject",                     /* CE/TJW 960425 183 */
  "ProtocolPreference",         /* TJW 960719 184 */
  "PutEnv",                     /* MMP 185 */
  "SpawnInit",                  /* MMP 186 */
  "LocalLog",                   /* MMP 187 */
  "ShortCostLog",               /* MMP 188 */
  "TimeSync",                   /* MMP 189 */
  "MailFlag",                   /* MMP 190 */
  "MakeDir",                    /* TJW 191 *//* obsolete */
  "SRIF",                       /* MTT 192 */
  "AfterConnect",               /* PE 193 */
  "MaxBusyAge",                 /* MMP 194 */
  "SpawnNoOK",                  /* MMP 195 */
  "CacheHold",                  /* MMP 196 */
  "PBAreas",                    /* TE 960516 197 */
  "ModemReject",                /* CE 198 */
  "AfterCallOut",               /* CE 199 */
  "ChangeMailTo",               /* CE 200 */

  "CostTimeCorrection",         /* TJW 201 */
  "IPC",                        /* TJW 202 */
  "ReadLog",                    /* TJW 203 */
  "EMSIbanner",                 /* TJW 204 */
  "CostCPS",                    /* TJW 205 */
  "MCPpipe",                    /* TJW 206 */

  "WindowTitleFmt",             /* TJW 207 */
  "OutboundRescanPeriod",       /* TJW 208 */
  "OutboundCache",              /* TJW 209 */

  "CheckPath",                  /* CE 210 */
  "AKAMatching",                /* CE 211 */
  "NodeInboundDir",             /* CE 212 */
  "SyncHydraSessions",          /* CE 213 */
  "NoModem",                    /* CE 214 */

  "HydraMsgSend",               /* mtt 215 */
  "MTrsrvd216",                 /* mtt 216 */
  "MTrsrvd217",                 /* mtt 217 */
  "MTrsrvd218",                 /* mtt 218 */
  "MTrsrvd219",                 /* mtt 219 */

  "SmartBiDi",                  /* R.Hoerner 220 */
  "RHrsrvd221",                 /* RH 221 */
  "RHrsrvd222",                 /* RH 222 */
  "RHrsrvd223",                 /* RH 223 */
  "RHrsrvd224",                 /* RH 224 */

  "TJWrsrvd225",                /* TJW 225 */
  "TJWrsrvd226",                /* TJW 226 */
  "TJWrsrvd227",                /* TJW 227 */
  "TJWrsrvd228",                /* TJW 228 */
  "TJWrsrvd229",                /* TJW 229 */

  "NoANSITrash",                /* TS  230 */
  "SIOMode",                    /* CFS 231 */
  "IPRepChar",                  /* TE 232  */
  "TErsrvd233",                 /* TE 233  */
  "NoZedZip",                   /* JNK 970615 234 */

  "BannerCID",                  /* CFS 235 */
  "ModemCID",                   /* CFS 236 */
  "StringRep",                  /* CFS 237 */
  "Callback",                   /* CFS 238 */
  "Nohundredths",               /* CFS 239 */

  "Assumebaud",                 /* R.Hoerner 240 */
  "Clock",                      /* R.Hoerner 241 */
  "BitType",                    /* R.Hoerner 242 */
  "ReDialTime",                 /* R.Hoerner 243 */
  "EmsiLog",                    /* R.Hoerner 244 */
  "NoErrDelay",                 /* R.Hoerner 245 */
  "BinkDir",                    /* R.Hoerner 246 */
  "ShowDomains",                /* R.Hoerner 247 */
  "PassWordFile",               /* R.Hoerner 248 */
  "ForcedRescan",               /* R.Hoerner 249 */
  "NoFancyStrings",             /* R.Hoerner 250 */
  "MaxRingWait",                /* R.Hoerner 251 */
  "HoldAfterBadConnect",        /* R.Hoerner 252 - AW obsolete! */
  "ShowPassword",               /* R.Hoerner 253 */
  "Override",                   /* R.Hoerner 254 */
  "NoWildCards",                /* R.Hoerner 255 */
  "PipeTo",                     /* R.Hoerner 256 */
  "DelBadCall",                 /* R.Hoerner 257 */
  "EventFile",                  /* R.Hoerner 258 */
  "SharePort",                  /* SM 970102 259 */
  "MaxPrm",                     /* AW 980219 260 */
  "ModemRing",                  /* SM        261 */
  "CfosCost",                   /* R.Hoerner 262 */

  "ModemCostReset",             /* CFS       263 */
  "StatsToMSG",                 /* CFS       264 */
  "UseCompletePaths",           /* MR 970213 265 */
  "ShowModemWin",               /* MR 970215 266 */
  "NoTranx",                    /* JNK 970312 267 */
  "CAPICID",                    /* CFS       268 */
  "CFOSLine",                   /* CFS       269 */
  "PreAnswer",                  /* MR 970310 270 */
  "CallerID",                   /* MR 970324 271 */
  "StringRepModem",             /* CFS       272 */
  "HangUp",                     /* MR        273 */
  "AutoChatOpen",               /* CFS       274 */
  "ChatLogDir",                 /* CFS       275 */
  "NodeExtraDir",               /* CFS       276 */
  "RandomSystem",               /* CFS       277 */
  "RandomSysop",                /* CFS       278 */
  "RandomMyLocation",           /* CFS       279 */
  "ShortHistory",               /* CFS       280 */
  "StatsDef",                   /* CFS       281 */
  "RejectIgnoreSize",           /* JH        282 */
  "ReadFlagTime",               /* CEH       283 */
  "LocalInbound",               /* CEH       284 */
  "OnLine",                     /* VRP 990820 285 */
  "SendHoldIfCaller",           /* VRP 990824 286 */
  "RestorePoll",                /* VRP 990826 287 */
  "Hidden",                     /* VRP 990830 288 */
  "NetMailFlag",                /* VRP 990918 289 */
  "ArcMailFlag",                /* VRP 990918 290 */
  "TicFlag",                    /* VRP 990918 291 */
  "SaveExtSession",             /* VRP 990920 292 */
  NULL
};


KWD_ERR
parse_config (char *config_file)
{
  FILE *stream;
  char *temp;
  char *c = NULL;               /* TS 960620  since maybe uninitialized */
  char tmpfname[PATHLEN];
  char buf[CFGLINEBUFSIZE];
  int i;
  int linecounter = 0;
  int usecfglines = 1;          /* TJW 960414 use config lines for this task ? init: yes */

  if (config_file == NULL)
    return kwd_bad;

  /* MR 961122 do not add BINKpath if config_file has a complete path
   * MR 970617 or if we are using UNCs */
  /* CEH 990110 Only add BINKpath if NO path is in config_file */

  if (strchr (config_file, DIR_SEPC) == NULL)
    sprintf (tmpfname, "%s%s", BINKpath, config_file);
  else
    sprintf (tmpfname, "%s", config_file);

  /* OK, let's open the file */
  if ((stream = share_fopen (tmpfname, read_ascii, DENY_WRITE)) == NULL)
  {
    printf ("ERR: cannot open configuration file %s\n", tmpfname);
    return kwd_bad;             /* no file, no work to do */
  }

  printf ("Reading configuration file %s...\n", tmpfname);

  /* MMP 960419. cfgfgets() substitutes environment variables, removes
   * comments, does conditional inclusion of lines, and truncates long lines */

  /* Now we parse the file ... */

  while ((temp = cfgfgets (buf, stream)) != NULL)  /* while not EOF */
  {
    linecounter++;              /* TJW 960524 */

    if (strlen (temp) < 3)      /* If too short, ignore it */
      continue;

    if ((i = parse (temp, config_lines)) != -1)
    {
      c = &temp[strlen (config_lines[i - 1])];
      c = skip_blanks (c);
    }

    switch (i)                  /* TJW 960414                    */
    {
    case 177:                  /* TJW 960830 [expression]       */
      {                         /* MR  970113 added multiple expressions */
        char *p;
        int use = TRUE;         /* use = -1 if error                     */
        int andop = -1;         /* operator - FALSE = OR, TRUE = AND     */

        killblanks (c);         /* remove blanks, tabs..                 */

        do
        {
          p = strchr (c, ']');
          if (p == NULL)
            use = -1;
          else
          {
            *p = '\0';          /* if there is a ] then kill it          */
            ++p;                /* save next pos                         */
            use = eval (c);
            if (use >= 0)       /* if expr = 1 then use lines            */
            {
              if (andop == -1)
                usecfglines = use;  /* first value ok */
              else if (andop == FALSE)
                usecfglines = usecfglines || use;  /* add use with OR */
              else
                usecfglines = usecfglines && use;  /* add use with AND */

              if (*p)           /* still something after ']'?      */
              {
                if ((p[0] == '&') && (p[1] == '['))  /* AND */
                  andop = TRUE;
                else if ((p[0] == '|') && (p[1] == '['))  /* OR */
                  andop = FALSE;
                else
                  use = -1;
                c = p + 2;
              }
              else
                c = NULL;
            }
          }
        }
        while ((c != NULL) && (use >= 0));

        if (use == -1)
        {
          printf (MSG_TXT (M_UNKNOWN_LINE), temp);
          bad_line (linecounter, tmpfname);
        }
      }
      break;

    default:
      if (usecfglines)
      {
        /* MR 970326 split up config parser */
        switch (parse_keyword (i, c))
        {
        case kwd_ok:           /*  TS 970328, this enum was not handled    */
          break;

        case kwd_bad:
          printf (MSG_TXT (M_UNKNOWN_LINE), temp);
          bad_line (linecounter, tmpfname);
          break;

        case kwd_unknown:
          printf ("ERR: %s: unsupported in this version\n", config_lines[i - 1]);
          bad_line (linecounter, tmpfname);
          break;

        case kwd_nomem:
          printf ("ERR: %s: cannot allocate memory\n", config_lines[i - 1]);
          bad_line (linecounter, tmpfname);
          break;

        case kwd_other_err:
          bad_line (linecounter, tmpfname);
          break;

        }
      }
    }
  }

  fclose (stream);              /* close input file */

  if (debugging_log)
    loglevel = loglevel < 6 ? 6 : loglevel;  /* allow loglevel 7 AND debug */

  return kwd_ok;
}


static KWD_ERR LOCALFUNC
parse_keyword (int keyword, char *c)
{
  int itemp;
  long ltemp;
  char *p = NULL;
  char *p2 = NULL;

#ifdef OS_2
  char *p3 = NULL;

#endif
  char junk[256];
  KWD_ERR error = kwd_ok;

  switch (keyword)
  {
  case 1:                      /* "SameRing"               */
    def_mdm_resps[1].disp = IGNORE;
    break;


  case 2:                      /* obsolete                 */
    error = kwd_unknown;
    break;


  case 3:                      /* obsolete                 */
    error = kwd_unknown;
    break;


  case 4:                      /* "Answerback"             */
    error = cfg_CopyString (keyword, &answerback, c);
    break;


  case 5:                      /* "Macro"                  */
    cfg_macro (c);
    break;


  case 6:                      /* "Shell"                  */
    itemp = atoi (c);
    if ((itemp <= 0) || (itemp > 9))
    {
      printf ("ERR: Shell: %s %d %s\n", MSG_TXT (M_SHELL_NUMBER),
              itemp, MSG_TXT (M_OUT_OF_RANGE));
      error = kwd_other_err;
      break;
    }
    c = skip_to_word (c);
    shells[itemp - 1] = calloc (1, strlen (c) + 1);
    strcpy (shells[itemp - 1], c);
    // strupr (shells[itemp - 1]);
    break;


  case 7:                      /* "Dial"                   */
    cfg_dial_string (c);
    break;


  case 8:                      /* "Event"                  */
    error = parse_event (c);
    break;


  case 9:                      /* obsolete                 */
    error = kwd_unknown;
    break;

  case 10:                     /* "MaxReq"                 */
    sscanf (c, "%d %d", &DEFAULT.rq_Limit, &DEFAULT.rq_Cum_Limit);
    if (!DEFAULT.rq_Limit)      /* No requests???       */
    {
      printf ("Note: MaxReq: 0 %s\n", MSG_TXT (M_REQUESTS_ALLOWED));
      error = kwd_other_err;
    }
    break;


  case 11:                     /* "LogLevel"               */
    itemp = atoi (c);
    if (abs (itemp) <= 7)       /* r.hoerner: allowed LOGLEVEL 0  */
    {
      loglevel = itemp;
    }
    else
    {
      printf ("ERR: LogLevel: %s %s\n", MSG_TXT (M_BAD_LOGLEVEL), c);
      error = kwd_other_err;
    }
    break;


  case 12:                     /* "Baud"                   */
    ltemp = atol (c);
    max_baud.rate_value = 0;
    for (itemp = 0; pbtypes[itemp].rate_value; itemp++)
    {
      if (pbtypes[itemp].rate_value == (unsigned long) ltemp)
      {
        max_baud.rate_mask = pbtypes[itemp].rate_mask;
        max_baud.rate_value = ltemp;
        break;
      }
    }
    if (!max_baud.rate_value)
      error = kwd_bad;
    break;


  case 13:                     /* "MaxPort"                */
#ifndef __unix__
    itemp = atoi (c);           /* make it binary           */
    if ((itemp > 0) && (itemp < 33))  /* if range is reasonable,  */
      MAXport = itemp;          /* Make it the max value    */
    else
    {
      printf ("ERR: MaxPort: %s %s\n", MSG_TXT (M_BAD_MAXPORT), c);
      error = kwd_other_err;
    }
#else
    printf ("ERR: MaxPort: ignored in linux-version");
#endif
    break;


  case 14:                     /* "Port"                   */
    itemp = atoi (c);           /* make it binary */

    /* r. hoerner: allow "Port COM3" i.e. "Port <device><number>"       */
    /* you can't use "port \\PIPE\ME", you can use "port \\PIPE\COM2"   */
    /* but it will not work at all ;)                                   */

#ifdef __unix__

    port_ptr = 0;
    original_port = 0;

    error = cfg_CopyString (keyword, &port_device, c);

    p = port_device;
    if (p == NULL)
      break;

    while (*p)
    {
      if (*p == ';' || *p == '%' || *p == ' ')
        *p = 0;
      if (*p)
        p++;
    }

    port_ptr = 0;
    original_port = 0;
    // printf("PORT: Using [%s]\n",port_device);
    break;

#else

    if (strlen (c) > 2)
    {
      port_ptr = 0;
      original_port = 0;

      error = cfg_CopyString (keyword, &port_device, c);

      p = port_device;
      if (p == NULL)
        break;

      while (*p)
      {
        if (*p == ';' || *p == '%' || *p == ' ')
          *p = 0;
        if (*p)
          p++;
      }

      if (isdigit (port_device[strlen (port_device) - 1]))
      {
        p = port_device;        /* start with "COM12"       */
        while (!isdigit (*p))   /* search the     ^^        */
          p++;
        itemp = atoi (p);       /* p = "12" -> i = 12       */
        *p = 0;                 /* "COM12" -> "COM"         */
      }
      else
        itemp = 255;            /* force error              */
    }
    else
      port_device = ctl_string ("COM");

    if ((itemp > 0) || (itemp < 33))  /* see if it's OK           */
    {
      port_ptr = itemp - 1;     /* store it if so           */
      if (MAXport < itemp)      /* See if MAXport is less   */
        MAXport = itemp;        /* If so, increase MAXport  */
      original_port = port_ptr;
    }
    else
    {
      printf ("ERR: Port: %s %s\n", MSG_TXT (M_ILLEGAL_PORT), c);
      error = kwd_other_err;
    }
    break;

#endif

  case 15:                     /* "Carrier"                */
    itemp = 0;
    sscanf (c, "%x", &itemp);   /* convert it to binary     */
    if (itemp != 0)             /* if we got anything       */
      carrier_mask = (unsigned int) itemp;
    else
    {
      printf ("ERR: Carrier: %s %s\n", MSG_TXT (M_ILLEGAL_CARRIER), c);
      error = kwd_other_err;
    }
    break;


  case 16:                     /* "StatusLog"              */
    error = cfg_CopyString (keyword, &log_name, c);

    if (binkdir && (log_name != NULL))
    {
      p = log_name;
      p += strlen (log_name);
      while ((p != log_name) && (*p != DIR_SEPC))
        p--;
      if (p == log_name)
        break;
      *p = 0;
      p2 = ctl_slash_string (binkdir, log_name);  /* create directory    */
      free (p2);                /* MR 970327 and free  */
      *p = DIR_SEPC;
    }
    break;


  case 17:                     /* "Reader"                 */
    error = cfg_CopyString (keyword, &BBSreader, c);
    break;


  case 18:                     /* "BossPhone"              */
    error = cfg_CopyString (keyword, &BOSSphone, c);
    break;


  case 19:                     /* "BossPwd"                */
    error = cfg_CopyString (keyword, &BOSSpwd, c);
    break;


  case 20:                     /* "Protocol"               */
    if (extern_index == 5)
    {
      printf ("ERR: Protocol: more then 5 protocols, skipped %s\n", c);
      error = kwd_other_err;
      break;
    }

    protocols[extern_index] = ctl_string (c);
    if (protocols[extern_index] != NULL)
      extern_index++;
    break;


  case 21:                     /* "System"                 */
    error = cfg_CopyString (keyword, &def_system_name, c);
    system_name = def_system_name;
    break;


  case 22:                     /* "Sysop"                  */
    error = cfg_CopyString (keyword, &def_sysop, c);
    sysop = def_sysop;
    break;


  case 23:                     /* "Boss"                   */
    parse_address (c, &boss_addr);
    break;


  case 24:                     /* obsolete, was: "Point"   */
  case 25:                     /* obsolete, was: "Aka"     */
    error = kwd_unknown;
    break;


  case 94:                     /* "Address"                */
    error = cfg_insert_AKA (c);
    /* Zone = alias->Zone;    First is real default, removed: TJW970202 */
    break;


  case 26:                     /* "Hold"                   */
  case 57:                     /* "Outbound"               */
    error = cfg_CopySlashString (keyword, &hold_area, c);

    if (hold_area != NULL)
    {
      domain_area = strdup (hold_area);
      domain_area[strlen (domain_area) - 1] = '\0';
      p = strrchr (domain_area, '\\');
      if (p == NULL)
      {
        p = strrchr (domain_area, '/');
        if (p == NULL)
          p = domain_area;
/*
 * CEH 19990514:
 * domain_area has a '\\' (or '/') at the end (cfg_CopySlashString adds this if
 * neccessary), so it we don't have to look for ':'
 *
 *         {
 *           p = strrchr (domain_area, ':');
 *           if (p == NULL)
 *             p = domain_area;
 *         }
 */
      }
      if (p != domain_area)
        ++p;
/*
 * CEH 19990514: domain_loc is no longer needed.
 *
 *       domain_loc = p;
 */
      *p = '\0';
    }
    break;


  case 27:                     /* "Downloads"              */
    error = cfg_CopySlashString (keyword, &download_path, c);
    break;


  case 28:                     /* "NetFile"                */
  case 63:                     /* "Inbound"                */
    error = cfg_CopySlashString (keyword, &DEFAULT.sc_Inbound, c);
    break;


  case 29:                     /* "Init"                   */
    if (modem.init_cnt < 4)
      modem.init[modem.init_cnt++] = ctl_string (c);
    else
    {
      printf ("ERR: Init: too many init strings, skipped %s\n", c);
      error = kwd_other_err;
    }
    break;


  case 30:                     /* "Busy"                   */
    error = cfg_CopyString (keyword, &modem.busy, c);
    break;


  case 31:                     /* "Prefix"                 */
    error = cfg_CopyString (keyword, &modem.predial, c);
    modem.normprefix = modem.predial;
    break;


  case 32:                     /* "NodeList"               */
    error = cfg_CopySlashString (keyword, &net_info, c);
    break;


  case 33:                     /* "Avail"                  */
#ifdef SPECIALMAGICS
    error = cfg_CopyString (keyword, &DEFAULT.rq_FILES, c);
#else
    error = kwd_unknown;
#endif
    break;


  case 34:                     /* "OKFile"                 */
    error = cfg_CopyString (keyword, &DEFAULT.rq_OKFile, c);
    break;


  case 35:                     /* "About"                  */
#ifdef SPECIALMAGICS
    error = cfg_CopyString (keyword, &DEFAULT.rq_About, c);
#else
    error = kwd_unknown;
#endif
    break;


  case 36:                     /* "MAILnote"               */
    error = cfg_CopyString (keyword, &MAILnote, c);
    break;


  case 37:                     /* "Banner"                 */
    error = cfg_CopyString (keyword, &BBSbanner, c);
    break;


  case 38:                     /* "UnAttended"             */
    if (!command_line_term)
    {
      un_attended = 1;
      command_line_un = 1;
    }
    break;


  case 39:                     /* "OverWrite"              */
    overwrite = 1;
    break;


  case 40:                     /* MR 970720 "NoOKCheck"    */
    modem.no_okcheck = 1;
    break;


  case 41:                     /* "ReqOnUs"                */
    error = cfg_set_nodeflag (&on_our_nickel, FREEREQ, c);  /* r. hoerner */
    break;


  case 42:                     /* "LockBaud"               */
    if (*c == '\0')             /* Case 1: no arg           */
    {
      lock_baud = 1;            /* Always lock              */
      break;
    }

    if (isdigit (*c))           /* Case 2: baud             */
    {
      lock_baud = atol (c);     /* Lock at >= baud          */
      break;
    }

    if (ARQs > 16)              /* Only 16 allowed          */
      break;

    p = ctl_string (c);         /* Case 3: ARQ              */
    if (p == NULL)              /* Give up if none          */
    {
      error = kwd_nomem;
      break;
    }

    ARQ[ARQs++] = p;            /* Update array            */
    break;


  case 43:                     /* "TimeOut"                */
    BBStimeout = atoi (c);
    if (BBStimeout < 20)
      BBStimeout = 20;
    if (BBStimeout > 655)
      BBStimeout = 655;
    BBStimeout *= PER_SECOND;
    break;


  case 44:                     /* "NoSLO"                  */
    no_overdrive = 1;
    break;


  case 45:                     /* "SlowModem"              */
    slowmodem = 1;
    break;


  case 46:                     /* "SmallWindow"            */
    small_window = 1;
    break;


  case 47:                     /* "NoPickup"               */
    error = cfg_set_nodeflag (&no_pickup, NOPICK, c);  /* r. hoerner */
    break;


  case 48:                     /* "NoRequests"             */
    error = cfg_set_nodeflag (&no_requests, NOREQ, c);  /* r. hoerner */
    break;


  case 49:                     /* "NetMail"                */
    if (*c != '$')
      error = cfg_CopySlashString (keyword, &netmail, c);
    else
      error = cfg_CopyString (keyword, &netmail, c);
    break;


  case 50:                     /* "Suffix"                 */
    error = cfg_CopyString (keyword, &modem.postdial, c);
    modem.normsuffix = modem.postdial;
    break;


  case 51:                     /* "SysopNDX"               */
    sysopndx = 1;
    break;


  case 52:                     /* "AutoBaud"               */
    autobaud = 1;
    break;


  case 53:                     /* "Gong"                   */
    error = cfg_set_nodeflag (&gong_allowed, DO_BELL, c);  /* r. hoerner */
    break;


  case 54:                     /* "NoCollide"              */
    no_collide = 1;
    break;


  case 55:                     /* TJW970714 "NoDirZap"     */
    no_dirzap = 1;              /* Todo: node dependant switch */
    break;


  case 56:                     /* TJW970715 "DebugFlags"   */
    sscanf (c, "%lx", &debugflags);  /* temporary debug bit flags */
    break;


  case 58:                     /* "ExtrnMail <errorlevel> <string>"    */
    if (!isdigit (*c))          /* changed 8/1/97            r. hoerner */
    {
      error = kwd_bad;
      break;
    }

    lev_ext_mail[num_ext_mail] = atoi (c);
    c = skip_to_blank (c);
    ext_mail_string[num_ext_mail] = ctl_string (c);
    if (ext_mail_string[num_ext_mail] != NULL)
      num_ext_mail++;
    break;


  case 59:                     /* "BBSnote"                */
    error = cfg_CopyString (keyword, &BBSnote, c);
    break;


  case 60:                     /* "BBS"                    */
    if (strnicmp (c, "exit", 4) == 0)
      BBSopt = OPT_EXIT;
    else if (strnicmp (c, "batch", 5) == 0)
      BBSopt = OPT_BATCH;
    else if (strnicmp (c, "spawn", 5) == 0)
      BBSopt = OPT_SPAWN;
#ifdef BBS_INTERNAL
    else if (strnicmp (c, "internal", 5) == 0)
      BBSopt = OPT_INTERNAL;
#endif
    else
      error = kwd_bad;
    break;


  case 61:                     /* "ScriptPath"             */
    error = cfg_CopySlashString (keyword, &script_path, c);
    break;


  case 62:                     /* "BoxType"                */
    boxtype = (short) atoi (c);
    if ((boxtype > 4) || (boxtype < 0))
      boxtype = 1;
    break;

  case 64:                     /* "Include"                */
    p = ctl_string (c);         /* TJW 960414 evaluate %xy% */
    error = parse_config (p);   /* in included files        */
    free (p);                   /* MR 970326 added free     */
    break;


  case 65:                     /* "CaptureFile"            */
    error = cfg_CopyString (keyword, &logptr, c);
    break;


  case 66:                     /* "CursorCol"              */
    cursor_col = atoi (c) - 1;
    break;


  case 67:                     /* "CursorRow"              */
    cursor_row = atoi (c) - 1;
#ifdef DOS16                    /*  TS 970602   */
    if ((cursor_row + 1) > getbiosscreenrows ())
      cursor_row = getbiosscreenrows () - 1;
#endif
    break;


  case 68:                     /* "DoingMail"              */
    sprintf (junk, "\r\r%s\r\r", c);
    noBBS = calloc (1, strlen (junk) + 1);
    strcpy (noBBS, junk);
    break;


  case 69:                     /* "EnterBBS"               */
    sprintf (junk, "\r%s\r", c);
    BBSesc = calloc (1, strlen (junk) + 1);
    strcpy (BBSesc, junk);
    break;


  case 70:                     /* "PrivateNet"             */
    pvtnet = (short) atoi (c);
    break;


  case 71:                     /* "Packer"                 */
    error = cfg_CopyString (keyword, &packer, c);
    break;


  case 72:                     /* "Cleanup"                */
    error = cfg_CopyString (keyword, &cleanup, c);
    break;


  case 73:                     /* "AfterMail"              */
    if (!memicmp (c, "quick", 5) && (isspace (c[5]) || c[5] == '\0'))
      quickaftermail = 1, c += 5;
    error = cfg_CopyString (keyword, &aftermail, c);
    break;


  case 74:                     /* "Colors"                 */
    sscanf (c, "%hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd",
            &colors.background, &colors.settings, &colors.history,
            &colors.hold, &colors.call, &colors.file,
            &colors.calling, &colors.popup,
            &colors.headers, &colors.frames,  /* TJW 960428 do it like EE */
            &colors.mw_sent, &colors.mw_rcvd  /* MR 970310 */
      );
    break;


  case 163:                    /* "BiDiBaud"               */
  case 75:                     /* "JanusBaud"              */
    janus_baud = atol (c);
    break;


  case 76:                     /* "ReqTemplate"            */
    error = cfg_CopyString (keyword, &DEFAULT.rq_Template, c);
    break;


  case 77:                     /* "KnownAvail"             */
#ifdef SPECIALMAGICS
    error = cfg_CopyString (keyword, &KNOWN.rq_FILES, c);
#else
    error = kwd_unknown;
#endif
    break;


  case 78:                     /* "KnownReqList"           */
    error = cfg_CopyString (keyword, &KNOWN.rq_OKFile, c);
    break;


  case 79:                     /* "KnownAbout"             */
#ifdef SPECIALMAGICS
    error = cfg_CopyString (keyword, &KNOWN.rq_About, c);
#else
    error = kwd_unknown;
#endif
    break;


  case 80:                     /* "KnownInbound"           */
    error = cfg_CopySlashString (keyword, &KNOWN.sc_Inbound, c);
    break;


  case 81:                     /* "KnownReqLim"            */
    sscanf (c, "%d %d", &KNOWN.rq_Limit, &KNOWN.rq_Cum_Limit);
    break;


  case 82:                     /* "KnownReqTpl"            */
    error = cfg_CopyString (keyword, &KNOWN.rq_Template, c);
    break;


  case 83:                     /* "ProtAvail"              */
#ifdef SPECIALMAGICS
    error = cfg_CopyString (keyword, &PROT.rq_FILES, c);
#else
    error = kwd_unknown;
#endif
    break;


  case 84:                     /* "ProtReqList"            */
    error = cfg_CopyString (keyword, &PROT.rq_OKFile, c);
    break;


  case 85:                     /* "ProtAbout"              */
#ifdef SPECIALMAGICS
    error = cfg_CopyString (keyword, &PROT.rq_About, c);
#else
    error = kwd_unknown;
#endif
    break;


  case 86:                     /* "ProtInbound"            */
    error = cfg_CopySlashString (keyword, &PROT.sc_Inbound, c);
    break;


  case 87:                     /* "ProtReqLim"             */
    sscanf (c, "%d %d", &PROT.rq_Limit, &PROT.rq_Cum_Limit);
    break;


  case 88:                     /* "ProtReqTpl"             */
    error = cfg_CopyString (keyword, &PROT.rq_Template, c);
    break;


  case 89:                     /* MR 980104 NodeTimeZone   */
    error = cfg_set_nodetimezone (c);
    break;


  case 90:                     /* obsolete "NoZones"       */
    error = kwd_unknown;
    // no_EMSI = 1;            /* no zones means no EMSI */
    // no_zones = 1;
    break;


  case 91:                     /* "Answer"                 */
    error = cfg_CopyString (keyword, &modem.answer, c);
    break;


  case 92:                     /* "PollTries"              */
    poll_tries = atoi (c);
    break;


  case 93:                     /* "SwapDir"                */
    error = cfg_CopySlashString (keyword, &swapdir, c);
    break;


  case 95:                     /* "Curmudgeon"             */
    curmudgeon = 1;             /* Do it like Randy         */
    break;


  case 96:                     /* "NoWaZOO"                */
    error = cfg_set_nodeflag (&no_WaZOO, NOWAZOO, c);  /* r. hoerner */
    break;


  case 97:                     /* "ScreenBlank"            */
    do_screen_blank = 1;
    if (strnicmp (c, MSG_TXT (M_KEY), 3) == 0)
      blank_on_key = 1;
    else if (strnicmp (c, MSG_TXT (M_CALL), 4) == 0)
      blank_on_key = 0;
    break;


  case 98:                     /* "Mark_Kromm"             */
    colors.background = 112;    /* The                      */
    colors.settings = 9;        /* Great                    */
    colors.history = 10;        /* BinkleyTerm              */
    colors.hold = 11;           /* Colors                   */
    colors.call = 14;           /* Contest                  */
    colors.file = 12;           /* Winner!!!!               */
    colors.calling = 56;        /* These colors from        */
    colors.popup = 15;          /* Mark Kromm, Fidonet      */
    break;                      /* 1:261/1034               */


  case 99:                     /* "Server"                 */
    server_mode = 1;
    break;


  case 100:                    /* "ModemTrans"             */
    cfg_mdm_trans (c);
    break;


  case 101:                    /* "PreDial"                */
    error = cfg_CopyString (keyword, &modem.dial_setup, c);
    break;


  case 102:                    /* "PreInit"                */
    error = cfg_CopyString (keyword, &modem.init_setup, c);
    break;


  case 103:                    /* "DTRHigh"                */
    // leave_dtr_high = 1;
    error = kwd_unknown;
    break;


  case 104:                    /* "Debug"                  */
    debugging_log = 1;
    break;


  case 105:                    /* "NoZedZap"               */
    error = cfg_set_nodeflag (&no_zapzed, NOZED, c);  /* r. hoerner */
    break;


  case 106:                    /* "NoResync"               */
    no_resync = 1;
    break;


  case 107:                    /* "NoSEAlink"              */
    error = cfg_set_nodeflag (&no_sealink, NOSEA, c);  /* r. hoerner */
    break;


  case 108:                    /* "FTS-0001"               */
    error = cfg_set_nodeflag (&no_sealink, NOSEA, c);  /* r. hoerner */
    error = cfg_set_nodeflag (&no_WaZOO, NOWAZOO, c);  /* r. hoerner */
    error = cfg_set_nodeflag (&no_EMSI, NOEMSI, c);  /* r. hoerner */
    no_resync = 1;
    no_overdrive = 1;
    break;


  case 109:                    /* "LineUpdate"             */
    immed_update = 1;
    break;


  case 162:                    /* "BiDiOK"                 */
  case 110:                    /* "JanusOK"                */
    cfg_Janus (c);
    break;


  case 111:                    /* "TermInit"               */
    if (modem.terminit_cnt < 4)
      modem.terminit[modem.terminit_cnt++] = ctl_string (c);
    else
      printf ("ERR: TermInit: too many init strings, skipped %s\n", c);
    break;


  case 112:                    /* "DomainKludge"           */
    error = cfg_domain_kludge (c);
    break;


  case 113:                    /* "Snoop"                  */
#if defined(OS_2) | defined(_WIN32)
    /* r.hoerner: name in cfgfile should override environment name  */
    error = cfg_CopyString (keyword, &pipename, c);

    if (pipename == NULL)
    {
      if (getenv ("SNOOPPIPE") != NULL)
        pipename = ctl_string (getenv ("SNOOPPIPE"));
    }

    if (pipename == NULL)
    {
      printf ("ERR: Snoop: no name defined, ignored\n");
      error = kwd_other_err;
    }
#else
    error = kwd_unknown;
#endif /* Snoop                    */
    break;


  case 114:                    /* "Flags"                  */
    error = cfg_CopySlashString (keyword, &flag_dir, c);
    break;


  case 115:                    /* "NoHydraChat"            */
    error = cfg_set_nodeflag (&nohydrachat, NOHYCHAT, c);  /* r. hoerner */
    break;


  case 116:                    /* "MaxBytes"               */
    sscanf (c, "%ld %ld", &DEFAULT.byte_Limit, &DEFAULT.byte_Cum_Limit);
    break;


  case 117:                    /* "KnownMaxBytes"          */
    sscanf (c, "%ld %ld", &KNOWN.byte_Limit, &KNOWN.byte_Cum_Limit);
    break;


  case 118:                    /* "ProtMaxBytes"           */
    sscanf (c, "%ld %ld", &PROT.byte_Limit, &PROT.byte_Cum_Limit);
    break;


  case 119:                    /* "StartBlkLen"            */
    fstblklen = atoi (c);
    /* if ((fstblklen < 64) || (fstblklen > 2048)) */
    if ((fstblklen < 64) || (fstblklen > 8192))
      fstblklen = 1024;
    break;


  case 120:                    /* "MaxTime"                */
    sscanf (c, "%d %ld", &DEFAULT.time_Limit, &DEFAULT.time_Cum_Limit);
    DEFAULT.time_Limit *= PER_MINUTE / PER_SECOND;
    DEFAULT.time_Cum_Limit *= PER_MINUTE / PER_SECOND;
    break;


  case 121:                    /* "KnownMaxTime"           */
    sscanf (c, "%d %ld", &KNOWN.time_Limit, &KNOWN.time_Cum_Limit);
    KNOWN.time_Limit *= PER_MINUTE / PER_SECOND;
    KNOWN.time_Cum_Limit *= PER_MINUTE / PER_SECOND;
    break;


  case 122:                    /* "ProtMaxTime"            */
    sscanf (c, "%d %ld", &PROT.time_Limit, &PROT.time_Cum_Limit);
    PROT.time_Limit *= PER_MINUTE / PER_SECOND;
    PROT.time_Cum_Limit *= PER_MINUTE / PER_SECOND;
    break;


  case 123:                    /* "RingTries"              */
    ring_tries = atoi (c);
    break;


  case 124:                    /* "NoSharing"              */
    no_sharing = 1;
    break;


  case 125:                    /* "Version7"               */
    version7 = 1;
    nodefunc = ver7find;
    userfunc = opususer;
    newnodelist = 0;
    if (!stricmp (c, "plus"))
      version7++;               /* V7+ -> version7==2 */
    break;


  case 126:                    /* "BlankWait"              */
    blank_time = atol (c) * PER_SECOND;
    break;


  case 127:                    /* "Serial"                 */
    if (isdigit (*c))
      serial = atoi (c);
    break;


  case 128:                    /* "NoDietIfna"             */
    error = cfg_set_nodeflag (&no_dietifna, NODIET, c);  /* r. hoerner */
    break;

  case 129:                    /* obsolete                */
    error = kwd_unknown;
    break;

  case 130:                    /* "WinSlice"               */
    winslice = 1;
    break;


  case 131:                    /* "NoSize"                 */
    no_size = 1;
    break;


  case 132:                    /* "ForcExit"               */
    forcexit = atoi (c);
    break;


  case 133:                    /* "Extern"                 */
    if (strnicmp (c, "exit", 4) == 0)
      EXTopt = OPT_EXIT;
    else if (strnicmp (c, "batch", 5) == 0)
      EXTopt = OPT_BATCH;
    else if (strnicmp (c, "spawn", 5) == 0)
      EXTopt = OPT_SPAWN;
    else
      error = kwd_bad;
    break;


  case 134:                    /* "NoEMSI"                 */
    error = cfg_set_nodeflag (&no_EMSI, NOEMSI, c);  /* r. hoerner */
    break;


  case 135:                    /* "MyLocation"             */
    error = cfg_CopyString (keyword, &def_location, c);
    location = def_location;
    break;


  case 136:                    /* "MyPhone"                */
    error = cfg_CopyString (keyword, &myphone, c);
    break;


  case 137:                    /* "MyListFlags"            */
    error = cfg_CopyString (keyword, &myflags, c);
    break;


  case 138:                    /* "MyMaxbaud"              */
    error = cfg_CopyString (keyword, &mymaxbaud, c);
    break;


  case 139:                    /* "PickupALL"              */
    pickup_all = 1;
    break;


  case 140:                    /* "FileSec"                */
    /* AW 980220 sscanf (c, "%d", &DEFAULT.security); */
    /* new: get classname string, convert into security in b_initva.c   */
    error = cfg_CopyString (keyword, &DEFAULT.class, c);
    break;


  case 141:                    /* "KnownSec"               */
    /* AW 980220 sscanf (c, "%d", &KNOWN.security); */
    /* new: get classname string, convert into security in b_initva.c   */
    error = cfg_CopyString (keyword, &KNOWN.class, c);
    break;


  case 142:                    /* "ProtSec"                */
    /* AW 980220 sscanf (c, "%d", &PROT.security); */
    /* new: get classname string, convert into security in b_initva.c   */
    error = cfg_CopyString (keyword, &PROT.class, c);
    break;


  case 143:                    /* "MaxAreas"               */
    error = cfg_CopyString (keyword, &fri_areadat, c);
    fri_areatype = FRI_MAXIMUS;
    break;


  case 144:                    /* "Domain"                 */
    cfg_domain (c);
    break;


  case 145:                    /* "MailSound"              */
#ifdef BINKLEY_SOUNDS
    fnMailSound = cfg_sound (c);
#else
    error = kwd_unknown;
#endif
    break;


  case 146:                    /* "FileSound"              */
#ifdef BINKLEY_SOUNDS
    fnFileSound = cfg_sound (c);
#else
    error = kwd_unknown;
#endif
    break;


  case 147:                    /* "BBSSound"               */
#ifdef BINKLEY_SOUNDS
    fnBBSSound = cfg_sound (c);
#else
    error = kwd_unknown;
#endif
    break;


  case 148:                    /* "EXTSound"               */
#ifdef BINKLEY_SOUNDS
    fnEXTSound = cfg_sound (c);
#else
    error = kwd_unknown;
#endif
    break;


  case 149:                    /* "StartSound"             */
#ifdef BINKLEY_SOUNDS
    fnStartSound = cfg_sound (c);
#else
    error = kwd_unknown;
#endif
    break;


  case 150:                    /* "FaxSound"               */
#ifdef BINKLEY_SOUNDS
    fnFAXSound = cfg_sound (c);
#else
    error = kwd_unknown;
#endif
    break;


  case 151:                    /* "FaxInDir"               */
    error = cfg_CopySlashString (keyword, &fax_in, c);
    break;


  case 152:                    /* "ModemFailure"           */
    error = cfg_mdm_response (c, FAILURE);
    break;


  case 153:                    /* "ModemIgnore"            */
    error = cfg_mdm_response (c, IGNORE);
    break;


  case 154:                    /* "ModemConnect"           */
    error = cfg_mdm_response (c, CONNECTED);
    break;


  case 155:                    /* "ModemRinging"           */
    error = cfg_mdm_response (c, RINGING);  /* "RING" */
    break;


  case 156:                    /* "ModemIncoming"          */
    error = cfg_mdm_response (c, INCOMING);  /* "NO DIALTONE" */
    break;


  case 157:                    /* "ModemFax"               */
    error = cfg_mdm_response (c, FAX);
    break;


  case 158:                    /* "CostLog"                */
    error = cfg_CopyString (keyword, &cost_log_name, c);
    break;


  case 159:                    /* "CostUnit"               */
    if (isdigit (*c))
      cost_unit = atoi (c);
    break;


  case 160:                    /* "EuroCost"               */
    eurocost = 1;
    break;


  case 161:                    /* "NoFilter"               */
    if (!*c)
    {
      noMNPFilter = 1;
      break;
    }

    if (noMNPFilter)
    {
      printf ("ERR: NoFilter: %s ignored, already using no filter at all\n", c);
      error = kwd_other_err;
      break;                    /* he's so stuuupid!        */
    }

    if (cMNP > 16)              /* Only 16 allowed          */
    {
      printf ("ERR: NoFilter: can't store more then 16 filters.\n");
      error = kwd_other_err;
      break;
    }

    p = ctl_string (c);         /* Allocate space to copy the result code */
    if (p == NULL)
    {
      error = kwd_nomem;
      break;
    }

    pchMNP[cMNP++] = p;         /* Copy the result code and store the pointer */
    break;


  case 164:                    /* "NoHydra"                */
    error = cfg_set_nodeflag (&no_hydra, NOHYDRA, c);  /* r. hoerner */
    break;


  case 165:                    /* "NoJanus"                */
    error = cfg_set_nodeflag (&no_janus, NOJANUS, c);  /* r. hoerner */
    break;


  case 166:                    /* "ExtBaudRates"           */
#ifdef EXTENDED_BAUDRATES
    extend_rates = 1;
    pbtypes = exttypes;
#else
    error = kwd_unknown;
#endif
    break;


  case 167:                    /* "AfterCall"              */
    if (isdigit (*c))
    {
      lines_aftercall = atoi (c);
      c = skip_to_blank (c);
    }

    error = cfg_CopyString (keyword, &modem.aftercall, c);
    break;


  case 168:                    /* "PktRsp"                 */
    pktrsp = 1;
    break;


  case 169:                    /* "ExtSession"             */
    if (!isxdigit (*c))
    {
      error = kwd_bad;
      break;
    }

    sscanf (c, "%x", &itemp);
    ExtMailMask = (word) itemp;

    if (!ExtMailMask)
    {
      error = kwd_bad;
      break;
    }

    c = skip_to_blank (c);
    error = cfg_CopyString (keyword, &ExtMailAgent, c);
    break;


  case 170:                    /* "FaxBaud"                */
    ltemp = atol (c);
    fax_baud = -1;
    for (itemp = 0; pbtypes[itemp].rate_value; itemp++)
    {
      if (pbtypes[itemp].rate_value == (unsigned long) ltemp)
      {
        fax_baud = itemp;
        break;
      }
    }
    if (fax_baud == -1)
      error = kwd_bad;
    break;


  case 171:                    /* "ErrLevelShell"          */
    itemp = atoi (c);
    if ((itemp <= 0) || (itemp > 255))
    {
      printf (MSG_TXT (M_BAD_ERRLEVELSHELLNUM), itemp);
      error = kwd_other_err;
      break;
    }
    c = skip_to_word (c);
    if (c)
    {
      // strupr (c);                      /* MR 970503 not good!      */
      error = cfg_CopyString (keyword, &errlvlshell[itemp - 1], c);
    }
    break;


  case 172:                    /* AltNumber                */
    error = cfg_alt_num (c);
    break;


  case 173:                    /* ModemRetry               */
    error = cfg_mdm_response (c, DIALRETRY);
    break;


  case 174:                    /* RecentActivityLines      */
    itemp = atoi (c);
    if (itemp > 0)
      scrllines = itemp;
    break;


  case 175:                    /* "RingWait"               */
    ring_wait = atoi (c);
    break;


  case 176:                    /* HJK - 99/01/22 FreqLongNames */
    freqlongnames = 1;
    break;


    /* case 177: TJW 960830 [expression] - see parse_config... */


  case 178:                    /* TJW 960830 TaskPath      */
    if (task_dir == BINKpath)
      task_dir = ctl_slash_string (binkdir, c);
    else
      error = cfg_CopySlashString (keyword, &task_dir, c);
    break;


  case 179:                    /* RDH 9604 "ShowAlive"     */
    should_show_alive = 1;
    break;


  case 180:                    /* "ReadHoldTime"           */
  case 208:                    /* "OutboundRescanPeriod"   */
    readholdtime = atol (c);
    if (readholdtime < 1)
      readholdtime = 1;
    readholdtime *= PER_MINUTE;
    break;


  case 181:                    /* "ReInitTime"             */
    reinittime = atol (c);      /* TJW 960421               */
    if (reinittime < 1)
      reinittime = 1;           /* TJW 960421               */
    reinittime *= PER_MINUTE;   /* TJW 960421               */
    break;


  case 182:                    /* CE/TJW 960425 "ConditionalPoll"  */
    error = cfg_condpoll (c);
    break;


  case 183:                    /* CE/TJW 960425 "Reject"   */
    error = cfg_CopyString (keyword, &modem.reject, c);
    break;


  case 184:                    /* TJW 960719 "ProtocolPreference"  */
    error = cfg_CopyString (keyword, &protpreference, c);
    strupr (protpreference);
    break;


  case 185:                    /* MMP 960419 "PutEnv"      */
    p = ctl_string (c);
    if (p == NULL)
    {
      error = kwd_nomem;
      break;
    }

    for (c = p; *c && *c != '='; c++)
      *c = (char) toupper (*c);
    putenv (p);
    break;


  case 186:                    /* MMP 960419 "SpawnInit"   */
    b_spawn (c);
    break;


  case 187:                    /* MMP 960419 "LocalLog"    */
    locallogs = 1;
    break;


  case 188:                    /* MMP 960422 "ShortCostLog"    */
    if (*c)
      error = cfg_CopyString (keyword, &shortcostlog, c);
    else
      shortcostlog = "$02y$02m$02d $02H$02M$02S $# $14A $6B $4s $4c $4r $8i $8o $8C $4Y";

    /* MMP 960513 Spaces removed */
    /* TJW 960813 added $4r for remote's cost */
    break;


  case 189:                    /* MMP/TJW 960419 "TimeSync"    */
    itemp = 0;
    error = cfg_set_nodeflag (&itemp, TIMESYNC, c);  /* MR 970506 */
    if (itemp)
      error = kwd_bad;          /* syntax error             */
    else
    {
      p = skip_to_word (c);
      if (*p)                   /* MR 970406 max_diff_time  */
        max_diff_trx = atol (p);  /*           if given       */
    }
    break;


  case 190:                    /* MMP 960419 "MailFlag"    */
    if (*c)
      error = cfg_CopyString (keyword, &MailFlag, c);
    else
      error = cfg_CopyString (keyword, &MailFlag, "%sbtmail.in");
    break;


  case 191:                    /* TJW 960429 "MakeDir"     */
#ifdef MAKEDIR
    makedir = 1;
#else
    error = kwd_unknown;
#endif
    break;


  case 192:                    /* mtt 960502 "SRIF"        */
    for (p = SRIF_commandline; *c; c++, p++)  /* TJW 960503               */
    {
      switch (*c)
      {
      case '$':
        *p = '%';               /* % is comment, so use $ instead in cfg    */
        break;
      default:
        *p = *c;
      }
    }
    *p = '\0';
    break;


  case 193:                    /* "AfterConnect" PE 1996-04-25 */
    if (isdigit (*c))
    {
      lines_afterconnect = atoi (c);
      c = skip_to_blank (c);
    }
    error = cfg_CopyString (keyword, &modem.afterconnect, c);
    break;


  case 194:                    /* MaxBusyAge               */
    maxbsyage = atol (c);
    break;


  case 195:                    /* SpawnNoOK                */
    error = cfg_CopyString (keyword, &spawnnook, c);
    break;

  case 196:                    /* "CacheHold"              */
  case 209:                    /* "OutboundCache"          */
#ifdef CACHE
    cachehold = *c ? atoi (c) : 2;
    c = skip_to_word (c);
    if (!memicmp (c, "stat", 4))
      rescanstat = 1;
    else
      rescanstat = 0;
#else
    error = kwd_unknown;
#endif
    break;


  case 197:                    /* "PBAreas"                */
    error = cfg_CopyString (keyword, &fri_areadat, c);
    fri_areatype = FRI_PROBOARD;
    break;


  case 198:                    /* ModemReject              */
    error = cfg_CopyString (keyword, &ModemReject, c);
    break;


  case 199:                    /* AfterCallOut             */
    if (isdigit (*c))
    {
      lines_aftercallout = atoi (c);
      c = skip_to_blank (c);
    }
    error = cfg_CopyString (keyword, &modem.aftercallout, c);
    break;


  case 200:                    /* ChangeMailTo             */
    if (!stricmp (c, "CRASH"))
      chg_mail = (int) 'c';
    else if (!stricmp (c, "DIRECT"))
      chg_mail = (int) 'd';
    else if (!stricmp (c, "HOLD"))
      chg_mail = (int) 'h';
    else if (!stricmp (c, "NORMAL"))
      chg_mail = (int) 'f';
    else
      error = kwd_bad;
    break;


  case 201:                    /* TJW 960721 CostTimeCorrection    */
    if (sscanf (c, "%d %d", &costtimecorr_conn, &costtimecorr_sess) != 2)
      error = kwd_bad;
    break;


  case 202:                    /* "IPC"                    */
    error = cfg_CopySlashString (keyword, &ipc_dir, c);
    break;


  case 203:                    /* "ReadLog"                */
    error = cfg_CopyString (keyword, &readlog_name, c);
    break;


  case 204:                    /* "EMSIbanner"             */
    EMSIbanner = 1;
    break;


  case 205:                    /* TJW 960802 "CostCPS"     */
    costcps = atol (c);
    break;


  case 206:                    /* TJW 961010 "MCPpipe"     */
#ifdef OS_2
    /* r.hoerner: name in cfg file should override environment name     */
    error = cfg_CopyString (keyword, &MCPpipename, c);

    if (MCPpipename == NULL)
    {
      if (getenv ("MCPPIPE") != NULL)
        MCPpipename = ctl_string (getenv ("MCPPIPE"));
    }
    if (MCPpipename == NULL)
    {
      printf ("ERR: MCPpipe: no name defined, ignored\n");
      error = kwd_other_err;
    }
#else
    error = kwd_unknown;
#endif
    break;


  case 207:                    /* TJW 961013 "WindowTitleFmt"  */
#if defined(OS_2) | defined(_WIN32)  // HJK 98/06/13 Added Windows support
    if (*c)
      error = cfg_CopyString (keyword, &title_format, c);
    else
      error = cfg_CopyString (keyword, &title_format, "BT-XE #%d: %s");
#else
    error = kwd_unknown;
#endif
    break;


  case 210:                    /* "CheckPath" */
    check_path = 1;             /* enable CheckPath */

    check_path_errorlevel = atoi (c);  /* get variable errorlevel */
    if (check_path_errorlevel < 0)  /* in range from 0 - 255 */
      check_path_errorlevel = 0;
    if (check_path_errorlevel > 255)
      check_path_errorlevel = 255;

    c = skip_to_word (c);
    check_path_time = atoi (c); /* get variable time for next */
    if (check_path_time < 1)    /* checking. min 1 second     */
      check_path_time = 1;

    break;


  case 211:                    /* AKAMatching */
    AKA_matching = 1;
    break;


  case 212:                    /* NodeInboundDir */
    error = cfg_set_nodeinbounddir (c);
    break;


  case 213:                    /* SyncHydraSessions */
    error = cfg_set_SyncHydraSessions (c);
    break;

  case 214:                    /* NoModem */
    NoModem = 1;
    break;


  case 215:                    /* MTT reserverd            */
    error = cfg_CopyString (keyword, &HydraMessage, c);
    break;


  case 216:
  case 217:
  case 218:
  case 219:
    error = kwd_bad;
    break;


  case 220:                    /* "SmartBiDi" (switch off hydra in case    */
    smart_bidi = 1;             /* of expected one-way transfer)            */
    if (!stricmp (c, "outgoing"))  /* TJW 970216 also enable       */
      smart_bidi_out = 1;       /* smartbidi on outgoing calls  */
    break;

  case 221:                    /* RH reserved              */
  case 222:
  case 223:
  case 224:
  case 225:                    /* TJW reserved             */
  case 226:
  case 227:
  case 228:
  case 229:
    error = kwd_bad;
    break;


  case 230:                    /* TS "NoANSITrash"         */
#ifdef DOS16
    noansitrash = 1;
#else
    error = kwd_unknown;
#endif
    break;


  case 231:                    /* CFS "SIOMode"            */
    SIOMode = 1;
    break;


  case 232:                    /* TE 971013 "IpRepChar" */
    if ((*(c + 1)) || (!*c))    /* other than single char  */
      error = kwd_bad;
    else
    {
      cIPRepChar = *c;
      error = kwd_ok;
    }
    break;


  case 233:
    error = kwd_bad;
    break;


  case 234:                    /* JNK 971506 NoZedZip      */
    error = cfg_set_nodeflag (&no_zipzed, NOZIP, c);
    error = cfg_set_nodeflag (&no_zapzed, NOZED, c);
    error = cfg_set_nodeflag (&no_EMSI, NOEMSI, c);
    break;

  case 235:                    /* CFS "BannerCID"          */
    parse_BannerCID (c);
    break;


  case 236:                    /* CFS "ModemCID"           */
    error = cfg_CopyString (keyword, &CIDline, c);
    break;


  case 237:                    /* CFS "StringRep"          */
    parse_stringrep (c, 0);
    break;


  case 238:                    /* CFS "Callback"           */
    itemp = 0;
    error = cfg_set_nodeflag (&itemp, CALLBACK, c);  /* MR 970324 */
    if (itemp)
      error = kwd_bad;          /* syntax error             */
    else
    {
      p = skip_to_word (c);
      if (*p)                   /* MR 970406 set CallerID   */
        error = cfg_set_callerid (c, NULL);  /* if given       */
    }
    break;


  case 239:                    /* CFS "Nohundredths"       */
    nologhundredths = 1;
    break;


  case 240:                    /* "AssumeBaud <baudrate>"  */
    assumebaud = atol (c);
    break;


  case 241:                    /* "Clock [color] [char]" in screensaver    */
    DrawClock = 1;
    if (c == NULL)
      break;
    if (isdigit (*c))
    {
      sscanf (c, "%u", &itemp);
      clock_color = itemp & 0xff;
      c = skip_to_word (c);
    }
    if (c == NULL)
      break;
    if (*c)
      clock_char = c[0];
    if (clock_char == ' ')
      DrawClock = 0;
    break;


  case 242:                    /* BitType                  */
    exact_mdm_type = 0;
    break;


  case 243:                    /* "ReDialTime <seconds>"   */
    ReDialTime = atol (c);
    if (ReDialTime < 10)
      ReDialTime = 10;
    ReDialTime *= PER_SECOND;
    break;


  case 244:                    /* "EmsiLog <filename>"     */
    error = cfg_CopyString (keyword, &emsilog, c);
    if (emsilog != NULL)
      debug_emsi = 1;
    break;


  case 245:                    /* "NoErrDelay"             */
    no_delay = 1;
    break;


  case 246:                    /* "BinkDir"                */
    binkdir = 1;
    break;


  case 247:                    /* "ShowDomains"            */
    hidedomains = 0;
    break;


  case 248:                    /* "PassWordFile <filename>" */
    error = cfg_CopyString (keyword, &passwordfile, c);
    break;


  case 249:                    /* "ForcedRescan"           */
    rescanforced = 1;
    break;


  case 250:                    /* "NoFancyStrings"         */
    nofancystrings = 1;
    break;


  case 251:                    /* "MaxRingWait <seconds>"  */
    MaxRingWait = atoi (c);
    if (MaxRingWait < 10)
      MaxRingWait = 10;
    if (MaxRingWait > 120)
      MaxRingWait = 120;
    break;


  case 252:                    /* obsolete, was: "HoldAfterBadConnect" */
    error = kwd_unknown;
    break;


  case 253:                    /* "ShowPassword"           */
    show_password = 1;
    break;


  case 254:                    /* "Override <params...>"   */
    error = cfg_override (c);
    break;


  case 255:                    /* "NoWildCards <...>"      */
    error = cfg_set_nodeflag (&noWildcards, NOJOKERS, c);
    break;


  case 256:                    /* "PipeTo <computer>"      */
    cfg_CopyString (keyword, &bink_pipe, c);
    break;


  case 257:                    /* "DelBadCall"             */
    DelBadCall = 1;
    break;


  case 258:                    /* "EventFile <filename>"   */
    if (strcmp (eventfile, PRDCT_PRFXlower ".evt") == 0)
      eventfile = ctl_string (c);
    else
      cfg_CopyString (keyword, &eventfile, c);
    break;


  case 259:                    /* "SharePort"              */
    // HJK 98/06/13 - In some cases it is useful to use it in Windows too
#if defined (OS_2) | defined (_WIN32)
#if defined (NEED_OS2COMMS) | defined (NT)
    share_port = TRUE;
#else
    error = kwd_unknown;
#endif
#else
    error = kwd_unknown;
#endif
    break;


  case 260:                    /* "MaxPrm"  AW 980219      */
    error = cfg_MaxPrm (keyword, c);
    break;


  case 261:                    /* "ModemRing"              */
    error = cfg_mdm_response (c, RING);  /* "RINGING" */
    UseModemRing = 1;
    break;


  case 262:                    /* "cFosCost"               */
    cFosCost = 1;
    break;


  case 263:                    /* CFS "ModemCostReset"     */
    error = cfg_mdm_response (c, COSTRESET);
    UseRingingCost = 1;
    break;


  case 264:                    /* CFS "StatsToMSG"         */
    if ((error = cfg_parse_pktstats (c, &PKTStats)) == kwd_ok)
      DoStats = 1;
    break;


  case 265:                    /* MR 970215 "UseCompletePaths" */
    addBINKpath = BINKpath;
    break;


  case 266:                    /* MR 970215 "ShowModemWin" */
    show_modemwin = 1;
    break;


  case 267:                    /* JNK 970312 "NoTranx"     */
    no_tranx = 1;
    break;


  case 268:                    /* CFS 970310 CAPICID       */
#if !defined(OS_2) || defined(__EMX__)
    error = kwd_unknown;
#else
    p = skip_to_word (c);
    p2 = skip_to_word (p);

    if (*c == '\0' || *p == '\0' || *p2 == '\0')
    {
      error = kwd_bad;
      break;
    }

    *skip_to_blank (c) = 0;
    *skip_to_blank (p) = 0;

    do
    {
      /* Word starts at P2 */
      AddCIDTo = (char **) realloc (AddCIDTo, (AddCIDToNumber + 1) * sizeof (char *));

      AddCIDTo[AddCIDToNumber++] = p3 = ctl_string (p2);
      while (*p3)
      {
        if (*p3 == ' ')
        {
          *p3 = 0;
          break;
        }
        p3++;
      }
      p2 = skip_to_word (p2);
    }
    while (*p2);

    // for (itemp=0; itemp<AddCIDToNumber; itemp++)
    //   printf ("Add ID to modem string: %s\n",AddCIDTo[itemp]);

    // printf ("EAZs: %s\n",c);
    // printf ("Services: %s\n",p);

    if (!CAPIInit (c, p))
    {
      UseCAPICID = 1;
      if (CIDline == NULL)
        CIDline = ctl_string ("/ID=");
    }
#endif
    break;


  case 269:                    /* CFS 971103 CFOS Line     */
#ifdef OS_2
    MonitorCFOS = CFOSMonitorStart ();
#else
    error = kwd_unknown;
#endif
    break;


  case 270:                    /* MR 970325 "PreAnswer"    */
    if (modem.answer_setup_cnt < 4)
      modem.answer_setup[modem.answer_setup_cnt++] = ctl_string (c);
    else
    {
      printf ("ERR: PreAnswer: too many init strings, skipped %s\n", c);
      error = kwd_other_err;
    }
    break;


  case 271:                    /* MR 970325 "CallerID"     */
    error = cfg_set_callerid (c, NULL);
    break;


  case 272:                    /* CFS "StringRepModem"     */
    parse_stringrep (c, 1);
    break;


  case 273:                    /* MR 970425 "HangUp"       */
    error = cfg_CopyString (keyword, &modem.hangup, c);
    break;


  case 274:                    /* CFS "Autochatopen"       */
    autochatopen = 1;
    break;


  case 275:                    /* CFS "ChatLogDir"         */
    error = cfg_set_chatlogdir (c);
    break;


  case 276:                    /* Node extra dir           */
    error = cfg_set_nodeextradir (c);
    break;


  case 277:                    /* CFS "Random System"      */
    error = cfg_set_randomsystem (c);
    break;


  case 278:                    /* CFS "Random Sysop"       */
    error = cfg_set_randomsysop (c);
    break;


  case 279:
    error = cfg_set_randomlocation (c);  /* CFS "Random location"    */
    break;


  case 280:                    /* CFS "Short History"      */
    HistMode = HIST_SHORT;
    break;


  case 281:                    /* CFS "StatsDef"           */
    error = cfg_statsdef (c);
    break;


  case 282:                    /* JH 980817 "RejectIgnoreSize */
    ltemp = atol (c);
    if (ltemp >= 0)
      rejectignoresize = ltemp;
    break;


  case 283:                    /* CEH 990604 "ReadFlagTime" */
    ltemp = atol (c);
    if (ltemp > 0)
      readflagtime = ltemp;
    break;


  case 284:                    /* "LocalInbound"            */
    error = cfg_CopySlashString (keyword, &local_inbound, c);
    break;


  case 285:                    /* VRP 990820 "OnLine"       */
    SetOnLineTime (c);
    break;


  case 286:                    /* VRP 990824 "SendHoldIfCaller" */
    SendHoldIfCaller = 1;
    break;


  case 287:                    /* VRP 990824 "RestorePoll" */
    RestorePoll = 1;
    RestorePollFlavour[0] = 'c';

    strlwr (c);
    if (strstr (c, "norm") != NULL)
      RestorePollFlavour[0] = 'n';
    else if (strstr (c, "dir") != NULL)
      RestorePollFlavour[0] = 'd';
    else if (strstr (c, "cra") != NULL)
      RestorePollFlavour[0] = 'c';
    else
    {
      printf ("ERR: RestorePoll: unknown flavour '%s', assuming 'crash'\n", c);
      error = kwd_other_err;
    }
    break;


  case 288:                    /* VRP 990830 "Override <params...>" */
    error = cfg_hidden (c);
    break;


  case 289:                    /* VRP 990918 "NetMailFlag" */
    if (*c)
      error = cfg_CopyString (keyword, &NetMailFlag, c);
    else
      error = cfg_CopyString (keyword, &NetMailFlag, "%sbtnetml.in");
    break;


  case 290:                    /* VRP 990918 "ArcMailFlag" */
    if (*c)
      error = cfg_CopyString (keyword, &ArcMailFlag, c);
    else
      error = cfg_CopyString (keyword, &ArcMailFlag, "%sbtarcml.in");
    break;


  case 291:                    /* VRP 990918 "TicFlag"     */
    if (*c)
      error = cfg_CopyString (keyword, &TicFlag, c);
    else
      error = cfg_CopyString (keyword, &TicFlag, "%sbttic.in");
    break;


  case 292:                    /* VRP 990920 "SaveExtSession" */
    SaveExtSession = SAVE_EXT_BOTH;
    if (c && *c)
    {
      strlwr (c);
      if (strncmp (c, "out", 3) == 0)
      {
        SaveExtSession = SAVE_EXT_OUT;
      }
      else if (strncmp (c, "in", 2) == 0)
      {
        SaveExtSession = SAVE_EXT_IN;
      }
    }
    break;


  default:
    error = kwd_bad;
    break;
  }

  return error;
}


/* MMP 960419
 * cfgfgets() also has some advantages: it supports indented config verbs,
 * another method to specifying task specific lines, and it does not
 * dynamically allocate memory. Using it instead of fgets() we are also sure
 * to NUL-terminate the input line (the original code in parse_config() could
 * trap on very long lines).
 *
 * TJW 960422 changed comment definition to support stuff like:
 *            PutEnv PATH=x:\bt;x:\bt\bin and similar cases.
 *
 * TE 960516 made some changes around the "%"-char to support % in Init strings.
 *
 * TJW 960523 well, just another change, because this was really TOO
 * complicated and yet makes too much complications, therefore the solution
 * (hopefully) - see comments below.
 */

static char *
cfgfgets (char buf[], FILE * fp)
{
  int bufpos = 0;               /* index into buf[] */
  char tmp[CFGLINEBUFSIZE];     /* temporary buffer */
  int tmppos;                   /* index into tmp[] */
  int max = CFGLINEBUFSIZE - 1; /* make sure there is room for end '\0' */
  int ch;
  int comment = 0;              /* 1 if reading a comment */
  int whitespacebefore = 1;     /* TJW 960422 begin of line or char(s) before were white space */
  char *var;

  /*
   * Check for EOF - we might have encountered it while reading previous line
   */
  if (feof (fp))
    return (NULL);

  /*
   * Read line. Check for the special characters '%' and ';' as we read.
   */
  while (ch = fgetc (fp), ch != '\n' && ch != EOF)
    if (!comment)
      switch (ch)
      {
      case '%':
        /*
         * '%' has a VERY ambiguous semantics:
         * - it can prefix a "rest of line"-comment (if it is at
         *   1st non-white-space position in the config line)
         * - it can designate the start/end of an environment
         *   variable reference.
         * - it can be used simply as "%" character
         *
         * We resolve this the following way:
         * 1. if first non-white-space char, it is a comment.
         * 2. if there are some characters between 2 percent signs
         *    try to evaluate an environment variable of this name.
         *    if not, meaning it is followed immediately by another
         *    %, use it as a single "%" character.
         * 4. if there is only a single "%" in this line, also use
         *    it as a single "%" character.
         */
        if (whitespacebefore && !bufpos)
        {                       /* "%" first non-ws char */
          comment = 1;
          break;
        }

        whitespacebefore = 0;
        tmppos = 0;

        while (ch = fgetc (fp), ch != EOF && ch != '\n' && ch != '%')
          if (tmppos < max)
            tmp[tmppos++] = (char) ch;
        tmp[tmppos] = '\0';

        /* how did we get out of this loop ? */
        if (ch == '%')
        {                       /* there was a 2nd "%" */
          if (tmppos == 0)
          {                     /* it was "%%" -> just use one % */
            if (bufpos < max)
              buf[bufpos++] = '%';
          }
          else
          {                     /* env var name is in tmp[] */
            strupr (tmp);       /* TJW 960422 make it uppercase! */
            var = getenv (tmp);
            if (var)
            {                   /* valid env var -> copy value */
              while (*var)
                if (bufpos < max)
                  buf[bufpos++] = *var++;
            }
            else
            {                   /* there is no such env var */
              /* -> copy verbatim chars in tmp */
              for (tmppos = 0;
                   tmp[tmppos] && (bufpos < max);
                   buf[bufpos++] = tmp[tmppos++]
                )
              {;
              }
              if (bufpos < max)
                buf[bufpos++] = '%';  /* copy terminating %, too */
            }
          }
        }
        else
        {                       /* EOF or \n -  there was no 2nd "%" */
          if (bufpos < max)
            buf[bufpos++] = '%';  /* use it as verbatim "%" char */

          for (tmppos = 0;
               tmp[tmppos] && (bufpos < max);
               buf[bufpos++] = tmp[tmppos++]) ;

          goto done;            /* there's nothing more in this line */
        }
        break;

        /*
         * ';' is a bit WEIRD, too:
         * - it can be a "rest-of-line"-comment
         * - it can be a literal character
         *
         * We resolve this the following way:
         * 1. if first non-white-space char, it is a comment.
         * 2. if there is a sequence "<white space>;;", then don't
         * begin a comment, but just read it as "<white space>;"
         * 3. if there is a sequence "<white space>;" (and no ";"
         *    following), it is a comment
         * 4. in any other case, just read it as ";" - no comment.
         */
      case ';':
        /*
         * '#' is checked here, too (though first non-whitespace
         * only). This makes the fall-through to default easier.
         */
      case '#':
        if (whitespacebefore)
        {
          if (!bufpos)          /* ";" first non-ws char */
          {
            comment = 1;
            break;
          }
          else if (ch == ';')
          {
            whitespacebefore = 0;
            ch = fgetc (fp);    /* look at next char */
            if (ch == ';')
            {                   /* ";" follows again - no comment */
              if (bufpos < max)
                buf[bufpos++] = ';';
            }
            else if (ch == '\n' || ch == EOF)
            {                   /* Null length comment */
              goto done;
            }
            else                /* this IS a comment */
              comment = 1;
            break;
          }
        }
        /* else: no break! fall through into default ! */

      default:
        whitespacebefore = isspace (ch);  /* don't store blanks  */
        if (!whitespacebefore || bufpos)
        {                       /* at begining of line */
          /* Store character literally */
          if (bufpos < max)
            buf[bufpos++] = (char) ch;
        }
        break;
      }

  /*
   * Done. Strip off trailing spaces, terminate buffer.
   */
done:
  while (bufpos && isspace (buf[bufpos - 1]))
    bufpos--;
  buf[bufpos] = '\0';

  /*
   * If the line begins with a digit, we parse this as a task number. If
   * it is different from TaskNumber, we clear the line buffer. If equal,
   * we remove the number. In this way, we can mix configurations of
   * different lines in the same configuration file.
   */
  if (isdigit (*buf))
  {
    if (atoi (buf) != TaskNumber)
      *buf = '\0';
    else
    {
      char *src, *dest;

      dest = src = buf;
      src = skip_to_word (src);
      while (*src)
        *dest++ = *src++;
      *dest = '\0';
    }
  }

  /*
   * Return result.
   */
  if (*buf == '\0' && ch == EOF)
    return (NULL);
  else
    return (buf);
}

static KWD_ERR
cfg_statsdef (char *c)
{
  LSTATS *ps, *news;
  PKTREP pktr;
  char *c0, *c1, *c2, *c3, *c4;
  int i;

  c0 = skip_to_word (c);        /* All lines or this line only */
  c1 = skip_to_word (c0);       /* Number of days to include in the report */
  c2 = skip_to_word (c1);       /* Number of items to save in each sorted list */
  c3 = skip_to_word (c2);       /* Parameters for cfg_parse_pktstats */

  if (!*c || !*c0 || !*c1 || !*c2 || !*c3)
    return kwd_other_err;

  if (cfg_parse_pktstats (c3, &pktr) != kwd_ok)
    return kwd_other_err;

  *(skip_to_blank (c)) = 0;

  /* Check definition name and create an entry with that name if no dupe */
  if (lstats)
  {
    for (ps = lstats; ps != NULL; ps = ps->next)
      if (!stricmp (ps->name, c))
      {
        printf ("Dupe stats definition name %s\n", c);
        return kwd_other_err;   /* Dupe region, ignore */
      }
  }

  news = (LSTATS *) malloc (sizeof (LSTATS));

  if (!lstats)
    lstats = news;
  else
  {
    for (ps = news; ps->next != NULL; ps = ps->next) ;
    {;
    }
    ps->next = news;
  }

  news->name = ctl_string (c);
  memcpy (&news->pktr, &pktr, sizeof (PKTREP));

  /* Check for All or This in second parameter */
  if (strstrci (c0, "This"))
    i = 0;
  else if (strstrci (c0, "All"))
    i = 1;
  else
  {
    printf ("Second parameter of StatsDef must be 'All' or 'This'.\n");
    strcpy (news->name, "--Invalid--");  /* Invalidate entry */
    return kwd_other_err;
  }

  news->alllines = i;

  /* Check second parameter for number of days */
  *(skip_to_blank (c1)) = 0;
  i = 1;

  for (c4 = c1; *c4; c4++)
  {
    if (!isdigit (*c4))
    {
      i = 0;
      break;
    }
  }

  if (i)
  {
    news->days = atoi (c1);
    news->coverage = DAYS_FIXED;
  }

  return kwd_ok;
}

static KWD_ERR
cfg_parse_pktstats (char *c, PKTREP * PKTStats)
{
  char *p, *p2;
  int itemp;

  parse_address (c, &PKTStats->from);
  memset (&PKTStats->password, 0, 8);
  c = skip_to_word (c);

  if (*c == 0 || c == NULL)
    return kwd_other_err;

  p = c;
  itemp = 0;

  while (*p && *p != ' ')
  {
    if (!isdigit (*p) && (*p != ':' && *p != '/' && *p != '.'))
    {
      itemp = 1;
      break;
    }

    p++;
  }

  p2 = skip_to_word (c);

  while (*p && *p != ' ')
    p++;

  if (*p == ' ')
    *p = 0;

  if (itemp)                    /* Send msg to echo         */
  {
    PKTStats->toecho = 1;
    strntcpy (PKTStats->echo, c, 65);
    // printf ("Logging to echo %s from %u:%u/%u.%u\n",
    //          PKTStats->echo,PKTStats->from.Zone,
    //          PKTStats->from.Net,PKTStats->from.Node,
    //          PKTStats->from.Point);
  }
  else
    /* Send msg to netmail      */
  {
    PKTStats->toecho = 0;
    parse_address (c, &PKTStats->to);
    // printf ("Logging to netmail to %u:%u/%u.%u from %u:%u/%u.%u\n",
    //          PKTStats->to.Zone,
    //          PKTStats->to.Net,PKTStats->to.Node,
    //          PKTStats->to.Point,
    //          PKTStats->from.Zone,
    //          PKTStats->from.Net,PKTStats->from.Node,
    //          PKTStats->from.Point);
  }
  if (p2 != NULL)
    if (*p2)
      /* There are eight bytes for the password - copying */
      /* more would screw the .PKT header up. Note that   */
      /* there isn't room for an ending zero, that's why  */
      /* we use strncpy.                                  */
    {
      strncpy (PKTStats->password, p2, 8);
      // printf ("Using password %s\n",p2);
    }
  return kwd_ok;
}


static KWD_ERR
cfg_set_randomsystem (char *c)
{
  char *t1, *t2, *t3;
  RANDOM_SYSTEM *group;

  t1 = c;
  t2 = skip_to_word (c);

  if (!*t2)
    return kwd_other_err;

  *(skip_to_blank (c)) = 0;

  while (t1 < t2)
  {
    t3 = t1;

    while (*t3 != ',' && *t3)
      t3++;

    *t3 = 0;
    group = QueryRandomGroup (t1, 1);

    if (group)
    {
      group->system = realloc (group->system, (group->systemcount + 1) *
                               sizeof (char *));

      group->system[group->systemcount++] = ctl_string (t2);
    }

    t1 = t3 + 1;
  }

  return kwd_ok;
}

static KWD_ERR
cfg_set_randomsysop (char *c)
{
  char *t1, *t2, *t3;
  RANDOM_SYSTEM *group;

  t1 = c;
  t2 = skip_to_word (c);

  if (!*t2)
    return kwd_other_err;

  *(skip_to_blank (c)) = 0;

  while (t1 < t2)
  {
    t3 = t1;

    while (*t3 != ',' && *t3)
      t3++;

    *t3 = 0;
    group = QueryRandomGroup (t1, 1);

    if (group)
    {
      group->sysop = realloc (group->sysop, (group->sysopcount + 1) * sizeof (char *));

      group->sysop[group->sysopcount++] = ctl_string (t2);
    }
    t1 = t3 + 1;
  }

  return kwd_ok;
}

static KWD_ERR
cfg_set_randomlocation (char *c)
{
  char *t1, *t2, *t3;
  RANDOM_SYSTEM *group;

  t1 = c;
  t2 = skip_to_word (c);

  if (!*t2)
    return kwd_other_err;

  *(skip_to_blank (c)) = 0;

  while (t1 < t2)
  {
    t3 = t1;

    while (*t3 != ',' && *t3)
      t3++;

    *t3 = 0;
    group = QueryRandomGroup (t1, 1);

    if (group)
    {
      group->location = realloc (group->location, (group->locationcount + 1)
                                 * sizeof (char *));

      group->location[group->locationcount++] = ctl_string (t2);
    }

    t1 = t3 + 1;
  }

  return kwd_ok;
}

#ifdef BINKLEY_SOUNDS
static char *
cfg_sound (char *c)             /* malloc & copy to ctl      */
{
  char *dest;
  int i;

  i = (int) strlen (c);         /* get length of remainder   */
  if (i < 1)                    /* must have at least 1      */
    return (NULL);              /* if not, return NULL       */
  if (!dexists (c))             /* Is the file there?        */
    return (NULL);              /* No, return NULL           */
  dest = ctl_string (c);        /* allocate space for string */
  return (dest);                /* return the directory name */
}
#endif


static KWD_ERR
cfg_macro (char *c)
{
  int i;
  char *p;

  i = atoi (c);
  if ((i <= 0) || (i > 9))
  {
    printf ("ERR: Macro: %s %d %s\n", MSG_TXT (M_MACRO_NUMBER), i,
            MSG_TXT (M_OUT_OF_RANGE));
    return kwd_other_err;
  }

  c = skip_to_word (c);
  keys[i - 1] = calloc (1, strlen (c) + 1);
  p = keys[i - 1];

  while (*c && (*c != '\n'))
  {
    if (*c == '|')
      *p++ = '\r';
    else
      *p++ = *c;
    ++c;
  }

  *p = '\0';
  return kwd_ok;
}


void
cfg_dial_string (char *p)
{
  char *p1;
  PN_TRNS *tpn;

  if ((*p == ';') || (*p == '\0'))
  {
    return;
  }

  tpn = (PN_TRNS *) calloc (1, sizeof (PN_TRNS));

  p1 = tpn->num;
  while ((*p) && (!isspace (*p)))
  {
    *p1++ = *p++;
  }
  *p1 = '\0';

  p = skip_blanks (p);

  p1 = tpn->pre;
  while ((*p) && (*p != '/') && (!isspace (*p)))
  {
    *p1++ = *p++;
  }
  *p1 = '\0';

  p1 = tpn->suf;
  if ((*p == '\0') || (isspace (*p)))
  {
    *p1 = '\0';
  }
  else
  {
    ++p;

    while ((*p) && (!isspace (*p)))
    {
      *p1++ = *p++;
    }
    *p1 = '\0';
  }

  tpn->len = (short) strlen (tpn->num);
  tpn->next = NULL;
  if (pn_head == NULL)
  {
    pn_head = tpn;
  }
  else
  {
    pn->next = tpn;
  }
  pn = tpn;
}


void
cfg_mdm_trans (char *p)
{
  char *p1;
  int i;
  MDM_TRNS *tmm;

  if ((*p == ';') || (*p == '\0'))
  {
    return;
  }

  tmm = (MDM_TRNS *) calloc (1, sizeof (MDM_TRNS));

  tmm->mdm = (byte) atoi (p);
  p = skip_to_word (p);

  p1 = tmm->pre;
  i = sizeof (tmm->pre);
  while (i && (*p) && (*p != '/') && (!isspace (*p)))
  {
    *p1++ = *p++;
    i--;
  }
  *p1 = '\0';

  p1 = tmm->suf;
  if ((*p == '\0') || (isspace (*p)))
  {
    *p1 = '\0';
  }
  else
  {
    ++p;

    i = sizeof (tmm->suf);
    while (i && (*p) && (*p != '/') && (!isspace (*p)))
    {
      *p1++ = *p++;
      i--;
    }
    *p1 = '\0';
  }

  p1 = tmm->ascii;
  while (isspace (*p))
    p++;

  if (*p != '/')
  {
    *p1 = '\0';
  }
  else
  {
    ++p;

    i = sizeof (tmm->ascii);
    while (i && (*p) && (*p != ';') && (!isspace (*p)))
    {
      *p1++ = *p++;
      i--;
    }
    *p1 = '\0';
  }

  tmm->next = NULL;
  if (mm_head == NULL)
  {
    mm_head = tmm;
  }
  else
  {
    mm->next = tmm;
  }
  mm = tmm;
}


void
cfg_domain (char *p)
{
  char *p1, *p2;
  int i;

  p = strtok (p, "; \t\r\n");
  if (p != NULL)
  {
    /* Have the domain name, now get the abbreviated name */
    p2 = strtok (NULL, "; \t\r\n");
    if (strlen (p2) > 8)
      *(p2 + 8) = '\0';
    if (p2 != NULL)
    {
      /* Have the domain name and abbreviated, now get the list name */
      p1 = strtok (NULL, "; \t\r\n");
      if (p1 == NULL)
        p1 = p2;

      /* Have the list name and the domain name, now use them */
      for (i = 0; (i < DOMAINS) && (domain_name[i] != NULL); i++)
      {
        if (strnicmp (domain_name[i], p, strlen (domain_name[i])) == 0)
        {
          domain_nodelist[i] = strdup (p1);
          domain_abbrev[i] = strdup (p2);
          break;
        }
      }
      if (i < DOMAINS)
        if (domain_name[i] == NULL)
        {
          domain_name[i] = strdup (p);
          domain_abbrev[i] = strdup (p2);
          domain_nodelist[i] = strdup (p1);
        }
    }
  }
}


static KWD_ERR
cfg_domain_kludge (char *p)
{
  word i;

  if (isdigit (*p))
  {
    if (num_domain_kludge == size_domain_kludge)
    {
      size_domain_kludge += 16;
      if (!num_domain_kludge)
        domainkludge = (DOMAINKLUDGE *) malloc (size_domain_kludge * sizeof (DOMAINKLUDGE));
      else
        domainkludge = (DOMAINKLUDGE *) realloc ((void *) domainkludge,
                                size_domain_kludge * sizeof (DOMAINKLUDGE));
    }

    domainkludge[num_domain_kludge].zone = (word) atoi (p);
    p = skip_to_blank (p);
  }
  else
    return kwd_bad;

  p = skip_blanks (p);
  p = strtok (p, "; \t\r\n");
  if (p != NULL)
  {
    for (i = 0; (i < DOMAINS) && (domain_name[i] != NULL); i++)
    {
      if (strnicmp (domain_name[i], p, strlen (domain_name[i])) == 0)
      {
        domainkludge[num_domain_kludge++].domain = i;
        break;
      }
    }
  }
  return kwd_ok;
}


static void
cfg_Janus (char *p)
{
  J_TYPES *tjt;
  char *p1;

  if ((*p == ';') || (*p == '\0'))
  {
    return;
  }

  tjt = (J_TYPES *) calloc (1, sizeof (J_TYPES));

  p1 = tjt->j_match;
  while ((*p) && (!isspace (*p)))
  {
    *p1++ = *p++;
  }
  *p1 = '\0';

  tjt->next = NULL;

  if (j_top == NULL)
  {
    j_top = tjt;
  }
  else
  {
    j_next->next = tjt;
  }
  j_next = tjt;
}


static KWD_ERR
cfg_alt_num (char *c)
{
  char *n1, *n2, *p;
  P_ANUMS a1, a2;

  /* Primary starts at first non-blank, ends at blank */

  p = skip_to_blank (c);
  *p++ = '\0';

  n1 = ctl_string (c);
  if (n1 == NULL)
  {
    printf ("ERR: AltNumber: %s\n", MSG_TXT (M_ALTNUM_NO_PRIMARY));
    return kwd_other_err;
  }

  c = skip_blanks (p);
  n2 = ctl_string (c);
  if (n2 == NULL)
  {
    printf ("ERR: AltNumber: %s\n", MSG_TXT (M_ALTNUM_NO_ALTERNATE));
    free (n1);
    return kwd_other_err;
  }

  a1 = calloc (1, sizeof (ANUMS));

  a1->num = n1;
  a1->alt = n2;

  if (anum_head == NULL)
  {
    anum_head = a1;
  }
  else
  {
    /* we need to maintain the order they were entered */
    for (a2 = anum_head; a2->next != NULL; a2 = a2->next) ;
    a2->next = a1;
  }

  return kwd_ok;
}


static KWD_ERR
cfg_mdm_response (char *c, int response)
{
  /* If we have no space, bump the array */
  if (resp_count == max_resp_count)
  {
    int new_max = max_resp_count + 32;
    int new_size = new_max * sizeof (struct resp_str);
    struct resp_str *new_resps;

    new_resps = (struct resp_str *) realloc (mdm_resps, new_size);
    if (new_resps == NULL)
    {
      return kwd_nomem;
    }
    max_resp_count = new_max;
    mdm_resps = new_resps;
  }
  mdm_resps[resp_count].resp = ctl_string (c);
  if (mdm_resps[resp_count].resp != NULL)
    mdm_resps[resp_count++].disp = response;

  return kwd_ok;
}


static void
bad_line (int linecounter, char *fname)
{
  long ltemp;

  printf ("ERR: line %d of file '%s'\007\n\n", linecounter, fname);

  if (!no_delay)
  {                             /* R. hoerner: don't bother me */
    ltemp = timerset (5 * PER_SECOND);
    while (!timeup (ltemp))
    {
#ifdef OS_2
      DosSleep (0)
#endif
        ;
    }                           /* TJW 960522 5s delay to show error */
  }
  /*
     (R. Hoerner)
     PLEASE:
     THIS STATEMENT CAUSES AN EXCEPTION! DO NOT ADD IT AGAIN.
     THE "MTASK_FIND" HAS NOT YET RUN!
     SEE B_INITVA.C FOR THE REASON.
     time_release(causes exception)
   */
}


/*--------------------------------------------------------------------*/
/* Remove comments, that are still in line                 R. Hoerner */
/*--------------------------------------------------------------------*/

static void
remove_comments (char *c)
{
  if (c == NULL)
    return;

  while (*c && (*c != ';') && (*c != '%'))
    c++;
  *c = 0;
}


/*--------------------------------------------------------------------*/
/* Set special flags on selective nodes                    R. Hoerner */
/* If no node is specified it sets the global variable to "1"         */
/*--------------------------------------------------------------------*/

static KWD_ERR
cfg_set_nodeflag (int *global, unsigned long flag, char *nodestring)
{
  ADDR addr;
  NODESTRUC *qnode = NULL;

  remove_comments (nodestring);

  if (strlen (nodestring) == 0) /* if not node specific */
  {
    *global = 1;                /* set global variable */
  }
  else
  {
    parse_address (nodestring, &addr);

    /* MR 970314 query node structure; if addr does not yet exist, create new
     *           struct and add it to our list.
     */

    qnode = QueryNodeStruct (&addr, 1);

    if (qnode)
      qnode->Flags |= flag;
    else
      return kwd_other_err;
  }

  return kwd_ok;
}

/*--------------------------------------------------------------------*/
/* Add region to region list                                          */
/*--------------------------------------------------------------------*/
static void LOCALFUNC
cfg_insert_region (word Zone, char *Region)
{
  AKREGION *pr, *newr;

  if (regions)
  {
    for (pr = regions; pr != NULL; pr = pr->next)
      if (pr->Zone == Zone && !strcmp (Region, pr->Region))
        return;                 /* Dupe region, ignore */
  }

  newr = (AKREGION *) malloc (sizeof (AKREGION));

  if (!regions)
    regions = newr;
  else
  {
    for (pr = regions; pr->next != NULL; pr = pr->next) ;
    pr->next = newr;
  }

  newr->Zone = Zone;
  newr->Region = ctl_string (Region);
  newr->next = NULL;
}

/*--------------------------------------------------------------------*/
/* insert your AKA in a list of AKAS                       R. Hoerner */
/*--------------------------------------------------------------------*/

static KWD_ERR
cfg_insert_AKA (char *nodestring)
{
  ADDR addr;
  AKA *pnode = NULL;
  AKA *qnode = NULL;
  char *c, *c2 = NULL;

  remove_comments (nodestring);
  c = skip_to_word (nodestring);

  if (*c)
  {
    c2 = ctl_string (c);
    *c = 0;
  }

  if (parse_address (nodestring, &addr) < 3)  /* Less than "Zone:Net/Node" */
  {
    printf ("ERR: Address: unsufficient data\n");
    return kwd_bad;
  }

  if (c2)
  {
    cfg_insert_region (addr.Zone, c2);
    free (c2);
  }

  if (alias != NULL)            /* if aka structure exists */
  {
    if ((alias->next == NULL)   /* and there is only 1 entry in list */
        && ((word) alias->Zone == 1)
        && ((word) alias->Net == 0xffff)
        && ((word) alias->Node == 0xffff))  /* and adress is 1:-1/-1 (default) */
    {
      copy_address (&addr, (ADDRP) alias);  /* than take this structure */
      return kwd_ok;
    }
  }

  for (pnode = alias; pnode != NULL; pnode = pnode->next)
  {
    if (SameAddress ((ADDR *) pnode, &addr))
    {
      printf ("Note: Address: duplicate address %u:%u/%u ignored\n",
              addr.Zone, addr.Net, addr.Node);

      if (pnode->Domain == NULL)
        pnode->Domain = addr.Domain;

      /* CFS 970504 - changed error to OK - Dupe AKA is NOT an error, it is
         common in multiline enviroments where the main AKA for each line is
         define twice (at the line specific part and at the common part). */
      return kwd_ok;
    }
  }

  /* if we reach this point then we didn't find this aka */

  qnode = (AKA *) calloc (1, sizeof (AKA));
  if (qnode == NULL)
  {
    return kwd_nomem;
  }

  if (alias == NULL)            /* if first structure */
    alias = qnode;
  else
  {
    for (pnode = alias; pnode->next != NULL; pnode = pnode->next) ;
    pnode->next = qnode;        /* add new structure */
  }

  copy_address (&addr, (ADDRP) qnode);
  qnode->next = NULL;

  return kwd_ok;
}


/*--------------------------------------------------------------------*/
static char *
GoToNextString (char *p)        /* return next in "xxx,yyy,aaa" */
/*--------------------------------------------------------------------*/
{
  if (p != NULL)
    while (*p && (*p != ' ') && (*p != ','))  /* search for end of flag */
      p++;

  return (p);
}


/*--------------------------------------------------------------------*/
/* set modem flag field of "Override" statement            r. hoerner */
/*--------------------------------------------------------------------*/

static word LOCALFUNC
SetModemFlag (char *modem)
{
  MDM_TRNS *mm;
  char *s, *p;
  word ret;

  /* no "ModemTrans" or no modemflags */
  if ((mm_head == NULL) || modem == NULL || (*modem == '-'))  /* VRP 990903 */
  {
    return (0);
  }

  modem = strupr (modem);
  ret = 0;

  /* walk ModemTrans data */
  for (mm = mm_head; mm != NULL; mm = mm->next)
  {
    s = p = modem;
    while (*p)
    {
      p = GoToNextString (p);   /* stops at '\0', ',' and ' ' */

      *p = 0;                   /* terminate string   */

      if (strnicmp (mm->ascii, s, strlen (s)) == 0)  /* compare string     */
        ret |= mm->mdm;

      if (!*p)                  /* last string? done! */
        break;

      s = ++p;                  /* else eat next portion    */
    }                           /* or check next ModemTrans */
  }

  if (!ret)
    printf ("ERR: Override: ignored modemflag '%s': no such data in ModemTrans\r\n", modem);

  return (ret);
}


/*--------------------------------------------------------------------*/
/* set fidoflag field in "Override" statement              r. hoerner */
/*--------------------------------------------------------------------*/

static word LOCALFUNC
SetFidoFlag (char *fido, ADDRP addr)
{
  word ret = 0;

  /* VRP 990903 start */

  if (fido == NULL || (*fido == '-'))
  {
    return (0);
  }

  /* VRP 990903 end */

  fido = strupr (fido);

  while (*fido && (*fido != ' '))
  {
    if (strnicmp (fido, "CM", 2) == 0)
      ret |= B_CM;
    else if (strnicmp (fido, "HUB", 3) == 0)
      ret |= B_hub;
    else if (strnicmp (fido, "RC", 2) == 0)
      ret |= B_region;
    else
      printf ("ERR: Override: unknown flag %s\n", fido);

    fido = GoToNextString (fido);  /* stops at '\0', ',' and ' ' */

    if (*fido == ',')
      fido++;
  }
  /* some fields are left:                              */

  if (addr->Point)              /* Point != 0: a Point       */
    ret |= B_point;

  if (addr->Node)               /* Node != 0: no host, no ZC */
    return (ret);

  if (addr->Net)                /* Node == 0, Net != 0: host */
    ret |= B_host;
  else
    ret |= B_zone;              /* Node == 0; Net == 0: a ZC */

  return (ret);
}

/* VRP 990820 start */

/*--------------------------------------------------------------------*/
/* Override node online time                            v. pomortseff */
/*                                                                    */
/* OnLine <address> <starttime>-<endtime> [weekdays]                  */
/*                                                                    */
/*--------------------------------------------------------------------*/

static KWD_ERR LOCALFUNC
SetOnLineTime (char *str)
{
  ADDR addr;
  ONLINE_TIME *online_time = NULL;
  short start = 0, end = 0, days = 0;
  char *p, *node = NULL, *time = NULL;

  remove_comments (str);

  node = str;

  p = skip_to_blank (node);     /* p points after nodenumber  */
  time = skip_to_word (node);   /* num points to phonenumber  */
  *p = 0;

  if (node)
  {
    if (parse_address (node, &addr) < 3)  /* Less than "Zone:Net/Node" */
    {
      printf ("ERR: Address: unsufficient data\n");
      return kwd_other_err;
    }

    if (cfg_parse_online_time (time, &start, &end, &days) == kwd_ok)
    {
      online_time = (ONLINE_TIME *) malloc (sizeof (ONLINE_TIME));

      if (online_time == NULL)
      {
        printf ("ERR: Memory error.\n");
        return kwd_nomem;
      }

      copy_address (&addr, (ADDRP) online_time);
      online_time->start = start;
      online_time->end = end;
      online_time->days = days;
      online_time->next = online_time1;
      online_time1 = online_time;

      return kwd_ok;
    }
  }

  return kwd_other_err;
}

static KWD_ERR
cfg_parse_online_time (char *time, short *start, short *end, short *days)
{
  int shour = 0, smin = 0, ehour = 24, emin = 0;

  *start = *end = 0;
  *days = DAY_ALL;

  if (time != NULL && time[0] != '\0')
  {
    if (time[0] == '-')
    {
      shour = smin = ehour = emin = 0;
    }
    else
    {
      if (time[0] != 'C' || time[1] != 'M')
        sscanf (time, "%d:%d - %d:%d", &shour, &smin, &ehour, &emin);
    }

    *start = (short) (shour * 60 + smin);
    *end = (short) (ehour * 60 + emin);
    *days = 0;

    strlwr (time);

    if (strstr (time, "mo") != NULL)
      *days |= DAY_MONDAY;
    if (strstr (time, "tu") != NULL)
      *days |= DAY_TUESDAY;
    if (strstr (time, "we") != NULL)
      *days |= DAY_WEDNESDAY;
    if (strstr (time, "th") != NULL)
      *days |= DAY_THURSDAY;
    if (strstr (time, "fr") != NULL)
      *days |= DAY_FRIDAY;
    if (strstr (time, "sa") != NULL)
      *days |= DAY_SATURDAY;
    if (strstr (time, "su") != NULL)
      *days |= DAY_SUNDAY;
    if (strstr (time, "wk") != NULL)
      *days |= DAY_WEEK;
    if (strstr (time, "wn") != NULL)
      *days |= DAY_WKEND;

    if (*days == 0)
      *days = DAY_ALL;
  }

  return kwd_ok;
}

/* VRP 990820 end */

/*--------------------------------------------------------------------*/
/* Override nodelist                                       r. hoerner */
/*                                                                    */
/* Override <address> <phone> <modemflag,..> <fidoflag,..>            */
/*                                                                    */
/*--------------------------------------------------------------------*/

static KWD_ERR
cfg_override (char *override)
{
  ADDR addr;
  char *p, *q, *r;
  char *node, *num, *modem, *fido;
  PHONE *phone, *tmp;
  word m_flag, f_flag;

  remove_comments (override);

  modem = fido = num = NULL;
  node = override;

  p = skip_to_blank (node);     /* p points after nodenumber  */
  num = skip_to_word (node);    /* num points to phonenumber  */
  if (num)
  {
    q = skip_to_blank (num);    /* q points after phonenumber */
    modem = skip_to_word (num); /* modem points to modemflags */
    if (modem)
    {
      r = skip_to_blank (modem);  /* r points after modemflags  */
      fido = skip_to_word (modem);  /* fido points to fidoflags   */
      *r = 0;
      r = skip_to_blank (fido); /* terminate all partial strings */
      *r = 0;
    }
    *q = 0;
  }
  *p = 0;

  if (parse_address (node, (ADDRP) & addr) < 3)  /* Less than "Zone:Net/Node" */
  {
    printf ("ERR: Override: incomplete nodenumber\n");
    return kwd_other_err;
  }

  if (*num == '-')              /* then this field is "blank" */
    num = NULL;

  if (strlen (modem) == 0)      /* then no such field exists  */
    modem = NULL;
  else if (*modem == '-')       /* then this field is "blank" */
    modem = NULL;

  if (strlen (fido) == 0)       /* then no such field exists  */
    fido = NULL;

  m_flag = f_flag = 0;          /* initialize flags */

  if (num)
    num = ctl_string (num);     /* allocates mem, copies and returns ptr */

  for (phone = phone1; phone != NULL; phone = phone->next)
  {
    if (SameAddress ((ADDR *) phone, &addr))
    {
      if (phone->Domain == NULL)
        ((PHONE *) phone)->Domain = addr.Domain;

      if ((phone->num != NULL) && (num != NULL))
        free (phone->num);      /* give up old stringspace in this case */

      goto insert_datafields;
    }
  }

  /* if we reach this point then we didn't find this aka */

  phone = (PHONE *) calloc (1, sizeof (PHONE));

  if (phone == NULL)
    return kwd_nomem;

  if (phone1 == NULL)           /* if first structure */
    phone1 = phone;
  else
  {
    for (tmp = phone1; tmp->next != NULL; tmp = tmp->next) ;
    tmp->next = phone;
  }

  copy_address (&addr, (ADDRP) phone);

insert_datafields:

  ((PHONE *) phone)->num = num; /* either NULL or valid */

  if (modem != NULL)
    m_flag = SetModemFlag (modem);  /* if any modemflag     */

  if (fido != NULL)             /* if any fidoflag      */
    f_flag = SetFidoFlag (fido, &addr);

  if (m_flag)                   /* if really modemflags */
    ((PHONE *) phone)->modem = m_flag;

  if (f_flag)                   /* if really fidoflags  */
    ((PHONE *) phone)->fido = f_flag;

  ((PHONE *) phone)->next = NULL;

  return kwd_ok;
}

/* VRP 990830 start */

/*--------------------------------------------------------------------*/
/* Add hidden line                                      v. pomortseff */
/*                                                                    */
/* Hidden <address> <phone> <modemflag,..> <fidoflag,..> <online>     */
/*                                                                    */
/*--------------------------------------------------------------------*/

static KWD_ERR
cfg_hidden (char *c)
{
  ADDR addr;
  char *p, *q, *r, *s;
  char *node, *num, *modem, *fido, *time;
  HIDDEN *hidden;
  HIDDEN_LINE *hidden_line;

  remove_comments (c);

  modem = fido = num = time = NULL;
  node = c;

  p = skip_to_blank (node);     /* p points after nodenumber  */
  num = skip_to_word (node);    /* num points to phonenumber  */
  if (num)
  {
    q = skip_to_blank (num);    /* q points after phonenumber */
    modem = skip_to_word (num); /* modem points to modemflags */
    if (modem)
    {
      r = skip_to_blank (modem);  /* r points after modemflags  */
      fido = skip_to_word (modem);  /* fido points to fidoflags   */

      if (fido)
      {
        s = skip_to_blank (fido);  /* s points after fidoflags */
        time = skip_to_word (fido);  /* time points to online time */
        *s = 0;
      }
      *r = 0;
    }
    *q = 0;
  }
  *p = 0;

  if (parse_address (node, (ADDRP) & addr) < 3)  /* Less than "Zone:Net/Node" */
  {
    printf ("ERR: Override: incomplete nodenumber\n");
    return kwd_other_err;
  }

  if (num == NULL || strlen (num) == 0 || *num == '-')  /* if this field is "blank" */
  {
    printf ("ERR: Hidden: incomplete phonenumber\n");
    return kwd_other_err;
  }

  if (modem != NULL && (strlen (modem) == 0 || *modem == '-'))
    modem = NULL;

  if (fido != NULL && (strlen (fido) == 0 || *fido == '-'))
    fido = NULL;

  if (num)
    num = ctl_string (num);     /* allocates mem, copies and returns ptr */

  for (hidden = hidden1; hidden != NULL; hidden = hidden->next)
  {
    if (SameAddress ((ADDR *) hidden, &addr))
    {
      if (hidden->Domain == NULL)
        ((HIDDEN *) hidden)->Domain = addr.Domain;

      hidden_line = (HIDDEN_LINE *) calloc (1, sizeof (HIDDEN_LINE));

      if (hidden_line == NULL)
        return kwd_nomem;

      hidden_line->next = hidden->hidden_line1;
      hidden->hidden_line1 = hidden_line;

      goto insert_datafields;
    }
  }

  /* if we reach this point then we didn't find this aka */

  hidden = (HIDDEN *) calloc (1, sizeof (HIDDEN));

  if (hidden == NULL)
    return kwd_nomem;

  copy_address (&addr, (ADDRP) hidden);

  hidden->hidden_line1 = (HIDDEN_LINE *) calloc (1, sizeof (HIDDEN_LINE));

  if (hidden->hidden_line1 == NULL)
  {
    free (hidden);
    return kwd_nomem;
  }

  hidden->next = hidden1;
  hidden->current = NULL;
  hidden1 = hidden;
  hidden1->hidden_line1->next = NULL;

insert_datafields:

  hidden1->hidden_line1->num = num;  /* either NULL or valid */
  hidden1->hidden_line1->modem = SetModemFlag (modem);  /* if any modemflag */
  hidden1->hidden_line1->fido = SetFidoFlag (fido, &addr);

  return (cfg_parse_online_time (time, &hidden1->hidden_line1->start,
                                 &hidden1->hidden_line1->end,
                                 &hidden1->hidden_line1->days));
}

/* VRP 990830 end */

/* MR 970314 cfg_condpoll */

static KWD_ERR
cfg_condpoll (char *c)
{
  ADDR addr;
  FPP newfp;
  NODESTRUC *qnode = NULL;
  KWD_ERR error = kwd_ok;

  remove_comments (c);

  if (strlen (c) == 0)          /* if not node specific */
    error = kwd_bad;
  else
  {
    newfp = (FPP) calloc (1, sizeof (FP));
    newfp->next = NULL;

    if (!strnicmp (c, "AND", 3))
      newfp->and_op = 1;        /* TJW 960505 boolean op */
    else if (!strnicmp (c, "OR", 2))
      newfp->and_op = 0;        /* TJW 960505 boolean op */

    c = skip_to_word (c);
    parse_address (c, &addr);

    /* MR 970314 query node structure; if addr does not yet exist, create new
     *           struct and add it to our list.
     */

    qnode = QueryNodeStruct (&addr, 1);

    if (qnode == NULL)
      error = kwd_other_err;
    else
    {
      c = skip_to_word (c);
      newfp->Size = atoi (c);   /* Size KB        */

      c = skip_to_word (c);
      newfp->Delta_T = atoi (c);  /* delta time     */

      newfp->MinDelta_T = 0;
      newfp->AttachType = 0;

      c = skip_to_word (c);

      if (isdigit (*c))         /* MR 970810      */
        if (isdigit (*(c + 1)))
          if (!isdigit (*(c + 2)))
          {
            newfp->MinDelta_T = atoi (c);  /* min delta time */
            c = skip_to_word (c);
          }

      if (strchr (c, 'N') || strchr (c, 'n'))
        newfp->AttachType |= AT_NETMAIL;

      if (strchr (c, 'E') || strchr (c, 'e'))
        newfp->AttachType |= AT_ECHOMAIL;

      if (strchr (c, 'O') || strchr (c, 'o'))
        newfp->AttachType |= AT_OTHER;

      if (newfp->AttachType)
        c = skip_to_word (c);

      if (*c)
        error = cfg_set_callerid (c, &addr);  /* phonenumber    */

      if (qnode->FPoll == NULL)
      {
        qnode->FPoll = newfp;
        qnode->Flags |= FREEPOLL;
      }
      else
      {
        FPP tmpfp = qnode->FPoll;

        while (tmpfp->next)
          tmpfp = tmpfp->next;

        tmpfp->next = newfp;
      }
    }
  }

  return error;
}


/* MR 970324 cfg_set_callerid */

static KWD_ERR
cfg_set_callerid (char *c, ADDRP gotaddr)
{
  ADDR addr;
  CIDLSTP newcid;
  CIDLSTP cid;
  NODESTRUC *qnode = NULL;
  KWD_ERR error = kwd_ok;

  if (gotaddr == NULL)
  {
    parse_address (c, &addr);
    gotaddr = &addr;
    c = skip_to_word (c);
  }

  qnode = QueryNodeStruct (gotaddr, 1);

  if (qnode != NULL)
  {
    newcid = (CIDLSTP) calloc (1, sizeof (CIDLST));

    if (newcid != NULL)
    {
      newcid->next = NULL;
      newcid->CallerID = ctl_string (c);

      if (newcid->CallerID != NULL)
      {
        if (qnode->CallerIDList == NULL)
          qnode->CallerIDList = newcid;
        else
        {
          cid = qnode->CallerIDList;

          while (cid->next != NULL)
            cid = cid->next;

          cid->next = newcid;
        }
      }
      else
        error = kwd_nomem;
    }
    else
      error = kwd_nomem;
  }
  else
    error = kwd_other_err;

  return error;
}

static KWD_ERR
cfg_set_chatlogdir (char *c)
{
  KWD_ERR error = kwd_ok;

  if (c == NULL)
    error = kwd_bad;

  if (!strlen (c))
    error = kwd_bad;

  ChatLogDir = realloc (ChatLogDir, strlen (c) + 2);
  strcpy (ChatLogDir, c);

  if (ChatLogDir[strlen (ChatLogDir) - 1] != DIR_SEPC)
    strcat (ChatLogDir, DIR_SEPS);

  return error;
}

/* CFS 970428 cfg_set_nodeextradir */

static KWD_ERR
cfg_set_nodeextradir (char *c)
{
  ADDR addr;
  NODESTRUC *qnode = NULL;
  KWD_ERR error = kwd_ok;
  char *t, *t2;

  t2 = c;

  parse_address (t2, &addr);
  qnode = QueryNodeStruct (&addr, 1);

  if (qnode != NULL)
  {
    c = skip_to_word (t2);
    t = skip_to_blank (c);

    if (*t)
    {
      *t = 0;
      t++;
      while (*t)
      {
        if (toupper (*t) == 'H')
          qnode->ExtraDirHold = 1;
        if (toupper (*t) == 'N')
          qnode->ExtraDirNotify = 1;
        t++;
      }
    }

    qnode->ExtraDir = (char *) malloc (strlen (c) + 2);
    strcpy (qnode->ExtraDir, c);
    if (qnode->ExtraDir[strlen (qnode->ExtraDir) - 1] != DIR_SEPC)
      strcat (qnode->ExtraDir, DIR_SEPS);
    error = kwd_ok;
  }
  else
    error = kwd_other_err;

  return error;
}

/* CE 980114 cfg_set_nodeinbounddir */

static KWD_ERR
cfg_set_nodeinbounddir (char *c)
{
  ADDR addr;
  NODESTRUC *qnode = NULL;
  KWD_ERR error = kwd_ok;
  char *t;

  t = c;

  parse_address (t, &addr);
  qnode = QueryNodeStruct (&addr, 1);

  c = skip_to_word (t);
  t = skip_to_blank (c);

  if (qnode != NULL)
  {
    qnode->ExtraDirIn = (char *) malloc (strlen (c) + 2);
    strcpy (qnode->ExtraDirIn, c);
    if (qnode->ExtraDirIn[strlen (qnode->ExtraDirIn) - 1] != DIR_SEPC)
      strcat (qnode->ExtraDirIn, DIR_SEPS);
    error = kwd_ok;
  }
  else
    error = kwd_other_err;

  return error;
}

/* CE 980119 cfg_set_SyncHydraSessions */


static KWD_ERR
cfg_set_SyncHydraSessions (char *c)
{
  ADDR addr;
  NODESTRUC *qnode = NULL;
  KWD_ERR error = kwd_ok;
  char *t;

  t = c;

  parse_address (t, &addr);
  qnode = QueryNodeStruct (&addr, 1);

  if (qnode != NULL)
  {
    qnode->SyncHydraSessions = 1;
    error = kwd_ok;
  }
  else
    error = kwd_other_err;

  return error;
}


/* MR 970204 cfg_set_nodetimezone */

static KWD_ERR
cfg_set_nodetimezone (char *c)
{
  ADDR addr;
  NODESTRUC *qnode = NULL;
  KWD_ERR error = kwd_ok;
  char *nodeTZ, *tmpTZ;
  char junk[128 + 4];

  parse_address (c, &addr);
  c = skip_to_word (c);
  qnode = QueryNodeStruct (&addr, 1);

  if (qnode != NULL)
  {                             /* AW980208 changed NodeTimeZone behaviour */
    nodeTZ = ctl_string (c);    /* compute remote TZUTC time diff to UTC */
    tmpTZ = store_TZ ();        /* save old TZ */

    strcpy (junk, "TZ=");       /* temporary set TZ of remote node */
    strntcpy (junk + 3, nodeTZ, 128);
    putenv (junk);
    tzset ();
    qnode->tzutc = TIMEZONE;    /* get "his" time diff to UTC */

    putenv (tmpTZ);             /* restore old TZ */
    tzset ();
    if (TZ)
      free (TZ);                /* TZ variable was malloc'd */
    TZ = tmpTZ;                 /* set new old TZ */
  }
  else
    error = kwd_other_err;

  return error;
}


/*--------------------------------------------------------------------*/
/* Copies a string with ctl_string. Frees duplicate allocated strings */
/* returns kwd_other_err or kwd_nomem in case of failure, else kwd_ok */
/*--------------------------------------------------------------------*/

KWD_ERR
cfg_CopyString (int keyword, char **p, char *c)
{
  KWD_ERR error = kwd_ok;

  if (*p != NULL)
  {
    printf ("ERR: %s: can only store one value\n", config_lines[keyword - 1]);
    printf ("ERR: %s: BT will forget '%s' and use '%s'.\n",
            config_lines[keyword - 1], *p, c);
    free ((void *) *p);
    *p = NULL;
    error = kwd_other_err;
  }

  if (*c)
  {
    *p = ctl_string (c);

    if (*p == NULL)
      error = kwd_nomem;
  }
  else
    error = kwd_bad;

  return error;
}

static KWD_ERR
cfg_CopySlashString (int keyword, char **p, char *c)
{
  KWD_ERR error = kwd_ok;

  if (*p != NULL)
  {
    printf ("ERR: %s: can only store one value\n", config_lines[keyword - 1]);
    printf ("ERR: %s: BT will forget '%s' and use '%s'\n",
            config_lines[keyword - 1], *p, c);
    free ((void *) *p);
    *p = NULL;
    error = kwd_other_err;
  }

  if (*c)
  {
    *p = ctl_slash_string (binkdir, c);

    if (*p == NULL)
      error = kwd_nomem;
  }
  else
    error = kwd_bad;

  return error;
}

/* $Id: btconfig.c,v 1.20 1999/09/30 09:38:00 mr Exp $ */
