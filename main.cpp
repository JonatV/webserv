/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 02:47:49 by jveirman          #+#    #+#             */
/*   Updated: 2025/06/04 13:58:30 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
#include "parse/Config.hpp"
#include "parse/LocationConfig.hpp"
#include "parse/ServerConfig.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>

std::vector<std::vector<int> > parsePorts(int ac, char *av[])
{
	std::vector<std::vector<int> > ports;

	for (int i = 1; i < ac; i++)
	{
		std::vector<int> serverPorts;
		std::string arg(av[i]);
		std::stringstream ss(arg);
		std::string port;

		// Split the argument by spaces
		while (std::getline(ss, port, ' '))
		{
			if (!port.empty())
				serverPorts.push_back(std::atoi(port.c_str())); // Convert to int and add to the server's ports
		}

		ports.push_back(serverPorts); // Add the server's ports to the main vector
	}

	return ports;
}

// int	main(int ac, char *av[])
// {
// 	std::vector<std::vector<int>> ports;
// 	// port check vector constructor //dev
// 	ports = parsePorts(ac, av);
// 	// print the ports
// 	for (size_t i = 0; i < ports.size(); i++)
// 	{
// 		std::cout << "Server " << i + 1 << " ports: ";
// 		for (size_t j = 0; j < ports[i].size(); j++)
// 		{
// 			std::cout << ports[i][j] << " ";
// 		}
// 		std::cout << std::endl;
// 	}
// 	// create the server
// 	// declare the web server
// 	std::string configFile = "config.conf";
// 	WebServer webServer(configFile, ports);
// 	webServer.start();
// }

int main(int argc, char **argv) {
	(void)argc;
	Config config;

	try {
		bool *resultPtr = config.parseFile(argv[1]);
		bool result = *resultPtr;
		delete resultPtr;
		
		if (result) {
			std::cout << "Configuration successfully parsed." << std::endl;
		}
	} 
	catch (Config::ConfigException& e) {
		std::cerr << "Error parsing configuration: " << e.what() << " (code: " << e.getCode() << ")" << std::endl;
		return 1;
	}
	catch (ServerConfig::ConfigException& e) {
		std::cerr << "Error parsing server configuration: " << e.what() << " (code: " << e.getCode() << ")" << std::endl;
		return 1;
	}
	catch (LocationConfig::ConfigException& e) {
		std::cerr << "Error parsing location configuration: " << e.what() << " (code: " << e.getCode() << ")" << std::endl;
		return 1;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected error: " << e.what() << std::endl;
		return 1;
	}


	WebServer webserv(config);
	webserv.start();
	return 0;
}
