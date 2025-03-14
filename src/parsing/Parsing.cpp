#include "../../includes/parsing/Parsing.hpp"

void trim(std::string &str) {
    while (!str.empty() && (str.back() == ' ' || str.back() == '\t' || str.back() == ';'))
        str.pop_back();
    while (!str.empty() && (str.front() == ' ' || str.front() == '\t'))
        str.erase(str.begin());
}

void printConf(std::ifstream &path) {
    std::stringstream buf;
    buf << path.rdbuf();
    std::string configContent = buf.str();
    std::cout << "Config file content : \n" << configContent << std::endl;
}

void parseServerBlock(std::ifstream &path, int &braceCount) {
    std::string line;

    std::cout << "--- parseServerBlock\n";
    while (std::getline(path, line)) {
        trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find('{') != std::string::npos)
            braceCount++;

        if (line.find('}') != std::string::npos)
            braceCount--;

        if (braceCount == 0) {
            std::cout << "End of server block\n";
            break;
        }

        if (line.find("listen") == 0) {
            std::cout << "Listen directive found: " << line << std::endl;
        } 
        else if (line.find("server_name") == 0) {
            std::cout << "Server name directive found: " << line << std::endl;
        }
        // location block
        else if (line.find("location /404") == 0) {
            std::cout << "Location /404 block found\n";
            while (std::getline(path, line)) {
                trim(line);
                if (line.empty() || line[0] == '#')
                    continue;

                if (line.find('{') != std::string::npos)
                    braceCount++;

                if (line.find('}') != std::string::npos)
                    braceCount--;

                if (braceCount == 1 && line.find("root") == 0) {
                    std::cout << "Root directive found in /404 location: " << line << std::endl;
                }

                if (braceCount == 1 && line.find('}') != std::string::npos) {
                    std::cout << "End of location /404 block\n";
                    break;
                }
            }
        }
    }
}

void parseHttpBlock(std::ifstream &path) {
    std::string line;
    int braceCount = 1;

    while (std::getline(path, line)) {
        trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line == "server {"){
            parseServerBlock(path, braceCount);
        }

        if (line.find('{') != std::string::npos)
            braceCount++;

        if (line.find('}') != std::string::npos)
            braceCount--;

        // std::cout << " --- |" << line << std::endl;

        if (braceCount == 0) {
            std::cout << "End of http block\n";
            break;
        }
    }
}

int readConfig(std::string configPath) {
    std::ifstream path(configPath);
    if (!path.is_open()) {
        std::cerr << "Error: Unable to open config file." << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(path, line)) {
        trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "http {")
            parseHttpBlock(path);
    }
    std::cout << "------------------------------------\n";
    // path.clear(); path.seekg(0); // beginning of path
    // printConf(path);
    
    return 0;
}

int    parseConfigFile(std::string filePath) {
    if (readConfig(filePath) != 1)
        return 1;

    return 0;
}

