#ifndef ConnectingSocket_hpp
#define ConnectingSocket_hpp

#include "Socket.hpp"

namespace http {
    class ConnectingSocket : public Socket {
        public:
        //Constructor
            ConnectingSocket(int domain, int service, int protocol, int port, unsigned long interface);

        //Virtual function from parent
        int connectToNetwork(int sock, struct sockaddr_in address);
    };
}

#endif