
/******************************
 *       screen classes       *
 *                            *
 *     cellstring classes     *
 *         headerfile         *
 *                            *
 * written by Michael Reinsch *
 *                            *
 *                      v0.20 *
 ******************************/

#include <string.h>
#include "cellstr.hpp"

// class implementation: CellString

CellString::CellString(const UINT16 len) /*fold00*/
{
  cellLen    = len * 2;
  cellCols   = len;
  cellRows   = 1;
  cellZPos   = 0;
  cell       = new char[cellLen];
  partPosRow = 0;
  partPosCol = 0;
  partRows   = cellRows;
  partCols   = cellCols;
  SetVCursor(0,0);
  Changed(false);
}

CellString::CellString(const UINT16 rows, const UINT16 cols) /*fold00*/
{
  cellLen    = rows * cols * 2;
  cellRows   = rows;
  cellCols   = cols;
  cellZPos   = 0;
  cell       = new char[cellLen];
  partPosRow = 0;
  partPosCol = 0;
  partRows   = cellRows;
  partCols   = cellCols;
  SetVCursor(0,0);
  Changed(false);
}

CellString::~CellString(void) /*FOLD00*/
{
  delete cell;
}

BOOLEAN CellString::Changed(const INT16 chng) /*fold00*/
{
  BOOLEAN tmp = changed;

  if ( chng == 0 )
    changed = false;
  else if ( chng > 0 )
    changed = true;

  return tmp;
}

void CellString::ChgSize(const UINT16 rows, const UINT16 cols) /*fold00*/
{
  delete cell;
  cellLen    = rows * cols * 2;
  cellRows   = rows;
  cellCols   = cols;
  cell       = new char[cellLen];
  partPosRow = 0;
  partPosCol = 0;
  partRows   = cellRows;
  partCols   = cellCols;
  SetVCursor(0,0);
  Changed(true);
}

BOOLEAN CellString::SetPart(const UINT16 prow, const UINT16 pcol, const UINT16 rows, const UINT16 cols) /*fold00*/
{
  if ( ( prow >= partRows ) || ( pcol >= partCols ) )
    return false;

  if ( (prow+rows) >= partRows )
    partRows = (UINT16) (partRows-prow);
  else
    partRows = rows;

  if ( (pcol+cols) >= partCols )
    partCols = (UINT16) (partCols-pcol);
  else
    partCols = cols;

  partPosRow += prow;
  partPosCol += pcol;

  return true;
}

void CellString::ResetPart(const UINT16 prow, const UINT16 pcol, const UINT16 rows, const UINT16 cols) /*fold00*/
{
  if (partPosRow > prow)
    partPosRow -= prow;
  else
    partPosRow = 0;
  if (partPosCol > pcol)
    partPosCol -= pcol;
  else
    partPosCol = 0;
  partRows    = rows;
  partCols    = cols;
}

void CellString::Fill(const CellString &tmp) /*fold00*/
{
  UINT16 r, c;
  UINT32 pos;
  UINT32 tmppos = tmp.calcpos();

  for ( r = 0; r < Rows(); r++ )
    for ( c = 0, pos = calcpos ( r, c ); c < Cols(); c++, pos++ )
    {
      SetChr(pos, tmp.GetChr(tmppos));
      SetAtt(pos, tmp.GetAtt(tmppos));
    }

  Changed(1);
}

void CellString::Fill(const INT16 chrctr, const INT16 attrib, const UINT16 len) /*fold00*/
{
  UINT16 r, c;
  UINT32 pos;
  UINT16 maxLen = 0;
  UINT16 l      = 0;
  UINT16 vcr    = 0;
  UINT16 vcc    = 0;

  if ( len == 0 )                   // len==0 -> fill hole window
    maxLen = (INT16) (Rows() * Cols());
  else                              // len!=0 -> fill cur. pos. len chrs
  {
    vcr = VCursorRow();
    vcc = VCursorCol();
    maxLen = len;
  }

  for ( r = vcr; r < Rows(); r++ )
    for ( c = vcc, pos = calcpos ( r, c ); c < Cols(); c++, vcc=0, pos++ )
    {
      if ( ++l > maxLen )
      {
        c = Cols();
        r = Rows();
      }
      else
      {
        if ( chrctr > -1 )
          SetChr(pos, (char) chrctr);
        if ( attrib > -1 )
          SetAtt(pos, (char) attrib);
      }
    }

  Changed(1);
}

void CellString::MoveFill(const UINT16 row, const UINT16 col, const INT16 chrctr, const INT16 attrib, const UINT16 len) /*fold00*/
{
  SetVCursor(row, col);
  Fill(chrctr, attrib, len);
}

void CellString::Put(const CellString &tmp) /*FOLD00*/
{
  UINT16 wr, wc;  // window row and col
  UINT16 sr, sc;  // screen row and col
  UINT32 spos, wpos;

  wr = 0;
  sr = VCursorRow();

  while ( ( wr < tmp.Rows() ) && ( sr < Rows() ) )
  {
    wc   = 0;
    sc   = VCursorCol();
    wpos = tmp.calcpos(wr, wc);
    spos = calcpos(sr, sc);

    while ( ( wc < tmp.Cols() ) && ( sc < Cols() ) )
    {
      SetChr(spos, tmp.GetChr(wpos));
      SetAtt(spos, tmp.GetAtt(wpos));
      ++wc;
      ++sc;
      ++wpos;
      ++spos;
    }

    ++wr;
    ++sr;
  }

  Changed(1);
}

void CellString::Put(const char tmp, const INT16 attrib) /*fold00*/
{
  UINT32 pos;

  if ( VCursorRow() < Rows() )
    if ( VCursorCol() < Cols() )
    {
      pos = calcpos(VCursorRow(), VCursorCol());
      SetChr(pos, tmp);
      if ( attrib > -1 )
        SetAtt(pos, (char) attrib);

      Changed ( 1 );
    }

  SetVCursor(VCursorRow(), (UINT16) (VCursorCol()+1));
}

void CellString::Put(const char *tmp, const INT16 attrib, const UINT16 len) /*fold00*/
{
  UINT16 i;
  UINT16 maxLen;
  UINT32 pos;

  if ( VCursorRow() < Rows() )
    if ( VCursorCol() < Cols() )
    {
      maxLen = (UINT16) strlen(tmp);

      if ( len && (len < maxLen) )
        maxLen = len;

      pos = calcpos(VCursorRow(), VCursorCol());

      if ( (VCursorCol()+maxLen) >= Cols() )
        maxLen = (UINT16) (Cols() - VCursorCol());

      for ( i = 0; i < maxLen; i++, pos++ )
      {
        SetChr(pos, tmp[i]);
        if ( attrib > -1 )
          SetAtt(pos, (char) attrib);
      }

      Changed(1);
    }

  SetVCursor(VCursorRow(), (UINT16) ( VCursorCol()+maxLen ));
}

void CellString::ScrollUp(const UINT16 lines, const UINT16 startrow, const UINT16 startcol, const UINT16 rows, const UINT16 cols) /*fold00*/
{
  UINT16 row;
  UINT16 col;
  UINT32 spos;
  UINT32 dpos;

  for ( row = (UINT16) (startrow+lines); row < (UINT16) (startrow+rows); row++ )
    for ( col = startcol, spos = calcpos(row, col), dpos = calcpos((UINT16) (row-lines), col); col < (UINT16) (startcol+cols); col++, spos++, dpos++ )
    {
      SetChr(dpos, GetChr(spos));
      SetAtt(dpos, GetAtt(spos));
    }

  Changed(1);
}

void CellString::ScrollDown(const UINT16 lines, const UINT16 startrow, const UINT16 startcol, const UINT16 rows, const UINT16 cols) /*fold00*/
{
  INT16  row;
  UINT16 col;
  UINT32 spos;
  UINT32 dpos;

  for ( row = (INT16) (startrow+rows-lines); row >= startrow; row-- )
  {
    for ( col = startcol, spos = calcpos(row, col), dpos = calcpos((UINT16) (row+lines), col); col < (UINT16) (startcol+cols); col++, spos++, dpos++ )
    {
      SetChr(dpos, GetChr(spos));
      SetAtt(dpos, GetAtt(spos));
    }
  }

  Changed ( 1 );
}
 /*FOLD00*/

// class implementation: ZBufCellString

ZBufCellString::ZBufCellString(const UINT16 len) /*fold00*/
               :CellString (len)
{
  zbuf_len = len;
  zbuffer  = new UINT32[zbuf_len];
  dbuf_len = 2;
  dbuffer  = new UINT16[dbuf_len];
  dummy    = 0;
  ClearZBuffer();
  ClearChanges();
}

ZBufCellString::ZBufCellString(const UINT16 rows, const UINT16 cols) /*fold00*/
               :CellString(rows, cols)
{
  zbuf_len = rows * cols;
  zbuffer  = new UINT32[zbuf_len];
  dbuf_len = rows * 2;
  dbuffer  = new UINT16[dbuf_len];
  dummy    = 0;
  ClearZBuffer();
  ClearChanges();
}

ZBufCellString::~ZBufCellString(void) /*fold00*/
{
  delete zbuffer;
  delete dbuffer;
}

void ZBufCellString::ClearZBuffer(const UINT32 zpos) /*fold00*/
{
  UINT16 r, c;
  UINT32 pos;

  for ( r = 0; r < Rows(); r++ )
    for ( c = 0, pos = calcpos(r, c); c < Cols(); c++, pos++ )
    {
      if ( GetZPos(pos) == zpos )
        SetZPos(pos, 0);
    }
}

void ZBufCellString::ClearZBuffer(void) /*fold00*/
{
  UINT16 r, c;
  UINT32 pos;

  for ( r = 0; r < Rows(); r++ )
    for ( c = 0, pos = calcpos(r, c); c < Cols(); c++, pos++ )
      SetZPos(pos, 0);
}

void ZBufCellString::Changed(const UINT16 row, const UINT16 col) /*fold00*/
{
  UINT16 prow = (UINT16) (row + PartPosRow());
  UINT16 pcol = (UINT16) (col + PartPosCol());
  UINT32 pos  = prow*2;

  if ( dbuffer[pos] > pcol )
  {
    if ( dbuffer[pos+1] == 0 )
    {
      dbuffer[pos]   = pcol;
      dbuffer[pos+1] = 1;
    }
    else
    {
      dbuffer[pos+1] = (UINT16) (dbuffer[pos+1] + (dbuffer[pos] - pcol + 1));
      dbuffer[pos]   = pcol;
    }
  }
  else if (dbuffer[pos] + dbuffer[pos+1] < pcol+1)
    dbuffer[pos+1] = (UINT16) (pcol+1-dbuffer[pos]);
}

void ZBufCellString::ClearChanges(void) /*fold00*/
{
  UINT32 i;

  for ( i=0; i < dbuf_len; i+=2 )
  {
    dbuffer[i]   = Cols();
    dbuffer[i+1] = 0;
  }
}

void ZBufCellString::ChgSize(const UINT16 rows, const UINT16 cols) /*fold00*/
{
  CellString::ChgSize(rows, cols);
  delete zbuffer;
  delete dbuffer;
  zbuf_len = rows * cols;
  zbuffer  = new UINT32[zbuf_len];
  dbuf_len = rows * 2;
  dbuffer  = new UINT16[dbuf_len];
  dummy    = 0;
  ClearZBuffer ();
  ClearChanges ();
}

void ZBufCellString::Put(const CellString &tmp) /*fold00*/
{
  UINT16  wr, wc;  // window row and col
  UINT16  sr, sc;  // screen row and col
  UINT32  spos, wpos;

  wr = 0;
  sr = VCursorRow();

  while ( (wr < tmp.Rows()) && (sr < Rows()) )
  {
    wc   = 0;
    sc   = VCursorCol();
    wpos = tmp.calcpos(wr, wc);
    spos = calcpos(sr, sc);

    while ( (wc < tmp.Cols()) && (sc < Cols()) )
    {
      if ( GetZPos(spos) <= tmp.GetZPos(wpos) )
      {
        SetZPos(spos, tmp.GetZPos(wpos));

        if ( (GetChr(spos) != tmp.GetChr(wpos)) ||
             (GetAtt(spos) != tmp.GetAtt(wpos)) )
        {
          SetChr(spos, tmp.GetChr(wpos));
          SetAtt(spos, tmp.GetAtt(wpos));
          Changed(sr,sc);
        }
      }

      ++wc;
      ++sc;
      ++wpos;
      ++spos;
    }

    ++wr;
    ++sr;
  }
}
 /*FOLD00*/

