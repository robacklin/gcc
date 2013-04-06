/* More paste corner cases from glibc.  */
/* { dg-do run } */

#include <stdlib.h>
#include <string.h>

#define symbol_version(name, version) name##@##version
#define str(x) xstr(x)
#define xstr(x) #x

const char a[] = str(symbol_version(getrlimit, GLIBC_2.0));
/* { dg-warning "valid preprocessing token" "" { target *-*-* } 11 } */
const char b[] = str(getrlimit@GLIBC_2.0);
const char c[] = "getrlimit@GLIBC_2.0";

int
main(void)
{
  if(strcmp(a, b))
    abort();
  if(strcmp(b, c))
    abort();
  if(strcmp(c, a))
    abort();

  return 0;
}
