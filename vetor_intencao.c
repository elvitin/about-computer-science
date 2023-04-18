
#include <sys/shm.h> // for shmget(), shmat()
#include <sys/wait.h>
#include <unistd.h>  // for fork(), sleep()
#include <stdlib.h>  // for EXIT_SUCCESS
#include <stdbool.h> // for bool
#include <stdio.h>   // for puts()

bool *flag;
int main()
{

  int shmid = shmget(IPC_CREAT, 2 * sizeof(bool), IPC_CREAT | 0666);
  flag = (bool *)shmat(shmid, NULL, 0);

  if (fork() > 0)
  {
    // parent
    flag[0] = true;
    while (flag[1])
    {
      flag[0] = false;
      sleep(1);
      flag[0] = true;
    }
    puts("regiao critica do pai");
    sleep(1);
    flag[0] = false;
    wait(NULL);
  }
  else
  {
    // child
    flag[1] = true;
    while (flag[0])
    {
      flag[1] = false;
      sleep(1);
      flag[1] = true;
    }
    puts("regiao critica do filho");
    sleep(1);
    flag[1] = false;
  }

  shmdt(flag);
  shmctl(shmid, IPC_RMID, NULL);
  return EXIT_SUCCESS;
}