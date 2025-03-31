/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 14:07:17 by jveirman          #+#    #+#             */
/*   Updated: 2025/03/31 11:06:29 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "method.hpp"
#include <iostream>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>

#define MAX_QUEUE 10

int main(int ac, char const *av[])
{
	// START SECURITY
	if (ac != 2)
		return (std::cout << "Use ./web <port>" << std::endl, 0);
	std::string port = av[1];
	if (port.empty())
		return (std::cout << "Port cannot be empty" << std::endl, 0);
	if (port.size() > 5 || port.size() < 4)
		return (std::cout << "Port must be between 1024 and 65535" << std::endl, 0);
	for (size_t i = 0; i < port.size(); i++)
	{
		if (!isdigit(port[i]))
			return (std::cout << "Port must be a number" << std::endl, 0);
	}
	int portNumber = atoi(av[1]);
	if (portNumber < 1024 || portNumber > 65535)
		return (std::cout << "Port must be between 1024 and 65535" << std::endl, 0);
	std::cout << "Port is valid" << std::endl;
	// ------------------------
	// START BINDING
	// create socket
	int	serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocketFd == -1)
		return (std::cout << "Error: Socket can't be created" << std::endl, 1);
	std::cout << "Socket created" << std::endl;
	// set more option for the socket (instant reuse of the addr if closed)
	// int enableReuse = 1;
	// if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(enableReuse)))
	// 	return (std::cout << "Error: Can't set socket options" << std::endl, 1);
	// init and fill the socket id card
	struct sockaddr_in serverSocket;
	if (memset(&serverSocket, 0, sizeof(serverSocket)) == NULL)
		return (std::cout << "Error: Can't allocate memory to the sockaddr_in" << std::endl, 1);
	serverSocket.sin_family = AF_INET;
	serverSocket.sin_port = htons(portNumber);
	serverSocket.sin_addr.s_addr = INADDR_ANY;
	std::cout << "Socket \e[3m\"id card\"\e[0m created" << std::endl;
	// bind the socket fd with its new id card
	if (bind(serverSocketFd, (struct sockaddr*)&serverSocket, sizeof(serverSocket)) == -1)
	{
		close(serverSocketFd);
		if (errno == EADDRINUSE)
			return (std::cout << "Error: Port already in use" << std::endl, 1);
		else
			return (std::cout << "Error: Can't bind socket" << std::endl, 1);
	}
	std::cout << "Socket binded" << std::endl;
	// which means that the socket has an fd and is binded. Binded means that
	// the socket is linked to an ip and a port.
	// ------------------------
	// START LISTENING
	if (listen(serverSocketFd, MAX_QUEUE) == -1)
	{
		close(serverSocketFd);
		return (std::cout << "Error: Can't listen on socket" << std::endl, 1);
	}
	std::cout << "Socket listening" << std::endl;
	// the socket is now listening for incoming connections. The second argument
	// is the maximum number of connections that can be queued. If the queue is
	// full, the connection will be refused.
	// ------------------------
	// START ACCEPTING
	while (true)
	{
		// create the client socket
		struct sockaddr_in clientSocket;
		socklen_t clientSocketLength = sizeof(clientSocket);
		// accept the connection
		int clientSocketFd = accept(serverSocketFd, (struct sockaddr *)&clientSocket, &clientSocketLength);
		if (clientSocketFd == -1)
		{
			std::cout << "Error: Can't accept connection" << std::endl;
			continue;
		}
		std::cout << "Socket accepted" << std::endl;
		// print the client ip and port
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientSocket.sin_addr, clientIP, INET_ADDRSTRLEN);
		std::cout << "Client connected: " << clientIP << ":" << ntohs(clientSocket.sin_port) << std::endl;
		while (true)
		{
			// receive the requests
			char buffer[2048];
			ssize_t bytesReceived = recv(clientSocketFd, buffer, sizeof(buffer) - 1, 0);
			if (bytesReceived == -1)
			{
				send(clientSocketFd, ERROR_500_RESPONSE.c_str(), ERROR_500_RESPONSE.size(), 0);
				std::cout << "\e[1;37;41mError: 500: Internal error\e[0m" << std::endl;
				break;
			}
			if (bytesReceived == 0)
			{
				send(clientSocketFd, ERROR_400_RESPONSE.c_str(), ERROR_400_RESPONSE.size(), 0);
				std::cout << "\e[1;37;41mError: 400: Bad request\e[0m" << std::endl;
				break;
			}
			if (bytesReceived > (ssize_t)sizeof(buffer))
			{
				send(clientSocketFd, ERROR_413_RESPONSE.c_str(), ERROR_413_RESPONSE.size(), 0);
				std::cout << "\e[1;37;41mError: 413: Request payload too large\e[0m" << std::endl;
				break;
			}
			buffer[bytesReceived] = '\0'; // null-terminate the string
			// print the request
			std::cout << "\e[34m===========================" << std::endl;
			std::cout << "Bytes received: " << bytesReceived << std::endl;
			std::cout << "Buffer size: " << sizeof(buffer) << std::endl;
			std::cout << "Buffer length: " << strlen(buffer) << std::endl;
			std::cout << " Request content: \n" << buffer << std::endl;
			std::cout << "===========================\e[0m" << std::endl;
			// detect the type of request
			std::string response;
			std::string request(buffer);
			if (request.find("Connection: close") != std::string::npos)
			{
				std::cout << "Client requested to close the connection" << std::endl;
				std::cout << "\e[1;32;42mClient requested to close the connection\e[0m" << std::endl;
				break;
			}
			else if (request.find("GET") != std::string::npos)
			{
				std::cout << "GET request" << std::endl;
				response = method::GET(request);
			}
			else if (request.find("POST") != std::string::npos)
			{
				std::cout << "POST request" << std::endl;
			}
			else if (request.find("DELETE") != std::string::npos)
			{
				std::cout << "DELETE request" << std::endl;
			}
			else
			{
				send(clientSocketFd, ERROR_405_RESPONSE.c_str(), ERROR_405_RESPONSE.size(), 0);
				break;
			}
			// print the request type
			// send the response
			send(clientSocketFd, response.c_str(), response.size(), 0);
			std::cout << "\e[35m===========================" << std::endl;
			std::cout << "Bytes sent: " << response.size() << std::endl;
			std::cout << "Response content: \n" << response << std::endl;
			std::cout << "===========================\e[0m" << std::endl;
			// the message is sent to the client. The first argument is the fd of the
			// socket that is connected to the client. The second argument is the message
			// to be sent. The third argument is the size of the message. The last argument
			// is the flags. 0 means no flags. The message is sent in a single packet.
			// The message is a simple HTTP response.
			// The first line is the status line.
			// The second line is the headers.
			// The headers are separated by \r\n.
			// The headers are not mandatory, but they are used to provide information about the response.
		}
		std::cout << "Client socket disconnected" << std::endl;
		close(clientSocketFd);
	}

	// ------------------------
	close(serverSocketFd);
	std::cout << "Socket closed" << std::endl;
	return 0;
}
