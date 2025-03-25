#include "../../includes/server/Server.hpp"
#include "../../includes/tools/stringManipulation.hpp"

http::Server::Server() : SimpleServer(AF_INET, SOCK_STREAM, 0, 2121, INADDR_ANY, 10) {
    launch();
}

int http::Server::accepter() {
    struct sockaddr_in address = getListeningSocket()->getAddress();
    int addrlen = sizeof(address);
    newSocket = accept(getListeningSocket()->getSock(),
        (struct sockaddr *)&address, (socklen_t *)&addrlen);
    (void)addrlen;

    if (newSocket >= 0) {
        std::cout << "--- New connection accepted: " << newSocket << std::endl;
        pollHandler.addSocket(newSocket);
    }
    else {
        std::cerr << "--- Failed to accept connection: " << strerror(errno) << std::endl;
        return 1;
    }
    return 0;
}

void http::Server::handler() {
    int ret = pollHandler.pollSockets();

    if (ret > 0) {
        for (int i = 0; i < pollHandler.getNumFds(); i++) {
            if (pollHandler.getRevents(i) & POLLIN) {
                int clientSocket = pollHandler.getPollfd()[i].fd;
                std::cout << "--- Handling data from socket: " << clientSocket << std::endl;
                memset(buffer, 0, sizeof(buffer));
                int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    responder(clientSocket);
                }
                else if (bytesRead == 0) {
                    std::cout << "--- Client disconnected: " << clientSocket << std::endl;
                    pollHandler.removeSocket(clientSocket);
                    close(clientSocket);
                }
                else
                    std::cerr << "--- Read error: " << strerror(errno) << std::endl;
            }
        }
    }
    else if (ret < 0)
        std::cerr << "--- Poll error: " << strerror(errno) << std::endl;
}

void http::Server::notFound(int clientSocket) {
    std::ifstream notFoundFile("./config/content/www/404error.html");
    
    if (notFoundFile.is_open()) {
        std::stringstream buffer;
        buffer << notFoundFile.rdbuf();
        std::string fileContent = buffer.str();
        std::string response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + to_string(fileContent.length()) + "\r\n"
            "\r\n" + fileContent;
        write(clientSocket, response.c_str(), response.length());
    } else {
        std::string response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        write(clientSocket, response.c_str(), response.length());
    }
}

void http::Server::responder(int clientSocket) {
    std::cout << "--- Responding to client..." << std::endl;
    std::cout << buffer << std::endl;
    std::string request(buffer);
    std::string filePath;

    if (request.find("GET / ") != std::string::npos) {
        filePath = "./config/content/www/index.html";
    } else if (request.find("GET /404error.html") != std::string::npos) {
        filePath = "./config/content/www/404error.html";
    } else {
        filePath = "";
    }

    if (!filePath.empty()) {
        std::ifstream file(filePath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string fileContent = buffer.str();
            std::string response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: " + getContentType(filePath) + "\r\n"
                "Content-Length: " + to_string(fileContent.length()) + "\r\n"
                "\r\n" + fileContent;
            write(clientSocket, response.c_str(), response.length());
        } else {
            notFound(clientSocket);
        }
    } else {
        notFound(clientSocket);
    }
    close(clientSocket);
}

std::string http::Server::getContentType(const std::string& filePath) {
    if (filePath.find(".html") != std::string::npos)
        return "text/html";

    return "text/plain";
}

void http::Server::launch() {
    while (true) {
        std::cout << "--- Waiting for connections..." << std::endl;
        if (accepter() != 0) {
            std::cerr << "--- Error accepting connection." << std::endl;
            break;
        }
        handler();
        std::cout << "--- Done handling connections.\n" << std::endl;
    }
}
