#if !defined    (_TYPESIZE_H)
#define _TYPESIZE_H

//
//  typesize.h                         written by tom schlangen
//  ----------                         modified by TJW
//                                     modified by MR
//
//  this is an attempt to somewhat reduce problems by unifying
//  compiler dependend type sizes.
//
//  the basic set/list of unified types required to be present
//  for each compiler is:
//
//  --------+-----------------+-------+-----------------------
//  name    | description     | bytes | range
//  --------+-----------------+-------+-----------------------
//  CHAR    | signed char     | 1     | -128..127
//  UCHAR   | unsigned char   | 1     | 0..255
//  INT16   | signed word     | 2     | -32768..32767
//  UINT16  | unsigned word   | 2     | 0..65535
//  INT32   | signed dword    | 4     | -2147483648..2147483647
//  UINT32  | unsigned dword  | 4     | 0..4294967295
//  --------+-----------------+-------+-----------------------
//
//  besides that, there are some further general purpose types
//  with guaranteed (either by ANSI C or by us) sizes/ranges.
//  these should be used with care, since beneath their
//  guaranteed size they are strictly compiler specific. so if
//  you use them, make sure you do so only within the
//  guaranteed range. also take care not to use them in byte-
//  aligned (`packed') structures, since the size of the re-
//  resulting structures may vary from compiler to compiler,
//  which may cause hazzard on in certain cases.
//
//  --------+-------------------------------------------------
//  name    | description
//  --------+-------------------------------------------------
//  INT     | general purpose compiler specific `signed int'.
//          | ANSI C guarantees at least 2 bytes,
//          | range -32768..32767 for this type.
//  UINT    | general purpose compiler specific `unsigned int',
//          | we guarantee at least 2 bytes,
//          | range 0..65535 for this type.
//  LONG    | general purpose compiler specific `signed long'.
//          | ANSI C * guarantees at least 4 bytes,
//          | range -2147483648..2147483647 for this type.
//  ULONG   | general purpose compiler specific `unsigned long'.
//          | we guarantee at least 4 bytes,
//          | range 0..4294967295 for this type.
//  --------+-------------------------------------------------
//
//  the following definition blocks are in alphabetical order
//  of the various compilers identification defines. please add
//  the definitions for your compiler, if not already present.
//

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//  add your compiler here if it doesn't match #else...

#if defined(__BORLANDC__)
  
typedef          char       CHAR;               // 1 byte
typedef unsigned char      UCHAR;               // 1 byte
typedef signed   short      INT16;              // 2 byte
typedef unsigned short     UINT16;              // 2 byte
typedef signed   long       INT32;              // 4 byte
typedef unsigned long      UINT32;              // 4 byte
// --------------------------------------------------------------------------
typedef signed   int        INT;                // 2/4 byte
typedef unsigned int       UINT;                // 2/4 byte
typedef signed   long       LONG;               // 4 byte
typedef unsigned long      ULONG;               // 4 byte

#else

typedef signed   char       CHAR;               // 1 byte
typedef unsigned char      UCHAR;               // 1 byte
typedef signed   short      INT16;              // 2 byte
typedef unsigned short     UINT16;              // 2 byte
typedef signed   long       INT32;              // 4 byte
typedef unsigned long      UINT32;              // 4 byte
// --------------------------------------------------------------------------
typedef signed   int        INT;                // 2/4 byte
typedef unsigned int       UINT;                // 2/4 byte
typedef signed   long       LONG;               // 4 byte
typedef unsigned long      ULONG;               // 4 byte
typedef          void       VOID;

typedef enum { false = 0, true = 1 } BOOLEAN;

#endif

#ifdef __cplusplus
}
#endif

#endif                                          // #if !defined(_TYPESIZE_H)

