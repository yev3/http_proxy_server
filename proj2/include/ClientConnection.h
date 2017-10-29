#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "AddrInfo.h"
#include "helpers.h"

class ClientConnection {
public:
  explicit ClientConnection(const char* hostname, const int port);
  virtual ~ClientConnection();
  int fd() const;
  bool isConnected() const;

private:
  int sockFd = -1; ///< socket file descriptor
};