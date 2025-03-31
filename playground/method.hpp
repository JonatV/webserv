#ifndef METHOD_HPP
#define METHOD_HPP

#include <iostream>
#include <string>
#include <fstream>

#define PARSER_MAX_PAYLOAD 100 // global that has to come from the parser
#define PARSER_GET_RIGHT 1
#define PARSER_POST_RIGHT 1
#define PARSER_DELETE_RIGHT 1

const std::string ERROR_400_RESPONSE =
	"HTTP/1.1 400 Bad Request\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 58\r\n"
	"\r\n"
	"<html><body><h1>400 Bad Request</h1><p>Invalid request.</p></body></html>";

const std::string ERROR_403_RESPONSE =
	"HTTP/1.1 403 Forbidden\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 58\r\n"
	"\r\n"
	"<html><body><h1>403 Forbidden</h1><p>Access denied.</p></body></html>";

const std::string ERROR_404_RESPONSE =
	"HTTP/1.1 404 Not Found\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 58\r\n"
	"\r\n"
	"<html><body><h1>404 Not Found</h1><p>Page not found.</p></body></html>";

const std::string ERROR_405_RESPONSE =
	"HTTP/1.1 405 Method Not Allowed\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 69\r\n"
	"\r\n"
	"<html><body><h1>405 Method Not Allowed</h1><p>Unsupported method.</p></body></html>";

const std::string ERROR_413_RESPONSE =
	"HTTP/1.1 413 Payload Too Large\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 67\r\n"
	"\r\n"
	"<html><body><h1>413 Payload Too Large</h1><p>Request is too big.</p></body></html>";
	
const std::string ERROR_500_RESPONSE =
	"HTTP/1.1 500 Internal Server Error\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 64\r\n"
	"\r\n"
	"<html><body><h1>500 Internal Server Error</h1><p>Server error.</p></body></html>";


namespace method
{
	std::string GET(const std::string& request);
	std::string POST(const std::string& request);
	std::string DELETE(const std::string& request);
	
	std::string foundPage(const std::string& filePath);
	std::string error404Page();
}

#endif
