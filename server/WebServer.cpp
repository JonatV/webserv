/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 15:55:05 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/16 18:46:17 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "Server.hpp"
#include "../misc/Evaluator.hpp"

WebServer::WebServer(Config& configFile)
{
	std::cout << "\e[2mCreating WebServer object\e[0m" << std::endl;
	Evaluator evaluator;
	initServers(configFile);
}

WebServer::~WebServer()
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		delete _servers[i];
	}
	_servers.clear();
	for (std::map<int, Server*>::iterator it = _fdsToServer.begin(); it != _fdsToServer.end(); ++it) {
		close(it->first);
	}
	_fdsToServer.clear();
	std::cout << "\e[2mDestroying WebServer object\e[0m" << std::endl;
}

void WebServer::shutdown()
{
	std::cout << "\e[2mInitiating graceful shutdown...\e[0m" << std::endl;
	
	// Close all client connections and server sockets
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers[i]) {
			_servers[i]->shutdown();  // We'll need to implement this in Server class
		}
	}
	
	// Close all file descriptors in the map
	for (std::map<int, Server*>::iterator it = _fdsToServer.begin(); it != _fdsToServer.end(); ++it) {
		close(it->first);
	}
	_fdsToServer.clear();
	
	std::cout << "\e[32mGraceful shutdown completed.\e[0m" << std::endl;
}


void WebServer::initServers(Config &config)
{
	for (size_t i = 0; i < config._servers.size(); i++)
	{
		_servers.push_back(new Server(config._servers[i]._port, config._servers[i]._host, config._servers[i]._root, config._servers[i]._serverName, config._servers[i]._clientBodyLimit, config._servers[i]._errorPages, config._servers[i]._locations, this));
	}
}


void WebServer::start()
{
	std::cout << "\e[2mStarting WebServer\e[0m" << std::endl;
	// Create the servers
	int sharedEpollFd = epoll_create1(0);
	if (sharedEpollFd == -1)
	{
		CERR_MSG("____", "Failed to create epoll fd");
		return;
	}
	// Initialize each server and add its sockets to the epoll instance
	for ( size_t i = 0; i < _servers.size(); i++ )
	{
		_servers[i]->setEpollFd(sharedEpollFd);
		try
		{
			_servers[i]->run();
		}
		catch (const std::runtime_error& e)
		{
			std::cout << e.what() << std::endl;
			continue; // Skip this server and proceed with the next
		}
		// add the server to the main socket table (every ports of every servers)
		const std::vector<int>& serverFds = _servers[i]->getServerSocketFds();
		for (size_t j = 0; j < serverFds.size(); j++)
			_fdsToServer[serverFds[j]] = _servers[i];
	}
	// check if no server was created
	bool hasRunningPorts = false;
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers[i]->getRunningPorts().size() > 0)
		{
			hasRunningPorts = true;
			break;
		}
	}
	if (!hasRunningPorts)
	{
		CERR_MSG("____", "No server was created");
		close(sharedEpollFd);
		return;
	}
	// accept incoming connections continuously. This is the main loop of the webserver
	evenLoop(sharedEpollFd);
}

void WebServer::evenLoop(int sharedEpollFd)
{
	struct epoll_event events[MAX_QUEUE];
	try {
		while (!SignalHandler::shouldShutdown())  // Check shutdown flag
		{
			// Use timeout in epoll_wait to check shutdown signal periodically
			int numEvents = epoll_wait(sharedEpollFd, events, MAX_QUEUE, 1000); // 1 second timeout
			
			if (SignalHandler::shouldShutdown()) {
				std::cout << "Shutdown signal received. Closing server..." << std::endl;
				break;
			}
			
			if (numEvents == -1)
			{
				close(sharedEpollFd);
				THROW_MSG("____", "Epoll wait failed");
			}
			
			if (numEvents == 0) {
				// Timeout occurred, continue to check shutdown flag
				continue;
			}
			
			for (int i = 0; i < numEvents; i++)
			{
				Server* server = _fdsToServer[events[i].data.fd];
				if (server == NULL)
					continue;
				if (server->isServerSocket(events[i].data.fd))
					server->acceptClient(events[i].data.fd);
				else
				{
					int clientPort = server->getClientPort(events[i].data.fd);
					if (clientPort == -1)
					{
						CERR_MSG(server->getPort(), "Error getting client port");
						continue;
					}
					if (events[i].events & EPOLLERR)
						server->closeClient(events[i], clientPort);
					int retValue = server->treatMethod(events[i], clientPort);
					std::cout << "\e[36m[" << clientPort << "]\e[0m\t" << "treatMethod returned: " << retValue << std::endl; //dev
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		close(sharedEpollFd);
	}
	
	// Perform graceful shutdown
	shutdown();
	close(sharedEpollFd);
}

void WebServer::registerClientFd(int fd, Server* server)
{
	_fdsToServer[fd] = server;
}

void WebServer::unregisterClientFd(int fd)
{
	_fdsToServer.erase(fd);
}

// error handling
const char *WebServer::err_404::what() const throw()
{
	return ("Error 404: Not Found");
}
