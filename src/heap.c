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
 * Filename    : $Source: E:/cvs/btxe/src/heap.c,v $
 * Revision    : $Revision: 1.3 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 01:15:56 $
 * State       : $State: Exp $
 *
 * Description : heap check
 *
 *---------------------------------------------------------------------------*/

#ifdef HEAPCHECK

#include "includes.h"

#if ((defined __WATCOMC__) | (defined _MSC_VER))  //NS030898 also for MSVC

static long heapmax = 0;

void
heapchk (int x)
{
  struct _heapinfo hi;
  long usedblocks = 0;
  long freeblocks = 0;
  long usedbytes = 0;
  long freebytes = 0;
  long heapnow;
  int chk;

  x = x;
  hi._pentry = NULL;
  while (_heapwalk (&hi) == _HEAPOK)
  {
    switch (hi._useflag)
    {
    case _USEDENTRY:
      usedbytes += hi._size;
      usedblocks++;
      break;
    case _FREEENTRY:
      freebytes += hi._size;
      freeblocks++;
      break;
    }
  }

  heapnow = (usedbytes + freebytes) + (usedblocks + freeblocks) * sizeof (hi);
  if (heapnow > heapmax)
  {
    status_line (":Heap: Used:%ld/%ld, Free:%ld/%ld, Size:%ld",
                 usedbytes, usedblocks,
                 freebytes, freeblocks,
                 heapnow);
    heapmax = heapnow;
  }

  chk = _heapchk ();
  if (chk != _HEAPOK)
    status_line ("!Heap: _heapchk() returns %d (not _HEAPOK)");
}

#elif defined __IBMC__

static long used_heap = 0;
int
heap_callback (void *obj, size_t size, int flag, int status, char *file, size_t line)
{
  static char junk[1024];
  static char object[40];
  int sze;

  if (status != _HEAPOK)
  {
    switch (status)
    {
    case _HEAPBADBEGIN:
      sprintf (junk, "!HEAP bad begin of heap\r\n");
      break;
    case _HEAPBADNODE:
      sprintf (junk, "!HEAP bad node\r\n");
      break;
    case _HEAPEMPTY:
      sprintf (junk, "!HEAP nothing in heap\r\n");
      break;
    default:
      sprintf (junk, "!HEAP unexpected status\r\n");
      break;
    }
    fprintf (stderr, junk);
    return (0);
  }

  if (flag == _FREEENTRY)
  {
    if (used_heap >= size)
      used_heap -= size;
    sprintf (junk, "!HEAP [%06u] free space of %u byte\r\n", used_heap, size);
    fprintf (stderr, junk);
    return (0);
  }

  if (file == NULL)
  {
    return (0);
  }

  used_heap += size;
  if (obj != NULL)
  {
    sze = size;
    if (sze > 39)
      sze = 39;

    memmove (&object, obj, sze);
    object[sze] = 0;
    if (isalpha (object[0]))
      sprintf (junk, "!HEAP [%06u] Obj=%s size=%u file=%s,line=%u", used_heap, object, size, file, line);
    else
      sprintf (junk, "!HEAP [%06u] Obj=?? size=%u file=%s,line=%u", used_heap, size, file, line);
  }
  else
    sprintf (junk, "!HEAP [%06u] Obj=?? size=%u file=%s,line=%u", used_heap, size, file, line);

  strcat (junk, "\r\n");
  fprintf (stderr, junk);

  return (0);
}

void
heapchk (int nr)
{
  used_heap = 0;
  switch (nr)
  {
  case 0:
    fprintf (stderr, "\r\n!HEAP: init\r\n");
    break;
  case 1:
    fprintf (stderr, "\r\n!HEAP: b_init\r\n");
    break;
  case 2:
    fprintf (stderr, "\r\n!HEAP: b_session_cleanup\r\n");
    break;
  }
  if (_heapchk () == _HEAPOK)
    _heap_walk (&heap_callback);
}

#elif defined __BORLANDC__
// 98/02/25 HJK

static long heapmax = 0;

void
heapchk (int x)
{
  struct heapinfo hi;
  long usedblocks = 0;
  long freeblocks = 0;
  long usedbytes = 0;
  long freebytes = 0;
  long heapnow;
  int chk;

  x = x;
  hi.ptr = NULL;
  while (heapwalk (&hi) == _HEAPOK)
  {
    if (hi.in_use == 0)
    {
      freebytes += hi.size;
      freeblocks++;
    }
    else
    {
      usedbytes += hi.size;
      usedblocks++;
    }
  }

  heapnow = (usedbytes + freebytes) + (usedblocks + freeblocks) * sizeof (hi);
  if (heapnow > heapmax)
  {
    status_line (">Heap: Used:%ld/%ld, Free:%ld/%ld, Size:%ld",
                 usedbytes, usedblocks, freebytes, freeblocks, heapnow);
    heapmax = heapnow;
  }

  chk = heapcheck ();
  if (chk != _HEAPOK)
    status_line ("!Heap: heapcheck() returns %d (not _HEAPOK)", chk);
}

#else /* all other compilers */

void
heapchk (int x)
{
  x = x;
}

#endif /* compilers           */
#endif /* ifef HEAPCHECK      */

/* $Id: heap.c,v 1.3 1999/02/27 01:15:56 mr Exp $ */
