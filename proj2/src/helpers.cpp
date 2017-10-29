#include "helpers.h"

void errorExit(const char* msg) {
  perror(msg);
  exit(errno);
}

int readLine(const int fd, std::stringstream &inStrm) {
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
