/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:10 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/31 19:34:41 by eschmitz         ###   ########.fr       */
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
	std::vector<std::string>	tokens;
	std::string					line, word;

	while (std::getline(file, line)) { // Reads every line of the file
		std::istringstream	iss(line); // Tranforms line in entry flow
		
		if (!line.empty() && line[0] == '#') // Ignores every line beginning with"#"
			continue;
		while (iss >> word) { // Reads word by word
			if (!word.empty() && word[word.length() - 1] == ';') { // Case in which ";" is next to the word
				word.erase(word.length() - 1);
				if (!word.empty())
					tokens.push_back(word);
				tokens.push_back(";");
			}
			else // Classic case (word followed by a space)
				tokens.push_back(word);
		}
	}
	return (tokens);
}

// Parse server blocks from configuration tokens
std::vector<ServerConfig> Config::_parseServers(std::vector<std::string> tokens) {
    std::vector<ServerConfig> servers;
    size_t i = 0;

    while (i < tokens.size()) {
        if (tokens[i] == "server" && i + 1 < tokens.size() && tokens[i + 1] == "{") {
            i += 2; // Skip "server" and "{"
            ServerConfig server;
            
            // Process server block until closing brace
            while (i < tokens.size() && tokens[i] != "}") {
                if (tokens[i] == "listen") {
                    int port = server._getPort(tokens, i);
                    server._port.push_back(port);
                } 
                else if (tokens[i] == "host") {
                    server._host = server._getHost(tokens, i);
                } 
                else if (tokens[i] == "client_max_body_size") {
                    server._clientBodyLimit = server._getClientBodyLimit(tokens, i);
                } 
                else if (tokens[i] == "server_name") {
                    server._serverName = server._getServerName(tokens, i);
                } 
                else if (tokens[i] == "location") {
                    std::map<std::string, LocationConfig> locs = server._getLocationConfig(tokens, i);
                    server._locations.insert(locs.begin(), locs.end());
                    
                    // Skip to the end of the location block
                    int braceCount = 1;
                    while (braceCount > 0 && i < tokens.size()) {
                        i++;
                        if (tokens[i] == "{") braceCount++;
                        if (tokens[i] == "}") braceCount--;
                    }
                }
                
                i++;
            }
            
            servers.push_back(server);
            
        } else {
            i++; // Skip tokens that aren't part of a server block
        }
    }
    
    return servers;
}

// Main function to parse configuration from a file
bool Config::_parseFile(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        // Handle file not found error
        return false;
    }
    
    try {
        // Tokenize the file
        _tokens = _getTokensFromFile(file);
        
        // Parse server blocks
        _servers = _parseServers(_tokens);
        
        return true;
    }
    catch (int e) {
        // Handle parsing errors
        return false;
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