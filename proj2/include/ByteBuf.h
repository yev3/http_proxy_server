#pragma once
#include <gsl/gsl>


/**
 * \brief Represents a buffer of bytes that can be moved and interated
 */
class ByteBuf
{
public:
  ByteBuf();
  ByteBuf(size_t _size);

  // move ctor
  ByteBuf(ByteBuf &&other) noexcept;

  // move assignment
  ByteBuf &operator=(ByteBuf &&other) noexcept;

  gsl::string_span<> string_span() const;

  char *data() const;

  size_t size() const;

private:
  size_t buf_size;               ///< Size of the buffer
  std::unique_ptr<char[]> pbuf;  ///< Holds the data

  // Do not allow this class to be copied or assigned. Only moved.
  ByteBuf(const ByteBuf& other) = delete;
  ByteBuf& operator=(const ByteBuf& other) = delete;
};


