/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 14:07:17 by jveirman          #+#    #+#             */
/*   Updated: 2025/03/29 22:24:12 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		// receive the request
		char buffer[2048];
		ssize_t bytesReceived = recv(clientSocketFd, buffer, sizeof(buffer) - 1, 0);
		if (bytesReceived == -1)
		{
			std::cout << "Error: Can't receive data" << std::endl;
			close(clientSocketFd);
			continue;
		}
		// check if the request is empty
		if (bytesReceived == 0)
		{
			std::cout << "Error: Empty request" << std::endl;
			close(clientSocketFd);
			continue;
		}
		// check if the request is too big
		if (bytesReceived > (ssize_t)sizeof(buffer))
		{
			std::cout << "Error: Request too big" << std::endl;
			close(clientSocketFd);
			continue;
		}
		buffer[bytesReceived] = '\0'; // null-terminate the string
		// print the request
		std::cout << "===========================" << std::endl;
		std::cout << "Bytes received: " << bytesReceived << std::endl;
		std::cout << "Buffer size: " << sizeof(buffer) << std::endl;
		std::cout << "Buffer length: " << strlen(buffer) << std::endl;
		std::cout << "Request content: \n" << buffer << std::endl;
		std::cout << "===========================" << std::endl;
		// detect the type of request
		std::string request(buffer);
		if (request.find("GET") != std::string::npos)
		{
			std::cout << "GET request" << std::endl;
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
			std::cout << "Error: Unknown request type" << std::endl;
			close(clientSocketFd);
			continue;
		}
		// print the request type
		// send the response

		const char* response = 
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 101\r\n" // still need to be calculated
			"\r\n"
			"<html><body><h1>Client connected to server, full duplex communication established!</h1></body></html>";
		send(clientSocketFd, response, strlen(response), 0);
		std::cout << "Message sent" << std::endl;
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

	// ------------------------
	close(serverSocketFd);
	std::cout << "Socket closed" << std::endl;
	return 0;
}
