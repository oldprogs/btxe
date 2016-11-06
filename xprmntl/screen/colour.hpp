
/******************************
 *       screen classes       *
 *                            *
 *        colour class        *
 *         headerfile         *
 *                            *
 * written by Michael Reinsch *
 *                            *
 *                      v0.20 *
 ******************************/

#ifndef _S_COLOUR_HPP_INCLUDED
#define _S_COLOUR_HPP_INCLUDED 0020
#include "typesize.h"

// class definition: Colour

// syntax: briblue on black
//         briblue          (doesn't change background colour)
//                 on black (doesn't change foreground colour)

class Colour
{
  private:
    char               attrib;
    static const char *availColours[];

  public:
    inline Colour(void)
    {
      attrib = 0;
    }

    inline Colour(const char att)
    {
      operator=(att);
    }

    inline Colour(const char *att)
    {
      operator=(att);
    }

  private:
    CHAR Str2Colour(const char *col);

  public:
    Colour &operator=(const char *str);          // set colour
    Colour &operator<<(const char col);          // set foreground colour
    Colour &operator<<(const char *str);         // set foreground colour
    Colour &operator>>(const char col);          // set background colour
    Colour &operator>>(const char *str);         // set background colour

    inline Colour &operator++(void)              // chg foreground col
    {
      operator<<((char) (attrib + 1));
      return *this;
    }

    inline Colour &operator++(const int)         // chg background col
    {
      operator>>((char) ((attrib >> 4) + 1));
      return *this;
    }

    inline Colour &operator+=(const char tmp)    // chg fore-/background col
    {
      attrib += tmp;
      return *this;
    }

    inline Colour &operator--(void)              // chg foreground col
    {
      operator<<((char) (attrib - 1));
      return *this;
    }

    inline Colour &operator--(const int)         // chg background col
    {
      operator>>((char) ((attrib >> 4) - 1));
      return *this;
    }

    inline Colour &operator-=(const char tmp)    // chg fore-/background col
    {
      attrib -= tmp;
      return *this;
    }

    inline Colour &operator=(const char col)     // set colour
    {
      attrib = col;
      return *this;
    }

    inline char Get(void) const
    {
      return attrib;
    }
};

#endif

