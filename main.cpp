/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 02:47:49 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/04 15:35:03 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
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
	if (!checkPorts(ac, av, ports))
		return (1);	// print the ports

	// create the server
	// declare the web server
	std::string configFile = "config.conf";
	WebServer webServer(configFile, ports);
	webServer.start();
}


