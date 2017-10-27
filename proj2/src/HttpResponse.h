#pragma once
#include <string>

enum class responseStatusType { Ok, NotFound, BadRequest, InternalServerError };

class HttpResponse
{

public:
	HttpResponse();
	~HttpResponse();

	/**
	* \brief Adds header of response
	* \param plain text of server's response header
	*/
	void addHeader(std::string headerString);

	/**
	* \brief Adds content of response
	* \param plain text of server's response content
	*/
	void addContent(std::string contentString);

	/**
	* \brief creates response given status to be sent to client
	* \param status type
	*/
	void createCustomResponse(const responseStatusType& status);

	/**
	* \brief Creates response using the header and content
	* \from the server
	*/
	void createResponseFromServer();

	/**
	* \brief returns the response created
	* \return string representing the entire response to send
	*/
	std::string getResponse();

private:
	std::string code;
	std::string status;
	std::string header;
	std::string content;
	std::string response;
};


