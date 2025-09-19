/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:12 by eschmitz          #+#    #+#             */
/*   Updated: 2025/09/19 16:09:52 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <sys/stat.h>
#include <unistd.h>

class LocationConfig {
private:
    std::string _locationName;
    std::string _index;
    std::vector<std::string> _allowedMethods;
    std::string _locationRoot;
    bool _autoindex;
    std::string _cgiPath;

    // Parsing functions - return by value, take const references
    std::string getIndex(const std::vector<std::string>& tokens, size_t i, const std::string& rootPath);
    std::vector<std::string> getAllowedMethods(const std::vector<std::string>& tokens, size_t& i);
    std::string getRoot(const std::vector<std::string>& tokens, size_t& i);
    bool getAutoIndex(const std::vector<std::string>& tokens, size_t& i);
    std::string getCgiPath(const std::vector<std::string>& tokens, size_t& i);

public:
    LocationConfig();
    LocationConfig(const std::string& root);
    ~LocationConfig();

    // Getters - keeping original verbose names for compatibility
    const std::string& getLocationName() const { return _locationName; }
    const std::string& getLocationRoot() const { return _locationRoot; }
    const std::string& getLocationIndex() const { return _index; }
    const std::vector<std::string>& getLocationAllowedMethods() const { return _allowedMethods; }
    bool getLocationAutoIndex() const { return _autoindex; }
    const std::string& getLocationCgiPath() const { return _cgiPath; }

    friend class ServerConfig;
    friend class Config;
    friend class WebServer;

    enum e_error {
        ERROR_NONE = 0,
        ERROR_INVALID_LOCATION_BLOCK = 200,
        ERROR_INVALID_PREFIX,
        ERROR_INVALID_ROOT_PATH,
        ERROR_INVALID_UPLOAD_PATH,
        ERROR_DUPLICATE_LOCATION,
        ERROR_ROOT_PATH_NO_ACCESS,
        ERROR_ROOT_PATH_NOT_DIRECTORY,
        ERROR_ROOT_PATH_NOT_FOUND,
        ERROR_INVALID_INDEX_FORMAT,
        ERROR_INDEX_FILE_NOT_FOUND,
        ERROR_INDEX_NOT_A_FILE,
        ERROR_INDEX_FILE_NO_ACCESS,
        ERROR_INVALID_REDIRECT = 220,
        ERROR_LOOPING_REDIRECT,
        ERROR_INVALID_ALLOWED_METHODS = 230,
        ERROR_INVALID_INDEX_FILES,
        ERROR_INVALID_ERROR_PAGE,
        ERROR_INVALID_CGI_PATH,
        ERROR_INVALID_AUTOINDEX = 240,
        ERROR_UNKNOWN_KEY = 250
    };

    class ConfigException : public std::exception {
    private:
        e_error _code;
        
    public:
        ConfigException(e_error code) : _code(code) {}
        e_error getCode() const { return _code; }
        
        const char* what() const throw() {
            switch (_code) {
                case ERROR_INVALID_LOCATION_BLOCK:
                    return "Invalid location block configuration";
                case ERROR_INVALID_PREFIX:
                    return "Invalid location prefix";
                case ERROR_INVALID_ROOT_PATH:
                    return "Invalid root path";
                case ERROR_ROOT_PATH_NO_ACCESS:
                    return "Root directory has no read access";
                case ERROR_ROOT_PATH_NOT_DIRECTORY:
                    return "Root path is not a directory";
                case ERROR_ROOT_PATH_NOT_FOUND:
                    return "Root path not found";
                case ERROR_INVALID_INDEX_FORMAT:
                    return "Invalid index file format (must have valid extension)";
                case ERROR_INDEX_FILE_NOT_FOUND:
                    return "Index file not found";
                case ERROR_INDEX_NOT_A_FILE:
                    return "Index path is not a regular file";
                case ERROR_INDEX_FILE_NO_ACCESS:
                    return "Index file has no read access";
                case ERROR_INVALID_UPLOAD_PATH:
                    return "Invalid upload path";
                case ERROR_DUPLICATE_LOCATION:
                    return "Duplicate location path";
                case ERROR_INVALID_REDIRECT:
                    return "Invalid redirection in location block";
                case ERROR_LOOPING_REDIRECT:
                    return "Redirect loop detected in location block";
                case ERROR_INVALID_ALLOWED_METHODS:
                    return "Invalid allowed methods (must be GET, POST, or DELETE)";
                case ERROR_INVALID_INDEX_FILES:
                    return "Invalid index files configuration";
                case ERROR_INVALID_ERROR_PAGE:
                    return "Invalid error page";
                case ERROR_INVALID_CGI_PATH:
                    return "Invalid CGI path (must be executable file)";
                case ERROR_INVALID_AUTOINDEX:
                    return "Invalid autoindex value (use 'on' or 'off')";
                case ERROR_UNKNOWN_KEY:
                    return "Unknown directive in location block";
                default:
                    return "Unknown location configuration error";
            }
        }
    };
};

#endif
