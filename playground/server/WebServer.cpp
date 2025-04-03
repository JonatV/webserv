/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 15:55:05 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/03 22:58:36 by jveirman         ###   ########.fr       */
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
	dev_addServer(_ports); // dev PARSER
	try
	{
		//create subprocesses for each server
		dispatchServer();
	}
	catch (const char* e)
	{
		std::cerr << "\e[1;37;41mError: " << e << "\e[0m" << std::endl; // todo change msg
		exit(EXIT_FAILURE);
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "\e[1;37;41mError: " << e.what() << "\e[0m" << std::endl; // todo change msg
		exit(EXIT_FAILURE);
	}
	catch (const std::exception& e)
	{
		std::cerr << "\e[1;37;41mError: " << e.what() << "\e[0m" << std::endl; // todo change msg
		exit(EXIT_FAILURE);
	}
	// Wait for all child processes to finish (like pipex)
	waitForChildProcess();
}

void WebServer::waitForChildProcess()
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		int status;
		pid_t pid = wait(&status);
		if (pid == -1)
		{
			std::cerr << "\e[31m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mError waiting for child process\e[0m" << std::endl;
			continue;
		}
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) == 0)
				std::cout << "\e[32m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mExited normally\e[0m" << std::endl;
			else
				std::cout << "\e[31m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mExited with error code " << WEXITSTATUS(status) << "\e[0m" << std::endl;
		}
		else if (WIFSIGNALED(status))
			std::cout << "\e[31m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mKilled by signal " << WTERMSIG(status) << "\e[0m" << std::endl;
		else
			std::cerr << "\e[31m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mBad exit\e[0m" << std::endl;
	}
}

void WebServer::dispatchServer()
{
	for ( size_t i = 0; i < _servers.size(); i++ )
	{
		std::cout << "\e[34m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mStarting server \e[0m" << std::endl;
		pid_t pid = fork();
		if (pid == -1)
			throw std::runtime_error("fork() failed");
		if (pid == 0)
		{
			_servers[i]->run();
			exit(EXIT_SUCCESS);
		}
	}
}

void WebServer::dev_addServer(std::vector<int> ports)
{
	for (size_t i = 0; i < ports.size(); i++)
	{
		_servers.push_back(new Server(ports[i]));
	}
}


// error handling
const char *WebServer::err_404::what() const throw()
{
	return ("Error 404: Not Found");
}
