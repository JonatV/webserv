#include "method.hpp"
#include "../includes/tools/stringManipulation.hpp"

std::string method::GET(const std::string& request)
{
	std::string response;
	std::string filePath;

	size_t start = request.find("GET") + 4; // 4 is to go after "GET "
	if (start == std::string::npos)
		return (ERROR_400_RESPONSE);
	size_t end = request.find(" ", start);
	if (end == std::string::npos)
		return (ERROR_400_RESPONSE);
	std::string path = request.substr(start, end - start);
	if (path == "/")
		filePath = "./www/index.html";
	else if (path == "/index.html")
		filePath = "./www/index.html";
	else if (path == "/index")
		filePath = "./www/index.html";
	else if (path == "/style/style.css")
		filePath = "./www/style/style.css";
	else if (path == "/404error.html")
		filePath = "./www/error_pages/404error.html";
	else if (path == "/404")
		filePath = "./www/error_pages/404error.html";
	else
		filePath = "";
	if (!filePath.empty()) {
		response = method::foundPage(filePath);
	} else {
		response = method::error404Page();
	}
	return response;
}

std::string method::foundPage(const std::string& filepath)
{
	std::ifstream	file(filepath.c_str());
	std::string		response;
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
		response = 
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/"+ textType +"\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content;
	} else 
		response = error404Page();
	return (response);
}

std::string method::error404Page()
{
	std::ifstream	file("./www/error_pages/404error.html");
	std::string		notFoundResponse;

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
		notFoundResponse = 
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content;
	}
	else
		notFoundResponse = ERROR_404_RESPONSE;
	return (notFoundResponse);
}
