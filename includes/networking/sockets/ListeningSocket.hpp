#ifndef ListeningSocket_hpp
#define ListeningSocket_hpp

#include "BindingSocket.hpp"

namespace http {
    class ListeningSocket : public BindingSocket {
        public :
            ListeningSocket(int domain, int service, int protocol, int port, unsigned long interface, int bklog);
            void startListening();
            int  getListening();
        private :
            int backlog;
            int listening;
    };
}

#endif