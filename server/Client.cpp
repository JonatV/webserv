/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:12:55 by jveirman          #+#    #+#             */
/*   Updated: 2025/06/04 14:06:39 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int clientSocketFd, struct sockaddr_in clientSocketId, int serverPort)
	: _clientSocketFd(clientSocketFd), _clientSocketId(clientSocketId), _serverPort(serverPort), _isRegistered(false), _cookies()
{
	std::cout << "\e[34m[" << _serverPort << "]\e[0m\t" << "\e[2mCreating Client object\e[0m" << std::endl;
	inet_ntop(AF_INET, &_clientSocketId.sin_addr, _clientIp, INET_ADDRSTRLEN);
}
Client::~Client()
{
	close(_clientSocketFd);
	std::cout << "\e[2mClient " << _clientIp << " disconnected\e[0m" << std::endl;
}
const char*	Client::getClientIp() const {
	return (_clientIp);
}

int	Client::getClientPort() const {
	return (_serverPort);
}

bool	Client::isRegistered() const {
	return (_isRegistered);
}

std::map<std::string, std::string> Client::getCookies() const {
	return (_cookies);
}

void	Client::setRegistered(bool registered) {
	_isRegistered = registered;
}

void	Client::setCookies(std::map<std::string, std::string> cookies) {
	_cookies = cookies;
}
