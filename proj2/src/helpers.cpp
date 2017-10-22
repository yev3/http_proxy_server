#include "helpers.h"


  //ClientConnection conn{ userArgs.hostname, userArgs.portNo };

  //if (!conn.isConnected()) {
  //  errorExit("cannot connect to socket");
  //}

  //write(conn.fd(), userArgs.username.c_str(), userArgs.username.size() + 1);

  //if (copyUntilCrLn(conn.fd(), STDOUT_FILENO) != 0) {
  //  errorExit("copying socket to stdout");
  //}

void errorExit(const char* msg)
{
  perror(msg);
  exit(errno);
}

void errorLog(const char* msg)
{
  std::cerr << msg << ": " << strerror(errno) << std::endl;
}

ClientAddrInfo::ClientAddrInfo(const std::string& hostname, const int port)
{
  addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_NUMERICSERV; // port no is numeric
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;
  if (0 != getaddrinfo(hostname.c_str(),
                       std::to_string(port).c_str(),
                       &hints,
                       &head))
  {
    // TODO: do not exit program
    errorExit("getaddrinfo");
  }
  nextVal = head;
}

ClientAddrInfo::~ClientAddrInfo()
{
  freeaddrinfo(head);
}

addrinfo* ClientAddrInfo::next()
{
  addrinfo* curr = nextVal;
  if (nextVal != nullptr)
    nextVal = nextVal->ai_next;
  return curr;
}

ClientConnection::ClientConnection(const std::string& hostname, const int port)
{
  ClientAddrInfo resolvedAddr{hostname, port};
  addrinfo* cur;

  while ((cur = resolvedAddr.next()) != nullptr)
  {
    sockFd = socket(cur->ai_family, cur->ai_socktype,
                    cur->ai_protocol);
    if (sockFd != -1)
    {
      if (connect(sockFd, cur->ai_addr, cur->ai_addrlen) != -1)
      {
        return;
      }
      close(sockFd);
      sockFd = -1;
    }
  }
}

ClientConnection::~ClientConnection()
{
  if (sockFd != -1)
  {
    close(sockFd);
  }
}

int ClientConnection::fd() const
{
  return sockFd;
}

bool ClientConnection::isConnected() const
{
  return sockFd != -1;
}

ServerAddrInfo::ServerAddrInfo(const int port)
{
  addrinfo hints = {0};
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;
  if (0 != getaddrinfo(nullptr,
                       std::to_string(port).c_str(),
                       &hints,
                       &head))
  {
    errorExit("getaddrinfo");
  }
  nextVal = head;
}

ServerAddrInfo::~ServerAddrInfo()
{
  freeaddrinfo(head);
}

addrinfo* ServerAddrInfo::next()
{
  addrinfo* curr = nextVal;
  if (nextVal != nullptr)
    nextVal = nextVal->ai_next;
  return curr;
}

ServerConnection::ServerConnection(const int port)
{
  ServerAddrInfo resolvedAddr{port};
  int optVal = 1;
  addrinfo* cur;

  while ((cur = resolvedAddr.next()) != nullptr)
  {
    listenFd = socket(cur->ai_family, cur->ai_socktype,
                      cur->ai_protocol);
    if (listenFd != -1)
    {
      if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optVal,
                     sizeof optVal) == -1)
      {
        errorExit("setsockopt");
      }

      if (bind(listenFd, cur->ai_addr, cur->ai_addrlen) != -1)
      {
        return;
      }
      close(listenFd);
      listenFd = -1;
    }
  }
}

ServerConnection::~ServerConnection()
{
  if (listenFd != -1)
  {
    close(listenFd);
  }
}

int ServerConnection::fd() const
{
  return listenFd;
}

bool ServerConnection::isConnected() const
{
  return listenFd != -1;
}
