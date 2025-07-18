#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
  struct sockaddr_in sa;
  int res;
  int SocketFD;

  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketFD == -1)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&sa, 0, sizeof sa);

  sa.sin_family = AF_INET;
  sa.sin_port = htons(1100);
  res = inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

  if (connect(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1)
  {
    perror("conexão falhou");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  for (size_t i = 0; i < 100; i++)
  {
    int n = write(SocketFD, "Hello", 5);
    if (n < 0)
      perror("ERROR writing to socket");
  }
  close(SocketFD);
  return EXIT_SUCCESS;
}