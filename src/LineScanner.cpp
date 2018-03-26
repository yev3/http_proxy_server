////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
//
// LineScanner.cpp
// Aids in retrieving lines from a socket.
//
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include <helpers.h>
#include "LineScanner.h"

LineScanner::LineScanner(int readFd) : readFd_(readFd) {
}

ssize_t LineScanner::scanLine() {
  ssize_t scanErr;
  while ((scanErr = tryScanLine()) <= 0) {
    int scanErrNo = errno;
    if (scanErrNo != EINTR && scanErrNo != EAGAIN) {
      LOG_ERROR("Failure trying to read line");
      break;
    }
  }
  return scanErr;
}

ssize_t LineScanner::tryScanLine() {
  char*  curBuf = bufStart;    ///< Receive result
  ssize_t curSize = 0;         ///< Receive result

  // Carry over data from previous call
  if (leftoverSize_ > 0) {
    curBuf = leftoverBuf_;
    curSize = leftoverSize_;
  }

  for (;;) {
    // Buffer empty?
    if (curSize <= 0) {
      // Try filling it
      curSize = recv(readFd_, bufStart, TOT_BUF_SIZE, 0);

      // If we need to wait, or error reading, return failure
      if (curSize <= 0) {
        return curSize;
      }
    }

    // Check to see if there's a newline in what received
    char *newlineLoc =
        (char*)memchr(curBuf, '\n', static_cast<size_t>(curSize));

    // When found newline
    if (newlineLoc != nullptr) {
      // Only partially write the buffer
      line_.write(curBuf, (ssize_t)(newlineLoc - curBuf));

      // Save the rest for later
      leftoverBuf_ = static_cast<char *>(newlineLoc + 1);
      leftoverSize_ = curSize - (leftoverBuf_ - curBuf);

      // Return 1 indicating that there's a newline that was just scanned
      return 1;
    }

    // When no newline, write all to buffer, try again
    line_.write(curBuf, curSize);
    curBuf = bufStart;
    curSize = 0;
  }
}

const char *LineScanner::leftoverBuf() {
  return leftoverBuf_;
}

size_t LineScanner::leftoverSize() {
  return leftoverSize_;
}

std::string LineScanner::line() {
  std::string lineStr = line_.str();
  line_.str(std::string());
  return lineStr;
}
