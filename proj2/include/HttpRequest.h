////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// HttpRequest.h
// Represents the HTTP request sent to the proxy. Constructed by reading 
// from the specified open file descriptor. Determines if the request 
// is a valid request by checking that the request type is 'GET' and that 
// the specified absolute URI is valid.  
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <sstream>
#include <vector>
#include "HttpUri.h"
#include "helpers.h"

/**
 * \brief Request status codes
 */
enum class HttpRequestStatus {
  Success,        ///< Valid and supported request line and headers
  NotGetRequest,  ///< The client did not request GET
  NotHttp,        ///< The client's URL is not an abs url starting with http://
  UriParseError,  ///< URL had errors in format and could not be parsed
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

  /**
   * \brief Reads the file descriptor stream and creates the HttpRequest
   * \param clientFd Socket or file descriptor to create from
   * \return Initialized HttpRequest 
   */
  static HttpRequest createFrom(const int clientFd);

  /**
   * \brief dtor.
   */
  virtual ~HttpRequest();

  /**
   * \brief True if valid request
   * \return True if valid request
   */
  bool isValid() const;


  /**
   * \brief returns the request with all fields rewritten
   * \return string representing the entire request to send
   */
  std::string getRequestStr();


  /**
   * \brief Returns the host in the request. 
   * Undefined if request is invalid.
   * \return Host the client wishes to connect to
   */
  const char* getHost() const;

  /**
   * \brief Returns the port no in the request. 
   * Undefined if request is invalid.
   * \return Numeric port the client wishes to connect to
   */
  int getPort() const;

  /**
  * \brief returns the status of the http request afer parsing
  * \return status of either success or explaining any errors
  */
  const char* statusStr() const;

private:

  /**
   * \brief Creates an HttpRequest when there is an error parsing it
   * \param status Status code
   */
  explicit HttpRequest(HttpRequestStatus status);

  /**
   * \brief Constructs an Http request when the first line is valid.
   * Note that this ctor is private, since it is created by the static
   * method createFrom(fd)
   * \param type_ type of the request, such as 'GET'
   * \param protocol_ The scheme of the request, such as 'http'
   * \param uri_ Parsed URI from the absolute URL in the request
   */
  explicit HttpRequest(std::string&& type_, std::string&& protocol_, HttpUri&& uri_);

  /**
   * \brief Checks to make sure the header field is valid, replaces any
   * headers such as connection, saves the header for later reproduction
   * when sending request to the server
   * \param line plain text user's request to proxy
   * \return true if valid
   */
  bool appendHeader(std::string&& line);

  HttpRequestStatus parseStatus;    ///< Status of the parsing
  std::vector<Header> headers;      ///< Container for header lines
  std::string type;                 ///< GET / POST / CONNECT, etc..
  std::string protocol;             ///< such as HTTP/1.0
  HttpUri url;                      ///< parsed url

};


