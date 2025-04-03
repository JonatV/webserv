/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:10 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/03 11:58:16 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"


Config::Config() {
}

Config::~Config() {
}

// Create tokens from the configuration file
std::vector<std::string>	Config::_getTokensFromFile(std::ifstream& file) {
    std::vector<std::string> tokens;
    std::string line, word;
    bool inComment = false;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        std::istringstream iss(line);
        
        // Check for line commented with #
        if (!line.empty() && line[0] == '#') {
            continue;
        }
        
        // Check for multi-line comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Skip empty lines after removing comments
        if (line.find_first_not_of(" \t\n\r") == std::string::npos) {
            continue;
        }
        
        // Process tokens in the line
        while (iss >> word) {
            // Handle braces as separate tokens
            if (word.find('{') != std::string::npos && word != "{") {
                size_t pos = word.find('{');
                if (pos > 0) {
                    tokens.push_back(word.substr(0, pos));
                }
                tokens.push_back("{");
                word = word.substr(pos + 1);
                if (word.empty()) {
                    continue;
                }
            }
            
            if (word.find('}') != std::string::npos && word != "}") {
                size_t pos = word.find('}');
                if (pos > 0) {
                    tokens.push_back(word.substr(0, pos));
                }
                tokens.push_back("}");
                word = word.substr(pos + 1);
                if (word.empty()) {
                    continue;
                }
            }
            
            // Handle semicolons
            if (!word.empty() && word[word.length() - 1] == ';') {
                word.erase(word.length() - 1);
                if (!word.empty()) {
                    tokens.push_back(word);
                }
                tokens.push_back(";");
            } else {
                tokens.push_back(word);
            }
        }
    }
    
    // Check for balanced braces
    int braceCount = 0;
    for (const auto& token : tokens) {
        if (token == "{") {
            braceCount++;
        } else if (token == "}") {
            braceCount--;
        }
        
        if (braceCount < 0) {
            throw ERROR_FILE_MALFORMED; // Unbalanced braces
        }
    }
    
    if (braceCount != 0) {
        throw ERROR_UNEXPECTED_EOF; // Unclosed braces at end of file
    }
    
    return tokens;
}

// Parse server blocks from configuration tokens
std::vector<ServerConfig> Config::_parseServers(std::vector<std::string> tokens) {
    std::vector<ServerConfig> servers;
    size_t i = 0;
    std::set<std::string> serverNames; // To check for duplicate server names
    std::map<std::pair<std::string, int>, bool> serverPorts; // To check for duplicate server:port combinations

    while (i < tokens.size()) {
        if (tokens[i] == "server") {
            // Check for opening brace
            if (i + 1 >= tokens.size() || tokens[i + 1] != "{") {
                throw ServerConfig::ERROR_INVALID_SERVER_BLOCK;
            }
            
            i += 2; // Skip "server" and "{"
            ServerConfig server;
            bool hasPort = false;
            bool hasHost = false;
            
            // Process server block until closing brace
            while (i < tokens.size() && tokens[i] != "}") {
                if (tokens[i] == "listen") {
                    int port = server._getPort(tokens, i);
                    server._port.push_back(port);
                    hasPort = true;
                    i++; // Move past the port token
                    
                    // Skip semicolon if present
                    if (i < tokens.size() && tokens[i] == ";") {
                        i++;
                    }
                } 
                else if (tokens[i] == "host") {
                    server._host = server._getHost(tokens, i);
                    hasHost = true;
                    i += 2; // Skip "host" and the host value
                    
                    // Skip semicolon if present
                    if (i < tokens.size() && tokens[i] == ";") {
                        i++;
                    }
                } 
                else if (tokens[i] == "client_max_body_size") {
                    server._clientBodyLimit = server._getClientBodyLimit(tokens, i);
                    i += 2; // Skip "client_max_body_size" and the size value
                    
                    // Skip semicolon if present
                    if (i < tokens.size() && tokens[i] == ";") {
                        i++;
                    }
                } 
                else if (tokens[i] == "server_name") {
                    server._serverName = server._getServerName(tokens, i);
                    
                    // Check for duplicate server names
                    if (!server._serverName.empty() && serverNames.find(server._serverName) != serverNames.end()) {
                        throw ServerConfig::ERROR_DUPLICATE_SERVER;
                    }
                    serverNames.insert(server._serverName);
                    
                    i += 2; // Skip "server_name" and the name value
                    
                    // Skip semicolon if present
                    if (i < tokens.size() && tokens[i] == ";") {
                        i++;
                    }
                } 
                else if (tokens[i] == "error_page") {
                    // Parse error page directive
                    if (i + 2 >= tokens.size()) {
                        throw ServerConfig::ERROR_INVALID_SERVER_BLOCK;
                    }
                    
                    i++; // Skip "error_page"
                    
                    // Parse error code
                    int errorCode;
                    try {
                        errorCode = std::stoi(tokens[i]);
                        if (errorCode < 100 || errorCode > 599) {
                            throw std::exception(); // Invalid HTTP error code
                        }
                    } catch (std::exception& e) {
                        throw LocationConfig::ERROR_INVALID_ERROR_PAGE;
                    }
                    
                    i++; // Skip error code
                    
                    // Parse error page path
                    std::string errorPage = tokens[i];
                    if (errorPage.empty()) {
                        throw LocationConfig::ERROR_INVALID_ERROR_PAGE;
                    }
                    
                    // Add to error pages map
                    server._errorPages[errorCode] = errorPage;
                    
                    i++; // Skip error page path
                    
                    // Skip semicolon if present
                    if (i < tokens.size() && tokens[i] == ";") {
                        i++;
                    } else {
                        throw LocationConfig::ERROR_INVALID_ERROR_PAGE; // Missing semicolon
                    }
                }
                else if (tokens[i] == "location") {
                    try {
                        // Parse location block and update locations map
                        std::map<std::string, LocationConfig> locs = server._getLocationConfig(tokens, i);
                        
                        // Check for duplicate location paths and merge
                        for (const auto& loc : locs) {
                            if (server._locations.find(loc.first) != server._locations.end()) {
                                throw LocationConfig::ERROR_DUPLICATE_LOCATION;
                            }
                        }
                        
                        server._locations.insert(locs.begin(), locs.end());
                        // Note: i is updated inside _getLocationConfig
                    } catch (int e) {
                        // Re-throw location errors
                        throw e;
                    }
                }
                else {
                    // Unknown directive in server block
                    throw ServerConfig::ERROR_UNKNOWN_KEY;
                }
            }
            
            // Check if we have mandatory configuration
            if (!hasPort) {
                throw ServerConfig::ERROR_INVALID_PORT;
            }
            
            if (!hasHost) {
                throw ServerConfig::ERROR_INVALID_HOST;
            }
            
            // Check for duplicate server:port combinations
            for (const auto& port : server._port) {
                std::pair<std::string, int> serverPort = std::make_pair(server._host, port);
                if (serverPorts.find(serverPort) != serverPorts.end()) {
                    throw ServerConfig::ERROR_DUPLICATE_SERVER;
                }
                serverPorts[serverPort] = true;
            }
            
            // Check if we reached the end of tokens without finding closing brace
            if (i >= tokens.size()) {
                throw ServerConfig::ERROR_INVALID_SERVER_BLOCK;
            }
            
            servers.push_back(server);
            
            // Skip the closing brace of server block
            if (tokens[i] == "}") {
                i++;
            } else {
                throw ServerConfig::ERROR_INVALID_SERVER_BLOCK;
            }
        } else {
            // Unknown top-level directive
            throw Config::ERROR_UNKNOWN_KEY;
        }
    }
    
    // Validate that we have at least one server
    if (servers.empty()) {
        throw ServerConfig::ERROR_INVALID_SERVER_BLOCK;
    }

    return servers;
}

// Main function to parse configuration from a file
bool Config::_parseFile(const std::string& filename) {
	std::ifstream file(filename);
    
    // Check if file exists and is readable
    if (!file.is_open()) {
        throw ERROR_FILE_NOT_FOUND;
    }
    
    // Check if file is empty
    if (file.peek() == std::ifstream::traits_type::eof()) {
        throw ERROR_FILE_EMPTY;
    }
    
    try {
        // Tokenize the file
        _tokens = _getTokensFromFile(file);
        
        // Check if tokens are empty
        if (_tokens.empty()) {
            throw ERROR_FILE_MALFORMED;
        }
        
        // Parse server blocks
        _servers = _parseServers(_tokens);
        
        // Validate configuration (check for redirects loops, etc.)
        // This could be expanded as needed
        
        return true;
    }
    catch (int e) {
        // Re-throw parsing errors
        throw e;
    }
    catch (std::exception& e) {
        // Handle unexpected exceptions
        throw ERROR_FILE_MALFORMED;
    }
}

// Function that displays the informations
void Config::displayConfig() const {
	std::cout << "==== SERVER CONFIGURATION ====" << std::endl;
	std::cout << "Total servers: " << _servers.size() << std::endl;

	for (size_t i = 0; i < _servers.size(); i++) {
		const ServerConfig& server = _servers[i];
		
		std::cout << "\n---- SERVER " << (i + 1) << " ----" << std::endl;
		
		// Display ports
		std::cout << "Ports: ";
		for (size_t j = 0; j < server._port.size(); j++) {
			std::cout << server._port[j];
			if (j < server._port.size() - 1) {
				std::cout << ", ";
			}
		}
		std::cout << std::endl;
		
		// Display host
		std::cout << "Host: " << server._host << std::endl;
		
		// Display server name
		std::cout << "Server Name: " << server._serverName << std::endl;
		
		// Display client body limit
		std::cout << "Client Body Limit: " << server._clientBodyLimit << " bytes" << std::endl;
		
		// Display error pages
		std::cout << "Error Pages: " << std::endl;
		for (const auto& error : server._errorPages) {
			std::cout << "  " << error.first << " -> " << error.second << std::endl;
		}
		
		// Display locations
		std::cout << "Locations: " << std::endl;
		for (const auto& location : server._locations) {
			std::cout << "  Location Path: " << location.first << std::endl;
			const LocationConfig& loc = location.second;
			
			// Display location details
			std::cout << "    Index: " << loc._index << std::endl;
			
			std::cout << "    Allowed Methods: ";
			for (size_t j = 0; j < loc._allowedMethods.size(); j++) {
				std::cout << loc._allowedMethods[j];
				if (j < loc._allowedMethods.size() - 1) {
					std::cout << ", ";
				}
			}
			std::cout << std::endl;
			
			std::cout << "    Root: " << loc._root << std::endl;
			std::cout << "    Autoindex: " << (loc._autoindex ? "On" : "Off") << std::endl;
			std::cout << std::endl;
		}
	}

	std::cout << "==== END OF CONFIGURATION ====" << std::endl;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }
    
    Config config;
    
    try {
        if (config._parseFile(argv[1])) {
            std::cout << "Configuration successfully parsed." << std::endl;
            config.displayConfig();
            return 0;
        }
    } catch (int errorCode) {
        std::string errorMessage;
        
        // Translate error code to meaningful message
        switch (errorCode) {
            // Config errors
            case Config::ERROR_FILE_NOT_FOUND:
                errorMessage = "Configuration file not found or permission denied";
                break;
            case Config::ERROR_FILE_EMPTY:
                errorMessage = "Configuration file is empty";
                break;
            case Config::ERROR_FILE_MALFORMED:
                errorMessage = "Configuration file is malformed";
                break;
            case Config::ERROR_UNEXPECTED_EOF:
                errorMessage = "Unexpected end of file (unclosed block)";
                break;
            case Config::ERROR_UNKNOWN_KEY:
                errorMessage = "Unknown configuration directive";
                break;
                
            // Server errors
            case ServerConfig::ERROR_INVALID_SERVER_BLOCK:
                errorMessage = "Invalid server block configuration";
                break;
            case ServerConfig::ERROR_INVALID_PORT:
                errorMessage = "Invalid port configuration";
                break;
            case ServerConfig::ERROR_INVALID_HOST:
                errorMessage = "Invalid host configuration";
                break;
            case ServerConfig::ERROR_DUPLICATE_SERVER:
                errorMessage = "Duplicate server name or server:port combination";
                break;
            case ServerConfig::ERROR_INVALID_SERVER_NAME:
                errorMessage = "Invalid server name";
                break;
            case ServerConfig::ERROR_INVALID_CLIENT_MAX_BODY_SIZE:
                errorMessage = "Invalid client_max_body_size value";
                break;
                
            // Location errors
            case LocationConfig::ERROR_INVALID_LOCATION_BLOCK:
                errorMessage = "Invalid location block configuration";
                break;
            case LocationConfig::ERROR_INVALID_PREFIX:
                errorMessage = "Invalid location prefix";
                break;
            case LocationConfig::ERROR_INVALID_ROOT_PATH:
                errorMessage = "Invalid root path";
                break;
            case LocationConfig::ERROR_INVALID_UPLOAD_PATH:
                errorMessage = "Invalid upload path";
                break;
            case LocationConfig::ERROR_DUPLICATE_LOCATION:
                errorMessage = "Duplicate location path";
                break;
            case LocationConfig::ERROR_INVALID_ALLOWED_METHODS:
                errorMessage = "Invalid allowed methods";
                break;
            case LocationConfig::ERROR_INVALID_INDEX_FILES:
                errorMessage = "Invalid index files";
                break;
            case LocationConfig::ERROR_INVALID_ERROR_PAGE:
                errorMessage = "Invalid error page";
                break;
            case LocationConfig::ERROR_INVALID_CGI_PATH:
                errorMessage = "Invalid CGI path";
                break;
            case LocationConfig::ERROR_INVALID_AUTOINDEX:
                errorMessage = "Invalid autoindex value (use 'on' or 'off')";
                break;
                
            // Redirection errors
            case LocationConfig::ERROR_INVALID_REDIRECT:
            case ServerConfig::ERROR_INVALID_REDIRECT:
                errorMessage = "Invalid redirection";
                break;
            case LocationConfig::ERROR_LOOPING_REDIRECT:
            case ServerConfig::ERROR_LOOPING_REDIRECT:
                errorMessage = "Redirect loop detected";
                break;
                
            default:
                errorMessage = "Unknown error occurred";
        }
        
        std::cerr << "Error parsing configuration: " << errorMessage << " (code: " << errorCode << ")" << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}