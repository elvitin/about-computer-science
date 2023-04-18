#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/stat.h>
#define SOCKET_PATH "/tmp/socket_repo/my_unix_socket"

int main(void)
{
  mkdir("/tmp/socket_repo", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  int socket_fd;
  struct sockaddr_un socket_addr;
  socklen_t address_size;
  char message[] = "hello, server!";
  char buffer[256];

  // cria o socket Unix domain
  socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (socket_fd < 0)
  {
    perror("falha ao criar o socket");
    exit(EXIT_FAILURE);
  }

  // configura o endereço do socket
  socket_addr.sun_family = AF_UNIX;
  strcpy(socket_addr.sun_path, SOCKET_PATH);
  address_size = sizeof(socket_addr);

  // conecta ao socket
  if (connect(socket_fd, (struct sockaddr *)&socket_addr, address_size) < 0)
  {
    perror("falha ao se conectar ao socket");
    exit(EXIT_FAILURE);
  }

  printf("conexão estabelecida!\n");
  sleep(5);
  // envia uma mensagem para o servidor
  puts("enviando mensagem para o servidor...");
  send(socket_fd, message, sizeof(message), 0);
  sleep(5);
  // recebe uma mensagem do servidor
  recv(socket_fd, buffer, sizeof(buffer), 0);

  printf("mensagem recebida: %s\n", buffer);

  // fecha o socket
  close(socket_fd);

  return EXIT_SUCCESS;
}
