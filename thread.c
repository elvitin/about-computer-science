#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define NUMBER_OF_THREADS 10

struct Person
{
  int age;
};

void *printHelloWorld(void *threadIdentificator)
{
  printf("Hello world, Welcome by thread: [%d]\n", (int)threadIdentificator);
  pthread_exit(EXIT_SUCCESS);
}

int main(int argmentsCounter, char *argmentsValue[])
{
  return EXIT_SUCCESS;
}