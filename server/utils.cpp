#include "utils.hpp"

std::string gnl(std::ifstream& file)
{
	std::string content = "";
	std::string line;

	while (std::getline(file, line))
	{
		//inject and href before the end of the body // todo improve
		// !client->getStatus() 
		if (line == "</body>")
			line = "<a href=\"/register\" class=\"register_link\">Register</a>\n" + line;
		content += line + "\n";
	}
	if (!content.empty())
		content.erase(content.length() - 1);
	file.close();
	return (content);
}
