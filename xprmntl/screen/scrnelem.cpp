
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

#include "scrnelem.hpp"

#ifdef _WIN32
#define DosSleep(x) Sleep(x)
#endif

// class implementation: BSE - Basic Screen Element

UINT32 BSE::nextFreeZ = 0;

BSE::BSE(void) /*fold00*/
{
  /* no Set methods in here as they call Send2Me, a pure virtual function */
  parent   = NULL;
  posRow   = 0;
  posCol   = 0;
  sizeRows = 0;
  sizeCols = 0;
  visible  = false;
  busy     = false;
  posRel   = false;
  sizeRel  = false;
}

void BSE::CheckRelative(void) /*fold00*/
{
  if (posRow < 0 || posCol < 0)
    posRel = true;
  else
    posRel = false;

  if (sizeRows < 0 || sizeCols < 0)
    sizeRel = true;
  else
    sizeRel = false;
}

void BSE::SetPos(const INT16 row, const INT16 col) /*fold00*/
{
  posRow = row;
  posCol = col;
  CheckRelative();
  Send2Me(PosChanged);
}

void BSE::SetSize(const INT16 rows, const INT16 cols) /*fold00*/
{
  sizeRows = rows;
  sizeCols = cols;
  CheckRelative();
  Send2Me(SizeChanged);
}

BOOLEAN BSE::SetBusy(const BOOLEAN newbusy) /*fold00*/
{
  if ( newbusy )
  {
    int i = 0;

    while ( busy && (i++ < 5) )
      DosSleep(1);

    if ( !busy )
    {
      busy = true;
      return true;
    }
  }
  else
  {
    busy = false;
    return true;
  }

  return false;
}

void BSE::SetVisible(const BOOLEAN vis) /*fold00*/
{
  if ( visible != vis)
  {
    visible = vis;
    Send2Me(VisibleChanged);
  }
}

UINT32 BSE::GetNextFreeZ(void) /*fold00*/
{
  if (nextFreeZ == ULONG_MAX)
  {
    Send2Me(MaxZPosReached);
    nextFreeZ = 0;
  }

  return nextFreeZ++;
}

void BSE::SetParent(BSE *par) /*fold00*/
{
  if ( parent != NULL )
  {
    Send2Me(ParentRemoved);
    parent->RemoveWin(this);
  }

  parent = par;

  if ( parent != NULL )
  {
    parent->AddWin(this);
    Send2Me(ParentAdded);
  }
}

UINT16 BSE::GetSizeRows(void) const /*fold00*/
{
  INT16 lrow = 0;

  if ( GetRelSizeRows() < 0 )                    // relative length (rows)
  {
    if ( GetParent() != NULL )
    {
      lrow = (INT16) (GetRelSizeRows() + GetParent()->GetSizeRows() + 1 - GetPosRow());
      if ( lrow < 0 )
        lrow = 0;
    }
  }
  else
    lrow = GetRelSizeRows();

  return (UINT16) lrow;
}

UINT16 BSE::GetSizeCols(void) const /*fold00*/
{
  INT16 lcol = 0;

  if ( GetRelSizeCols() < 0 )                    // relative length (cols)
  {
    if ( GetParent() != NULL )
    {
      lcol = (INT16) (GetRelSizeCols() + GetParent()->GetSizeCols() + 1 - GetPosCol());
      if ( lcol < 0 )
        lcol = 0;
    }
  }
  else
    lcol = GetRelSizeCols();

  return (UINT16) lcol;
}

UINT16 BSE::GetPosRow(void) const /*fold00*/
{
  INT16 lrow = 0;

  if ( GetRelPosRow() < 0 )                    // relative position (row)
  {
    if ( GetParent() != NULL )
    {
      lrow = (INT16) (GetRelPosRow() + GetParent()->GetSizeRows() + 1);
      if ( lrow < 0 )
        lrow = 0;
    }
  }
  else
    lrow = GetRelPosRow();

  return (UINT16) lrow;
}

UINT16 BSE::GetPosCol(void) const /*fold00*/
{
  INT16 lcol = 0;

  if ( GetRelPosCol() < 0 )                    // relative position (col)
  {
    if ( GetParent() != NULL )
    {
      lcol = (INT16) (GetRelPosCol() + GetParent()->GetSizeCols() + 1);
      if ( lcol < 0 )
        lcol = 0;
    }
  }
  else
    lcol = GetRelPosCol();

  return (UINT16) lcol;
}

UINT16 BSE::CalcPosRow(const INT16 row) const /*fold00*/
{
  INT16 prow = row;
  if ( prow < 0 )
  {
    prow = (INT16) (prow + GetSizeRows());
    if ( prow < 0 )
      prow = 0;
  }
  return prow;
}

UINT16 BSE::CalcPosCol(const INT16 col) const /*fold00*/
{
  INT16 pcol = col;
  if ( pcol < 0 )
  {
    pcol = (INT16) (pcol + GetSizeCols());
    if ( pcol < 0 )
      pcol = 0;
  }
  return pcol;
}

UINT16 BSE::CalcSizeRows(const INT16 rows) const /*fold00*/
{
  INT16 prows = rows;
  if ( prows < 0 )
  {
    prows = (INT16) (prows + GetSizeRows() + 1);
    if ( prows < 0 )
      prows = 0;
  }
  return prows;
}

UINT16 BSE::CalcSizeCols(const INT16 cols) const /*fold00*/
{
  INT16 pcols = cols;
  if ( pcols < 0 )
  {
    pcols = (INT16) (pcols + GetSizeCols() + 1);
    if ( pcols < 0 )
      pcols = 0;
  }
  return pcols;
}

void BSE::Send2Children(const BSEMsg &msg) /*fold00*/
{
  if ( children.Entries() )          // do we have children?
  {
    BOOLEAN i = children.JmpFirst();

    while ( i )
    {
      children.GetEntry()->Send2Me(msg);
      if ( msg.IsGlobal() )          // global msg? send to children of child
        children.GetEntry()->Send2Children(msg);
      i = children.JmpNext();
    }
  }
}

void BSE::Send2Parent(const BSEMsg &msg) /*fold00*/
{
  if ( parent )                      // do we have a parent?
    parent->Send2Me(msg);            // send msg to parent
}

void BSE::Send2Root(const BSEMsg &msg) /*fold00*/
{
  if ( parent )                      // do we have a parent?
    parent->Send2Root(msg);          // tell parent to send msg to root
  else
    Send2Me(msg, true);              // we are root, tell Send2Msg
}

void BSE::Send2All(const BSEMsg &msg) /*fold00*/
{
  if ( parent )                      // do we have a parent?
    parent->Send2All(msg);           // send msg to parent
  else                               // no parent? -> we are root
  {
    BSEMsg newmsg = msg;
    newmsg.SetGlobal(true);          // set global flag
    Send2Me(newmsg);                 // send msg to ourselve
    Send2Children(newmsg);           // send msg to our children
  }
}
 /*FOLD00*/

// class implementation: CSE - CellString Screen Element

CSE::CSE(void) /*fold00*/
    :BSE()
{
  // Note: you cannot call SetSize() or SetParent in the constructor and
  // SetPos(), SetVisible() cannot be called before contents was created.
  // BTW: Pos and Size are therefor set in BSE::BSE, parent here.
  contents = new CellString;
  SetZPos();
  SetColour(7);
}

CSE::~CSE(void) /*fold00*/
{
  SetParent(NULL);
  delete contents;
}

void CSE::ScrollUp(const UINT16 lines, const UINT16 startrow, const UINT16 startcol, const UINT16 rows, const UINT16 cols) /*fold00*/
{
  UINT16 srows = rows;
  UINT16 scols = cols;

  if ( srows == 0 )
    srows = GetSizeRows();
  if ( scols == 0 )
    scols = GetSizeCols();

  contents->ScrollUp(lines, startrow, startcol, srows, scols);

  for ( int i = 0; i < lines; i++ )
    MoveFill((UINT16) (startrow+rows+i-1), startcol, 32, scols);
}

void CSE::ScrollDown(const UINT16 lines, const UINT16 startrow, const UINT16 startcol, const UINT16 rows, const UINT16 cols) /*fold00*/
{
  UINT16 srows = rows;
  UINT16 scols = cols;

  if ( srows == 0 )
    srows = GetSizeRows();
  if ( scols == 0 )
    scols = GetSizeCols();

  contents->ScrollDown(lines, startrow, startcol, srows, scols);

  for ( int i = 0; i < lines; i++ )
    MoveFill((UINT16) (startrow+i), startcol, 32, scols);
}

void CSE::DrawHorLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char mid) /*fold00*/
{
  UINT16 prow = CalcPosRow(row);
  UINT16 pcol = CalcPosCol(col);

  for (UINT16 i = col; i < (col+CalcSizeCols(len)); i++ )
    MovePut(prow, i, mid, attrib);
}

void CSE::DrawHorLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char beg, const char mid, const char end) /*fold00*/
{
  UINT16 prow = CalcPosRow(row);
  UINT16 pcol = CalcPosCol(col);
  UINT16 plen = CalcSizeCols(len);;

  MovePut(prow, pcol, beg, attrib);
  DrawHorLine(prow, (INT16) (pcol+1), (INT16) (plen-1), attrib, mid);
  MovePut(prow, (INT16) (pcol+plen-1), end, attrib);
}

void CSE::DrawVerLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char mid) /*fold00*/
{
  UINT16 prow = CalcPosRow(row);
  UINT16 pcol = CalcPosCol(col);

  for ( UINT16 i = prow; i < (prow+CalcSizeRows(len)); i++ )
    MovePut(i, pcol, mid, attrib);
}

void CSE::DrawVerLine(const INT16 row, const INT16 col, const INT16 len, const Colour attrib, const char beg, const char mid, const char end) /*fold00*/
{
  UINT16 prow = CalcPosRow(row);
  UINT16 pcol = CalcPosCol(col);
  UINT16 plen = CalcSizeRows(len);;

  MovePut(prow, pcol, beg, attrib);
  DrawVerLine((INT16) (prow+1), pcol, (INT16) (plen-1), attrib, mid);
  MovePut((INT16) (prow+plen-1), pcol, end, attrib);
}

void CSE::DrawFrame(const Frame &type) /*fold00*/
{
  contents->ResetPart(1, 1, GetSizeRows(), GetSizeCols());
  DrawHorLine( 0,  0, -1, type.FrameColour(), type.LeftUpperCorn() , type.UpperLine(), type.RightUpperCorn());
  DrawVerLine( 1,  0, -2, type.FrameColour(), type.LeftVerLine());
  DrawVerLine( 1, -1, -2, type.FrameColour(), type.RightVerLine());
  DrawHorLine(-1,  0, -1, type.FrameColour(), type.LeftLowerCorn() , type.LowerLine(), type.RightLowerCorn());
  if (type.CaptionText() != NULL)
  {
    UINT16 len  = (UINT16) strlen(type.CaptionText());
    UINT16 cols = GetSizeCols();
    if (cols < 7 )
      len = 0;
    else if (len > cols-6 )
      len = (UINT16) (cols-6);
    if (len != 0)
    {
      MovePut(0, 2, type.LeftCapLimiter(), type.FrameColour());
      MovePut(0, 3, type.CaptionText(), type.CaptionColour(), len);
      MovePut(0, (UINT16) (3+len), type.RightCapLimiter(), type.FrameColour());
    }
  }
  contents->SetPart(1, 1, (UINT16) (GetSizeRows()-2), (UINT16) (GetSizeCols()-2));
}

void CSE::Send2Me(const BSEMsg &msg, const BOOLEAN frmRoot) /*fold00*/
{
  switch (msg.GetMsg())
  {
    case ChildRemoved:
      {
        UINT32 zPos = contents->GetZPos();
        BSEMsg newMsg(ClearZBuffer, &zPos);
        Send2Root(newMsg);
      }
      SetChngFlag();
      break;

    case PosChanged:
      {
        UINT32 zPos = contents->GetZPos();
        BSEMsg clearZBuf(ClearZBuffer, &zPos);
        Send2Root(clearZBuf);             // clear my Z buffer
        if ( IsSizeRelative() )           // check if we need to change size
          SetSize(GetRelSizeRows(), GetRelSizeCols());
        SetChngFlag();
        Send2Parent(ElementChanged);      // parent must refresh, too
      }
      break;

    case SizeChanged:
      {
        UINT32 zPos = contents->GetZPos();
        BSEMsg clearZBuf(ClearZBuffer, &zPos);
        contents->ChgSize(GetSizeRows(), GetSizeCols());
        Redraw();
        Send2Root(clearZBuf);             // clear my Z buffer
        Send2Children(ParentSizeChanged); // tell our children we've changed
        Send2Parent(ElementChanged);      // parent must refresh, too
      }
      break;

    case ParentSizeChanged:
      if ( IsSizeRelative() )             // check if we need to change size
        SetSize(GetRelSizeRows(), GetRelSizeCols());
      break;

    case VisibleChanged:
      if ( !IsVisible() )
      {
        UINT32 zPos = contents->GetZPos();
        BSEMsg newMsg(ClearZBuffer, &zPos);
        Send2Root(newMsg);
      }
      SetChngFlag();
      Send2Parent(ElementChanged);        // parent must refresh, too
      break;

    case ParentRemoved:
      {
        UINT32 zPos = contents->GetZPos();
        BSEMsg clearZBuf(ClearZBuffer, &zPos);
        Send2Root(clearZBuf);             // clear my Z buffer
        Send2Parent(ElementChanged);      // parent must refresh, too
      }
      break;

    case ParentAdded:                     // check if we need to change size
      if ( IsSizeRelative() || IsPosRelative() ) 
        SetSize(GetRelSizeRows(), GetRelSizeCols());
      SetChngFlag();
      break;

    case ElementChanged:
      SetChngFlag();
      break;

    case RefreshRequired:
      if ( !frmRoot )     // came msg from Send2Root?
        Send2Root(msg);   // needed to avoid possible endless loop
      break;

    case DoRefresh:
      {
        ZBufCellString *scrn    = (ZBufCellString*) (msg.GetAddData());
        UINT16          oldrows = scrn->Rows();
        UINT16          oldcols = scrn->Cols();

        if ( contents->Changed(0) )           // contents changed?
        {
          // We have to make sure that the whole window is
          // written to screen and not only a part of it.
          UINT16 myoldrows = contents->Rows();       // save old part
          UINT16 myoldcols = contents->Cols();
          UINT16 myoldprow = contents->PartPosRow();
          UINT16 myoldpcol = contents->PartPosCol();
          contents->ResetPart(myoldprow, myoldpcol, GetSizeRows(), GetSizeCols());
          scrn->MovePut ( GetPosRow(), GetPosCol(), *contents );
          contents->SetPart(myoldprow, myoldpcol, myoldrows, myoldcols);
        }

        scrn->SetPart(GetPosRow(), GetPosCol(), contents->Rows(), contents->Cols());
        Send2Children(msg);
        scrn->ResetPart(GetPosRow(), GetPosCol(), oldrows, oldcols);
      }
      break;

    case ClearZBuffer:
      if ( !frmRoot )     // came msg from Send2Root?
        Send2Root(msg);   // actually unreachable code, maybe throw exception?
      break;

    case MaxZPosReached:
      // missing code: renumber Z positions (not required?)

    case ChildAdded:
    default:
      break;
  }
}
 /*FOLD00*/

// class implementation: Screen

Screen::Screen(void) /*fold00*/
       :CSE ()
{
  CursorOff();  // switch cursor off
  scrnContents = new ZBufCellString;
}

Screen::~Screen(void) /*fold00*/
{
  delete scrnContents;
  CursorOn();   // switch cursor on
}

void Screen::GetScrnMode(void) /*fold00*/
{
  VIOMODEINFO ScrnInfo;
  ScrnInfo.cb = sizeof(ScrnInfo);
  VioGetMode(&ScrnInfo, 0);
  scrnContents->ChgSize(ScrnInfo.row, ScrnInfo.col);
  SetSize(ScrnInfo.row, ScrnInfo.col);
}

void Screen::SetScrnMode(const UINT16 rows, const UINT16 cols) /*fold00*/
{
  if (IsVisible())
  {
    VIOMODEINFO ScrnInfo;
    ScrnInfo.cb = sizeof(ScrnInfo);
    VioGetMode(&ScrnInfo, 0);
    ScrnInfo.row = rows;
    ScrnInfo.col = cols;
    VioSetMode(&ScrnInfo, 0);
    VioGetMode(&ScrnInfo, 0);
    scrnContents->ChgSize(ScrnInfo.row, ScrnInfo.col);
    SetSize(ScrnInfo.row, ScrnInfo.col);
    Refresh();
  }
}

void Screen::CursorOff(void) /*fold00*/
{
  if (IsVisible())
  {
    VIOCURSORINFO CursorData;
    CursorData.yStart = 0;
    CursorData.cEnd   = 0;
    CursorData.cx     = 0;
    CursorData.attr   = -1;
    VioSetCurType(&CursorData, 0);
  }
}

void Screen::CursorOn(const UINT16 len) /*fold00*/
{
  if (!IsVisible())
  {
    VIOCURSORINFO CursorData;
    CursorData.yStart = (UINT16) (-100 + len);
    CursorData.cEnd   = (UINT16)  -100;
    CursorData.cx     = 0;
    CursorData.attr   = 0;
    VioSetCurType(&CursorData, 0);
  }
}

void Screen::Send2Me(const BSEMsg &msg, const BOOLEAN frmRoot) /*fold00*/
{
  switch (msg.GetMsg())
  {
    case RefreshRequired:
      if (IsVisible())
      {
        BSEMsg newMsg(DoRefresh, scrnContents);
        Send2All(newMsg);
        Put2Scrn();
      }
      break;

    case ClearZBuffer:
      scrnContents->ClearZBuffer(*(INT32*)msg.GetAddData());
      break;

    default:
      CSE::Send2Me(msg, frmRoot);
      break;
  }
}

void Screen::Put2Scrn(void) /*fold00*/
{
  UINT16 i;
  // char  tmpbuf[255];
  
  for ( i=0; i < scrnContents->Rows(); i++ )
  {
    if ( scrnContents->ChangesLen(i) > 0 )
    {
      VioWrtCellStr(scrnContents->GetCell(i, scrnContents->ChangesStart(i)),
                    (UINT16) (scrnContents->ChangesLen(i)*2),
                    i, scrnContents->ChangesStart(i), 0 );

      // sprintf(tmpbuf, "updated row: %2d start: %2d len %2d", i, scrnContents->ChangesStart(i), scrnContents->ChangesLen(i));
      // VioWrtCharStr(tmpbuf, strlen(tmpbuf), 0, 0, 0);
      // DosSleep(50);
    }
  }
  
  scrnContents->ClearChanges();
}
 /*FOLD00*/

