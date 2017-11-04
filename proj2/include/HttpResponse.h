////////////////////////////////////////////////////////////////////////////////
// Project2, CPSC 5510 Networking, Seattle University
// Team: Zach Madigan, David Pierce, and Yevgeni Kamenski
// 
// HttpResponse.h
// 
// This is free and unencumbered software released into the public domain.
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <sstream>

class HttpResponse { 
public:
	HttpResponse(const char* msg);
	HttpResponse();
	virtual ~HttpResponse();

	/**
	* \brief returns the response created
	* \return string representing the entire response to send
	*/
  const std::string &str();

private:
	std::string response;	///< http response
};


