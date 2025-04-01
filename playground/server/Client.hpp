/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 14:15:53 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

class Client
{
	private:
		int					_clientSocketFd;
		int					_clientPort;
		struct sockaddr_in	_clientAddress;
		char				_clientIp[INET_ADDRSTRLEN];
		std::string 		_requestHeader;
		std::string 		_responseHeader;
};

#endif
