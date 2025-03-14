#include "../../../includes/networking/sockets/ListeningSocket.hpp"
#include <iostream>


http::ListeningSocket::ListeningSocket(int domain, int service, int protocol, int port, unsigned long interface, int bklog)
    : BindingSocket(domain, service, protocol, port, interface) {
        backlog = bklog;
        startListening();
        testConnection(getListening());
}

void http::ListeningSocket::startListening() {
    std::cout << "--- Listening on socket: " << getSock() << std::endl;
    listening = listen(getSock(), backlog);
}

int http::ListeningSocket::getListening() {
    return listening;
}