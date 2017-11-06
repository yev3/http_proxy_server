//
// Created by Yevgeni Kamenski on 11/5/17.
//

#pragma  once
#include <unistd.h>
#include <sys/socket.h>
#include <sstream>

class LineScanner {
 public:
  explicit LineScanner(int readFd);

  /**
   * \brief Try to scan a line.
   * \return 1 if line scanned. 0 if EOF was reached but no line present.
   * File descriptor read error otherwise, such as EAGAIN or EINTR.
   */
  ssize_t tryScanLine();

  /**
   * \brief Scans a line.
   * \return 1 if line scanned. 0 if EOF was reached but no line present.
   * File descriptor read error otherwise, such as EAGAIN or EINTR.
   */
  ssize_t scanLine();

  const char* leftoverBuf();
  size_t leftoverSize();

  std::string line();

 private:
  constexpr static const size_t TOT_BUF_SIZE = 4096;

  char * leftoverBuf_ = nullptr;      ///< Leftover buffer
  size_t leftoverSize_ = 0;           ///< Size of the leftover buffer

  char bufStart[TOT_BUF_SIZE];

  int readFd_;                ///< Where reading from

  std::stringstream line_;    ///< Stored line
};

