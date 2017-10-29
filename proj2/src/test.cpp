#include <iostream>
#include <fstream>
#include "HttpUri.h"

std::ostream& operator<<(std::ostream & strm, const HttpUri & uri) {
  strm << "Absolute: " << uri.absoluteUri << std::endl;
  strm << "scheme:   " << uri.scheme << std::endl;
  strm << "host:     " << uri.host << std::endl;
  strm << "port:     " << uri.port << std::endl;
  strm << "path:     " << uri.path << std::endl;
  strm << "isValid:  " << uri.isValidFlag << std::endl;
  return strm;
}

int main(int argc, char *argv[]) {
  std::cout << "Hello" << std::endl;

  std::string line;
  while (std::cout << "> ", std::getline(std::cin, line)) {
    HttpUri t{ std::move(line ) };
    std::cout << t << std::endl;
  }

  return 0;
}
