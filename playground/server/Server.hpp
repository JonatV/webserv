/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 13:59:15 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Listener.hpp"
#include "Epoll.hpp"
#include "Client.hpp"
#include <vector>
#include <map>

class Server
{
	private:
		std::vector<Listener>	_listeners;
		std::map<int, Client>	_clients;
		Epoll					_observer;
};

#endif
