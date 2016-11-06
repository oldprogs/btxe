
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

#include "string.h"
#include "colour.hpp"

// class implementation: Colour

const char *Colour::availColours[] =
{
  "black", "blue", "green", "cyan", "red", "magenta", "brown", "white",
  "gray", "brightblue", "brightgreen", "brightcyan", "brightred",
  "brightmagenta", "yellow", "brightwhite", NULL
};


CHAR Colour::Str2Colour(const char *col) /*fold00*/
{
  char i;

  if ( col != NULL )
    if ( *col )
      for ( i = 0; availColours[i] != NULL; i++ )
        if ( !strnicmp(col, availColours[i], strlen(availColours[i])) )
          return i;

  return -1;
}

Colour &Colour::operator=(const char *str) /*fold00*/
{
  // syntax: briblue on black
  //         briblue          (doesn't change background colour)
  //                 on black (doesn't change foreground colour)

  const char *tmp = str;

  if ( tmp != NULL )
  {
    for (; *tmp && (*tmp==' '); tmp++ );      // skip leading blanks

    if ( strnicmp(tmp, "on ", 3) )            // not started with "ON "
    {
      operator<<(tmp);                        // set foreground colour
      for (; *tmp && (*tmp!=' '); tmp++ );    // jump to next blank
      for (; *tmp && (*tmp==' '); tmp++ );    // skip leading blanks

      if ( !strnicmp (tmp, "on ", 3) )        // shall we set bckgrnd col?
        for (; *tmp && (*tmp!=' '); tmp++ );  // jump to next blank
      else
        tmp = NULL;
    }
    else                                      // only set bckgrnd colour
      for (; *tmp && (*tmp!=' '); tmp++ );    // jump to next blank
  }

  if ( tmp != NULL )
  {
    for (; *tmp && (*tmp==' '); tmp++ );      // skip leading blanks
    operator>>(tmp);                          // set background colour
  }

  return *this;
}

Colour &Colour::operator<<(const char col)   // set foreground colour /*fold00*/
{
  attrib = (char) ((attrib & 0xF0) + (col & 0xF));
  return *this;
}

Colour &Colour::operator<<(const char *str)  // set foreground colour /*fold00*/
{
  CHAR col = Str2Colour(str);

  if ( col != -1 )
    operator<<((char) col);

  return *this;
}

Colour &Colour::operator>>(const char col)   // set background colour /*fold00*/
{
  attrib = (char) ((attrib & 0xF) + ((col & 0xF) << 4));
  return *this;
}

Colour &Colour::operator>>(const char *str)  // set background colour /*fold00*/
{
  CHAR col = Str2Colour(str);

  if ( col != -1 )
    operator>>((char) col);

  return *this;
}
 /*FOLD00*/

