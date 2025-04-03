/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:17 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/03 17:26:49 by eschmitz         ###   ########.fr       */
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
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ERROR_INVALID_PORT;
	}
	int port = -1;
	// Skip the "listen" token
	i++;
	// Check if next token is a valid port number
	try {
		port = std::stoi(tokens[i]);
		// Basic port validation
		if (port < 0 || port > 65535) {
			throw ERROR_INVALID_PORT;
		}
	} catch (std::exception& e) {
		throw ERROR_INVALID_PORT;
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ERROR_INVALID_PORT; // Missing semicolon
	}
	return port;
}

// Parse host address from configuration tokens
std::string	ServerConfig::_getHost(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ERROR_INVALID_HOST;
	}
	// Skip the "host" token
	i++;
	std::string host = tokens[i];
	// Basic host validation (could be expanded)
	if (host.empty()) {
		throw ERROR_INVALID_HOST;
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ERROR_INVALID_HOST; // Missing semicolon
	}
	return host;
}

// Parse client body size limit from configuration tokens
size_t	ServerConfig::_getClientBodyLimit(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ERROR_INVALID_CLIENT_MAX_BODY_SIZE;
	}
	// Skip the "client_max_body_size" token
	i++;
	size_t limit = 0;
	// Check if next token is a valid size
	try {
		limit = std::stoull(tokens[i]);
		// Basic validation
		if (limit == 0) {
			throw ERROR_INVALID_CLIENT_MAX_BODY_SIZE;
		}
	} catch (std::exception& e) {
		throw ERROR_INVALID_CLIENT_MAX_BODY_SIZE;
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ERROR_INVALID_CLIENT_MAX_BODY_SIZE; // Missing semicolon
	}
	return limit;
}

// Parse server name from configuration tokens
std::string	ServerConfig::_getServerName(std::vector<std::string> tokens, size_t i) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ERROR_INVALID_SERVER_NAME;
	}
	// Skip the "server_name" token
	i++;
	std::string name = tokens[i];
	// Basic validation
	if (name.empty()) {
		throw ERROR_INVALID_SERVER_NAME;
	}
	// Check for semicolon
	if (i + 1 < tokens.size() && tokens[i + 1] != ";") {
		throw ERROR_INVALID_SERVER_NAME; // Missing semicolon
	}
	return name;
}

// Parse location configuration from tokens
std::map<std::string, LocationConfig>	ServerConfig::_getLocationConfig(std::vector<std::string> tokens, size_t& i) {
	std::map<std::string, LocationConfig> locations;
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw LocationConfig::ERROR_INVALID_PREFIX;
	}
	// Path for this location
	std::string path = tokens[i + 1];
	i += 2; // Skip "location" and the path
	// Check for opening brace
	if (i >= tokens.size() || tokens[i] != "{") {
		throw LocationConfig::ERROR_INVALID_LOCATION_BLOCK;
	}
	i++; // Skip the opening brace
	LocationConfig locationConfig;
	// Process location block until closing brace
	while (i < tokens.size() && tokens[i] != "}") {
		if (tokens[i] == "index") {
			// Parse index directive
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ERROR_INVALID_INDEX_FILES;
			}
			i++; // Skip "index"
			std::string index = tokens[i];
			if (index.empty()) {
				throw LocationConfig::ERROR_INVALID_INDEX_FILES;
			}
			locationConfig._index = index;
			i++; // Move to next token
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ERROR_INVALID_INDEX_FILES; // Missing semicolon
			}
		} 
		else if (tokens[i] == "allowed_methods") {
			// Parse allowed methods
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ERROR_INVALID_ALLOWED_METHODS;
			}
			i++; // Skip "allowed_methods"
			bool foundMethod = false;
			// Collect method names until semicolon
			while (i < tokens.size() && tokens[i] != ";") {
				// Validate HTTP method
				std::string method = tokens[i];
				if (method != "GET" && method != "POST" && method != "PUT" && 
					method != "DELETE" && method != "HEAD") {
					throw LocationConfig::ERROR_INVALID_ALLOWED_METHODS;
				}
				
				locationConfig._allowedMethods.push_back(method);
				foundMethod = true;
				i++;
			}
			if (!foundMethod) {
				throw LocationConfig::ERROR_INVALID_ALLOWED_METHODS;
			}
			// Skip semicolon
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ERROR_INVALID_ALLOWED_METHODS; // Missing semicolon
			}
		} 
		else if (tokens[i] == "root") {
			// Parse root directive
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ERROR_INVALID_ROOT_PATH;
			}
			i++; // Skip "root"
			std::string root = tokens[i];
			if (root.empty()) {
				throw LocationConfig::ERROR_INVALID_ROOT_PATH;
			}
			locationConfig._root = root;
			i++; // Move to next token
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ERROR_INVALID_ROOT_PATH; // Missing semicolon
			}
		} 
		else if (tokens[i] == "autoindex") {
			// Parse autoindex directive
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ERROR_INVALID_AUTOINDEX;
			}
			i++; // Skip "autoindex"
			
			if (tokens[i] == "on") {
				locationConfig._autoindex = true;
			} else if (tokens[i] == "off") {
				locationConfig._autoindex = false;
			} else {
				throw LocationConfig::ERROR_INVALID_AUTOINDEX;
			}
			i++; // Move to next token
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ERROR_INVALID_AUTOINDEX; // Missing semicolon
			}
		}
		else if (tokens[i] == "return") {
			// Parse redirection
			if (i + 2 >= tokens.size()) {
				throw LocationConfig::ERROR_INVALID_REDIRECT;
			}
			i += 3; // Skip "return", code, and URL
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ERROR_INVALID_REDIRECT; // Missing semicolon
			}
		}
		else if (tokens[i] == "cgi_path") {
			// Parse CGI path
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ERROR_INVALID_CGI_PATH;
			}
			i += 2; // Skip "cgi_path" and the path
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ERROR_INVALID_CGI_PATH; // Missing semicolon
			}
		}
		else if (tokens[i] == "upload_path") {
			// Parse upload path
			if (i + 1 >= tokens.size()) {
				throw LocationConfig::ERROR_INVALID_UPLOAD_PATH;
			}
			i += 2; // Skip "upload_path" and the path
			// Skip semicolon if present
			if (i < tokens.size() && tokens[i] == ";") {
				i++;
			} else {
				throw LocationConfig::ERROR_INVALID_UPLOAD_PATH; // Missing semicolon
			}
		}
		else {
			// Unknown directive in location block
			throw LocationConfig::ERROR_UNKNOWN_KEY;
		}
	}
	// Check if we reached the end of tokens without finding closing brace
	if (i >= tokens.size()) {
		throw LocationConfig::ERROR_INVALID_LOCATION_BLOCK;
	}
	// Check for empty or incomplete configuration
	if (locationConfig._root.empty()) {
		throw LocationConfig::ERROR_INVALID_ROOT_PATH;
	}
	// Check for duplicate locations
	if (locations.find(path) != locations.end()) {
		throw LocationConfig::ERROR_DUPLICATE_LOCATION;
	}
	// Add this location config to map
	locations[path] = locationConfig;
	// Skip the closing brace
	if (tokens[i] == "}") {
		i++;
	} else {
		throw LocationConfig::ERROR_INVALID_LOCATION_BLOCK;
	}
	return locations;
}
