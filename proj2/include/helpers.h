#pragma once
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <sstream>
#include <unistd.h>
#include <limits>

/*
 * Simple terminal colors
 * https://stackoverflow.com/a/30304782
 */
#define TERM_RST  "\x1B[0m"
#define TERM_BLU  "\x1B[34m"
#define TERM_FMT_BLU(X) TERM_BLU X TERM_RST

#ifdef _DEBUG 
/* 
 * Display diagnostics when compiled with _DEBUG defined 
 */

// Ideas from: https://github.com/dmcrodrigues/macro-logger 
#define LOG_FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define LOG_FORMAT TERM_FMT_BLU("%s %s:%d:%s(): ")
#define LOG_ARGS(LOG_TYPE) (LOG_TYPE), LOG_FILE, __LINE__, __FUNCTION__
#define PRINT_FUNCTION(PRINT_FORMAT, ...) fprintf(stderr, (PRINT_FORMAT), __VA_ARGS__)

#define LOG_DEBUG(LOG_MSG, ...) \
  PRINT_FUNCTION(LOG_FORMAT LOG_MSG "\n", LOG_ARGS("DEBUG"), ## __VA_ARGS__)
#define LOG_TRACE(LOG_MSG, ...) \
  PRINT_FUNCTION(LOG_FORMAT LOG_MSG "\n", LOG_ARGS("TRACE"), ## __VA_ARGS__)
#define LOG_ERROR(LOG_MSG, ...) \
  PRINT_FUNCTION(LOG_FORMAT LOG_MSG ": %s\n", LOG_ARGS("ERROR"), ## __VA_ARGS__, strerror(errno))

#else 
/* 
 * Do not log anything when _DEBUG is undefined 
 */

#define LOG_ERROR(message, ...)
#define LOG_TRACE(message, ...)
#define LOG_DEBUG(message, ...)

#endif /* #ifdef _DEBUG */

/**
 * \brief Exits the program, displaying the error string
 * \param msg message to display
 */
void errorExit(const char* msg);

/**
 * \brief Exits the program displaying the error string
 * \param errNo Error number to lookup
 * \param msg Additional message
 */
void errorExit(int errNo, const char* msg);

/**
 * \brief reads a line
 * \param fd file descriptor to read from
 * \param inStrm buffer to store
 * \return number of chars actually read
 */
ssize_t readLineIntoStrm(int fd, std::stringstream &inStrm);

/**
 * \brief 
 * \param fd file descriptor to write stream to
 * \param strm stream object data to write to fd
 * \param size number of characters from stream to write
 * \return 
 */
ssize_t writeStrm(int fd, std::iostream &strm, int size);


/**
 * \brief writes all bytes in buffer to file descriptor. Implementation 
 * borrowed from: http://man7.org/tlpi/code/online/dist/sockets/rdwrn.c.html
 * \param fd file descriptor
 * \param buffer buffer to write
 * \param n buffer size
 * \return number of bytes written from the buffer on success
 */
ssize_t writeBuffer(int fd, const void *buffer, size_t bufSize);

/**
 * \brief Writes the contents of the string to the file descriptor
 * \param fd file descriptor
 * \param str string to write
 * \return the size of the string on success
 */
ssize_t writeString(int fd, const std::string& str);

/**
 * \brief Copies lines from a file descriptor to the STDOUT. Does not allow
 * the output to exceed 80 chars. Displays all headers. Displays the first
 * 5 lines and the last 2 lines of the body or entire body, whichever is
 * smaller.
 * \param fd source file descriptor
 * \param displayLimit maximum number of lines to display
 * \return 0 on success, errno otherwise
 */
int prettyPrintHttpResponse(int fd,
                            int displayLimit = std::numeric_limits<int>::max());

int prettyPrintHttpResponse(std::stringstream& strm,
                            int displayLimit = std::numeric_limits<int>::max());

/**
 * \brief Copies characters into the specified stream until encountering
 * the end of request header marker. (the first <CR><LN>)
 * \param fd source file descriptor
 * \param strm output stream
 * \return Number of bytes read or -1 if an error occurred.
 */
ssize_t receiveResponseHeaders(int fd, std::stringstream &strm);

/**
 * \brief Copies all bytes from the source file descriptor into the
 * destination file descriptor until the source socket disconnects or an
 * error occurs.
 * \param fdSrc source file descriptor
 * \param fdDst destination file descriptor
 * \return number of bytes read or -1 if error occurred.
 */
ssize_t copyUntilEOF(int fdSrc, int fdDst);
