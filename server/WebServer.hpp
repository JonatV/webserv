/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/14 15:00:01 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "utils.hpp"
#include "../parse/Config.hpp"
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
		std::vector<Server *>	_servers;
		std::map<int, Server *>	_fdsToServer;
	public:
		WebServer(Config &);
		~WebServer();
		void start();
		
		void registerClientFd(int fd, Server* server);
		void unregisterClientFd(int fd);
		void initServers(Config &config);
	
		// error handling
		class err_404 : public std::exception
		{
			public:
				const char *what() const throw();
		};
		
};

#endif
