/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 14:07:45 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTENER_HPP
#define LISTENER_HPP

class Listener
{
	private:
		int	_serverSocketFd;
		int	_port;
		int	_maxConnections;
};

#endif
