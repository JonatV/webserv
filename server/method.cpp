#include "cookies_session.hpp"
#include "method.hpp"

std::string method::GET(const std::string& request, int port, Server& server, bool isRegistered)
{
	size_t start = request.find("GET") + 4;
	size_t end = request.find(" ", start);
	if (start == std::string::npos || end == std::string::npos) 
		throw std::runtime_error(ERROR_400_RESPONSE);
	if (request.find("GET /register") != std::string::npos)
		return (POST_303_RESPONSE("/index.html", true));

	std::string fullPath = request.substr(start, end - start);
	std::string path = fullPath;
	
	size_t questionMarkPos = fullPath.find('?');
	if (questionMarkPos != std::string::npos) {
		path = fullPath.substr(0, questionMarkPos);
	}
	
	const LocationConfig* location = server.matchLocation(path);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);

	if (checkPermissions("GET", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);

	std::string locationName = location->getLocationName();
	std::string locationRoot = location->getLocationRoot();
	std::string locationIndex = location->getLocationIndex();

	if (!locationRoot.empty() && !locationIndex.empty())
	{
		std::string filePath = locationRoot + locationIndex;
		// Check if it's a CGI script
		if (isCGIScript(filePath))
			return (handleCGI(request, filePath, port));
	}

	if (locationName != "/" && locationName[locationName.length() - 1] == '/') 
	{
		if (location->getLocationAutoIndex())
		{
			if (path[path.length() - 1] != '/')
			{
				size_t pos = path.find_last_of("/");
				std::string lastPath;
				if (pos != std::string::npos)
					lastPath = locationRoot + path.substr(pos + 1);
				else
					throw std::runtime_error(ERROR_404_RESPONSE);
				return (method::foundPage(lastPath, isRegistered));
			}
			return (generateAutoIndexPage(location, isRegistered));
		}
		else 
		{
			throw std::runtime_error(ERROR_404_RESPONSE);
		}
	}

	if (locationRoot.empty() || locationIndex.empty())
		throw std::runtime_error(ERROR_500_RESPONSE);
	std::string filePath = locationRoot + locationIndex;
	if (!filePath.empty())
		return (method::foundPage(filePath, isRegistered));
	else
		throw std::runtime_error(ERROR_404_RESPONSE);
}

std::string method::foundPage(const std::string& filepath, bool isRegistered)
{
	std::ifstream	file(filepath.c_str());
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
			CERR_MSG(port, "GET Sending error " + errorCode + ": sending default 404 fallback page");
			return (errorMessage);
		}
	}
	else
	{
		CERR_MSG(port, "GET Sending error " + errorCode + ": sending default 404 fallback page");
		return (errorMessage);
	}
}

std::string method::POST(const std::string& request, int port, Server &server)
{
	size_t start = request.find("POST") + 5;
	size_t end = request.find(" ", start);

	if (start == std::string::npos || end == std::string::npos) 
		throw std::runtime_error(ERROR_400_RESPONSE);

	std::string fullPathName = request.substr(start, end - start);
	std::string pathName = fullPathName;
	
	size_t questionMarkPos = fullPathName.find('?');
	if (questionMarkPos != std::string::npos) {
		pathName = fullPathName.substr(0, questionMarkPos);
	}

	if (request.find("POST /delete") != std::string::npos)
		return (checkDeleteRequest(request, server));

	const LocationConfig* location = server.matchLocation(pathName);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);

	if (checkPermissions("POST", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);

	std::string locationRoot = location->getLocationRoot();
	std::string locationIndex = location->getLocationIndex();

	if (!locationRoot.empty() && !locationIndex.empty())
	{
		std::string filePath = locationRoot + locationIndex;
		if (isCGIScript(filePath))
			return (handleCGI(request, filePath, port));
	}

	std::string contentLengthHeader;
	size_t contentLengthPos = request.find("Content-Length: ");
	if (contentLengthPos != std::string::npos)
	{
		size_t lineEnd = request.find("\r\n", contentLengthPos);
		if (lineEnd != std::string::npos)
		{
			contentLengthHeader = request.substr(contentLengthPos + 16, lineEnd - contentLengthPos - 16);
			ssize_t contentLength = atoi(contentLengthHeader.c_str());
			if (contentLength > server.getClientBodyLimit())
				throw std::runtime_error(ERROR_413_RESPONSE);
		}
	}
	if (request.find("User-Agent: curl") != std::string::npos)
		return (postFromTerminal(request, server));
	else if (request.find("Content-Type: multipart/form-data") != std::string::npos)
		return (handleFileUpload(request, server));
	else if (request.find("Content-Type: application/x-www-form-urlencoded") != std::string::npos)
		return (postFromDashboard(request, server));
	else
		return (postFromDashboard(request, server));
}

std::string method::handleFileUpload(const std::string& request, Server& server)
{
	std::string boundary;
	size_t boundaryPos = request.find("boundary=");
	if (boundaryPos != std::string::npos)
	{
		size_t lineEnd = request.find("\r\n", boundaryPos);
		boundary = request.substr(boundaryPos + 9, lineEnd - boundaryPos - 9);
	}
	
	if (boundary.empty())
		throw std::runtime_error(ERROR_400_RESPONSE);
	size_t bodyStart = request.find("\r\n\r\n");
	if (bodyStart == std::string::npos)
		throw std::runtime_error(ERROR_400_RESPONSE);
	
	std::string body = request.substr(bodyStart + 4);
	if ((ssize_t)body.length() > server.getClientBodyLimit())
		throw std::runtime_error(ERROR_413_RESPONSE);

	std::string fileName = UPLOAD_PATH + to_string(time(0)) + "_upload.txt";
	while (std::ifstream(fileName.c_str()))
		fileName = UPLOAD_PATH + to_string(time(0)) + "_upload.txt";
	
	std::ofstream file(fileName.c_str());
	if (file.is_open())
	{
		file << "=== File uploaded via multipart/form-data ===" << std::endl;
		file << "Boundary: " << boundary << std::endl;
		file << "Content length: " << body.length() << std::endl;
		file << "=== Raw content ===" << std::endl;
		file << body;
		file.close();
		return (POST_201_RESPONSE);
	}
	else
		throw std::runtime_error(ERROR_500_RESPONSE);
}

std::string method::checkDeleteRequest(const std::string &request, Server &server)
{
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
		throw std::runtime_error(ERROR_400_RESPONSE);
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
	std::string extension = ".txt";
	if (request.find("Content-Type: application/json") != std::string::npos)
		extension = ".json";
	else if (request.find("Content-Type: text/html") != std::string::npos)
		extension = ".html";
	else if (request.find("Content-Type: text/xml") != std::string::npos)
		extension = ".xml";

	std::string fileName = UPLOAD_PATH + to_string(time(0)) + extension;
	while (std::ifstream(fileName.c_str()))
		fileName = UPLOAD_PATH + to_string(time(0)) + extension;

	std::ofstream file(fileName.c_str());
	if (file.is_open())
	{
		file << body;
		file.close();
		
		std::string response = 
			"HTTP/1.1 201 Created\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: ";
		
		std::string jsonResponse = "{\"status\":\"success\",\"message\":\"File created\",\"filename\":\"" 
								 + fileName + "\",\"size\":" + to_string(bytesReceived) + "}";
		
		response += to_string(jsonResponse.length()) + "\r\n\r\n" + jsonResponse;
		return response;
	}
	else
		throw std::runtime_error(ERROR_500_RESPONSE);
}

std::string method::postFromDashboard(const std::string &request, Server &server)
{
	if (request.find("User-Agent: curl") != std::string::npos)
		return postFromTerminal(request, server);

	std::string body = request.substr(request.find("\r\n\r\n") + 4);
	std::string content = "";
	
	size_t msgStart = body.find("MSG_TEXTAREA=");
	if (msgStart != std::string::npos)
	{
		content = body.substr(msgStart + std::string("MSG_TEXTAREA=").length());
		
		size_t plusPos = 0;
		while ((plusPos = content.find("+", plusPos)) != std::string::npos)
		{
			content.replace(plusPos, 1, " ");
			plusPos++;
		}
		
		size_t percentPos = 0;
		while ((percentPos = content.find("%", percentPos)) != std::string::npos)
		{
			if (percentPos + 2 < content.length())
			{
				if (content.substr(percentPos, 3) == "%0A")
					content.replace(percentPos, 3, "\n");
				else if (content.substr(percentPos, 3) == "%20")
					content.replace(percentPos, 3, " ");
				else if (content.substr(percentPos, 3) == "%0D")
					content.replace(percentPos, 3, "\r");
			}
			percentPos++;
		}
	}
	else
		content = body; // Fallback
	if (content.empty())
	{
		return (
			"HTTP/1.1 303 See Other\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 0\r\n"
			"Location: /methods.html?error=empty\r\n"
			"\r\n");
	}
	std::string fileName = UPLOAD_PATH + to_string(time(0)) + ".txt";
	while (std::ifstream(fileName.c_str()))
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

std::string method::DELETE(const std::string& request, Server &server)
{
	std::string requestPath;
	size_t start = request.find("DELETE") + 7;
	size_t end = request.find(" ", start);
	if (start == std::string::npos || end == std::string::npos) throw std::runtime_error(ERROR_400_RESPONSE);
	requestPath = request.substr(start, end - start);
	
	const LocationConfig* location = server.matchLocation(requestPath);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);
	if (checkPermissions("DELETE", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);
	
	std::string locationRoot = location->getLocationRoot();
	if (locationRoot.empty())
		throw std::runtime_error(ERROR_500_RESPONSE);
		
	std::string filename;
	size_t lastSlash = requestPath.find_last_of('/');
	if (lastSlash != std::string::npos && lastSlash < requestPath.length() - 1) {
		filename = requestPath.substr(lastSlash + 1);
	} else {
		throw std::runtime_error(ERROR_400_RESPONSE); // Invalid path
	}
	
	std::string filePath = locationRoot + filename;
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		throw std::runtime_error(ERROR_404_RESPONSE);
	file.close();
	
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
		throw std::runtime_error(ERROR_500_RESPONSE);
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
		throw std::runtime_error(ERROR_500_RESPONSE);
	return (files);
}

std::string method::generateMethodsPage(bool isRegistered)
{
	std::ifstream file("./www/methods.html");

	if (file.is_open())
	{
		std::string content = gnl(file, isRegistered);
		std::vector<std::string> allFiles = listFiles(UPLOAD_PATH);
		std::string htmlList = generateListCheckHtml(allFiles, UPLOAD_PATH);
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

std::string method::generateListCheckHtml(std::vector<std::string> allFiles, const std::string& path)
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
			currentFile.read(tempBuffer, 30);
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
			throw std::runtime_error(ERROR_500_RESPONSE);
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

std::string method::POST_303_RESPONSE(const std::string& location, bool setCookie) {
	if (setCookie)
	{
		std::string cookieId = cookies::generateCookieId();
		return (
			"HTTP/1.1 303 See Other\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 0\r\n"
			"Set-Cookie: session-id="+ to_string(cookieId) + ";\r\n"
			"Location: " + location + "\r\n"
			"\r\n");
	}
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
	std::vector<std::string> allowedMethods = location->getLocationAllowedMethods();
	if (allowedMethods.empty())
		throw std::runtime_error(ERROR_500_RESPONSE);
	for (std::vector<std::string>::const_iterator it = allowedMethods.begin(); it != allowedMethods.end(); ++it)
	{
		if (*it == type)
			return (true);
	}
	return (false);
}

std::string method::handleCGI(const std::string& request, const std::string& cgiFilePath, int port) {
    (void)port;
    
    // Parse HTTP request
    std::istringstream requestStream(request);
    std::string requestLine;
    std::getline(requestStream, requestLine);
    
    std::istringstream lineStream(requestLine);
    std::string method, fullPath, protocol;
    lineStream >> method >> fullPath >> protocol;
    
    // Extract path and query string
    std::string path, queryString;
    size_t questionMarkPos = fullPath.find('?');
    if (questionMarkPos != std::string::npos) {
        path = fullPath.substr(0, questionMarkPos);
        queryString = fullPath.substr(questionMarkPos + 1);
    } else {
        path = fullPath;
    }
    
    // Parse HTTP headers
    std::map<std::string, std::string> headers;
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && !headerLine.empty() && headerLine != "\r") {
        if (!headerLine.empty() && headerLine[headerLine.length() - 1] == '\r') 
    		headerLine.erase(headerLine.length() - 1);
        
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            headers[headerLine.substr(0, colonPos)] = headerLine.substr(colonPos + 2);
        }
    }
    
    // Extract request body for POST
    std::string requestBody;
    if (method == "POST") {
        std::string bodyLine;
        while (std::getline(requestStream, bodyLine)) {
            if (!requestBody.empty()) requestBody += "\n";
            requestBody += bodyLine;
        }
    }
    
    // Verify CGI script exists and is executable
    struct stat statbuf;
    if (stat(cgiFilePath.c_str(), &statbuf) != 0 || !(statbuf.st_mode & S_IXUSR)) {
        throw std::runtime_error(ERROR_404_RESPONSE);
    }

    // Create pipes
    int stdinPipe[2], stdoutPipe[2];
    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
        throw std::runtime_error(ERROR_500_RESPONSE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(stdinPipe[0]); close(stdinPipe[1]);
        close(stdoutPipe[0]); close(stdoutPipe[1]);
        throw std::runtime_error(ERROR_500_RESPONSE);
    }

    if (pid == 0) {
        // Child process: setup and execute CGI script
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        close(stdinPipe[0]); close(stdinPipe[1]);
        close(stdoutPipe[0]); close(stdoutPipe[1]);

        // Set CGI environment variables
        setenv("REQUEST_METHOD", method.c_str(), 1);
        setenv("QUERY_STRING", queryString.c_str(), 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("SCRIPT_NAME", cgiFilePath.c_str(), 1);
        setenv("SCRIPT_FILENAME", cgiFilePath.c_str(), 1);
        setenv("PATH_INFO", path.c_str(), 1);
        setenv("SERVER_NAME", "localhost", 1);
        setenv("SERVER_PORT", to_string(port).c_str(), 1);
        
        if (method == "POST") {
            std::string contentType = headers.count("Content-Type") ? headers["Content-Type"] : "application/x-www-form-urlencoded";
            setenv("CONTENT_TYPE", contentType.c_str(), 1);
            setenv("CONTENT_LENGTH", to_string(requestBody.length()).c_str(), 1);
        } else {
            setenv("CONTENT_LENGTH", "0", 1);
        }
        
        // Add HTTP headers as environment variables
        for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
			std::string envName = "HTTP_" + it->first;
			for (size_t i = 0; i < envName.length(); ++i) {
				if (envName[i] == '-') envName[i] = '_';
				envName[i] = std::toupper(envName[i]);
			}
			setenv(envName.c_str(), it->second.c_str(), 1);
		}

        execl(cgiFilePath.c_str(), cgiFilePath.c_str(), NULL);
        perror("execl failed");
        exit(1);
        
    } else {
        // Parent process: handle I/O and collect output
        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        // Send POST data if present
        if (method == "POST" && !requestBody.empty()) {
            write(stdinPipe[1], requestBody.c_str(), requestBody.length());
        }
        close(stdinPipe[1]);

        // Read CGI output with timeout
        std::string output;
        char buffer[4096];
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        
        while (true) {
            FD_ZERO(&readfds);
            FD_SET(stdoutPipe[0], &readfds);
            
            int selectResult = select(stdoutPipe[0] + 1, &readfds, NULL, NULL, &timeout);
            if (selectResult <= 0) break;
            
            ssize_t bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) break;
            
            output.append(buffer, bytesRead);
        }
        
        close(stdoutPipe[0]);
        waitpid(pid, NULL, 0);
        
        if (output.empty()) {
            throw std::runtime_error(ERROR_500_RESPONSE);
        }
        
        return parseCGIResponse(output);
    }
}

std::string method::parseCGIResponse(const std::string& cgiOutput) {
    // Find header/body separator
    size_t headerEndPos = cgiOutput.find("\r\n\r\n");
    if (headerEndPos == std::string::npos) {
        headerEndPos = cgiOutput.find("\n\n");
        if (headerEndPos == std::string::npos) {
            // No headers found, treat as pure HTML
            return "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n"
                   "Content-Length: " + to_string(cgiOutput.length()) + "\r\n\r\n" + cgiOutput;
        }
        headerEndPos += 2;
    } else {
        headerEndPos += 4;
    }
    
    std::string cgiHeaders = cgiOutput.substr(0, headerEndPos);
    std::string cgiBody = cgiOutput.substr(headerEndPos);
    
    // Build HTTP response
    std::string httpResponse = "HTTP/1.1 200 OK\r\n";
    bool hasContentType = false;
    
    if (!cgiHeaders.empty()) {
        std::istringstream headerStream(cgiHeaders);
        std::string headerLine;
        
        while (std::getline(headerStream, headerLine)) {
            if (headerLine.empty() || headerLine == "\r") continue;
            
            if (!headerLine.empty() && headerLine[headerLine.length() - 1] == '\r') 
    			headerLine.erase(headerLine.length() - 1);
            
            if (headerLine.find("Content-Type:") != std::string::npos) {
                hasContentType = true;
            }
            httpResponse += headerLine + "\r\n";
        }
    }
    
    if (!hasContentType) {
        httpResponse += "Content-Type: text/html\r\n";
    }
    
    httpResponse += "Content-Length: " + to_string(cgiBody.length()) + "\r\n\r\n" + cgiBody;
    return httpResponse;
}

bool method::isCGIScript(const std::string& filePath) {
    // Check file extension
    if (filePath.find(".py") != std::string::npos || filePath.find(".sh") != std::string::npos)
        return true;
    
    // Check if file is executable
    struct stat statbuf;
    return (stat(filePath.c_str(), &statbuf) == 0 && (statbuf.st_mode & S_IXUSR));
}
