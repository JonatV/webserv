#include "../../../includes/networking/sockets/BindingSocket.hpp"

//Constructor
http::BindingSocket::BindingSocket(int domain, int service, int protocol, int port, unsigned long interface) 
    : Socket(domain, service, protocol, port, interface) {
        setConnection(connectToNetwork(getSock(), getAddress()));
        testConnection(getConnection());
    }

//Definition of connectToNetwork virtual function
int http::BindingSocket::connectToNetwork(int sock, struct sockaddr_in address) {
    return bind(sock, (struct sockaddr *)&address, sizeof(address));
} 