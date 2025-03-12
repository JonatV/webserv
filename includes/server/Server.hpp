#ifndef SERVER_HPP
#define SERVER_HPP

#include "SimpleServer.hpp"
#include "PollHandler.hpp"

namespace http {
    class Server : public SimpleServer {
        public :
            Server();
            void launch();
        private :
            char buffer[30000];
            int newSocket;
            PollHandler pollHandler;
            
            void accepter();
            void handler();
            void responder(int clientSocket);
    };
}

#endif