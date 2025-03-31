/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:17 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/31 19:26:42 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

// ServerConfig Constructor & Destructor
ServerConfig::ServerConfig() : _clientBodyLimit(0) {
    // Initialize with default values
}

ServerConfig::~ServerConfig() {
    // Clean up if needed
}

// Parse port number(s) from configuration tokens
int	ServerConfig::_getPort(std::vector<std::string> tokens, size_t i) {
	int port = -1;

	// Skip the "listen" token
	i++;

	// Check if next token is a valid port number
	if (i < tokens.size()) {
		try {
			port = std::stoi(tokens[i]);
			
			// Basic port validation
			if (port < 0 || port > 65535) {
				throw ERROR_INVALID_PORT;
			}
		} catch (std::exception& e) {
			throw ERROR_INVALID_PORT;
		}
	} else {
		throw ERROR_INVALID_PORT;
	}

	return port;
}

// Parse host address from configuration tokens
std::string	ServerConfig::_getHost(std::vector<std::string> tokens, size_t i) {
	std::string host;

	// Skip the "host" token
	i++;

	// Check if next token is a valid host
	if (i < tokens.size()) {
		host = tokens[i];
		
		// Skip the semicolon if it exists
		if (i + 1 < tokens.size() && tokens[i + 1] == ";") {
			i++;
		}
	} else {
		throw ERROR_INVALID_HOST;
	}

	return host;
}

// Parse client body size limit from configuration tokens
size_t	ServerConfig::_getClientBodyLimit(std::vector<std::string> tokens, size_t i) {
	size_t limit = 0;

	// Skip the "client_max_body_size" token
	i++;

	// Check if next token is a valid size
	if (i < tokens.size()) {
		try {
			limit = std::stoull(tokens[i]);
		} catch (std::exception& e) {
			throw ERROR_INVALID_CLIENT_MAX_BODY_SIZE;
		}
	} else {
		throw ERROR_INVALID_CLIENT_MAX_BODY_SIZE;
	}

	return limit;
}

// Parse server name from configuration tokens
std::string	ServerConfig::_getServerName(std::vector<std::string> tokens, size_t i) {
	std::string name;

	// Skip the "server_name" token
	i++;

	// Check if next token is a valid server name
	if (i < tokens.size()) {
		name = tokens[i];
		
		// Skip the semicolon if it exists
		if (i + 1 < tokens.size() && tokens[i + 1] == ";") {
			i++;
		}
	} else {
		throw ERROR_INVALID_SERVER_NAME;
	}

	return name;
}

// Parse location configuration from tokens
std::map<std::string, LocationConfig>	ServerConfig::_getLocationConfig(std::vector<std::string> tokens, size_t i) {
	std::map<std::string, LocationConfig> locations;

	// Skip the "location" token
	i++;

	// Get the location path
	if (i < tokens.size()) {
		std::string path = tokens[i];
		i++;
		
		// Check for opening brace
		if (i < tokens.size() && tokens[i] == "{") {
			i++;
			
			LocationConfig locationConfig;
			
			// Process location block until closing brace
			while (i < tokens.size() && tokens[i] != "}") {
				if (tokens[i] == "index") {
					std::string index = locationConfig._getIndex(tokens);
					// You'd set this on the locationConfig object
				} else if (tokens[i] == "allowed_methods") {
					std::string methods = locationConfig._getAllowedMethods(tokens);
					// Parse and set allowed methods
				} else if (tokens[i] == "root") {
					std::string root = locationConfig._getRoot(tokens);
					// Set root directory
				} else if (tokens[i] == "autoindex") {
					bool autoindex = locationConfig._getAutoIndex(tokens);
					// Set autoindex flag
				}
				
				i++;
			}
			
			// Add this location config to map
			locations[path] = locationConfig;
			
		} else {
			throw LocationConfig::ERROR_INVALID_LOCATION_BLOCK;
		}
	} else {
		throw LocationConfig::ERROR_INVALID_PREFIX;
	}

	return locations;
}
