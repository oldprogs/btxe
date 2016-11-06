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
 * Filename    : $Source: E:/cvs/btxe/xprmntl/bbs/ansi2bs.c,v $
 * Revision    : $Revision: 1.4 $
 * Tagname     : $Name: XH6 $
 * Last updated: $Date: 1999/02/27 12:10:48 $
 * State       : $State: Exp $
 * Orig. Author: C. Eike Hofmeister, 2:2426/2280
 *
 * Description : ANSI to BBS-Script converter
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#ifdef __unix__
#include "../typesize/typesize.h"
#else
#include "..\typesize\typesize.h"
#endif

FILE *fin, *fout;
UCHAR dummy[256], dummy2[256];
INT i, i2, s[5], sn;
CHAR HiLo;
UCHAR ColorAttr, ColorFG, ColorBG, Blink;

UCHAR ColTab[8][2] =
{
  {0, 8},
  {4, 12},
  {2, 10},
  {6, 14},
  {1, 9},
  {5, 13},
  {3, 11},
  {7, 15}};

/*         ( (Black     , DarkGray),                     *
 *           (Red       , LightRed),                     *
 *           (Green     , LightGreen),                   *
 *           (Brown     , Yellow),                       *
 *           (Blue      , LightBlue),                    *
 *           (Magenta   , LightMagenta),                 *
 *           (Cyan      , LightCyan),                    *
 *           (LightGray , White) );                      */

void 
main (int argc, char *argv[], char *envp[])
{

  if (argc != 2)
  {
    printf ("\nTo call this programm type:\n%s <filename>\n\n", argv[0]);
    return;
  }                             /* endif */

  if ((fin = fopen (argv[1], "r")) == NULL)
  {
    printf ("\nFile %s not found!\n", argv[1]);
    return;
  }                             /* endif */

  strcpy (&(dummy[0]), argv[1]);
  for (i = strlen (dummy) - 1; i >= 0; i--)
  {
    if ((dummy[i] == '.'))
    {
      dummy[i] = '\0';
      break;
    }
  }                             /* endfor */
  strncat (&(dummy[0]), ".BS", 3);
  if ((fout = fopen (dummy, "wb+")) == NULL)
  {
    fclose (fin);
    printf ("\nCould not write %s!\n", dummy);
    return;
  }                             /* endif */

  ColorFG = 7;
  ColorBG = 0;
  HiLo = 0;
  Blink = 0;

  while ((fgets (dummy, 255, fin)) != NULL)
  {

    fprintf (fout, "print \"");
    i = 0;
    while (dummy[i] != '\0')
    {

      if (dummy[i] != 27)
      {
        if ((dummy[i] != 10) && (dummy[i] != 13))
          fputc (dummy[i], fout);
      }
      else
      {

        dummy2[0] = '\0';
        i2 = 0;
        do
        {
          dummy2[i2++] = dummy[++i];
          dummy2[i2] = '\0';
        }
        while ((toupper (dummy[i]) < 'A') || ('Z' < toupper (dummy[i])));  /* enddo */

        for (i2 = 0; i2 < 3; i2++)
          s[i2] = 1;
        sn = sscanf (dummy2, "[%d;%d;%d;%d;%d", &s[0], &s[1], &s[2], &s[3], &s[4]);

        switch (dummy2[strlen (dummy2) - 1])
        {
        case 'H':              /* 'H': GoToXY(nu[2],nu[1]); */
        case 'f':              /* 'f': GoToXY(nu[2],nu[1]); */
          fprintf (fout, "{m%d,%d}", s[1], s[0]);
          break;
        case 'A':              /* 'A': If WhereY-nu[1]>0 Then GoToXY(WhereX,WhereY-nu[1]); */
          fprintf (fout, "{^%d}", s[0]);
          break;
        case 'B':              /* 'B': If WhereY+nu[1]<RandUnten Then GoToXY(WhereX,WhereY+nu[1]); */
          fprintf (fout, "{!%d}", s[0]);
          break;
        case 'C':              /* 'C': If WhereX+nu[1]<81 Then GoToXY(WhereX+nu[1],WhereY); */
          fprintf (fout, "{>%d}", s[0]);
          break;
        case 'D':              /* 'D': If WhereX-nu[1]>0 Then GoToXY(WhereX-nu[1],WhereY); */
          fprintf (fout, "{<%d}", s[0]);
          break;
        case 'J':              /* 'J': If nu[1]=2 Then ClrScr; */
          fprintf (fout, "{x}");
          break;
        case 'm':
          for (i2 = 0; i2 < sn; i2++)
          {
            switch (s[i2])
            {
            case 0:
              ColorBG = 0;
              HiLo = 0;
              Blink = 0;
              break;
            case 1:
              HiLo = 1;
              break;
            case 2:
              Blink = 1;
              break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
              ColorFG = s[i2] - 30;
              break;
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
              ColorBG = s[i2] - 40;
              break;
            default:
              printf ("\n\nUnknown value in ANSI-Sequence\n");
              fclose (fout);
              fclose (fin);
              return;
              break;
            }                   /* endswitch */
          }                     /* endfor */
          fprintf (fout, "{c%x}", (UCHAR) (128 * Blink) + (ColTab[(int) ColorBG][0] << 4) + (ColTab[(int) ColorFG][(int) HiLo]));
          break;
        default:
          printf ("\nUnknown/unsupported ANSI-Sequence (%s) ignored.", dummy2);
          break;
        }                       /* endswitch */

      }                         /* endif */
      i++;
    }                           /* endwhile */
    fprintf (fout, "\"\n");

  }                             /* endwhile */
  printf ("\n\n");

}                               /* end main */
