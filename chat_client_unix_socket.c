#include <stdio.h>      // for puts, perror
#include <stdlib.h>     // for EXIT_SUCCESS, EXIT_FAILURE
#include <stdbool.h>    // for bool, true, false
#include <string.h>     // for memset, strcpy
#include <signal.h>     // for signal, SIGINT
#include <unistd.h>     // for close, unlink
#include <sys/un.h>     // for sockaddr_un
#include <sys/socket.h> // for socket, AF_UNIX, SOCK_STREAM, socklen_t
#include <sys/stat.h>   // for S_IRWXU, S_IRWXG, S_IROTH, S_IXOTH
#include <pthread.h>    // for pthread_create, pthread_join

#define SOCKET_PATH "/tmp/socket_repo"
#define SOCKET_FILE "/tmp/socket_repo/my_unix_socket"
#define BUFFER_SIZE 256
char keyboardInput[BUFFER_SIZE];

struct SocketState
{
  int socketFileDescriptor;
  socklen_t addressSize;
  pthread_t messageListenerThread;
  pthread_t userInputListenerThread;
  bool stopOperation;
  bool deregistrationExecuted;
  bool fullConnectionEstablished;
  struct sockaddr_un socketAddress;
  char buffer[BUFFER_SIZE];
};

struct SocketState socketState;

void initSocketState(struct SocketState *socketState)
{
  socketState->socketFileDescriptor = -1;
  socketState->addressSize = 0;
  socketState->stopOperation = false;
  socketState->deregistrationExecuted = false;
  socketState->fullConnectionEstablished = false;
  socketState->messageListenerThread = 0;
  socketState->userInputListenerThread = 0;
  memset(socketState->buffer, 0, BUFFER_SIZE);
  memset(socketState->socketAddress.sun_path, 0, sizeof(socketState->socketAddress.sun_path));
}

void unregisterSocket()
{
  if (socketState.deregistrationExecuted == true)
    return;

  puts("\ndesregistrando o socket...");
  bool opSucess = true;
  if (close(socketState.socketFileDescriptor) < 0)
  {
    perror("falha ao desregistrar descritor de arquivo do socket");
    opSucess = false;
  }
  if (unlink(socketState.socketAddress.sun_path) < 0)
  {
    perror("falha ao desvincular o arquivo socket, provavelmente já foi desvinculado pelo outro processo");
    opSucess = false;
  }
  socketState.deregistrationExecuted = true;
  socketState.stopOperation = true;
  puts(opSucess ? "socket desregistrado!" : "socket desregistrado, porem algumas etapas falharam");
  if (socketState.fullConnectionEstablished == true)
  {
    pthread_kill(socketState.messageListenerThread, SIGKILL);
    pthread_kill(socketState.userInputListenerThread, SIGKILL);
  }
  else if (socketState.fullConnectionEstablished == false)
  {
    kill(getpid(), SIGKILL);
  }
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
    sleep(2);
    puts("tentando novamente...");
    if (socketState->stopOperation == false)
      establishConnection(socketState);
  }
  else
  {
    socketState->fullConnectionEstablished = true;
    puts("conexão estabelecida!");
  }
}

void messageListener(struct SocketState *socketState)
{
  ssize_t status = 1;
  while (status != 0 && status != -1 && socketState->stopOperation == false)
  {
    status = recv(socketState->socketFileDescriptor, socketState->buffer, BUFFER_SIZE, 0);
    if (status == -1)
      perror("falha ao receber a mensagem");
    else if (status == 0)
      perror("conexão encerrada");
    else
      printf("%s", socketState->buffer);
  }
  unregisterSocket();
  // pthread_exit(NULL);
}

void userInputListener(struct SocketState *socketState)
{
  char msgString[BUFFER_SIZE];
  bool opSucess = true;
  while (opSucess && socketState->stopOperation == false)
  {
    memset(msgString, 0, BUFFER_SIZE);
    memset(keyboardInput, 0, BUFFER_SIZE);
    setbuf(stdin, keyboardInput);
    // pesquisar como cancelar fgets
    if (fgets(msgString, BUFFER_SIZE, stdin) == NULL)
    {
      perror("falha ao ler a mensagem do input");
      opSucess = false;
    }
    else if (send(socketState->socketFileDescriptor, msgString, strlen(msgString) + 1, 0) == -1)
    {
      perror("falha ao enviar mensagem");
      opSucess = false;
    }
  }
  unregisterSocket();
  pthread_exit(NULL);
}

int main(void)
{
  puts("[client]");
  initSocketState(&socketState);
  signal(SIGINT, unregisterSocket);
  createSocket(&socketState);
  configureSocketAddress(&socketState);
  establishConnection(&socketState);
  sleep(2);
  system("clear");
  puts("[client@localhost]: digite algo, e pressione enter.\n");

  pthread_create(&socketState.messageListenerThread, NULL, (void *)messageListener, (void *)&socketState);
  pthread_create(&socketState.userInputListenerThread, NULL, (void *)userInputListener, (void *)&socketState);

  pthread_join(socketState.messageListenerThread, NULL);
  pthread_join(socketState.userInputListenerThread, NULL);
  return EXIT_SUCCESS;
}