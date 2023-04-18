#include <stdio.h>
#include <gnu/libc-version.h>

int main()
{
  puts(gnu_get_libc_release());
  puts(gnu_get_libc_version());
  return 0;
}
