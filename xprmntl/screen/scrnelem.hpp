
/******************************
 *       screen classes       *
 *                            *
 *   screen element classes   *
 *         headerfile         *
 *                            *
 * written by Michael Reinsch *
 *                            *
 *                      v0.20 *
 ******************************/

#ifndef _S_SCRNELEM_HPP_INCLUDED
#define _S_SCRNELEM_HPP_INCLUDED 0020
#include <string.h>
#include <limits.h>
#include "typesize.h"
#include "pntdlist.hpp"
#include "cellstr.hpp"
#include "colour.hpp"
#include "frame.hpp"
#ifdef __OS2__
#  define  INCL_DOSPROCESS
#  define  INCL_VIO
#  define  INCL_NOPMAPI
#  include <os2.h>
#  define  OS_SUPPORTED
#endif
#ifdef __DOS__
#  include "scrndos.h"
#  define  OS_SUPPORTED
#endif
#if _WIN32
#  include <windows.h>
#  include "scrnw32.h"
#  define  OS_SUPPORTED
#endif


// class definition: BSEMsg - Basic Screen Element Messages

enum  BSEMsgID /*fold00*/
{
  // Screen Element internal Messages
  // those are processed internally, i.e. you don't need to process them
  PosChanged,         // make sure contents gets written to screen
  SizeChanged,        // resize contents, tell childrens
  ParentSizeChanged,  // parent changed size, resize if relative
  VisibleChanged,     // make sure contents gets written to screen
  ChildRemoved,       // remove cild element
  ChildAdded,         // show child element
  ParentRemoved,      // make sure contents gets written to screen
  ParentAdded,        // make sure contents gets written to screen
  ElementChanged,     // make sure contents gets written to screen
  RefreshRequired,    // complete refresh required
  DoRefresh,          // answer to RefreshRequired from Screen class
  MaxZPosReached,     // renumber Z positions
  ClearZBuffer,       // clear Z buffer

  // external Messages
  // those are NOT processed internally, you can process them
  KeyPressed,         // Key was pressed, see addData (not yet def.)
  MouseAction,        // Mouse did somthing, see addData (not yet def.)
  PrgDefMessage       // Programmer def. Message, see addData (not yet def.)
};

class BSEMsg /*fold00*/
{
  private:
    BSEMsgID  msgID;     // the Msg ID
    BOOLEAN   global;    // global msg?
    void     *addData;   // some additional data

  public:
    inline BSEMsg(const BSEMsgID msg, void *data = NULL)
    {
      msgID   = msg;
      global  = false;
      addData = data;
    }

    inline BSEMsgID GetMsg(void) const
    {
      return msgID;
    }

    inline BOOLEAN IsGlobal(void) const
    {
      return global;
    }

    inline void SetGlobal(BOOLEAN newglob)
    {
      global = newglob;
    }

    inline void *GetAddData(void) const
    {
      return addData;
    }

    inline void SetAddData(void *data)
    {
      addData = data;
    }
};
 /*FOLD00*/

// class definition: BSE - Basic Screen Element

class BSE /*fold00*/
{
  private:
    BSE           *parent;      // parent element
    PntDList<BSE>  children;    // List of embedded elements
    INT16          posRow;      // position of element (row) - might be
    INT16          posCol;      // position of element (col) - relative
    INT16          sizeRows;    // size of element (rows) - might be
    INT16          sizeCols;    // size of element (cols) - relative
    BOOLEAN        visible;     // element visible?
    BOOLEAN        busy;        // element busy? (for multithreading)
    BOOLEAN        posRel;      // pos relative?
    BOOLEAN        sizeRel;     // size relative?
    static UINT32  nextFreeZ;   // next free z position

  public:
    BSE(void);
    virtual ~BSE(void){};

  private:
    void CheckRelative(void);

  protected:
    UINT32 GetNextFreeZ(void);
    BOOLEAN SetBusy(const BOOLEAN newbusy = true);
    virtual void Send2Me(const BSEMsg &msg, const BOOLEAN frmRoot = false) = 0;
    void Send2Children(const BSEMsg &msg);
    void Send2Parent(const BSEMsg &msg);
    void Send2Root(const BSEMsg &msg);
    void Send2All(const BSEMsg &msg);

    inline INT16 GetRelPosRow(void) const
    {
      return posRow;
    }

    inline INT16 GetRelPosCol(void) const
    {
      return posCol;
    }

    inline INT16 GetRelSizeRows(void) const
    {
      return sizeRows;
    }

    inline INT16 GetRelSizeCols(void) const
    {
      return sizeCols;
    }

    inline void AddWin(BSE *element)
    {
      children.Add(element);
      Send2Me(ChildAdded);
    }

    inline void RemoveWin(BSE *element)
    {
      children.Remove(element);
      Send2Me(ChildRemoved);
    }

  public:
    void SetPos(const INT16 row, const INT16 col);
    void SetSize(const INT16 rows, const INT16 cols);
    void SetVisible(const BOOLEAN vis);
    void SetParent(BSE *par);
    UINT16 GetPosRow(void) const;
    UINT16 GetPosCol(void) const;
    UINT16 GetSizeRows(void) const;
    UINT16 GetSizeCols(void) const;
    UINT16 CalcPosRow(const INT16 row) const;
    UINT16 CalcPosCol(const INT16 col) const;
    UINT16 CalcSizeRows(const INT16 rows) const;
    UINT16 CalcSizeCols(const INT16 cols) const;

    inline BOOLEAN IsVisible(void) const
    {
      return visible;
    }

    inline BOOLEAN IsPosRelative(void) const
    {
      return posRel;
    }

    inline BOOLEAN IsSizeRelative(void) const
    {
      return sizeRel;
    }

    inline BSE *GetParent(void) const
    {
      return parent;
    }

    inline void Refresh(void)
    {
      Send2Me(RefreshRequired);
    }
};
 /*FOLD00*/

// class definition: CSE - CellString Screen Element

class CSE : public BSE /*fold00*/
{
  private:
    CellString *contents;     // holds contents of element
    Colour      dfltColour;   // default colour

  public:
    CSE(void);
    virtual ~CSE(void);

  protected:
    virtual void Send2Me(const BSEMsg &msg, const BOOLEAN frmRoot = false);
    virtual void Redraw(void) = 0;
    void ScrollUp(const UINT16 lines = 1, const UINT16 startrow = 0, const UINT16 startcol = 0, const UINT16 rows = 0, const UINT16 cols = 0);
    void ScrollDown(const UINT16 lines = 1, const UINT16 startrow = 0, const UINT16 startcol = 0, const UINT16 rows = 0, const UINT16 cols = 0 );
    void DrawHorLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char mid);
    void DrawHorLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char beg, const char mid, const char end);
    void DrawVerLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char mid);
    void DrawVerLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char beg, const char mid, const char end);
    void DrawFrame(const Frame &type);

    inline void SetColour(const Colour &tmp)
    {
      dfltColour = tmp;
    }

    inline void SetCursorPos(const INT16 row, const INT16 col)
    {
      contents->SetVCursor(CalcPosRow(row), CalcPosCol(col));
    }

    inline void SetZPos(void)
    {
      contents->SetZPos(0, GetNextFreeZ());
    }

    inline void SetChngFlag(void)
    {
      contents->Changed(1);
    }

    inline void Clear(void)
    {
      Fill(0x20, dfltColour);
    }

    inline void Fill(const char chrctr, const UINT16 len = 0)
    {
      contents->Fill(chrctr, dfltColour.Get(), len);
    }

    inline void Fill(const Colour &attrib, const UINT16 len = 0)
    {
      contents->Fill(-1, attrib.Get(), len);
    }

    inline void Fill(const char chrctr, const Colour &attrib, const UINT16 len = 0)
    {
      contents->Fill(chrctr, attrib.Get(), len);
    }

    inline void MoveFill(const UINT16 row, const UINT16 col, const char chrctr, const UINT16 len = 0)
    {
      contents->MoveFill(row, col, chrctr, dfltColour.Get(), len);
    }

    inline void MoveFill(const UINT16 row, const UINT16 col, const Colour &attrib, const UINT16 len = 0)
    {
      contents->MoveFill(row, col, -1, attrib.Get(), len);
    }

    inline void MoveFill(const UINT16 row, const UINT16 col, const char chrctr, const Colour &attrib, const UINT16 len = 0)
    {
      contents->MoveFill(row, col, chrctr, attrib.Get(), len);
    }

    inline void Put(const char tmp)
    {
      contents->Put(tmp, dfltColour.Get());
    }

    inline void Put(const char tmp, const Colour &attrib)
    {
      contents->Put(tmp, attrib.Get());
    }

    inline void Put(const char *tmp, const UINT16 len = 0)
    {
      contents->Put(tmp, dfltColour.Get(), len);
    }

    inline void Put(const char *tmp, const Colour &attrib, const UINT16 len = 0)
    {
      contents->Put(tmp, attrib.Get(), len);
    }

    inline void MovePut(const INT16 row, const INT16 col, const char tmp)
    {
      contents->MovePut(CalcPosRow(row), CalcPosCol(col), tmp, dfltColour.Get());
    }

    inline void MovePut(const INT16 row, const INT16 col, const char tmp, const Colour &attrib)
    {
      contents->MovePut(CalcPosRow(row), CalcPosCol(col), tmp, attrib.Get());
    }

    inline void MovePut(const INT16 row, const INT16 col, const char *tmp, const INT16 len = 0 )
    {
      contents->MovePut(CalcPosRow(row), CalcPosCol(col), tmp, dfltColour.Get(), CalcSizeCols(len));
    }

    inline void MovePut(const INT16 row, const INT16 col, const char *tmp, const Colour &attrib, const INT16 len = 0 )
    {
      contents->MovePut(CalcPosRow(row), CalcPosCol(col), tmp, attrib.Get(), CalcSizeCols(len));
    }
};
 /*FOLD00*/

// class definition: Screen

class Screen : public CSE /*fold00*/
{
  private:
    ZBufCellString *scrnContents;  // holds contents of Screen

  public:
    Screen(void);
    ~Screen(void);

  protected:
    virtual void Send2Me(const BSEMsg &msg, const BOOLEAN frmRoot = false);
    void Put2Scrn(void);
    void CursorOff(void);
    void CursorOn(const UINT16 len = 25);  // len = percentage...

  public:
    void GetScrnMode(void);
    void SetScrnMode(const UINT16 rows, const UINT16 cols);
};
 /*FOLD00*/
#endif

