#include "ClientConnection.h"

ClientConnection::ClientConnection(const char* hostname, const int port) {
  AddrInfo resolvedAddr{hostname, port};
  addrinfo* cur;

  while ((cur = resolvedAddr.next()) != nullptr) {
    sockFd = socket(cur->ai_family, cur->ai_socktype,
                    cur->ai_protocol);
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