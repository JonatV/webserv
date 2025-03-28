/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:12 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/28 17:02:19 by eschmitz         ###   ########.fr       */
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
		std::string	_getIndex(std::vector<std::string>);
		std::string	_getAllowedMethods(std::vector<std::string>);
		std::string	_getRoot(std::vector<std::string>);
		bool		_getAutoIndex(std::vector<std::string>);

	public:
		LocationConfig();
		~LocationConfig();

		// Different possible errors from the lcoation part of the config file
		enum e_error {
			ERROR_NONE,

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
			ERROR_INVALID_AUTOINDEX,

			// Configuration Key Errors
    		ERROR_UNKNOWN_KEY 
		};

};

#endif