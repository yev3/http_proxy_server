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
#include <sys/wait.h>
#include <cstring>

/**
 * \brief displays an error message with a message string
 * \param msg Message to display when exiting
 */
void errorExit(const char *msg) {
  perror(msg);
  exit(errno);
}

/**
 * \brief Displays an errno string and message
 * \param msg Message to display to user 
 */
void errorLog(const char *msg) {
  std::cerr << msg << ": " << strerror(errno) << std::endl;
}

/**
 * \brief reaps the zombie children
 * \param sig unused argument
 */
void processZombieChildren(int sig) {
  int savedErr = errno;
  while (waitpid(-1, nullptr, WNOHANG) > 0) {}
  errno = savedErr;

}

void handleClient(int fdClient) {
  char buf[256] = {0};
  read(fdClient, buf, 255);

  std::string userName;
  std::stringstream strm{buf};
  std::getline(strm, userName);

  std::cout << "Received: " << userName << std::endl;

  dup2(fdClient, STDOUT_FILENO);
  dup2(fdClient, STDERR_FILENO);
  execlp("finger", "finger", userName.c_str(), 0);
  errorExit("execlp");
}

addrinfo* getAddrFromPort(int portNo) {
  addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  addrinfo *addrInfo;
  if (0 != getaddrinfo(nullptr,
                       std::to_string(portNo).c_str(),
                       &hints,
                       &addrInfo)) {
    errorExit("getAddrFromPort");
  }
  return addrInfo;
}

void setupGrimReaper() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = processZombieChildren;
  if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
    errorExit("sigaction");
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: fingerserver port" << std::endl;
    exit(-1);
  }

  std::stringstream strm{argv[1]};
  int portNo;
  strm >> portNo;

  addrinfo *addr = getAddrFromPort(portNo);

  // ignore SIGPIPEs
  signal(SIGPIPE, SIG_IGN);

  // bind the socket
  int fdListen;
  addrinfo *curAddr;
  int optVal = 1;
  for (curAddr = addr; curAddr != nullptr; curAddr = curAddr->ai_next) {
    fdListen = socket(curAddr->ai_family,
                      curAddr->ai_socktype,
                      curAddr->ai_protocol);

    if (fdListen != -1) {

      if (setsockopt(fdListen, SOL_SOCKET, SO_REUSEADDR, &optVal,
                     sizeof optVal) == -1) {
        errorExit("setsockopt");
      }

      if (-1 == bind(fdListen, curAddr->ai_addr, curAddr->ai_addrlen)) {
        close(fdListen);
        continue;
      }

      break;
    }
  }

  freeaddrinfo(addr);

  if (curAddr == nullptr) errorExit("Could not bind a socket");

  // make the socket listen
  const int BACKLOG = 50;
  if (listen(fdListen, BACKLOG) == -1) errorExit("listen");

  // ensure we don't get zombie processes
  setupGrimReaper();

  while (true) {
    int fdClient = accept(fdListen, nullptr, nullptr);
    if (fdClient == -1) {
      errorLog("error accepting");
      continue;
    }

    switch (fork()) {
    case -1:  // parent
      errorLog("fork");
      break;
    case 0:   // child
      close(fdListen);
      handleClient(fdClient);
    default:  // parent
      break;
    }

    // parent does not need the client fd
    close(fdClient); 
  }
}
