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

  /**
  * \brief returns whether or not the uri is valid
  */
  bool isValid() const;

  /**
  * \brief returns the uri schema
  */
  const std::string& getScheme() const;

  /**
  * \brief returns the uri host
  */
  const std::string& getHost() const;

  /**
  * \brief returns the uri sport number
  */
  int getPort() const;

  /**
  * \brief returns the uri object path
  */
  const std::string& getPath() const;

  /**
  * \brief Outputs the uri to a specified stream
  * \param strm Output stream
  * \param uri Uri to output to stream
  * \return reference to the modified stream
  */
  friend std::ostream& operator<<(std::ostream& strm, const HttpUri& uri);

private:
  std::string absoluteUri;  ///< original passed into the ctor
  std::string scheme;       ///< such as 'http', 'https', 'fpt', etc..
  std::string host;         ///< such as 'www.cnn.com', 'google.com', etc..
  int port = 80;            ///< Default 80 if omitted
  std::string path;         ///< such as '/index.html', etc..
  bool isValidFlag = false; ///< Set during parsing if problems encountered
};

