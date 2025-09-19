/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/09/19 17:04:36 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "Config.hpp"

LocationConfig::LocationConfig() : _locationRoot(ConfigConstants::DEFAULT_ROOT), _autoindex(false) {
}

LocationConfig::LocationConfig(const std::string& root) : _locationRoot(root), _autoindex(false) {
}

LocationConfig::~LocationConfig() {
}

/**
 * Parses and validates index file configuration
 * Validates file extension, constructs full path, and checks file accessibility
 * Supports web content files (.html, .css, .txt, .ico) and CGI scripts (.py, .sh, .pl, .cgi)
 * @param tokens Configuration tokens
 * @param i Current position in tokens (not modified - semicolon handled by caller)
 * @param rootPath Root directory for resolving relative index file path
 * @return Index filename (relative to root)
 */
std::string LocationConfig::getIndex(const std::vector<std::string>& tokens, size_t i, const std::string& rootPath) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_INDEX_FILES);
    }
    
    i++;
    std::string indexName = tokens[i];
    
    if (indexName.empty() || !TokenHelper::isValidFileExtension(indexName)) {
        throw ConfigException(ERROR_INVALID_INDEX_FORMAT);
    }
    
    if (i + 1 >= tokens.size() || tokens[i + 1] != ";") {
        throw ConfigException(ERROR_INVALID_INDEX_FILES);
    }
    
    std::string fullPath = rootPath;
    if (!rootPath.empty() && rootPath[rootPath.length() - 1] != '/') {
        fullPath += "/";
    }
    fullPath += indexName;
    
    PathValidator::validateFile(fullPath);
    return indexName;
}

/**
 * Parses allowed HTTP methods for the location
 * Validates method names against supported HTTP methods (GET, POST, DELETE)
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Vector of validated HTTP method names
 */
std::vector<std::string> LocationConfig::getAllowedMethods(const std::vector<std::string>& tokens, size_t& i) {
    std::vector<std::string> methods;
    
    i++;
    while (i < tokens.size() && tokens[i] != ";") {
        if (!TokenHelper::isValidHttpMethod(tokens[i])) {
            throw ConfigException(ERROR_INVALID_ALLOWED_METHODS);
        }
        methods.push_back(tokens[i]);
        i++;
    }
    
    if (methods.empty()) {
        throw ConfigException(ERROR_INVALID_ALLOWED_METHODS);
    }
    
    TokenHelper::expectSemicolon(tokens, i);
    return methods;
}

/**
 * Parses and validates root directory path for location
 * Ensures directory exists, is accessible, and has proper permissions
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Validated root directory path
 */
std::string LocationConfig::getRoot(const std::vector<std::string>& tokens, size_t& i) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_ROOT_PATH);
    }
    i++;
    
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
 * Parses autoindex directive (directory listing configuration)
 * Accepts "on" or "off" values to enable/disable automatic directory indexing
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return Autoindex setting (true for "on", false for "off")
 */
bool LocationConfig::getAutoIndex(const std::vector<std::string>& tokens, size_t& i) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_AUTOINDEX);
    }
    i++;
    
    bool autoindex;
    if (tokens[i] == "on") {
        autoindex = true;
    } else if (tokens[i] == "off") {
        autoindex = false;
    } else {
        throw ConfigException(ERROR_INVALID_AUTOINDEX);
    }
    
    i++;
    TokenHelper::expectSemicolon(tokens, i);
    return autoindex;
}

/**
 * Parses and validates CGI script path configuration
 * Constructs full path, validates file existence and execute permissions
 * @param tokens Configuration tokens
 * @param i Current position in tokens, updated to position after semicolon
 * @return CGI script path (relative to location root)
 */
std::string LocationConfig::getCgiPath(const std::vector<std::string>& tokens, size_t& i) {
    if (i + 1 >= tokens.size()) {
        throw ConfigException(ERROR_INVALID_CGI_PATH);
    }
    i++;
    
    std::string cgiPath = tokens[i];
    if (cgiPath.empty()) {
        throw ConfigException(ERROR_INVALID_CGI_PATH);
    }
    
    std::string fullPath = _locationRoot;
    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/') {
        fullPath += "/";
    }
    fullPath += cgiPath;
    
    PathValidator::validateExecutable(fullPath);
    
    i++;
    TokenHelper::expectSemicolon(tokens, i);
    return cgiPath;
}
