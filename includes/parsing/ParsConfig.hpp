/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 18:47:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/24 19:02:33 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSCONFIG_HPP
# define PARSCONFIG_HPP

# include <iostream>
# include <fstream>

class Configuration {
	private:
		void	parsConfigFile(std::string configFile);
	
	public:
		Configuration();
		~Configuration();
		enum config_error {
			ERROR_NONE,
			ERROR_UNKNOWN_KEY,
			ERROR_INVALID_PORT,
			ERROR_INVALID_HOST,
			ERROR_INVALID_SERVER_NAME,
			ERROR_INVALID_INDEX,
			ERROR_INVALID_ALLOWED_METHODS,
			ERROR_INVALID_ERROR_PAGE,
			ERROR_INVALID_ROOT,
			ERROR_INVALID_LOCATION,
			ERROR_INVALID_PREFIX,
			ERROR_INVALID_REDIRECT,
			ERROR_INVALID_CGI_PATH,
			ERROR_INVALID_AUTOINDEX,
			ERROR_INVALID_UPLOAD_PATH,
		};
		config_error	error;
		
}

#endif