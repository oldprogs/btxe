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
 * Filename    : $Source: E:/cvs/btxe/src/version7.c,v $
 * Revision    : $Revision: 1.9 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/27 20:51:39 $
 * State       : $State: Exp $
 * Orig. Author: Bill Andrus, Vince Perriello; V7+ support by Thomas Waldmann
 *
 * Description : BinkleyTerm Version7 Nodelist processing module.
 *
 * Note        : V7+ works, but is far from being completed.
 *
 *---------------------------------------------------------------------------*/

// Include this file before any other includes or defines!

#include "includes.h"

#ifdef PORTABLEBINFILES
#include "foffsets.h"
#endif

static int LOCALFUNC btree (PV7RCB);
static int LOCALFUNC _btree (PV7RCB, long);
static int LOCALFUNC get7node (PV7RCB, long, struct _ndx far *);
static int LOCALFUNC unpk (char *instr, char *outp, int many);
static int LOCALFUNC addr_compare (void *, void *, int);
static int LOCALFUNC name_compare (void *, void *, int);
static void LOCALFUNC set_vars (PV7NODE, ADDRP);

extern int get_nodelist_name (ADDRP);

extern char *nodelist_base;
extern char *nodelist_name;

// r. hoerner: added "far" statement 1/3/97
static struct _ndx far *noderef = NULL;  // far pointer index file

static char aline[160];

static FILE *idx_fp, *dat_fp, *dtp_fp;

static int find_first;          // is this findfirst or findnext ?

static ADDR searchaddr;
static char searchname[40];

static void *desired;
static int desired_len;
static int (LOCALFUNC * compare_fn) (void *key, void *desired, int len);

static PV7NODEL beginp, lastp, actp;

static DTPHEADER dtphdr;


// call this function before you use any other V7* function!

int
V7Initialize (void)
{

  if (node_index == NULL)
    node_index = malloc (sizeof (struct _ndx));

  if (!node_index)
  {
    status_line (MSG_TXT (M_NODELIST_MEM));
    return V7_OUT_OF_MEMORY;
  }

  if (noderef == NULL)
    noderef = malloc (sizeof (struct _ndx));

  if (!noderef)
  {
    status_line (MSG_TXT (M_NODELIST_MEM));
    return V7_OUT_OF_MEMORY;
  }

  return V7_SUCCESS;
}

// call this function when you're finished with V7* functions

int
V7Finish (void)
{

  if (node_index)
    free (node_index);
  node_index = NULL;

  if (noderef)
    free (noderef);
  noderef = NULL;

  return V7_SUCCESS;
}

// builds names of V7 / V7+ files

void
V7buildnames (PV7RCB p, char *IndexName, char *DataName, char *DTPName)
{
  char *fmt = "%s%s.%s";

  sprintf (DataName, fmt, p->pszPath, p->pszName, "dat");
  if ((p->miscflags & MF_v7plus) > 0)
    sprintf (DTPName, fmt, p->pszPath, p->pszName, "dtp");

  switch (p->whichindex)
  {
  case WI_ndx:
    sprintf (IndexName, fmt, p->pszPath, p->pszName, "ndx");
    break;

  case WI_sdx:
    if (p->miscflags & MF_sysopndx)
      sprintf (IndexName, fmt, p->pszPath, "sysop", "ndx");
    else
      sprintf (IndexName, fmt, p->pszPath, p->pszName, "sdx");
    break;

  case WI_pdx:
    sprintf (IndexName, fmt, p->pszPath, p->pszName, "pdx");
    break;
  }
}

// find all matching V7 entries and return first of them

int
V7FindFirst (PV7RCB p)
{

  char IndexName[PATHLEN], DataName[PATHLEN], DTPName[PATHLEN];
  char last_name_first[26], midname[26], *c, *p2, *m;
  int namelen;

  V7buildnames (p, IndexName, DataName, DTPName);

  idx_fp = share_fopen (IndexName, read_binary, DENY_WRITE);
  if (idx_fp == NULL)
  {
    status_line (MSG_TXT (M_UNABLE_TO_OPEN), IndexName);
    return (V7_IDX_OPENFAILED); /* no file, no work to do */
  }

  dat_fp = share_fopen (DataName, read_binary, DENY_WRITE);
  if (dat_fp == NULL)
  {
    status_line (MSG_TXT (M_UNABLE_TO_OPEN), DataName);
    fclose (idx_fp);
    idx_fp = NULL;              /* CEH, 971227 */
    return (V7_DAT_OPENFAILED); /* no file, no work to do */
  }

  if ((p->miscflags & MF_v7plus) > 0)
  {
    dtp_fp = share_fopen (DTPName, read_binary, DENY_WRITE);
    if (dtp_fp == NULL)
    {
      status_line (MSG_TXT (M_UNABLE_TO_OPEN), DTPName);
      fclose (idx_fp);
      idx_fp = NULL;            /* CEH, 971227 */
      fclose (dat_fp);
      dat_fp = NULL;            /* CEH, 971227 */
      return (V7_DTP_OPENFAILED);  /* no file, no work to do */
    }

#ifndef PORTABLEBINFILES
    if (fread (&dtphdr, sizeof (dtphdr), 1, dtp_fp) < 1)
    {
      fclose (idx_fp);
      idx_fp = NULL;            // CEH, 971227

      fclose (dat_fp);
      dat_fp = NULL;            // CEH, 971227

      fclose (dtp_fp);
      dtp_fp = NULL;            // CEH, 971227

      return V7_DTP_READERROR;
    }
#else
    {
      unsigned char buf[11], *bufp = buf;

      if (fread (&buf, 11, 1, dtp_fp) < 1)
      {
        fclose (idx_fp);
        idx_fp = NULL;          // CEH, 971227

        fclose (dat_fp);
        dat_fp = NULL;          // CEH, 971227

        fclose (dtp_fp);
        dtp_fp = NULL;          // CEH, 971227

        return V7_DTP_READERROR;
      }
      dtphdr.control.size = get_word (buf + OFS_DTP_CTRL + OFS_DTP_size_W);
      dtphdr.control.version = *(bufp + OFS_DTP_CTRL + OFS_DTP_versi_B);
      dtphdr.control.AllFixSize = *(bufp + OFS_DTP_CTRL + OFS_DTP_allfs_B);
      dtphdr.control.AddFixSize = *(bufp + OFS_DTP_CTRL + OFS_DTP_addfs_B);
      dtphdr.TopLink.ndowns = get_word (buf + OFS_DTP_TOPL + OFS_DTP_ndowns_W);
      dtphdr.TopLink.FlOfs = get_long (buf + OFS_DTP_TOPL + OFS_DTP_FlOfs_L);
    }
#endif

  }

  switch (p->whichindex)
  {
  case WI_ndx:
    searchaddr.Zone = p->pV7Node->ZoneNumber;
    searchaddr.Net = p->pV7Node->NetNumber;
    searchaddr.Node = p->pV7Node->NodeNumber;
    searchaddr.Point = p->pV7Node->HubNode;
    compare_fn = addr_compare;
    desired = &searchaddr;
    desired_len = p->wLen;
    break;

  case WI_sdx:
    c = midname;                /* Start of temp name buff */
    p2 = p->pV7Node->SysopName; /* Point to start of name */
    m = NULL;                   /* Init pointer to space */
    *c = *p2++;
    while (*c)                  /* Go entire length of name */
    {
      if (*c == ' ')            /* Look for space */
        m = c;                  /* Save location */
      c++;
      *c = *p2++;
    }

    if (m != NULL)              /* If we have a pointer, */
    {
      *m++ = '\0';              /* Terminate the first half */
      strcpy (last_name_first, m);  /* Now copy the last name */
      strcat (last_name_first, ", ");  /* Insert a comma and space */
      strcat (last_name_first, midname);  /* Finally copy first half */
    }
    else
      strcpy (last_name_first, midname);  /* Use whole name otherwise */

    fancy_str (last_name_first);  /* Get caps in where needed */
    namelen = (int) strlen (last_name_first);  /* Calc length now */
    namelen = min (25, namelen);

    strncpy (searchname, last_name_first, namelen);
    searchname[namelen] = '\0';

    compare_fn = name_compare;
    desired = &searchname;
    desired_len = namelen;
    break;

  case WI_pdx:
    /* not implemented yet! */
    break;
  }

  find_first = 1;
  return V7FindNext (p);
}

// return next match

int
V7FindNext (PV7RCB p)
{
  int ret;
  PV7NODEL p1, p2;

  if (find_first)
  {
    beginp = lastp = actp = NULL;
    if (btree (p) != 0)
      return V7_FINDNEXT_END;
  }

  if (find_first)
    actp = beginp;
  else
    actp = actp->next;

  if (actp == NULL)
  {                             /* if this is the end, free memory */
    for (p1 = beginp; p1; p1 = p2)
    {
      p2 = p1->next;
      free (p1);
    }

    beginp = NULL;
    ret = V7_FINDNEXT_END;
  }
  else
  {
    ret = V7_SUCCESS;
    *(p->pV7Node) = actp->v7n;
  }

  find_first = 0;
  return ret;
}

// call this after findnext returns V7_FINDNEXT_END

int
V7FindClose (PV7RCB p)
{
  PV7NODEL p1, p2;

  p = p;                        /* Make compiler happy */

  if (beginp)
  {
    for (p1 = beginp; p1; p1 = p2)
    {
      p2 = p1->next;
      free (p1);
    }
  }

  beginp = NULL;

  if (idx_fp)
  {
    fclose (idx_fp);
    idx_fp = NULL;
  }

  if (dat_fp)
  {
    fclose (dat_fp);
    dat_fp = NULL;
  }

  if ((p->miscflags & MF_v7plus) > 0)
  {
    if (dtp_fp)
    {
      fclose (dtp_fp);
      dtp_fp = NULL;
    }
  }

  return V7_SUCCESS;
}

// find a single match in V7

int
V7Find (PV7RCB p)
{
  int ret;

  V7Initialize ();
  ret = V7FindFirst (p);
  V7FindClose (p);
  // V7Finish();  // don't call this and avoid many malloc/free calls 
  return ret;
}

// conversion char -> minute of day (time is UTC!), see FSC-0062

short
minute_of_day (char c)
{
  char upper_c;
  short hour, minutes;

  upper_c = toupper (c);

  if (upper_c > 'X')
    return -1;                  /* invalid ! */

  hour = upper_c - 'A';         /* 00 .. 23 */
  minutes = hour * 60;

  if (islower (c))              /* uppercase -> time=hh:00, lowercase -> time=hh:30 */
    minutes += 30;

  return minutes;
}

// process raw nodelist flags

static int LOCALFUNC
process_flag (V7NODE * pv7, char *rawflag)
{
  static short userflag;
  char *p = rawflag;

  if (!p)                       /* init call */
    userflag = 0;
  else
  {
    if (toupper (*p) == 'U')
    {                           /* from now on: user flags! */
      userflag = 1;
      p++;
    }

    if (*p)                     /* anything left (after ",U") ? */
    {
      // ,Txy online time flag as in FSC-0062 v3 !?
      if (userflag && !(pv7->NodeFlags & B_CM) && toupper (*p) == 'T')
      {
        short start, end;

        // status_line(" Online flags: %s", p+1);
        if ((start = minute_of_day (*++p)) < 0 || (end = minute_of_day (*++p)) < 0)
          status_line (MSG_TXT (M_V7P_INVALID_TXY), rawflag);
        else
        {
          pv7->online_start = start;
          pv7->online_end = end;
        }
      }
    }
  }

  return 0;
}

// parse / process raw nodelist line

static int LOCALFUNC
process_raw (V7NODE * pv7, char *raw)
{
  int field = 0;
  int len;
  char *p;

  process_flag (pv7, NULL);     /* init flag function */

  if (*raw == ',')
  {
    field++;                    /* first field is empty, but counts! */
    raw++;
  }

  pv7->rawFlags[0] = '\0';

  for (p = strtok (raw, ","); p; field++, p = strtok (NULL, ","))
  {
    switch (field)
    {
    case 0:                    /* Node, Hub, Host, Region, Zone */
      strcpy (pv7->rawType, p);
      break;

    case 1:                    /* Zone/Net/Node/Point number */
      break;                    /* nothing to do with that yet! */

    case 2:                    /* System name */
      strcpy (pv7->rawSystemName, p);
      break;

    case 3:                    /* Location / City */
      strcpy (pv7->rawMiscInfo, p);
      break;

    case 4:                    /* Sysop */

      strcpy (pv7->rawSysopName, p);
      break;

    case 5:                    /* Phone */
    case 6:                    /* Baud */
      break;                    /* nothing to do with that yet! */

    default:                   /* >=7: flags: CM,XA,... */
      len = strlen (pv7->rawFlags);  /* AW 980124: rawFlags has max. 64 Bytes */
      if (len + strlen (p) + 1 < 64)
      {                         /* TJW 980125: only add comma AND field */
        if (field > 7)
          strcat (pv7->rawFlags, ",");
        strcat (pv7->rawFlags, p);
      }

      process_flag (pv7, p);
      break;
    }
  }

  return 0;
}

// V7 dat lookup at datpos

static int LOCALFUNC
foundmatch (PV7RCB prcb, long datpos)
{
  struct _vers7 vers7;
  char a_line[400], a_line2[256];
  char *cp, *cp2;
  long dtpoffset;
  int i, unpack_len;
  unsigned short fieldlen;
  char fieldbuf[1024];

  PV7NODEL p;

  if ((p = (PV7NODEL) calloc (1, sizeof (V7NODEL))) == NULL)
    return 1;

  strcpy (lastfound_SysopName, "(unknown)");

  if (beginp == NULL)
    beginp = p;
  else
    lastp->next = p;
  lastp = p;
  lastp->next = NULL;

  if (fseek (dat_fp, datpos, SEEK_SET))
  {                             /* point at record */
    status_line (MSG_TXT (M_NODELIST_SEEK_ERR), prcb->pszName, "dat");
    return 1;
  }

#ifndef PORTABLEBINFILES
  if (fread ((char *) &vers7, sizeof (struct _vers7), 1, dat_fp) != 1)
  {
    status_line (MSG_TXT (M_NODELIST_READ_ERR), prcb->pszName, "dat");
    return 1;
  }
#else
  {
    unsigned char buf[22], *bufp = buf;

    if (fread (&buf, 22, 1, dat_fp) != 1)
    {
      status_line (MSG_TXT (M_NODELIST_READ_ERR), prcb->pszName, "dat");
      return 1;
    }
    vers7.Zone = get_short (bufp + OFS_v7_Zone_S);
    vers7.Net = get_short (bufp + OFS_v7_Net_S);
    vers7.Node = get_short (bufp + OFS_v7_Node_S);
    vers7.HubNode = get_short (bufp + OFS_v7_HubNode_S);
    vers7.CallCost = get_word (bufp + OFS_v7_CallCost_W);
    vers7.MsgFee = get_word (bufp + OFS_v7_MsgFee_W);
    vers7.NodeFlags = get_word (bufp + OFS_v7_NodeFlags_W);
    vers7.ModemType = *(bufp + OFS_v7_ModemType_B);
    vers7.Phone_len = *(bufp + OFS_v7_Phone_len_B);
    vers7.Password_len = *(bufp + OFS_v7_Password_len_B);
    vers7.Bname_len = *(bufp + OFS_v7_Bname_len_B);
    vers7.Sname_len = *(bufp + OFS_v7_Sname_len_B);
    vers7.Cname_len = *(bufp + OFS_v7_Cname_len_B);
    vers7.pack_len = *(bufp + OFS_v7_pack_len_B);
    vers7.BaudRate = *(bufp + OFS_v7_BaudRate_B);
  }
#endif

  p->v7n.ulOffset = datpos;
  p->v7n.ZoneNumber = vers7.Zone;
  p->v7n.NetNumber = vers7.Net;
  p->v7n.NodeNumber = vers7.Node;
  p->v7n.HubNode = vers7.HubNode;
  p->v7n.BaudRate = 300 * vers7.BaudRate;
  p->v7n.ModemType = vers7.ModemType;
  p->v7n.NodeFlags = vers7.NodeFlags;
  p->v7n.RealCost = vers7.CallCost;
  p->v7n.Cost = vers7.CallCost; /* ????? */

  fread (p->v7n.PhoneNumber, vers7.Phone_len, 1, dat_fp);
  fread (p->v7n.Password, vers7.Password_len, 1, dat_fp);

  memset (a_line, '\0', 400);
  memset (a_line2, '\0', 256);

  if (fread (a_line2, vers7.pack_len, 1, dat_fp) != 1)
  {
    status_line (MSG_TXT (M_NODELIST_READ_ERR), prcb->pszName, "dat");
    return 1;
  }

  unpack_len = unpk (a_line2, a_line, vers7.pack_len);

  cp = &a_line[0];

  // BBS / system name 
  i = vers7.Bname_len;
  memcpy (p->v7n.SystemName, cp, min (33, i));
  cp += i;
  unpack_len -= i;

  // Sysop name 
  i = vers7.Sname_len;
  memcpy (p->v7n.SysopName, cp, min (25, i));
  cp += i;
  unpack_len -= i;

  // City name 
  i = vers7.Cname_len;
  memcpy (p->v7n.MiscInfo, cp, min (29, i));
  cp += i;
  unpack_len -= i;

  // TJW 970409 V7+ extensions
  // check if there are 8 hex characters left == V7+ DTP offset
  for (cp2 = cp, i = 0; *cp2 && i < 8 && i < unpack_len; cp2++, i++)
  {
    if (!isxdigit (*cp2))
      break;
  }

  if ((prcb->miscflags & MF_v7plus) > 0)
  {                             /* v7+ enabled and */
    if (i == 8)
    {                           /* v7+ DTP offs detected */
      // status_line(" V7+ extension detected. Processing fields ...");
      sscanf (cp, "%8lx", &dtpoffset);
      // status_line(" V7+ DTP offset-value: '%lx'", dtpoffset);

      if (fseek (dtp_fp, dtpoffset, SEEK_SET))
      {
        status_line (MSG_TXT (M_NODELIST_SEEK_ERR), prcb->pszName, "dtp");
        return 1;
      }

      // skip DTPALLLNK structure (not used yet)
      // skip DTPNODELNK structure, nodes only (not used yet)
      if (vers7.NodeFlags & B_point)
        fseek (dtp_fp, (long) dtphdr.control.AllFixSize, SEEK_CUR);
      else
        fseek (dtp_fp, (long) dtphdr.control.AllFixSize
               + (long) dtphdr.control.AddFixSize, SEEK_CUR);

      fieldlen = 0xFFFFU;
      if (fread ((char *) &fieldlen, sizeof (fieldlen), 1, dtp_fp) != 1)
      {
        status_line (MSG_TXT (M_NODELIST_READ_ERR), prcb->pszName, "dtp");
        return 1;
      }
      if (fieldlen != 0xFFFFU)
      {
        // status_line (" V7+ Field %d, len=%d", field, (int) fieldlen);
        if (fread ((char *) &fieldbuf, (size_t) fieldlen, 1, dtp_fp) != 1)
        {
          status_line (MSG_TXT (M_NODELIST_READ_ERR), prcb->pszName, "dtp");
          return 1;

        }
        // status_line (" V7+ Field %d: %s", field, fieldbuf);
        process_raw (&(p->v7n), fieldbuf);
      }
    }
    else
    {
      status_line ("!Version7 Plus enabled, but not detected in this record");
      p->v7n.rawType[0] = '\0';
      p->v7n.rawFlags[0] = '\0';
      strcpy (p->v7n.rawSystemName, p->v7n.SystemName);
      strcpy (p->v7n.rawMiscInfo, p->v7n.MiscInfo);
      strcpy (p->v7n.rawSysopName, p->v7n.SysopName);
    }
  }
  else
  {                             /* V7+ not enabled, use V7 stuff */
    p->v7n.rawType[0] = '\0';
    p->v7n.rawFlags[0] = '\0';
    strcpy (p->v7n.rawSystemName, p->v7n.SystemName);
    strcpy (p->v7n.rawMiscInfo, p->v7n.MiscInfo);
    strcpy (p->v7n.rawSysopName, p->v7n.SysopName);
  }

  return 0;
}

// compare function for name lookup

static int LOCALFUNC
name_compare (void *key, void *desired, int _len)
{
  return (strnicmp ((char *) key, (char *) desired, (unsigned int) min (desired_len, _len)));
}

// compare function for address lookup

static int LOCALFUNC
addr_compare (void *key, void *desired, int _len)
{
  int k, len = desired_len;

  if (len >= 2)
  {
    k = ((ADDRP) key)->Zone - ((ADDRP) desired)->Zone;
    if (k)
      return (k);
  }
  else
    return 0;

  if (len >= 4)
  {
    k = ((ADDRP) key)->Net - ((ADDRP) desired)->Net;
    if (k)
      return (k);
  }
  else
    return 0;

  if (len >= 6)
  {
    k = ((ADDRP) key)->Node - ((ADDRP) desired)->Node;
    if (k)
      return (k);
  }
  else
    return 0;

  // Node matches.
  //
  // The rule for points:
  //  1) If len == 6, treat key value for Point as Zero.
  //  2) Return comparison of key Point and desired Point.

  if (_len == 6)                /* spezial treatment for key point */
    ((ADDRP) key)->Point = 0;

  if (len >= 8)
  {
    k = ((ADDRP) key)->Point - ((ADDRP) desired)->Point;
    if (k)
      return (k);
  }
  return 0;
}

// General V7 nodelist engine. Used by both the by-node and by-sysop
// lookups.
//
// Thanks to Phil Becker for showing me how nice it looks in assembler.
// It helped me see how nice it could be in C.
//
// (I know, Phil, it's still nicer in assembler!)
//
// B-Tree search function
// returns >0 on failure
//          0 on success

static int LOCALFUNC
btree (PV7RCB prcb)
{
  if (!get7node (prcb, 0L, noderef))  /* Get CtlRec */
    return 1;

  return _btree (prcb, noderef->ndx.CtlBlk.CtlRoot);  /* start recursion */
}

static int LOCALFUNC
_btree (PV7RCB prcb, long record)
{
  struct _ndx nodeidx;
  int j, j_done = -1, k = 0, l, ret = 0, count;
  struct _IndxRef far *ip = NULL;
  struct _LeafRef far *lp = NULL;
  char far *tp;
  char *np;

  if (!get7node (prcb, record * noderef->ndx.CtlBlk.CtlBlkSize, &nodeidx))
    return 1;

  // Follow the node tree until we either match a key right in the index
  // node, or locate the leaf node which must contain the entry.

  if (nodeidx.ndx.INodeBlk.IndxFirst != -1)  /* INodeBlk */
  {
    count = nodeidx.ndx.INodeBlk.IndxCnt;  /* count may be 0 */
    if (count == 0)
    {
      // status_line(">Down 1");
      ret = _btree (prcb, nodeidx.ndx.INodeBlk.IndxFirst);
      // status_line(">Up   1");
    }
    else
    {
      for (j = 0; j < count; j++)  /* check 20 or less */
      {
        ip = &(nodeidx.ndx.INodeBlk.IndxRef[j]);
        tp = (char far *) &nodeidx + ip->IndxOfs;

        k = l = ip->IndxLen;    // binary length in bytes
        // addresses are stored as ADDR structs
        // names are stored as "2ndName, 1stName"

        for (np = aline; k > 0; k--)
          *np++ = *tp++;

        k = (*compare_fn) ((void *) aline, desired, l);  /* k=key-desired */
        if (k > 0)
          break;

        if (k == 0)
        {
          // Key matches in the index node. Since we're just doing lookup, we
          // can assume its pointer is valid. If we were doing updates, that
          // assumption would not work, because leaf nodes update first. So in
          // an update environment, the entire code segment relating to k == 0
          // should not execute, and we should walk the tree all the way down.

          if (j == 0)
          {
            // status_line(">Down 2");
            ret = ret + _btree (prcb, nodeidx.ndx.INodeBlk.IndxFirst);
            // status_line(">Up   2");
          }
          else if (j - 1 != j_done)
          {
            // status_line(">Down 3");
            ret = ret + _btree (prcb, nodeidx.ndx.INodeBlk.IndxRef[j - 1].IndxPtr);
            // status_line(">Up   3");
          }

          // foundmatch (nodeidx.ndx.INodeBlk.IndxRef[j].IndxData);
          // status_line(">Down 4");
          ret = ret + _btree (prcb, nodeidx.ndx.INodeBlk.IndxRef[j].IndxPtr);
          // status_line(">Up   4");
          j_done = j;
        }
      }

      if (k != 0)
      {
        if (j == 0)
        {
          // status_line(">Down 5");
          ret = ret + _btree (prcb, nodeidx.ndx.INodeBlk.IndxFirst);
          // status_line(">Up   5");
        }
        else if (j - 1 != j_done)
        {
          // status_line(">Down 6");
          ret = ret + _btree (prcb, nodeidx.ndx.INodeBlk.IndxRef[j - 1].IndxPtr);
          // status_line(">Up   6");
        }
      }
    }
  }
  else
  {                             /* LNodeBlk */
    count = nodeidx.ndx.LNodeBlk.IndxCnt;  /* count may be 0 */

    // Search for a higher key
    for (j = 0; j < count; j++) /* check 30 or less */
    {
      lp = &(nodeidx.ndx.LNodeBlk.LeafRef[j]);
      tp = (char far *) &nodeidx + lp->KeyOfs;
      k = l = lp->KeyLen;

      for (np = aline; k > 0; k--)
        *np++ = *tp++;

      k = (*compare_fn) ((void *) aline, desired, l);

      if (k > 0)
        break;
      else if (k == 0)
        ret = ret + foundmatch (prcb, nodeidx.ndx.LNodeBlk.LeafRef[j].KeyVal);
    }
  }

  return ret;
}

// read index entry

static int LOCALFUNC
get7node (PV7RCB prcb, long position, struct _ndx far * ndx)
{
  if (fseek (idx_fp, position, SEEK_SET))
  {
    status_line (MSG_TXT (M_NODELIST_SEEK_ERR), prcb->pszName, "?dx");
    return (0);
  }

#ifndef PORTABLEBINFILES
  if (fread (ndx, sizeof (struct _ndx), 1, idx_fp) != 1)
  {
    status_line (MSG_TXT (M_NODELIST_READ_ERR), prcb->pszName, "?dx");
    return (0);
  }
#else
  {
    unsigned char buf[512], *bufp = buf, *refp;
    long i;

    if (fread (buf, 512, 1, idx_fp) != 1)
    {
      status_line (MSG_TXT (M_NODELIST_READ_ERR), prcb->pszName, "?dx");
      return (0);
    }

    if (!position)
    {
      ndx->ndx.CtlBlk.CtlBlkSize = get_word (bufp + OFS_Ctl_BlkSize_W);
      ndx->ndx.CtlBlk.CtlRoot = get_long (bufp + OFS_Ctl_Root_L);
      ndx->ndx.CtlBlk.CtlHiBlk = get_long (bufp + OFS_Ctl_HiBlk_L);
      ndx->ndx.CtlBlk.CtlLoLeaf = get_long (bufp + OFS_Ctl_LoLeaf_L);
      ndx->ndx.CtlBlk.CtlHiLeaf = get_long (bufp + OFS_Ctl_HiLeaf_L);
      ndx->ndx.CtlBlk.CtlFree = get_long (bufp + OFS_Ctl_Free_L);
      ndx->ndx.CtlBlk.CtlLvls = get_word (bufp + OFS_Ctl_Lvls_W);
      ndx->ndx.CtlBlk.CtlParity = get_word (bufp + OFS_Ctl_Parity_W);
    }
    else
    {
      i = get_long (bufp + OFS_Node_First_L);
      if (i != -1)
      {
        ndx->ndx.INodeBlk.IndxFirst = i;
        ndx->ndx.INodeBlk.IndxBLink = get_long (bufp + OFS_Node_BLink_L);
        ndx->ndx.INodeBlk.IndxFLink = get_long (bufp + OFS_Node_FLink_L);
        ndx->ndx.INodeBlk.IndxCnt = get_short (bufp + OFS_Node_Cnt_S);
        ndx->ndx.INodeBlk.IndxStr = get_word (bufp + OFS_Node_Str_W);
        for (i = 0, refp = bufp + OFS_Node_Ref_Strt; i < 41; i++, refp += OFS_ILength)
        {
          ndx->ndx.INodeBlk.IndxRef[i].IndxOfs = get_word (refp + OFS_IOfs_W);
          ndx->ndx.INodeBlk.IndxRef[i].IndxLen = get_word (refp + OFS_ILen_W);
          ndx->ndx.INodeBlk.IndxRef[i].IndxData = get_long (refp + OFS_IData_L);
          ndx->ndx.INodeBlk.IndxRef[i].IndxPtr = get_long (refp + OFS_IPtr_L);
        }
        ndx->ndx.INodeBlk.IndxRef[41].IndxOfs = get_word (refp + OFS_IOfs_W);
        ndx->ndx.INodeBlk.IndxRef[41].IndxLen = get_word (refp + OFS_ILen_W);
      }
      else
      {
        ndx->ndx.LNodeBlk.IndxFirst = i;
        ndx->ndx.LNodeBlk.IndxBLink = get_long (bufp + OFS_Node_BLink_L);
        ndx->ndx.LNodeBlk.IndxFLink = get_long (bufp + OFS_Node_FLink_L);
        ndx->ndx.LNodeBlk.IndxCnt = get_short (bufp + OFS_Node_Cnt_S);
        ndx->ndx.LNodeBlk.IndxStr = get_word (bufp + OFS_Node_Str_W);
        for (i = 0, refp = bufp + OFS_Node_Ref_Strt; i < 62; i++, refp += OFS_LLength)
        {
          ndx->ndx.LNodeBlk.LeafRef[i].KeyOfs = get_word (refp + OFS_LOfs_W);
          ndx->ndx.LNodeBlk.LeafRef[i].KeyLen = get_word (refp + OFS_LLen_W);
          ndx->ndx.LNodeBlk.LeafRef[i].KeyVal = get_long (refp + OFS_LVal_L);
        }
      }
    }
  }
#endif

  return (1);
}

// unpack a dense version of a symbol (base 40 polynomial)

static int LOCALFUNC
unpk (char *instr, char *outp, int count)
{
  // This table has been modified to minimize searches 
  static char unwrk[] = " EANROSTILCHBDMUGPKYWFVJXZQ-'0123456789";

  struct chars
  {
    unsigned char c1;
    unsigned char c2;
  };

  union
  {
    unsigned short w1;
    struct chars d;
  }
  u;

  register short i, j;          /* MB int */
  char obuf[4];
  int outlen;

  outp[0] = '\0';
  outlen = 0;

  while (count)
  {
    u.d.c1 = *instr++;
    u.d.c2 = *instr++;
    count -= 2;
    for (j = 2; j >= 0; j--)
    {
      i = u.w1 % 40;
      u.w1 /= 40;
      obuf[j] = unwrk[i];
    }

    obuf[3] = '\0';
    strcat (outp, obuf);
    outlen += 3;
  }

  return outlen;
}

// these are for Binkley's use:
//
// address lookup
// 1 = success
// 0 = failure

int
ver7find (ADDRP opus_addr, int have_boss_data)
{
  ADDR faddr;
  V7RCB v7rcb;
  V7NODE v7n;

  strcpy (lastfound_SysopName, "(unknown)");
  have_boss_data = have_boss_data;  /* make compiler happy */
  memset (&newnodedes, 0, sizeof (struct _newnode));

  // status_line(" clearing newnodedes");

  memset (&v7rcb, 0, sizeof (v7rcb));
  memset (&v7n, 0, sizeof (v7n));
  v7rcb.pV7Node = &v7n;

  if (!get_nodelist_name (opus_addr))
    return 0;

  v7rcb.pszPath = (PSZ) net_info;
  v7rcb.pszName = (PSZ) nodelist_base;
  v7rcb.whichindex = WI_ndx;

  v7rcb.miscflags |= (version7 > 1) ? 1 : 0;
  v7rcb.miscflags |= (sysopndx) ? MF_sysopndx : 0;

  v7n.ZoneNumber = opus_addr->Zone;
  v7n.NetNumber = opus_addr->Net;
  v7n.NodeNumber = opus_addr->Node;
  v7n.HubNode = opus_addr->Point;
  v7rcb.wLen = 8;

  if (V7Find (&v7rcb) == V7_SUCCESS)
  {
    set_vars (&v7n, &faddr);
    return 1;
  }
  else
    return 0;
}

// name lookup

void
opususer (char *name, ADDRP faddr)
{
  V7RCB v7rcb;
  V7NODE v7n;
  int namelen;

  strcpy (lastfound_SysopName, "(unknown)");
  faddr->Zone = faddr->Net = faddr->Node = faddr->Point = (unsigned short) -1;
  faddr->Domain = NULL;

  memset (&newnodedes, 0, sizeof (struct _newnode));

  memset (&v7rcb, 0, sizeof (v7rcb));
  memset (&v7n, 0, sizeof (v7n));
  v7rcb.pV7Node = &v7n;

  if (nodelist_base == NULL)
    get_nodelist_name (&my_addr);

  v7rcb.pszPath = (PSZ) net_info;
  v7rcb.pszName = (PSZ) nodelist_base;
  v7rcb.whichindex = WI_sdx;

  v7rcb.miscflags |= (version7 > 1) ? 1 : 0;
  v7rcb.miscflags |= (sysopndx) ? MF_sysopndx : 0;

  namelen = min (25, strlen (name));
  strncpy (v7n.SysopName, name, namelen);
  v7n.SysopName[namelen] = '\0';
  v7rcb.wLen = namelen;

  if (V7Find (&v7rcb) == V7_SUCCESS)
    set_vars (&v7n, faddr);
}

// set BT's variables

static void LOCALFUNC
set_vars (PV7NODE p, ADDRP faddr)
{
  // status_line(" setting newnodedes: %u/%u.%u ci: %d",
  //             p->NetNumber,p->NodeNumber,p->HubNode,p->RealCost);
  newnodedes.NetNumber = p->NetNumber;
  newnodedes.NodeNumber = p->NodeNumber;
  newnodedes.HubNode = p->HubNode;
  newnodedes.BaudRate = (byte) (p->BaudRate / 300);
  newnodedes.ModemType = p->ModemType;
  newnodedes.NodeFlags = p->NodeFlags;
  newnodedes.online_start = p->online_start;
  newnodedes.online_end = p->online_end;

  GetAltModemflag ((word *) & newnodedes.ModemType, faddr);
  GetAltFidoFlag ((word *) & newnodedes.NodeFlags, faddr);
  if (GetAltOnLineTime (&newnodedes.online_start, &newnodedes.online_end, faddr))
  {
    newnodedes.NodeFlags |= B_CM;  /* VRP 990820 */
  }

  newnodedes.Cost = p->RealCost;
  newnodedes.RealCost = p->RealCost;

  strcpy (lastfound_SysopName, p->SysopName);
  if (!nofancystrings)
    fancy_str (lastfound_SysopName);

  strcpy (newnodedes.SystemName, p->SystemName);
  if (!nofancystrings)
    fancy_str (newnodedes.SystemName);

  strcpy (newnodedes.PhoneNumber, p->PhoneNumber);

  strcpy (newnodedes.MiscInfo, p->MiscInfo);
  if (!nofancystrings)
    fancy_str (newnodedes.MiscInfo);

  memcpy (newnodedes.Password, p->Password, 8);

  found_zone = p->ZoneNumber;
  found_net = p->NetNumber;

  faddr->Zone = p->ZoneNumber;
  faddr->Net = p->NetNumber;
  faddr->Node = p->NodeNumber;

  if (p->NodeFlags & B_point)
    faddr->Point = p->HubNode;
  else
    faddr->Point = 0;

  faddr->Domain = NULL;
}

/* $Id: version7.c,v 1.9 1999/09/27 20:51:39 mr Exp $ */
