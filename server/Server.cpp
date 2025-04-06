/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:16:47 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/06 22:07:48 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port) : _port(port), _serverSocketFd(-1)
{
	std::cout << "\e[34m[" << _port << "]\e[0m\t" << "\e[2mCreating Server\e[0m" << std::endl;
}

void Server::run()
{
	// create socket
	_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocketFd == -1)
		THROW_MSG(_port, "Socket can't be created");
	// Set the socket to be non-blocking
	int retValue = setNonBlocking(_serverSocketFd);
	if (retValue == -1)
	{
		close(_serverSocketFd);
		THROW_MSG(_port, "Failed to retrieve socket flags");
	}
	else if (retValue == -2)
	{
		close(_serverSocketFd);
		THROW_MSG(_port, "Failed to set server socket to non-blocking");
	}
	// init and bind the socket
	initSocketId(_serverSocketId);
	bindSocketFdWithID();
	// set more option for the socket (instant reuse of the addr if closed) // todo check if has to be done
	// listen on the socket
	if (listen(_serverSocketFd, MAX_QUEUE) == -1)
	{
		close(_serverSocketFd);
		THROW_MSG(_port, "Failed to listen on socket");
	}
	std::cout << "\e[34m[" << _port << "]\e[0m\t" << "\e[2mServer listening \e[0m" << std::endl;
	// create epoll fd and add server socket to epoll
	createEpollFd();
	addServerSocketToEpoll();
	// accept incoming connections continuously. This is the main loop of the server which will run until the server is stopped
	struct epoll_event	events[MAX_QUEUE];
	while (true)
	{
		int numEvents = epoll_wait(_epollFd, events, MAX_QUEUE, -1); // give the number of events waiting to be processed
		if (numEvents == -1)
		{
			close(_serverSocketFd);
			close(_epollFd);
			THROW_MSG(_port, "Epoll wait failed");
		}
		for (int i = 0; i < numEvents; i++)
		{
			if (events[i].data.fd == _serverSocketFd)
				acceptClient();
			else {
				if (events[i].events & EPOLLIN)
				{
					retValue = treatMethod(events[i]);
					if (retValue == 0)				// client disconnected
						closeClient(events[i]);
					else if (retValue == -1)		// error occured
					{
						CERR_MSG(_port, "Error processing request");
						closeClient(events[i]);
					}
				}
				else if((events[i].events & EPOLLOUT) || (events[i].events & EPOLLERR))
					closeClient(events[i]);
			}
		}
	}
}


/// @return	-1 if an error function failed
/// 		0 if the request is a client disconnection
/// 		1 if the request is treated
int	Server::treatMethod(struct epoll_event &event)
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
		std::string response = selectMethod(buffer);
		if (send(clientSocketFd, response.c_str(), response.size(), 0) == -1)
			return (-1);
	}
	catch (const std::runtime_error &e)
	{
		std::string errorResponse = method::getErrorHtml(_port, e.what());
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
std::string	Server::selectMethod(char  buffer[BUFFER_SIZE])
{
	std::string	request(buffer);
	if (request.find("GET") != std::string::npos)
		return (method::GET(request, _port));
	else if (request.find("POST") != std::string::npos)
		return (method::POST(request, _port));
	else if (request.find("DELETE") != std::string::npos)
		return (method::DELETE(request, _port));
	else
		throw std::runtime_error(ERROR_405_RESPONSE);
}

void Server::acceptClient()
{
	struct sockaddr_in clientSocketId;
	socklen_t clientSocketLength = sizeof(clientSocketId);
	// accept the connection
	int clientSocketFd = accept(_serverSocketFd, (struct sockaddr *)&clientSocketId, &clientSocketLength);
	if (clientSocketFd == -1)
	{
		CERR_MSG(_port, "Failed to accept client connection");
		return ;
	}
	// set the client socket to be non-blocking
	int retValue = setNonBlocking(clientSocketFd);
	if (retValue == -1)
	{
		CERR_MSG(_port, "Failed to retrieve socket flags");
		sendErrorAndCloseClient(clientSocketFd, ERROR_500_RESPONSE);
		return;
	}
	else if (retValue == -2)
	{
		CERR_MSG(_port, "Failed to set client socket to non-blocking");
		sendErrorAndCloseClient(clientSocketFd, ERROR_500_RESPONSE);
		return;
	}
	// add the client socket to epoll
	struct epoll_event newEventClient;
	newEventClient.events = EPOLLIN | EPOLLET; // edge triggered (enable the possibility to handls partial data)
	newEventClient.data.fd = clientSocketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSocketFd, &newEventClient) == -1)
	{
		CERR_MSG(_port, "Failed to add client socket to epoll");
		sendErrorAndCloseClient(clientSocketFd, ERROR_500_RESPONSE);
		return;
	}
	// add the client to the map
	Client *newClient = new Client(clientSocketFd, clientSocketId, _port);
	_clients.insert(std::make_pair(clientSocketFd, newClient));
}

void Server::sendErrorAndCloseClient(int clientSocketFd, const std::string &errorResponse)
{
	if (send(clientSocketFd, errorResponse.c_str(), errorResponse.size(), 0) == -1)
		CERR_MSG(_port, "Failed to send error response to client");
	close(clientSocketFd);
}

void Server::closeClient(struct epoll_event &event)
{
	int clientSocketFd = event.data.fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientSocketFd, NULL) == -1)
	{
		close(clientSocketFd);
		THROW_MSG(_port, "Failed to remove client socket from epoll");
	}
	COUT_MSG(_port, "Client disconnected");
	_clients.erase(clientSocketFd);
	close(clientSocketFd);
}

void Server::createEpollFd()
{
	_epollFd = epoll_create1(0);
	if (_epollFd == -1)
	{
		close(_serverSocketFd);
		THROW_MSG(_port, "Epoll create failed");
	}
}

void Server::addServerSocketToEpoll()
{
	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = _serverSocketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverSocketFd, &event) == -1)
	{
		close(_serverSocketFd);
		close(_epollFd);
		THROW_MSG(_port, "Failed to add server socket to epoll");
	}
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
void Server::bindSocketFdWithID()
{
	if (bind(_serverSocketFd, (struct sockaddr *)&_serverSocketId, sizeof(_serverSocketId)) == -1)
	{
		close(_serverSocketFd);
		if (errno == EADDRINUSE)
			THROW_MSG(_port, "Port already in use");
		else
			THROW_MSG(_port, "Failed to bind socket");
	}
}
void Server::initSocketId(struct sockaddr_in &socketId)
{
	if (memset(&socketId, 0, sizeof(socketId)) == NULL)
		THROW_MSG(_port, "Failed to initialize socket structure");
	socketId.sin_family = AF_INET;
	socketId.sin_port = htons(_port);
	socketId.sin_addr.s_addr = INADDR_ANY;
}

Server::~Server()
{
	std::cout << "\e[34m[" << _port << "]\e[0m\t" << "\e[2mDestroying Server\e[0m" << std::endl;
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();
	close(_serverSocketFd);
}

int Server::getPort() const
{
	return (_port);
}
