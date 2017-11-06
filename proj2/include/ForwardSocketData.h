//
// Created by Yevgeni Kamenski on 11/5/17.
//

#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include "helpers.h"

class ForwardSocketData {
 public:
  ForwardSocketData(int srcFd, int dstFd);


  void receiveBuf(bool &bufIsFull, bool &willBlock, bool &eof, int flags = 0);


  void sendBuf(bool &bufIsEmpty, bool &willBlock, int flags = 0);

  void start();

 private:
  // TODO
//  constexpr static const size_t TOT_BUF_SIZE = 16;
  constexpr static const size_t TOT_BUF_SIZE = 4096;

  char buf[TOT_BUF_SIZE]{};
  int srcFd_;
  int dstFd_;
  ssize_t nxtSend = 0;
  ssize_t endRecv = 0;

  bool haveError = false;

  enum class States {
    Empty,
    PartialRecv,
    PartialSent,
    Full,
    Receiving,
    Sending,
    StateEOF,
  };
  States curState = States::Empty;
};

