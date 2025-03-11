#include "../../includes/server/TestServer.hpp"

http::TestServer::TestServer() : SimpleServer(AF_INET, SOCK_STREAM, 0, 2121, INADDR_ANY, 10) {
    launch();
}

void http::TestServer::accepter() {
    struct sockaddr_in address = getListeningSocket()->getAddress();
    int addrlen = sizeof(address);
    newSocket = accept(getListeningSocket()->getSock(),
        (struct sockaddr *)&address, (socklen_t *)&addrlen);
    
    read(newSocket, buffer, 30000);
}

void http::TestServer::handler() {
    std::cout << "--- this is the buffer : \n" << std::endl;
    std::cout << buffer;
    std::cout << "--- this is the end of the buffer :" << std::endl;
}

void http::TestServer::responder() {
    std::string request(buffer);
    std::string filePath = "./content/index.html"; // Default to 404 error page

    // Check if the request is for the index.html page
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
        write(newSocket, response.c_str(), response.length());
    } else {
        const char *notFoundContent = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 45\r\n"
            "\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        write(newSocket, notFoundContent, strlen(notFoundContent));
    }
    close(newSocket);
}


void http::TestServer::launch() {
    while (true) {
        std::cout << "Waiting..." << std::endl;
        accepter();
        std::cout << "ACCEPTED ";
        handler();
        responder();
        std::cout << "Done !\n" << std::endl;
    }
}