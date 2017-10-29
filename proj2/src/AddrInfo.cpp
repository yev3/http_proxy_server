#include "AddrInfo.h"

/**
 * \brief Resolves the host to numeric addresses. Assumes a passive request
 * with wildcard ip address of the system when the host parameter is nullptr.
 * \param host Host to resolve. When nullptr, assumes wildcard machine address.
 * \param port Port number
 * \return Pointer to the first node of the result linked list, nullptr if error
 */
addrinfo * resolveAddrInfo(const char * host, const int port) {
  std::string portStr = std::to_string(port);
  addrinfo *result = nullptr;
  addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = (host == nullptr) ? AI_PASSIVE : AI_NUMERICSERV;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;


  if (getaddrinfo(host, portStr.c_str(), &hints, &result)) {
    LOG_ERROR("getaddrinfo");
    freeaddrinfo(result);
    result = nullptr;
  }

  return result;
}

AddrInfo::AddrInfo(const int port) : AddrInfo(nullptr, port) { }

AddrInfo::AddrInfo(const char* host, const int port) {
  head = resolveAddrInfo(host, port);
  nextVal = head;
}

AddrInfo::~AddrInfo() {
  freeaddrinfo(head);
}

addrinfo *AddrInfo::next() {
  addrinfo *curr = nextVal;
  if (nextVal != nullptr)
    nextVal = nextVal->ai_next;
  return curr;
}
