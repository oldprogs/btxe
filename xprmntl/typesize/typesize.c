#include    <stdio.h>

void main(void)
{
  fprintf(stdout, "\n------------------------------------------------------");
  fprintf(stdout, "\nThe sizes of some common types with your compiler are:");
  fprintf(stdout, "\n------------------------------------------------------");
  fprintf(stdout, "\nsizeof char\t: %i byte(s)", (int) sizeof(char));
  fprintf(stdout, "\nsizeof short\t: %i byte(s)", (int) sizeof(short));
  fprintf(stdout, "\nsizeof int\t: %i byte(s)", (int) sizeof(int));
  fprintf(stdout, "\nsizeof long\t: %i byte(s)", (int) sizeof(long));
  fprintf(stdout, "\n------------------------------------------------------\n");

  fprintf(stdout, "\n------------------------------------------------------");
  fprintf(stdout, "\nValid usability ranges are:");
  fprintf(stdout, "\n------------------------------------------------------");
  fprintf(stdout, "\n1 byte   signed      -128..127");
  fprintf(stdout, "\n1 byte   unsigned    0..255");
  fprintf(stdout, "\n2 bytes  signed      -32768..32767");
  fprintf(stdout, "\n2 bytes  unsigned    0..65535");
  fprintf(stdout, "\n4 bytes  signed      -2147483648..2147483647");
  fprintf(stdout, "\n4 bytes  unsigned    0..4294967295");
  fprintf(stdout, "\n------------------------------------------------------\n");
}
