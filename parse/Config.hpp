/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:08 by eschmitz          #+#    #+#             */
/*   Updated: 2025/09/19 16:03:09 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include "ServerConfig.hpp"

// Configuration constants to replace magic numbers
namespace ConfigConstants {
    const int DEFAULT_PORT = 8080;
    const int MIN_PORT = 1024;
    const int MAX_PORT = 65535;
    const size_t DEFAULT_BODY_LIMIT = 100000;
    const std::string DEFAULT_HOST = "127.0.0.1";
    const std::string DEFAULT_SERVER_NAME = "localhost";
    const std::string DEFAULT_ROOT = "./www/";
}

class Config {
private:
    std::vector<ServerConfig> _servers;
    std::vector<std::string> _tokens;
    std::set<std::string> _nonServerSections;

    // Core tokenization functions - separated for clarity and maintainability
    std::vector<std::string> tokenizeBasic(std::ifstream& file);
    std::vector<std::string> processBraces(const std::vector<std::string>& rawTokens);
    void validateBraces(const std::vector<std::string>& tokens);
    
    // Main tokenization coordinator
    std::vector<std::string> getTokensFromFile(std::ifstream& file);
    
    // Server parsing functions - now return by value instead of dynamic allocation
    std::vector<ServerConfig> parseServers(const std::vector<std::string>& tokens);
    ServerConfig parseServerBlock(const std::vector<std::string>& tokens, size_t& i,
                                 std::set<std::string>& serverNames,
                                 std::map<std::pair<std::string, int>, bool>& serverPorts);
    
    // Validation and utility functions
    void validateServerConfig(ServerConfig& server, bool hasPort,
                             std::map<std::pair<std::string, int>, bool>& serverPorts);
    void parseErrorPage(const std::vector<std::string>& tokens, size_t& i,
                       std::map<int, std::string>& errorPages);
    void validateUniqueServerName(const std::string& name, std::set<std::string>& serverNames);
    void mergeLocations(std::map<std::string, LocationConfig>& serverLocations,
                       const std::map<std::string, LocationConfig>& newLocations);
    
    // Helper functions
    size_t skipBlock(const std::vector<std::string>& tokens, size_t startPos);
    bool isNonServerSection(const std::string& token);

public:
    Config();
    ~Config();
    
    /**
     * Parses configuration file and populates server configurations
     * Opens file, tokenizes content, and validates server blocks
     * @param filename Path to configuration file
     * @return Success status (true if parsing succeeded)
     * @throws ConfigException on parsing errors
     */
    bool parseFile(const std::string& filename);
    
    friend class WebServer;

    enum e_error {
        ERROR_NONE = 0,
        ERROR_FILE_NOT_FOUND = 1,
        ERROR_FILE_PERMISSION_DENIED,
        ERROR_FILE_EMPTY,
        ERROR_FILE_MALFORMED,
        ERROR_UNEXPECTED_EOF,
        ERROR_INVALID_REDIRECT = 10,
        ERROR_LOOPING_REDIRECT,
        ERROR_UNKNOWN_KEY = 20
    };

    class ConfigException : public std::exception {
    private:
        e_error _code;
        
    public:
        ConfigException(e_error code) : _code(code) {}
        e_error getCode() const { return _code; }
        
        const char* what() const throw() {
            switch (_code) {
                case ERROR_FILE_NOT_FOUND:
                    return "Configuration file not found";
                case ERROR_FILE_PERMISSION_DENIED:
                    return "Permission denied when accessing configuration file";
                case ERROR_FILE_EMPTY:
                    return "Configuration file is empty";
                case ERROR_FILE_MALFORMED:
                    return "Configuration file is malformed";
                case ERROR_UNEXPECTED_EOF:
                    return "Unexpected end of file (unclosed block)";
                case ERROR_INVALID_REDIRECT:
                    return "Invalid redirection in config";
                case ERROR_LOOPING_REDIRECT:
                    return "Redirect loop detected in config";
                case ERROR_UNKNOWN_KEY:
                    return "Unknown top-level configuration directive";
                default:
                    return "Unknown configuration error";
            }
        }
    };
};

// Utility classes for common validation and parsing tasks
class PathValidator {
public:
    /**
     * Validates file existence, type, and permissions
     * Ensures path points to a readable regular file
     */
    static void validateFile(const std::string& path, bool checkRead = true);
    
    /**
     * Validates directory existence, type, and permissions
     * Ensures path points to a readable directory
     */
    static void validateDirectory(const std::string& path, bool checkRead = true);
    
    /**
     * Validates executable file permissions
     * Ensures file exists and has execute permissions
     */
    static void validateExecutable(const std::string& path);
};

class TokenHelper {
public:
    /**
     * Validates and consumes semicolon token
     * Advances index past semicolon or throws exception
     */
    static void expectSemicolon(const std::vector<std::string>& tokens, size_t& i);
    
    /**
     * Validates HTTP method names
     * Returns true for GET, POST, DELETE
     */
    static bool isValidHttpMethod(const std::string& method);
    
    /**
     * Validates file extensions for index files
     * Returns true for supported web and CGI file extensions
     */
    static bool isValidFileExtension(const std::string& filename);
};

#endif
