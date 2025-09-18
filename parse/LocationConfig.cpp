/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/09/18 13:20:20 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig() : _locationRoot("./www/"), _autoindex(false) {
    // Initialize with default values
}

LocationConfig::LocationConfig(std::string root) : _locationRoot(root), _autoindex(false) {
    // Initialize with root values
}

LocationConfig::~LocationConfig() {
    // Clean up if needed
}

// Parse index file from configuration tokens
// Dans LocationConfig.cpp, remplace la fonction getIndex par :

std::string *LocationConfig::getIndex(std::vector<std::string> tokens, size_t i, const std::string& rootPath) {
	// Check if we have enough tokens
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_INDEX_FILES);
	}
	// Skip the "index" token
	i++;
	std::string indexName = tokens[i];
	// Basic validation
	if (indexName.empty()) {
		throw ConfigException(ERROR_INVALID_INDEX_FILES);
	}
	
	// Check for valid extensions (ajout des extensions CGI)
	bool validExtension = false;
	if (indexName.length() >= 5 && indexName.substr(indexName.length() - 5) == ".html") validExtension = true;
	if (indexName.length() >= 4 && indexName.substr(indexName.length() - 4) == ".css") validExtension = true;
	if (indexName.length() >= 4 && indexName.substr(indexName.length() - 4) == ".ico") validExtension = true;
	if (indexName.length() >= 4 && indexName.substr(indexName.length() - 4) == ".txt") validExtension = true;
	// *** AJOUT DES EXTENSIONS CGI ***
	if (indexName.length() >= 3 && indexName.substr(indexName.length() - 3) == ".py") validExtension = true;
	if (indexName.length() >= 3 && indexName.substr(indexName.length() - 3) == ".sh") validExtension = true;
	if (indexName.length() >= 3 && indexName.substr(indexName.length() - 3) == ".pl") validExtension = true;
	if (indexName.length() >= 4 && indexName.substr(indexName.length() - 4) == ".cgi") validExtension = true;
	
	if (!validExtension) {
		throw ConfigException(ERROR_INVALID_INDEX_FORMAT);
	}
	
	// Check for semicolon
	if (i + 1 >= tokens.size() || tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_INDEX_FILES); // Missing semicolon
	}
	// Construct full path
	std::string fullPath = rootPath;
	// Make sure rootPath ends with a slash
	if (!rootPath.empty() && rootPath[rootPath.length() - 1] != '/') {
		fullPath += "/";
	}
	fullPath += indexName;
	// Check if file exists and is readable
	struct stat file_stat;
	if (stat(fullPath.c_str(), &file_stat) != 0) {
		throw ConfigException(ERROR_INDEX_FILE_NOT_FOUND);
	}
	// Check if it's a regular file
	if (!S_ISREG(file_stat.st_mode)) {
		throw ConfigException(ERROR_INDEX_NOT_A_FILE);
	}
	// Check for read permissions
	if (access(fullPath.c_str(), R_OK) != 0) {
		throw ConfigException(ERROR_INDEX_FILE_NO_ACCESS);
	}
	std::string *result = new std::string(indexName);
	return result;
}

// Parse allowed HTTP methods from configuration tokens
std::string *LocationConfig::getAllowedMethods(std::vector<std::string> tokens) {
	std::string methods;

	// Find allowed_methods directive and collect values
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "allowed_methods") {
			i++; // Move past the "allowed_methods" token
			
			// Collect method names until semicolon
			while (i < tokens.size() && tokens[i] != ";") {
				methods += tokens[i] + " ";
				i++;
			}
			
			break;
		}
	}
	if (methods.empty()) {
		throw ConfigException(ERROR_INVALID_ALLOWED_METHODS);
	}
	// Remove trailing space
	if (!methods.empty()) {
		methods.erase(methods.length() - 1, 1);
	}
	std::string *result = new std::string(methods);
	return result;
}

// Parse root directory from configuration tokens
std::string *LocationConfig::getRoot(std::vector<std::string> tokens, size_t i) {
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

// Parse autoindex flag from configuration tokens
bool *LocationConfig::getAutoIndex(std::vector<std::string> tokens) {
	bool autoindex = false;

	// Find autoindex directive and get value
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "autoindex" && i + 1 < tokens.size()) {
			std::string value = tokens[i + 1];
			
			if (value == "on") {
				autoindex = true;
			} else if (value == "off") {
				autoindex = false;
			} else {
				throw ConfigException(ERROR_INVALID_AUTOINDEX);
			}
			break;
		}
	}
	bool *result = new bool(autoindex);
	return result;
}

// Parse CGI path from configuration tokens
std::string *LocationConfig::getCgiPath(std::vector<std::string> tokens, size_t i) {
	if (i + 1 >= tokens.size()) {
		throw ConfigException(ERROR_INVALID_CGI_PATH);
	}
	i++;
	std::string cgiPath = tokens[i];
	if (cgiPath.empty()) {
		throw ConfigException(ERROR_INVALID_CGI_PATH);
	}
	if (i + 1 >= tokens.size() || tokens[i + 1] != ";") {
		throw ConfigException(ERROR_INVALID_CGI_PATH); // Missing semicolon
	}
	// construct full path
	std::string fullPath = this->getLocationRoot();
	if (fullPath.empty()) {
		throw ConfigException(ERROR_INVALID_CGI_PATH);
	}
	if (fullPath[fullPath.length() - 1] != '/') {
		fullPath += "/";
	}
	fullPath += cgiPath;
	// Check if CGI path exists and is executable
	struct stat file_stat;
	if (stat(fullPath.c_str(), &file_stat) != 0) {
		throw ConfigException(ERROR_INVALID_CGI_PATH);
	}
	if (!S_ISREG(file_stat.st_mode)) {
		throw ConfigException(ERROR_INVALID_CGI_PATH);
	}
	if (access(fullPath.c_str(), X_OK) != 0) {
		throw ConfigException(ERROR_INVALID_CGI_PATH);
	}
	std::string *result = new std::string(cgiPath);
	return result;
}

// getters <3
std::string LocationConfig::getLocationName() const {
	return (_locationName);
}
std::string LocationConfig::getLocationRoot() const {
	return (_locationRoot);
}
std::string LocationConfig::getLocationIndex() const {
	return (_index);
}
std::vector<std::string> LocationConfig::getLocationAllowedMethods() const {
	return (_allowedMethods);
}
bool LocationConfig::getLocationAutoIndex() const {
	return (_autoindex);
}
std::string LocationConfig::getLocationCgiPath() const {
	return (_cgiPath);
}
