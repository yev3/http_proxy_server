////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// ListenConnection.h
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "ListenConnection.h"

ListenConnection::ListenConnection(const int port) {
  AddrInfo resolvedAddr{port};
  int optVal = 1;
  addrinfo *cur;

  while ((cur = resolvedAddr.next()) != nullptr) {
    listenFd = socket(cur->ai_family, cur->ai_socktype,
                      cur->ai_protocol);
    if (listenFd != -1) {
      if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optVal,
                     sizeof(optVal)) != -1) {
        if (bind(listenFd, cur->ai_addr, cur->ai_addrlen) != -1) {
          break;
        }
      } else {
        LOG_ERROR("setsockopt");
      }
      close(listenFd);
      listenFd = -1;
    }
  }

  // start listening
  if (listen(listenFd, BACKLOG) == -1) {
    listenFd = -1;
    LOG_ERROR("listen");
  }
}

ListenConnection::~ListenConnection() {
  if (listenFd != -1) {
    close(listenFd);
  }
}

int ListenConnection::fd() const {
  return listenFd;
}

bool ListenConnection::isConnected() const {
  return listenFd != -1;
}
