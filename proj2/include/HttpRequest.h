#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "helpers.h"
#include "HttpUri.h"

/**
 * \brief Request status codes
 */
enum class HttpRequestStatus {
  Success,        ///< Valid and supported request line and headers
  NotGetRequest,  ///< The client did not request GET
  NotHttp,        ///< The client's URL is not an absolute url starting with http://
  UriParseError,  ///< The client's URL had errors in its format and could not be parsed
  HeaderError,    ///< Error was while parsing the headers
};

/**
 * \brief Converts an enum to a long description.
 * \param status one of the Http status codes
 * \return c-style string of the description
 */
const char *getHttpRequestStatusStr(const HttpRequestStatus status);

/**
 * \brief Holds a single header line
 */
struct Header {
  std::string name;   ///< Name of the HTTP header
  std::string value;  ///< Values of the HTTP header
};

/**
 * \brief Outputs the header to the specified stream in `name: value` format
 * \param strm Output stream
 * \param header Header to output to stream
 * \return reference to the modified stream
 */
std::ostream& operator<<(std::ostream& strm, const Header& header);

/**
 * \brief Represents the HTTP request sent to the proxy. Constructed
 * by reading from the specified open file descriptor. Determines
 * if the request is a valid request by checking that the request
 * type is 'GET' and that the specified absolute URI is valid.
 */
class HttpRequest {
public:

  explicit HttpRequest(HttpRequestStatus status);
  explicit HttpRequest(std::string&& type_, std::string&& protocol_, HttpUri&& uri_);



  virtual ~HttpRequest();

  /**
   * \brief True if valid request
   * \return True if valid request
   */
  bool isValid() const;

  const char* getStatus() const;


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

  /**
   * \brief Reads the file descriptor stream and creates the HttpRequest
   * \param clientFd Socket or file descriptor to create from
   * \return Initialized HttpRequest 
   */
  static HttpRequest createFrom(const int clientFd);

  std::vector<Header> headers;

  const char* getHost() const;
  int getPort() const;
  std::string type;       ///< GET / POST / CONNECT, etc..
  std::string protocol;   ///< such as HTTP/1.0
  HttpUri url;            ///< parsed url


private:
  HttpRequestStatus parseStatus;

};


