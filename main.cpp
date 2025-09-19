/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 02:47:49 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/19 16:08:33 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
#include "server/Signals.hpp"
#include "parse/Config.hpp"
#include "parse/LocationConfig.hpp"
#include "parse/ServerConfig.hpp"
#include "server/utils.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>

int main(int argc, char **argv) {
	// Basic argument validation
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}
	
	// Set up signal handlers for graceful shutdown
	SignalHandler::setupSignals();
	Config config;

	try {
		bool result = config.parseFile(argv[1]);
		
		if (result)
			logs::msg(NOPORT, logs::Green, "Configuration file parsed successfully", true);
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
