#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  time_t t;
  struct tm *stm;
  char *s;
  
  printf(
"Time-Check - Freeware written by Thomas Waldmann, 14.4.1997\n"
"\n"
"This program is an example for using the time functions of the standard C\n"
"library: tzset(), time(), localtime() and gmtime(). You also can check your\n"
"time related settings as all related informations will be displayed...\n"
"\n"
  );
  
  
  printf("Invoking TZ=getenv(\"TZ\") to get TZ environment variable ...\n");
  s=getenv("TZ");
  printf("TZ = %s\n",s);
  printf("Example for correct TZ string for Germany:\n"
         "     CET-1CDT,3,-1,0,7200,10,-1,0,10800,3600\n"
         "\n"
  );
  
  printf("Invoking tzset() to initialize time functions ...\n\n");
  tzset();

  printf("Invoking t=time(NULL) to get current time ...\n");
  
  sscanf( argv[1], "%ld", &t);
  
  printf("time      = %ld (seconds since 1.1.1970 00:00 UTC)\n\n",t);

  printf("Invoking localtime(&t) to convert to local time ...\n");
  stm = localtime(&t);
  printf("localtime = %s %s",
         (stm->tm_isdst>0) ? "DST" : "STD",
         asctime(stm));
  printf("            DST = daylight saving time, german: Sommerzeit\n"
         "            STD = standard time,        german: Winterzeit\n"
         "\n"
  );

  printf("Invoking gmtime(&t) to convert to GMT / UTC ...\n");
  stm = gmtime(&t);
  printf("gmtime    = %s %s", "UTC", asctime(stm));

  return 0;
}
