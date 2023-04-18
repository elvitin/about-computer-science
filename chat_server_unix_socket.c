#include <stdio.h>      // for perror, puts
#include <string.h>     // for memset, strcpy
#include <unistd.h>     //for sleep()
#include <sys/un.h>     // for sockaddr_un
#include <sys/socket.h> // for socket, AF_UNIX, SOCK_STREAM, socklen_t
#include <sys/stat.h>   // for S_IRWXU, S_IRWXG, S_IROTH, S_IXOTH
#include <stdlib.h>     // for EXIT_SUCCESS, EXIT_FAILURE, strtol()
#include <stdbool.h>    //for bool, true, false
#include <ctype.h>      // for isdigit()
#include <limits.h>     // for INT_MAX
#include <signal.h>     // for signal(), SIGINT

#define SOCKET_PATH "/tmp/socket_repo"
#define SOCKET_FILE "/tmp/socket_repo/my_unix_socket"
#define BUFFER_SIZE 256
// this file is a chat server
struct SocketState
{
  int socketFileDescriptor;
  int connectionFileDescriptor;
  int maxConnections;
  socklen_t addressSize;
  struct sockaddr_un socketAddress;
  char buffer[BUFFER_SIZE];
};

void createSocket(struct SocketState *socketState)
{
  puts("criando o socket unix domain...");
  socketState->socketFileDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socketState->socketFileDescriptor < 0)
  {
    perror("falha ao criar o socket");
    exit(EXIT_FAILURE);
  }
  if (unlink(SOCKET_FILE) < 0)
  {
    perror("falha ao desvincular o arquivo socket, ele provavelmente já não existe");
  }
  puts("socket criado!");
}

void configureSocketAddress(struct SocketState *socketState)
{
  puts("configurando o endereço do socket...");
  memset(&socketState->socketAddress, 0, sizeof(socketState->socketAddress));
  socketState->socketAddress.sun_family = AF_UNIX;
  // esse peguei na net, confesso que não entendi esses pipes, somente as constantes que são definidas no header sys/stat.h
  mkdir(SOCKET_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  strcpy(socketState->socketAddress.sun_path, SOCKET_FILE);
  socketState->addressSize = sizeof(socketState->socketAddress);
}

void bindSocket(struct SocketState *socketState)
{
  puts("ligando o socket...");
  if (bind(socketState->socketFileDescriptor, (struct sockaddr *)&socketState->socketAddress, socketState->addressSize) < 0)
  {
    perror("falha ao ligar o socket");
    exit(EXIT_FAILURE);
  }
  puts("socket ligado!");
}

void listenForConnections(struct SocketState *socketState)
{
  puts("registrando ouvintes de conexão...");
  if (listen(socketState->socketFileDescriptor, socketState->maxConnections) < 0)
  {
    perror("falha ao registrar ouvintes de conexão");
    exit(EXIT_FAILURE);
  }
  puts("ouvintes registrados!");
}

void acceptConnection(struct SocketState *socketState)
{
  puts("aceitando conexão...");
  socketState->connectionFileDescriptor = accept(socketState->socketFileDescriptor, (struct sockaddr *)&socketState->socketAddress, &socketState->addressSize);
  if (socketState->connectionFileDescriptor < 0)
  {
    perror("falha ao aceitar conexão");
    exit(EXIT_FAILURE);
  }
  puts("conexão aceita!");
}

void setMaxConnetions(int argumentsCounter, char *argumentValues[], struct SocketState *socketState)
{
  socketState->maxConnections = 5;
  if (argumentsCounter > 1)
  {
    char *endptr;
    long int number = strtol((argumentValues[1]), &endptr, 10);
    if (number > INT_MAX)
    {
      puts("o valor passado é muito grande, usando 5 conexões como padrão!");
      return;
    }

    if (*endptr == '\0')
      socketState->maxConnections = (int)number;
    else
      puts("argumento inválido, usando 5 conexões como padrão!");
  }
  else
    puts("sem argumentos, usando 5 conexões como padrão!");
}

void waitForMessage(struct SocketState *socketState)
{
  recv(socketState->connectionFileDescriptor, socketState->buffer, BUFFER_SIZE, 0);
}

void sendMessage(struct SocketState *socketState, char *msgString)
{
  send(socketState->connectionFileDescriptor, msgString, strlen(msgString) + 1, 0);
}

struct SocketState socketState;

void unregisterSocket()
{
  puts("\ndesregistrando o socket...");
  int status = EXIT_SUCCESS;
  if (close(socketState.connectionFileDescriptor) < 0)
  {
    perror("falha ao desregistrar descritor de arquivo da conexão");
    status = EXIT_FAILURE;
  }

  if (close(socketState.socketFileDescriptor) < 0)
  {
    perror("falha ao desregistrar descritor de arquivo do socket");
    status = EXIT_FAILURE;
  }

  if (unlink(socketState.socketAddress.sun_path) < 0)
  {
    perror("falha ao desvincular o arquivo socket");
    status = EXIT_FAILURE;
  }
  if (status == EXIT_SUCCESS)
    puts("socket desregistrado!");
  exit(status);
}

int main(int argc, char *argv[])
{
  /**
   * importante
   *
   * as funções são chamadas em um ordem lógica, no qual não pode ser alterada.
   * todas as etapas na sequência que estão,  são necessárias para preparar o
   * ambiente de socket.
   */

  signal(SIGINT, unregisterSocket);
  setMaxConnetions(argc, argv, &socketState);
  createSocket(&socketState);
  configureSocketAddress(&socketState);
  bindSocket(&socketState);
  listenForConnections(&socketState);
  acceptConnection(&socketState); // código pausa esperando a primeira conexão

  puts("\niniciando loop de comunicação...\n");
  char messages[][BUFFER_SIZE] = {
      "Ola cliente!",
      "Tudo sim! e com você?",
      "Que otimo!",
      "Vamos sim!",
      "Tchau tchau!"};

  // chamar função para desregistrar o socket tanto no signal quanto no erro do waitForMessage ou sendMessage
  size_t len = sizeof messages / sizeof messages[0];
  int i = 0;
  while (true)
  {
    if (i == len)
    {
      putchar('\n');
      i = 0;
    }
    waitForMessage(&socketState);
    puts(socketState.buffer);
    sleep(1);

    sendMessage(&socketState, messages[i]);
    i++;
    sleep(1);
  }
  return EXIT_SUCCESS;
}