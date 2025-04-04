#include "utils.hpp"

std::string gnl(std::ifstream& file)
{
	std::string content = "";
	std::string line;

	while (std::getline(file, line))
		content += line + "\n";
	if (!content.empty())
		content.erase(content.length() - 1);
	file.close();
	return (content);
}
