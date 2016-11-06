
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

#ifndef _S_CELLSTR_HPP_INCLUDED
#define _S_CELLSTR_HPP_INCLUDED 0020
#include "typesize.h"

// class definition: CellString

class CellString
{
  private:
    char    *cell;          // the cell string
    UINT32   cellLen;       // length of cell string
    UINT16   cellRows;      // number of rows (height)
    UINT16   cellCols;      // number of cols (width)
    UINT32   cellZPos;      // z position of cell
    UINT16   vCursorRow;    // virtual cursor row (position)
    UINT16   vCursorCol;    // virtual cursor col (position)
    UINT16   partRows;      // number of rows of part
    UINT16   partCols;      // number of cols of part
    UINT16   partPosRow;    // pos 0 (row) of part in cell
    UINT16   partPosCol;    // pos 0 (col) of part in cell
    BOOLEAN  changed;       // changed flag
    char     dummy;         // char dummy

  public:
    CellString(const UINT16 len = 0);
    CellString(const UINT16 rows, const UINT16 cols);
    virtual ~CellString(void);

    BOOLEAN Changed(const INT16 chng = -1);
    void ChgSize(const UINT16 rows, const UINT16 cols);
    BOOLEAN SetPart(const UINT16 prow, const UINT16 pcol, const UINT16 rows, const UINT16 cols);
    void ResetPart(const UINT16 prow, const UINT16 pcol, const UINT16 rows, const UINT16 cols);
    void Fill(const CellString &tmp);
    void Fill(const INT16 chrctr, const INT16 attrib = -1, const UINT16 len = 0);
    void MoveFill(const UINT16 row, const UINT16 col, const INT16 chrctr, const INT16 attrib = -1, const UINT16 len = 0);
    virtual void Put(const CellString &tmp);
    void Put(const char tmp, const INT16 attrib = -1);
    void Put(const char *tmp, const INT16 attrib = -1, const UINT16 len = 0);
    void ScrollUp(const UINT16 lines, const UINT16 startrow, const UINT16 startcol, const UINT16 rows, const UINT16 cols);
    void ScrollDown(const UINT16 lines, const UINT16 startrow, const UINT16 startcol, const UINT16 rows, const UINT16 cols);

    inline UINT16 Rows(void) const
    {
      return partRows;
    }

    inline UINT16 Cols(void) const
    {
      return partCols;
    }

    inline UINT16 PartPosRow(void) const
    {
      return partPosRow;
    }

    inline UINT16 PartPosCol(void) const
    {
      return partPosRow;
    }

    inline UINT16 VCursorRow(void) const
    {
      return vCursorRow;
    }

    inline UINT16 VCursorCol(void) const
    {
      return vCursorCol;
    }

    inline void SetVCursor(const UINT16 row = 0, const UINT16 col = 0)
    {
      vCursorRow = row;
      vCursorCol = col;
    }

    inline UINT32 GetZPos(const UINT32 pos = 0) const
    {
      UINT32 dummy = pos;  // make compiler happy
      return cellZPos;
    }

    inline BOOLEAN SetZPos(const UINT32 pos, const UINT32 z)
    {
      UINT32 dummy = pos;  // make compiler happy
      cellZPos = z;
      return true;
    }

    inline UINT32 calcpos(const UINT16 row, const UINT16 col) const
    {
      return (row+partPosRow) * cellCols + (col+partPosCol);
    }

    inline UINT32 calcpos(void) const
    {
      return calcpos(VCursorRow(), VCursorCol());
    }

    inline char GetChr(const UINT32 pos) const
    {
      UINT32 tmp = pos * 2 + 0;

      if ( tmp < cellLen )
        return cell[tmp];
      else
        return dummy;
    }

    inline BOOLEAN SetChr(const UINT32 pos, const char c)
    {
      UINT32 tmp = pos * 2 + 0;

      if ( tmp < cellLen )
      {
        cell[tmp] = c;
        return true;
      }
      else
        return false;
    }

    inline char GetAtt(const UINT32 pos) const
    {
      UINT32 tmp = pos * 2 + 1;

      if ( tmp < cellLen )
        return cell[tmp];
      else
        return dummy;
    }

    inline BOOLEAN SetAtt(const UINT32 pos, const char a)
    {
      UINT32 tmp = pos * 2 + 1;

      if ( tmp < cellLen )
      {
        cell[tmp] = a;
        return true;
      }
      else
        return false;
    }

    inline char *GetCell(void) const
    {
      return cell;
    }

    inline char *GetCell(const UINT16 row, const UINT16 col) const
    {
      return &cell[calcpos(row,col)*2];
    }

    inline void MovePut(const UINT16 row, const UINT16 col, const CellString &tmp)
    {
      SetVCursor(row, col);
      Put(tmp);
    }

    inline void MovePut(const UINT16 row, const UINT16 col, const char tmp, const INT16 attrib = -1)
    {
      SetVCursor(row, col);
      Put(tmp, attrib);
    }

    inline void MovePut(const UINT16 row, const UINT16 col, const char *tmp, const INT16 attrib = -1, const UINT16 len = 0)
    {
      SetVCursor(row, col);
      Put(tmp, attrib, len);
    }
};


// class definition: ZBufCellString

class ZBufCellString : public CellString
{
  private:   // variables
    UINT32 *zbuffer;       // z buffer
    UINT32  zbuf_len;      // length of z buffer
    UINT16 *dbuffer;       // delta buffer - for each row: start,len of changes
    UINT32  dbuf_len;      // length of delta buffer
    UINT32  dummy;         // dummy

  public:    // methods

    ZBufCellString(const UINT16 len = 0);
    ZBufCellString(const UINT16 rows, const UINT16 cols);
    ~ZBufCellString(void);

    void ClearZBuffer(const UINT32 zpos);
    void ClearZBuffer(void);
    void Changed(const UINT16 row, const UINT16 col);
    void ClearChanges(void);
    void ChgSize(const UINT16 rows, const UINT16 cols);
    void Put(const CellString &tmp);

    inline UINT32 GetZPos(const UINT32 pos) const
    {
      if ( pos < zbuf_len )
        return zbuffer[pos];
      else
        return dummy;
    }

    inline BOOLEAN SetZPos(const UINT32 pos, const UINT32 z)
    {
      if ( pos < zbuf_len )
      {
        zbuffer[pos] = z;
        return true;
      }
      else
        return false;
    }

    inline UINT16 ChangesStart(const UINT16 row) const
    {
      return dbuffer[row * 2 + 0];
    }

    inline UINT16 ChangesLen(const UINT16 row) const
    {
      return dbuffer[row * 2 + 1];
    }
};

#endif

