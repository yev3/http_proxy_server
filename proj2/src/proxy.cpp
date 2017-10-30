////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// proxy.cpp
// Main entry point of the program
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

/**
 * \brief Sends the headers and a message for an http 500 error
 * \param fd Socket file descriptor
 * \param errorMsg Message to include in the 500 response
 */
void handleError(const int fd, const char *errorMsg);

/**
 * \brief Handles the user's TCP connection
 * \param clientFd Client socket file descriptor
 */
void handleConnectionOn(int clientFd);

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

  /*
   * For Milestone 1, process only 1 connection at a time
   */

  while (true) {
    int clientFd;
    do {
      clientFd = accept(conn.fd(), nullptr, nullptr);
      if (clientFd == -1) {
        LOG_ERROR("accept");
      }
    } while (clientFd < 0);

    handleConnectionOn(clientFd);
  }
}

void handleConnectionOn(int clientFd) {
  LOG_TRACE("Client connected.");

  // Read from the client socket and parse the HttpRequest
  HttpRequest usrRequest = HttpRequest::createFrom(clientFd);

  if (usrRequest.isValid()) {
    // The user sent valid request and headers, make a connection
    ClientConnection conn{ usrRequest.getHost(), usrRequest.getPort() };

    if (conn.isConnected()) {
      // Connection established, send headers to the host
      const std::string requestStr = usrRequest.getRequestStr();
      writeString(conn.fd(), requestStr);

      // Receive the response from the server
      std::stringstream response = receiveSingleResponse(conn.fd());
      std::string responseStr = response.str();

      // Send it back to the original client
      writeString(clientFd, responseStr);

      // Pretty print the response headers to the server console
      prettyPrintHttpResponse(response);
      //prettyPrintHttpResponse(conn.fd());

      // Pretty print the response body, but limit to just 10 lines
      prettyPrintHttpResponse(response, 10);
      //prettyPrintHttpResponse(conn.fd(), 10);
    } else {
      // Send back 500 with info about connecting 
      std::stringstream connErrMsg{}; 
      connErrMsg << "Cannot connect to host "
                 << usrRequest.getHost() << " on port "
                 << usrRequest.getPort() << ".";
      handleError(clientFd, connErrMsg.str().c_str());
    }
  } else {
    // Errors occurred while parsing request, send back 500
    handleError(clientFd, usrRequest.statusStr());
  }

  // Finished with the client
  close(clientFd); 
  LOG_TRACE("Client disconnected.");
}

void handleError(const int fd, const char *errorMsg) {
  // User's request or headers had problems, send back http 500 error
  HttpResponse response{errorMsg};

  // For MS1, display on console
  std::cout << response.str() << std::flush;

  // Send the error response to the client
  writeString(fd, response.str());
}