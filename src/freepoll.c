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
 * Filename    : $Source: E:/cvs/btxe/src/freepoll.c,v $
 * Revision    : $Revision: 1.13 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/30 17:50:09 $
 * State       : $State: Exp $
 * Orig. Author: Carsten Ellermann
 *
 * Description : BinkleyTerm FreePoll Function
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

/*
 * r.hoerner:
 * i removed it, and changed "netsize()" in mailovly.c so that it
 * matches this format:
 *
 * static long netsize (MAILP p, char *ext);
 */

static long filesize (ADDRP, char *);
static long dirsize (char *ExtraDir);

int
freepoll (ADDRP addr, FPP fpoll, time_t t)
{
  char trx_file[PATHLEN];
  char *dirname;
  FILE *trx;
  struct stat stat_buffer;
  long size;

  if (t)                        /* MR 970714             */
  {
    dirname = HoldAreaNameMunge (addr);  /* MR 970715 create missing dirs */

    delete_backslash (dirname);
    mkdir (dirname);

    if (addr->Point != 0)
    {
      sprintf (trx_file, "%s" DIR_SEPS "%04hx%04hx.pnt",
               dirname, addr->Net, addr->Node);
      mkdir (trx_file);
    }

    sprintf (trx_file, "%s" DIR_SEPS "%s.trx",
             dirname, Hex_Addr_Str (addr));

    if (unix_stat (trx_file, &stat_buffer))  /* get info from file    */
    {
      status_line (MSG_TXT (M_ACCEPT1STCALLEVER),  /* TJW 960530      */
                   Full_Addr_Str (addr),
                   fpoll->and_op ? "AND" : "OR");

      trx = fopen (trx_file, write_binary);  /* create first TRX-file */

      if (trx == NULL)
        status_line (MSG_TXT (M_WRITEPROTECTED), trx_file);
      else
        fclose (trx);           /* close new file        */

      return 1;                 /* accept call           */
    }

    trx = fopen (trx_file, write_binary);  /* touch file */

    if (trx == NULL)
    {
      status_line (MSG_TXT (M_WRITEPROTECTED), trx_file);
      return (1);
    }

    fclose (trx);

    if ((t < stat_buffer.st_mtime + fpoll->Delta_T) &&
        (t > stat_buffer.st_mtime + fpoll->MinDelta_T))  /* MR 970810   */
    {                           /* caller rings twice in delta_t time   */
      status_line (MSG_TXT (M_ACCEPT2NDCALL),  /* TJW 960505 added addr */
                   Full_Addr_Str (addr),
                   (long) fpoll->Delta_T,
                   fpoll->and_op ? "AND" : "OR");
      return 1;                 /* accept call */
    }
  }

  size = totalsize (addr, fpoll->AttachType);  /* get size of all for a user */

  if (size <= fpoll->Size * 1024L)  /* netsize <= Limit? */
  {
    status_line (MSG_TXT (M_REJECTLOWVOLUMECALL),  /* TJW 960505 added addr */
                 Full_Addr_Str (addr),
                 size / 1024, fpoll->Size,
                 fpoll->and_op ? "AND" : "OR");
    return 0;                   /* reject call */
  }

  status_line (MSG_TXT (M_ACCEPTHIGHVOLUMECALL),  /* TJW 960505 added addr */
               Full_Addr_Str (addr),
               size / 1024, fpoll->Size,
               fpoll->and_op ? "AND" : "OR");
  return 1;                     /* accept call */
}


void
change_call_to_normal (ADDRP addr)
{
  char iname[PATHLEN], oname[PATHLEN], tname[PATHLEN];
  char *HoldName, *result, *temp_buffer;
  struct FILEINFO fileinfo;
  FILE *ihandle;
  FILE *ohandle;
  int check;
  int written = 1;
  int buff_size;
  long loc_moh;

  status_line (MSG_TXT (M_CALL_REJECT));

  // JH, 980817: Ignore reject if theres more than x byte of mail
  loc_moh = totalsize (addr, 0);
  if ((rejectignoresize > -1) && (rejectignoresize < loc_moh))
    status_line (" Reject ignored, LocMOH is %ld", loc_moh);
  else
  {
    /* first append/rename ?LO */
    HoldName = HoldAreaNameMunge (addr);
    sprintf (tname, "%s%s.?lo", HoldName, Hex_Addr_Str (addr));
    sprintf (oname, "%s%s.%clo", HoldName, Hex_Addr_Str (addr),
             (char) chg_mail);

    if (!dfind (&fileinfo, tname, 0))
    {
      do
      {
        buff_size = (int) fileinfo.size;
        strcpy (iname, fileinfo.name);
        result = strchr (iname, '.');
        result++;

        if (*result != 'f')
        {
          if (addr->Point != 0)
          {
            sprintf (iname, "%s%04x%04x.pnt" DIR_SEPS "%s",
                     HoldName, addr->Net, addr->Node, fileinfo.name);
          }
          else
            sprintf (iname, "%s%s", HoldName, fileinfo.name);

          if (rename (iname, oname) != 0)
          {
            temp_buffer = (char *) malloc (buff_size + 1);
            ihandle = fopen (iname, read_binary);
            ohandle = fopen (oname, append_binary);

            if (temp_buffer)
            {
              do
              {
                check = fread (temp_buffer, sizeof (char), buff_size, ihandle);

                if (check)
                  written = fwrite (temp_buffer, sizeof (char), check, ohandle);
              }
              while (check > 0);
            }

            fclose (ihandle);
            fclose (ohandle);

            if (written > 0)
              unlink (iname);

            if (temp_buffer)
              free (temp_buffer);
          }
        }
      }
      while (!dfind (&fileinfo, NULL, 1));

      (dfind (&fileinfo, NULL, 2));
    }

    /* now rename ?UT */
    if ((char) chg_mail == 'f')
      chg_mail = (int) 'o';

    sprintf (tname, "%s%s.?ut", HoldName, Hex_Addr_Str (addr));
    sprintf (oname, "%s%s.%cut", HoldName, Hex_Addr_Str (addr),
             (char) chg_mail);

    if ((char) chg_mail == 'o')
      chg_mail = (int) 'f';

    if (!dfind (&fileinfo, tname, 0))
    {
      do
      {
        buff_size = (int) fileinfo.size;
        strcpy (iname, fileinfo.name);
        result = strchr (iname, '.');
        result++;

        if (*result != 'o')
        {
          if (addr->Point != 0)
          {
            sprintf (iname, "%s%04x%04x.pnt" DIR_SEPS "%s",
                     HoldName, addr->Net, addr->Node, fileinfo.name);
          }
          else
            sprintf (iname, "%s%s", HoldName, fileinfo.name);

          if ((rename (iname, oname)) != 0)
          {
            if (buff_size > 60)
              append_pkt (NULL, buff_size, iname, oname);
          }                     /* do append if rename failed */
        }
      }
      while (!dfind (&fileinfo, NULL, 1));

      (dfind (&fileinfo, NULL, 2));
    }                           /* end of ?UT handling */
  }

  xmit_reset (0, 1);
  return;
}


long
totalsize (ADDRP addr, unsigned short AttachType)  /* get size of all for a user */
{
  MAIL mail;
  long size = 0L;
  int oldloglevel = loglevel;   /* beware of perror() in file_dos.c */
  NODESTRUC *np;

  loglevel = 0;                 /* r.hoerner */

  if (!AttachType || AttachType & AT_NETMAIL)
  {
    size += filesize (addr, "cut");
    size += filesize (addr, "hut");
    size += filesize (addr, "dut");
    size += filesize (addr, "out");
    size += filesize (addr, "req");
  }

  mail.mail_addr = *addr;       /* r.hoerner 1/4/97 */
  mail.numfiles = 0;

  size += netsize (&mail, "clo", AttachType);  /* see cache.c */
  size += netsize (&mail, "hlo", AttachType);
  size += netsize (&mail, "dlo", AttachType);
  size += netsize (&mail, "flo", AttachType);

  if ((np = QueryNodeStruct (addr, 0)) != NULL)  /* filesize of NodeExtraDir */
    if (np->ExtraDir)
      size += dirsize (np->ExtraDir);

  loglevel = oldloglevel;

  return size;
}


static long
filesize (ADDRP p, char *ext)
{
  struct stat stbuf;
  char net_path[PATHLEN];

  sprintf (net_path, "%s%s.%s", HoldAreaNameMunge (p), Hex_Addr_Str (p), ext);

  if (unix_stat (net_path, &stbuf))  /* file exist? */
    return (0L);
  else
    return (stbuf.st_size);
}


static long
dirsize (char *ExtraDir)
{
  char s[PATHLEN];
  struct FILEINFO dta;
  int i;
  long size = 0;

  strcpy (s, ExtraDir);
  strcat (s, MATCHALL);
  i = dfind (&dta, s, 0);
  while (!i)
  {
    if (!(dta.attr & FA_SUBDIR))
      if (dta.name[0] != '.')
        size += dta.size;
    i = dfind (&dta, s, 1);
  };

  dfind (&dta, s, 2);

  return (size);
}

/* $Id: freepoll.c,v 1.13 1999/09/30 17:50:09 ceh Exp $ */
