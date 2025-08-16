/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 02:47:49 by jveirman          #+#    #+#             */
/*   Updated: 2025/08/16 16:08:06 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
#include "server/Signals.hpp"
#include "parse/Config.hpp"
#include "parse/LocationConfig.hpp"
#include "parse/ServerConfig.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>

int main(int argc, char **argv) {
	(void)argc;
	
	// Set up signal handlers for graceful shutdown
	SignalHandler::setupSignals();
	
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
	
	if (SignalHandler::shouldShutdown()) {
		std::cout << "Server shutdown completed successfully." << std::endl;
	}
	
	return 0;
}
