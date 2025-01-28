#ifndef BindingSocket_hpp
#define BindingSocket_hpp

#include "Socket.hpp"

namespace http {
    class BindingSocket : public Socket {
        public:
        //Constructor
            BindingSocket(int domain, int service, int protocol, int port, unsigned long interface);

        //Virtual function from parent
            int connectToNetwork(int sock, struct sockaddr_in address);
    };
}

#endif