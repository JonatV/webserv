#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <string>
#include <fstream>

// Convert any type to string
template <typename T>
std::string to_string(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Tiny gnl
std::string gnl(std::ifstream& file, bool isRegistered);

#endif
