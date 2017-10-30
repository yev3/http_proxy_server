////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// ListenConnection.h
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <unistd.h>
#include "helpers.h"
#include "AddrInfo.h"

/**
 * \brief Helper for creating a listening socket for the server
 */
class ListenConnection {
public:
  /**
   * \brief creates a helper for binding sockets
   * \param port port number to bind the socket to
   */
  ListenConnection(const int port);

  /**
   * \brief closes any active listening sockets
   */
  virtual ~ListenConnection();

  /**
   * \brief returns the listening socket or -1 if not bound
   * \return listening socket file descriptor or -1 if not bound
   */
  int fd() const;

  /**
   * \brief returns true if socket is bound
   * \return true if the socket is bound and ready to accept connections
   */
  bool isConnected() const;

private:
  int listenFd = -1;      ///< socket file descriptor
  const int BACKLOG = 50; ///< socket's accept backlog
};
