/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:17 by eschmitz          #+#    #+#             */
/*   Updated: 2025/09/19 16:04:39 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "Config.hpp"
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

ServerConfig::ServerConfig() : _root(ConfigConstants::DEFAULT_ROOT), _clientBodyLimit(ConfigConstants::DEFAULT_BODY_LIMIT) {
}

ServerConfig::~ServerConfig() {
}

/**
 * Parses port configuration from listen directive
 * Handles single or multiple port specifications with proper validation
 * Supports both explicit port lists and default port when none specified
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Vector of validated port numbers (1024-65535)
 */
std::vector<int> ServerConfig::getPort(const std::vector<std::string>& tokens, size_t& i) {
    std::vector<int> ports;
    
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_PORT);
    }
    
    i++; // Skip "listen"
    
    size_t startPos = i;
    while (i < tokens.size() && tokens[i] != ";") {
        i++;
    }
    
    if (i >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_PORT);
    }
    
    // If no ports specified between "listen" and ";", use default
    if (i == startPos) {
        ports.push_back(ConfigConstants::DEFAULT_PORT);
    } else {
        // Parse all specified port numbers
        for (size_t pos = startPos; pos < i; pos++) {
            int port = std::atoi(tokens[pos].c_str());
            if (port < ConfigConstants::MIN_PORT || port > ConfigConstants::MAX_PORT) {
                throw ConfigException(ERROR_INVALID_PORT);
            }
            ports.push_back(port);
        }
    }
    
    i++; // Skip semicolon
    return ports;
}

/**
 * Parses host address configuration with default fallback
 * Accepts explicit host specification or uses default localhost
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Host address string (IP address or hostname)
 */
std::string ServerConfig::getHost(const std::vector<std::string>& tokens, size_t& i) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_HOST);
    }
    
    i++; // Skip "host"
    
    std::string host = ConfigConstants::DEFAULT_HOST;
    if (tokens[i] != ";") {
        host = tokens[i];
        if (host.empty()) {
            throw ConfigException(ERROR_INVALID_HOST);
        }
        i++;
    }
    
    TokenHelper::expectSemicolon(tokens, i);
    return host;
}

/**
 * Parses and validates root directory path
 * Ensures directory exists, is accessible, and has proper permissions
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Validated root directory path
 */
std::string ServerConfig::getRoot(const std::vector<std::string>& tokens, size_t& i) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_ROOT_PATH);
    }
    
    i++; // Skip "root"
    
    std::string path = tokens[i];
    if (path.empty()) {
        throw ConfigException(ERROR_INVALID_ROOT_PATH);
    }
    
    PathValidator::validateDirectory(path);
    
    i++;
    TokenHelper::expectSemicolon(tokens, i);
    return path;
}

/**
 * Parses client body size limit with validation
 * Ensures positive numeric value for maximum request body size
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Validated body size limit in bytes
 */
ssize_t ServerConfig::getClientBodyLimit(const std::vector<std::string>& tokens, size_t& i) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_CLIENT_MAX_BODY_SIZE);
    }
    
    i++; // Skip "client_max_body_size"
    
    ssize_t limit = std::atol(tokens[i].c_str());
    if (limit <= 0) {
        throw ConfigException(ERROR_INVALID_CLIENT_MAX_BODY_SIZE);
    }
    
    i++;
    TokenHelper::expectSemicolon(tokens, i);
    return limit;
}

/**
 * Parses server name with basic validation
 * Validates that server name is non-empty
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Validated server name string
 */
std::string ServerConfig::getServerName(const std::vector<std::string>& tokens, size_t& i) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_SERVER_NAME);
    }
    
    i++; // Skip "server_name"
    
    std::string name = tokens[i];
    if (name.empty()) {
        throw ConfigException(ERROR_INVALID_SERVER_NAME);
    }
    
    i++;
    TokenHelper::expectSemicolon(tokens, i);
    return name;
}

/**
 * Parses location configuration block and creates LocationConfig objects
 * Handles location path extraction, block parsing, and inheritance of server settings
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after location block
 * @return Map of location path to LocationConfig object
 */
std::map<std::string, LocationConfig> ServerConfig::getLocationConfig(const std::vector<std::string>& tokens, size_t& i) {
    std::map<std::string, LocationConfig> locations;

    if (i + 1 >= tokens.size()) {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_PREFIX);
    }
    
    std::string path = tokens[i + 1];
    i += 2; // Skip "location" and path
    
    LocationConfig locationConfig(_root);
    locationConfig._locationName = path;

    if (i >= tokens.size() || tokens[i] != "{") {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_LOCATION_BLOCK);
    }
    i++; // Skip opening brace

    // Parse location block directives
    while (i < tokens.size() && tokens[i] != "}") {
        if (tokens[i] == "index") {
            locationConfig._index = locationConfig.getIndex(tokens, i, locationConfig._locationRoot);
            i += 2; // Skip "index" and value
            if (i < tokens.size() && tokens[i] == ";") {
                i++;
            }
        }
        else if (tokens[i] == "allowed_methods") {
            locationConfig._allowedMethods = locationConfig.getAllowedMethods(tokens, i);
        }
        else if (tokens[i] == "root") {
            locationConfig._locationRoot = locationConfig.getRoot(tokens, i);
        }
        else if (tokens[i] == "autoindex") {
            locationConfig._autoindex = locationConfig.getAutoIndex(tokens, i);
        }
        else if (tokens[i] == "cgi_path") {
            locationConfig._cgiPath = locationConfig.getCgiPath(tokens, i);
        }
        else if (tokens[i] == "return") {
            // Skip redirection directive
            if (i + 2 >= tokens.size()) {
                throw ConfigException(ERROR_INVALID_REDIRECT);
            }
            i += 3; // Skip "return", code, and URL
            if (i < tokens.size() && tokens[i] == ";") {
                i++;
            } else {
                throw ConfigException(ERROR_INVALID_REDIRECT);
            }
        }
        else if (tokens[i] == "upload_path") {
            // Skip upload path directive
            if (i + 1 >= tokens.size()) {
                throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_UPLOAD_PATH);
            }
            i += 2; // Skip "upload_path" and path
            if (i < tokens.size() && tokens[i] == ";") {
                i++;
            } else {
                throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_UPLOAD_PATH);
            }
        }
        else {
            throw LocationConfig::ConfigException(LocationConfig::ERROR_UNKNOWN_KEY);
        }
    }

    if (i >= tokens.size()) {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_LOCATION_BLOCK);
    }

    // Inherit server root if location doesn't specify its own
    if (locationConfig._locationRoot.empty()) {
        if (!_root.empty()) {
            locationConfig._locationRoot = _root;
        } else {
            throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ROOT_PATH);
        }
    }

    locations[path] = locationConfig;
    
    if (tokens[i] == "}") {
        i++; // Skip closing brace
    } else {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_LOCATION_BLOCK);
    }

    return locations;
}
