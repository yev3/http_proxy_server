////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// AddrInfo.h
// Holds the DNS resolution entries.
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <netdb.h>
#include <string>
#include "helpers.h"

/**
 * \brief Simple helper to traverse the addrinfo linked list
 */
class AddrInfo
{
public:
  /**
   * \brief constructs a linked list of addrinfo structures that can be traversed
   * Note: the host address is the wildcard ip address of the system
   * \param port port number
   */
  explicit AddrInfo(const int port);

  explicit AddrInfo(const char* host, const int port);

  /**
   * \brief Frees up memory of addrinfo structures upon desctruction
   */
  virtual ~AddrInfo();

  /**
   * \brief returns the next addrinfo structure or nullptr if none
   * \return next addrinfo structure in the linked list or nullptr if none
   */
  addrinfo* next();

private:
  addrinfo *head = nullptr;     ///< head of the linked list
  addrinfo *nextVal = nullptr;  ///< current node
};