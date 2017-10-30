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
#include "ListenConnection.h"
#include "ClientConnection.h"

bool getPage(HttpRequest &userRequest, HttpResponse &httpResonse) {
  LOG_TRACE("Opening connection to: %s on port %d", 
    userRequest.getHost(), userRequest.getPort());

  ClientConnection conn{ userRequest.getHost(), userRequest.getPort() };
  if (conn.isConnected()) {
    LOG_TRACE("Connection established.");
    const std::string requestStr = userRequest.getPageRequest();

    LOG_TRACE("Sending headers to host:\n%s", requestStr.c_str());
    writeString(conn.fd(), requestStr);
    LOG_TRACE("Headers sent.");
    LOG_TRACE("Response Headers:");
    prettyPrintHttpResponse(conn.fd());
    LOG_TRACE("Copy content to screen until server terminates connection..");
    std::string content = "";
    prettyPrintHttpResponse(conn.fd(), 10);
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
            << ", use <CTRL>-C to quit." << std::endl;

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
      writeString(clientFd, httpResponse.getResponse());
    }

    LOG_TRACE("-------------------- Done with connection --------------------");

    close(clientFd); // parent does not need the client fd
  }
}
