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
#include <csignal>
#include <sstream>
#include <cstring>
#include <vector>W
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ListenConnection.h"
#include "ClientConnection.h"

/**
 * \brief Sends the headers and a message for an http 500 error
 * \param fd Socket file descriptor
 * \param errorMsg Message to include in the 500 response
 */
void handleError(int fd, const char *errorMsg);

/**
 * \brief Handles the user's TCP connection
 * \param clientFd Client socket file descriptor
 */
static void * handleConnectionOn(void * fd);

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

  // Ignore SIGPIPE errors
  struct sigaction sa{};
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, 0) == -1)
    errorExit("sigaction");

  // Program can't continue if unable to listen for connections
  if (!conn.isConnected())
    errorExit("Could not bind a socket");

  std::cout << "Listening for clients on port " << portNo
            << ", use <CTRL>-C to quit." << std::endl;

  /*
   * For Milestone 1, process only 1 connection at a time
   */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
  while (true) {
    int clientFd;
    do {
      clientFd = accept(conn.fd(), nullptr, nullptr);
      if (clientFd == -1) {
        LOG_ERROR("accept");
      }
    } while (clientFd < 0);

    pthread_t thr;
    pthread_attr_t attr{};
    int status;

    status = pthread_attr_init(&attr);
    if (status != 0)
      errorExit(status, "pthread_attr_init");

    status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (status != 0)
      errorExit(status, "pthread_attr_setdetachstate");

    status = pthread_create(&thr, &attr,
                            handleConnectionOn, (void *) (intptr_t) clientFd);
    if (status != 0)
      errorExit(status, "pthread_create");

    status = pthread_attr_destroy(&attr); /* No longer needed */
    if (status != 0)
      errorExit(status, "pthread_attr_destroy");
  }

#pragma clang diagnostic pop
}

static void * handleConnectionOn(void *fd) {
  int clientFd = (int)(size_t)fd;
  LOG_TRACE("%3d Client connected.", clientFd);

  // Read from the client socket and parse the HttpRequest
  HttpRequest usrRequest = HttpRequest::createFrom(clientFd);

  if (usrRequest.isValid()) {
    // The user sent valid request and headers, make a connection
    ClientConnection conn{ usrRequest.getHost(), usrRequest.getPort() };

    if (conn.isConnected()) {
      // Connection established, send headers to the host
      const std::string requestStr = usrRequest.getRequestStr();
      writeString(conn.fd(), requestStr);

      // TODO - remove, testing
      {
        std::stringstream requestStrm{requestStr};
        std::string requestLine;
        std::getline(requestStrm, requestLine);
        LOG_TRACE("%3d Requested: %s", clientFd, requestLine.c_str());
      }

      // Receive the response from the server
      std::stringstream response; 
      ssize_t bytesRead = receiveResponseHeaders(conn.fd(), response);

      // TODO - remove, testing
      {
        LOG_TRACE("%3d Response header size: %d", conn.fd(), (int)bytesRead);
        std::stringstream responseStrmCpy{response.str()};
        std::string responseLine;
        std::getline(responseStrmCpy, responseLine);
        LOG_TRACE("%3d Response: %s", conn.fd(), responseLine.c_str());
      }

      // Send headers back to the original client
      std::string responseStr = response.str();
      writeString(clientFd, responseStr);

      // Act like a tunnel until the connection is closed by the site
      copyUntilEOF(conn.fd(), clientFd);

      // prettyPrintHttpResponse(response, 10);

    } else {
      // Send back 500 with info about connecting 
      std::stringstream connErrMsg;
      connErrMsg << "Cannot connect to host "
                 << usrRequest.getHost() << " on port "
                 << usrRequest.getPort() << ".";
      handleError(clientFd, connErrMsg.str().c_str());
    }
  } else {
    // Errors occurred while parsing request, send back 500
    handleError(clientFd, usrRequest.statusStr());
  }

  LOG_TRACE("Disconnecting client %d.", clientFd);

  // Finished with the client
  close(clientFd);
  pthread_exit(nullptr);
}

void handleError(const int fd, const char *errorMsg) {
  // User's request or headers had problems, send back http 500 error
  HttpResponse response{errorMsg};

  // For MS1, display on console
  std::cout << response.str() << std::flush;

  // Send the error response to the client
  writeString(fd, response.str());
}