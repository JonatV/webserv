/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 15:55:05 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 16:09:15 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer(const std::string& configFile)
{
	std::cout << "\e[2mCreating WebServer object\e[0m" << std::endl;
}

WebServer::~WebServer()
{
	std::cout << "\e[2mDestroying WebServer object\e[0m" << std::endl;
}

void WebServer::start()
{
	for ( size_t i = 0; i < _servers.size(); i++ )
	{
		std::cout << "\e[2mStarting server " << i << "\e[0m" << std::endl;
		pid_t pid = fork();
		if (pid == -1)
		{
			std::cerr << "\e[1;37;41mError: Fork failed\e[0m" << std::endl;
			exit(EXIT_FAILURE);
		}
		else if (pid == 0)
		{
			_servers[i].run();
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
