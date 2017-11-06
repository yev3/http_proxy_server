////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
//
// HttpHeaderBuilder.cpp
// Aids in creation of the HttpHeaders.
//
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "HttpHeaderBuilder.h"

std::unique_ptr<HttpRequest>
HttpHeaderBuilder::makeErrorHeader(HttpRequestStatus errStatus) {
  LOG_ERROR("%s", getHttpRequestStatusStr(errStatus));
  // Mark that we are done processing header
  curState = States::HeaderEnd;
  return std::unique_ptr<HttpRequest>(new HttpRequest(errStatus));
}

std::unique_ptr<HttpRequest>
HttpHeaderBuilder::makeHeaderFromRequest (std::string&& line) {
  std::stringstream strm {line};
  firstLine_ = move(line);

  std::string type;
  std::string absUrl;
  std::string protocol;

  strm >> type;
  strm >> absUrl;
  strm >> protocol;

  // Parse the URI of the request
  HttpUri parsedUrl {move(absUrl)};

  // Check all the first line is valid
  if (type != "GET") {
    return makeErrorHeader(HttpRequestStatus::NotGetRequest);
  } else if (!parsedUrl.isValid()) {
    return makeErrorHeader(HttpRequestStatus::UriParseError);
  } else if (parsedUrl.getScheme() != "http") {
    return makeErrorHeader(HttpRequestStatus::NotHttp);
  } else {
    return std::unique_ptr<HttpRequest>(new HttpRequest(move(type),
                                                        move(protocol),
                                                        std::move(parsedUrl)));
  }
}

std::unique_ptr<HttpRequest> HttpHeaderBuilder::receiveHeader() {
  for (;;) {
    // Attempt to scan a line
    ssize_t lineScanResult = lineReceiver_.tryScanLine();

    // If the scan didn't succeed, return to caller
    if (lineScanResult <= 0) {
      errno_t scanErr = errno;
      if (scanErr != EINTR && scanErr != EAGAIN) {
        LOG_ERROR("Failure reading a line, can't create header..");
        return makeErrorHeader(HttpRequestStatus::HeaderError);
      }
    }

    if (curState == States::FirstLine)
    {
      // Parsing first line state, move to getting header lines state
      request = makeHeaderFromRequest(lineReceiver_.line());
      curState = States::HeaderLines;
    }
    else if (curState == States::HeaderLines)
    {
      std::string line = lineReceiver_.line();
      bool isBlankLine = line.empty() || (line.size() == 1 && line[0] == '\r');

      if (!isBlankLine)
      {
        if (line.back() == '\r')
          line.erase(line.end() - 1);
        request->appendHeader(move(line));
      }
      else
      {
        curState = States::HeaderEnd;
        return std::move(request);
      }
    }
  }
}

HttpHeaderBuilder::HttpHeaderBuilder(LineScanner &lr)
    : lineReceiver_(lr), request (nullptr) { }

const std::string &HttpHeaderBuilder::requestLine() {
  return firstLine_;
}
