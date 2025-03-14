#include "../../includes/server/SimpleServer.hpp"

http::SimpleServer::SimpleServer(int domain, int service, int protocol, int port, unsigned long interface, int bklog) {
    socket = new ListeningSocket(domain, service, protocol, port, interface, bklog);
}

http::ListeningSocket * http::SimpleServer::getListeningSocket() {
    return socket;
}

int http::SimpleServer::accepter() {
    return 0;
}