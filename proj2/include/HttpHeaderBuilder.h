//
// Created by Yevgeni Kamenski on 11/5/17.
//

#pragma once

#include <string>
#include <cassert>
#include "HttpRequest.h"
#include "LineScanner.h"

class HttpHeaderBuilder {
 public:
  /**
   * \brief Construct a builder for HttpHeader
   * \param lr Use the linereceiver to process lines
   */
  explicit HttpHeaderBuilder(LineScanner& lr);

  /**
   * \brief Tries to receive a header from socket
   * \return 1 indicating header was received. 0 if EOF encountered before a
   * complete header. Error reading from socket otherwise, such as EAGAIN or
   * EPIPE, etc..
   */
  std::unique_ptr<HttpRequest> receiveHeader();

  const std::string& requestLine();

 private:
  /**
   * \brief Different states of the header building process
   */
  enum class States {
    FirstLine,          ///< Receiving first line
    HeaderLines,        ///< Receiving the rest of headers
    HeaderEnd           ///< Blank line (end of headers) encountered
  };

  LineScanner& lineReceiver_;
  States curState = States::FirstLine;
  std::string firstLine_;
  std::unique_ptr<HttpRequest> request;

  std::unique_ptr<HttpRequest> makeErrorHeader(HttpRequestStatus errStatus);
  std::unique_ptr<HttpRequest> makeHeaderFromRequest(std::string&& line);
};
