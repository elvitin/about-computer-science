#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int *turn;
int main()
{
  int shmid = shmget(IPC_CREAT, 2 * sizeof(int), IPC_CREAT | 0666);
  turn = (int *)shmat(shmid, NULL, 0);
  turn[0] = 0;
  if (fork() > 0)
  {
    while (turn[0] == 1)
      printf("estou bloqueado - processo %d: \n", getpid());
    turn[0] = 1;
    printf("< critical section %d: >\n", getpid());
    turn[0] = 0;
  }
  else
  {
    while (turn[0] == 0)
      printf("estou bloqueado - processo %d: \n", getpid());
    turn[0] = 0;
    printf("< critical section %d: >\n", getpid());
    turn[0] = 1;
  }
  // libera alocacao
  shmdt(turn);
  shmctl(shmid, IPC_RMID, NULL);
  exit(1);
}