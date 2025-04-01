/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:16:47 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 18:59:43 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port) : _port(port), _serverSocketFd(-1)
{}

void Server::run()
{
	// create socket
	_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocketFd == -1)
		throw std::runtime_error("Socket can't be created");
	// Set the socket to be non-blocking // todo check if has to be done
	setNonBlocking(_serverSocketFd);
	// init and bind the socket
	initSocketId();
	bindSocketFdWithID();
	// set more option for the socket (instant reuse of the addr if closed) // todo check if has to be done
	// listen on the socket
	if (listen(_serverSocketFd, MAX_QUEUE) == -1)
	{
		close(_serverSocketFd);
		throw std::runtime_error("Can't listen on socket");
	}
	std::cout << "Server listening on port \e[1;32;42m" << _port << "\e[0m" << std::endl;
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
				acceptClient(events[i]);
			else
			{
				if (events[i].events & EPOLLIN)
					treatMethod(events[i]);
				else if(events[i].events & EPOLLOUT | EPOLLERR)
					closeClient(events[i]);
			}
		}
	}
}

void Server::acceptClient(struct epoll_event &event)
{
	struct sockaddr_in clientSocket;
	socklen_t clientSocketLength = sizeof(clientSocket);
	// accept the connection
	int clientSocketFd = accept(_serverSocketFd, (struct sockaddr *)&clientSocket, &clientSocketLength);
	if (clientSocketFd == -1)
	{
		// do something
	}
	setNonBlocking(clientSocketFd);
	// add the client socket to epoll
	struct epoll_event newEventClient;
	newEventClient.events = EPOLLIN | EPOLLET; // edge triggered
	newEventClient.data.fd = clientSocketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSocketFd, &newEventClient) == -1)
	{
		close(clientSocketFd);
		throw std::runtime_error("Failed to add client socket to epoll");
	}
	// add the client to the map
	_clients[clientSocketFd] = Client(clientSocketFd, clientSocket);
	// print the client ip and port
	std::cout << "\e[1;32;42mClient connected ip: " << _clients[clientSocketFd].getClientIp() << ":" << _clients[clientSocketFd].getClientPort() << "\e[0m" << std::endl;
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
void Server::initSocketId()
{
	if (memset(&_serverSocketId, 0, sizeof(_serverSocketId)) == NULL)
		throw std::runtime_error("Failed to initialize server socket structure");
	_serverSocketId.sin_family = AF_INET;
	_serverSocketId.sin_port = htons(_port);
	_serverSocketId.sin_addr.s_addr = INADDR_ANY;
}

Server::~Server()
{
	close(_serverSocketFd);
}
