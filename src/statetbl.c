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
 * Filename    : $Source: E:/cvs/btxe/src/statetbl.c,v $
 * Revision    : $Revision: 1.2 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:16:21 $
 * State       : $State: Exp $
 * Orig. Author: Bob Hartman
 *
 * Description : BinkleyTerm General State Machine Driver.
 *
 *---------------------------------------------------------------------------*/

/* Include this file before any other includes or defines! */

#include "includes.h"

#ifndef DEBUG
#define DEBUG
#endif

int
state_machine (STATEP machine, void *passed_struct, int start_state)
{
  int cur_state;

  ((STATEBASEP) passed_struct)->control = start_state;

#ifdef DEBUG
  status_line ("&Entering State Machine with state %d", start_state);
#endif

  cur_state = (*(machine[0].state_func)) (passed_struct);

#ifdef DEBUG
  status_line ("&State after init = %d", cur_state);
#endif

  while (cur_state > 0)
  {
#ifdef DEBUG
    status_line ("&State: %s", machine[cur_state].state_name);
#endif
    cur_state = (*(machine[cur_state].state_func)) (passed_struct);
  }

#ifdef DEBUG
  status_line ("&Out of state machine with state = %d", cur_state);
#endif

  ((STATEBASEP) passed_struct)->control = cur_state;
  cur_state = (*(machine[1].state_func)) (passed_struct);

#ifdef DEBUG
  status_line ("&Exiting after state end with state = %d", cur_state);
#endif

  return (cur_state);
}

/* $Id: statetbl.c,v 1.2 1999/02/27 01:16:21 mr Exp $ */
