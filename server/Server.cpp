/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:16:47 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/18 11:11:09 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "WebServer.hpp"
#include "cookies_session.hpp"

Server::Server(std::vector<int>ports, std::string host, std::string root, std::vector<std::string> serverName, size_t clientBodyLimit, std::map<int, std::string> errorPages, std::map<std::string, LocationConfig> locations, WebServer* webserver)
: _ports(ports), _host(host), _root(root), _serverName(serverName), _clientBodyLimit(clientBodyLimit), _errorPages(errorPages), _locations(locations), _epollFd(-1), _webServer(webserver), _runningPorts()
{
	for (size_t i = 0; i < ports.size(); i++)
	{
		std::cout << "\e[34m[" << ports[i] << "]\e[0m\t" << "\e[2mCreating Server\e[0m" << std::endl;
	}
}

void Server::run()
{ 
	for (size_t i = 0; i < _ports.size(); i++)
	{
		int port = _ports[i];
		std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[2mStarting server \e[0m" << std::endl;
		int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
		// create socket
		if (serverSocketFd == -1)
			THROW_MSG(port, "Socket can't be created");
		
		// Set SO_REUSEADDR to allow immediate port reuse after shutdown
		int reuse = 1;
		if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
		{
			close(serverSocketFd);
			THROW_MSG(port, "Failed to set SO_REUSEADDR");
		}
		
		// Set the socket to be non-blocking
		int retValue = setNonBlocking(serverSocketFd);
		if (retValue == -1)
		{
			close(serverSocketFd);
			THROW_MSG(port, "Failed to retrieve socket flags");
		}
		else if (retValue == -2)
		{
			close(serverSocketFd);
			THROW_MSG(port, "Failed to set server socket to non-blocking");
		}
		// init and bind the socket
		struct sockaddr_in serverSocketId;
		initSocketId(serverSocketId, port);
		if (bind(serverSocketFd, (struct sockaddr *)&serverSocketId, sizeof(serverSocketId)) == -1)
		{
			close(serverSocketFd);
			if (errno == EADDRINUSE)
				THROW_MSG(port, "Port already in use");
			else
				THROW_MSG(port, "Failed to bind socket");
		}
		// listen on the socket
		if (listen(serverSocketFd, MAX_QUEUE) == -1)
		{
			close(serverSocketFd);
			THROW_MSG(port, "Failed to listen on socket");
		}
		//add server socket to epoll
		struct epoll_event	event;
		event.events = EPOLLIN;
		event.data.fd = serverSocketFd;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, serverSocketFd, &event) == -1)
		{
			close(serverSocketFd);
			close(_epollFd);
			THROW_MSG(port, "Failed to add server socket to epoll");
		}
		// add the server socket to the vector
		_serverSocketFds.push_back(serverSocketFd);
		_serverSocketIds.push_back(serverSocketId);
		_socketFdToPort[serverSocketFd] = port;
		_runningPorts.push_back(port);
		std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[2mServer listening \e[0m" << std::endl;
	}
}


/// @return	-1 if an error function failed
/// 		0 if the request is a client disconnection
/// 		1 if the request is treated
int	Server::treatMethod(struct epoll_event &event, int clientPort)
{
	int clientSocketFd = event.data.fd;
	Client *client = _clients[clientSocketFd];
	
	if (!client) {
		COUT_MSG(clientPort, "Ignoring event for disconnected client");
		return 0;
	}
	
	if (event.events & EPOLLIN) {
		return handleReadEvent(client, clientPort);
	} else if (event.events & EPOLLOUT) {
		return handleWriteEvent(client, clientPort);
	}
	return -1;
}

int Server::handleReadEvent(Client* client, int clientPort)
{
	char buffer[BUFFER_LENGTH];
	ssize_t bytesRead = recv(client->getClientSocketFd(), buffer, sizeof(buffer) - 1, 0);
	if (bytesRead == -1 || bytesRead == 0)
		return bytesRead;
	buffer[bytesRead] = '\0';
	client->appendToRequestBuffer(buffer);
	if (client->getState() == Client::READING_HEADERS)
	{
		handleReadHeaders(client);
		if (client->getState() == Client::READING_BODY)
			handleReadBody(client);
		if (client->getState() == Client::READY_TO_RESPOND)
			handleReadyToRespond(client, buffer, clientPort);
		
	}
	else if (client->getState() == Client::READING_BODY)
	{
		handleReadBody(client);
		if (client->getState() == Client::READY_TO_RESPOND)
			handleReadyToRespond(client, buffer, clientPort);
	}
	else if (client->getState() == Client::READY_TO_RESPOND)
	{
		handleReadyToRespond(client, buffer, clientPort);
	}
	return 1;
}

void Server::handleReadHeaders(Client* client)
{
	if (client->requestBufferContains("\r\n\r\n", 0) != -1) {
		client->setHeadersComplete(true);
		parseRequestHeaders(client);
	}
}

void Server::handleReadBody(Client* client)
{
	size_t headerEndPos = client->getRequestBuffer().find("\r\n\r\n");
	if (headerEndPos == std::string::npos) return;
	
	size_t bodyStartPos = headerEndPos + 4;
	size_t totalBufferSize = client->getRequestBuffer().size();
	
	if (totalBufferSize > bodyStartPos) {
		size_t currentBodySize = totalBufferSize - bodyStartPos;
		size_t expectedBodySize = client->getExpectedContentLength();
		std::cout << "\e[2mChecker informations: " << currentBodySize << " | " << expectedBodySize << "\e[0m" << std::endl;
		if (currentBodySize >= expectedBodySize) {
			client->setBodyComplete(true);
			client->setParsed(true);
			client->setState(Client::READY_TO_RESPOND);
		}
		else {
			client->setReceivedContentLength(currentBodySize);
			client->setState(Client::READING_BODY);
			std::cout << "Body not complete yet: " << currentBodySize << " / " << expectedBodySize << std::endl;
		}
	}
}

void Server::handleReadyToRespond(Client* client, char* buffer, int clientPort)
{
	try {
		cookies::cookTheCookies(buffer, client);
		std::string response = selectMethod(client->getRequestBuffer().c_str(), clientPort, client->getIsRegisteredCookies());
		client->setResponse(response);
		client->setState(Client::WRITING_RESPONSE);
		switchToWriteMode(client->getClientSocketFd(), clientPort);
	} catch (const std::runtime_error& e) {
		std::string errorResponse = method::getErrorHtml(clientPort, e.what(), *this, client->getIsRegisteredCookies());
		if (errorResponse.empty()) {
			errorResponse = ERROR_500_RESPONSE; // fallback
		}
		client->setResponse(errorResponse);
		client->setState(Client::WRITING_RESPONSE);
		client->setKeepAlive(false);
		switchToWriteMode(client->getClientSocketFd(), clientPort);
	}
}

/// @brief Handle EPOLLOUT events - sending response to client
/// @return -1 error, 0 close connection, 1 continue
int Server::handleWriteEvent(Client* client, int clientPort)
{
	if (client->getState() != Client::WRITING_RESPONSE) {
		std::cout << "\e[31m[" << clientPort << "]\e[0m\t" << "handleWriteEvent called but client not in WRITING_RESPONSE state" << std::endl; //dev
		return -1;
	}
	
	std::string response = client->getResponse();
	std::cout << "\e[36m[" << clientPort << "]\e[0m\t" << "Sending response (" << response.size() << " bytes)" << std::endl; //dev
	
	ssize_t sentNow = send(client->getClientSocketFd(), response.c_str(), response.size(), 0);
	if (sentNow <= 0 || ((size_t)sentNow != response.size())) {
		return 0; // Close connection
	}
	if (client->getKeepAlive()) {
		std::cout << "\e[32m[" << clientPort << "]\e[0m\t" << "Keep-alive: resetting for new request" << std::endl; //dev
		client->resetForNewRequest();
		switchToReadMode(client->getClientSocketFd(), clientPort);
		return 1;
	} else {
		std::cout << "\e[33m[" << clientPort << "]\e[0m\t" << "No keep-alive: closing connection" << std::endl; //dev
		return 0;
	}
}

/// @brief find the method in the request and call the corresponding method
/// @param buffer header of the request
/// @return return the response of the method OR an empty string if the method is not allowed
std::string Server::selectMethod(const char* buffer, int port, bool isRegistered)
{
	std::string	request(buffer);
	size_t end = request.find(" ");
	if (end == std::string::npos)
		throw std::runtime_error(ERROR_400_RESPONSE);
	std::string methodName = request.substr(0, end);
	if (methodName == "GET")
		return (method::GET(request, port, *this, isRegistered));
	else if (methodName == "POST")
		return (method::POST(request, port, *this));
	else if (methodName == "DELETE")
		return (method::DELETE(request, port, *this));
	else
		throw std::runtime_error(ERROR_405_RESPONSE);
}

void Server::acceptClient(int serverSocketFd)
{
	struct sockaddr_in	clientSocketId;
	socklen_t clientSocketLength = sizeof(clientSocketId);
	int port = _socketFdToPort[serverSocketFd];
	// accept the connection
	int clientSocketFd = accept(serverSocketFd, (struct sockaddr *)&clientSocketId, &clientSocketLength);
	if (clientSocketFd == -1)
	{
		CERR_MSG(port, "Failed to accept client connection"); // todo test this for port value check
		return ;
	}
	// set the client socket to be non-blocking
	int retValue = setNonBlocking(clientSocketFd);
	if (retValue == -1)
	{
		CERR_MSG(port, "Failed to retrieve socket flags");
		sendErrorAndCloseClient(clientSocketFd, ERROR_500_RESPONSE, port);
		return;
	}
	else if (retValue == -2)
	{
		CERR_MSG(port, "Failed to set client socket to non-blocking");
		sendErrorAndCloseClient(clientSocketFd, ERROR_500_RESPONSE, port);
		return;
	}
	// add the client socket to epoll
	struct epoll_event newEventClient;
	newEventClient.events = EPOLLIN; // disable edge triggered for clients to allow reading partial data
	newEventClient.data.fd = clientSocketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSocketFd, &newEventClient) == -1)
	{
		CERR_MSG(port, "Failed to add client socket to epoll");
		sendErrorAndCloseClient(clientSocketFd, ERROR_500_RESPONSE, port);
		return;
	}
	// add the client to the map
	Client *newClient = new Client(clientSocketFd, clientSocketId, port);
	_clients.insert(std::make_pair(clientSocketFd, newClient));
	// add the client to the epoll
	_webServer->registerClientFd(clientSocketFd, this);
}

void Server::sendErrorAndCloseClient(int clientSocketFd, const std::string &errorResponse, int port)
{
	if (send(clientSocketFd, errorResponse.c_str(), errorResponse.size(), 0) == -1)
		CERR_MSG(port, "Failed to send error response to client");
	close(clientSocketFd);
	_webServer->unregisterClientFd(clientSocketFd);
}

void Server::closeClient(struct epoll_event &event, int port)
{
	int clientSocketFd = event.data.fd;
	
	// Check if client exists before closing
	if (_clients.find(clientSocketFd) == _clients.end()) {
		std::cout << "\e[33m[" << port << "]\e[0m\t" << "Client " << clientSocketFd << " already closed/doesn't exist" << std::endl; //dev
		return;
	}
	
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientSocketFd, NULL) == -1)
	{
		close(clientSocketFd);
		_webServer->unregisterClientFd(clientSocketFd);
		THROW_MSG(port, "Failed to remove client socket from epoll");
	}
	COUT_MSG(port, "Client disconnected");
	
	// Delete the client object before erasing from map
	delete _clients[clientSocketFd];
	_clients.erase(clientSocketFd);
	close(clientSocketFd);
	_webServer->unregisterClientFd(clientSocketFd);
}

int	Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return (-1);
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return (-2);
	return (0);
}

void Server::initSocketId(struct sockaddr_in &socketId, int port)
{
	if (memset(&socketId, 0, sizeof(socketId)) == NULL)
		THROW_MSG(port, "Failed to initialize socket structure");
	socketId.sin_family = AF_INET;
	socketId.sin_port = htons(port);
	socketId.sin_addr.s_addr = INADDR_ANY;
}

bool Server::isServerSocket(int fd)
{
	for (size_t i = 0; i < _serverSocketFds.size(); i++)
	{
		if (_serverSocketFds[i] == fd)
			return (true);
	}
	return (false);
}

//	exact match :
//		"/delete" - "/uploads/" - "dir/folder/index.html"
//	Ambiguous match:
//		"/uploads/file.txt"				for closest location "/uploads/"
//		"/uploads/assets/profile.txt"	for closest location "/uploads/assets/"
//	
const LocationConfig* Server::matchLocation(std::string& path)
{
	const LocationConfig* bestMatch = NULL;
	size_t bestLength = 0;
	for (std::map<std::string, LocationConfig>::const_iterator it = _locations.begin(); it != _locations.end(); ++it)
	{
		const std::string &locationPath = it->first;
		// exact match
		if (path == locationPath)
		{
			if (locationPath == "/")
			{
				// std::cout << "\e[32m======= exact match for root '/' \e[0m" << std::endl; //dev
				return (&it->second);
			}
			if (locationPath[locationPath.length() - 1] == '/' && it->second.getLocationAutoIndex() == false)
			{
				std::cout << "\e[31m======= autoindex OFF for " << locationPath << "\e[0m" << std::endl;
				return (NULL);
			}
			// std::cout << "\e[32m======= exact match for " << path << " is " << locationPath << "\e[0m" << std::endl; //dev
			return (&it->second);
		}
		// prefix match
		if (path.compare(0, locationPath.length(), locationPath) == 0)
		{
			if (locationPath.length() > bestLength)
			{
				bestLength = locationPath.length();
				bestMatch = &it->second;
				// std::cout << "======= current best match for " << path << " is " << locationPath << std::endl; // dev
			}
		}
	}
	if (bestMatch && bestMatch->getLocationAutoIndex())
	{
		std::cout << "\e[32m======= best match for " << path << " is " << bestMatch->getLocationRoot() << "\e[0m" << std::endl;
		return (bestMatch);
	}
	std::cout << "\e[31m======= autoindex OFF for " << bestMatch->getLocationRoot() << "\e[0m" << std::endl;
	return (NULL);
}

Server::~Server()
{
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();
	for (size_t i = 0; i < _serverSocketFds.size(); i++)
	{
		std::cout << "\e[34m[" << _ports[i] << "]\e[0m\t" << "\e[2mDestroying Server\e[0m" << std::endl;
		close(_serverSocketFds[i]);
	}
}

/*
┌───────────────────────────────────┐
│              GETTER               │
└───────────────────────────────────┘
*/

int Server::getPort() const
{
	return (_ports[0]);
}

std::vector<int> Server::getServerSocketFds() const
{
	return (_serverSocketFds);
}

std::vector<int> Server::getRunningPorts() const
{
	return (_runningPorts);
}

std::map<int, std::string> Server::getErrorPages() const
{
	return (_errorPages);
}

ssize_t Server::getClientBodyLimit() const
{
	return (_clientBodyLimit);
}

std::map<int, Client *> Server::getClients() const
{
	return (_clients);
}

int	Server::getClientPort(int fd)
{
	Client *client = _clients[fd];
	if (!client)
		return (-1);
	return (client->getClientPort());
}

/*
┌───────────────────────────────────┐
│              SETTER               │
└───────────────────────────────────┘
*/

void Server::setEpollFd(int epollFd)
{
	_epollFd = epollFd;
}

/*
┌───────────────────────────────────┐
│              PARSER               │
└───────────────────────────────────┘
*/

void Server::parseRequestHeaders(Client* client)
{
	std::string request = client->getRequestBuffer();
	parseContentLength(request, client);
	parseKeepAlive(request, client);
	if (client->getHasContentLength()) {
		client->setState(Client::READING_BODY);
	} else {
		client->setState(Client::READY_TO_RESPOND);
		client->setParsed(true);
	}
}

void	Server::parseContentLength(const std::string& request, Client* client)
{
	size_t pos = request.find("Content-Length:");
	if (pos != std::string::npos) {
		pos += 15; // Move past "Content-Length:"
		while (pos < request.size() && (request[pos] == ' ' || request[pos] == '\t'))
			++pos; // Skip whitespace
		size_t endPos = request.find("\r\n", pos);
		if (endPos != std::string::npos) {
			std::string lengthStr = request.substr(pos, endPos - pos);
			std::stringstream ss(lengthStr);
			size_t contentLength;
			ss >> contentLength;
			client->setExpectedContentLength(contentLength);
			client->setHasContentLength(true);
		}
	}
}

void	Server::parseKeepAlive(const std::string& request, Client* client)
{
	size_t pos = request.find("Connection:");
	if (pos != std::string::npos) {
		pos += 11; // Move past "Connection:"
		while (pos < request.size() && (request[pos] == ' ' || request[pos] == '\t'))
			++pos; // Skip whitespace
		size_t endPos = request.find("\r\n", pos);
		if (endPos != std::string::npos) {
			std::string connectionValue = request.substr(pos, endPos - pos);
			if (connectionValue == "keep-alive" || connectionValue == "Keep-Alive") {
				client->setKeepAlive(true);
				std::cout << "\e[32m[" << client->getClientPort() << "]\e[0m\t" << "Keep-alive: TRUE (found '" << connectionValue << "')" << std::endl; //dev
			} else {
				client->setKeepAlive(false);
				std::cout << "\e[33m[" << client->getClientPort() << "]\e[0m\t" << "Keep-alive: FALSE (found '" << connectionValue << "')" << std::endl; //dev
			}
		}
	} else {
		client->setKeepAlive(true);
		std::cout << "\e[32m[" << client->getClientPort() << "]\e[0m\t" << "Keep-alive: TRUE (default, no Connection header)" << std::endl; //dev
	}
}

/*
┌───────────────────────────────────┐
│              HELPER               │
└───────────────────────────────────┘
*/

void Server::switchToWriteMode(int clientSocketFd, int clientPort)
{
	(void)clientPort;
	struct epoll_event writeEvent;
	writeEvent.events = EPOLLOUT;
	writeEvent.data.fd = clientSocketFd;
	
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientSocketFd, &writeEvent) == -1) {
		throw std::runtime_error(ERROR_500_RESPONSE);
	}
}

void Server::switchToReadMode(int clientSocketFd, int clientPort)
{
	(void)clientPort;
	struct epoll_event readEvent;
	readEvent.events = EPOLLIN; // Edge triggered is disabled here to allow reading partial data
	readEvent.data.fd = clientSocketFd;
	
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientSocketFd, &readEvent) == -1) {
		throw std::runtime_error(ERROR_500_RESPONSE);
	}
}

void Server::shutdown()
{
	COUT_MSG(getPort(), "Shutting down server...");
	
	// Close all client connections
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second) {
			COUT_MSG(it->second->getClientPort(), "Closing client connection");
			close(it->first);  // Close client socket
		}
	}
	
	// Close all server sockets
	for (size_t i = 0; i < _serverSocketFds.size(); i++)
	{
		COUT_MSG(_ports[i], "Closing server socket");
		close(_serverSocketFds[i]);
	}
	
	COUT_MSG(getPort(), "Server shutdown complete");
}
