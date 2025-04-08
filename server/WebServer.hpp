/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/07 17:59:17 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "utils.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sys/epoll.h>
#include <map>
#define MAX_QUEUE 10
#define CERR_MSG(port, msg) std::cerr << "\e[31m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m" << std::endl
#define THROW_MSG(port, msg) throw std::runtime_error("\e[31m[" + to_string(port) + "]\e[0m\t" + "\e[2m" + msg + "\e[0m")

class Server;

class WebServer
{
	private:
		std::vector<Server *> _servers;
		std::string _configFile;// dev: waiting for the Parser part (could be the filepath)
		std::vector<int> _ports; //dev
		std::map<int, Server*> _fdToServer;
	public:
		WebServer(std::string &configFile, std::vector<int> ports); // dev: ports is temporary
		WebServer(std::string &configFile);
		~WebServer();
		void start();
		
		void registerClientFd(int fd, Server* server);
		void unregisterClientFd(int fd);
	
		void	dev_addServer(std::vector<int> ports); //dev
		// error handling
		class err_404 : public std::exception
		{
			public:
				const char *what() const throw();
		};
		
};

#endif
