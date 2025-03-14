#include "../../../includes/networking/sockets/Socket.hpp"
#include <iostream>

//Constructor
http::Socket::Socket(int domain, int service, int protocol, int port, unsigned long interface) {
    //Defining address struct
    address.sin_family = domain;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(interface);

    //Establishing socket
    sock = socket(domain, service, protocol);
    std::cout << "--- Creating socket: " << sock << std::endl;
    testConnection(sock);
}

//Test connection 
void http::Socket::testConnection(int testItem) {
    if (testItem < 0) {
        perror("Failed to connect...");
        exit(EXIT_FAILURE);
    }
}

//Getter functions
struct sockaddr_in http::Socket::getAddress() {
    return address;
}

int http::Socket::getSock() {
    return sock;
}

int http::Socket::getConnection() {
    return connection;
}

void http::Socket::setConnection(int con) {
    connection = con;
}