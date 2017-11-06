////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
//
// ForwardSocketData.h
// Forwards the socket data using a circular buffer.
//
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include "helpers.h"

/**
 * \brief Forwards the socket data using a circular buffer.
 */
class ForwardSocketData {
 public:
  /**
   * \brief Constructs a socket forwarder using the source and destination fds
   * \param srcFd source socket file descriptor
   * \param dstFd destination socket file descriptor
   */
  ForwardSocketData(int srcFd, int dstFd);

  /**
   * \brief Begins to transfer data usinga buffer. Returns on completion or
   * error.
   */
  void start();

 private:

  /**
   * \brief Receive data into a buffer
   * \param bufIsFull [OUT] set to true when the buffer is full
   * \param willBlock [OUT] set to true when the call would have blocked
   * \param eof [OUT] set to true when the source socket is closed
   * \param flags [IN] flags to pass to the recv() method
   */
  void receiveBuf(bool &bufIsFull, bool &willBlock, bool &eof, int flags = 0);

  /**
   * \brief Send the data out of the buffer to the client
   * \param bufIsEmpty [OUT] set to true when the buffer is empty
   * \param willBlock [OUT] set to true when a sync call would have blocked
   * \param flags [IN] flags to pass to the send() method
   */
  void sendBuf(bool &bufIsEmpty, bool &willBlock, int flags = 0);


  constexpr static const size_t TOT_BUF_SIZE = 8192;  ///< Buffer size
  char buf[TOT_BUF_SIZE]{};                           ///< Circular buffer

  int srcFd_;                 ///< Source file descriptor
  int dstFd_;                 ///< Destination file descriptor
  ssize_t nxtSend = 0;        ///< Index of the next data to send
  ssize_t endRecv = 0;        ///< Index of the next data to receive
  bool haveError = false;     ///< When an error occurs, set to true

  /**
   * \brief Describes different states of the buffer
   */
  enum class States {
    Empty,        ///< Buffer is empty
    PartialRecv,  ///< Buffer is partially full after receiving
    PartialSent,  ///< Buffer is partially full after sending
    Full,         ///< Buffer is full
    Receiving,    ///< Receiving into buffer
    Sending,      ///< Sending from a buffer
    StateEOF,     ///< Reached an EOF on input
  };

  /**
   * \brief Initial state of the buffer
   */
  States curState = States::Empty;
};

