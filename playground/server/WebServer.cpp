/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 15:55:05 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/02 04:46:19 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer(std::string& configFile, std::vector<int> ports) : _configFile(configFile), _ports(ports) // dev
{
	std::cout << "\e[2mCreating WebServer object\e[0m" << std::endl;
}

WebServer::WebServer(std::string& configFile) : _configFile(configFile)
{
	std::cout << "\e[2mCreating WebServer object\e[0m" << std::endl;
}

WebServer::~WebServer()
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		delete _servers[i];
	}
	_servers.clear();
	_ports.clear(); //dev 
	_configFile.clear(); //dev 
	std::cout << "\e[2mDestroying WebServer object\e[0m" << std::endl;
}

void WebServer::start()
{
	std::cout << "\e[2mStarting WebServer\e[0m" << std::endl;
	// Create the servers
	dev_addServer(_ports);
	for ( size_t i = 0; i < _servers.size(); i++ )
	{
		std::cout << "\e[34m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mStarting server \e[0m" << std::endl;
		pid_t pid = fork();
		if (pid == -1)
		{
			std::cerr << "\e[1;37;41mError: Fork failed\e[0m" << std::endl;
			exit(EXIT_FAILURE);
		}
		else if (pid == 0)
		{
			_servers[i]->run();
			std::cout << "\e[1;32;42mServer " << i << " started\e[0m" << std::endl;
			exit(EXIT_SUCCESS);
		}
	}
	// Wait for all child processes to finish
	for (size_t i = 0; i < _servers.size(); i++)
	{
		int status;
		wait(&status);
		if (WIFEXITED(status))
			std::cout << "\e[1;32;42mServer " << i << " exited with status " << WEXITSTATUS(status) << "\e[0m" << std::endl;
		else
			std::cout << "\e[1;37;41mServer " << i << " bad exit\e[0m" << std::endl;
	}
}

void WebServer::dev_addServer(std::vector<int> ports)
{
	for (size_t i = 0; i < ports.size(); i++)
	{
		_servers.push_back(new Server(ports[i]));
	}
}
