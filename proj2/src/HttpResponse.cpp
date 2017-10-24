#include "HttpResponse.h"
#include <string>

HttpResponse::HttpResponse()
{
	response = "";
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::addHeader(std::string headerString)
{
	header += headerString;
}

void HttpResponse::addContent(std::string ContentString)
{
	content = ContentString;
}

void HttpResponse::createCustomResponse(const responseStatusType& type) {
	switch (type) {
	case responseStatusType::Ok:
		code = "200";
		status = "OK";
		break;
	case responseStatusType::NotFound:
		code = "404";
		status = "Not Found";
		break;
	case responseStatusType::BadRequest:
		code = "301";
		status = "Bad Request";
		break;
	case responseStatusType::InternalServerError:
		code = "500";
		status = "Internal Server Error";
		break;
	}
	response = "HTTP/1.0 " + code + " " + status + "\r\n";
}

void HttpResponse::createResponseFromServer()
{
	response = header + "\r\n" + content;
}

std::string HttpResponse::getResponse()
{
	return response;
}


