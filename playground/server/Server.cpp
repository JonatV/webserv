/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:16:47 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/02 06:17:21 by jveirman         ###   ########.fr       */
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
		throw std::runtime_error("Socket can't be created");
	// Set the socket to be non-blocking // todo check if has to be done
	setNonBlocking(_serverSocketFd);
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
	// accept incoming connections
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
			{
				acceptClient();
			}
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
		//todo: do something
	}
	// set the client socket to be non-blocking
	// setNonBlocking(clientSocketFd);
	// init the client address
	// initSocketId(clientSocketId);
	// add the client socket to epoll
	struct epoll_event newEventClient;
	newEventClient.events = EPOLLIN | EPOLLET; // edge triggered
	newEventClient.data.fd = clientSocketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSocketFd, &newEventClient) == -1)
	{
		std::cout << "\e[1;37;41mError: Failed to add client socket to epoll\e[0m" << std::endl;
		close(clientSocketFd);
		throw std::runtime_error("Failed to add client socket to epoll");
	}
	// add the client to the map
	Client *newClient = new Client(clientSocketFd, clientSocketId, _port);
	_clients.insert(std::make_pair(clientSocketFd, newClient));
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
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverSocketFd, &event) == -1)
	{
		close(_serverSocketFd);
		close(_epollFd);
		throw std::runtime_error("Failed to add server socket to epoll");
	}
}

void	Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		close(fd);
		throw std::runtime_error("Can't get socket flags");
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		close(fd);
		throw std::runtime_error("Can't set socket to non-blocking");
	}
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
	std::cout << "\e[2mDestroying Server object for " << _port << "\e[0m" << std::endl;
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
