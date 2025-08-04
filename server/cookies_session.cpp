#include "cookies_session.hpp"

void cookies::cookTheCookies(char buffer[], Client *client)
{
	std::string request(buffer);
	// check if the its a unregistered client
	if (client->isRegistered())
		return ;
	// check if the request is a GET
	if (request.find("GET") == std::string::npos)
		return ;
	// check if the header has a cookie
	if (!parseCookieHeader(request, client))
		return ;
	// print the cookies
	std::map<std::string, std::string> cookies = client->getCookies();
	std::cout << "\e[34m[" << client->getClientPort() << "]\e[0m\t" << "\e[32mCookies received: \n";
	for (std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it)
		std::cout << "\t" << it->first << " : " << it->second << "\e[0m" << std::endl;
	// check the cookies values
	if (!checkCookies(client->getCookies()))
		throw std::runtime_error(ERROR_400_RESPONSE);
	// set the client as registered
	client->setRegistered(true);
}

bool	cookies::parseCookieHeader(std::string request, Client *client)
{
	std::map<std::string, std::string> cookies;
	size_t pos = request.find("Cookie: ");
	size_t end = request.find("\r\n", pos);
	if (pos == std::string::npos || end == std::string::npos)
		return (false);
	pos += 8;
	std::string cookieHeader = request.substr(pos, end - pos);
	std::string cookieName;
	std::string cookieValue;
	size_t start = 0;
	while ((end = cookieHeader.find(";", start)) != std::string::npos)
	{
		std::string cookie = cookieHeader.substr(start, end - start);
		size_t equalPos = cookie.find("="); 
		if (equalPos != std::string::npos)
		{
			cookieName = cookie.substr(0, equalPos);
			cookieValue = cookie.substr(equalPos + 1);
			cookieName.erase(0, cookieName.find_first_not_of(" "));
			cookies[cookieName] = cookieValue;
		}
		start = end + 1;
	}
	if (start < cookieHeader.length())
	{
		std::string cookie = cookieHeader.substr(start);
		size_t equalPos = cookie.find("=");
		if (equalPos != std::string::npos)
		{
			cookieName = cookie.substr(0, equalPos);
			cookieValue = cookie.substr(equalPos + 1);
			cookieName.erase(0, cookieName.find_first_not_of(" "));
			cookies[cookieName] = cookieValue;
		}
	}
	// push the cookies in the client
	client->setCookies(cookies);
	return (true);
}

// add condition to allow more cookies
// current cookies are:
// - session-id
bool cookies::checkCookies(std::map<std::string, std::string> cookies)
{
	for (std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it)
	{
		if (it->first != "session-id")
			return (false);
	}
	// check the session-id value
	if (cookies["session-id"].empty())
		return (false);
	if (cookies["session-id"].length() != 10)
		return (false);
	if (cookies["session-id"].find_first_not_of("0123456789") != std::string::npos)
		return (false);
	return (true);
}

std::string cookies::generateCookieId()
{
	// generate a random cookie id of 10 digits
	std::string cookieId;
	for (int i = 0; i < 10; ++i)
		cookieId += '0' + (rand() % 10);
	return (cookieId);
}
