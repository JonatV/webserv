#ifndef SIGNALS_HPP
#define SIGNALS_HPP

#include <signal.h>

class SignalHandler {
private:
    static volatile sig_atomic_t _shutdown;
    static void signalHandler(int signum);

public:
    static void setupSignals();
    static bool shouldShutdown();
};

#endif
