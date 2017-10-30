////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// HttpUri.h
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <sstream>
#include <iostream>

class HttpUri {
public:
  HttpUri() = default;
  HttpUri(std::string &&absoluteUri);

  virtual ~HttpUri();

  bool isValid() const;

  const std::string& getScheme() const;
  const std::string& getHost() const;
  int getPort() const;
  const std::string& getPath() const;
  friend std::ostream& operator<<(std::ostream& strm, const HttpUri& uri);

private:
  std::string absoluteUri;  ///< original passed into the ctor
  std::string scheme;       ///< such as 'http', 'https', 'fpt', etc..
  std::string host;         ///< such as 'www.cnn.com', 'google.com', etc..
  int port = 80;            ///< Default 80 if omitted
  std::string path;         ///< such as '/index.html', etc..
  bool isValidFlag = false; ///< Set during parsing if problems encountered
};

