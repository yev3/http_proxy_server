#include "helpers.h"

void errorExit(const char* msg) {
  perror(msg);
  exit(errno);
}

ssize_t readLineIntoStrm(const int fd, std::stringstream &inStrm) {
  int numRead = 0; ///< how many were read
  char c;
  for (;;) {
    int r = read(fd, &c, 1);
    if (r == -1) {
      if (errno != EINTR) return -1; // return an error unless interrupted
    } else if (r == 0) {
      break;
    } else {
      if ('\n' == c) {
        break;
      }
      inStrm.put(c);
      ++numRead;
    }
  }

  return numRead;
}

ssize_t writeStrm(const int fd, std::iostream &strm, int size) {
  int writtenSize;              ///< number of chars written to file descriptor
  for (writtenSize = 0; writtenSize < size; ++writtenSize) {
    char c;
    if (strm.get(c)) {
      // keep trying to write char if interrupted
      int writeResult;          
      do {
        writeResult = write(fd, &c, 1);
      } while (-1 == writeResult && EINTR == errno);

      // do not continue should an error occur
      if (-1 == writeResult)
        return -1;

    } else {
      // EOF on the input stream
      break;
    }
  }
  return writtenSize;
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

int prettyPrintHttpResponse(std::stringstream& strm, int displayLimit) {
  static constexpr int LINE_WIDTH = 80;   ///< When to force a wrap
  static const char NL = '\n';            ///< Newline char
  char buf[LINE_WIDTH];                   ///< Temp buffer to hold cur line
  int numLinesShown = 0;                  ///< How many lines shown
  ssize_t bytesRead = 0;                  ///< Total # bytes read
  bool readBlankLine = false;             ///< When true, marks a section sep
  std::string line;                       ///< current line

  while (!readBlankLine) {
    std::getline(strm, line);

    bytesRead = bytesRead + line.size() + 1; // Getline strips \n char

    readBlankLine = line.size() == 0 || (line.size() == 1 && line == "\r");

    if (line.size() > 0) {
      std::stringstream lineStrm{ line };
      int readNum;
      while ((readNum = lineStrm.readsome(buf, LINE_WIDTH)) > 0) {
        if (++numLinesShown <= displayLimit) {
          writeBuffer(STDOUT_FILENO, buf, static_cast<const size_t>(readNum));
          writeBuffer(STDOUT_FILENO, &NL, 1);
        }
      }
    } 
  }

  if (numLinesShown > displayLimit) {
    std::cout << "... Omitted " << numLinesShown - displayLimit
      << " lines for brevity. Content size is " << bytesRead
      << " bytes." << std::endl;
  }

  return 0;
}

int prettyPrintHttpResponse(const int fd, int displayLimit) {
  static constexpr int LINE_WIDTH = 80;   ///< When to force a wrap
  static const char NL = '\n';            ///< Newline char
  char buf[LINE_WIDTH];                   ///< Temp buffer to hold cur line
  int numLinesShown = 0;                  ///< How many lines shown
  ssize_t bytesRead = 0;                  ///< Total # bytes read
  bool readBlankLine = false;             ///< When true, marks a section sep
  std::stringstream strm;                 ///< Stream to read and write from fd

  while (!readBlankLine) {
    // Clear the buffer, read from the source
    strm.str(std::string());

    ssize_t readNum = readLineIntoStrm(fd, strm);
    bytesRead = bytesRead + readNum + 1; // Getline strips \n char

    readBlankLine = readNum == 0 || (readNum == 1 && strm.str() == "\r");

    if (readNum > 0) {
      while ((readNum = strm.readsome(buf, LINE_WIDTH)) > 0) {
        if (++numLinesShown <= displayLimit) {
          writeBuffer(STDOUT_FILENO, buf, static_cast<const size_t>(readNum));
          writeBuffer(STDOUT_FILENO, &NL, 1);
        }
      }
    } else if (readNum < 0) {
      return readNum; // error case
    } else {
      break;          // EOF case
    }
  }

  if (numLinesShown > displayLimit) {
    std::cout << "... Omitted " << numLinesShown - displayLimit
      << " lines for brevity. Content size is " << bytesRead
      << " bytes." << std::endl;
  }

  return 0;
}

ssize_t receiveSingleResponse(const int fd, std::stringstream& strm) {
  ssize_t bytesRead = 0;                  ///< Total # bytes read
  int numBlankLinesLeft = 2;              ///< Stop when encounter a snd line
  std::stringstream line;                 ///< current line

  while (numBlankLinesLeft) {
    // Clear the line
    line.str(std::string());

    ssize_t readNum = readLineIntoStrm(fd, line);
    bytesRead = bytesRead + readNum + 1; // Getline strips \n char

    if(readNum == 0 || (readNum == 1 && line.str() == "\r")) {
      --numBlankLinesLeft;
    }

    strm << line.str() << '\n';
  }

  return bytesRead;
}
