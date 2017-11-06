////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
//
// ForwardSocketData.cpp
// Forwards the socket data using a circular buffer.
//
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "ForwardSocketData.h"

ForwardSocketData::ForwardSocketData(int srcFd, int dstFd)
    : srcFd_(srcFd), dstFd_(dstFd) { }

void ForwardSocketData::receiveBuf(bool &bufIsFull,
                                   bool &willBlock,
                                   bool &eof,
                                   int flags) {
  // Max bytes that can receive
  ssize_t maxRecv = (nxtSend <= endRecv) ?
                    (ssize_t)TOT_BUF_SIZE - endRecv :
                    nxtSend - endRecv;

  // Receive into buffer
  ssize_t numRecv = recv(srcFd_, buf + endRecv, (size_t)(maxRecv), flags);

  // Determine if EOF reached
  eof = numRecv == 0;
  if (eof)
    return;

  // Handle errors
  if (numRecv < 0) {
    errno_t recvErr = errno;
    if (recvErr == EINTR) {
      return;
    } else if (recvErr == EWOULDBLOCK) {
      willBlock = true;
      return;
    } else {
      haveError = true;
      return;
    }
  }

  willBlock = false;
  endRecv += numRecv;

  if (endRecv >= TOT_BUF_SIZE) {
    endRecv = 0;
  }

  bufIsFull = endRecv == nxtSend;
}

void ForwardSocketData::sendBuf(bool &bufIsEmpty, bool &willBlock, int flags) {
  // Max bytes that can send
  ssize_t maxSend = (nxtSend < endRecv) ?
                    endRecv - nxtSend :
                    (ssize_t)TOT_BUF_SIZE - nxtSend;

  // Send bytes from buffer
  ssize_t numSent = send(dstFd_, buf + nxtSend, (size_t)(maxSend), flags);

  // Handle error cases
  if (numSent <= 0) {
    errno_t sendErr = errno;
    if (sendErr == EINTR) {
      return;
    } else if (sendErr == EWOULDBLOCK) {
      willBlock = true;
      return;
    } else {
      haveError = true;
      return;
    }
  }

  willBlock = false;
  nxtSend += numSent;

  if (nxtSend >= TOT_BUF_SIZE) {
    nxtSend = 0;
  }

  bufIsEmpty = nxtSend == endRecv;
}

void ForwardSocketData::start() {
  bool bufferEmpty; ///< Indicates an empty buffer
  bool bufferFull;  ///< Indicates a full buffer
  bool willBlock;   ///< Indicates that a blocking call would have occurred
  bool isEOF;       ///< Indicates that no more can be received

  // Continue until an error
  while(!haveError)
  {
    if (curState == States::Empty)
    {
      receiveBuf(bufferFull, willBlock, isEOF, 0);

      // buffer is empty, so on EOF nothing more to do
      if (isEOF) {
        break;
      }

      if (bufferFull) {
        curState = States::Full;
      } else {
        curState = States::Receiving;
      }
    }
    else if(curState == States::Receiving)
    {
      receiveBuf(bufferFull, willBlock, isEOF, MSG_DONTWAIT);
      if (isEOF) {
        curState = States::StateEOF;
      } else if(bufferFull) {
        curState = States::Full;
      } else if(willBlock) {
        curState = States::PartialRecv;
      }
    }
    else if(curState == States::PartialRecv)
    {
      sendBuf(bufferEmpty, willBlock, 0);
      if (bufferEmpty) {
        curState = States::Empty;
      } else {
        curState = States::Sending;
      }
    }
    else if(curState == States::Sending){
      sendBuf(bufferEmpty, willBlock, MSG_DONTWAIT);
      if (bufferEmpty) {
        curState = States::Empty;
      } else if(willBlock) {
        curState = States::PartialSent;
      }
    }
    else if(curState == States::PartialSent)
    {
      receiveBuf(bufferFull, willBlock, isEOF, 0);
      if (isEOF) {
        curState = States::StateEOF;
      } else if(bufferFull) {
        curState = States::Full;
      } else {
        curState = States::Receiving;
      }
    }
    else if(curState == States::Full)
    {
      sendBuf(bufferEmpty, willBlock, 0);
      if (bufferEmpty) {
        curState = States::Empty;
      } else {
        curState = States::Sending;
      }
    }
    else if(curState == States::StateEOF){
      sendBuf(bufferEmpty, willBlock, 0);
      if (bufferEmpty) {
        return;
      }
    } else {
      break;  // Unknown state
    }
    if (haveError) {
      LOG_ERROR("Error occurred copying..");
    }
  }
}
