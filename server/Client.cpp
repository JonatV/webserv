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
	: _clientSocketFd(clientSocketFd), _clientSocketId(clientSocketId), _serverPort(serverPort), 
	  _isRegistered(false), _cookies(), _expectedContentLength(0), _hasContentLength(false)
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

// State machine methods
void Client::appendRequestData(const char* data, size_t len) {
	_requestBuffer.append(data, len);
}

bool Client::hasCompleteRequest() {
	// Debug: Show current buffer state
	std::cout << "\e[90m[DEBUG] Buffer size: " << _requestBuffer.length() 
			  << ", Has Content-Length: " << (_hasContentLength ? "YES" : "NO") << "\e[0m" << std::endl;
	
	// Enhanced debug: Show buffer content around potential boundaries
	if (_requestBuffer.length() > 10) {
		std::string debugBuffer = _requestBuffer;
		// Replace control chars for visibility
		for (size_t i = 0; i < debugBuffer.length(); i++) {
			if (debugBuffer[i] == '\r') debugBuffer[i] = 'R';
			if (debugBuffer[i] == '\n') debugBuffer[i] = 'N';
		}
		
		// Show last 100 characters to see where we are
		size_t startPos = debugBuffer.length() > 100 ? debugBuffer.length() - 100 : 0;
		std::cout << "\e[90m[DEBUG] Buffer tail: ..." << debugBuffer.substr(startPos) << "\e[0m" << std::endl;
	}
	
	// Check if we have complete headers
	size_t headerEnd = _requestBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		std::cout << "\e[90m[DEBUG] Headers incomplete - no \\r\\n\\r\\n found\e[0m" << std::endl;
		
		// Additional check: look for partial delimiters near the end
		if (_requestBuffer.length() >= 3) {
			std::string tail = _requestBuffer.substr(_requestBuffer.length() - 3);
			if (tail.find("\r\n\r") != std::string::npos || tail.find("\n\r\n") != std::string::npos) {
				std::cout << "\e[90m[DEBUG] Potential split delimiter detected\e[0m" << std::endl;
			}
		}
		return false; // Headers not complete yet
	}
	
	std::cout << "\e[90m[DEBUG] Headers complete at position: " << headerEnd << "\e[0m" << std::endl;
	
	// If no Content-Length parsed yet, check for it
	if (!_hasContentLength) {
		size_t contentLengthPos = _requestBuffer.find("Content-Length:");
		if (contentLengthPos == std::string::npos) {
			// Also check case variations
			contentLengthPos = _requestBuffer.find("content-length:");
			if (contentLengthPos == std::string::npos) {
				contentLengthPos = _requestBuffer.find("Content-length:");
			}
		}
		
		if (contentLengthPos == std::string::npos) {
			std::cout << "\e[90m[DEBUG] No Content-Length found - request complete\e[0m" << std::endl;
			return true; // No body expected
		}
		
		// Parse Content-Length
		size_t lineEnd = _requestBuffer.find("\r\n", contentLengthPos);
		if (lineEnd != std::string::npos) {
			// Find the colon and skip whitespace
			size_t colonPos = _requestBuffer.find(":", contentLengthPos);
			if (colonPos != std::string::npos && colonPos < lineEnd) {
				size_t valueStart = colonPos + 1;
				while (valueStart < lineEnd && (_requestBuffer[valueStart] == ' ' || _requestBuffer[valueStart] == '\t')) {
					valueStart++;
				}
				
				std::string contentLengthStr = _requestBuffer.substr(valueStart, lineEnd - valueStart);
				_expectedContentLength = atoi(contentLengthStr.c_str());
				_hasContentLength = true;
				std::cout << "\e[90m[DEBUG] Content-Length parsed: " << _expectedContentLength << "\e[0m" << std::endl;
			}
		}
	}
	
	// Check if we have complete body
	if (_hasContentLength) {
		size_t bodyStart = headerEnd + 4;
		size_t currentBodySize = _requestBuffer.length() - bodyStart;
		std::cout << "\e[90m[DEBUG] Body: " << currentBodySize << "/" << _expectedContentLength << " bytes\e[0m" << std::endl;
		return currentBodySize >= _expectedContentLength;
	}
	
	return true; // No body expected
}

std::string Client::getCompleteRequest() {
	return _requestBuffer;
}

size_t Client::getRequestBufferSize() const {
	return _requestBuffer.length();
}

void Client::setResponse(const std::string& response) {
	_response = response;
}

const std::string& Client::getResponse() const {
	return _response;
}

void Client::resetForNewRequest() {
	_requestBuffer.clear();
	_response.clear();
	_expectedContentLength = 0;
	_hasContentLength = false;
	std::cout << "\e[90m[DEBUG] Client state reset for new request\e[0m" << std::endl;
}
