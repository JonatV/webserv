/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:46:15 by jveirman          #+#    #+#             */
/*   Updated: 2025/04/01 14:27:30 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <sys/epoll.h>

class Epoll
{
	private:
		int		_epollFd;
		struct	epoll_event	*_events;
		int		_maxEvents;
};

#endif
