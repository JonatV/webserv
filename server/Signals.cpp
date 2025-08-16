#include "Signals.hpp"
#include <iostream>

// Static member definition
volatile sig_atomic_t SignalHandler::_shutdown = 0;

void SignalHandler::signalHandler(int signum) {
    if (signum == SIGTERM || signum == SIGQUIT || signum == SIGINT) {
        _shutdown = 1;
        std::cout << "\nReceived signal " << signum << ". Initiating graceful shutdown..." << std::endl;
    }
}

void SignalHandler::setupSignals() {
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGINT, signalHandler);
}

bool SignalHandler::shouldShutdown() {
    return _shutdown != 0;
}
