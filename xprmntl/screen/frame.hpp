
/******************************
 *       screen classes       *
 *                            *
 *        frame  class        *
 *         headerfile         *
 *                            *
 * written by Michael Reinsch *
 *                            *
 *                      v0.20 *
 ******************************/

#ifndef _S_FRAME_HPP_INCLUDED
#define _S_FRAME_HPP_INCLUDED 0020

// class definition: Frame

enum FrameType
{
  ASCII,
  SingleSingle,
  SingleDouble,
  DoubleSingle,
  DoubleDouble
};


class Frame
{
  private:
    char        frameChars[10];
    Colour      frameColour;
    const char *captionText;
    Colour      captionColour;

  public:
    Frame(const FrameType type, const Colour colour)
    {
      switch(type)
      {
        case ASCII:
          frameChars[0] = frameChars[2] = frameChars[5] = frameChars[6] = '+';
          frameChars[1] = frameChars[7] = '-';
          frameChars[3] = frameChars[4] = '|';
          break;
        case SingleSingle:
          frameChars[0] = '�';
          frameChars[1] = frameChars[6] = '�';
          frameChars[2] = '�';
          frameChars[3] = frameChars[4] = '�';
          frameChars[5] = '�';
          frameChars[7] = '�';
          break;
        case SingleDouble:
          frameChars[0] = '�';
          frameChars[1] = frameChars[6] = '�';
          frameChars[2] = '�';
          frameChars[3] = frameChars[4] = '�';
          frameChars[5] = '�';
          frameChars[7] = '�';
          break;
        case DoubleSingle:
          frameChars[0] = '�';
          frameChars[1] = frameChars[6] = '�';
          frameChars[2] = '�';
          frameChars[3] = frameChars[4] = '�';
          frameChars[5] = '�';
          frameChars[7] = '�';
          break;
        case DoubleDouble:
          frameChars[0] = '�';
          frameChars[1] = frameChars[6] = '�';
          frameChars[2] = '�';
          frameChars[3] = frameChars[4] = '�';
          frameChars[5] = '�';
          frameChars[7] = '�';
          break;
      }

      frameChars[8] = '[';  // Left Caption Limiter
      frameChars[9] = ']';  // Right Caption Limiter
      frameColour   = colour;
      captionText   = NULL;
      captionColour = colour;
    }

    Frame(const char f1, const char f2, const char f3, const char f4, const char f5, const char f6, const char f7, const char f8, const Colour colour)
    {
      frameChars[0] = f1;   // Left Upper Corner
      frameChars[1] = f2;   // Upper Line
      frameChars[2] = f3;   // Right Upper Corner
      frameChars[3] = f4;   // Left Vertical Line
      frameChars[4] = f5;   // Right Vertical Line
      frameChars[5] = f6;   // Left Lower Corner
      frameChars[6] = f7;   // Lower Line
      frameChars[7] = f8;   // Right Lower Corner
      frameChars[8] = '[';  // Left Caption Limiter
      frameChars[9] = ']';  // Right Caption Limiter
      frameColour   = colour;
      captionText   = NULL;
      captionColour = colour;
    }

    inline void SetCaption(const char* caption)
    {
      captionText = caption;
    }

    inline void SetCaption(const char* caption, const Colour colour)
    {
      captionText   = caption;
      captionColour = colour;
    }

    inline Colour FrameColour(void) const
    {
      return frameColour;
    }

    inline const char* CaptionText(void) const
    {
      return captionText;
    }

    inline Colour CaptionColour(void) const
    {
      return captionColour;
    }

    inline char LeftUpperCorn(void) const
    {
      return frameChars[0];
    }

    inline char UpperLine(void) const
    {
      return frameChars[1];
    }

    inline char RightUpperCorn(void) const
    {
      return frameChars[2];
    }

    inline char LeftVerLine(void) const
    {
      return frameChars[3];
    }

    inline char RightVerLine(void) const
    {
      return frameChars[4];
    }

    inline char LeftLowerCorn(void) const
    {
      return frameChars[5];
    }

    inline char LowerLine(void) const
    {
      return frameChars[6];
    }

    inline char RightLowerCorn(void) const
    {
      return frameChars[7];
    }

    inline char LeftCapLimiter(void) const
    {
      return frameChars[8];
    }

    inline char RightCapLimiter(void) const
    {
      return frameChars[9];
    }
};

#endif

