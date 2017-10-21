// Project 1: A Simple Client/Server Warmup Project
// David Pierce
// CPSC 5510
// fingerclient.cpp

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static const int MAX_PORT = 65535;
static const int BUFFER_SIZE = 32;

void error(std::string message) {
  fprintf(stderr, "%s", message.c_str());
  exit(EXIT_FAILURE);
}

// sendMessage
// takes socket fd and any size string message and sends message
void sendMessage(int sockfd, std::string message)
{
  send(sockfd, message.c_str(), message.length(),0);
}

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

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  char *userHostPort;     // username@hostname:server_port
  char *username;         // user to use finger on
  char *hostname;         // host to connect to
  char *charServerPort;   // port to connect to
  int intPort;            // port converted to int

  // exit with failure if no argument is provided
  if(argc < 2) {
    error("Please provide a username, hostname, and server port, formatted like:\n    fingerclient username@hostname:server port\n\n");
  }
  // use argument 2 for user/host/port
  userHostPort = argv[1];

  // parse user/host/port for valid data, exit with failure if data is missing
  username = strtok(userHostPort,"@:");
  hostname = strtok(NULL, "@:");
  if(hostname == NULL) {
    error("Please provide a hostname in the second argument, formatted like:\n    fingerclient username@hostname:server port\n\n");
  }
  charServerPort = strtok(NULL, "@:");
  intPort = atoi(charServerPort);
  if(charServerPort == NULL || intPort <= 0 || intPort > MAX_PORT) {
    error("Please provide a server port in the second argument, formatted like:\n    fingerclient username@hostname:server port\n\n");
  }
  
  // network stack
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    error("Error: failed to open socket\n");
  }
  server = gethostbyname(hostname);

  if (server == NULL)
  {
    error("Error: host not found\n");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
       (char *)&serv_addr.sin_addr.s_addr,
       server->h_length);
  serv_addr.sin_port = htons(intPort);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
  {
    error("Error: failed to connect to host\n");
  }

  // send username to server
  sendMessage(sockfd, username);

  // print response recieved from server
  std::string fingerResponse = receiveMessage(sockfd);
  std::cout << fingerResponse << std::endl;

  return 0;
}