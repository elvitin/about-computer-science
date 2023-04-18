#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define NUMBER_OF_THREADS 10

void *printHelloWorld(void *threadIdentificator)
{
  printf("Hello world, Welcome by thread: [%d]\n", (int)threadIdentificator);
  pthread_exit(EXIT_SUCCESS);
}

int main(int argmentsCounter, char *argmentsValue[])
{
  pthread_t myThreads[NUMBER_OF_THREADS];
  int status;

  for (int i = 0; i < NUMBER_OF_THREADS; i += 2)
  {
    // printf("main, create thread ID: [%d]\n", i);
    status = pthread_create(&myThreads[i], NULL, printHelloWorld, (void *)i);
    // printf("status: [%d]\n", status);
    if (status != 0)
    {
      printf("erro when create a new thread, error code: [%d]", status);
      exit(EXIT_FAILURE);
    }
  }
  return EXIT_SUCCESS;
}