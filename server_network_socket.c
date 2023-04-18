#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024
int main(void)
{
  struct sockaddr_in internetSocketAddress;

  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (SocketFD == -1)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&internetSocketAddress, 0, sizeof internetSocketAddress);

  internetSocketAddress.sin_family = AF_INET;
  internetSocketAddress.sin_port = htons(1100);
  internetSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(SocketFD, (struct sockaddr *)&internetSocketAddress, sizeof internetSocketAddress) == -1)
  {
    perror("bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if (listen(SocketFD, 10) == -1)
  {
    perror("listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  puts("waiting for connection... ");
  int ConnectFD = accept(SocketFD, NULL, NULL); // código é pausado aqui
  if (ConnectFD == -1)
  {
    perror("accept failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  puts("connection accepted! ");
  while (1)
  {
    puts("\nwaiting for message...");
    char buffer[BUFF_SIZE];
    int n = read(ConnectFD, buffer, BUFF_SIZE - 1);
    if (n < 0)
      perror("ERROR reading from socket");
    else
    {
      puts("----------------------------------------");
      puts(buffer);
      puts("----------------------------------------");
    }
  }
  // if (shutdown(ConnectFD, SHUT_RDWR) == -1)
  // {
  //   perror("shutdown failed");
  //   close(ConnectFD);
  //   close(SocketFD);
  //   exit(EXIT_FAILURE);
  // }
  // close(ConnectFD);
  close(SocketFD);
  return EXIT_SUCCESS;
}