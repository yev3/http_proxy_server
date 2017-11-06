////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// ClientConnection.cpp
// Responsible for establishing a connection to the given address and
// port. Socket is closed on instance destruction.
//
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "ClientConnection.h"

ClientConnection::ClientConnection(const char* hostname, const int port) {
  AddrInfo resolvedAddr{hostname, port};    ///< Resolved DNS entries
  addrinfo* cur;                            ///< Current entry

  while ((cur = resolvedAddr.next()) != nullptr) {
    sockFd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
    if (sockFd != -1) {
      if (connect(sockFd, cur->ai_addr, cur->ai_addrlen)) {
        LOG_ERROR("connecting");
        close(sockFd);
        sockFd = -1;
      } else {
        break;  // successfully connected
      }
    }
  }
}


ClientConnection::~ClientConnection() {
  if (sockFd != -1) {
    close(sockFd);
  }
}

int ClientConnection::fd() const {
  return sockFd;
}

bool ClientConnection::isConnected() const {
  return sockFd != -1;
}