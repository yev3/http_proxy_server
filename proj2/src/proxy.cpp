////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// 
// proxy.cpp
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "helpers.h"
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sstream>
#include <cstring>
#include <vector>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <assert.h>
#include "ListenConnection.h"
#include "ClientConnection.h"

/**
 * \brief Copies characters from one file descriptor to another until EOF
 * \param fdSrc source file descriptor
 * \param fdDst destination file descriptor
 * \return 0 on success, errno otherwise
 */
int copyUntilEOF(const int fdSrc, const int fdDst) {
  char c;
  int numRead;
  while (true) {
    numRead = read(fdSrc, &c, 1);
    if (numRead == -1) {
      if (EINTR != errno) 
        return -1;
    } else if (numRead == 0) {
      return 0; // EOF case
    } else {
      write(fdDst, &c, 1);
    }
  }
}


/**
 * \brief reads a line
 * \param fd file descriptor to read from
 * \param buf buffer to store
 * \param size buffer size
 * \return number of chars actually read
 */
int readLine(const int fd, char *buf, int size) {
  int numRead = 0; ///< how many were read
  char c;
  for (;;) {
    int r = read(fd, &c, 1);
    if (r == -1) {
      if (errno != EINTR) return -1; // return an error unless interrupted
    } else if (r == 0) {
      break;
    } else {
      if ('\n' == c) break;
      *buf++ = c;
      ++numRead;
    }
    // when used up the entire buffer
    if (numRead == (size - 1)) break;
  }

  if (0 != numRead) {
    *buf = '\0';
  }

  return numRead;
}

/**
 * \brief Copies LINES from one file descriptor to another until CRCL
 * \param fdSrc source file descriptor
 * \param fdDst destination file descriptor
 * \return 0 on success, errno otherwise
 */
int copyLinesUntilCRLN(const int fdSrc, const int fdDst) {
  static const int bufSize = 256;
  static char buf[256];
  bool lastLineFull = false;
  static char nl = '\n';
  while (true) {
    int lineSize = readLine(fdSrc, buf, bufSize);
    if (lineSize > 0) {
      write(fdDst, buf, lineSize);
      write(fdDst, &nl, 1);
    } else {
      return lineSize; // error occurred
    }
    if (!lastLineFull) {
      if (lineSize == 1 && buf[0] == '\r') return 0;
    }
    lastLineFull = lineSize == 255;
  }
}

/**
* \brief Copies LINES from one file descriptor to a string until CRCL
* \param fdSrc source file descriptor
* \param copyString is the string that it copies into
* \return 0 on success, errno otherwise
*/
int copyLinesToString(const int fdSrc, std::string &copyString) {
  static const int bufSize = 256;
  static char buf[256];
  bool lastLineFull = false;
  while (true) {
    int lineSize = readLine(fdSrc, buf, bufSize);
    if (lineSize > 0) {
      copyString += buf;
    } else {
      return lineSize; // error occurred
    }
    if (!lastLineFull) {
      if (lineSize == 1 && buf[0] == '\r') return 0;
    }
    lastLineFull = lineSize == 255;
  }
}

int sendString(int fd, const std::string &str) {
  int numLeft = str.size();
  int numSent = 0;
  const char *buf = str.c_str();
  while (numLeft > 0) {
    int written = write(fd, buf, numLeft);
    if (written == -1) {
      if (errno != EINTR) return -1; // return an error unless interrupted
    }
    buf += written;
    numLeft -= written;
    numSent += written;
  }
  ///* We should have written no more than COUNT bytes!   */ 
  assert(numSent == (int)str.size());
  ///* The number of bytes written is exactly COUNT.  */ 
  return numSent;
}


bool getPage(HttpRequest &userRequest, HttpResponse &httpResonse) {
  LOG_TRACE("Opening connection to: %s on port %d", 
    userRequest.getHost(), userRequest.getPort());

  ClientConnection conn{ userRequest.getHost(), userRequest.getPort() };
  if (conn.isConnected()) {
    LOG_TRACE("Connection established.");
    const std::string requestStr = userRequest.getPageRequest();

    LOG_TRACE("Sending headers to host:\n%s", requestStr.c_str());
    sendString(conn.fd(), requestStr);
    LOG_TRACE("Headers sent.");
    LOG_TRACE("Response Headers:");
    copyLinesUntilCRLN(conn.fd(), STDOUT_FILENO);
    LOG_TRACE("Copy content to screen until server terminates connection..");
    std::string content = "";
    copyLinesUntilCRLN(conn.fd(), STDOUT_FILENO);;
  } else {
    LOG_ERROR("can't connect to host");
    return false;
  }

  return true;
}


/**
 * \brief Proxy program main entry point.
 * \param argc number or program arguments.
 * \param argv arguments
 * \return 0 when successfully exited, error code otherwise
 */
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: proxy port" << std::endl;
    exit(-1);
  }

  // User's desired port number
  int portNo;
  (std::stringstream{ argv[1] }) >> portNo;
  ListenConnection conn{ portNo };

  // Program can't continue if unable to listen for connections
  if (!conn.isConnected())
    errorExit("Could not bind a socket");

  std::cout << "Listening for clients on port " << portNo
            << ", use <CTRL>-C to qu,it." << std::endl;

  while (true) {
    const int clientFd = accept(conn.fd(), nullptr, nullptr);
    if (clientFd == -1) {
      LOG_ERROR("accept");
      continue;
    }

    LOG_TRACE("Client connected..");
    LOG_TRACE("------------------------------------------------------------");

    LOG_TRACE("Header info:");

    HttpRequest usrRequest = HttpRequest::createFrom(clientFd);
    std::vector<Header> headers = usrRequest.headers;
    for (uint i = 0; i < headers.size(); ++i)
      std::cout << "header[" << i << "] " << headers[i] << std::endl;

    LOG_TRACE("Request info:");

    LOG_TRACE("type:     %s", usrRequest.type.c_str());
    LOG_TRACE("protocol: %s", usrRequest.protocol.c_str());
    LOG_TRACE("");
    LOG_TRACE("scheme: %s", usrRequest.url.getScheme().c_str());
    LOG_TRACE("host:   %s", usrRequest.url.getHost().c_str());
    LOG_TRACE("path:   %s", usrRequest.url.getPath().c_str());
    LOG_TRACE("port:   %d", usrRequest.url.getPort());
    LOG_TRACE("valid:  %d", usrRequest.url.isValid());

    HttpResponse httpResponse{};
    if (usrRequest.isValid()) {
      if (getPage(usrRequest, httpResponse)) {
        LOG_TRACE("Got a valid page from server.");
      } else {
        LOG_ERROR("Cannot connect to: %s", usrRequest.url.getHost().c_str());
        httpResponse.createCustomResponse(responseStatusType::InternalServerError);
      }
    } else {
      LOG_ERROR("Invalid request: %s", usrRequest.getStatus());
      httpResponse.createCustomResponse(
        responseStatusType::InternalServerError);
    }

    //TODO: Won't need this check is phase 2
    if (httpResponse.getResponse() != "") {
      sendString(clientFd, httpResponse.getResponse());
    }

    LOG_TRACE("-------------------- Done with connection --------------------");

    close(clientFd); // parent does not need the client fd
  }
}
