#include "../../includes/server/Server.hpp"

http::Server::Server() : SimpleServer(AF_INET, SOCK_STREAM, 0, 2121, INADDR_ANY, 10) {
    launch();
}

void http::Server::accepter() {
    struct sockaddr_in address = getListeningSocket()->getAddress();
    int addrlen = sizeof(address);
    newSocket = accept(getListeningSocket()->getSock(),
        (struct sockaddr *)&address, (socklen_t *)&addrlen);
    
    if (newSocket >= 0) {
        std::cout << "New connection accepted: " << newSocket << std::endl;
        pollHandler.addSocket(newSocket);
    } else {
        std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
    }
}

void http::Server::handler() {
    int ret = pollHandler.pollSockets();
    if (ret > 0) {
        for (int i = 0; i < pollHandler.getNumFds(); i++) {
            if (pollHandler.getRevents(i) & POLLIN) {
                int clientSocket = pollHandler.getPollfd()[i].fd;
                std::cout << "Handling data from socket: " << clientSocket << std::endl;
                read(clientSocket, buffer, 30000);
                responder(clientSocket);
                pollHandler.removeSocket(clientSocket);
            }
        }
    } else if (ret < 0) {
        std::cerr << "Poll error: " << strerror(errno) << std::endl;
    }
}

void http::Server::responder(int clientSocket) {
    std::string request(buffer);
    std::string filePath = "./content/index.html";

    if (request.find("GET /index.html") != std::string::npos) {
        filePath = "./content/index.html";
    }

    std::ifstream file(filePath);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string htmlContent = buffer.str();
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(htmlContent.length()) + "\r\n"
            "\r\n" + htmlContent;
        write(clientSocket, response.c_str(), response.length());
    } else {
        const char *notFoundContent = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 45\r\n"
            "\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        write(clientSocket, notFoundContent, strlen(notFoundContent));
    }
    close(clientSocket);
}

void http::Server::launch() {
    while (true) {
        std::cout << "Waiting for connections..." << std::endl;
        accepter();
        handler();
        std::cout << "Done handling connections." << std::endl;
    }
}