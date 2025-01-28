#ifndef TestServer_hpp
#define TestServer_hpp

#include "SimpleServer.hpp"

namespace http {
    class TestServer : public SimpleServer {
        public :
            TestServer();
            void launch();
        private :
            char buffer[30000];
            int newSocket;
            
            void accepter();
            void handler();
            void responder();
    };
}

#endif