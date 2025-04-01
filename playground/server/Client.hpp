/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 18:55:29 by jveirman         ###   ########.fr       */
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
		struct sockaddr_in	_clientAddress;
		int					_clientPort;
		char				_clientIp[INET_ADDRSTRLEN];
		std::string 		_requestHeader;
		std::string 		_responseHeader;
	public:
		Client(int clientSocketFd, struct sockaddr_in clientAddress);
		~Client();
		const char*		getClientIp() const;
		int				getClientPort() const;

};

#endif
