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
 * Filename    : $Source: E:/cvs/btxe/src/v7debug.c,v $
 * Revision    : $Revision: 1.5 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/03/22 03:47:43 $
 * State       : $State: Exp $
 *
 * Description : V7 debugger
 *
 *---------------------------------------------------------------------------*/

#include "includes.h"

static int LOCALFUNC btree (void);
static int LOCALFUNC _btree (long, int);
static struct _ndx far *LOCALFUNC get7node (FILE *, long, struct _ndx far *);
static void LOCALFUNC unpk (char *instr, char *outp, int many);
static int LOCALFUNC addr_compare (void *, void *, int);
static int LOCALFUNC name_compare (void *, void *, int);

static struct _ndx *noderef = NULL;  /* index file             */
static char aline[160];

static FILE *idx_fp, *dat_fp;

static int find_first;          /* is this findfirst or findnext ? */

static ADDR searchaddr;
static char searchname[40];

static void *desired;
static int desired_len;
static int (LOCALFUNC * compare_fn) (void *key, void *desired, int len);

static PV7RCB pv7rcb;

static PV7NODEL beginp, lastp, actp;

#define status_line(X) printf("#X")
#define share_fopen(fn, mode, share) fopen(fn, mode)

int
V7Initialize (void)
{                               /* allocate memory */
  if (noderef == NULL)
    noderef = _fmalloc (sizeof (struct _ndx));

  if (!noderef)
  {
    status_line (MSG_TXT (M_NODELIST_MEM));
    return V7NL_ERROR;
  }

  return V7NL_SUCCESS;
}

void
V7Finish (void)
{                               /* free memory */
  _ffree (noderef);
  noderef = NULL;
}

int
V7FindFirstNode (PV7RCB p)
{                               /* open node idx/dat and find first match */
  idx_fp = share_fopen (p->pszIndexName, read_binary, DENY_WRITE);
  if (idx_fp == NULL)
  {
    status_line (MSG_TXT (M_UNABLE_TO_OPEN), p->pszIndexName);
    return (V7NL_NODEINDEX_OPENFAILED);  /* no file, no work to do */
  }

  dat_fp = share_fopen (p->pszDataName, read_binary, DENY_WRITE);
  if (dat_fp == NULL)
  {
    status_line (MSG_TXT (M_UNABLE_TO_OPEN), p->pszDataName);
    return (V7NL_NODEDAT_OPENFAILED);  /* no file, no work to do */
  }

  searchaddr.Zone = p->pV7Node->ZoneNumber;
  searchaddr.Net = p->pV7Node->NetNumber;
  searchaddr.Node = p->pV7Node->NodeNumber;
  searchaddr.Point = p->pV7Node->HubNode;

  compare_fn = addr_compare;
  desired = &searchaddr;
  desired_len = p->wLen;

  find_first = 1;

  return btree ();
}

int
V7FindFirstSysOp (PV7RCB p)
{                               /* open sysop idx/dat and find first match */
  char last_name_first[26], midname[26], *c, *p2, *m;
  int namelen;

  idx_fp = share_fopen (p->pszSysOpIndex, read_binary, DENY_WRITE);
  if (idx_fp == NULL)
  {
    status_line (MSG_TXT (M_UNABLE_TO_OPEN), p->pszSysOpIndex);
    return (V7NL_SYSOPINDEX_OPENFAILED);  /* no file, no work to do */
  }

  dat_fp = share_fopen (p->pszDataName, read_binary, DENY_WRITE);
  if (dat_fp == NULL)
  {
    status_line (MSG_TXT (M_UNABLE_TO_OPEN), p->pszDataName);
    return (V7NL_NODEDAT_OPENFAILED);  /* no file, no work to do */
  }

  c = midname;                  /* Start of temp name buff   */
  p2 = p->pV7Node->SysopName;   /* Point to start of name    */
  m = NULL;                     /* Init pointer to space     */

  *c = *p2++;
  while (*c)                    /* Go entire length of name  */
  {
    if (*c == ' ')              /* Look for space            */
      m = c;                    /* Save location             */
    c++;
    *c = *p2++;
  }

  if (m != NULL)                /* If we have a pointer,     */
  {
    *m++ = '\0';                /* Terminate the first half  */
    strcpy (last_name_first, m);  /* Now copy the last name    */
    strcat (last_name_first, ", ");  /* Insert a comma and space  */
    strcat (last_name_first, midname);  /* Finally copy first half */
  }
  else
    strcpy (last_name_first, midname);  /* Use whole name otherwise */

  namelen = (int) strlen (last_name_first);  /* Calc length now           */
  namelen = min (25, namelen);

  strncpy (searchname, last_name_first, namelen);
  searchname[namelen] = '\0';

  compare_fn = name_compare;
  desired = &searchname;
  desired_len = namelen;

  find_first = 1;

  return btree ();
}

void
V7FindClose (PV7RCB p)
{                               /* close files */
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
}

static int LOCALFUNC
foundmatch (long datpos, int level)
{
  struct _vers7 vers7;
  char aline[160], aline2[160];
  char *cp;
  char PhoneNumber[40], Password[9], SystemName[34], SysopName[26], MiscInfo[30];
  char *indent;

  if (fseek (dat_fp, datpos, SEEK_SET))
  {                             /* point at record  */
    status_line (MSG_TXT (M_NODELIST_SEEK_ERR), pv7rcb->pszDataName);
    return 1;
  }

  if (fread ((char *) &vers7, sizeof (struct _vers7), 1, dat_fp) != 1)
  {
    status_line (MSG_TXT (M_NODELIST_REC_ERR), pv7rcb->pszDataName);
    return 1;
  }

  memset (PhoneNumber, 0, 40);
  fread (PhoneNumber, vers7.Phone_len, 1, dat_fp);

  memset (Password, 0, 9);
  fread (Password, vers7.Password_len, 1, dat_fp);

  memset (aline, '\0', 160);
  memset (aline2, '\0', 160);

  if (fread (aline2, vers7.pack_len, 1, dat_fp) != 1)
  {
    status_line (MSG_TXT (M_NODELIST_REC_ERR), pv7rcb->pszDataName);
    return 1;
  }

  unpk (aline2, aline, vers7.pack_len);

  cp = &aline[0];

  memset (SystemName, 0, 34);
  memcpy (SystemName, cp, min (33, vers7.Bname_len));
  cp += vers7.Bname_len;

  memset (SysopName, 0, 26);
  memcpy (SysopName, cp, min (25, vers7.Sname_len));
  cp += vers7.Sname_len;

  memset (MiscInfo, 0, 30);
  memcpy (MiscInfo, cp, min (29, vers7.Cname_len));

  indent = "----------"[9 - level];

  printf ("%-10s %-32s ", indent, SysopName);

  sprintf (aline,
           "%d:%d/%d.%d",
           (int) vers7.Zone,
           (int) vers7.Net,
           (int) vers7.Node,
           (int) (vers7.NodeFlags & B_point) ? vers7.HubNode : 0);
  printf ("%-16s ", aline);

  printf ("%2s %5d %3d '%-8s'\n",
          (vers7.NodeFlags & B_CM) ? "CM" : "  ",
          (int) vers7.BaudRate * 300,
          (int) vers7.ModemType,
          (Password[0] != '\0') ? "-" : Password);

  return 0;
}

static int LOCALFUNC
name_compare (void *key, void *desired, int _len)
{
  return (strnicmp ((char *) key, (char *) desired, (unsigned int) min (desired_len, _len)));
}

static int LOCALFUNC
addr_compare (void *key, void *desired, int _len)
{
  int k, len = desired_len;     /* min(desired_len, _len); */

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

  if (_len == 6)                /* spezial treatment for key point */
    ((ADDRP) key)->Point = 0;

  if (len >= 8)
  {
    k = ((ADDRP) key)->Point - ((ADDRP) desired)->Point;
    if (k)
      return (k);
  }
  else
    return 0;

  return 0;
}

/* General V7 nodelist engine. Used by both the by-node and by-sysop
 * lookups.
 *
 * Thanks to Phil Becker for showing me how nice it looks in assembler.
 * It helped me see how nice it could be in C.
 *
 * (I know, Phil, it's still nicer in assembler!) */

static int LOCALFUNC
btree (void)
{
  /* Get CtlRec */
  if (get7node (idx_fp, 0L, noderef) == NULL)
    return 1;

  return _btree (noderef->ndx.CtlBlk.CtlRoot, 0);  /* start recursion */
}

static int LOCALFUNC
_btree (long record, int level)
{
  struct _ndx nodeidx;
  int j, j_done = -1, k, l, ret = 0, count;
  struct _IndxRef far *ip = NULL;
  struct _LeafRef far *lp = NULL;
  char far *tp;
  char *np;

  if (get7node (idx_fp, record * noderef->ndx.CtlBlk.CtlBlkSize, &nodeidx) == NULL)
    return 1;

  /* Follow the node tree until we either match a key right in the index
   * node, or locate the leaf node which must contain the entry. */

  if (nodeidx.ndx.INodeBlk.IndxFirst != -1)  /* INodeBlk */
  {
    count = nodeidx.ndx.INodeBlk.IndxCnt;  /* count may be 0 */
    if (count == 0)
    {
      status_line (">Down 1");
      ret = _btree (nodeidx.ndx.INodeBlk.IndxFirst, level + 1);
      status_line (">Up   1");
    }
    else
    {
      for (j = 0; j < count; j++)  /* check 20 or less */
      {
        ip = &(nodeidx.ndx.INodeBlk.IndxRef[j]);
        tp = (char far *) &nodeidx + ip->IndxOfs;

        k = l = ip->IndxLen;    /* binary length in bytes */
        /* addresses are stored as ADDR structs */
        /* names are stored as "2ndName, 1stName" */

        for (np = aline; k > 0; k--)
          *np++ = *tp++;

        k = (*compare_fn) ((void *) aline, desired, l);  /* k=key-desired */

        if (k > 0)
          break;

        if (k == 0)
        {
          /* Key matches in the index node. Since we're just doing lookup, we
           * can assume its pointer is valid. If we were doing updates, that
           * assumption would not work, because leaf nodes update first. So in
           * an update environment, the entire code segment relating to k == 0
           * should not execute, and we should walk the tree all the way down.
           */

          if (j == 0)
          {
            status_line (">Down 2");
            ret = ret + _btree (nodeidx.ndx.INodeBlk.IndxFirst, level + 1);
            status_line (">Up   2");
          }
          else if (j - 1 != j_done)
          {
            status_line (">Down 3");
            ret = ret + _btree (nodeidx.ndx.INodeBlk.IndxRef[j - 1].IndxPtr, level + 1);
            status_line (">Up   3");
          }

          /* foundmatch (nodeidx.ndx.INodeBlk.IndxRef[j].IndxData); */
          status_line (">Down 4");
          ret = ret + _btree (nodeidx.ndx.INodeBlk.IndxRef[j].IndxPtr, level + 1);
          status_line (">Up   4");
          j_done = j;
        }
      }

      if (k != 0)
      {
        if (j == 0)
        {
          status_line (">Down 5");
          ret = ret + _btree (nodeidx.ndx.INodeBlk.IndxFirst, level + 1);
          status_line (">Up   5");
        }
        else if (j - 1 != j_done)
        {
          status_line (">Down 6");
          ret = ret + _btree (nodeidx.ndx.INodeBlk.IndxRef[j - 1].IndxPtr, level + 1);
          status_line (">Up   6");
        }
      }
    }
  }
  else
  {                             /* LNodeBlk */
    count = nodeidx.ndx.LNodeBlk.IndxCnt;  /* count may be 0 */

    /* Search for a higher key */
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
        ret = ret + foundmatch (nodeidx.ndx.LNodeBlk.LeafRef[j].KeyVal, level);
    }
  }

  return ret;
}

static struct _ndx far *LOCALFUNC
get7node (FILE * stream, long pos, struct _ndx far * ndx)
{
  fseek (stream, pos, SEEK_SET);

  if (fread (ndx, sizeof (struct _ndx), 1, stream) != 1)
  {
    status_line (MSG_TXT (M_NODELIST_READ_ERR));
    return (NULL);
  }
  return (ndx);
}

/* ====================================================================
 * unpack a dense version of a symbol (base 40 polynomial)
 * ==================================================================== */

static void LOCALFUNC
unpk (char *instr, char *outp, int count)
{
  /* This table has been modified to minimize searches */
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

  outp[0] = '\0';

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
  }
}

int
main (int argc, char *argv[])
{
  char *sysndxfname = argv[1];  /* sysop index filename     */
  char *adrndxfname = argv[2];  /* address index filename   */
  char *datfname = argv[3];     /* data filename            */
  V7RCB V7rcb;                  /* request block            */
  V7NODE V7node;                /* V7 format node record    */
  int V7rc;                     /* return code              */
  int count;

  V7Initialize ();
  memset (&V7rcb, '\0', sizeof (V7RCB));
  V7rcb.pszSysOpIndex = sysndxfname;
  V7rcb.pszDataName = datfname;
  V7rcb.pV7Node = &V7node;
  memset (&V7node, '\0', sizeof (V7NODE));
  strcpy (V7node.SysopName, "");
  V7FindFirstSysOp (&V7rcb);
  V7FindClose (&V7rcb);
  V7Finish ();
}

/* $Id: v7debug.c,v 1.5 1999/03/22 03:47:43 mr Exp $ */
