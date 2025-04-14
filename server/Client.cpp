/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:12:55 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/14 15:05:27 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int clientSocketFd, struct sockaddr_in clientSocketId, int serverPort)
	: _clientSocketFd(clientSocketFd), _clientSocketId(clientSocketId), _serverPort(serverPort)
{
	std::cout << "\e[34m[" << _serverPort << "]\e[0m\t" << "\e[2mCreating Client object\e[0m" << std::endl;
	_clientPort = ntohs(_clientSocketId.sin_port);
	inet_ntop(AF_INET, &_clientSocketId.sin_addr, _clientIp, INET_ADDRSTRLEN);
}
Client::~Client()
{
	close(_clientSocketFd);
	std::cout << "\e[2mClient " << _clientIp << ":" << _clientPort << " disconnected\e[0m" << std::endl;
}
const char*	Client::getClientIp() const {
	return (_clientIp);
}

int	Client::getClientPort() const {
	return (_clientPort);
}
