/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/15 10:29:39 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

class Client
{
	private:
		int					_clientSocketFd;
		struct sockaddr_in	_clientSocketId;
		char				_clientIp[INET_ADDRSTRLEN];
		std::string 		_requestHeader;
		std::string 		_responseHeader;
		int					_serverPort;
	public:
		Client(int clientSocketFd, struct sockaddr_in clientSocketId, int serverPort);
		~Client();
		const char*		getClientIp() const;
		int				getClientPort() const;

};

#endif
