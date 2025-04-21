#include "method.hpp"

std::string method::GET(const std::string& request, int port, Server& server, bool isRegistered)
{
	size_t start = request.find("GET") + 4; // 4 is to go after "GET "
	size_t end = request.find(" ", start);
	if (start == std::string::npos || end == std::string::npos) throw std::runtime_error(ERROR_400_RESPONSE);

	if (request.find("GET /register") != std::string::npos)
		return (POST_303_RESPONSE("/index.html", true));
	std::string path = request.substr(start, end - start);
	const LocationConfig* location = server.matchLocation(path);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);

	if (checkPermissions("GET", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);

	if (location->getLocationName() != "/" && location->getLocationName().back() == '/') {
		if (location->getLocationAutoIndex())
		{
			if (path.back() != '/')
			{
				size_t pos = path.find_last_of("/");
				std::string lastPath;
				if (pos != std::string::npos)
					lastPath = location->getLocationRoot() + path.substr(pos + 1);
				else
					throw std::runtime_error(ERROR_404_RESPONSE);
				return (method::foundPage(lastPath, port, isRegistered));
			}
			return (generateAutoIndexPage(location, isRegistered));
		}
		else 
			throw std::runtime_error(ERROR_404_RESPONSE);
	}

	std::string locationRoot = location->getLocationRoot();
	std::string locationIndex = location->getLocationIndex();
	if (locationRoot.empty() || locationIndex.empty())
		throw std::runtime_error(ERROR_500_RESPONSE);

	std::string filePath = locationRoot + locationIndex;
	if (path.find("cgi-bin") != std::string::npos) // todo
		return (method::handleCGI(path, port));
	if (!filePath.empty())
		return (method::foundPage(filePath, port, isRegistered));
	else
		throw std::runtime_error(ERROR_404_RESPONSE);
}

std::string method::foundPage(const std::string& filepath, int port, bool isRegistered)
{
	std::ifstream	file(filepath.c_str());
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mGET request for file: " << filepath << "\e[0m" << std::endl; //dev
	if (file.is_open())
	{
		std::string	textType;
		if (filepath == "./www/methods.html")
			return (generateMethodsPage(isRegistered));
		std::string	content = gnl(file, isRegistered);
		if (filepath.find(".css") != std::string::npos)
			textType = "css";
		else if (filepath.find(".txt") != std::string::npos)
			textType = "txt";
		else if (filepath.find(".html") != std::string::npos)
			textType = "html";
		else if (filepath.find(".ico") != std::string::npos)
			textType = "ico";
		else 
			throw std::runtime_error(ERROR_400_RESPONSE);
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/" + textType + "\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content);
	}
	else
		throw std::runtime_error(ERROR_404_RESPONSE);
}

std::string method::getErrorHtml(int port, const std::string& errorMessage, Server &server, bool isRegistered)
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
			std::string content = gnl(file, isRegistered);
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

std::string method::POST(const std::string& request, int port, Server &server)
{
	size_t start = request.find("POST") + 5;
	size_t end = request.find(" ", start);
	if (request.find("POST /delete") != std::string::npos)
		return (checkDeleteRequest(port, request, server));
	if (start == std::string::npos || end == std::string::npos) throw std::runtime_error(ERROR_400_RESPONSE);
	std::string pathName = request.substr(start, end - start);
	const LocationConfig* location = server.matchLocation(pathName);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);
	if (checkPermissions("POST", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);
	std::string content = {0};
	if (request.find("User-Agent: curl") != std::string::npos) // POST from terminal
	{
		std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request from terminal\e[0m" << std::endl;
		content = postFromTerminal(request, server);
	}
	else			// POST from dashboard
	{
		std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request\e[0m" << std::endl;
		content = postFromDashboard(request, server);
	}
	return (content);
}

std::string method::checkDeleteRequest(int port, const std::string &request, Server &server)
{
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request for delete\e[0m" << std::endl;
	std::string lastPart;
	size_t refererStart = request.find("Referer: ");
	size_t refererEnd = request.find("\r\n", refererStart);
	if (refererStart != std::string::npos && refererEnd != std::string::npos)
	{
		refererStart += 9;
		std::string referer = request.substr(refererStart, refererEnd - refererStart);
		size_t lastSlash = referer.find_last_of("/");
		if (lastSlash != std::string::npos)
			lastPart = referer.substr(lastSlash);
		else
			throw std::runtime_error(ERROR_400_RESPONSE);
	}
	else
	{
		std::cout << "\e[31m[" << port << "]\e[0m\t" << "\e[2mNo referer found in POST request\e[0m" << std::endl;
		throw std::runtime_error(ERROR_400_RESPONSE);
	}
	const LocationConfig *location = server.matchLocation(lastPart);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);
	if (checkPermissions("DELETE", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);
	return (handleDeleteRequest(request));
}

std::string method::postFromTerminal(const std::string &request, Server &server)
{
	std::string body = request.substr(request.find("\r\n\r\n") + 4);
	if (body.empty())
		throw std::runtime_error(ERROR_400_RESPONSE);
	ssize_t bytesReceived = body.length();
	if (bytesReceived > server.getClientBodyLimit())
		throw std::runtime_error(ERROR_413_RESPONSE);
	std::string fileName = UPLOAD_PATH + to_string(time(0)) + ".txt";
	while (std::ifstream(fileName.c_str()))
		fileName = UPLOAD_PATH + to_string(time(0)) + ".txt";
	std::ofstream file(fileName.c_str());
	if (file.is_open())
	{
		file << body;
		file.close();
		return (POST_201_RESPONSE);
	}
	else
		throw std::runtime_error(ERROR_500_RESPONSE);
}

std::string method::postFromDashboard(const std::string &request, Server &server)
{
	std::string content = "";
	if (request.find("User-Agent: curl") != std::string::npos)
		return (content);
	size_t start = request.find("MSG_TEXTAREA=");
	content = request.substr(start + std::string("MSG_TEXTAREA=").length());
	ssize_t bytesReceived = content.length();
	if (bytesReceived > server.getClientBodyLimit())
		throw std::runtime_error(ERROR_413_RESPONSE);
	std::string fileName = UPLOAD_PATH + to_string(time(0)) + ".txt";
	while (std::ifstream(fileName.c_str())) // security loop to not have dup filename
		fileName = UPLOAD_PATH + to_string(time(0)) + ".txt";
	std::ofstream file(fileName.c_str());
	if (file.is_open())
	{
		file << content;
		file.close();
		return (POST_303_RESPONSE("/methods.html"));
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
	if (request.find("Content-Length: 0") != std::string::npos)
		return (POST_303_RESPONSE("/methods.html"));
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

std::string method::deleteTargetFiles(std::vector<std::string>files)
{
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
	{
		std::string filePath = UPLOAD_PATH + *it;
		std::ifstream file(filePath.c_str());
		if (!file.is_open())
			return (ERROR_404_RESPONSE);
		file.close();
		if (std::remove(filePath.c_str()) != 0)
			throw std::runtime_error(ERROR_500_RESPONSE);
	}
	return (POST_303_RESPONSE("/methods.html"));
}

std::string method::trimFileName(std::string str)
{
	// str = "file1=on&file2=on&file3=on"
	// or str = "file1=on"
	size_t start = 0;
	size_t end = str.find("=");
	return (str.substr(start, end - start));
}


std::string method::DELETE(const std::string& request, int port, Server &server)
{
	std::string filePath;
	size_t start = request.find("DELETE") + 7;
	size_t end = request.find(" ", start);
	if (start == std::string::npos || end == std::string::npos) throw std::runtime_error(ERROR_400_RESPONSE);
	filePath = request.substr(start, end - start);
	const LocationConfig* location = server.matchLocation(filePath);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);
	if (checkPermissions("DELETE", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);
	std::string locationRoot = location->getLocationRoot();
	std::string locationIndex = location->getLocationIndex();
	if (locationRoot.empty() || locationIndex.empty())
		throw std::runtime_error(ERROR_500_RESPONSE);
	filePath = locationRoot + locationIndex;
	std::ifstream	file(filePath.c_str());
	if (!file.is_open())
		throw std::runtime_error(ERROR_404_RESPONSE);
	file.close();
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mDELETE request for file: " << filePath << "\e[0m" << std::endl;
	if (std::remove(filePath.c_str()) == 0)
		return (DELETE_200_RESPONSE);
	else
		throw std::runtime_error(ERROR_500_RESPONSE);
}

std::vector<std::string> method::listFiles(const char* path)
{
	std::vector<std::string> files = std::vector<std::string>();
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

std::string method::generateMethodsPage(bool isRegistered)
{
	std::ifstream file("./www/methods.html");

	if (file.is_open())
	{
		std::string content = gnl(file, isRegistered);
		std::vector<std::string> allFiles = listFiles(UPLOAD_PATH);
		std::string htmlList = generaleListCheckHtml(allFiles, UPLOAD_PATH);
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

std::string method::generaleListCheckHtml(std::vector<std::string> allFiles, const std::string& path)
{
	std::string fullList = "";

	if (allFiles.empty())
	{
		fullList +=
			"	<span class=\"no_file_method\">No files found</span>";
		return (fullList);
	}
	fullList += "<ul class = \"to_delete_ul\">";
	for (std::vector<std::string>::iterator it = allFiles.begin(); it != allFiles.end(); ++it)
	{
		std::string		fileContent;
		std::ifstream	currentFile((path + *it).c_str());
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
		"<button type=\"submit\" class=\"bigBtn\">Delete</button>";
	return (fullList);
}


std::string method::generateAutoIndexPage(const LocationConfig* location, bool isRegistered)
{
	std::ifstream file("./www/autoindex.html");
	
	if (file.is_open())
	{
		std::string content = gnl(file, isRegistered);
		std::vector<std::string> allFiles = listFiles(location->getLocationRoot().c_str());
		std::string htmlList = generateListHrefHtml(allFiles);
		size_t pos = content.find("<span class=\"file_name_autoindex\">Directory is empty</span>");
		if (pos != std::string::npos)
			content.replace(pos, 61, htmlList);
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

std::string method::generateListHrefHtml(std::vector<std::string> allFiles)
{
	std::string fullList = "";

	if (allFiles.empty())
	{
		fullList +=
			"	<span class=\"file_name_autoindex\">directory is empty</span>";
		return (fullList);
	}
	fullList += "<span class=\"file_name_autoindex\">Files:</span>";
	fullList += "<ul class=\"file_list\">";
	for (std::vector<std::string>::iterator it = allFiles.begin(); it != allFiles.end(); ++it)
	{
		fullList +=
			"<li><a href=\"" + *it + "\" class=\"file_link\">" + *it + "</a></li>";
	}
	fullList += 
		"</ul>";
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

std::string method::POST_303_RESPONSE(const std::string& location, bool setCookie) {
	if (setCookie)
		return (
			"HTTP/1.1 303 See Other\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 0\r\n"
			"Set-Cookie: session-id=1234567890;\r\n" // todo set cookie
			"Location: " + location + "\r\n"
			"\r\n");
	return (
		"HTTP/1.1 303 See Other\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 0\r\n"
		"Location: " + location + "\r\n"
		"\r\n");
}

bool method::checkPermissions(const std::string& type, const LocationConfig* location)
{
	if (location == NULL)
		throw std::runtime_error(ERROR_500_RESPONSE);
	if (type == "GET" && location->getLocationAutoIndex())
		return (true);
	for (const std::string& method : location->getLocationAllowedMethods())
	{
		if (method == type)
			return (true);
	}
	return (false);
}
