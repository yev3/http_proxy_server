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

#define LOG_DEBUG(LOG_MSG, args...) \
  PRINT_FUNCTION(LOG_FORMAT LOG_MSG "\n", LOG_ARGS("DEBUG"), ## args)
#define LOG_TRACE(LOG_MSG, args...) \
  PRINT_FUNCTION(LOG_FORMAT LOG_MSG "\n", LOG_ARGS("TRACE"), ## args)
#define LOG_ERROR(LOG_MSG, args...) \
  PRINT_FUNCTION(LOG_FORMAT LOG_MSG ": %s\n", LOG_ARGS("ERROR"), ## args, strerror(errno))

#else 
/* 
 * Do not log anything when _DEBUG is undefined 
 */

#define LOG_ERROR(message, args...)
#define LOG_TRACE(message, args...)
#define LOG_DEBUG(message, args...)

#endif /* #ifdef _DEBUG */

/**
 * \brief Exits the program, displaying the error string
 * \param msg message to display
 */
void errorExit(const char* msg);

/**
 * \brief reads a line
 * \param fd file descriptor to read from
 * \param inStrm buffer to store
 * \return number of chars actually read
 */
ssize_t readLineIntoStrm(const int fd, std::stringstream &inStrm);

/**
 * \brief 
 * \param fd file descriptor to write stream to
 * \param strm stream object data to write to fd
 * \param size number of characters from stream to write
 * \return 
 */
ssize_t writeStrm(const int fd, std::iostream &strm, int size);


/**
 * \brief writes all bytes in buffer to file descriptor. Implementation 
 * borrowed from: http://man7.org/tlpi/code/online/dist/sockets/rdwrn.c.html
 * \param fd file descriptor
 * \param buffer buffer to write
 * \param n buffer size
 * \return number of bytes written from the buffer on success
 */
ssize_t writeBuffer(const int fd, const void *buffer, const size_t bufSize);

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
int prettyPrintHttpResponse(const int fd,
                            int displayLimit = std::numeric_limits<int>::max());

////////////////////////////////////////////////////////////////////////////////
