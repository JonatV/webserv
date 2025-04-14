/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 02:47:49 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/09 12:22:47 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
#include "parse/Config.hpp"
#include "parse/LocationConfig.hpp"
#include "parse/ServerConfig.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>

bool checkPorts(int ac, char *av[], std::vector<int> &ports)
{
	if (ac < 2)
		return (std::cout << "Use ./web <port>" << std::endl, false);
	size_t i = 1;
	while (av[i])
	{
		std::string port = av[i];
		for (size_t j = 1; av[j] && j != i; j++)
		{
			if (port == av[j])
				return (std::cout << "Port " << port << " is already used" << std::endl, false);
		}
		if (port.empty())
			return (std::cout << "Port cannot be empty" << std::endl, false);
		if (port[0] == '-')
			return (std::cout << "Port cannot be negative" << std::endl, false);
		if (port.size() > 5 || port.size() < 4)
			return (std::cout << "Port must be between 1024 and 65535" << std::endl, false);
		for (size_t i = 0; i < port.size(); i++)
		{
			if (!isdigit(port[i]))
				return (std::cout << "Port must be a number" << std::endl, false);
		}
		int portNumber = atoi(av[i]);
		if (portNumber < 1024 || portNumber > 65535)
			return (std::cout << "Port must be between 1024 and 65535" << std::endl, false);
		ports.push_back(portNumber);
		i++;
	}
	return (std::cout << "Port(s) is(are) \e[32mvalid\e[0m" << std::endl, true);
}

int	main(int ac, char *av[])
{
	std::vector<int> ports;
	// port check //dev
	// if (!checkPorts(ac, av, ports))
	// 	return (1);	// print the ports

	// create the server
	// declare the web server
	std::string configFile = "config.conf";
	WebServer webServer(configFile, ports);
	webServer.start();
}

// int main(int argc, char **argv) {
// 	if (argc != 2) {
// 		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
// 		return 1;
// 	}

// 	Config config;

// 	try {
// 		bool *resultPtr = config.parseFile(argv[1]);
// 		bool result = *resultPtr;
// 		delete resultPtr;
		
// 		if (result) {
// 			std::cout << "Configuration successfully parsed." << std::endl;
// 			config.displayConfig();
// 			return 0;
// 		}
// 	} 
// 	catch (Config::ConfigException& e) {
// 		std::cerr << "Error parsing configuration: " << e.what() << " (code: " << e.getCode() << ")" << std::endl;
// 		return 1;
// 	}
// 	catch (ServerConfig::ConfigException& e) {
// 		std::cerr << "Error parsing server configuration: " << e.what() << " (code: " << e.getCode() << ")" << std::endl;
// 		return 1;
// 	}
// 	catch (LocationConfig::ConfigException& e) {
// 		std::cerr << "Error parsing location configuration: " << e.what() << " (code: " << e.getCode() << ")" << std::endl;
// 		return 1;
// 	}
// 	catch (const std::exception& e) {
// 		std::cerr << "Unexpected error: " << e.what() << std::endl;
// 		return 1;
// 	}

// 	return 0;
// }
