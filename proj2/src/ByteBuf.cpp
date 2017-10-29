#include "ByteBuf.h"

ByteBuf::ByteBuf(): buf_size(0), pbuf(nullptr) { }

ByteBuf::ByteBuf(unsigned _size): buf_size(_size), pbuf(new char[_size]) { }

ByteBuf::ByteBuf(ByteBuf &&other) noexcept:
  buf_size(other.buf_size) {
  pbuf = std::move(other.pbuf);
}

ByteBuf &ByteBuf::operator=(ByteBuf &&other) noexcept {
  buf_size = other.buf_size;
  pbuf = std::move(other.pbuf);
  return *this;
}

gsl::string_span<> ByteBuf::string_span() const {
  return gsl::string_span<>(pbuf.get(), buf_size);
}

char *ByteBuf::data() const { return pbuf.get(); }

unsigned ByteBuf::size() const { return buf_size; }
