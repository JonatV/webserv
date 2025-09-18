#include "utils.hpp"
#include <iostream>

std::string gnl(std::ifstream& file, bool isRegistered)
{
	std::string content = "";
	std::string line;

	while (std::getline(file, line))
	{
		if (line == "</body>" && !isRegistered)
			line = "<a href=\"/register\" class=\"register_link\">Register</a>\n" + line;
		content += line + "\n";
	}
	if (!content.empty())
		content.erase(content.length() - 1);
	file.close();
	return (content);
}

namespace logs
{
	static inline void setColor(int code) { std::cout << "\033[" << code << 'm'; }
	static inline void reset() { std::cout << "\033[0m"; }

	void msg(int port, Color portColor, const std::string& message, bool greyMessage)
	{
		setColor(portColor);
		if (port == NOPORT)
			std::cout << "[####]";
		else
			std::cout << '[' << port << ']';
		reset();
		std::cout << '\t';
		if (greyMessage) setColor(2);
		std::cout << message;
		reset();
		std::cout << std::endl;
	}
}
