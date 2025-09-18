/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Evaluator.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 15:19:01 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/18 13:54:52 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVALUATOR_HPP
# define EVALUATOR_HPP

#include <string>
#include <fstream>
#include <iostream>

class Evaluator {
	private:
		std::string	_name;
		std::string	_balance;
		std::string	_pictureURL;

		void update_hack_html();
	public:
		Evaluator();
		~Evaluator();
};

#endif
