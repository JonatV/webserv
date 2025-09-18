/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/18 13:57:25 by jveirman         ###   ########.fr       */
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
#define BUFFER_LENGTH 8192 // 8kb 
#define UPLOAD_PATH "./www/uploads/"
#define THROW_MSG(port, msg) throw std::runtime_error("\e[31m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m")

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
		int										setNonBlocking(int fd);
		void									initSocketId(struct sockaddr_in &socketId, int port);
		std::string 							selectMethod(const char* buffer, int port, bool);
		void									sendErrorAndCloseClient(int clientSocketFd, const std::string &errorResponse, int port);
		int										handleReadEvent(Client *client, int clientPort);
		int										handleWriteEvent(Client *client);
		void									switchToWriteMode(int clientSocketFd);
		void									switchToReadMode(int clientSocketFd);
		// Prevent Copying
		Server(const Server& other);
		Server&									operator=(const Server& other);
		
	public:
		// Generic
		Server(std::vector<int>ports, std::string host, std::string root, std::vector<std::string> serverName, size_t clientBodyLimit, std::map<int, std::string> errorPages, std::map<std::string, LocationConfig> locations, WebServer* webserver);
		~Server();
		// methods
		void									run();
		void									shutdown();
		void									acceptClient(int fd);
		void									closeClient(struct epoll_event &event, int port);
		int										treatMethod(struct epoll_event &event, int clientPort);
		bool									isServerSocket(int fd);
		const LocationConfig*					matchLocation(std::string& path);
		// request handling
		void									handleReadHeaders(Client* client);
		void									handleReadBody(Client* client);
		void									handleReadyToRespond(Client* client, char* buffer, int clientPort);
		// request parser
		void									parseRequestHeaders(Client* client);
		void									parseContentLength(const std::string& request, Client* client);
		void									parseKeepAlive(const std::string& request, Client* client);
		// getters
		int										getPort() const;
		std::vector<int>						getServerSocketFds() const;
		int										getClientPort(int clientSocketFd);
		std::vector<int>						getRunningPorts() const;
		std::map<int, std::string>				getErrorPages() const;
		ssize_t									getClientBodyLimit() const;
		std::map<int, Client *>					getClients() const;
		// setters
		void									setEpollFd(int epollFd);
};

#endif
