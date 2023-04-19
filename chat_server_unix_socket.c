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
#include <pthread.h>    // for pthread_create(), pthread_join()

#define SOCKET_PATH "/tmp/socket_repo"
#define SOCKET_FILE "/tmp/socket_repo/my_unix_socket"
#define BUFFER_SIZE 256
char keyboardInput[BUFFER_SIZE];

struct SocketState
{
  int socketFileDescriptor;
  int connectionFileDescriptor;
  int maxConnections;
  bool stopOperation;
  bool deregistrationExecuted;
  bool fullConnectionEstablished;
  pthread_t messageListenerThread;
  pthread_t userInputListenerThread;
  socklen_t addressSize;
  struct sockaddr_un socketAddress;
  char buffer[BUFFER_SIZE];
};

struct SocketState socketState;

void initSocketState(struct SocketState *socketState)
{
  socketState->socketFileDescriptor = -1;
  socketState->connectionFileDescriptor = -1;
  socketState->maxConnections = 1;
  socketState->stopOperation = false;
  socketState->deregistrationExecuted = false;
  socketState->fullConnectionEstablished = false;
  socketState->messageListenerThread = 0;
  socketState->userInputListenerThread = 0;
  socketState->addressSize = 0;
  memset(socketState->buffer, 0, BUFFER_SIZE);
  memset(socketState->socketAddress.sun_path, 0, sizeof(socketState->socketAddress.sun_path));
}

void unregisterSocket()
{
  if (socketState.deregistrationExecuted == true)
    return;

  puts("\ndesregistrando o socket...");
  bool opSucess = true;
  if (socketState.fullConnectionEstablished == true)
  {
    if (close(socketState.connectionFileDescriptor) < 0)
    {
      perror("falha ao desregistrar descritor de arquivo da conexão");
      opSucess = false;
    }
  }
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
  if (unlink(SOCKET_FILE) < 0)
  {
    perror("falha ao desvincular o arquivo socket, ele provavelmente já não existe");
  }
  puts("socket criado!");
}

void configureSocketAddress(struct SocketState *socketState)
{
  puts("configurando o endereço do socket...");
  // memset(&socketState->socketAddress, 0, sizeof(socketState->socketAddress));
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
  socketState->fullConnectionEstablished = true;
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

void messageListener(struct SocketState *socketState)
{
  ssize_t status = 1;
  while (status != 0 && status != -1 && socketState->stopOperation == false)
  {
    status = recv(socketState->connectionFileDescriptor, socketState->buffer, BUFFER_SIZE, 0);
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
    if (fgets(msgString, BUFFER_SIZE, stdin) == NULL)
    {
      perror("falha ao ler a mensagem do input");
      opSucess == false;
    }
    else if (send(socketState->connectionFileDescriptor, msgString, strlen(msgString) + 1, 0) == -1)
    {
      perror("falha ao enviar a mensagem");
      opSucess == false;
    }
  }
  unregisterSocket();
  // pthread_exit(NULL);
}

int main(int argumentsCounter, char *argumentValues[])
{
  puts("[server]");
  initSocketState(&socketState);
  signal(SIGINT, unregisterSocket);
  setMaxConnetions(argumentsCounter, argumentValues, &socketState);
  createSocket(&socketState);
  configureSocketAddress(&socketState);
  bindSocket(&socketState);
  listenForConnections(&socketState);
  acceptConnection(&socketState); // código pausa esperando a primeira conexão
  sleep(2);
  system("clear");
  puts("[server@localhost]: digite algo, e pressione enter.\n");

  pthread_create(&socketState.messageListenerThread, NULL, (void *)messageListener, (void *)&socketState);
  pthread_create(&socketState.userInputListenerThread, NULL, (void *)userInputListener, (void *)&socketState);

  pthread_join(socketState.messageListenerThread, NULL);
  pthread_join(socketState.userInputListenerThread, NULL);

  return EXIT_SUCCESS;
}