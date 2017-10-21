////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// 
// proxy.cpp
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
#include <vector>

/**
 * \brief exits the program displaying proper usage
 */
void exitUsage() {
  std::cout << "usage: proxy port" << std::endl;
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
 * \brief Helper for resolving addresses for the server
 */
class ServerAddrInfo {
public:
  /**
   * \brief constructs a linked list of addrinfo structures
   * \param port port number to listen to
   */
  ServerAddrInfo(const int port) {
    addrinfo hints = { 0 };
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
    ServerAddrInfo resolvedAddr{ port };
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

/**
 * \brief reads a line
 * \param fd file descriptor to read from
 * \param buf buffer to store
 * \param size buffer size
 * \return number of chars actually read
 */
int readLine(const int fd, char* buf, int size)
{
  int numRead = 0;   ///< how many were read
  char c;
  for (;;) {
    int r = read(fd, &c, 1);
    if (r == -1)
    {
      if (errno != EINTR) return -1; // return an error unless interrupted
    } else if (r == 0)
    {
      break;
    } else {
      if ('\n' == c) break;
      *buf++ = c;
      ++numRead;
    }
    // when used up the entire buffer
    if (numRead == (size - 1)) break;
  }

  if (0 != numRead)
  {
    *buf = '\0';
  }

  return numRead;
}


/**
 * \brief Copies LINES from one file descriptor to another until CRCL
 * \param fdSrc source file descriptor
 * \param fdDst destination file descriptor
 * \return 0 on success, errno otherwise
 */
int copyLines(const int fdSrc, const int fdDst) {
  static const int bufSize = 256;
  static char buf[256];
  bool lastLineFull = false;
  static char nl = '\n';
  while (true) {
    int lineSize = readLine(fdSrc, buf, bufSize);
    if (lineSize >= 0) {
      write(fdDst, buf, lineSize);
      write(fdDst, &nl, 1);
    } else {
      return lineSize;  // error occurred
    }
    if (!lastLineFull)
    {
      if (lineSize == 1 && buf[0] == '\r') return 0;
    }
    lastLineFull = lineSize == 255;
  }
}

bool isBlankNewline(const char* buf, int size)
{
  return size == 1 && buf[0] == '\r';

}

/**
 * \brief returns a collection of lines from user's request
 * \param fd socket file descriptor
 * \return header lines
 */
std::vector<std::string> getHeaderLines(const int fd)
{
  std::vector<std::string> lines;
  std::stringstream line;
  static const int bufSize = 256;
  static char buf[bufSize];

  while (true) {
    int lineSize = readLine(fd, buf, bufSize);
    if (lineSize < 0) break;    // error occurred
    if (isBlankNewline(buf, lineSize)) break;  // blank newline
    lines.emplace_back(buf, lineSize);
  }
  return lines;
}

/**
 * \brief Copies characters from one file descriptor to another until CRCL
 * \param fdSrc source file descriptor
 * \param fdDst destination file descriptor
 * \return 0 on success, errno otherwise
 */
int copyUntilCrLn(const int fdSrc, const int fdDst) {
  char c;
  while (true) {
    switch (read(fdSrc, &c, 1)) {
    case -1:
      if (errno != EINTR) return -1; // return an error unless interrupted
      break;
    case 0:
      return 0; // EOF
    default:
      write(fdDst, &c, 1);
    }
  }
}

/**
 * \brief processes a client connection
 * \param clientFd connected client socket file descriptor
 */
void handleClient(const int clientFd) {

  // receive username string from user
  char buf[256] = { 0 }; // typically max username is 71, but saw 256 also
  read(clientFd, buf, 255);

  // strip any newlines
  std::string userName;
  std::stringstream strm{ buf };
  std::getline(strm, userName);

  dup2(clientFd, STDOUT_FILENO);
  dup2(clientFd, STDERR_FILENO);
  execlp("finger", "finger", userName.c_str(), 0);
  errorExit("execlp"); // should not get here
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: proxy port" << std::endl;
    exit(-1);
  }

  int portNo;
  (std::stringstream{ argv[1] }) >> portNo;

  ServerAddrInfo resolvedAddr{ portNo };

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

    // TODO: make this multi-client
    //switch (fork()) {
    //case 0: // child
    //  close(conn.fd());
    //  handleClient(clientFd);
    //case -1: // parent
    //  errorLog("fork");
    //default: // parent
    //  break;
    //}

    std::cout << "\nTrace - client connected.." << std::endl;

    std::vector<std::string> header = getHeaderLines(clientFd);
    for (size_t i = 0; i < header.size(); ++i)
    {
      std::cout << "header[" << i << "]: " << header[i] << std::endl;
    }

    //if (copyLines(clientFd, STDOUT_FILENO) != 0) {
    //  errorExit("copying socket to stdout");
    //}
    std::cout << "\nTrace - found CRLN, client done." << std::endl;

    close(clientFd); // parent does not need the client fd
  }
}
