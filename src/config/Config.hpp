/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:08 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/27 17:12:11 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <vector>
# include <map>
# include "ServerConfig.hpp"

class Config {
	private:
		std::vector<ServerConfig>	_servers;

	public:
		Config();
		~Config();

		// Different possible errors from the config file
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
};

#endif