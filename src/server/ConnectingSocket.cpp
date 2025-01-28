#include "../../includes/server/ConnectingSocket.hpp"

//Constructor
http::ConnectingSocket::ConnectingSocket(int domain, int service, int protocol, int port, unsigned long interface)
    : Socket(domain, service, protocol, port, interface) {
        setConnection(connectToNetwork(getSock(), getAddress()));
        testConnection(getConnection());
}

//Definition of ConnectToNetwork virtual function
int http::ConnectingSocket::connectToNetwork(int sock, struct sockaddr_in address) {
    return connect(sock, (struct sockaddr *)&address, sizeof(address));
}