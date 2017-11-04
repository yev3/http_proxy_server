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
  /**
  * \brief creates a helper for connecting sockets
  * \param hostname host name of server to connect to
  * \param port port number that the server is listening on
  */
  explicit ClientConnection(const char* hostname, const int port);

  /**
   * \brief closes any active connected sockets
   */
  virtual ~ClientConnection();

  /**
  * \brief returns the client socket or -1 if not connection to a server
  * \return client socket file descriptor or -1 if not connected
  */
  int fd() const;

  /**
  * \brief returns true if socket is connected to a server
  * \return true if the socket is connected to a server
  */
  bool isConnected() const;

private:
  int sockFd = -1; ///< socket file descriptor
};