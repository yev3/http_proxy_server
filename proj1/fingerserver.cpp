////////////////////////////////////////////////////////////////////////////////
// Project1, CPSC 5510 Networking, Seattle University
// Yevgeni Kamenski
// 
// fingerserver.cpp - 09/26/2017 3:51 PM
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sstream>

void execCommand(std::string &line);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Program must be called with one argument." << std::endl;
    exit(-1);
  }

  std::stringstream strm{argv[1]};
  int portNo;
  strm >> portNo;

  std::cout << "Attempting to attach to port: " << portNo << ".." << std::flush;

  // GETADDRINFO
  addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  // ignore SIGPIPEs
  signal(SIGPIPE, SIG_IGN);

  // getaddrinfo
  addrinfo *addrInfoResult;
  if (0 != getaddrinfo(nullptr, std::to_string(portNo).c_str(), &hints,
                       &addrInfoResult)) {
    perror("getaddrinfo");
    exit(-1);
  }

  // bind the socket
  bool sockBound = false;
  int fdListen;
  addrinfo *curAddr;
  int optVal = 1;
  for (curAddr = addrInfoResult; !sockBound && curAddr != nullptr;
       curAddr = curAddr->ai_next) {
    fdListen = socket(curAddr->ai_family, curAddr->ai_socktype,
                      curAddr->ai_protocol);

    if (fdListen == -1) continue;

    if (setsockopt(fdListen, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == -1) {
      
    }

      if (bind(fdListen, curAddr->ai_addr, curAddr->ai_addrlen) != -1) {
        sockBound = true;
      } else {
        close(fdListen);
      }
  }

  if (!sockBound) {
    std::cerr << "Could not bind a socket." << std::endl;
    exit(-1);
  }

  // make the socket listen
  const int BACKLOG = 50;
  if (listen(fdListen, BACKLOG) == -1) {
    perror("Socket listen");
    exit(errno);
  }

  freeaddrinfo(addrInfoResult);

  sockaddr_storage clientAddress;
  socklen_t clientAddressLen = sizeof(sockaddr_storage);
  int fdClient = accept(fdListen, reinterpret_cast<sockaddr*>(&clientAddress),
                        &clientAddressLen);

  while (true) {

    std::cout << "\nEnter username: " << std::flush;
    std::string line;
    std::getline(std::cin, line);

    if (!line.empty() && std::tolower(line[0]) == 'q') {
      exit(0);
    }

    execCommand(line);

    std::cout << "You entered: " << line << std::endl;
  }

}

void execCommand(std::string &line) {
  pid_t child = fork();

  std::cout << "About to finger.." << std::endl;
  switch (child) {
  case -1:
    std::cout << "fork error" << std::endl;
    exit(-1);
  case 0:
    // CHILD
    execlp("finger", "finger", line.c_str(), 0);
    perror("Should not have returned from execlp()..");
    //std::cout << "Error: should not have returned from execlp().." << std::endl;
    _exit(-1);
  default:
    // PARENT
    std::cout << "Parent returns.." << std::endl;
    break;
  }

}
