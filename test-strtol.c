#include <stdio.h>
#include <stdlib.h>

int main()
{
  char str[30] = "2030300 This is test";
  char *ptr;
  long ret;

  ret = strtol(str, &ptr, 10);
  perror("strtol");
  printf("The number(unsigned long integer) is %ld\n", ret);
  printf("String part is |%s|\n", ptr);

  return (0);
}