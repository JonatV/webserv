#ifndef Socket_hpp
#define Socket_hpp

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace http {
    class Socket {
        public:
        //Constructor
            Socket(int domain, int service, int protocol, int port, unsigned long interface);

        //Connect to a network
            virtual int connectToNetwork(int sock, struct sockaddr_in address) = 0;

        //Testing the sock function    
            void testConnection(int testItem);

        //Getter functions
            struct sockaddr_in getAddress();
            int getSock();
            int getConnection();

        //Setter functions
            void setConnection(int con);

        private:
            struct sockaddr_in address;
            int sock;
            int connection;
    };
}



#endif 