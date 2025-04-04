/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/04 13:55:51 by jveirman         ###   ########.fr       */
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
#include <utility>
#include <cstdlib>

#define MAX_QUEUE 10
#define BUFFER_SIZE 2048
#define THROW_MSG(port, msg) throw std::runtime_error("\e[31m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m")
#define CERR_MSG(port, msg) std::cerr << "\e[31m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m" << std::endl
#define COUT_MSG(port, msg) std::cout << "\e[34m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m" << std::endl

class Server
{
	private:
		int						_port; // Parser
		int						_serverSocketFd;
		struct sockaddr_in		_serverSocketId;
		std::map<int, Client *>	_clients;
		int						_epollFd;

		// methods
		int					setNonBlocking(int fd);
		void				createEpollFd();
		void				addServerSocketToEpoll();
		void				initSocketId(struct sockaddr_in &socketId);
		void				bindSocketFdWithID();

		void				acceptClient();
		void				closeClient(struct epoll_event &event);
		int					treatMethod(struct epoll_event &event);
		std::string			selectMethod(char buffer[BUFFER_SIZE]);
		void				sendErrorResponse(int clientSocketFd, const std::string &errorResponse);

		// Prevent Copying
		Server(const Server& other);
		Server& operator=(const Server& other);
	public:
		Server(int port);
		~Server();
		void run();

		int	getPort() const;
};

#endif
