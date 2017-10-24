#include "HttpResponse.h"
#include <string>
#include <iostream>

HttpResponse::HttpResponse(const responseStatusType& type)
{
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
	response = "";
}

HttpResponse::HttpResponse()
{
	response = "";
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::createResponse() {
	response = "HTTP/1.0 " + code + " " + status + "\r\n";
}

void HttpResponse::appendReponse(std::string data)
{
	response += data;
}


std::string HttpResponse::getResponse()
{
	if (response == "") {
		createResponse();
	}
	return response;
}


