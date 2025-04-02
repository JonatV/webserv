/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 23:08:28 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "method.hpp"
#include <vector>
#include <map>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_QUEUE 10
#define BUFFER_SIZE 2048

class Server
{
	private:
		int						_port; // Parser
		int						_serverSocketFd;
		struct sockaddr_in		_serverSocketId;
		std::map<int, Client>	_clients;
		int						_epollFd;

		// methods
		void				setNonBlocking(int fd);
		void				createEpollFd();
		void				addServerSocketToEpoll();
		void				initSocketId();
		void				bindSocketFdWithID();

		void				acceptClient(struct epoll_event &event);
		void				treatMethod(struct epoll_event &event);
		void				closeClient(struct epoll_event &event);
	public:
		Server(int port);
		~Server();
		void run();
};

#endif
