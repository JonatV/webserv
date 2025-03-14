#ifndef SimpleServer_hpp
#define SimpleServer_hpp

#include <sstream>
#include <fstream>
#include "../networking/sockets/AllSockets.hpp"

namespace http {
    class SimpleServer {
        public :
            SimpleServer(int domain, int service, int protocol, int port, unsigned long interface, int bklog);
            virtual void launch() = 0;
            ListeningSocket *getListeningSocket();
        private :
            ListeningSocket *socket;
            virtual int accepter();
            virtual void handler() = 0;
            virtual void responder(int newSocket) = 0;
    };
}

#endif