/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:14 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/27 17:04:35 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVECONFIG_HPP

# include <vector>
# include <map>
# include <iostream>
# include <sstream>

class ServerConfig {
	private:
		std::vector<int>	_port;
		std::string	_host;
		std::string	_serverName;
		size_t	_clientBodyLimit;
		std::string	_root;
		

	public:
		ServerConfig();
		~ServerConfig();
};

#endif