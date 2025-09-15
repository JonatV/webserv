/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/08/22 22:38:11 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "method.hpp"
#include "../parse/LocationConfig.hpp"
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
#define UPLOAD_PATH "./www/uploads/"
#define THROW_MSG(port, msg) throw std::runtime_error("\e[31m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m")
#define COUT_MSG(port, msg) std::cout << "\e[34m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m" << std::endl

class WebServer;

class Server
{
	private:
		// Parser informations
		std::vector<int>						_ports;
		std::string								_host;
		std::string								_root;
		std::vector<std::string>				_serverName;
		ssize_t									_clientBodyLimit;
		std::map<int, std::string> 				_errorPages;
		std::map<std::string, LocationConfig>	_locations;

		// Server
		std::vector<int>						_serverSocketFds;
		std::vector<struct sockaddr_in>			_serverSocketIds;
		std::map<int, int>						_socketFdToPort;
		std::map<int, Client *>					_clients;
		int										_epollFd;
		WebServer*								_webServer;
		std::vector<int>						_runningPorts;
		
		// methods
		int					setNonBlocking(int fd);
		void				initSocketId(struct sockaddr_in &socketId, int port);

		std::string 		selectMethod(const char* buffer, int port, bool);
		void				sendErrorAndCloseClient(int clientSocketFd, const std::string &errorResponse, int port);
		int					handleReadEvent(int clientSocketFd, Client* client, int clientPort);
		int					handleWriteEvent(int clientSocketFd, Client* client, int clientPort);
		
		// Prevent Copying
		Server(const Server& other);
		Server& operator=(const Server& other);
		
	public:
		Server(std::vector<int>ports, std::string host, std::string root, std::vector<std::string> serverName, size_t clientBodyLimit, std::map<int, std::string> errorPages, std::map<std::string, LocationConfig> locations, WebServer* webserver);

		~Server();
		void						run();
		void						shutdown();
		void						acceptClient(int fd);
		void						closeClient(struct epoll_event &event, int port);
		int							treatMethod(struct epoll_event &event, int clientPort);
		bool						isServerSocket(int fd);
		const LocationConfig*		matchLocation(std::string& path);

		int							getPort() const;
		std::vector<int>			getServerSocketFds() const;
		int							getClientPort(int clientSocketFd);
		std::vector<int>			getRunningPorts() const;
		std::map<int, std::string>	getErrorPages() const;
		ssize_t						getClientBodyLimit() const;
		std::map<int, Client *>		getClients() const;

		void					setEpollFd(int epollFd);
};

#endif
