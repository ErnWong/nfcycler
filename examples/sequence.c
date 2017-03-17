#include <stdio.h>

#define DLOG 0
#define dlog(...) \
  do { \
    if (DLOG) \
    { \
      fprintf(stderr,"[seq] "); \
      fprintf(stderr,__VA_ARGS__); \
    }\
  } while (0)

int main()
{
  dlog("Sequence stderr Hi!\n");
  dlog("Putting '5' to stdout\n");
  puts("5");
  fflush(stdout);
  int x;
  dlog("Waiting for input\n");
  while (scanf("%d", &x) != EOF)
  {
    dlog("Got input %d\n", x);
    /*
    if (x % 2 == 1)
    {
      x *= 7 + x * x + 2*x;
      x %= 15364;
    }
    else
      x /= 2;
      */
    x++;
    x %= 100;
    dlog("Putting '%d' to stdout\n", x);
    printf("%d\n", x);
    fflush(stdout);
    dlog("Waiting for input\n");
  }
  dlog("Ending sequence program\n");
}
