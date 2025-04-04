/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/03 22:54:56 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Server.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

class WebServer
{
	private:
		std::vector<Server *> _servers;
		std::string _configFile;// dev: waiting for the Parser part (could be the filepath)
		std::vector<int> _ports; //dev
	public:
		WebServer(std::string &configFile, std::vector<int> ports); // dev: ports is temporary
		WebServer(std::string &configFile);
		~WebServer();
		void start();

		
		void dispatchServer();
		void waitForChildProcess();

		void	dev_addServer(std::vector<int> ports); //dev


		// error handling
		class err_404 : public std::exception
		{
			public:
				const char *what() const throw();
		};
		
};

#endif
