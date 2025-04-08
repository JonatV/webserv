/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 15:55:05 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/07 17:59:24 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "Server.hpp"

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
	for (std::map<int, Server*>::iterator it = _fdToServer.begin(); it != _fdToServer.end(); ++it) {
		close(it->first);
	}
	_fdToServer.clear();
	std::cout << "\e[2mDestroying WebServer object\e[0m" << std::endl;
}

void WebServer::start()
{
	std::cout << "\e[2mStarting WebServer\e[0m" << std::endl;
	// Create the servers
	dev_addServer(_ports); // dev PARSER
	int sharedEpollFd = epoll_create1(0);
	if (sharedEpollFd == -1)
	{
		CERR_MSG("____", "Failed to create epoll fd");
		return;
	}
	for ( size_t i = 0; i < _servers.size(); i++ )
	{
		std::cout << "\e[34m[" << _servers[i]->getPort() << "]\e[0m\t" << "\e[2mStarting server \e[0m" << std::endl;
		_servers[i]->setEpollFd(sharedEpollFd);
		_servers[i]->run();
		// add the server to the map
		int serverFd = _servers[i]->getServerSocketFd();
		_fdToServer[serverFd] = _servers[i];
	}
	
	// accept incoming connections continuously. This is the main loop of the webserver
	struct epoll_event events[MAX_QUEUE];
	try {
		while (true)
		{
			int numEvents = epoll_wait(sharedEpollFd, events, MAX_QUEUE, -1); // give the number of events waiting to be processed
			if (numEvents == -1)
			{
				close(sharedEpollFd);
				THROW_MSG("____", "Epoll wait failed");
			}
			for (int i = 0; i < numEvents; i++)
			{
				
				Server* server = _fdToServer[events[i].data.fd];
				if (server == NULL)
					continue;
				if (events[i].data.fd == server->getServerSocketFd())
					server->acceptClient();
				else
				{
					if (events[i].events & EPOLLIN)
					{
						int retValue = server->treatMethod(events[i]);
						if (retValue == -1)
						{
							server->closeClient(events[i]);
							CERR_MSG(server->getPort(), "Error processing request");
						}
						else if (retValue == 0)
							server->closeClient(events[i]);
						
					}
					else if ((events[i].events & EPOLLOUT) || (events[i].events & EPOLLERR))
						server->closeClient(events[i]);
				}
				
			}
		}
	}
	catch (const std::exception& e)
	{
		close(sharedEpollFd);
	}
}

void WebServer::dev_addServer(std::vector<int> ports)
{
	for (size_t i = 0; i < ports.size(); i++)
	{
		_servers.push_back(new Server(ports[i], this));
	}
}


void WebServer::registerClientFd(int fd, Server* server)
{
	_fdToServer[fd] = server;
}

void WebServer::unregisterClientFd(int fd)
{
	_fdToServer.erase(fd);
}

// error handling
const char *WebServer::err_404::what() const throw()
{
	return ("Error 404: Not Found");
}
