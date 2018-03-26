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
#include <vector>
#include <memory>
#include <ScopeGuard.h>
#include <ForwardSocketData.h>
#include "HttpRequest.h"
#include "HttpHeaderBuilder.h"
#include "HttpResponse.h"
#include "ListenConnection.h"
#include "ClientConnection.h"

/**
 * \brief This handler ensures that even a "rough" exit such as ctrl-c or
 * ctrl-\ from program
 * \param sig Not used
 */
void cleanExit(int sig) {
  exit(0);
}

/**
 * \brief Sends the headers and a message for an http 500 error
 * \param fd Socket file descriptor
 * \param errorMsg Message to include in the 500 response
 */
void handleError(int fd, const char *errorMsg);

/**
 * \brief Handles the user's TCP connection on a separate thread
 * \param clientFd Client socket file descriptor
 */
static void * handleConnectionOn(void * fd);

/**
 * \brief Configure prog to ignore SIGPIPE signals and exit gracefully
 * when quitting
 */
void configureSignals();

/*******************************************************************************
 * MAIN ENTRY POINT
 ******************************************************************************/

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

  int portNo;             ///< User's desired port number
  pthread_t thrId;        ///< Current thread id, unused, since detached
  pthread_attr_t attr{};  ///< Pthread attributes
  int hasError;           ///< When nonzero, indicates an error return result

  /*
   * Process command arguments
   */
  (std::stringstream{ argv[1] }) >> portNo;

  /*
   * Configure prog to ignore SIGPIPE signals and exit gracefully when quitting
   */
  configureSignals();

  /*
   * Open a listening connection w/ given port. Can't continue if can't listen.
   */
  ListenConnection conn{ portNo };
  if (!conn.isConnected())
    errorExit("Could not bind a socket");

  std::cout << "Listening for clients on port " << portNo
            << ", use <CTRL>-C to quit." << std::endl;

  /*
   * Set up threads to be detached. attrib object is reused
   */
  hasError = pthread_attr_init(&attr);
  if (hasError)
    errorExit(hasError, "pthread_attr_init");

  hasError = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (hasError)
    errorExit(hasError, "pthread_attr_setdetachstate");

  /*
   * Begin to accept connections, spawning a new detached thread for each client
   */
  while (true) {
    int clientFd;   ///< Current accepted socket file descriptor
    do {
      clientFd = accept(conn.fd(), nullptr, nullptr);
      if (clientFd == -1) {
        LOG_ERROR("accept");
      }
    } while (clientFd < 0);

    // Create a new detached thread to handle the connection
    hasError = pthread_create(&thrId, &attr,
                              handleConnectionOn, (void *) (intptr_t) clientFd);
    if (hasError) errorExit(hasError, "pthread_create");
  }
}

void configureSignals() {
  // Ignore SIGPIPE signals
  struct sigaction sa{};
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, nullptr) == -1)
    errorExit("sigaction");

  // Ensure that SIGINT/SIGQUIT cause a clean exit
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = cleanExit;
  sa.sa_flags = 0;
  if ((sigaction(SIGINT, &sa, nullptr) == -1) ||
      (sigaction(SIGTERM, &sa, nullptr) == -1) ||
      (sigaction(SIGQUIT, &sa, nullptr) == -1)) {
    errorExit("sigaction");
  }
}

static void * handleConnectionOn(void *fd) {
  int clientFd = (int)(size_t)fd;
  LOG_TRACE("%3d Client connected.", clientFd);

  // Guard to close the socket in case thread terminates unexpectedly
  auto clientFdCloseGuard = make_guard([=] {
    if (clientFd > 0)
      close(clientFd);
  });

  // Receive a request from the user
  LineScanner requestLines {clientFd};
  HttpHeaderBuilder builder {requestLines};
  std::unique_ptr<HttpRequest> req = builder.receiveHeader();

  LOG_TRACE("%3d Req line: %s", clientFd, builder.requestLine().c_str());

  if (req->isValid()) {

    // The user sent valid request and headers, make a connection
    ClientConnection remoteConn{ req->getHost(), req->getPort() };

    if (remoteConn.isConnected()) {
      // Connection established, send headers to the host
      const std::string requestStr = req->getRequestStr();
      writeString(remoteConn.fd(), requestStr);

      // Receive the response line from the server
      LineScanner respLines {remoteConn.fd()};
      ssize_t scanResult = respLines.scanLine();
      if (scanResult <= 0) {
        LOG_ERROR("Invalid response line");
      }

      std::string replyLine = respLines.line();
      LOG_TRACE("%3d Response: %s", remoteConn.fd(), replyLine.c_str());

      // Forward the line and anything left int the line scanner
      writeString(clientFd, replyLine);
      writeBuffer(clientFd, "\n", 1);
      writeBuffer(clientFd, respLines.leftoverBuf(), respLines.leftoverSize());

      // Act like a tunnel until the connection is closed by the site
      ForwardSocketData fwd{remoteConn.fd(), clientFd};
      fwd.start();

    } else {
      // Send back 500 with info about connecting 
      std::stringstream connErrMsg;
      connErrMsg << "Cannot connect to host "
                 << req->getHost() << " on port "
                 << req->getPort() << ".";
      std::string errMsgStr = connErrMsg.str();
      LOG_TRACE("%3d Response: %s", clientFd, errMsgStr.c_str());
      handleError(clientFd, errMsgStr.c_str());
    }
  } else {
    // Errors occurred while parsing request, send back 500
    LOG_TRACE("%3d Response: %s", clientFd, req->getErrorStr());
    handleError(clientFd, req->getErrorStr());
  }

  // Cleanup
  LOG_TRACE("Disconnecting client %d.", clientFd);
  close(clientFd);
  clientFdCloseGuard.dismiss();
  pthread_exit(nullptr);
}

void handleError(const int fd, const char *errorMsg) {
  // User's request or headers had problems, send back http 500 error
  HttpResponse response{errorMsg};

  // Send the error response to the client
  writeString(fd, response.str());
}
