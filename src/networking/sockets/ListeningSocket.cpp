#include "../../../includes/networking/sockets/ListeningSocket.hpp"


http::ListeningSocket::ListeningSocket(int domain, int service, int protocol, int port, unsigned long interface, int bklog)
    : BindingSocket(domain, service, protocol, port, interface) {
        backlog = bklog;
        startListening();
        testConnection(getListening());
}

void http::ListeningSocket::startListening() {
    listening = listen(getSock(), backlog);
}

int http::ListeningSocket::getListening() {
    return listening;
}