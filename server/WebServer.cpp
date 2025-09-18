/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 15:55:05 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/18 14:16:20 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "Server.hpp"
#include "../misc/Evaluator.hpp"

WebServer::WebServer(Config& configFile)
{
	logs::msg(NOPORT, logs::Blue, "Creating WebServer object", true);
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
}

void	WebServer::shutdown()
{
	logs::msg(NOPORT, logs::Blue, "Initiating shutdown...", true);
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers[i])
			_servers[i]->shutdown();
	}
	for (std::map<int, Server*>::iterator it = _fdsToServer.begin(); it != _fdsToServer.end(); ++it) {
		close(it->first);
	}
	_fdsToServer.clear();
	logs::msg(NOPORT, logs::Green, "Shutdown completed.", true);
}

void	WebServer::initServers(Config &config)
{
	for (size_t i = 0; i < config._servers.size(); i++)
	{
		_servers.push_back(new Server(config._servers[i]._port, config._servers[i]._host, config._servers[i]._root, config._servers[i]._serverName, config._servers[i]._clientBodyLimit, config._servers[i]._errorPages, config._servers[i]._locations, this));
	}
}


void	WebServer::start()
{
	logs::msg(NOPORT, logs::Blue, "Starting WebServer", true);
	// Create the servers
	int sharedEpollFd = epoll_create1(0);
	if (sharedEpollFd == -1)
	{
		CERR_MSG("____", "Failed to create epoll fd");
		return;
	}
	for ( size_t i = 0; i < _servers.size(); i++ )
	{
		_servers[i]->setEpollFd(sharedEpollFd);
		try
		{
			_servers[i]->run();
		}
		catch (const std::runtime_error& e)
		{
			logs::msg(NOPORT, logs::Red, e.what(), true);
			continue;
		}
		const std::vector<int>& serverFds = _servers[i]->getServerSocketFds();
		for (size_t j = 0; j < serverFds.size(); j++)
			_fdsToServer[serverFds[j]] = _servers[i];
	}
	// check if no server created
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
		CERR_MSG("____", "No server created");
		close(sharedEpollFd);
		return;
	}
	evenLoop(sharedEpollFd);
}

void	WebServer::evenLoop(int sharedEpollFd)
{
	struct epoll_event events[MAX_QUEUE];
	try {
		while (!SignalHandler::shouldShutdown())
		{
			int numEvents = epoll_wait(sharedEpollFd, events, MAX_QUEUE, 2000);
			
			if (SignalHandler::shouldShutdown())
				break;
			if (numEvents == -1)
				THROW_MSG("____", "Epoll wait failed");
			if (numEvents == 0)
				continue;
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
						server->closeClient(events[i], server->getPort());
						continue;
					}
					if (events[i].events & EPOLLERR)
						server->closeClient(events[i], clientPort);
					else {
						int retValue = server->treatMethod(events[i], clientPort);
						if (retValue == 0 || retValue == -1)
							server->closeClient(events[i], clientPort);
					}
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		logs::msg(NOPORT, logs::Red, e.what(), true);
	}
	shutdown();
	close(sharedEpollFd);
}

void	WebServer::registerClientFd(int fd, Server* server)
{
	_fdsToServer[fd] = server;
}

void	WebServer::unregisterClientFd(int fd)
{
	_fdsToServer.erase(fd);
}
