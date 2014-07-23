#include <time.h>

static void printtime() {
   time_t timer;
   char buffer[25];
   struct tm* tm_info;

   time(&timer);
   tm_info = localtime(&timer);

   strftime(buffer, 25, "[%H:%M:%S]: ", tm_info);
   printf("%s",buffer);
}