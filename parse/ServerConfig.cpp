/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:17 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/14 15:11:05 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include <sstream>
#include <cstdlib> // for atoi and strtoul
#include <sys/stat.h>
#include <unistd.h>

// ServerConfig Constructor & Destructor
ServerConfig::ServerConfig() : _root("./www/"), _clientBodyLimit(100000) {
    // Initialize with default values
}

ServerConfig::~ServerConfig() {
    // Clean up if needed
}

std::vector<int> ServerConfig::getPort(std::vector<std::string> tokens, size_t i, size_t &endPos) {
	// Vector to store our ports
	std::vector<int> ports;

	// Check if we have enough tokens for at least the "listen" token
	if (i >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_PORT);
	}

	// Skip the "listen" token
	i++;

	// Find the semicolon position
	size_t semicolonPos = i;
	while (semicolonPos < tokens.size() && tokens[semicolonPos] != ";") {
		semicolonPos++;
	}

	if (semicolonPos >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_PORT); // No semicolon found
	}

	// If no ports specified (just "listen;"), add default port 8080
	if (i == semicolonPos) {
		ports.push_back(8080);
	} else {
		// Process all ports until semicolon
		while (i < semicolonPos) {
			try {
				int port = -1;
				std::istringstream iss(tokens[i]);
				iss >> port;
				
				if (iss.fail()) {
					throw ConfigException(ERROR_INVALID_PORT);
				}
				
				// Basic port validation
				if (port < 1024 || port > 65535) {
					throw ConfigException(ERROR_INVALID_PORT);
				}
				
				ports.push_back(port);
				
			} catch (std::exception& e) {
				throw ConfigException(ERROR_INVALID_PORT);
			}
			
			i++;
		}
	}

	// Return the position after the semicolon
	endPos = semicolonPos + 1;

	return ports;
}

// Parse host address from configuration tokens
std::string *ServerConfig::getHost(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens for at least the "host" token
	if (i >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_HOST);
	}

	std::string host = "127.0.0.1"; // Default host value

	// Skip the "host" token
	i++;

	// Check if there are more tokens and the next token is not a semicolon
	if (i < tokens.size() && tokens[i] != ";") {
		host = tokens[i];
		
		// Basic host validation
		if (host.empty()) {
			throw ConfigException(ERROR_INVALID_HOST);
		}
		
		i++; // Move to the next token after host
	}

	// Check for semicolon
	if (i >= tokens.size() || tokens[i] != ";") {
		throw ConfigException(ERROR_INVALID_HOST); // Missing semicolon
	}

	std::string *result = new std::string(host);
	return result;
}

std::string *ServerConfig::getRoot(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}
	// Skip the "root" token
	i++;
	std::string path = tokens[i];
	// Basic validation
	if (path.empty()) {
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}
	// Check for semicolon
	if (i + 1 >= tokens.size() || tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_ROOT_PATH); // Missing semicolon
	}
	// Check if path is a directory and accessible
	struct stat path_stat;
	if (stat(path.c_str(), &path_stat) != 0) {
		throw ConfigException(ERROR_ROOT_PATH_NOT_FOUND);
	}

	// Check if it's a directory
	if (!S_ISDIR(path_stat.st_mode)) {
		throw ConfigException(ERROR_ROOT_PATH_NOT_DIRECTORY);
	}

	// Check for read permissions
	if (access(path.c_str(), R_OK) != 0) {
		throw ConfigException(ERROR_ROOT_PATH_NO_ACCESS);
	}

	std::string *result = new std::string(path);
	return result;
}

// Parse client body size limit from configuration tokens
size_t *ServerConfig::getClientBodyLimit(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_CLIENT_MAX_BODY_SIZE);
	}
	// Skip the "client_max_body_size" token
	i++;

	size_t limit = 0;

	// Check if next token is a valid size
	try {
		// C++98 compatible conversion from string to size_t
		std::istringstream iss(tokens[i]);
		iss >> limit;
		if (iss.fail()) {
			throw ConfigException(ERROR_INVALID_CLIENT_MAX_BODY_SIZE);
		}
		// Basic validation
		if (limit == 0) {
			throw ConfigException(ERROR_INVALID_CLIENT_MAX_BODY_SIZE);
		}
	} catch (std::exception& e) {
		throw ConfigException(ERROR_INVALID_CLIENT_MAX_BODY_SIZE);
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_CLIENT_MAX_BODY_SIZE); // Missing semicolon
	}
	size_t *result = new size_t(limit);
	return result;
}

// Parse server name from configuration tokens
std::string *ServerConfig::getServerName(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_SERVER_NAME);
	}
	// Skip the "server_name" token
	i++;
	std::string name = tokens[i];
	// Basic validation
	if (name.empty()) {
		throw ConfigException(ERROR_INVALID_SERVER_NAME);
	}
	// Check for semicolon
	if (i + 1 >= tokens.size() || tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_SERVER_NAME); // Missing semicolon
	}
	std::string *result = new std::string(name);
	return result;
}

// Parse location configuration from tokens
std::map<std::string, LocationConfig> *ServerConfig::getLocationConfig(std::vector<std::string> tokens, size_t& i) {
	std::map<std::string, LocationConfig> *locations = new std::map<std::string, LocationConfig>();

	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_PREFIX);
	}
	// Path for this location
	std::string path = tokens[i + 1];
	i += 1; // Skip "location"

	LocationConfig locationConfig;

	locationConfig._locationName = tokens[i];
	i += 1; // Skip the path put as _locationName

	// Check for opening brace
	if (i >= tokens.size() || tokens[i] != "{") {
		throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_LOCATION_BLOCK);
	}
	i++; // Skip the opening brace

	// Process location block until closing brace
	while (i < tokens.size() && tokens[i] != "}") {
		if (tokens[i] == "index") {
			std::string *indexPtr = locationConfig.getIndex(tokens, i, locationConfig._locationRoot);
			locationConfig._index = *indexPtr;
			delete indexPtr;
			
			i += 2; // Skip "index" and the index value
			// Skip semicolon
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			}
		}
		else if (tokens[i] == "allowed_methods") {
			// Parse allowed methods
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ALLOWED_METHODS);
			}
			i++; // Skip "allowed_methods"
			bool foundMethod = false;
			// Collect method names until semicolon
			while (i < tokens.size() && tokens[i] != ";") {
				// Validate HTTP method
				std::string method = tokens[i];
				if (method != "GET" && method != "POST" && method != "DELETE")
					throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ALLOWED_METHODS);
				
				locationConfig._allowedMethods.push_back(method);
				foundMethod = true;
				i++;
			}
			if (!foundMethod) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ALLOWED_METHODS);
			}
			// Skip semicolon
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ALLOWED_METHODS); // Missing semicolon
			}
		} 
		else if (tokens[i] == "root") {
			std::string *rootPtr = locationConfig.getRoot(tokens, i);
			locationConfig._locationRoot = *rootPtr;
			delete rootPtr;
			
			i += 2; // Skip "root" and the path value
			// Skip semicolon
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			}
		}
		else if (tokens[i] == "autoindex") {
			// Parse autoindex directive
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_AUTOINDEX);
			}
			i++; // Skip "autoindex"
			
			if (tokens[i] == "on") {
				locationConfig._autoindex = true;
			} else if (tokens[i] == "off") {
				locationConfig._autoindex = false;
			} else {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_AUTOINDEX);
			}
			i++; // Move to next token
			
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_AUTOINDEX); // Missing semicolon
			}
		}
		else if (tokens[i] == "return") {
			// Parse redirection
			if (i + 2 >= tokens.size()) {
				throw ConfigException(ERROR_INVALID_REDIRECT);
			}
			i += 3; // Skip "return", code, and URL
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw ConfigException(ERROR_INVALID_REDIRECT); // Missing semicolon
			}
		}
		else if (tokens[i] == "cgi_path") {
			// Parse CGI path
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_CGI_PATH);
			}
			i += 2; // Skip "cgi_path" and the path
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_CGI_PATH); // Missing semicolon
			}
		}
		else if (tokens[i] == "upload_path") {
			// Parse upload path
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_UPLOAD_PATH);
			}
			i += 2; // Skip "upload_path" and the path
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_UPLOAD_PATH); // Missing semicolon
			}
		}
		else {
			// Unknown directive in location block
			throw LocationConfig::ConfigException(LocationConfig::ERROR_UNKNOWN_KEY);
		}
	}
	// Check if we reached the end of tokens without finding closing brace
	if (i >= tokens.size()) {
		throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_LOCATION_BLOCK);
	}
	// Modified section: If location doesn't specify its own root, inherit from server's root
	if (locationConfig._locationRoot.empty()) {
		if (!_root.empty()) {
			locationConfig._locationRoot = _root;
		} else {
			// Only throw an error if both location and server lack a root path
			throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ROOT_PATH);
		}
	}
	// Check for duplicate locations
	if (locations->find(path) != locations->end()) {
		throw LocationConfig::ConfigException(LocationConfig::ERROR_DUPLICATE_LOCATION);
	}
	// Add this location config to map in C++98 style
	locations->insert(std::pair<std::string, LocationConfig>(path, locationConfig));
	// Skip the closing brace
	if (tokens[i] == "}") {
		i++;
	} else {
		throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_LOCATION_BLOCK);
	}
	return locations;
}
