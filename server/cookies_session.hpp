#ifndef COOKIES_SESSION_HPP
#define COOKIES_SESSION_HPP

#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <string>
#include <map>

namespace cookies
{
	void		cookTheCookies(char buffer[], Client *client);
	bool		parseCookieHeader(std::string request, Client *client);
	bool		checkCookies(std::map<std::string, std::string> cookies);
	std::string	generateCookieId();
}

#endif
