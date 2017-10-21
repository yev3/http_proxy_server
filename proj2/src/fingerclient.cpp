////////////////////////////////////////////////////////////////////////////////
// Project 1, CPSC 5510 Networking, Seattle University
// Yevgeni Kamenski
// 
// fingerclient.cpp - 09/30/2017 7:53 PM
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


/**
 * \brief Exits the program, displaying the error string
 * \param msg message to display
 */
void errorExit(const char *msg) {
  perror(msg);
  exit(errno);
}

/**
 * \brief Holds user's argument information
 */
class FingerArguments {
public:
  std::string username; ///< username from the command line
  std::string hostname; ///< hostname from the command line
  int portNo = -1; ///< port number

  /**
   * \brief parses the connection string and initializes the argument values
   * \param argc argument count
   * \param argv c-style string from args
   */
  FingerArguments(const int argc, const char *argv[]) {
    if (argc == 2) {
      std::stringstream argStrm{ argv[1] };
      std::getline(argStrm, username, '@');
      std::getline(argStrm, hostname, ':');
      argStrm >> portNo;
    }
  }

  /**
   * \brief Returns true when the user-supplied arguments are valid
   * \return true if the user-supplied arguments are valid
   */
  bool isValid() const {
    return !(username.empty() || hostname.empty() || portNo == -1);
  }
};

class ClientAddrInfo {
public:
  ClientAddrInfo(const std::string &hostname, const int port) {
    addrinfo hints = { 0 };
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICSERV; // port no is numeric
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    if (0 != getaddrinfo(hostname.c_str(),
      std::to_string(port).c_str(),
      &hints,
      &head)) {
      errorExit("getaddrinfo");
    }
    nextVal = head;
  }

  ~ClientAddrInfo() {
    freeaddrinfo(head);
  }

  addrinfo* next() {
    addrinfo *curr = nextVal;
    if (nextVal != nullptr)
      nextVal = nextVal->ai_next;
    return curr;
  }

private:
  addrinfo *head = nullptr;
  addrinfo *nextVal = nullptr;
};

class ClientConnection {
public:
  ClientConnection(const std::string &hostname, const int port) {
    ClientAddrInfo resolvedAddr{ hostname, port };
    addrinfo *cur;

    while ((cur = resolvedAddr.next()) != nullptr) {
      sockFd = socket(cur->ai_family, cur->ai_socktype,
        cur->ai_protocol);
      if (sockFd != -1) {
        if (connect(sockFd, cur->ai_addr, cur->ai_addrlen) != -1) {
          return;
        }
        close(sockFd);
        sockFd = -1;
      }
    }
  }

  ~ClientConnection() {
    if (sockFd != -1) {
      close(sockFd);
    }
  }

  int fd() const {
    return sockFd;
  }

  bool isConnected() const {
    return sockFd != -1;
  }

private:
  int sockFd = -1; ///< socket file descriptor
};

/**
 * \brief Copies characters from one file descriptor to another until EOF
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

int main(const int argc, const char *argv[]) {
  FingerArguments userArgs{ argc, argv };
  if (!userArgs.isValid()) {
    std::cout << "usage: fingerclient user@host:port" << std::endl;
    exit(-1);
  }

  ClientConnection conn{ userArgs.hostname, userArgs.portNo };

  if (!conn.isConnected()) {
    errorExit("cannot connect to socket");
  }

  write(conn.fd(), userArgs.username.c_str(), userArgs.username.size() + 1);

  if (copyUntilCrLn(conn.fd(), STDOUT_FILENO) != 0) {
    errorExit("copying socket to stdout");
  }

  return 0;
}
