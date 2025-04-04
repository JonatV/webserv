/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:12 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/04 12:48:02 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <iostream>
# include <vector>
# include <map>

class LocationConfig {
	private:
		// Attributes of location
		std::string					_index;
		std::vector<std::string>	_allowedMethods;
		std::string					_root;
		bool						_autoindex;

		// Functions to parse location
		std::string	*getIndex(std::vector<std::string> tokens);
		std::string	*getAllowedMethods(std::vector<std::string> tokens);
		std::string	*getRoot(std::vector<std::string>  tokens);
		bool		*getAutoIndex(std::vector<std::string> tokens);

	public:
		LocationConfig();
		~LocationConfig();

		friend class ServerConfig;
		friend class Config;
		
		// Different possible errors from the location part of the config file
		enum e_error {
			ERROR_NONE = 0,

			// Location Errors
			ERROR_INVALID_LOCATION_BLOCK = 200,
			ERROR_INVALID_PREFIX,
			ERROR_INVALID_ROOT_PATH,
			ERROR_INVALID_UPLOAD_PATH,
			ERROR_DUPLICATE_LOCATION,

			// Routing & Redirection Errors
			ERROR_INVALID_REDIRECT = 210,
			ERROR_LOOPING_REDIRECT,

			// Request Handling Errors
			ERROR_INVALID_ALLOWED_METHODS = 220,
			ERROR_INVALID_INDEX_FILES,
			ERROR_INVALID_ERROR_PAGE,
			ERROR_INVALID_CGI_PATH,

			// Security & Limits
			ERROR_INVALID_AUTOINDEX = 230,

			// Configuration Key Errors
    		ERROR_UNKNOWN_KEY = 240
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