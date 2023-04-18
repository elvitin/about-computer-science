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
  int connection_fd;
  struct sockaddr_un socket_addr;
  socklen_t address_size;
  char message[] = "hello, client!";
  char buffer[256];

  // cria o socket unix domain
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

  // liga o socket
  if (bind(socket_fd, (struct sockaddr *)&socket_addr, address_size) < 0)
  {
    perror("falha ao ligar o socket");
    exit(EXIT_FAILURE);
  }

  // aguarda por conexões
  if (listen(socket_fd, 5) < 0)
  {
    perror("falha ao aguardar por conexões");
    exit(EXIT_FAILURE);
  }

  // aceita uma conexão - aqui o código fica bloqueado
  puts("aguardando pela conexão do \"cliente\", pausei na funcao accept...");
  connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &address_size);

  if (connection_fd < 0)
  {
    perror("falha ao aceitar conexão");
    exit(EXIT_FAILURE);
  }

  printf("conexão aceita!\n");

  // envia uma mensagem para o cliente
  send(connection_fd, message, sizeof(message), 0);

  // recebe uma mensagem do cliente
  puts("enviei uma mensagem para o cliente e pausei no metodo recv...");
  recv(connection_fd, buffer, sizeof(buffer), 0);

  printf("recebi uma mensagem do cliente: %s\n", buffer);

  // fecha a conexão e o socket
  close(connection_fd);
  close(socket_fd);

  // remove o arquivo de socket
  unlink(SOCKET_PATH);

  return 0;
}