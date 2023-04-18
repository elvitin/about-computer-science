#include <stdio.h>      // for puts, perror
#include <stdlib.h>     // for EXIT_SUCCESS, EXIT_FAILURE
#include <stdbool.h>    // for bool, true, false
#include <string.h>     // for memset, strcpy
#include <signal.h>     // for signal, SIGINT
#include <unistd.h>     // for close, unlink
#include <sys/un.h>     // for sockaddr_un
#include <sys/socket.h> // for socket, AF_UNIX, SOCK_STREAM, socklen_t
#include <sys/stat.h>   // for S_IRWXU, S_IRWXG, S_IROTH, S_IXOTH

#define SOCKET_PATH "/tmp/socket_repo"
#define SOCKET_FILE "/tmp/socket_repo/my_unix_socket"
#define BUFFER_SIZE 256

struct SocketState
{
  int socketFileDescriptor;
  socklen_t addressSize;
  struct sockaddr_un socketAddress;
  char buffer[BUFFER_SIZE];
};

struct SocketState socketState;

void unregisterSocket()
{
  puts("\ndesregistrando o socket...");
  int status = EXIT_SUCCESS;

  if (close(socketState.socketFileDescriptor) < 0)
  {
    perror("falha ao desregistrar descritor de arquivo do socket");
    status = EXIT_FAILURE;
  }

  if (unlink(socketState.socketAddress.sun_path) < 0)
  {
    perror("falha ao desvincular o arquivo socket, provavelmente já foi desvinculado pelo outro processo");
    status = EXIT_FAILURE;
  }
  if (status == EXIT_SUCCESS)
    puts("socket desregistrado!");
  exit(status);
}

void createSocket(struct SocketState *socketState)
{
  puts("criando o socket unix domain...");
  socketState->socketFileDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socketState->socketFileDescriptor < 0)
  {
    perror("falha ao criar o socket");
    exit(EXIT_FAILURE);
  }
  puts("socket criado!");
}

void configureSocketAddress(struct SocketState *socketState)
{
  puts("configurando o endereço do socket...");
  memset(&socketState->socketAddress, 0, sizeof(socketState->socketAddress));
  socketState->socketAddress.sun_family = AF_UNIX;
  mkdir(SOCKET_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  strcpy(socketState->socketAddress.sun_path, SOCKET_FILE);
  socketState->addressSize = sizeof(socketState->socketAddress);
}

void establishConnection(struct SocketState *socketState)
{
  puts("estabelecendo conexão...");
  if (connect(socketState->socketFileDescriptor, (struct sockaddr *)&socketState->socketAddress, socketState->addressSize) < 0)
  {
    perror("falha ao estabelecer conexão");
    sleep(3);
    puts("tentando novamente...");
    establishConnection(socketState);
    // exit(EXIT_FAILURE);
  }
  else
    puts("conexão estabelecida!");
}

void sendMessage(struct SocketState *socketState, char *msgString)
{
  if (send(socketState->socketFileDescriptor, msgString, strlen(msgString) + 1, 0) == -1)
  {
    perror("falha ao enviar mensagem");
    unregisterSocket();
  }
}

void waitForMessage(struct SocketState *socketState)
{
  if (recv(socketState->socketFileDescriptor, socketState->buffer, BUFFER_SIZE, 0) == -1)
  {
    perror("falha ao receber mensagem");
    unregisterSocket();
  }
}

int main(void)
{
  signal(SIGINT, unregisterSocket);
  createSocket(&socketState);
  configureSocketAddress(&socketState);
  establishConnection(&socketState);

  puts("\niniciando loop de comunicação...\n");

  char messages[][BUFFER_SIZE] = {
      "Ola servidor?",
      "Tudo bem com voce?",
      "Comigo esta tudo bem!",
      "Vamos iniciar novamente?",
      "Tchauzinho!"};

  size_t len = sizeof messages / sizeof messages[0];
  int i = 0;
  while (true)
  {
    if (i == len)
    {
      putchar('\n');
      i = 0;
    }

    sendMessage(&socketState, messages[i]);
    i++;
    waitForMessage(&socketState);
    puts(socketState.buffer);
  }

  return EXIT_SUCCESS;
}