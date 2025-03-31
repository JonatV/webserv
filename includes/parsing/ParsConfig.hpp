/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 18:47:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/31 14:00:14 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSCONFIG_HPP
# define PARSCONFIG_HPP

# include <iostream>
# include <fstream>
# include <map>
# include <sstream>
# include <vector>
# include <regex>

class Configuration {
	private:
		void	parsConfigFile(std::string configFile);
		int		_port;
		std::string	_mimeTypes;
		std::string	_defaultType;
		int		_keepAliveTimeout;
		std::string	_serverName;

	
	public:
		Configuration();
		~Configuration();
		enum e_error {
			ERROR_NONE,

			// File Errors
			ERROR_FILE_NOT_FOUND,
			ERROR_FILE_PERMISSION_DENIED,
			ERROR_FILE_EMPTY,
			ERROR_FILE_MALFORMED,
			ERROR_UNEXPECTED_EOF,

			// Server Errors
			ERROR_INVALID_SERVER_BLOCK,
			ERROR_INVALID_PORT,
			ERROR_INVALID_HOST,
			ERROR_DUPLICATE_SERVER,
			ERROR_INVALID_SERVER_NAME,

			// Location Errors
			ERROR_INVALID_LOCATION_BLOCK,
			ERROR_INVALID_PREFIX,
			ERROR_INVALID_ROOT_PATH,
			ERROR_INVALID_UPLOAD_PATH,
			ERROR_DUPLICATE_LOCATION,

			// Routing & Redirection Errors
			ERROR_INVALID_REDIRECT,
			ERROR_LOOPING_REDIRECT,

			// Request Handling Errors
			ERROR_INVALID_ALLOWED_METHODS,
			ERROR_INVALID_INDEX_FILES,
			ERROR_INVALID_ERROR_PAGE,
			ERROR_INVALID_CGI_PATH,

			// Security & Limits
			ERROR_INVALID_CLIENT_MAX_BODY_SIZE,
			ERROR_INVALID_AUTOINDEX,

			// Configuration Key Errors
    		ERROR_UNKNOWN_KEY 
    	};
		
		// Function to get error message as a string
		std::string getErrorMessage(e_error error);

		// Checks if the server block is valid
		e_error validateServerBlock(const ServerConfig& server);

		// Checks for duplicate server names or blocks
		e_error checkDuplicateServers(const std::vector<ServerConfig>& servers, const ServerConfig& new_server);

		// Parses a single server block
		e_error parseServerBlock(std::ifstream& file, ServerConfig& server);

		// Main function to load and validate the configuration
		e_error loadConfig(const std::string& filename, ServerConfig& server);
};

#endif