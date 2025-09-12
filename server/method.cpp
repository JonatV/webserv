#include "cookies_session.hpp"
#include "method.hpp"

std::string method::GET(const std::string& request, int port, Server& server, bool isRegistered)
{
	size_t start = request.find("GET") + 4; // 4 is to go after "GET "
	size_t end = request.find(" ", start);
	if (start == std::string::npos || end == std::string::npos) 
		throw std::runtime_error(ERROR_400_RESPONSE);

	// Cas spécial : register
	if (request.find("GET /register") != std::string::npos)
		return (POST_303_RESPONSE("/index.html", true));

	// Extraire le path et séparer la query string
	std::string fullPath = request.substr(start, end - start);
	std::string path = fullPath;
	
	// *** IMPORTANT: Séparer path et query string pour le matching ***
	size_t questionMarkPos = fullPath.find('?');
	if (questionMarkPos != std::string::npos) {
		path = fullPath.substr(0, questionMarkPos);
		// La query string sera gérée par handleCGI
	}
	
	const LocationConfig* location = server.matchLocation(path);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);

	// Vérifier les permissions
	if (checkPermissions("GET", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);

	// Obtenir les informations de la location
	std::string locationName = location->getLocationName();
	std::string locationRoot = location->getLocationRoot();
	std::string locationIndex = location->getLocationIndex();

	// *** VÉRIFICATION CGI EN PREMIER ***
	// Construire le chemin du fichier AVANT les autres vérifications
	if (!locationRoot.empty() && !locationIndex.empty())
	{
		std::string filePath = locationRoot + locationIndex;
		
		// Check if it's a CGI script
		if (isCGIScript(filePath))
		{
			std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mGET CGI request: " << filePath << "\e[0m" << std::endl;
			return (handleCGI(request, filePath, port)); // *** RETURN ICI - NE PAS CONTINUER ***
		}
	}

	// *** SEULEMENT SI CE N'EST PAS UN CGI, continuer avec les autres vérifications ***
	
	// Vérifier si c'est un répertoire avec autoindex
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
				return (method::foundPage(lastPath, port, isRegistered));
			}
			return (generateAutoIndexPage(location, isRegistered));
		}
		else 
		{
			throw std::runtime_error(ERROR_404_RESPONSE);
		}
	}

	// *** FICHIER STATIQUE ***
	if (locationRoot.empty() || locationIndex.empty())
		throw std::runtime_error(ERROR_500_RESPONSE);
	
	std::string filePath = locationRoot + locationIndex;
	if (!filePath.empty())
		return (method::foundPage(filePath, port, isRegistered));
	else
		throw std::runtime_error(ERROR_404_RESPONSE);
}

std::string method::foundPage(const std::string& filepath, int port, bool isRegistered)
{
	std::ifstream	file(filepath.c_str());
	(void)port; // dev
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
	// Parse la ligne de requête POST
	size_t start = request.find("POST") + 5; // 5 = longueur de "POST "
	size_t end = request.find(" ", start);
	
	if (start == std::string::npos || end == std::string::npos) 
		throw std::runtime_error(ERROR_400_RESPONSE);
	
	std::string fullPathName = request.substr(start, end - start);
	std::string pathName = fullPathName;
	
	// *** IMPORTANT: Séparer path et query string pour le matching ***
	size_t questionMarkPos = fullPathName.find('?');
	if (questionMarkPos != std::string::npos) {
		pathName = fullPathName.substr(0, questionMarkPos);
	}
	
	// Cas spécial : requête de suppression via POST
	if (request.find("POST /delete") != std::string::npos)
		return (checkDeleteRequest(port, request, server));
	
	// Trouver la location correspondante
	const LocationConfig* location = server.matchLocation(pathName);
	if (!location)
		throw std::runtime_error(ERROR_404_RESPONSE);
	
	// Vérifier les permissions POST
	if (checkPermissions("POST", location) == false)
		throw std::runtime_error(ERROR_403_RESPONSE);
	
	// Construire le chemin du fichier
	std::string locationRoot = location->getLocationRoot();
	std::string locationIndex = location->getLocationIndex();
	
	// *** VÉRIFICATION CGI EN PREMIER ***
	if (!locationRoot.empty() && !locationIndex.empty())
	{
		std::string filePath = locationRoot + locationIndex;
		
		if (isCGIScript(filePath))
		{
			std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST CGI request: " << filePath << "\e[0m" << std::endl;
			return (handleCGI(request, filePath, port)); // *** RETURN ICI - NE PAS CONTINUER ***
		}
	}
	
	// *** TRAITEMENT POST CLASSIQUE seulement si ce n'est pas du CGI ***
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mPOST request for: " << pathName << "\e[0m" << std::endl;
	
	// Vérifier la limite de taille du body
	std::string contentLengthHeader;
	size_t contentLengthPos = request.find("Content-Length: ");
	if (contentLengthPos != std::string::npos)
	{
		size_t lineEnd = request.find("\r\n", contentLengthPos);
		if (lineEnd != std::string::npos)
		{
			contentLengthHeader = request.substr(contentLengthPos + 16, lineEnd - contentLengthPos - 16);
			// ssize_t contentLength = std::atoi(contentLengthHeader.c_str()); //wip
			
			// if (contentLength > server.getClientBodyLimit()) //wip
			// { //wip
			// 	std::cout << "\e[31m[" << port << "]\e[0m\t" << "Request body too large: "  //wip
			// 			  << contentLength << " > " << server.getClientBodyLimit() << std::endl; //wip
			// 	std::cout << "\e[31mDebug simple POST \e[0m" << std::endl; //wip
			// 	throw std::runtime_error(ERROR_413_RESPONSE); //wip
			// } //wip
		}
	}
	
	// Déterminer le type de requête POST
	if (request.find("User-Agent: curl") != std::string::npos)
	{
		// POST depuis terminal/curl
		return (postFromTerminal(request, server));
	}
	else if (request.find("Content-Type: multipart/form-data") != std::string::npos)
	{
		// Upload de fichier
		return (handleFileUpload(request, server, port));
	}
	else if (request.find("Content-Type: application/x-www-form-urlencoded") != std::string::npos)
	{
		// Formulaire HTML standard
		return (postFromDashboard(request, server));
	}
	else
	{
		// Fallback : traiter comme formulaire simple
		std::cout << "\e[33m[" << port << "]\e[0m\t" << "Unknown POST content type, treating as form" << std::endl;
		return (postFromDashboard(request, server));
	}
}

// Fonction helper pour gérer l'upload de fichiers
std::string method::handleFileUpload(const std::string& request, Server& server, int port)
{
	std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mFile upload request\e[0m" << std::endl;
	
	// Extraire le boundary du Content-Type
	std::string boundary;
	size_t boundaryPos = request.find("boundary=");
	if (boundaryPos != std::string::npos)
	{
		size_t lineEnd = request.find("\r\n", boundaryPos);
		boundary = request.substr(boundaryPos + 9, lineEnd - boundaryPos - 9);
	}
	
	if (boundary.empty())
	{
		std::cout << "\e[31m[" << port << "]\e[0m\t" << "No boundary found in multipart request" << std::endl;
		throw std::runtime_error(ERROR_400_RESPONSE);
	}
	
	// Pour une implémentation complète de multipart/form-data, il faudrait parser
	// chaque partie séparément. Ici on fait une version simplifiée.
	
	// Extraire le body
	size_t bodyStart = request.find("\r\n\r\n");
	if (bodyStart == std::string::npos)
		throw std::runtime_error(ERROR_400_RESPONSE);
	
	std::string body = request.substr(bodyStart + 4);
	
	// Vérifier la taille
	if ((ssize_t)body.length() > server.getClientBodyLimit())
	{
		std::cout << "\e[31mDebug in handleFileUpload \e[0m" << std::endl;
		throw std::runtime_error(ERROR_413_RESPONSE);
	}
	
	// Générer un nom de fichier unique
	std::string fileName = UPLOAD_PATH + to_string(time(0)) + "_upload.txt";
	while (std::ifstream(fileName.c_str()))
		fileName = UPLOAD_PATH + to_string(time(0)) + "_upload.txt";
	
	// Sauvegarder (version simplifiée - dans un vrai serveur, il faudrait parser le multipart)
	std::ofstream file(fileName.c_str());
	if (file.is_open())
	{
		file << "=== File uploaded via multipart/form-data ===" << std::endl;
		file << "Boundary: " << boundary << std::endl;
		file << "Content length: " << body.length() << std::endl;
		file << "=== Raw content ===" << std::endl;
		file << body;
		file.close();
		
		std::cout << "\e[32m[" << port << "]\e[0m\t" << "File uploaded: " << fileName << std::endl;
		return (POST_201_RESPONSE);
	}
	else
	{
		throw std::runtime_error(ERROR_500_RESPONSE);
	}
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
	{
		std::cout << "\e[31mDebug POSTFROMTERMINAL \e[0m" << std::endl;
		throw std::runtime_error(ERROR_413_RESPONSE);
	}

	// Déterminer l'extension selon le Content-Type
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
		
		// Réponse avec plus d'informations
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

	// Extraire le body
	std::string body = request.substr(request.find("\r\n\r\n") + 4);
	std::string content = "";
	
	// Parser les données de formulaire
	size_t msgStart = body.find("MSG_TEXTAREA=");
	if (msgStart != std::string::npos)
	{
		content = body.substr(msgStart + std::string("MSG_TEXTAREA=").length());
		
		// Décoder URL encoding basique
		size_t plusPos = 0;
		while ((plusPos = content.find("+", plusPos)) != std::string::npos)
		{
			content.replace(plusPos, 1, " ");
			plusPos++;
		}
		
		// Gérer %20, %0A, etc. (version simplifiée)
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
	{
		content = body; // Fallback
	}

	// ssize_t bytesReceived = content.length(); //wip
	// if (bytesReceived > server.getClientBodyLimit()) //wip
	// { //wip
	// 	std::cout << "\e[31mDebug POSTFROMDASHBOARD \e[0m" << std::endl; //wip
	// 	throw std::runtime_error(ERROR_413_RESPONSE); //wip
	// } //wip

	if (content.empty())
	{
		// Rediriger vers la page methods avec un message d'erreur
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

std::string method::POST_303_RESPONSE(const std::string& location, bool setCookie) {
	if (setCookie)
	{
		// create a cookie string
		std::string cookieId = cookies::generateCookieId();
		std::cout << "\033[31m" << "========= cookies id: " << cookieId << "\033[0m" << std::endl;
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
    (void)port; // pour éviter warning unused parameter
    
    // Parse la requête HTTP pour extraire les infos
    std::istringstream requestStream(request);
    std::string requestLine;
    std::getline(requestStream, requestLine);
    
    // Extraire méthode, path et query string
    std::istringstream lineStream(requestLine);
    std::string method, fullPath, protocol;
    lineStream >> method >> fullPath >> protocol;
    
    std::string path, queryString;
    size_t questionMarkPos = fullPath.find('?');
	if (questionMarkPos != std::string::npos) {
		path = fullPath.substr(0, questionMarkPos);
		queryString = fullPath.substr(questionMarkPos + 1);
	} else {
		path = fullPath;
		queryString = "";
	}
    
    // Parse les headers HTTP
    std::map<std::string, std::string> headers;
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && headerLine != "\r" && headerLine != "") {
        if (!headerLine.empty() && headerLine[headerLine.length() - 1] == '\r') 
            headerLine.erase(headerLine.length() - 1);
        
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = headerLine.substr(0, colonPos);
            std::string headerValue = headerLine.substr(colonPos + 2); // +2 pour ": "
            headers[headerName] = headerValue;
        }
    }
    
    // Extraire le body pour POST
    std::string requestBody;
    if (method == "POST") {
        // Le body est tout ce qui reste après les headers
        std::string bodyLine;
        while (std::getline(requestStream, bodyLine)) {
            if (!requestBody.empty()) requestBody += "\n";
            requestBody += bodyLine;
        }
    }
    
    // Check if the CGI script exists and is executable
    struct stat statbuf;
    if (stat(cgiFilePath.c_str(), &statbuf) != 0 || !(statbuf.st_mode & S_IXUSR)) {
        throw std::runtime_error(ERROR_404_RESPONSE);
    }

    // Create pipes for stdin, stdout
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

    if (pid == 0) { // Child process
        // Redirect stdin and stdout
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);

        // Close all pipe ends
        close(stdinPipe[0]); close(stdinPipe[1]);
        close(stdoutPipe[0]); close(stdoutPipe[1]);

        // Set up environment variables
        setenv("REQUEST_METHOD", method.c_str(), 1);
        setenv("QUERY_STRING", queryString.c_str(), 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("SCRIPT_NAME", cgiFilePath.c_str(), 1);
        setenv("SCRIPT_FILENAME", cgiFilePath.c_str(), 1);
        setenv("PATH_INFO", path.c_str(), 1);
        setenv("SERVER_NAME", "localhost", 1);
        setenv("SERVER_PORT", to_string(port).c_str(), 1);
        
        // Headers spécifiques pour POST
        if (method == "POST") {
            std::string contentType = headers.count("Content-Type") ? headers["Content-Type"] : "application/x-www-form-urlencoded";
            std::string contentLength = to_string(requestBody.length());
            
            setenv("CONTENT_TYPE", contentType.c_str(), 1);
            setenv("CONTENT_LENGTH", contentLength.c_str(), 1);
        } else {
            setenv("CONTENT_LENGTH", "0", 1);
        }
        
        // Ajouter les headers HTTP comme variables d'environnement
        for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
            std::string envName = "HTTP_" + it->first;
            // Remplacer - par _ et mettre en majuscules
            for (size_t i = 0; i < envName.length(); ++i) {
                if (envName[i] == '-') envName[i] = '_';
                envName[i] = std::toupper(envName[i]);
            }
            setenv(envName.c_str(), it->second.c_str(), 1);
        }

        // Execute the CGI script
        execl(cgiFilePath.c_str(), cgiFilePath.c_str(), NULL);

        // Si execl échoue
        perror("execl failed");
        exit(1);
        
    } else { // Parent process
        close(stdinPipe[0]);  // On n'a pas besoin de lire depuis stdin
        close(stdoutPipe[1]); // On n'a pas besoin d'écrire vers stdout

        // Pour POST, envoyer le body au script via stdin
        if (method == "POST" && !requestBody.empty()) {
            ssize_t bytesWritten = write(stdinPipe[1], requestBody.c_str(), requestBody.length());
            if (bytesWritten == -1) {
                perror("Failed to write POST data to CGI script");
            }
        }
        close(stdinPipe[1]); // Fermer stdin après écriture

        // Read output from the CGI script
        char buffer[4096];
        std::string output;
        ssize_t bytesRead;
        
        // Timeout pour éviter de rester bloqué
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 10;  // 10 secondes timeout
        timeout.tv_usec = 0;
        
        while (true) {
            FD_ZERO(&readfds);
            FD_SET(stdoutPipe[0], &readfds);
            
            int selectResult = select(stdoutPipe[0] + 1, &readfds, NULL, NULL, &timeout);
            if (selectResult == -1) {
                break; // Erreur
            } else if (selectResult == 0) {
                break; // Timeout
            }
            
            bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) break;
            
            buffer[bytesRead] = '\0';
            output.append(buffer, bytesRead);
        }
        
        close(stdoutPipe[0]);

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);
        
        if (output.empty()) {
            throw std::runtime_error(ERROR_500_RESPONSE);
        }
        
        // Parse CGI response (headers + body)
        std::string cgiResponse = parseCGIResponse(output);
        return cgiResponse;
    }
}

// Fonction helper pour parser la réponse CGI
std::string method::parseCGIResponse(const std::string& cgiOutput) {
    // Séparer headers et body
    size_t headerEndPos = cgiOutput.find("\r\n\r\n");
    if (headerEndPos == std::string::npos) {
        headerEndPos = cgiOutput.find("\n\n");
        if (headerEndPos == std::string::npos) {
            // Pas de séparation headers/body trouvée, traiter comme du HTML pur
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: " + to_string(cgiOutput.length()) + "\r\n\r\n";
            response += cgiOutput;
            return response;
        }
        headerEndPos += 2; // pour "\n\n"
    } else {
        headerEndPos += 4; // pour "\r\n\r\n"
    }
    
    std::string cgiHeaders = cgiOutput.substr(0, headerEndPos);
    std::string cgiBody = cgiOutput.substr(headerEndPos);
    
    // Construire la réponse HTTP finale
    std::string httpResponse = "HTTP/1.1 200 OK\r\n";
    
    // Ajouter les headers du CGI (s'il y en a)
    if (!cgiHeaders.empty()) {
        // Si le CGI a envoyé des headers, les inclure
        std::istringstream headerStream(cgiHeaders);
        std::string headerLine;
        bool hasContentType = false;
        
        while (std::getline(headerStream, headerLine)) {
            if (headerLine.empty() || headerLine == "\r") continue;
            
            if (!headerLine.empty() && headerLine[headerLine.length() - 1] == '\r') 
                headerLine.erase(headerLine.length() - 1);
            
            if (headerLine.find("Content-Type:") != std::string::npos) {
                hasContentType = true;
            }
            
            httpResponse += headerLine + "\r\n";
        }
        
        // Si pas de Content-Type spécifié, ajouter par défaut
        if (!hasContentType) {
            httpResponse += "Content-Type: text/html\r\n";
        }
    } else {
        httpResponse += "Content-Type: text/html\r\n";
    }
    
    // Ajouter Content-Length
    httpResponse += "Content-Length: " + to_string(cgiBody.length()) + "\r\n\r\n";
    
    // Ajouter le body
    httpResponse += cgiBody;
    
    return httpResponse;
}

bool method::isCGIScript(const std::string& filePath)
{
	// Check by extension
	if (filePath.find(".py") != std::string::npos) return true;
	if (filePath.find(".sh") != std::string::npos) return true;
	if (filePath.find(".pl") != std::string::npos) return true;
	if (filePath.find(".cgi") != std::string::npos) return true;
	
	// Check if file is executable
	struct stat statbuf;
	if (stat(filePath.c_str(), &statbuf) == 0) {
		return (statbuf.st_mode & S_IXUSR); // Executable by owner
	}
	
	return false;
}
