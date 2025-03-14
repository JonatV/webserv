#ifndef PARSING_HPP
#define PARSING_HPP

#include "../Networkinglib.hpp"

void    trim(std::string &str);
int     readConfig(std::string configPath);
int     parseConfigFile(std::string filePath);
void    printConf(std::ifstream &path);

#endif