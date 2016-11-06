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
 * Filename    : $Source: E:/cvs/btxe/src/fcfgread.c,v $
 * Revision    : $Revision: 1.10 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/09/12 21:53:35 $
 * State       : $State: Exp $
 * Orig. Author: C. Eike Hofmeister, 2:2426/2280
 *
 * Description : read fidoconfig-data (see http://fidosoft.sar-gmbh.com)
 *
 *---------------------------------------------------------------------------*/

#define bit     fcfg_bit
#define byte    fcfg_byte
#define word    fcfg_word
#define dword   fcfg_dword

#ifndef DOS16
#include "fidoconfig.h"
#else
#include "fidoconf.h"
#endif

#undef bit
#undef byte
#undef word
#undef dword
#undef tell
#undef stricmp

#include "includes.h"

int error = 0;

char *
gs (char *s)                    /* generate string */
{
  char *p;

  if (s != NULL)
  {
    if ((p = malloc (strlen (s) + 1)) == NULL)
      error = 1;
    else
      strcpy (p, s);
  }
  else
  {
    error = 1;
    p = NULL;
  }

  return (p);
}

s_fidoconfig *cfg;

int
read_fidoconfig ()
{
  AKA *qnode = NULL, *pnode;
  char *p, dummy[512];
  int i;

  cfg = readConfig ();
  if (cfg == NULL)
    return (1);

  printf ("Reading fidoconfig (version %u.%u)...", cfg->cfgVersionMajor,
          cfg->cfgVersionMinor);

  def_sysop = sysop = cfg->sysop;
  def_system_name = system_name = cfg->name;
  def_location = location = cfg->location;

  for (i = 0; i < cfg->addrCount; i++)
  {
    if (alias != NULL)          /* if aka structure exists */
    {
      if ((alias->next == NULL) /* and there is only 1 entry in list */
          && ((word) alias->Zone == 1)
          && ((word) alias->Net == 0xffff)
          && ((word) alias->Node == 0xffff))  /* and adress is 1:-1/-1 (default) */
      {
        alias->Zone = cfg->addr[i].zone;
        alias->Net = cfg->addr[i].net;
        alias->Node = cfg->addr[i].node;
        alias->Point = cfg->addr[i].point;
        alias->Domain = NULL;
        continue;
      }
    }

    if ((qnode = (AKA *) calloc (1, sizeof (AKA))) == NULL)
      return (1);

    if (alias == NULL)
      alias = qnode;
    else
    {
      for (pnode = alias; pnode->next != NULL; pnode = pnode->next) ;
      pnode->next = qnode;
    }

    qnode->Zone = cfg->addr[i].zone;
    qnode->Net = cfg->addr[i].net;
    qnode->Node = cfg->addr[i].node;
    qnode->Point = cfg->addr[i].point;
    qnode->Domain = NULL;
    qnode->next = NULL;
  }                             /* endfor cfg->addrCount */

  DEFAULT.sc_Inbound = cfg->inbound;
  PROT.sc_Inbound = cfg->protInbound;
  KNOWN.sc_Inbound = cfg->listInbound;
  if (NULL != cfg->localInbound)
    local_inbound = cfg->localInbound;
  else
    local_inbound = cfg->protInbound;
  hold_area = cfg->outbound;
  net_info = cfg->nodelistDir;
  sprintf (dummy, "%s" PRDCT_PRFXlower ".l%02i", cfg->logFileDir, TaskNumber);
  log_name = gs (dummy);

  switch (cfg->netMailArea.msgbType)
  {
  case MSGTYPE_SQUISH:
    dummy[0] = '$';
    dummy[1] = '\0';
    strcat (dummy, cfg->netMailArea.fileName);
    netmail = gs (dummy);
    break;
  case MSGTYPE_SDM:
    netmail = cfg->netMailArea.fileName;
    break;
  }

  if (PROT.sc_Inbound == NULL)
    PROT.sc_Inbound = DEFAULT.sc_Inbound;
  if (KNOWN.sc_Inbound == NULL)
    KNOWN.sc_Inbound = DEFAULT.sc_Inbound;

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
    }

    if (p != domain_area)
      ++p;

    *p = '\0';
  }

  return (error);
}

void
setpathsfromfidoconfig ()
{
  config_name = (char *) getConfigFileNameForProgram ("BINKLEY", "binkley");
  eventfile = (char *) getConfigFileNameForProgram ("BINKLEY", "binkley.evt");
  printf ("\nUsing config <%s> and EventFile <%s>\n", config_name, eventfile);
}

char *
PasswordFromFidoconfig (ADDRP addr)
{
  int i;

  for (i = 0; i < cfg->linkCount; i++)
  {
    if ((cfg->links[i].hisAka.zone == addr->Zone) &&
        (cfg->links[i].hisAka.net == addr->Net) &&
        (cfg->links[i].hisAka.node == addr->Node) &&
        (cfg->links[i].hisAka.point == addr->Point))
      return (cfg->links[i].sessionPwd);
  }

  return (NULL);
}

/* $Id: fcfgread.c,v 1.10 1999/09/12 21:53:35 sms Exp $ */
