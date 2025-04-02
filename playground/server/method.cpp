#include "method.hpp"

std::string method::GET(const std::string& request, int port)
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
	else if (path == "/assets/favicon.ico" || path == "/favicon.ico")	// favicon.ico
		filePath = "./www/assets/favicon.ico";
	else if (path == "/404" || path == "/404error.html")			// 404error.html
		filePath = "./www/error_pages/404error.html";
	else
		filePath = "";
	if (!filePath.empty())
		return (method::foundPage(filePath, port));
	else
		return (method::error404Page());
}

std::string method::foundPage(const std::string& filepath, int port)
{
	std::ifstream	file(filepath.c_str());

	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mGET request for file: " << filepath << "\e[0m" << std::endl;
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
		return(method::error404Page());
}

std::string method::error404Page()
{
	std::ifstream	file("./www/error_pages/404error.html");

	if (file.is_open())
	{
		std::string content = gnl(file);
		return (
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content);
	}
	else
		return (ERROR_404_RESPONSE);
}

std::string method::POST(const std::string& request, int port)
{
	if (!PARSER_POST_RIGHT) return (ERROR_403_RESPONSE);
	if (request.find("POST /delete") != std::string::npos)
	{
		std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request for delete\e[0m" << std::endl;
		return (handleDeleteRequest(request));
	}
	
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request\e[0m" << std::endl;
	std::string	content;
	size_t start = request.find("MSG_TEXTAREA=");
	if (start == std::string::npos || request.find("application/x-www-form-urlencoded") == std::string::npos)
		return (ERROR_400_RESPONSE); // bad request
	content = request.substr(start + std::string("MSG_TEXTAREA=").length());
	std::cout << "\t\e[2mcontent: " << content << "\e[0m" << std::endl;
	ssize_t bytesReceived = content.length();
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
			"HTTP/1.1 303 See Other\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 0\r\n"
			"Location: /dashboard.html\r\n"
			"\r\n");
	}
	else
		return (ERROR_500_RESPONSE);
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
	if (!PARSER_DELETE_RIGHT) return (ERROR_403_RESPONSE);
	if (request.find("Content-Length: 0") != std::string::npos)
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 0\r\n"
			"Location: /delete.html\r\n"
			"\r\n");
	if (request.find("=on") == std::string::npos)
		return (ERROR_400_RESPONSE);
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
		std::string filePath = "./www/tmp/" + *it;
		if (std::remove(filePath.c_str()) != 0)
			return (ERROR_500_RESPONSE);
	}
	return (
		"HTTP/1.1 303 See Other\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 0\r\n"
		"Location: /delete.html\r\n"
		"\r\n");
}

std::string method::DELETE(const std::string& request, int port)
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
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mDELETE request for file: " << filePath << "\e[0m" << std::endl;
	if (std::remove(filePath.c_str()) == 0)
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 58\r\n"
			"\r\n"
			"<html><body><h1>200 OK</h1><p>File deleted.</p></body></html>");
	else
		return (ERROR_500_RESPONSE);
}

std::vector<std::string> method::listFiles()
{
	std::vector<std::string> files;
	DIR *dir;
	struct dirent *current_entry;

	dir = opendir("./www/tmp/");
	if (dir)
	{
		while ((current_entry = readdir(dir)))
		{
			if (current_entry->d_type == DT_REG && strcmp(current_entry->d_name, ".") != 0)
			{
				files.push_back(current_entry->d_name);
			}
		}
	}
	closedir(dir);
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
			return (ERROR_500_RESPONSE);
		return (
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + to_string(content.length()) + "\r\n"
			"\r\n" + content);
	}
	else
		return (ERROR_404_RESPONSE);
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
			fullList +=
				"<li>"
				"	<label for=\"" + *it + "\">"
				"		<span class=\"file_name\">" + *it + "</span>"
				"		<p class=\"file_content file_content_error\">The file is empty</p>"
				"	</label>"
				"	<input type=\"checkbox\" name=\"" + *it + "\" id=\"" + *it + "\">"
				"</li>";
		}
	}
	fullList +=
		"</ul>"
		"<button type=\"submit\" class=\"bigBtn\">Delete selected files</button>";
	return (fullList);
}

std::string method::gnl(std::ifstream& file)
{
	std::string content = "";
	std::string line;

	while (std::getline(file, line))
		content += line + "\n";
	if (!content.empty())
		content.erase(content.length() - 1);
	file.close();
	return (content);
}
