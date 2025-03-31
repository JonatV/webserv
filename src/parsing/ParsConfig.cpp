/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 18:14:47 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/27 16:05:48 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parsing/ParsConfig.hpp"

// Structure to store parsed config
struct LocationConfig {
	std::string prefix;
	std::string root;
	std::vector<std::string> allowed_methods;
	std::string upload_path;
	std::string redirect_to;
};

struct ServerConfig {
	int port = 8080;  // Default port if none is specified
	std::string host = "localhost";  // Default host
	std::string name = "";  // Server name (empty string by default)
	std::string root = "";  // Root directory (empty by default)
	std::vector<std::string> error_pages;  // List of error pages
	std::vector<LocationConfig> locations;  // Locations within the server
	int client_max_body_size = 0;  // Default size (no limit)
	bool autoindex = false;  // Default autoindex setting
};

// Function to get error message as a string
std::string Configuration::getErrorMessage(e_error error) {
	switch (error) {
		case ERROR_NONE: return "No errors detected.";
		
		// File Errors
		case ERROR_FILE_NOT_FOUND: return "Configuration file not found.";
		case ERROR_FILE_PERMISSION_DENIED: return "Permission denied: Cannot read the configuration file.";
		case ERROR_FILE_EMPTY: return "The configuration file is empty.";
		case ERROR_FILE_MALFORMED: return "Malformed configuration file: Check syntax.";
		case ERROR_UNEXPECTED_EOF: return "Unexpected end of file: Possibly unclosed brackets.";

		// Server Errors
		case ERROR_INVALID_SERVER_BLOCK: return "Invalid or missing [server] block in configuration.";
		case ERROR_INVALID_PORT: return "Invalid port: Must be between 0 and 65535.";
		case ERROR_INVALID_HOST: return "Invalid host: Must be a valid IP or domain name.";
		case ERROR_DUPLICATE_SERVER: return "Duplicate server definition: Host and port must be unique.";
		case ERROR_INVALID_SERVER_NAME: return "Invalid server_name: Contains invalid characters.";

		// Location Errors
		case ERROR_INVALID_LOCATION_BLOCK: return "Invalid or missing [location] block.";
		case ERROR_INVALID_PREFIX: return "Invalid location prefix: Must be a valid path.";
		case ERROR_INVALID_ROOT_PATH: return "Invalid root path: Directory does not exist.";
		case ERROR_INVALID_UPLOAD_PATH: return "Invalid upload path: Check directory and permissions.";
		case ERROR_DUPLICATE_LOCATION: return "Duplicate location prefix in configuration.";

		// Routing & Redirection Errors
		case ERROR_INVALID_REDIRECT: return "Invalid redirect format or missing destination URL.";
		case ERROR_LOOPING_REDIRECT: return "Detected infinite redirect loop.";

		// Request Handling Errors
		case ERROR_INVALID_ALLOWED_METHODS: return "Invalid allowed_methods: Contains unknown HTTP methods.";
		case ERROR_INVALID_INDEX_FILES: return "Invalid index files: Must be valid filenames.";
		case ERROR_INVALID_ERROR_PAGE: return "Invalid error page definition: Check format.";
		case ERROR_INVALID_CGI_PATH: return "Invalid CGI path: Must be an executable file.";

		// Security & Limits
		case ERROR_INVALID_CLIENT_MAX_BODY_SIZE: return "Invalid client_max_body_size: Must be a valid size (e.g., 1MB, 10KB).";
		case ERROR_INVALID_AUTOINDEX: return "Invalid autoindex value: Must be 'on' or 'off'.";

		default: return "Unknown error.";
	}
}

// Function to trim whitespace
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Parses a list of strings from `[...]`
std::vector<std::string> parseList(const std::string& line) {
    std::vector<std::string> result;
    std::regex list_regex(R"(\[\s*\"([^\"]+)\"\s*(?:,\s*\"([^\"]+)\"\s*)*\])"); //pas a la norme 98
    std::smatch match;
    if (std::regex_search(line, match, list_regex)) {
        for (size_t i = 1; i < match.size(); i++) {
            if (match[i].matched) result.push_back(match[i].str());
        }
    }
    return result;
}

// Parses a key-value line (e.g., `port = 8080`)
bool parseKeyValue(const std::string& line, std::string& key, std::string& value) {
    size_t equalsPos = line.find('=');
    if (equalsPos == std::string::npos)
		return false;
    key = trim(line.substr(0, equalsPos));
    value = trim(line.substr(equalsPos + 1));
    // Remove surrounding quotes for strings
    if (value.size() > 1 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    return true;
}

// Checks if the server block is valid
Configuration::e_error validateServerBlock(const ServerConfig& server) {
    if (server.port < 1 || server.port > 65535) return Configuration::ERROR_INVALID_PORT;
    if (server.host.empty()) return Configuration::ERROR_INVALID_HOST;
    if (server.name.empty()) return Configuration::ERROR_INVALID_SERVER_NAME;
    return Configuration::ERROR_NONE;
}

// Checks for duplicate server names or blocks
Configuration::e_error checkDuplicateServers(const std::vector<ServerConfig>& servers, const ServerConfig& new_server) {
    for (const auto& server : servers) {
        if (server.name == new_server.name) {
            return Configuration::ERROR_DUPLICATE_SERVER;
        }
    }
    return Configuration::ERROR_NONE;
}

// Function to validate and parse the server block
Configuration::e_error parseServerBlock(std::ifstream& file, ServerConfig& server, std::vector<ServerConfig>& servers) {
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
			continue; // Skip empty lines and comments
        if (line == "}")
			break; // End of server block
        std::string key, value;
        if (!parseKeyValue(line, key, value))
			return Configuration::ERROR_FILE_MALFORMED;
        if (key == "port") {
            server.port = std::stoi(value);
        } else if (key == "host") {
            server.host = value;
        } else if (key == "name") {
            server.name = value;
        } else if (key == "root") {
            server.root = value;
        } else if (key == "error_page") {
            server.error_pages = parseList(value);
        } else if (key == "location") {
            LocationConfig location;
            // Parse location details
            // [Handle location-specific fields like "prefix", "root", etc.]
            server.locations.push_back(location);
        } else {
            return Configuration::ERROR_UNKNOWN_KEY;
        }
    }
    // Check for duplicate servers
    return checkDuplicateServers(servers, server);
}

// Main function to load and validate the configuration
Configuration::e_error loadConfig(const std::string& filename, std::vector<ServerConfig>& servers) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return Configuration::ERROR_FILE_NOT_FOUND;
    }
    std::string line;
    ServerConfig server;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line == "server = {") {
            Configuration::e_error err = parseServerBlock(file, server, servers);
            if (err != Configuration::ERROR_NONE) {
                return err;
            }

            // Validate server block before adding to the list
            err = validateServerBlock(server);
            if (err != Configuration::ERROR_NONE) {
                return err;
            }
            servers.push_back(server);
            server = ServerConfig(); // Reset server for the next block
        } else {
            return Configuration::ERROR_FILE_MALFORMED;
        }
    }

    return Configuration::ERROR_NONE;
}