/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/09/19 16:03:02 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <vector>
#include <map>
#include <iostream>
#include "LocationConfig.hpp"

class ServerConfig {
private:
    std::vector<int> _port;
    std::string _host;
    std::string _root;
    std::vector<std::string> _serverName;
    ssize_t _clientBodyLimit;
    std::map<int, std::string> _errorPages;
    std::map<std::string, LocationConfig> _locations;

    // Parsing functions - now return by value and take const references
    std::vector<int> getPort(const std::vector<std::string>& tokens, size_t& i);
    std::string getHost(const std::vector<std::string>& tokens, size_t& i);
    std::string getRoot(const std::vector<std::string>& tokens, size_t& i);
    ssize_t getClientBodyLimit(const std::vector<std::string>& tokens, size_t& i);
    std::string getServerName(const std::vector<std::string>& tokens, size_t& i);
    std::map<std::string, LocationConfig> getLocationConfig(const std::vector<std::string>& tokens, size_t& i);

public:
    ServerConfig();
    ~ServerConfig();

    friend class Config;
    friend class WebServer;

    enum e_error {
        ERROR_NONE = 0,
        ERROR_INVALID_SERVER_BLOCK = 100,
        ERROR_INVALID_PORT,
        ERROR_INVALID_HOST,
        ERROR_DUPLICATE_SERVER,
        ERROR_INVALID_SERVER_NAME,
        ERROR_INVALID_ROOT_PATH,
        ERROR_INVALID_ERROR_PAGE_PATH,
        ERROR_ROOT_PATH_NOT_FOUND,
        ERROR_ROOT_PATH_NOT_DIRECTORY,
        ERROR_ROOT_PATH_NO_ACCESS,
        ERROR_INVALID_REDIRECT = 120,
        ERROR_LOOPING_REDIRECT,
        ERROR_INVALID_CLIENT_MAX_BODY_SIZE = 130,
        ERROR_UNKNOWN_KEY = 140
    };

    class ConfigException : public std::exception {
    private:
        e_error _code;
        
    public:
        ConfigException(e_error code) : _code(code) {}
        e_error getCode() const { return _code; }
        
        const char* what() const throw() {
            switch (_code) {
                case ERROR_INVALID_SERVER_BLOCK:
                    return "Invalid server block configuration";
                case ERROR_INVALID_PORT:
                    return "Invalid port configuration (must be 1024-65535)";
                case ERROR_INVALID_HOST:
                    return "Invalid host configuration";
                case ERROR_DUPLICATE_SERVER:
                    return "Duplicate server name or server:port combination";
                case ERROR_INVALID_SERVER_NAME:
                    return "Invalid server name";
                case ERROR_INVALID_ROOT_PATH:
                    return "Invalid root path in server block";
                case ERROR_ROOT_PATH_NO_ACCESS:
                    return "Root directory has no read access";
                case ERROR_ROOT_PATH_NOT_DIRECTORY:
                    return "Root path is not a directory";
                case ERROR_ROOT_PATH_NOT_FOUND:
                    return "Root path not found";
                case ERROR_INVALID_REDIRECT:
                    return "Invalid redirection in server block";
                case ERROR_LOOPING_REDIRECT:
                    return "Redirect loop detected in server block";
                case ERROR_INVALID_CLIENT_MAX_BODY_SIZE:
                    return "Invalid client_max_body_size value (must be positive)";
                case ERROR_UNKNOWN_KEY:
                    return "Unknown directive in server block";
                default:
                    return "Unknown server configuration error";
            }
        }
    };
};

#endif
