////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
//
// helpers.cpp
// Miscellaneous helper functions for convenience.
//
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "helpers.h"

void errorExit(const char* msg) {
  perror(msg);
  exit(errno);
}

void errorExit(int errNo, const char* msg) {
  std::cerr << msg << ": " << strerror(errNo) << std::endl;
  exit(errno);
}

ssize_t writeBuffer(const int fd, const void *buffer, const size_t bufSize) {
  size_t totWritten;              ///< Total # of bytes written
  const char *buf = static_cast<const char*>(buffer);

  for (totWritten = 0; totWritten < bufSize;) {
    ssize_t numWritten = write(fd, buf, bufSize - totWritten);

    // The "write() returns 0" case should never happen, but the
    // following ensures that we don't loop forever if it does 
    if (numWritten <= 0) {
      if (numWritten == -1 && errno == EINTR)
        continue;         // Interrupted --> restart write()
      else
        return -1;        // Some other error
    }
    totWritten += numWritten;
    buf += numWritten;
  }
  return totWritten;      // Must be 'bufSize' bytes if we get here
}

ssize_t writeString(int fd, const std::string &str) {
  return writeBuffer(fd, str.c_str(), str.size());
}
