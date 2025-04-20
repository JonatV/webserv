/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:16:47 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/19 16:33:22 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "WebServer.hpp"

Server::Server(std::vector<int>ports, std::string host, std::string root, std::vector<std::string> serverName, size_t clientBodyLimit, std::map<int, std::string> errorPages, std::map<std::string, LocationConfig> locations, WebServer* webserver)
: _ports(ports), _host(host), _root(root), _serverName(serverName), _clientBodyLimit(clientBodyLimit), _errorPages(errorPages), _locations(locations), _epollFd(-1), _webServer(webserver), _runningPorts({})
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
	std::string response = "";
	int clientSocketFd = event.data.fd;
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytesReceived = recv(clientSocketFd, buffer, sizeof(buffer) - 1, 0);
	if (bytesReceived == 0)
		return (0);
	if (bytesReceived < 0)
		return (-1);
	buffer[bytesReceived] = '\0';
	try
	{
		std::string response = selectMethod(buffer, clientPort);
		if (send(clientSocketFd, response.c_str(), response.size(), 0) == -1)
			return (-1);
	}
	catch (const std::runtime_error &e)
	{
		std::string errorResponse = method::getErrorHtml(clientPort, e.what(), *this);
		if (errorResponse.empty())
			errorResponse = ERROR_500_RESPONSE;
		if (send(clientSocketFd, errorResponse.c_str(), errorResponse.size(), 0) == -1)
			return (-1);
	}
	return (1);
}

/// @brief find the method in the request and call the corresponding method
/// @param buffer header of the request
/// @return return the response of the method OR an empty string if the method is not allowed
std::string	Server::selectMethod(char buffer[BUFFER_SIZE], int port)
{
	std::string	request(buffer);
	size_t end = request.find(" ");
	if (end == std::string::npos)
		throw std::runtime_error(ERROR_400_RESPONSE);
	std::string methodName = request.substr(0, end);
	if (methodName == "GET")
		return (method::GET(request, port, *this));
	else if (methodName == "POST")
		return (method::POST(request, port, *this));
	else if (methodName == "DELETE")
		return (method::DELETE(request, port, *this));
	else
		throw std::runtime_error(ERROR_405_RESPONSE);
}

void Server::acceptClient(int serverSocketFd)
{
	struct sockaddr_in clientSocketId;
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
	newEventClient.events = EPOLLIN | EPOLLET; // edge triggered (enable the possibility to handls partial data)
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
	const LocationConfig* bestMatch = nullptr;
	size_t bestLength = 0;
	for (std::map<std::string, LocationConfig>::const_iterator it = _locations.begin(); it != _locations.end(); ++it)
	{
		const std::string &locationPath = it->first;
		// exact match
		if (path == locationPath)
		{
			if (locationPath == "/")
			{
				std::cout << "\e[32m======= exact match for root '/' \e[0m" << std::endl;
				return (&it->second);
			}
			if (locationPath.back() == '/' && it->second.getLocationAutoIndex() == false)
			{
				std::cout << "\e[31m======= autoindex OFF for " << locationPath << "\e[0m" << std::endl;
				return (nullptr);
			}
			std::cout << "\e[32m======= exact match for " << path << " is " << locationPath << "\e[0m" << std::endl;
			return (&it->second);
		}
		// prefix match
		if (path.compare(0, locationPath.length(), locationPath) == 0)
		{
			if (locationPath.length() > bestLength)
			{
				bestLength = locationPath.length();
				bestMatch = &it->second;
				std::cout << "======= current best match for " << path << " is " << locationPath << std::endl;
			}
		}
	}
	if (bestMatch && bestMatch->getLocationAutoIndex())
	{
		std::cout << "\e[32m======= best match for " << path << " is " << bestMatch->getLocationRoot() << "\e[0m" << std::endl;
		return (bestMatch);
	}
	std::cout << "\e[31m======= autoindex OFF for " << bestMatch->getLocationRoot() << "\e[0m" << std::endl;
	return (nullptr);
}

int	Server::getClientPort(int fd)
{
	Client *client = _clients[fd];
	if (!client)
		return (-1);
	return (client->getClientPort());
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
