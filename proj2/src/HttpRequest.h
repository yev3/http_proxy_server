#pragma once
#include <string>
#include <sstream>
#include <vector>

struct Header
{
  std::string name;
  std::string value;
  //friend std::ostream& operator<<(std::ostream& strm, const HttpRequest::Header& header);
};

std::ostream& operator<<(std::ostream& strm, const Header& header);

class HttpRequest
{

public:
  /**
   * \brief ctor from the request line
   * \param requestLine request line (first line)
   */
  explicit HttpRequest(std::string& requestLine);

  explicit HttpRequest(const char* buf, size_t n);

  HttpRequest();
  ~HttpRequest();

  /**
   * \brief True if valid request
   * \return True if valid request
   */
  bool isValid() const;

  /**
   * \brief Returns user-readable string of any errors that occurred
   * \return text of parsing errors
   */
  std::string getErrorText();

  void adjustHeaderIfNeeded(Header& header);
  bool headerIsFiltered(const Header& header);
  /**
   * \brief Checks to make sure the header field is valid, replaces any
   * headers such as connection, saves the header for later reproduction
   * when sending request to the server
   * \param line plain text user's request to proxy
   * \return true if valid
   */
  bool appendHeader(std::string&& line);

  /**
   * \brief returns the request with all fields rewritten
   * \return string representing the entire request to send
   */
  std::string getPageRequest();

  std::vector<Header> headers;

  struct {
    std::string type;       ///< GET / POST / CONNECT, etc..
    std::string protocol;   ///< such as HTTP/1.0
    std::string absUrl;     ///< absolute url
  } requestLine;

  struct {
    std::string scheme;     ///< such as 'http', 'https', 'fpt', etc..
    std::string authority;  ///< such as 'www.cnn.com', 'google.com', etc..
    std::string port;       ///< such as '80'
    std::string path;       ///< such as '/index.html', etc..
  } uri;

private:
  enum class UriParseErrType { Success, NotHttp, Malformed };
  UriParseErrType parseStatus;
  bool hasHeaderConnection = false;
  bool hasHeaderHost = false;

  void parseFirstLine(const std::string& line);
  void parseUri();
};


