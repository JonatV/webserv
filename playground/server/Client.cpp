/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:12:55 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 18:55:52 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int clientSocketFd, struct sockaddr_in clientAddress)
	: _clientSocketFd(clientSocketFd), _clientAddress(clientAddress)
{
	_clientPort = ntohs(clientAddress.sin_port);
	inet_ntop(AF_INET, &clientAddress.sin_addr, _clientIp, INET_ADDRSTRLEN);
}
Client::~Client()
{
	close(_clientSocketFd);
	std::cout << "\e[1;32;42mClient " << _clientIp << ":" << _clientPort << " disconnected\e[0m" << std::endl;
}
const char*	Client::getClientIp() const {
	return (_clientIp);
}

int	Client::getClientPort() const {
	return (_clientPort);
}
