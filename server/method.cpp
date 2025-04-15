#include "method.hpp"

std::string method::GET(const std::string& request, int port, Server& server)
{
	size_t start = request.find("GET") + 4; // 4 is to go after "GET "
	if (start == std::string::npos) throw std::runtime_error(ERROR_400_RESPONSE);
	size_t end = request.find(" ", start);
	if (end == std::string::npos) throw std::runtime_error(ERROR_400_RESPONSE);
	std::string path = request.substr(start, end - start);
	const LocationConfig* location = server.matchLocation(path); // wip return the correct location
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE); // todo that will be a check into error page map
	if (std::find(location->getLocationAllowedMethods().begin(), location->getLocationAllowedMethods().end(), "GET") == location->getLocationAllowedMethods().end())
		throw std::runtime_error(ERROR_403_RESPONSE);
	std::string locationRoot = location->getLocationRoot();
	std::string locationIndex = location->getLocationIndex();
	if (locationRoot.empty() || locationIndex.empty())
		throw std::runtime_error(ERROR_500_RESPONSE);
	std::string filePath = locationRoot + locationIndex;
	std::cout << filePath << " vs " << locationRoot + path << std::endl; // dev

	// if (path == "/stress" || path == "/stress.html")					// stress.html
	// 	filePath = "./www/stress.html";
	// else if (path == "/" || path == "/index" || path == "/index.html")	// index.html
	// 	filePath = "./www/index.html";
	// else if (path == "/dashboard" || path == "/dashboard.html")			// dashboard.html
	// 	filePath = "./www/dashboard.html";
	// else if (path == "/delete" || path == "/delete.html")				// delete.html - It will have a special handling for dynamic content
	// 	filePath = "./www/delete.html";
	// else if (path == "/style/style.css")								// style.css
	// 	filePath = "./www/style/style.css";
	// else if (path == "/assets/favicon.ico" || path == "/favicon.ico")	// favicon.ico
	// 	filePath = "./www/assets/favicon.ico";
	// else if (path == "/cgi-bin/test.cgi" || path == "/cgi-bin/test")	// test.cgi
	// 	filePath = "./www/cgi-bin/test.cgi";
	// else if (path == "/404" || path == "/404error.html")				// 404error.html
	// 	filePath = "./www/error_pages/404error.html";
	// else
	// 	filePath = "";
	if (path.find("cgi-bin") != std::string::npos) // todo
		return (method::handleCGI(path, port));
	if (!filePath.empty())
		return (method::foundPage(filePath, port));
	else
		throw std::runtime_error(ERROR_404_RESPONSE);
}

std::string method::foundPage(const std::string& filepath, int port)
{
	std::ifstream	file(filepath.c_str());
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mGET request for file: " << filepath << "\e[0m" << std::endl; //dev uncomment
	if (file.is_open())
	{
		std::string	textType;
		if (filepath == "./www/delete.html")
			return (generateDeletePage());
		std::string	content = gnl(file);
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
		throw std::runtime_error(ERROR_404_RESPONSE);
}

std::string method::getErrorHtml(int port, const std::string& errorMessage, Server &server)
{
	std::string errorCode;
	size_t posStart = errorMessage.find(" ");
	if (posStart == std::string::npos)
		errorCode = "500";
	else
	{
		errorCode = errorMessage.substr(posStart + 1, 3);
		if (errorCode.length() != 3)
			errorCode = "500";
	}
	// check iun the server map error page
	std::map<int, std::string> errorPages = server.getErrorPages();
	int errorCodeInt = atoi(errorCode.c_str());
	if (errorPages.find(errorCodeInt) != errorPages.end())
	{
		std::string errorFilePath = errorPages[errorCodeInt];
		CERR_MSG(port, "GET Sending error " + errorCode + ": " + errorFilePath);
		std::ifstream file(errorFilePath.c_str());
		if (file.is_open())
		{
			std::string content = gnl(file);
			std::string errorType;
			size_t posEnd = errorMessage.find("\r\n", posStart);
			if (posEnd == std::string::npos)
				return ("");
			else
				errorType = errorMessage.substr(posStart + 4, posEnd - posStart - 4);
			return (
				"HTTP/1.1 " + errorCode + " " + errorType + "\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: " + to_string(content.length()) + "\r\n"
				"\r\n" + content);
		}
		else
		{
			CERR_MSG(port, "GET Sending error " + errorCode + ": " + errorMessage);
			return (errorMessage);
		}
	}
	else
	{
		CERR_MSG(port, "GET Sending error " + errorCode + ": " + errorMessage);
		return (errorMessage);
	}
}

std::string method::POST(const std::string& request, int port)
{
	if (!PARSER_POST_RIGHT) throw std::runtime_error(ERROR_403_RESPONSE);
	
	size_t start = request.find(" ");
	size_t end = request.find(" ", start + 1);
	if (start == std::string::npos || end == std::string::npos) throw std::runtime_error(ERROR_500_RESPONSE);
	std::string pathName = request.substr(start + 1, end - start - 1);
	if (request.find("POST /delete") != std::string::npos)
	{
		std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request for delete\e[0m" << std::endl;
		return (handleDeleteRequest(request));
	}
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request\e[0m" << std::endl;
	std::string content = {0};
	content = postFromDashboard(request); // POST from dashboard
	if (content.empty()) // POST from a terminal (curl)
	{
		if (pathName == "/tmp/")
		{
			std::string body = request.substr(request.find("\r\n\r\n") + 4);
			if (body.empty())
				throw std::runtime_error(ERROR_400_RESPONSE);
			ssize_t bytesReceived = body.length();
			if (bytesReceived > PARSER_MAX_PAYLOAD)
				throw std::runtime_error(ERROR_413_RESPONSE);
			std::string fileName = "./www/tmp/" + to_string(time(0)) + ".txt";
			while (std::ifstream(fileName.c_str()))
				fileName = "./www/tmp/" + to_string(time(0)) + ".txt";
			std::ofstream file(fileName.c_str());
			if (file.is_open())
			{
				file << body;
				file.close();
				content = POST_201_RESPONSE;
			}
			else
				throw std::runtime_error(ERROR_500_RESPONSE);
		}
		else
			throw std::runtime_error(ERROR_403_RESPONSE);
	}
	return (content);
}

std::string method::postFromDashboard(const std::string &request)
{
	std::string content = "";
	size_t start = request.find("MSG_TEXTAREA=");
	if (start == std::string::npos || request.find("application/x-www-form-urlencoded") == std::string::npos)
		return ("");
	content = request.substr(start + std::string("MSG_TEXTAREA=").length());
	ssize_t bytesReceived = content.length();
	if (bytesReceived > PARSER_MAX_PAYLOAD)
		throw std::runtime_error(ERROR_413_RESPONSE);
	std::string fileName = "./www/tmp/" + to_string(time(0)) + ".txt"; // todo check if the path needs to be dynamic
	while (std::ifstream(fileName.c_str()))
		fileName = "./www/tmp/" + to_string(time(0)) + ".txt";
	std::ofstream file(fileName.c_str());
	if (file.is_open())
	{
		file << content;
		file.close();
		return (POST_303_RESPONSE("/dashboard.html"));
	}
	else
		throw std::runtime_error(ERROR_500_RESPONSE);
}

/*
*	Check permission
*	Check body exist or error in the body (no "=on", means nothing check to be deleted)
*	Check if body has multiple file to delete
*	Trim the =on or =on&
*	Delete the files
*/
std::string method::handleDeleteRequest(const std::string& request)
{
	if (!PARSER_DELETE_RIGHT) throw std::runtime_error(ERROR_403_RESPONSE);
	
	if (request.find("Content-Length: 0") != std::string::npos)
		return (POST_303_RESPONSE("/delete.html"));
	if (request.find("=on") == std::string::npos)
		throw std::runtime_error(ERROR_400_RESPONSE);
	std::string body = request.substr(request.find("\r\n\r\n") + 4);
	std::vector<std::string> targetFiles;
	if (body.find("=on&") != std::string::npos)
	{
		size_t start = 0;
		size_t end = body.find("&");
		while (end != std::string::npos)
		{
			targetFiles.push_back(trimFileName(body.substr(start, end - start)));
			start = end + 1;
			end = body.find("&", start);
		}
		if (start < body.length())
			targetFiles.push_back(trimFileName(body.substr(start)));
	}
	else
		targetFiles.push_back(trimFileName(body));
	return (deleteTargetFiles(targetFiles));
}

std::string method::trimFileName(std::string str)
{
	// str = "file1=on&file2=on&file3=on"
	// or str = "file1=on"
	size_t start = 0;
	size_t end = str.find("=");
	return (str.substr(start, end - start));
}

std::string method::deleteTargetFiles(std::vector<std::string>files)
{
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
	{
		std::string filePath = "./www/tmp/" + *it; // todo check if the path needs to be dynamic
		std::ifstream file(filePath.c_str());
		if (!file.is_open())
			return (ERROR_404_RESPONSE);
		file.close();
		if (std::remove(filePath.c_str()) != 0)
			throw std::runtime_error(ERROR_500_RESPONSE);
	}
	return (POST_303_RESPONSE("/delete.html"));
}
std::string method::DELETE(const std::string& request, int port)
{
	if (!PARSER_DELETE_RIGHT) throw std::runtime_error(ERROR_403_RESPONSE);
	std::string filePath;

	size_t start = request.find("DELETE") + 7;
	if (start == std::string::npos)
		throw std::runtime_error(ERROR_400_RESPONSE);
	size_t end = request.find(" ", start);
	if (end == std::string::npos)
		throw std::runtime_error(ERROR_400_RESPONSE);
	filePath = request.substr(start, end - start);
	if (filePath == "/tmp/")
		throw std::runtime_error(ERROR_403_RESPONSE); // Bad request for attempting to delete /tmp/ exactly
	if (filePath.length() >= 5)
	{
		if (filePath.compare(0, 5, "/tmp/") != 0)	// todo check if the path needs to be dynamic
			throw std::runtime_error(ERROR_403_RESPONSE);
	}
	else
		throw std::runtime_error(ERROR_403_RESPONSE);
	filePath = "./www" + filePath;
	std::ifstream	file(filePath.c_str());
	if (!file.is_open())
		throw std::runtime_error(ERROR_404_RESPONSE);
	file.close();
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mDELETE request for file: " << filePath << "\e[0m" << std::endl;
	if (std::remove(filePath.c_str()) == 0)
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 61\r\n"
			"\r\n"
			"<html><body><h1>200 OK</h1><p>File deleted.</p></body></html>");
	else
		throw std::runtime_error(ERROR_500_RESPONSE);
}

std::vector<std::string> method::listFiles()
{
	std::vector<std::string> files = std::vector<std::string>();
	const char* path = "./www/tmp/";
	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		std::cerr << "Error opening directory: " << path << " for listing files." << std::endl; // wip refactor error msg
		throw std::runtime_error(ERROR_500_RESPONSE);
	}
	struct dirent *current_entry;
	while ((current_entry = readdir(dir)))
	{
		if (current_entry->d_type == DT_REG)
		{
			std::string fileName = current_entry->d_name;
			if (fileName != "." && fileName != "..")
				files.push_back(fileName);
		}
	}
	if (closedir(dir) == -1)
	{
		std::cerr << "Error closing directory: " << path << std::endl; // wip refactor error msg
		throw std::runtime_error(ERROR_500_RESPONSE);
	}
	return (files);
}

std::string method::generateDeletePage()
{
	std::ifstream file("./www/delete.html");
	
	if (file.is_open())
	{
		std::string content = gnl(file);
		std::vector<std::string> allFiles = listFiles(); 
		std::string htmlList = generateListHtml(allFiles);
		size_t pos = content.find("<span>No file yet</span>");
		if (pos != std::string::npos)
			content.replace(pos, 24, htmlList);
		else
			throw std::runtime_error(ERROR_500_RESPONSE);
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content);
	}
	else
		throw std::runtime_error(ERROR_404_RESPONSE);
}

std::string method::generateListHtml(std::vector<std::string> allFiles)
{
	std::string fullList = "";

	if (allFiles.empty())
	{
		fullList +=
			"	<span class=\"file_name\">No files found</span>";
		return (fullList);
	}
	fullList += "<ul>";
	for (std::vector<std::string>::iterator it = allFiles.begin(); it != allFiles.end(); ++it)
	{
		std::string		fileContent;
		std::ifstream	currentFile(("./www/tmp/" + *it).c_str());
		std::string		buffer;
		if (currentFile.is_open())
		{
			char tempBuffer[31] = {0};
			currentFile.read(tempBuffer, 30); // i am not using gnl here because i want to limit the size of the content
			buffer = std::string(tempBuffer);
			currentFile.close();
			fullList +=
				"<li>"
				"	<label for=\"" + *it + "\">"
				"		<span class=\"file_name\">" + *it + "</span>"
				"		<p class=\"file_content\">" + buffer + "</p>"
				"	</label>"
				"	<input type=\"checkbox\" name=\"" + *it + "\" id=\"" + *it + "\">"
				"</li>";
		}
		else
		{
			std::cerr << "Error opening file: " << *it << std::endl; // wip refactor error msg
			throw std::runtime_error(ERROR_500_RESPONSE);
		}
	}
	fullList +=
		"</ul>"
		"<button type=\"submit\" class=\"bigBtn\">Delete selected files</button>";
	return (fullList);
}

std::string	method::handleCGI(const std::string& request, int port)
{
	(void)port; // dev
	std::string cgiPath;
	if (request.find(CGI1) != std::string::npos)
		cgiPath = "./www/cgi-bin/" + std::string(CGI1);
	else if (request.find(CGI2) != std::string::npos)
		cgiPath = "./www/cgi-bin/" + std::string(CGI2);
	else
		throw std::runtime_error(ERROR_404_RESPONSE);
	int pipeFd[2];
	if (pipe(pipeFd) == -1)
		throw std::runtime_error(ERROR_500_RESPONSE);
	return (""); // dev: to be implemented
}

std::string method::POST_303_RESPONSE(const std::string& location) {
	return (
		"HTTP/1.1 303 See Other\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 0\r\n"
		"Location: " + location + "\r\n"
		"\r\n");
}
