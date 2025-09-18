#include "Signals.hpp"
#include <iostream>

volatile sig_atomic_t SignalHandler::_shutdown = 0;

void	SignalHandler::signalHandler(int signum) {
	if (signum == SIGTERM || signum == SIGQUIT || signum == SIGINT) {
		_shutdown = 1;
	}
}

void	SignalHandler::setupSignals() {
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGINT, signalHandler);
}

bool	SignalHandler::shouldShutdown() {
	return _shutdown != 0;
}
