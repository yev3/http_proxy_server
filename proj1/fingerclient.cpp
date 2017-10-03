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
 * \brief Copies characters from one file descriptor to another until EOF
 * \param fdSrc source file descriptor
 * \param fdDst destination file descriptor
 * \return 0 on success, errno otherwise
 */
int copyUntilEOF(const int fdSrc, const int fdDst) {
  char c;
  while (true) {
    switch (read(fdSrc, &c, 1)) {
    case -1:
      if (errno != EINTR) return -1;  // return an error unless interrupted
      break;
    case 0:
      return 0; // EOF
    default:
      write(fdDst, &c, 1);
    }
  }
}

void errorExit(const char *msg) {
  perror(msg);
  exit(errno);
}


int main(const int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: fingerclient user@host:port" << std::endl;
    exit(-1);
  }

  std::string username;
  std::string hostname;
  int portNo = -1;

  std::stringstream argStrm{ argv[1] };
  std::getline(argStrm, username, '@');
  std::getline(argStrm, hostname, ':');
  argStrm >> portNo;
  std::string portNoStr = std::to_string(portNo);

  std::cout << "Username: " << username << std::endl;
  std::cout << "Hostname: " << hostname << std::endl;
  std::cout << "Port no : " << portNo << std::endl;

  // GETADDRINFO

  addrinfo hints = { 0 };
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_NUMERICSERV;  // port no is numeric
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  addrinfo *addrInfoResult;
  if (0 != getaddrinfo(hostname.c_str(), portNoStr.c_str(), &hints, &addrInfoResult)) {
    perror("getaddrinfo");
    exit(-1);
  }

  //std::cout << "getaddrinfo() results: " << std::endl;
  //std::cout << "---Result---" << std::endl;
  //std::cout << "ai_addr: \n" << *r->ai_addr << std::endl;
  bool sockConnected = false;
  int sockfd;
  addrinfo *curAddr;
  for (curAddr = addrInfoResult; !sockConnected && curAddr != nullptr; curAddr = curAddr->ai_next) {
    sockfd = socket(curAddr->ai_family, curAddr->ai_socktype, curAddr->ai_protocol);
    if (sockfd != -1) {
      if (connect(sockfd, curAddr->ai_addr, curAddr->ai_addrlen) != -1) {
        sockConnected = true;
      } else {
        close(sockfd);
      }
    }
  }

  if (!sockConnected) {
    std::cerr << "Could not create a socket." << std::endl;
    exit(-1);
  }

  freeaddrinfo(addrInfoResult);

  write(sockfd, username.c_str(), username.size() + 1);

  int copyResult = copyUntilEOF(sockfd, STDOUT_FILENO);
  close(sockfd);

  if (copyResult != 0) {
    perror("Copying socket: ");
    exit(copyResult);
  }

  std::cout << "Finished, exiting..." << std::endl;

  return 0;
}
