////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// HttpResponse.cpp
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#include "HttpResponse.h"

HttpResponse::HttpResponse(const char *msg) {
  std::stringstream strm;
  strm <<
    "HTTP/1.0 500 Internal Server Error\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "\r\n";
  strm << ((msg == nullptr) ? "Proxy server error has occurred." : msg);
  strm << "\r\n";
  response = strm.str();
}

HttpResponse::HttpResponse() : HttpResponse(nullptr) {}

HttpResponse::~HttpResponse() { }

const std::string &HttpResponse::str() {
	return response;
}


