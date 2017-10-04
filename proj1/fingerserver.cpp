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
 * \brief exits the program displaying proper usage
 */
void exitUsage() {
  std::cout << "usage: fingerserver port" << std::endl;
  exit(-1);
}

/**
 * \brief Exits the program, displaying the error string
 * \param msg message to display
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

/**
 * \brief Sets up to reap zombie fork children
 */
void setupGrimReaper() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = processZombieChildren;
  if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
    errorExit("sigaction");
  }
}

/**
 * \brief processes a client connection
 * \param clientFd connected client socket file descriptor
 */
void handleClient(const int clientFd) {
  // receive username string from user
  char buf[256] = {0}; // typically max username is 71, but saw 256 also
  read(clientFd, buf, 255);

  // strip any newlines
  std::string userName;
  std::stringstream strm{buf};
  std::getline(strm, userName);

  dup2(clientFd, STDOUT_FILENO);
  dup2(clientFd, STDERR_FILENO);
  execlp("finger", "finger", userName.c_str(), 0);
  errorExit("execlp"); // should not get here
}

/**
 * \brief Helper for resolving addresses for the server
 */
class ServerAddrInfo {
public:
  /**
   * \brief constructs a linked list of addrinfo structures
   * \param port port number to listen to
   */
  ServerAddrInfo(const int port) {
    addrinfo hints = {0};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    if (0 != getaddrinfo(nullptr,
                         std::to_string(port).c_str(),
                         &hints,
                         &head)) {
      errorExit("getaddrinfo");
    }
    nextVal = head;
  }

  /**
   * \brief Frees up memory of addrinfo structures
   */
  ~ServerAddrInfo() {
    freeaddrinfo(head);
  }

  /**
   * \brief returns the next addrinfo structure or nullptr if none
   * \return next addrinfo structure in the linked list or nullptr if none
   */
  addrinfo* next() {
    addrinfo *curr = nextVal;
    if (nextVal != nullptr)
      nextVal = nextVal->ai_next;
    return curr;
  }

private:
  addrinfo *head = nullptr;     ///< head of the linked list
  addrinfo *nextVal = nullptr;  ///< current node
};

/**
 * \brief Helper for creating a listening socket for the server
 */
class ServerConnection {
public:
  /**
   * \brief creates a helper for binding sockets
   * \param port port number to bind the socket to
   */
  ServerConnection(const int port) {
    ServerAddrInfo resolvedAddr{port};
    int optVal = 1;
    addrinfo *cur;

    while ((cur = resolvedAddr.next()) != nullptr) {
      listenFd = socket(cur->ai_family, cur->ai_socktype,
                        cur->ai_protocol);
      if (listenFd != -1) {

        if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optVal,
                       sizeof optVal) == -1) {
          errorExit("setsockopt");
        }

        if (bind(listenFd, cur->ai_addr, cur->ai_addrlen) != -1) {
          return;
        }
        close(listenFd);
        listenFd = -1;
      }
    }
  }

  /**
   * \brief closes any active listening sockets
   */
  ~ServerConnection() {
    if (listenFd != -1) {
      close(listenFd);
    }
  }

  /**
   * \brief returns the listening socket or -1 if not bound
   * \return listening socket file descriptor or -1 if not bound
   */
  int fd() const {
    return listenFd;
  }

  /**
   * \brief returns true if socket is bound
   * \return true if the socket is bound and ready to accept connections
   */
  bool isConnected() const {
    return listenFd != -1;
  }

private:
  int listenFd = -1; ///< socket file descriptor
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: fingerserver port" << std::endl;
    exit(-1);
  }

  int portNo;
  (std::stringstream{argv[1]}) >> portNo;

  ServerAddrInfo resolvedAddr{portNo};

  // ignore SIGPIPEs
  signal(SIGPIPE, SIG_IGN);

  // ensure we don't get zombie processes
  setupGrimReaper();

  ServerConnection conn{ portNo };
  if (!conn.isConnected())
    errorExit("Could not bind a socket");

  const int BACKLOG = 50;
  if (listen(conn.fd(), BACKLOG) == -1) errorExit("listen");

  std::cout << "Listening for clients on port " << portNo <<
    ", use <CTRL>-C to quit." << std::endl;

  while (true) {
    const int clientFd = accept(conn.fd(), nullptr, nullptr);
    if (clientFd == -1) {
      errorLog("accept");
      continue;
    }

    switch (fork()) {
    case 0: // child
      close(conn.fd());
      handleClient(clientFd);
    case -1: // parent
      errorLog("fork");
    default: // parent
      break;
    }
    close(clientFd); // parent does not need the client fd
  }
}
