// Project 1: A Simple Client/Server Warmup Project
// David Pierce
// CPSC 5510
// fingerserver.cpp

// revisions from grade feedback

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static const int BUFFER_SIZE = 512;

// safe port range: 10180 to 10189
static const int CLASS_ROSTER = 19;     // current class roster pos. (10/2)
static const int MAX_CONNECTIONS = 9;   // maximum # of clients at once
static const int MIN_PORT = 10000 + (CLASS_ROSTER-1)* 10;
static const int LISTEN_PORT = MIN_PORT + 2;
static const int MAX_PORT = 10000 + CLASS_ROSTER * 10 - 1;
int currentPort = LISTEN_PORT;

void error(std::string message) {
  fprintf(stderr, "%s", message.c_str());
  exit(EXIT_FAILURE);
}

// sendMessage()
// takes socket fd and any size string message and sends message
void sendMessage(int sockfd, std::string message)
{
  send(sockfd, message.c_str(), message.length(),0);
}

// receiveMessage()
// receives a message sent by sendMessage from specified socket fd
// returns string sent through socket
std::string receiveMessage(int sockfd)
{
  // create the buffer with space for the data
  std::vector<char> buffer(BUFFER_SIZE);
  std::string message;
  int bytesReceived = 0;
  do {
      bytesReceived = recv(sockfd, buffer.data(), buffer.size(), 0);
      // append string from buffer.
      if ( bytesReceived == -1 )
      {
        // error, try again
      }
      else
      {
        message.append(buffer.cbegin(), buffer.cend());
      }
  }
  while ( bytesReceived == BUFFER_SIZE );

  return message;
}

// newPort()
// generates a new port within safe range (10180 to 10189)
int newPort() {
  // too many prior connections, use an old port
  if(currentPort == MAX_PORT) {
    currentPort = LISTEN_PORT;
  }
  // iterate by 1
  currentPort++;
  return currentPort;
}

// serveClientFinger()
// forks out a parent process for finger, with a child process to send finger
// output to client
int serveClientFinger(int sockfd, char *username) {
  int pid;
  std::string fingerOutput;
  std::vector<char> buffer(BUFFER_SIZE);
  int bytesReceived = 0;
  
  // open a pipe
  int pipefd[2];
  pipe(pipefd);
  
  // fork
  switch (pid = fork()) {
    
    // child
    case 0:
      // close reading side of pipe
      close(pipefd[0]);
      
      // send standard output to pipe
      dup2(pipefd[1], 1);
      
      execl("/usr/bin/finger", "finger", username, (char *)0);
      break;
    
    // parent
    default:
      // close the write end of the pipe in the parent
      close(pipefd[1]);
      
      // make a string from pipe from child
      do {
        bytesReceived = read(pipefd[0], buffer.data(), buffer.size());
        // append string from buffer.
        if ( bytesReceived == -1 ) {
          // error, try again
        } else {
          fingerOutput.append(buffer.cbegin(), buffer.cend());
        }
      }
      while ( bytesReceived == BUFFER_SIZE );
      sendMessage(sockfd, fingerOutput);
      
      close(sockfd);
      break;
    
    // error
    case -1:
      error("Error running finger\n");
      close(sockfd);
      exit(1);
  } // end switch()

  return 0;
}

// main()
// open socket, listen at port 
int main() {
  int sockfd, newsockfd, clilen;  // network stack variables
  struct sockaddr_in serv_addr, cli_addr;

  int numOfClients = 0; // current count of clients connected
  int pid, status;      // process id and status indicator for client fork

  // open socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    error("Error opening socket!");
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));

  // write to the sockaddr_in structure
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(currentPort);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  // bind socket to address
  while(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
    error("ERROR on binding\n");
  }

  // listen for a client
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  printf("Opening a listening port on %d...\n", LISTEN_PORT);

  while(1) {

    // accept client connection, create new socket, create new fork
    while ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen)) > 0) {
      std::string strUser = receiveMessage(newsockfd);
      char * username = &strUser[0];

      // fork new client
      switch (pid = fork()) {
        
        // child
        case 0:
          printf("Client connected, requesting %s.\n", username);
          // run finger for the client process;
          serveClientFinger(newsockfd, username);
          exit(0);
        
        // parent
        default:
          // wait for child to end
          while ((pid = wait(&status)) != -1) {
            //fprintf(stderr, "Client on process %d exits with %d\n", pid, WEXITSTATUS(status));
          }
          break;
        
        // error
        case -1:
          error("Error forking new client connection\n");
          close(newsockfd);
          exit(1);
      } // end switch()

      close(newsockfd);
    
    } // end new client socket
  } // end listening while()

  // close socket before ending
  close(sockfd);
  return 0;
}