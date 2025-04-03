/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:16:47 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/03 23:40:13 by jveirman         ###   ########.fr       */
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
	if (_serverSocketFd == -1 )
		throw std::runtime_error("Socket can't be created");
	// Set the socket to be non-blocking // todo check if has to be done
	int retValue = setNonBlocking(_serverSocketFd);
	if (retValue == -1)
	{
		std::cerr << "\e[31m[" << _port << "]\e[0m\t" << "\e[2mFailed to retrieve socket flags\e[0m" << std::endl;
		close(_serverSocketFd);
		exit(EXIT_FAILURE);
	}
	else if (retValue == -2)
	{
		std::cerr << "\e[31m[" << _port << "]\e[0m\t" << "\e[2mFailed to set server socket to non-blocking\e[0m" << std::endl;
		close(_serverSocketFd);
		exit(EXIT_FAILURE);
	}
	// init and bind the socket
	initSocketId(_serverSocketId);
	bindSocketFdWithID();
	// set more option for the socket (instant reuse of the addr if closed) // todo check if has to be done
	// listen on the socket
	if (listen(_serverSocketFd, MAX_QUEUE) == -1)
	{
		close(_serverSocketFd);
		throw std::runtime_error("Can't listen on socket");
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
			throw std::runtime_error("Epoll wait failed");
		}
		for (int i = 0; i < numEvents; i++)
		{
			if (events[i].data.fd == _serverSocketFd)
				acceptClient(); // wip arrive here for the check of the errors
			else {
				if (events[i].events & EPOLLIN)
				{
					if (treatMethod(events[i]) == -1)
						continue;
				}
				else if((events[i].events & EPOLLOUT) || (events[i].events & EPOLLERR))
					closeClient(events[i]);
			}
		}
	}
}

int	Server::treatMethod(struct epoll_event &event)
{
	int clientSocketFd = event.data.fd;
	// read the data from the client
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytesReceived = recv(clientSocketFd, buffer, sizeof(buffer) - 1, 0);
	//todo check bytesReveived error
	if (bytesReceived == 0)
	{
		std::cout << "\e[34m[" << _port << "]\e[0m\t" << "\e[2mClient disconnected\e[0m" << std::endl;
		closeClient(event);
		return (0);
	}
	if (bytesReceived < 0)
	{
		std::cout << "\e[34m[" << _port << "]\e[0m\t" << "\e[1;37;41mError: 500: Internal error\e[0m" << std::endl;
		send(clientSocketFd, ERROR_500_RESPONSE.c_str(), ERROR_500_RESPONSE.size(), 0);
		return (-1);
	}
	buffer[bytesReceived] = '\0';

	std::string response = selectMethod(buffer);
	if (response.empty())
	{
		send(clientSocketFd, ERROR_405_RESPONSE.c_str(), ERROR_405_RESPONSE.size(), 0);
		return (-1); //dev this has to be checked, dont know if it should stop the client connection or not
	}
	send(clientSocketFd, response.c_str(), response.size(), 0);
	return (0);
}

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
	{
		std::cout << "\e[1;37;41mError: 405: Method not allowed\e[0m" << std::endl;
		return ("");
	}
}

void Server::acceptClient()
{
	struct sockaddr_in clientSocketId;
	socklen_t clientSocketLength = sizeof(clientSocketId);
	// accept the connection
	int clientSocketFd = accept(_serverSocketFd, (struct sockaddr *)&clientSocketId, &clientSocketLength);
	if (clientSocketFd == -1)
	{
		std::cerr << "\e[31m[" << _port << "]\e[0m\t" << "\e[2mFailed to accept client connection\e[0m" << std::endl;
		return;
	}
	// set the client socket to be non-blocking
	int retValue = setNonBlocking(clientSocketFd);
	if (retValue == -1)
	{
		std::cout << "\e[31m[" << _port << "]\e[0m\t" << "\e[2mFailed to retrieve socket flags\e[0m" << std::endl;
		sendErrorResponse(clientSocketFd, ERROR_500_RESPONSE);
		return;
	}
	else if (retValue == -2)
	{
		std::cerr << "\e[31m[" << _port << "]\e[0m\t" << "\e[2mFailed to set client socket to non-blocking\e[0m" << std::endl;
		sendErrorResponse(clientSocketFd, ERROR_500_RESPONSE);
		return;
	}
	// add the client socket to epoll
	struct epoll_event newEventClient;
	newEventClient.events = EPOLLIN | EPOLLET; // edge triggered (enable the possibility to handls partial data)
	newEventClient.data.fd = clientSocketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSocketFd, &newEventClient) == -1)
	{
		std::cerr << "\e[31m[" << _port << "]\e[0m\t" << "\e[2mFailed to add client socket to epoll\e[0m" << std::endl;
		sendErrorResponse(clientSocketFd, ERROR_500_RESPONSE);
		return;
	}
	// add the client to the map
	Client *newClient = new Client(clientSocketFd, clientSocketId, _port);
	_clients.insert(std::make_pair(clientSocketFd, newClient));
}

void Server::sendErrorResponse(int clientSocketFd, const std::string &errorResponse)
{
	if (send(clientSocketFd, errorResponse.c_str(), errorResponse.size(), 0) == -1)
	{
		std::cerr << "\e[31m[" << _port << "]\e[0m\t" << "\e[2mFailed to send error response to client\e[0m" << std::endl;
	}
	close(clientSocketFd);
}

void Server::closeClient(struct epoll_event &event)
{
	int clientSocketFd = event.data.fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientSocketFd, NULL) == -1)
	{
		close(clientSocketFd);
		throw std::runtime_error("Failed to remove client socket from epoll");
	}
	_clients.erase(clientSocketFd);
	close(clientSocketFd);
}

void Server::createEpollFd()
{
	_epollFd = epoll_create1(0);
	if (_epollFd == -1)
	{
		close(_serverSocketFd);
		throw std::runtime_error("Epoll create failed");
	}
}

void Server::addServerSocketToEpoll()
{
	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = _serverSocketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverSocketFd, &event) == 0)
	{
		close(_serverSocketFd);
		close(_epollFd);
		throw std::runtime_error("Failed to add server socket to epoll");
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
			throw std::runtime_error("Port already in use");
		else
			throw std::runtime_error("Can't bind socket");
	}
}
void Server::initSocketId(struct sockaddr_in &socketId)
{
	if (memset(&socketId, 0, sizeof(socketId)) == NULL)
		throw std::runtime_error("Failed to initialize a socket structure");
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
