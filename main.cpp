#include "./includes/Networkinglib.hpp"
#include "./includes/parsing/Parsing.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "./webserv <config_file path>" << std::endl;
        return 1;
    }
    std::string configPath = argv[1];
    // if (parseConfigFile(configPath) == 1)
    //     return 1;

    http::Server test;
    return 0;
}