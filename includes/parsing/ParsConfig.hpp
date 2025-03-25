/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 18:47:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/25 11:45:41 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSCONFIG_HPP
# define PARSCONFIG_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <map>

class Configuration {
	private:
		void	parsConfigFile(std::string configFile);
	
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
			ERROR_INVALID_AUTOINDEX
    	};
		

		// Function to get error message as a string
		static std::string getErrorMessage(e_error error) {
			switch (error) {
				case ERROR_NONE: return "No errors detected.";
				
				// File Errors
				case ERROR_FILE_NOT_FOUND: return "Configuration file not found.";
				case ERROR_FILE_PERMISSION_DENIED: return "Permission denied: Cannot read the configuration file.";
				case ERROR_FILE_EMPTY: return "The configuration file is empty.";
				case ERROR_FILE_MALFORMED: return "Malformed configuration file: Check syntax.";
				case ERROR_UNEXPECTED_EOF: return "Unexpected end of file: Possibly unclosed brackets.";

				// Server Errors
				case ERROR_INVALID_SERVER_BLOCK: return "Invalid or missing [server] block in configuration.";
				case ERROR_INVALID_PORT: return "Invalid port: Must be between 0 and 65535.";
				case ERROR_INVALID_HOST: return "Invalid host: Must be a valid IP or domain name.";
				case ERROR_DUPLICATE_SERVER: return "Duplicate server definition: Host and port must be unique.";
				case ERROR_INVALID_SERVER_NAME: return "Invalid server_name: Contains invalid characters.";

				// Location Errors
				case ERROR_INVALID_LOCATION_BLOCK: return "Invalid or missing [location] block.";
				case ERROR_INVALID_PREFIX: return "Invalid location prefix: Must be a valid path.";
				case ERROR_INVALID_ROOT_PATH: return "Invalid root path: Directory does not exist.";
				case ERROR_INVALID_UPLOAD_PATH: return "Invalid upload path: Check directory and permissions.";
				case ERROR_DUPLICATE_LOCATION: return "Duplicate location prefix in configuration.";

				// Routing & Redirection Errors
				case ERROR_INVALID_REDIRECT: return "Invalid redirect format or missing destination URL.";
				case ERROR_LOOPING_REDIRECT: return "Detected infinite redirect loop.";

				// Request Handling Errors
				case ERROR_INVALID_ALLOWED_METHODS: return "Invalid allowed_methods: Contains unknown HTTP methods.";
				case ERROR_INVALID_INDEX_FILES: return "Invalid index files: Must be valid filenames.";
				case ERROR_INVALID_ERROR_PAGE: return "Invalid error page definition: Check format.";
				case ERROR_INVALID_CGI_PATH: return "Invalid CGI path: Must be an executable file.";

				// Security & Limits
				case ERROR_INVALID_CLIENT_MAX_BODY_SIZE: return "Invalid client_max_body_size: Must be a valid size (e.g., 1MB, 10KB).";
				case ERROR_INVALID_AUTOINDEX: return "Invalid autoindex value: Must be 'on' or 'off'.";

				default: return "Unknown error.";
			}
		}
};

#endif