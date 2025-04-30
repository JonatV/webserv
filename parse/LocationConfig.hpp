/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:12 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/30 17:17:14 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <iostream>
# include <vector>
# include <map>
# include <sys/stat.h>
# include <unistd.h>

class LocationConfig {
	private:
		// Attributes of location
		std::string					_locationName;
		std::string					_index;
		std::vector<std::string>	_allowedMethods;
		std::string					_locationRoot;
		bool						_autoindex;

		// Functions to parse location
		std::string *getIndex(std::vector<std::string> tokens, size_t i, const std::string& rootPath);
		std::string	*getAllowedMethods(std::vector<std::string> tokens);
		std::string *getRoot(std::vector<std::string> tokens, size_t i);
		bool		*getAutoIndex(std::vector<std::string> tokens);
	public:
		LocationConfig();
		LocationConfig(std::string);
		~LocationConfig();

		// Getters
		std::string					getLocationName() const;
		std::string					getLocationRoot() const;
		std::string					getLocationIndex() const;
		std::vector<std::string>	getLocationAllowedMethods() const;
		bool						getLocationAutoIndex() const;

		friend class ServerConfig;
		friend class Config;
		friend class WebServer;
		
		// Different possible errors from the location part of the config file
		enum e_error {
			ERROR_NONE = 0,

			// Location Errors
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

			// Routing & Redirection Errors
			ERROR_INVALID_REDIRECT = 220,
			ERROR_LOOPING_REDIRECT,

			// Request Handling Errors
			ERROR_INVALID_ALLOWED_METHODS = 230,
			ERROR_INVALID_INDEX_FILES,
			ERROR_INVALID_ERROR_PAGE,
			ERROR_INVALID_CGI_PATH,

			// Security & Limits
			ERROR_INVALID_AUTOINDEX = 240,

			// Configuration Key Errors
			ERROR_UNKNOWN_KEY = 250
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
						case ERROR_INVALID_LOCATION_BLOCK:
							return "Invalid location block configuration";
						case ERROR_INVALID_PREFIX:
							return "Invalid location prefix";
						case ERROR_INVALID_ROOT_PATH:
							return "Invalid root path";
						case ERROR_ROOT_PATH_NO_ACCESS:
							return "Root in location block has no read access";
						case ERROR_ROOT_PATH_NOT_DIRECTORY:
							return "Root in location block is not a directory";
						case ERROR_ROOT_PATH_NOT_FOUND:
							return "Root path not found in location block";
						case ERROR_INVALID_INDEX_FORMAT:
							return "Invalid index file format in location block";
						case ERROR_INDEX_FILE_NOT_FOUND:
							return "Index file not found in location block";
    					case ERROR_INDEX_NOT_A_FILE:
							return "Index is not a file in location block";
    					case ERROR_INDEX_FILE_NO_ACCESS:
							return "Index file has no read access in location block";
						case ERROR_INVALID_UPLOAD_PATH:
							return "Invalid upload path";
						case ERROR_DUPLICATE_LOCATION:
							return "Duplicate location path";
						case ERROR_INVALID_REDIRECT:
							return "Invalid redirection in location block";
						case ERROR_LOOPING_REDIRECT:
							return "Redirect loop detected in location block";
						case ERROR_INVALID_ALLOWED_METHODS:
							return "Invalid allowed methods";
						case ERROR_INVALID_INDEX_FILES:
							return "Invalid index files";
						case ERROR_INVALID_ERROR_PAGE:
							return "Invalid error page";
						case ERROR_INVALID_CGI_PATH:
							return "Invalid CGI path";
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
