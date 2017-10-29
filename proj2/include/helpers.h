#pragma once
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sstream>
#include <unistd.h>

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
int readLine(const int fd, std::stringstream &inStrm);
