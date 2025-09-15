/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:16:47 by jveirman          #+#    #+#             */
/*   Updated: 2025/08/22 22:38:31 by eschmitz         ###   ########.fr       */
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
	if (client == NULL)
		throw std::runtime_error(ERROR_500_RESPONSE);

	// Handle based on event type
	if (event.events & EPOLLIN) {
		return handleReadEvent(clientSocketFd, client, clientPort);
	} else if (event.events & EPOLLOUT) {
		return handleWriteEvent(clientSocketFd, client, clientPort);
	}
	
	return -1; // Unknown event type
}

/// @brief Handle EPOLLIN events - reading data from client
/// @return -1 error, 0 close connection, 1 continue
int Server::handleReadEvent(int clientSocketFd, Client* client, int clientPort)
{
	char buffer[BUFFER_SIZE] = {0};
	
	// ONLY ONE recv() call per epoll cycle - this is critical for evaluation
	ssize_t bytesReceived = recv(clientSocketFd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytesReceived == 0) return 0;  // Connection closed
	if (bytesReceived < 0) return -1;  // Error
	
	buffer[bytesReceived] = '\0';
	
	try {
		cookies::cookTheCookies(buffer, client);
		
		// Append received data to client's request buffer
		client->appendRequestData(buffer, bytesReceived);
		
		std::cout << "\e[36m[" << clientPort << "]\e[0m\t" 
				  << "Received " << bytesReceived << " bytes (total: " 
				  << client->getRequestBufferSize() << " bytes)" << std::endl;
		
		// Debug: Show end of current buffer to check for header boundaries
		std::string currentBuffer = client->getCompleteRequest();
		if (currentBuffer.length() > 50) {
			std::string bufferEnd = currentBuffer.substr(currentBuffer.length() - 50);
			// Replace non-printable chars for debug
			for (size_t i = 0; i < bufferEnd.length(); i++) {
				if (bufferEnd[i] == '\r') bufferEnd[i] = '?';
				if (bufferEnd[i] == '\n') bufferEnd[i] = '!';
			}
			std::cout << "\e[35m[" << clientPort << "]\e[0m\t" 
					  << "Buffer end: ..." << bufferEnd << std::endl;
		}
		
		// Check if we have a complete HTTP request
		if (client->hasCompleteRequest()) {
			// Process the complete request
			std::string completeRequest = client->getCompleteRequest();
			std::cout << "\e[32m[" << clientPort << "]\e[0m\t" 
					  << "Complete request received (" << completeRequest.length() << " bytes)" << std::endl;
			
			// Try to generate response - catch errors here
			try {
				std::string response = selectMethod(completeRequest.c_str(), clientPort, client->isRegistered());
				client->setResponse(response);
			}
			catch (const std::runtime_error &e) {
				// Generate error response immediately and set it
				std::string errorResponse = method::getErrorHtml(clientPort, e.what(), *this, client->isRegistered());
				if (errorResponse.empty())
					errorResponse = ERROR_500_RESPONSE;
				
				client->setResponse(errorResponse);
				std::cout << "\e[31m[" << clientPort << "]\e[0m\t" 
						  << "Generated error response for: " << e.what() << std::endl;
			}
			
			// Switch to EPOLLOUT mode for sending response (or error)
			struct epoll_event writeEvent;
			writeEvent.events = EPOLLOUT;
			writeEvent.data.fd = clientSocketFd;
			
			if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientSocketFd, &writeEvent) == -1) {
				std::cout << "\e[31m[" << clientPort << "]\e[0m\t" 
						  << "Failed to switch to EPOLLOUT mode" << std::endl;
				return -1;
			}
			
			std::cout << "\e[33m[" << clientPort << "]\e[0m\t" 
					  << "Switched to write mode" << std::endl;
		}
		// If request not complete, stay in EPOLLIN mode for next cycle
		
		return 1;
	}
	catch (const std::exception &e) {
		// Handle unexpected errors during request reading/parsing
		std::cout << "\e[31m[" << clientPort << "]\e[0m\t" 
				  << "Unexpected error during request reading: " << e.what() << std::endl;
		return -1;
	}
}

/// @brief Handle EPOLLOUT events - sending response to client
/// @return -1 error, 0 close connection, 1 continue
int Server::handleWriteEvent(int clientSocketFd, Client* client, int clientPort)
{
	const std::string& response = client->getResponse();
	
	if (response.empty()) {
		std::cout << "\e[31m[" << clientPort << "]\e[0m\t" 
				  << "No response to send" << std::endl;
		return 0; // Close connection
	}
	
	// ONLY ONE send() call per epoll cycle - this is critical for evaluation
	ssize_t bytesSent = send(clientSocketFd, response.c_str(), response.size(), 0);
	
	if (bytesSent < 0) {
		std::cout << "\e[31m[" << clientPort << "]\e[0m\t" 
				  << "Failed to send response" << std::endl;
		return -1;
	}
	
	std::cout << "\e[32m[" << clientPort << "]\e[0m\t" 
			  << "Sent " << bytesSent << " bytes" << std::endl;
	
	if (bytesSent == (ssize_t)response.size()) {
		// Complete response sent
		std::cout << "\e[32m[" << clientPort << "]\e[0m\t" 
				  << "Response sent successfully" << std::endl;
		
		// Check if we should keep connection alive
		std::string completeRequest = client->getCompleteRequest();
		bool keepAlive = (completeRequest.find("Connection: keep-alive") != std::string::npos) ||
						 (completeRequest.find("Connection: Keep-Alive") != std::string::npos);
		
		if (keepAlive) {
			// Reset client state for new request and switch back to EPOLLIN
			client->resetForNewRequest();
			
			struct epoll_event readEvent;
			readEvent.events = EPOLLIN;
			readEvent.data.fd = clientSocketFd;
			
			if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientSocketFd, &readEvent) == -1) {
				std::cout << "\e[31m[" << clientPort << "]\e[0m\t" 
						  << "Failed to switch back to EPOLLIN mode" << std::endl;
				return 0; // Close on error
			}
			
			std::cout << "\e[33m[" << clientPort << "]\e[0m\t" 
					  << "Keep-alive: switched back to read mode" << std::endl;
			return 1; // Keep connection open
		} else {
			return 0; // Close connection
		}
	} else {
		// Partial send - update response with remaining data
		std::string remaining = response.substr(bytesSent);
		client->setResponse(remaining);
		
		std::cout << "\e[33m[" << clientPort << "]\e[0m\t" 
				  << "Partial send, " << remaining.size() << " bytes remaining" << std::endl;
		return 1; // Continue in EPOLLOUT mode
	}
}

/// @brief find the method in the request and call the corresponding method
/// @param buffer header of the request
/// @return return the response of the method OR an empty string if the method is not allowed
std::string Server::selectMethod(const char* buffer, int port, bool isRegistered)
{
	std::string	request(buffer);
	// std::cout << "\e[34m[" << port << "]\e[0m\t" << "\e[32mRequest received: \n" << request << "\e[0m" << std::endl; //dev
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
	newEventClient.events = EPOLLIN | EPOLLET; // edge triggered (enable the possibility to handle partial data)
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
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientSocketFd, NULL) == -1)
	{
		close(clientSocketFd);
		_webServer->unregisterClientFd(clientSocketFd);
		THROW_MSG(port, "Failed to remove client socket from epoll");
	}
	COUT_MSG(port, "Client disconnected");
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

int	Server::getClientPort(int fd)
{
	Client *client = _clients[fd];
	if (!client)
		return (-1);
	return (client->getClientPort());
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

int Server::getPort() const
{
	return (_ports[0]);
}

std::vector<int> Server::getServerSocketFds() const
{
	return (_serverSocketFds);
}

void Server::setEpollFd(int epollFd)
{
	_epollFd = epollFd;
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
