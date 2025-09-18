/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Evaluator.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jveirman <jveirman@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 15:18:46 by jveirman          #+#    #+#             */
/*   Updated: 2025/09/18 14:10:21 by jveirman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Evaluator.hpp"

Evaluator::Evaluator() {
	// read a file from misc/evaluator.conf
	// extract the name, balance, and picture url
	// one info per line
	// 1st line: name
	// 2nd line: balance
	// 3rd line: picture url
	std::ifstream file("config/evaluator.conf");
	if (file.is_open()) {
		std::getline(file, _name);
		std::getline(file, _balance);
		std::getline(file, _pictureURL);
		file.close();
		if (_name.empty() || _balance.empty() || _pictureURL.empty()) {
			_name = "Eschmitz";
			_balance = "-42";
			_pictureURL = "https://cdn.intra.42.fr/users/d3a7ccedc76389e1859d7ddc54968fb1/eschmitz.jpg";
		}
	} else {
		_name = "Eschmitz";
		_balance = "-42";
		_pictureURL = "https://cdn.intra.42.fr/users/d3a7ccedc76389e1859d7ddc54968fb1/eschmitz.jpg";
		std::cerr << "Error: Could not open evaluator.conf for reading. Using default values." << std::endl;
	}
	update_hack_html();
}

Evaluator::~Evaluator() {
}

void Evaluator::update_hack_html() {
	std::string content;
	std::ifstream hackFile("www/hack.html");
	if (hackFile.is_open()) {
		std::string line;
		while (std::getline(hackFile, line)) {
			content += line + "\n";
		}
		hackFile.close();
	} else {
		std::cerr << "Error: Could not open hack.html for reading." << std::endl;
		return;
	}
	std::string imgTag = "{{pic}}";
	size_t pos = content.find(imgTag);
	if (pos != std::string::npos)
		content.replace(pos, imgTag.length(), _pictureURL);
	std::string nameTag = "{{name}}";
	pos = content.find(nameTag);
	if (pos != std::string::npos)
		content.replace(pos, nameTag.length(), _name);
	std::string balanceTag = "{{balance}}";
	pos = content.find(balanceTag);
	if (pos != std::string::npos)
		content.replace(pos, balanceTag.length(), _balance);
	std::ofstream outFile("www/hack.html");
	if (outFile.is_open()) {
		outFile << content;
		outFile.close();
	} else {
		std::cerr << "Error: Could not open hack.html for writing." << std::endl;
		return;
	}
}
