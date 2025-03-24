/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stringManipulation.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 16:53:55 by jveirman          #+#    #+#             */
/*   Updated: 2025/03/24 16:57:31 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRINGMANIPULATION_HPP
#define STRINGMANIPULATION_HPP

#include <sstream>
#include <string>

template <typename T>
std::string to_string(const T& value)
{
	std::ostringstream	oss;
	oss << value;
	return (oss.str());
}

#endif
