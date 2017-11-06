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

ssize_t copyUntilEOF(int fdSrc, int fdDst) {
	static constexpr size_t BUF_SIZE = 4096;    ///< Temp buffer size
	char buf[BUF_SIZE];                         ///< Buffer for reading/writing
	ssize_t numRead;                            ///< Curr #bytes rcvd from src
	size_t totRead = 0;                         ///< Tot #bytes forwarded

	for (;;) {
		numRead = read(fdSrc, buf, BUF_SIZE);

		if (numRead > 0) {                      // Normal case, forward data
			writeBuffer(fdDst, buf, (const size_t)(numRead));
			totRead += numRead;
		}
		else if (0 == numRead) {              // Reached EOF
			return totRead;
		}
		else if (EINTR == errno) {            // When interrupted, restart
			continue;
		}
		else {                                // Return on all other errors
			return -1;
		}
	}
}