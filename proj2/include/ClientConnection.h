////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// ClientConnection.h
// Client connection class to abstract the system calls. File descriptor is
// closed on instance destruction.
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

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