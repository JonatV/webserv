/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/16 18:58:44 by jveirman         ###   ########.fr       */
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
#include <map>
#include <cstdlib>


class Client
{
	public:
		// Client state machine
		enum State { 
			READING_HEADERS,
			READING_BODY,
			READY_TO_RESPOND,
			WRITING_RESPONSE,
			CLOSING
		};
		
	private:
		// settings (id)
		int					_clientSocketFd;
		struct sockaddr_in	_clientSocketId;
		char				_clientIp[INET_ADDRSTRLEN];
		int					_serverPort;
		bool				_isRegisteredCookies;
		// request storage
		std::string			_requestBuffer;		// Accumulates request data
		std::string			_response;			// Response to send
		size_t				_bytesSent;			// Track partial response sends
		// request info
		State				_state;
		bool				_parsed;             // Request fully parsed flag
		bool				_keepAlive;          // Connection: keep-alive flag
		// header info
		bool				_headersComplete;    // Headers fully received flag
		bool				_hasContentLength;
		// body settings
		size_t				_expectedContentLength;
		size_t				_receivedContentLength; // Tracks body bytes received
		bool				_bodyComplete;
		
		// cookies storage
		std::map<std::string, std::string> _cookies;
		
	public:
		Client(int clientSocketFd, struct sockaddr_in clientSocketId, int serverPort);
		~Client();

		void	appendToRequestBuffer(const char* data);
		int		requestBufferContains(const std::string& str, size_t startPos) const;
		void	resetForNewRequest();

		/*
		┌───────────────────────────────────┐
		│              GETTER               │
		└───────────────────────────────────┘
		*/
		int				getClientSocketFd() const;
		const char*		getClientIp() const;
		int				getClientPort() const;
		bool			getIsRegisteredCookies() const;
		std::map<std::string, std::string> getCookies() const;
		State			getState() const;
		std::string		getRequestBuffer() const;
		size_t			getExpectedContentLength() const;
		bool			getHasContentLength() const;
		bool			getKeepAlive() const;
		bool			getParsed() const;
		std::string		getResponse() const;
		size_t			getBytesSent() const;

		/*
		┌───────────────────────────────────┐
		│              SETTER               │
		└───────────────────────────────────┘
		*/
		void			setRegistered(bool registered);
		void			setCookies(std::map<std::string, std::string> cookies);
		void			setState(State state);
		void			setHeadersComplete(bool complete);
		void			setExpectedContentLength(size_t length);
		void			setHasContentLength(bool hasContentLength);
		void			setKeepAlive(bool keepAlive);
		void			setParsed(bool parsed);
		void			setBodyComplete(bool complete);
		void			setResponse(const std::string& response);
		void			setBytesSent(size_t bytes);
};

#endif
