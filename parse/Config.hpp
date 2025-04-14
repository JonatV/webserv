/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:08 by eschmitz          #+#    #+#             */
/*   Updated: 2025/04/14 14:44:46 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <vector>
# include <map>
# include <set>
# include <fstream>
# include <sstream>
# include "ServerConfig.hpp"
# include <sys/stat.h>
# include <unistd.h>

class Config {
	private:
		std::vector<ServerConfig>	_servers;
		std::vector<std::string>	_tokens;
		std::set<std::string>		_nonServerSections; // Store non-server section names

		// Functions to parse the config file
		std::vector<ServerConfig>	*parseServers(std::vector<std::string> tokens);
		std::vector<std::string>	*getTokensFromFile(std::ifstream& file);
		size_t						skipBlock(const std::vector<std::string>& tokens, size_t startPos);
		bool						isNonServerSection(const std::string& token);
		
		public:
		Config();
		~Config();
		friend class WebServer;
		// Function to parse file
		bool 						*parseFile(const std::string& filename);
		
		// Different possible errors from the config file
		enum e_error {
			ERROR_NONE = 0,

			// File Errors
			ERROR_FILE_NOT_FOUND = 1,
			ERROR_FILE_PERMISSION_DENIED,
			ERROR_FILE_EMPTY,
			ERROR_FILE_MALFORMED,
			ERROR_UNEXPECTED_EOF,

			// Routing & Redirection Errors
			ERROR_INVALID_REDIRECT = 10,
			ERROR_LOOPING_REDIRECT,

			// Configuration Key Errors
    		ERROR_UNKNOWN_KEY = 20
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

		// Function to display the informations extracted and put in the class
		void displayConfig() const;
};

#endif