#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <string>
#include <fstream>

#define NOPORT -1

template <typename T>
std::string	to_string(const T& value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

// Tiny gnl
std::string	gnl(std::ifstream& file, bool isRegistered);

namespace logs
{
	enum Color {
		Default = 39,
		Black = 30,
		Red = 31,
		Green = 32,
		Yellow = 33,
		Blue = 34,
		Magenta = 35,
		Cyan = 36,
		White = 37
	};
	void msg(int port, Color portColor, const std::string& message, bool greyMessage = false);
}

#endif
