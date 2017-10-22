#pragma once
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <string>

/**
 * \brief Exits the program, displaying the error string
 * \param msg message to display
 */
void errorExit(const char* msg);

/**
 * \brief Displays an errno string and message
 * \param msg Message to display to user
 */
void errorLog(const char* msg);


class ClientAddrInfo {
public:
  ClientAddrInfo(const std::string& hostname, const int port);
  ~ClientAddrInfo();
  addrinfo* next();
private:
  addrinfo *head = nullptr;
  addrinfo *nextVal = nullptr;
};

class ClientConnection {
public:
  ClientConnection(const std::string& hostname, const int port);
  ~ClientConnection();
  int fd() const;
  bool isConnected() const;

private:
  int sockFd = -1; ///< socket file descriptor
};

/**
 * \brief Helper for resolving addresses for the server
 */
class ServerAddrInfo {
public:
  /**
   * \brief constructs a linked list of addrinfo structures
   * \param port port number to listen to
   */
  explicit ServerAddrInfo(const int port);

  /**
   * \brief Frees up memory of addrinfo structures
   */
  ~ServerAddrInfo();

  /**
   * \brief returns the next addrinfo structure or nullptr if none
   * \return next addrinfo structure in the linked list or nullptr if none
   */
  addrinfo* next();

private:
  addrinfo *head = nullptr;     ///< head of the linked list
  addrinfo *nextVal = nullptr;  ///< current node
};

/**
 * \brief Helper for creating a listening socket for the server
 */
class ServerConnection {
public:
  /**
   * \brief creates a helper for binding sockets
   * \param port port number to bind the socket to
   */
  ServerConnection(const int port);

  /**
   * \brief closes any active listening sockets
   */
  ~ServerConnection();

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
  int listenFd = -1; ///< socket file descriptor
};
