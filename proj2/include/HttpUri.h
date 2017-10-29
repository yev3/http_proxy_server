#pragma once
#include <string>
#include <sstream>

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

