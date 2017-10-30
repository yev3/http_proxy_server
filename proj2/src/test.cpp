#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include "HttpUri.h"
#include "helpers.h"

std::ostream& operator<<(std::ostream & strm, const HttpUri & uri) {
  strm << "Absolute: " << uri.absoluteUri << std::endl;
  strm << "scheme:   " << uri.scheme << std::endl;
  strm << "host:     " << uri.host << std::endl;
  strm << "port:     " << uri.port << std::endl;
  strm << "path:     " << uri.path << std::endl;
  strm << "isValid:  " << uri.isValidFlag << std::endl;
  return strm;
}

void testURLParsing() {
  std::string line;
  while (std::cout << "parse URL> ", std::getline(std::cin, line)) {
    HttpUri t{ std::move(line ) };
    std::cout << t << std::endl;
  }
}

void testPrettyPrint(const std::string &rawText,
                     int displayLimit = std::numeric_limits<int>::max()) {
  static constexpr int LINE_WIDTH = 4;
  char buf[LINE_WIDTH];
  static const char NL = '\n';

  std::istringstream scanner{rawText};
  int numLinesDisp = 0;
  ssize_t bytesRead = 0;

  bool readBlankLine = false;
  std::string line;
  std::stringstream lineBuf {line};

  while (!readBlankLine) {
    // Read the line from the socket
    std::getline(scanner, line);
    lineBuf.str(line);

    ssize_t readNum = line.size();
    bytesRead = bytesRead + readNum + 1;    // Getline strips \n char

    readBlankLine = readNum == 0 ||
                   (readNum == 1 && lineBuf.str() == "\r");

    while((readNum = lineBuf.readsome(buf, LINE_WIDTH)) > 0) {
      if (++numLinesDisp <= displayLimit) {
        writeBuffer(STDOUT_FILENO, buf, static_cast<const size_t>(readNum));
        writeBuffer(STDOUT_FILENO, &NL, 1);
      }
    }
  }

  if (numLinesDisp > displayLimit) {
    std::cout << "... Omitted " << numLinesDisp - displayLimit
              << " lines for brevity. Content size is " << bytesRead
              << " bytes." << std::endl;
  }
}


int main(int argc, char *argv[]) {
  testPrettyPrint(
    "hi123451234512345123451234512345123451234512345123451234512345123451234512345123451234512345\r\n\r\n"
      "content2\r\nef\r\n\r\n", 2);
  return 0;
}
