/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/28 17:09:33 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVECONFIG_HPP

# include <vector>
# include <map>
# include <iostream>
# include "LocationConfig.hpp"

class ServerConfig {
	private:
		// Attributes of server
		std::vector<int>						_port;
		std::string								_host;
		std::string								_serverName;
		size_t									_clientBodyLimit;
		std::map<int, std::string> 				_errorPages;
		std::map<std::string, LocationConfig>	_locations;

		// Functions to parse Server
		std::map<std::string, LocationConfig>	_getLocationConfig(std::vector<std::string>);
		int										_getPort(std::vector<std::string>);
		std::string								_getHost(std::vector<std::string>);
		size_t									_getClientBodyLimit(std::vector<std::string>);
		std::string								_getServerName(std::vector<std::string>);


	public:
		ServerConfig();
		~ServerConfig();

		// Different possible errors for the server part of the config file
		enum e_error {
			ERROR_NONE,

			// Server Errors
			ERROR_INVALID_SERVER_BLOCK,
			ERROR_INVALID_PORT,
			ERROR_INVALID_HOST,
			ERROR_DUPLICATE_SERVER,
			ERROR_INVALID_SERVER_NAME,

			// Routing & Redirection Errors
			ERROR_INVALID_REDIRECT,
			ERROR_LOOPING_REDIRECT,

			// Security & Limits
			ERROR_INVALID_CLIENT_MAX_BODY_SIZE,

			// Configuration Key Errors
    		ERROR_UNKNOWN_KEY 
		};
};

#endif