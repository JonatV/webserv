#include "method.hpp"
#include "../includes/tools/stringManipulation.hpp"

std::string method::GET(const std::string& request)
{
	if (!PARSER_GET_RIGHT) return (ERROR_403_RESPONSE);
	std::string	filePath;

	size_t start = request.find("GET") + 4; // 4 is to go after "GET "
	if (start == std::string::npos)
		return (ERROR_400_RESPONSE);
	size_t end = request.find(" ", start);
	if (end == std::string::npos)
		return (ERROR_400_RESPONSE);
	std::string path = request.substr(start, end - start);
	if (path == "/" || path == "/index" || path == "/index.html")	// index.html
		filePath = "./www/index.html";
	else if (path == "/dashboard" || path == "/dashboard.html")		// dashboard.html
		filePath = "./www/dashboard.html";
	else if (path == "/delete" || path == "/delete.html")			// delete.html - It will have a special handling for dynamic content
		filePath = "./www/delete.html";
	else if (path == "/style/style.css")							// style.css
		filePath = "./www/style/style.css";
	else if (path == "/404" || path == "/404error.html")			// 404error.html
		filePath = "./www/error_pages/404error.html";
	else
		filePath = "";
	if (!filePath.empty())
		return (method::foundPage(filePath));
	else
		return (method::error404Page());
}

std::string method::foundPage(const std::string& filepath)
{
	std::ifstream	file(filepath.c_str());

	std::cout << "\e[32mGET request for file: " << filepath << "\e[0m" << std::endl;
	if (file.is_open())
	{
		std::string	textType;
		std::string	content = "";
		std::string	line;
		
		while (std::getline(file, line))
			content += line + "\n";
		// remove the last newline character
		if (!content.empty())
			content.erase(content.length() - 1);
		file.close();
		if (filepath.find(".css") != std::string::npos)
			textType = "css";
		else
			textType = "html";
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/" + textType + "\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content);
	}
	else
		return(method::error404Page());
}

std::string method::error404Page()
{
	std::ifstream	file("./www/error_pages/404error.html");

	if (file.is_open())
	{
		std::string content = "";
		std::string line;
		while (std::getline(file, line))
			content += line + "\n";
		// remove the last newline character
		if (!content.empty())
			content.erase(content.length() - 1);
		file.close();
		return (
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content);
	}
	else
		return (ERROR_404_RESPONSE);
}

// 1 parse the content
// 2 save the content
	// 2.1 create a tmp file_name
		// 2.1.1 check if the file exists
		// 2.1.2 if it exists, create a new file_name and repeat 2.1.1
	// 2.2 check if the file exists
	// 2.3 write the content to the file
	// 2.4 close the file
// 3 send the response
std::string method::POST(const std::string& request)
{
	if (!PARSER_POST_RIGHT) return (ERROR_403_RESPONSE);
	std::string	content;

	size_t start = request.find("MSG_TEXTAREA=");
	if (start == std::string::npos || request.find("application/x-www-form-urlencoded") == std::string::npos)
		return (ERROR_400_RESPONSE); // bad request
	content = request.substr(start + std::string("MSG_TEXTAREA=").length());
	std::cout << "\e[30m" << content << std::endl;
	ssize_t bytesReceived = content.length();
	std::cout << "Bytes received: " << bytesReceived << "\e[0m" << std::endl;
	if (bytesReceived > PARSER_MAX_PAYLOAD)
		return (ERROR_413_RESPONSE);
	std::string		fileName = "./www/tmp/" + to_string(time(0)) + ".txt";
	while (std::ifstream(fileName.c_str()))
		fileName = "./www/tmp/" + to_string(time(0)) + ".txt";
	std::ofstream	file(fileName.c_str());
	if (file.is_open())
	{
		file << content;
		file.close();
		return (
			"HTTP/1.1 201 Created\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 58\r\n"
			"\r\n"
			"<html><body><h1>201 Created</h1><p>Message saved.</p></body></html>");
	}
	else
		return (ERROR_500_RESPONSE);
}

std::string method::DELETE(const std::string& request)
{
	if (!PARSER_DELETE_RIGHT) return (ERROR_403_RESPONSE);
	std::string filePath;

	size_t start = request.find("DELETE") + 7;
	if (start == std::string::npos)
		return (ERROR_400_RESPONSE);
	size_t end = request.find(" ", start);
	if (end == std::string::npos)
		return (ERROR_400_RESPONSE);
	filePath = request.substr(start, end - start);
	if (filePath.length() >= 5)
	{
		if (filePath.compare(0, 5, "/tmp/") != 0)
			return (ERROR_403_RESPONSE);
	}
	else
		return (ERROR_403_RESPONSE);
	filePath = "./www" + filePath;
	std::ifstream	file(filePath.c_str());
	if (!file.is_open())
		return (ERROR_404_RESPONSE);
	file.close();
	if (std::remove(filePath.c_str()) == 0)
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 58\r\n"
			"\r\n"
			"<html><body><h1>200 OK</h1><p>File deleted.</p></body></html>");
	else
		return (ERROR_404_RESPONSE);
}
