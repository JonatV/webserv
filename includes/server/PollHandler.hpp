#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP

#include <sys/poll.h>
#include <vector>

class PollHandler {
    private :
        std::vector<struct pollfd> pollfds;
        int timeout;
    public :
        PollHandler();
        ~PollHandler();
        void addSocket(int fd);
        void removeSocket(int fd);
        int pollSockets();
        int getNumFds();
        int getRevents(int index);
        const std::vector<struct pollfd>& getPollfd() const { return pollfds; } // Add getter method
};

#endif