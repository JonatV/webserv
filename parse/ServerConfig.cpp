/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:17 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/09 18:03:18 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include <sstream>
#include <cstdlib> // for atoi and strtoul
#include <sys/stat.h>
#include <unistd.h>

// ServerConfig Constructor & Destructor
ServerConfig::ServerConfig() : _clientBodyLimit(0) {
    // Initialize with default values
}

ServerConfig::~ServerConfig() {
    // Clean up if needed
}

// Parse port number(s) from configuration tokens
int *ServerConfig::getPort(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_PORT);
	}

	int port = -1;

	// Skip the "listen" token
	i++;
	// Check if next token is a valid port number
	try {
		// C++98 compatible conversion from string to int
		std::istringstream iss(tokens[i]);
		iss >> port;
		if (iss.fail()) {
			throw ConfigException(ERROR_INVALID_PORT);
		}
		// Basic port validation
		if (port < 1024 || port > 65535) {
			throw ConfigException(ERROR_INVALID_PORT);
		}
	} catch (std::exception& e) {
		throw ConfigException(ERROR_INVALID_PORT);
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_PORT); // Missing semicolon
	}
	int *result = new int(port);
	return result;
}

// Parse host address from configuration tokens
std::string *ServerConfig::getHost(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_HOST);
	}
	// Skip the "host" token
	i++;

	std::string host = tokens[i];

	// Basic host validation (could be expanded)
	if (host.empty()) {
		throw ConfigException(ERROR_INVALID_HOST);
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_HOST); // Missing semicolon
	}
	std::string *result = new std::string(host);
	return result;
}

// Parse root directory from configuration tokens
std::string *ServerConfig::getRoot(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}
	// Skip the "root" token
	i++;

	std::string root = tokens[i];

	// Basic validation
	if (root.empty()) {
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}
	// Normalize path - remove trailing slashes if any
	while (root.length() > 1 && root[root.length() - 1] == '/') {
		root.erase(root.length() - 1, 1);
	}
	// Add leading ./ if path doesn't have absolute or relative path indicators
	if (root[0] != '/' && root[0] != '.' && root[0] != '~') {
		root = "./../www/" + root;
	}
	// Path validation
	struct stat path_stat;
	if (stat(root.c_str(), &path_stat) != 0) {
		// Path doesn't exist
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}
	if (!S_ISDIR(path_stat.st_mode)) {
		// Path exists but is not a directory
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}
	// Check access permissions
	if (access(root.c_str(), R_OK) != 0) {
		// No read permission
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_ROOT_PATH); // Missing semicolon
	}
	std::string *result = new std::string(root);
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
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
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
	i += 2; // Skip "location" and the path
	// Check for opening brace
	if (i >= tokens.size() || tokens[i] != "{") {
		throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_LOCATION_BLOCK);
	}
	i++; // Skip the opening brace

	LocationConfig locationConfig;

	// Process location block until closing brace
	while (i < tokens.size() && tokens[i] != "}") {
		if (tokens[i] == "index") {
			// Parse index directive
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_INDEX_FILES);
			}
			i++; // Skip "index"
			std::string index = tokens[i];
			
			if (index.empty()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_INDEX_FILES);
			}
			locationConfig._index = index;
			i++; // Move to next token
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_INDEX_FILES); // Missing semicolon
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
				if (method != "GET" && method != "POST" && method != "PUT" && 
					method != "DELETE" && method != "HEAD") {
					throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ALLOWED_METHODS);
				}
				
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
			// Parse root directive
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ROOT_PATH);
			}
			i++; // Skip "root"
			std::string root = tokens[i];
			
			if (root.empty()) {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ROOT_PATH);
			}
			locationConfig._locationRoot = root;
			i++; // Move to next token
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ROOT_PATH); // Missing semicolon
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
