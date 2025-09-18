/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:12:55 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/18 14:51:01 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int clientSocketFd, struct sockaddr_in clientSocketId, int serverPort)
	: _clientSocketFd(clientSocketFd), _clientSocketId(clientSocketId), _serverPort(serverPort), 
	  _isRegisteredCookies(false), _requestBuffer(""), _response(""), _bytesSent(0), _state(READING_HEADERS),
	  _parsed(false), _keepAlive(false), _headersComplete(false), _hasContentLength(false),
	  _expectedContentLength(0), _receivedContentLength(0), _bodyComplete(false), _cookies()
{
	inet_ntop(AF_INET, &_clientSocketId.sin_addr, _clientIp, INET_ADDRSTRLEN);
}

Client::~Client()
{
	close(_clientSocketFd);
}

/*
┌───────────────────────────────────┐
│              METHOD               │
└───────────────────────────────────┘
*/
void	Client::appendToRequestBuffer(const char* data) {
	_requestBuffer.append(data);
}

int	Client::requestBufferContains(const std::string& str, size_t startPos) const {
	int pos = _requestBuffer.find(str, startPos);
	return (pos);
}

/*
┌───────────────────────────────────┐
│              GETTER               │
└───────────────────────────────────┘
*/

int									Client::getClientSocketFd() const {
	return (_clientSocketFd);
}

int									Client::getClientPort() const {
	return (_serverPort);
}

bool								Client::getIsRegisteredCookies() const {
	return (_isRegisteredCookies);
}

std::map<std::string, std::string>	Client::getCookies() const {
	return (_cookies);
}

Client::State						Client::getState() const {
	return (_state);
}

std::string							Client::getRequestBuffer() const {
	return (_requestBuffer);
}

size_t								Client::getExpectedContentLength() const {
	return (_expectedContentLength);
}

bool								Client::getHasContentLength() const {
	return (_hasContentLength);
}

bool								Client::getKeepAlive() const {
	return (_keepAlive);
}

bool								Client::getParsed() const {
	return (_parsed);
}

std::string							Client::getResponse() const {
	return (_response);
}

/*
┌───────────────────────────────────┐
│              SETTER               │
└───────────────────────────────────┘
*/
void								Client::setRegistered(bool registered) {
	_isRegisteredCookies = registered;
}

void								Client::setCookies(std::map<std::string, std::string> cookies) {
	_cookies = cookies;
}

void								Client::setState(State state) {
	_state = state;
}

void								Client::setHeadersComplete(bool complete) {
	_headersComplete = complete;
}

void								Client::setExpectedContentLength(size_t length) {
	_expectedContentLength = length;
}

void								Client::setHasContentLength(bool hasContentLength) {
	_hasContentLength = hasContentLength;
}

void								Client::setKeepAlive(bool keepAlive) {
	_keepAlive = keepAlive;
}

void								Client::setParsed(bool parsed) {
	_parsed = parsed;
}

void								Client::setBodyComplete(bool complete) {
	_bodyComplete = complete;
}

void								Client::setResponse(const std::string& response) {
	_response = response;
}

void								Client::setBytesSent(size_t bytes) {
	_bytesSent = bytes;
}

void								Client::resetForNewRequest() {
	_requestBuffer.clear();
	_response.clear();
	_state = READING_HEADERS;
	_parsed = false;
	_headersComplete = false;
	_hasContentLength = false;
	_expectedContentLength = 0;
	_receivedContentLength = 0;
	_bodyComplete = false;
}
