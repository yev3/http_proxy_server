#pragma once
#include <string>
#include <sstream>
#include <vector>

enum class responseStatusType { Ok, NotFound, BadRequest, InternalServerError };

class HttpResponse
{

public:
	/**
	* \brief ctor from the request line
	* \param requestLine request line (first line)
	*/
	explicit HttpResponse(const responseStatusType& status);

	HttpResponse();
	~HttpResponse();

	/**
	* \brief Checks to make sure the header field is valid, replaces any
	* headers such as connection, saves the header for later reproduction
	* when sending request to the server
	* \param line plain text user's request to proxy
	* \return true if valid
	*/
	void appendReponse(std::string line);

	void createResponse();

	/**
	* \brief returns the request with all fields rewritten
	* \return string representing the entire request to send
	*/
	std::string getResponse();

private:
	std::string code;
	std::string status;
	std::string response;
};


