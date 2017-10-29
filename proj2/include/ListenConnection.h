#pragma once
#include <unistd.h>

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
