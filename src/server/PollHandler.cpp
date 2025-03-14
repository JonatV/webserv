#include "../../includes/Networkinglib.hpp"

PollHandler::PollHandler() {
    timeout = 5000;
}

PollHandler::~PollHandler() {
}

void PollHandler::addSocket(int fd) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pollfds.push_back(pfd);
    std::cout << "--- Socket added: " << fd << " | Total socket : " << getNumFds() << std::endl;
}

void PollHandler::removeSocket(int fd) {
    for (std::vector<struct pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it) {
        if (it->fd == fd) {
            pollfds.erase(it);
            std::cout << "--- Socket removed: " << fd << std::endl;
            break;
        }
    }
}

int PollHandler::pollSockets() {
    int ret = poll(pollfds.data(), pollfds.size(), timeout);
    if (ret < 0) {
        std::cerr << "--- Poll error: " << strerror(errno) << std::endl;
        return -1;
    }

    return ret;
}

int PollHandler::getNumFds() {
    return pollfds.size();
}

int PollHandler::getRevents(int index) {
    if (index >= 0 && static_cast<size_t>(index) < pollfds.size())
        return pollfds[index].revents;

    return 0;
}