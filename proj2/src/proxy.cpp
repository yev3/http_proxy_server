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
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <assert.h>


/**
 * \brief reaps the zombie children
 * \param sig unused argument
 */
void processZombieChildren(int sig) {
  int savedErr = errno;
  while (waitpid(-1, nullptr, WNOHANG) > 0) {}
  errno = savedErr;

}

/**
 * \brief Sets up to reap zombie fork children
 */
void setupGrimReaper() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = processZombieChildren;
  if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
    errorExit("sigaction");
  }
}

/**
 * \brief reads a line
 * \param fd file descriptor to read from
 * \param buf buffer to store
 * \param size buffer size
 * \return number of chars actually read
 */
int readLine(const int fd, char* buf, int size)
{
  int numRead = 0;   ///< how many were read
  char c;
  for (;;) {
    int r = read(fd, &c, 1);
    if (r == -1)
    {
      if (errno != EINTR) return -1; // return an error unless interrupted
    } else if (r == 0)
    {
      break;
    } else {
      if ('\n' == c) break;
      *buf++ = c;
      ++numRead;
    }
    // when used up the entire buffer
    if (numRead == (size - 1)) break;
  }

  if (0 != numRead)
  {
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
      return lineSize;  // error occurred
    }
    if (!lastLineFull)
    {
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
int copyLinesToString(const int fdSrc, std::string& copyString) {
	static const int bufSize = 256;
	static char buf[256];
	bool lastLineFull = false;
	while (true) {
		int lineSize = readLine(fdSrc, buf, bufSize);
		if (lineSize > 0) {
			copyString += buf;
		}
		else {
			return lineSize;  // error occurred
		}
		if (!lastLineFull)
		{
			if (lineSize == 1 && buf[0] == '\r') return 0;
		}
		lastLineFull = lineSize == 255;
	}
}


int sendString(int fd, const std::string& str)
{
  int numLeft = str.size();
  int numSent = 0;
  const char* buf = str.c_str();
  while (numLeft > 0) 
  {
    int written = write (fd, buf, numLeft); 
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

bool isBlankNewline(const char* buf, int size) {
  return size == 1 && buf[0] == '\r';
}

/**
 * \brief returns a collection of lines from user's request
 * \param fd socket file descriptor
 * \return header lines
 */
HttpRequest getHttpRequest(const int fd)
{
  static const int bufSize = 256;
  static char buf[bufSize];

  // read the request line (first line)
  int lineSize = readLine(fd, buf, bufSize);
  if (lineSize < 0) {
    errorLog("request line read");
    return HttpRequest{};
  }

  HttpRequest req{ buf, bufSize };

  while (true) {
    lineSize = readLine(fd, buf, bufSize);
    if (lineSize < 0) {
      errorLog("header read from socket");
      return HttpRequest{};
    }
    if (isBlankNewline(buf, lineSize)) break;  // blank newline
    req.appendHeader(std::string(buf, lineSize));
  }
  return req;
}





///**
// * \brief Copies characters from one file descriptor to another until CRCL
// * \param fdSrc source file descriptor
// * \param fdDst destination file descriptor
// * \return 0 on success, errno otherwise
// */
//int copyUntilCrLn(const int fdSrc, const int fdDst) {
//  char c;
//  while (true) {
//    switch (read(fdSrc, &c, 1)) {
//    case -1:
//      if (errno != EINTR) return -1; // return an error unless interrupted
//      break;
//    case 0:
//      return 0; // EOF
//    default:
//      write(fdDst, &c, 1);
//    }
//  }
//}

/**
 * \brief processes a client connection
 * \param clientFd connected client socket file descriptor
 */
void handleClient(const int clientFd) {

  // receive username string from user
  char buf[256] = { 0 }; // typically max username is 71, but saw 256 also
  read(clientFd, buf, 255);

  // strip any newlines
  std::string userName;
  std::stringstream strm{ buf };
  std::getline(strm, userName);

  dup2(clientFd, STDOUT_FILENO);
  dup2(clientFd, STDERR_FILENO);
  execlp("finger", "finger", userName.c_str(), 0);
  errorExit("execlp"); // should not get here
}


bool getPage(HttpRequest& userRequest, HttpResponse& httpResonse)
{
  std::cout << "Trace - Opening connection to: " << userRequest.uri.authority << std::endl;

  std::cout << "Trace - Requesting page, sending headers:" << std::endl;
  std::string req = userRequest.getPageRequest();
  std::cout << req << std::flush;
  ClientConnection conn{ userRequest.uri.authority, 80 };
  if (conn.isConnected()) {
    std::cout << "Trace - sending the headers.." << std::endl;
    sendString(conn.fd(), req);
    std::cout << "Trace - headers sent, waiting for response.." << std::endl;
    std::cout << "Trace - Got a response!" << std::endl;
    std::cout << "--- HEADERS:" << std::endl;
    copyLinesUntilCRLN(conn.fd(), STDOUT_FILENO);
    std::cout << "--- CONTENT:" << std::endl;
	std::string content = "";
	copyLinesUntilCRLN(conn.fd(), STDOUT_FILENO);;
  } else {
	  errorLog("can't connect to host");
	  return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: proxy port" << std::endl;
    exit(-1);
  }

  int portNo;
  (std::stringstream{ argv[1] }) >> portNo;

  ServerAddrInfo resolvedAddr{ portNo };

  // ignore SIGPIPEs
  signal(SIGPIPE, SIG_IGN);

  // ensure we don't get zombie processes
  setupGrimReaper();

  ServerConnection conn{ portNo };
  if (!conn.isConnected())
    errorExit("Could not bind a socket");

  const int BACKLOG = 50;
  if (listen(conn.fd(), BACKLOG) == -1) errorExit("listen");

  std::cout << "Listening for clients on port " << portNo <<
    ", use <CTRL>-C to quit." << std::endl;

  while (true) {
    const int clientFd = accept(conn.fd(), nullptr, nullptr);
    if (clientFd == -1) {
      errorLog("accept");
      continue;
    }

    // TODO: make this multi-client
    //switch (fork()) {
    //case 0: // child
    //  close(conn.fd());
    //  handleClient(clientFd);
    //case -1: // parent
    //  errorLog("fork");
    //default: // parent
    //  break;
    //}

    std::cout << "\nTrace - client connected.." << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    std::cout << "\nTrace - header info:" << std::endl;
    HttpRequest usrRequest = getHttpRequest(clientFd);
    std::vector<Header> headers = usrRequest.headers;
    for (uint i = 0; i < headers.size(); ++i)
      std::cout << "header[" << i << "] " << headers[i] << std::endl;

    std::cout << "\nTrace - request info:" << std::endl;
	HttpResponse httpResponse{};
    if (usrRequest.isValid()) {
      std::cout << "type:     " << usrRequest.requestLine.type << std::endl;
      std::cout << "url:      " << usrRequest.requestLine.absUrl << std::endl;
      std::cout << "protocol: " << usrRequest.requestLine.protocol << std::endl;
      std::cout <<  std::endl;
      std::cout << "scheme: " << usrRequest.uri.scheme << std::endl;
      std::cout << "auth:   " << usrRequest.uri.authority << std::endl;
      std::cout << "path:   " << usrRequest.uri.path << std::endl;

	  if (!getPage(usrRequest, httpResponse)) {
		  std::cout << "Error: Cannot connect to host: " << usrRequest.uri.authority << std::endl;
		  httpResponse.createCustomResponse(responseStatusType::InternalServerError);
	  }

      std::cout << "\nTrace - found CRLN, client done." << std::endl;
    } else {
      std::cout << "Error: invalid request: " << usrRequest.getErrorText() << std::endl;
	  httpResponse.createCustomResponse(responseStatusType::InternalServerError);
    }
    //if (copyLinesUntilCRLN(clientFd, STDOUT_FILENO) != 0) {
    //  errorExit("copying socket to stdout");
    //}


	//TODO: Won't need this check is phase 2
	if (httpResponse.getResponse() != "") {
		sendString(clientFd, httpResponse.getResponse());
	}
		sendString(clientFd, httpResponse.getResponse());

    std::cout << "------------------------------------------------------------" << std::endl;
    std::cout << "\nTrace - found CRLN, client done." << std::endl;

    close(clientFd); // parent does not need the client fd
  }
}
