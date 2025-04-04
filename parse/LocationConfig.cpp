/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 14:03:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/04 12:32:32 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig() : _autoindex(false) {
}

LocationConfig::~LocationConfig() {
}

// Parse index file from configuration tokens
std::string *LocationConfig::getIndex(std::vector<std::string> tokens) {
	std::string index;

	// Find index directive and get value
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "index" && i + 1 < tokens.size()) {
			index = tokens[i + 1];
			
			// Skip the semicolon if it exists
			if (i + 2 < tokens.size() && tokens[i + 2] == ";") {
				break;
			}
		}
	}

	if (index.empty()) {
		throw ConfigException(ERROR_INVALID_INDEX_FILES);
	}

	std::string *result = new std::string(index);
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
		methods.pop_back();
	}

	std::string *result = new std::string(methods);
	return result;
}

// Parse root directory from configuration tokens
std::string *LocationConfig::getRoot(std::vector<std::string> tokens) {
	std::string root;

	// Find root directive and get value
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "root" && i + 1 < tokens.size()) {
			root = tokens[i + 1];
			
			// Skip the semicolon if it exists
			if (i + 2 < tokens.size() && tokens[i + 2] == ";") {
				break;
			}
		}
	}

	if (root.empty()) {
		throw ConfigException(ERROR_INVALID_ROOT_PATH);
	}

	std::string *result = new std::string(root);
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