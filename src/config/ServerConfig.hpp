/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/27 17:10:01 by eschmitz         ###   ########.fr       */
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
		std::vector<int>	_port;
		std::string	_host;
		std::string	_serverName;
		size_t	_clientBodyLimit;
		std::map<int, std::string> _errorPages;
		std::vector<LocationConfig>	_locations;

	public:
		ServerConfig();
		~ServerConfig();
};

#endif