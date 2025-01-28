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
    std::cout << buffer << std::endl;
}

void http::TestServer::responder() {
    const char *helloMessage = "Hello from server";
    write(newSocket, helloMessage, strlen(helloMessage));
    close(newSocket);
}

void http::TestServer::launch() {
    while (true) {
        std::cout << "Waiting..." << std::endl;
        accepter();
        handler();
        responder();
        std::cout << "Done !" << std::endl;
    }
}