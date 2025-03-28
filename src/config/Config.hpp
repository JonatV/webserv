/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:08 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/28 17:11:13 by eschmitz         ###   ########.fr       */
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
		std::vector<std::string>	_tokens;

		// Functions to parse the config file
		ServerConfig				_parseServer(std::vector<std::string>);
		std::vector<std::string>	_getTokensFromFile(std::string);

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

			// Routing & Redirection Errors
			ERROR_INVALID_REDIRECT,
			ERROR_LOOPING_REDIRECT,

			// Configuration Key Errors
    		ERROR_UNKNOWN_KEY 
    	};
};

#endif