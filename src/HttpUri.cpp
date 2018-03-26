////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// HttpUri.cpp
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "HttpUri.h"

bool findChar(char c, std::string::iterator &it,
              const std::string::iterator &end) {
  while (it != end) {
    if (*it == c) return true;
    ++it;
  }
  return false;
}

HttpUri::HttpUri(std::string &&absoluteUri_) :
  absoluteUri(move(absoluteUri_)) {

  std::string::iterator it = absoluteUri.begin();
  std::string::iterator fm = it;
  const std::string::iterator absEnd = absoluteUri.end();

  if (it == absEnd) return;

  if(findChar(':', it, absEnd)) {
    scheme = std::string(fm, it);
  } else {
    return;
  }

  if ((it + 3) >= absEnd || *(it + 1) != '/' || *(it + 2) != '/') {
    return;
  }

  // move past the second slash
  it = it + 3;

  // find the start of the relative path
  std::string::iterator pathBegin = it;
  if (!findChar('/', pathBegin, absEnd)) return;
  path = std::string(pathBegin, absEnd);

  // find the start of host
  std::string::iterator hostBegin = it;

  // discard the username/pass part
  if (findChar('@', it, pathBegin)) {
    hostBegin = it + 1;
    
  }

  // determine if the port number is supplied
  it = hostBegin;
  if (findChar(':', it, pathBegin)) {
    std::istringstream portStrm{ std::string {it + 1, pathBegin} };
    portStrm >> port;
  }

  // iterator at the end of the host from last op
  host = std::string{ hostBegin, it };

  isValidFlag = true;
}


HttpUri::~HttpUri() { }

bool HttpUri::isValid() const {
  return isValidFlag;
}

const std::string & HttpUri::getScheme() const {
  return scheme;
}

const std::string & HttpUri::getHost() const {
  return host;
}

int HttpUri::getPort() const {
  return port;
}

const std::string & HttpUri::getPath() const {
  return path;
}
