//
// little test program
//

#include "screen.hpp"

#ifdef __DOS__
#define DosSleep(x)
#endif

#ifdef _WIN32
#define DosSleep(x) Sleep(x)
#endif


class Background : public Screen
{
  public:
    Background(void) : Screen()
    {
      SetVisible(true);  // display background
      GetScrnMode();     // get current screen mode and adjust backgrnd
    }

    ~Background(void)
    {
      Clear();           // clear screen upon exit
      Refresh();
    }

  protected:
    void Redraw(void)
    {
      Colour col("white on black");
      Fill ( '°', col );
    }
};


class TestWindow : public CSE
{
  public:
    TestWindow(BSE *par) : CSE()
    {
      SetPos(10, 10);
      SetSize(10, 10);
      SetVisible(true);
      SetParent(par);
    }

  protected:
    void Redraw(void)
    {
      Frame  myFrame(SingleSingle, "blue on black");
      myFrame.SetCaption(" Testwindow ", "brightcyan on black");
      Clear();
      DrawFrame(myFrame);
      MovePut( 0,0,"This is a test!");
      MovePut( 2,2,"TestTest");
    }
};


class HeadLine : public CSE
{
  public:
    HeadLine(BSE *par) : CSE()
    {
      SetPos(0, 0);
      SetSize(1, -1);
      SetColour("yellow on blue");
      SetVisible(true);
      SetParent(par);
    }

  protected:
    void Redraw(void)
    {
      Clear();
      MovePut( 0,   0, "[xx]  Node: 2:2474/100");
      MovePut( 0, -23, "...und ueberhaupt!? Box");
    }
};

class FootLine : public CSE
{
  public:
    FootLine(BSE *par) : CSE()
    {
      SetPos(-2, 0);
      SetSize(1, -1);
      SetColour("yellow on blue");
      SetVisible(true);
      SetParent(par);
    }

  protected:
    void Redraw(void)
    {
      Clear();
      MovePut( 0,   0, "BinkleyTerm 2.60XE/Gamma-7 Design Study");
      MovePut( 0, -20, "hit Alt-F10 for Help");
    }
};

class CurrentSettings : public CSE
{
  public:
    CurrentSettings(BSE *par, INT16 pRow, INT16 pCol, INT16 sRows, INT16 sCols) : CSE()
    {
      SetPos(pRow, pCol);
      SetSize(sRows, sCols);
      SetColour("brightgreen on blue");
      SetVisible(true);
      SetParent(par);
    }

  protected:
    void Redraw(void)
    {
      Clear();
    }
};

class StatusWindow : public CSE
{
  private:
    CurrentSettings *curset;

  public:
    StatusWindow(BSE *par) : CSE()
    {
      SetPos(1, 0);
      SetSize(11, -1);
      SetVisible(true);
      SetParent(par);
      curset = new CurrentSettings(this, 1, 1, 10, 18);
    }

  protected:
    void Redraw(void)
    {
      Clear();
    }
};






int main()
{
  int i;

  Background bckgrnd;
  bckgrnd.Refresh();
  DosSleep(200);

  {
    HeadLine head(&bckgrnd);
    bckgrnd.Refresh();
    FootLine foot(&bckgrnd);
    bckgrnd.Refresh();

    TestWindow win1(&bckgrnd);
    win1.Refresh();
    DosSleep(200);

    for (i=1; i<8; ++i)
    {
      win1.SetPos(10-i, 10-i);
      win1.Refresh();
      DosSleep(100);
    }

    for (i=1; i<5; ++i)
    {
      win1.SetSize(10+i, 10+i);
      win1.Refresh();
      DosSleep(100);
    }

    for (i=1; i<10; ++i)
    {
      win1.SetSize(15, 14+i);
      win1.Refresh();
      DosSleep(100);
    }

    DosSleep(1000);

    for (i=1; i<5; ++i)
    {
      win1.SetSize(15-i, 25-i);
      win1.Refresh();
      DosSleep(100);
    }

    DosSleep(1000);

    win1.SetSize(15, 30);
    win1.Refresh();

    DosSleep(1000);

    win1.SetSize(-2, -2);
    win1.Refresh();

    DosSleep(1000);

    bckgrnd.SetScrnMode(25,80);

    DosSleep(1000);

    bckgrnd.SetScrnMode(50,80);

    DosSleep(1000);

    bckgrnd.SetScrnMode(30,80);

    for (i=1; i<9; ++i)
    {
      win1.SetPos(2+i, 2+i);
      win1.Refresh();
      DosSleep(100);
    }

  }

  DosSleep(1000);

  return 0;
}

