/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:10 by eschmitz          #+#    #+#             */
/*   Updated: 2025/09/19 17:02:12 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>

Config::Config() {
}

Config::~Config() {
}

/**
 * Validates that a file exists, is readable, and is a regular file
 * Used for index files, error pages, and CGI scripts
 * @param path File path to validate
 * @param checkRead Whether to verify read permissions
 * @throws ConfigException if validation fails
 */
void PathValidator::validateFile(const std::string& path, bool checkRead) {
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != 0) {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INDEX_FILE_NOT_FOUND);
    }
    if (!S_ISREG(file_stat.st_mode)) {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INDEX_NOT_A_FILE);
    }
    if (checkRead && access(path.c_str(), R_OK) != 0) {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INDEX_FILE_NO_ACCESS);
    }
}

/**
 * Validates that a directory exists, is accessible, and is a directory
 * Used for root paths and upload directories
 * @param path Directory path to validate
 * @param checkRead Whether to verify read permissions
 * @throws ConfigException if validation fails
 */
void PathValidator::validateDirectory(const std::string& path, bool checkRead) {
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_ROOT_PATH_NOT_FOUND);
    }
    if (!S_ISDIR(path_stat.st_mode)) {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_ROOT_PATH_NOT_DIRECTORY);
    }
    if (checkRead && access(path.c_str(), R_OK) != 0) {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_ROOT_PATH_NO_ACCESS);
    }
}

/**
 * Validates that a file is executable
 * Used specifically for CGI script validation
 * @param path Executable path to validate
 * @throws ConfigException if validation fails
 */
void PathValidator::validateExecutable(const std::string& path) {
    validateFile(path, false);
    if (access(path.c_str(), X_OK) != 0) {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_CGI_PATH);
    }
}

/**
 * Checks for semicolon token and advances past it
 * Centralizes semicolon validation across all parsing functions
 * @param tokens Vector of configuration tokens
 * @param i Current position, advanced past semicolon on success
 * @throws ConfigException if semicolon is missing
 */
void TokenHelper::expectSemicolon(const std::vector<std::string>& tokens, size_t& i) {
    if (i >= tokens.size() || tokens[i] != ";") {
        throw Config::ConfigException(Config::ERROR_FILE_MALFORMED);
    }
    i++;
}

/**
 * Validates HTTP method names against supported methods
 * @param method Method name to validate
 * @return true if method is GET, POST, or DELETE
 */
bool TokenHelper::isValidHttpMethod(const std::string& method) {
    return (method == "GET" || method == "POST" || method == "DELETE");
}

/**
 * Validates file extensions for index files
 * Checks against supported web content and CGI script extensions
 * @param filename Filename to check
 * @return true if extension is supported
 */
bool TokenHelper::isValidFileExtension(const std::string& filename) {
    static std::set<std::string> validExtensions;
    
    if (validExtensions.empty()) {
        validExtensions.insert(".html");
        validExtensions.insert(".css");
        validExtensions.insert(".ico");
        validExtensions.insert(".txt");
        validExtensions.insert(".py");
        validExtensions.insert(".sh");
        validExtensions.insert(".pl");
        validExtensions.insert(".cgi");
    }
    
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string extension = filename.substr(dotPos);
    return validExtensions.find(extension) != validExtensions.end();
}

/**
 * Extracts basic tokens from configuration file
 * Handles comment removal and basic word separation
 * @param file Input file stream to tokenize
 * @return Vector of raw tokens without special character processing
 */
std::vector<std::string> Config::tokenizeBasic(std::ifstream& file) {
    std::vector<std::string> tokens;
    std::string line, word;

    while (std::getline(file, line)) {
        if (!line.empty() && line[0] == '#') {
            continue;
        }
        
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        if (line.find_first_not_of(" \t\n\r") == std::string::npos) {
            continue;
        }
        
        std::istringstream iss(line);
        while (iss >> word) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

/**
 * Processes raw tokens to separate braces and semicolons
 * Ensures configuration block delimiters are individual tokens
 * @param rawTokens Vector of basic tokens from file
 * @return Vector of processed tokens with separated special characters
 */
std::vector<std::string> Config::processBraces(const std::vector<std::string>& rawTokens) {
    std::vector<std::string> tokens;
    
    for (size_t i = 0; i < rawTokens.size(); i++) {
        std::string word = rawTokens[i];
        
        size_t bracePos = word.find('{');
        while (bracePos != std::string::npos) {
            if (bracePos > 0) {
                tokens.push_back(word.substr(0, bracePos));
            }
            tokens.push_back("{");
            word = word.substr(bracePos + 1);
            bracePos = word.find('{');
        }
        
        bracePos = word.find('}');
        while (bracePos != std::string::npos) {
            if (bracePos > 0) {
                tokens.push_back(word.substr(0, bracePos));
            }
            tokens.push_back("}");
            word = word.substr(bracePos + 1);
            bracePos = word.find('}');
        }
        
        if (!word.empty() && word[word.length() - 1] == ';') {
            word.erase(word.length() - 1);
            if (!word.empty()) {
                tokens.push_back(word);
            }
            tokens.push_back(";");
        } else if (!word.empty()) {
            tokens.push_back(word);
        }
    }
    
    return tokens;
}

/**
 * Validates proper brace balancing in token stream
 * Ensures all configuration blocks have matching opening/closing braces
 * @param tokens Vector of processed tokens to validate
 * @throws ConfigException if braces are unbalanced
 */
void Config::validateBraces(const std::vector<std::string>& tokens) {
    int braceCount = 0;
    
    for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
        if (*it == "{") {
            braceCount++;
        } else if (*it == "}") {
            braceCount--;
            if (braceCount < 0) {
                throw ConfigException(ERROR_FILE_MALFORMED);
            }
        }
    }
    
    if (braceCount != 0) {
        throw ConfigException(ERROR_UNEXPECTED_EOF);
    }
}

/**
 * Main tokenization coordinator function
 * Orchestrates the complete tokenization process from file to validated tokens
 * @param file Input file stream to process
 * @return Complete vector of validated, processed tokens
 */
std::vector<std::string> Config::getTokensFromFile(std::ifstream& file) {
    std::vector<std::string> rawTokens = tokenizeBasic(file);
    std::vector<std::string> tokens = processBraces(rawTokens);
    validateBraces(tokens);
    return tokens;
}

/**
 * Skips over complete configuration blocks
 * Used to ignore non-server sections like 'http', 'events', etc.
 * @param tokens Vector of configuration tokens
 * @param startPos Position after the block identifier token
 * @return Position immediately after the closing brace
 */
size_t Config::skipBlock(const std::vector<std::string>& tokens, size_t startPos) {
    size_t i = startPos;
    int braceCount = 0;
    
    while (i < tokens.size() && tokens[i] != "{") {
        i++;
    }
    
    if (i >= tokens.size()) {
        throw ConfigException(ERROR_FILE_MALFORMED);
    }
    
    braceCount = 1;
    i++;
    
    while (i < tokens.size() && braceCount > 0) {
        if (tokens[i] == "{") {
            braceCount++;
        } else if (tokens[i] == "}") {
            braceCount--;
        }
        i++;
    }
    
    if (braceCount != 0) {
        throw ConfigException(ERROR_UNEXPECTED_EOF);
    }
    
    return i;
}

/**
 * Identifies known non-server configuration sections
 * These are valid nginx-style config blocks that should be ignored
 * @param token Token to check
 * @return true if token represents a known non-server section
 */
bool Config::isNonServerSection(const std::string& token) {
    static std::set<std::string> knownSections;
    
    if (knownSections.empty()) {
        knownSections.insert("http");
        knownSections.insert("stream");
        knownSections.insert("events");
        knownSections.insert("mail");
        knownSections.insert("types");
        knownSections.insert("upstream");
    }
    
    return knownSections.find(token) != knownSections.end();
}

/**
 * Parses all server configuration blocks from tokenized input
 * Coordinates parsing of individual server blocks and validates global uniqueness
 * @param tokens Complete vector of configuration tokens
 * @return Vector of validated server configurations
 */
std::vector<ServerConfig> Config::parseServers(const std::vector<std::string>& tokens) {
    std::vector<ServerConfig> servers;
    size_t i = 0;
    std::set<std::string> serverNames;
    std::map<std::pair<std::string, int>, bool> serverPorts;

    while (i < tokens.size()) {
        if (tokens[i] == "server") {
            servers.push_back(parseServerBlock(tokens, i, serverNames, serverPorts));
        } 
        else if (isNonServerSection(tokens[i])) {
            _nonServerSections.insert(tokens[i]);
            i = skipBlock(tokens, i + 1);
        }
        else if (tokens[i] == ";" || tokens[i].find(';') != std::string::npos) {
            i++;
        }
        else {
            i++;
        }
    }
    
    return servers;
}

/**
 * Parses individual server configuration block
 * Handles all server-level directives and validates required configuration
 * @param tokens Configuration tokens
 * @param i Current position, updated to position after server block
 * @param serverNames Set for tracking duplicate server names
 * @param serverPorts Map for tracking duplicate server:port combinations
 * @return Fully configured ServerConfig object
 */
ServerConfig Config::parseServerBlock(const std::vector<std::string>& tokens, size_t& i,
                                     std::set<std::string>& serverNames,
                                     std::map<std::pair<std::string, int>, bool>& serverPorts) {
    
    if (i + 1 >= tokens.size() || tokens[i + 1] != "{") {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_INVALID_SERVER_BLOCK);
    }
    
    i += 2;
    ServerConfig server;
    bool hasPort = false;

    while (i < tokens.size() && tokens[i] != "}") {
        if (tokens[i] == "listen") {
            std::vector<int> ports = server.getPort(tokens, i);
            server._port.insert(server._port.end(), ports.begin(), ports.end());
            hasPort = true;
        }
        else if (tokens[i] == "host") {
            server._host = server.getHost(tokens, i);
        }
        else if (tokens[i] == "server_name") {
            std::string name = server.getServerName(tokens, i);
            validateUniqueServerName(name, serverNames);
            server._serverName.push_back(name);
        }
        else if (tokens[i] == "client_max_body_size") {
            server._clientBodyLimit = server.getClientBodyLimit(tokens, i);
        }
        else if (tokens[i] == "root") {
            server._root = server.getRoot(tokens, i);
        }
        else if (tokens[i] == "error_page") {
            parseErrorPage(tokens, i, server._errorPages);
        }
        else if (tokens[i] == "location") {
            std::map<std::string, LocationConfig> locations = server.getLocationConfig(tokens, i);
            mergeLocations(server._locations, locations);
        }
        else {
            throw ServerConfig::ConfigException(ServerConfig::ERROR_UNKNOWN_KEY);
        }
    }

    validateServerConfig(server, hasPort, serverPorts);
    
    if (i >= tokens.size() || tokens[i] != "}") {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_INVALID_SERVER_BLOCK);
    }
    i++;

    return server;
}

/**
 * Validates server configuration and applies default values
 * Ensures required configuration is present and checks for port conflicts
 * @param server Server configuration to validate and modify
 * @param hasPort Whether port was explicitly configured
 * @param serverPorts Map for checking duplicate server:port combinations
 */
void Config::validateServerConfig(ServerConfig& server, bool hasPort,
                                 std::map<std::pair<std::string, int>, bool>& serverPorts) {
    
    if (!hasPort) {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_INVALID_PORT);
    }
    
    if (server._host.empty()) {
        server._host = ConfigConstants::DEFAULT_HOST;
    }
    
    if (server._serverName.empty()) {
        server._serverName.push_back(ConfigConstants::DEFAULT_SERVER_NAME);
    }
    
    for (std::vector<int>::const_iterator portIt = server._port.begin(); 
         portIt != server._port.end(); ++portIt) {
        
        std::pair<std::string, int> serverPort = std::make_pair(server._host, *portIt);
        if (serverPorts.find(serverPort) != serverPorts.end()) {
            throw ServerConfig::ConfigException(ServerConfig::ERROR_DUPLICATE_SERVER);
        }
        serverPorts[serverPort] = true;
    }
}

/**
 * Parses error page directive and validates configuration
 * Handles error code validation, path construction, and file validation
 * @param tokens Configuration tokens
 * @param i Current position, updated after parsing
 * @param errorPages Map to store error page configurations
 */
void Config::parseErrorPage(const std::vector<std::string>& tokens, size_t& i,
                           std::map<int, std::string>& errorPages) {
    
    if (i + 2 >= tokens.size()) {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_INVALID_SERVER_BLOCK);
    }
    i++;
    
    int errorCode = std::atoi(tokens[i].c_str());
    if (errorCode < 100 || errorCode > 599) {
        throw LocationConfig::ConfigException(LocationConfig::ERROR_INVALID_ERROR_PAGE);
    }
    i++;
    
    std::string errorPage = tokens[i];
    if (errorPage[0] != '/' && errorPage[0] != '.' && errorPage[0] != '~') {
        errorPage = "./www/error_pages/" + errorPage;
    }
    
    PathValidator::validateFile(errorPage);
    
    errorPages[errorCode] = errorPage;
    i++;
    
    TokenHelper::expectSemicolon(tokens, i);
}

/**
 * Validates server name uniqueness across all servers
 * @param name Server name to validate
 * @param serverNames Set of existing server names
 */
void Config::validateUniqueServerName(const std::string& name, std::set<std::string>& serverNames) {
    if (serverNames.find(name) != serverNames.end()) {
        throw ServerConfig::ConfigException(ServerConfig::ERROR_DUPLICATE_SERVER);
    }
    serverNames.insert(name);
}

/**
 * Merges new location configurations into existing server locations
 * Validates that location paths are unique within the server
 * @param serverLocations Existing server location configurations
 * @param newLocations New location configurations to merge
 */
void Config::mergeLocations(std::map<std::string, LocationConfig>& serverLocations,
                           const std::map<std::string, LocationConfig>& newLocations) {
    
    for (std::map<std::string, LocationConfig>::const_iterator it = newLocations.begin(); 
         it != newLocations.end(); ++it) {
        
        if (serverLocations.find(it->first) != serverLocations.end()) {
            throw LocationConfig::ConfigException(LocationConfig::ERROR_DUPLICATE_LOCATION);
        }
        serverLocations[it->first] = it->second;
    }
}

/**
 * Main configuration file parsing entry point
 * Opens file, validates accessibility, tokenizes content, and parses server blocks
 * @param filename Path to configuration file
 * @return Success status (true if parsing succeeded)
 */
bool Config::parseFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    
    if (!file.is_open()) {
        throw ConfigException(ERROR_FILE_NOT_FOUND);
    }
    
    if (file.peek() == std::ifstream::traits_type::eof()) {
        throw ConfigException(ERROR_FILE_EMPTY);
    }
    
    try {
        _tokens = getTokensFromFile(file);
        
        if (_tokens.empty()) {
            throw ConfigException(ERROR_FILE_MALFORMED);
        }
        
        _servers = parseServers(_tokens);
        return true;
        
    } catch (const std::exception& e) {
        throw;
    }
}
