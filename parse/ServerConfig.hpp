/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/18 12:16:18 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <vector>
# include <map>
# include <iostream>
# include "LocationConfig.hpp"

class ServerConfig {
	private:
		// Attributes of server
		std::vector<int>						_port;
		std::string								_host;
		std::string								_root;
		std::vector<std::string>				_serverName;
		ssize_t									_clientBodyLimit;
		std::map<int, std::string> 				_errorPages;
		std::map<std::string, LocationConfig>	_locations;

		// Functions to parse Server
		std::map<std::string, LocationConfig>	*getLocationConfig(std::vector<std::string> tokens, size_t& i);
		std::vector<int>						getPort(std::vector<std::string> tokens, size_t i, size_t &endPos);
		std::string								*getHost(std::vector<std::string> tokens, size_t i);
		std::string								*getRoot(std::vector<std::string> tokens, size_t i);
		ssize_t									*getClientBodyLimit(std::vector<std::string> tokens, size_t i);
		std::string								*getServerName(std::vector<std::string> tokens, size_t i);

	public:
		ServerConfig();
		~ServerConfig();

		// So that I can access private attributes in the config class
		friend class Config;
		friend class WebServer;
		
		// Different possible errors for the server part of the config file
		enum e_error {
			ERROR_NONE = 0,

			// Server Errors
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

			// Routing & Redirection Errors
			ERROR_INVALID_REDIRECT = 120,
			ERROR_LOOPING_REDIRECT,

			// Security & Limits
			ERROR_INVALID_CLIENT_MAX_BODY_SIZE = 130,

			// Configuration Key Errors
    		ERROR_UNKNOWN_KEY = 140
		};
		
		// Custom exception class
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
							return "Invalid port configuration";
						case ERROR_INVALID_HOST:
							return "Invalid host configuration";
						case ERROR_DUPLICATE_SERVER:
							return "Duplicate server name or server: port combination";
						case ERROR_INVALID_SERVER_NAME:
							return "Invalid server name";
						case ERROR_INVALID_ROOT_PATH:
							return "Invalid root path in server block";
						case ERROR_ROOT_PATH_NO_ACCESS:
							return "Root in server block has no read access";
						case ERROR_ROOT_PATH_NOT_DIRECTORY:
							return "Root in server block is not a directory";
						case ERROR_ROOT_PATH_NOT_FOUND:
							return "Root path not found in server block";
						case ERROR_INVALID_REDIRECT:
							return "Invalid redirection in server block";
						case ERROR_LOOPING_REDIRECT:
							return "Redirect loop detected in server block";
						case ERROR_INVALID_CLIENT_MAX_BODY_SIZE:
							return "Invalid client_max_body_size value";
						case ERROR_UNKNOWN_KEY:
							return "Unknown directive in server block";
						default:
							return "Unknown server configuration error";
					}
				}
			};
};

#endif
