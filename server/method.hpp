#ifndef METHOD_HPP
#define METHOD_HPP

#include "utils.hpp"
#include "Server.hpp"
#include "../parse/LocationConfig.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>

#define CGI1 "test.cgi"
#define CGI2 "myscript.cgi"
#define CERR_MSG(port, msg) std::cerr << "\e[31m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m" << std::endl

const std::string DELETE_200_RESPONSE =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 61\r\n"
	"\r\n"
	"<html><body><h1>200 OK</h1><p>File deleted.</p></body></html>";

const std::string ERROR_400_RESPONSE =
	"HTTP/1.1 400 Bad Request\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 73\r\n"
	"\r\n"
	"<html><body><h1>400 Bad Request</h1><p>Invalid request.</p></body></html>";

const std::string ERROR_403_RESPONSE =
	"HTTP/1.1 403 Forbidden\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 69\r\n"
	"\r\n"
	"<html><body><h1>403 Forbidden</h1><p>Access denied.</p></body></html>";

const std::string ERROR_404_RESPONSE =
	"HTTP/1.1 404 Not Found\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 70\r\n"
	"\r\n"
	"<html><body><h1>404 Not Found</h1><p>Page not found.</p></body></html>";

const std::string ERROR_405_RESPONSE =
	"HTTP/1.1 405 Method Not Allowed\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 83\r\n"
	"\r\n"
	"<html><body><h1>405 Method Not Allowed</h1><p>Unsupported method.</p></body></html>";

const std::string ERROR_413_RESPONSE =
	"HTTP/1.1 413 Payload Too Large\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 82\r\n"
	"\r\n"
	"<html><body><h1>413 Payload Too Large</h1><p>Request is too big.</p></body></html>";
	
const std::string ERROR_500_RESPONSE =
	"HTTP/1.1 500 Internal Server Error\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 80\r\n"
	"\r\n"
	"<html><body><h1>500 Internal Server Error</h1><p>Server error.</p></body></html>";

const std::string POST_201_RESPONSE =
	"HTTP/1.1 201 Created\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 66\r\n"
	"\r\n"
	"<html><body><h1>201 Created</h1><p>File created.</p></body></html>";

class Server;
namespace method
{
	std::string GET(const std::string& request, int port, Server &server);
	std::string POST(const std::string &request, int port, Server &server);
	std::string DELETE(const std::string& request, int port, Server &server);
	
	std::string foundPage(const std::string& filePath, int port);
	std::string getErrorHtml(int port, const std::string& errorMessage, Server &server);
	
	std::vector<std::string> listFiles(const char* path);
	std::string	generateMethodsPage();
	std::string	generateAutoIndexPage(const LocationConfig* location);
	std::string	generateListHrefHtml(std::vector<std::string> allFiles);
	std::string	generaleListCheckHtml(std::vector<std::string> allFiles, const std::string& path);
	std::string checkDeleteRequest(int port, const std::string &request, Server &server);
	std::string	handleDeleteRequest(const std::string& request);
	std::string	deleteTargetFiles(std::vector<std::string>);
	std::string	trimFileName(std::string);
	std::string	postFromDashboard(const std::string &request, Server &server);
	std::string	postFromTerminal(const std::string &request, Server &server);
	bool		checkPermissions(const std::string& type, const LocationConfig* location);
	
	// CGI
	std::string handleCGI(const std::string& request, int port);

	// helper status code
	std::string POST_303_RESPONSE(const std::string& location);
}

#endif
