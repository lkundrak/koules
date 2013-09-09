#ifndef HAVEUSLEEP
#if __STDC__
#include <stdlib.h>
#endif
#include <time.h>
#include <sys/time.h>

void
myusleep (usecs)
     unsigned long usecs;
{
  struct timeval tv;
  tv.tv_sec  = usecs / 1000000L;
  tv.tv_usec = usecs % 1000000L;
  (void) select (0, (void *)0, (void *)0, (void *)0, &tv);
}
#else
void dummy(void)
{
}
#endif
